/*
 * File     : canvas_jni.cpp
 * Brief    : canvas jni 
 * Author   : Li XianJing <xianjimli@hotmail.com>
 * Web Site : http://www.tangide.com/
 *
 * Copyright (c) 2015 - 2015 Tangram HD.
 *
**/

#include <jni.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/bitmap.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/time.h>    
#include <unistd.h>

static long get_time() {
	struct timeval tv = {0};
	struct timezone tz = {0};

	gettimeofday (&tv, NULL);

	long value = (tv.tv_sec & 0xffff) * 1000 + tv.tv_usec/1000;

	return value;
}

#include "command_dispatcher.h"

#include "nanovg.h"
#include "nanovg_gl.h"
#include <pthread.h>

#define CMD_BUFF_LENGTH 20 * 1024

typedef struct _GraphicContext {
	int w;
	int h;
	int dpi;
	int fontSans;
	int fontSerif;
	int fontSansBold;
	NVGcontext* vg;
	pthread_mutex_t mutex;

	bool commandsChanged;
	char commands[2][CMD_BUFF_LENGTH+1];

	bool showFPS;
	time_t startTime;
	int renderTimes;
}GraphicContext;

static int gResizeTimes = 0;
static CommandDispatcher gCommandDispatcher = {};
static GraphicContext gGraphicContext = {};

static void graphic_context_set_show_fps(bool showFPS) {
	gGraphicContext.showFPS = showFPS;
	gGraphicContext.startTime = get_time();
	gGraphicContext.renderTimes = 1;

	return;
}

static const char* boldFonts[] = {
	"/system/fonts/Miui-Bold.ttf",
	"/system/fonts/Roboto-Bold.ttf",
	NULL
};

static const char* normalFonts[] = {
	"/system/fonts/Miui-Regular.ttf",
	"/system/fonts/Roboto-Regular.ttf",
	"/system/fonts/DroidSans.ttf",
	NULL
};

static void graphic_context_load_fonts() {
	int i = 0;
	const char* fileName = NULL;
#ifdef FONTSTASH_IMPLEMENTATION
	for(i = 0; boldFonts[i]; i++) {
		fileName = boldFonts[i];
		gGraphicContext.fontSansBold = nvgCreateFont(gGraphicContext.vg, "sans-bold", fileName);
		if(gGraphicContext.fontSansBold >= 0) {
			LOGI("load font: %s %d\n", fileName, gGraphicContext.fontSansBold);
			break;
		}
	}
	
	for(i = 0; normalFonts[i]; i++) {
		fileName = normalFonts[i];
		gGraphicContext.fontSans = nvgCreateFont(gGraphicContext.vg, "sans", fileName);
		if(gGraphicContext.fontSans >= 0) {
			LOGI("load font: %s %d\n", fileName, gGraphicContext.fontSans);
			break;
		}
	}
#endif	
	return;
}

static void graphic_context_init() {
	memset(&gGraphicContext, 0x00, sizeof(gGraphicContext));
	pthread_mutex_init(&gGraphicContext.mutex, NULL);
	gGraphicContext.vg = nvgCreateGLES2(NVG_STENCIL_STROKES | NVG_DEBUG);
	command_dispatcher_init(&gCommandDispatcher, gGraphicContext.vg);
	graphic_context_load_fonts();

	LOGI("graphic_context_init\n");

	return;
}

static void graphic_context_resize(int w, int h, int dpi) {
	gResizeTimes++;
	
	glClearDepthf(1.0f);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

	gGraphicContext.w = w;
	gGraphicContext.h = h;
	glViewport(0, 0, gGraphicContext.w, gGraphicContext.h);

	graphic_context_set_show_fps(true);

	LOGI("graphic_context_resize: times=%d w=%d h=%d dpi=%d\n", gResizeTimes, w, h, dpi);

	return;
}

static char* graphic_context_get_render_commands() {
	if(gGraphicContext.commandsChanged) {
		pthread_mutex_lock(&gGraphicContext.mutex);
		gGraphicContext.commandsChanged = false;
		strcpy(gGraphicContext.commands[0], gGraphicContext.commands[1]);
		pthread_mutex_unlock(&gGraphicContext.mutex);
		//LOGI("renderCommands:%s\n", gGraphicContext.commands[0]);
	}

	return gGraphicContext.commands[0];
}

static void graphic_context_exec() {
	const char* commands = NULL;
	NVGcontext* vg = gGraphicContext.vg;
	
	commands = graphic_context_get_render_commands();
	if(!commands[0]) {
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
		
		nvgBeginFrame(vg, gGraphicContext.w, gGraphicContext.h, 1);
		nvgEndFrame(vg);
	}
	else {
		nvgBeginFrame(vg, gGraphicContext.w, gGraphicContext.h, 1);
		command_dispatcher_set_commands(&gCommandDispatcher, commands);
		command_dispatcher_run(&gCommandDispatcher, gGraphicContext.w, gGraphicContext.h);
		nvgEndFrame(vg);
	}
	gGraphicContext.renderTimes++;

	int dt = get_time() - gGraphicContext.startTime;
	if(dt > 0) {
		int fps = (1000 * gGraphicContext.renderTimes)/dt;
//		LOGI("fps=%d gGraphicContext.renderTimes=%d dt=%d\n", fps, gGraphicContext.renderTimes, dt);
	}

	return;
}

static void graphic_context_delete_image(int imageID) {
	NVGcontext* vg = gGraphicContext.vg;

	nvgDeleteImage(vg, imageID);

	LOGI("delete image %d\n", imageID);

	return;
}

static const char* str_from_jchars(const jchar* in, int inLen, char* out, int outLen) {
	int i = 0;
	int n = inLen < (outLen-1) ? inLen : outLen-1;
	for(i = 0; i < n; i++) {
		out[i] = (char)in[i];
	}
	out[i] = '\0';

	return out;
}

/////////////////////////////////////////////////////////////////////////////

#define JNIAPI extern "C" JNIEXPORT void JNICALL
#define JNIAPIINT extern "C" JNIEXPORT int JNICALL

JNIAPI Java_com_tangide_cantk_CanvasJNI_surfaceCreated(JNIEnv * env, jobject obj) 
{
	graphic_context_init();
}

JNIAPI Java_com_tangide_cantk_CanvasJNI_surfaceChanged(JNIEnv * env, jobject obj,  
	jint width, jint height, jint density)
{
	graphic_context_resize(width, height, 0);
}

JNIAPI Java_com_tangide_cantk_CanvasJNI_render(JNIEnv * env, jobject obj)
{
	graphic_context_exec();
}

JNIAPIINT Java_com_tangide_cantk_CanvasJNI_setRenderCommands(JNIEnv * env, jobject obj, jstring renderCommands) 
{
	int fps = 0;
	const char* p = NULL;
	const jchar* str = env->GetStringCritical(renderCommands, 0);
	int length = env->GetStringLength(renderCommands);
	if(length < CMD_BUFF_LENGTH) {
		pthread_mutex_lock(&gGraphicContext.mutex);
		gGraphicContext.commandsChanged = true;
		p = str_from_jchars(str, length, gGraphicContext.commands[1], CMD_BUFF_LENGTH);
		pthread_mutex_unlock(&gGraphicContext.mutex);
	}
	env->ReleaseStringCritical(renderCommands, str);
	
	int dt = get_time() - gGraphicContext.startTime;
	if(dt > 0) {
		fps = (1000 * gGraphicContext.renderTimes)/dt;
	}

	fps = fps > 60 ? 60 : fps;

//	LOGI("fps=%d\n", fps);

	return fps;
}

JNIAPI Java_com_tangide_cantk_CanvasJNI_deleteImage(JNIEnv * env, jobject obj, jint id)
{
	graphic_context_delete_image(id);
}

JNIAPI Java_com_tangide_cantk_CanvasJNI_setShowFPS(JNIEnv * env, jobject obj, jint showFPS) {
	graphic_context_set_show_fps(showFPS);

	return;
}

JNIAPI Java_com_tangide_cantk_CanvasJNI_loadImage(JNIEnv * env, jobject obj, 
	jstring url, jobject bitmap, jobject imageInfo) 
{
	int id = 0;
	int width = 0;
	int height = 0;
	void* pixels = NULL;
	AndroidBitmapInfo  info;
	NVGcontext* vg = gGraphicContext.vg;

	if(AndroidBitmap_getInfo(env, bitmap, &info) < 0) {
		LOGI("AndroidBitmap_getInfo failed.");
		return;
	}

	width = info.width;
	height = info.height;

	LOGI("Bitmap info: w=%d h=%d format=%d\n", width, height, info.format);

	if(info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
		LOGI("bitmap has invalid format");
		return;
	}

	if(AndroidBitmap_lockPixels(env, bitmap, &pixels) < 0) {
		LOGI("AndroidBitmap_lockPixels failed.");
		return;
	}

	id = nvgCreateImageRGBA(vg, width, height, 0, (const unsigned char*)pixels);
	LOGI("Bitmap info: w=%d h=%d format=%d id=%d\n", width, height, info.format, id);

	AndroidBitmap_unlockPixels(env, bitmap);

	jclass cls = env->GetObjectClass(imageInfo);
	jfieldID fid = env->GetFieldID(cls, "id", "I");
	env->SetIntField(imageInfo, fid, (int)id);

	fid = env->GetFieldID(cls, "width", "I");
	env->SetIntField(imageInfo, fid, (int)width);
	
	fid = env->GetFieldID(cls, "height", "I");
	env->SetIntField(imageInfo, fid, (int)height);

	return;
}

