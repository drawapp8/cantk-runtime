/*
 * File     : command_dispatcher.h
 * Brief    : command dispatcher
 * Author   : Li XianJing <xianjimli@hotmail.com>
 * Web Site : http://www.tangide.com/
 *
 * Copyright (c) 2015 - 2015 Tangram HD.
 *
**/

#include "nanovg.h"
#include "color_parser.h"
#include "command_handlers.h"
#include "command_dispatcher.h"

static void command_dump(Command* cmd) {
	int i = 0;
	LOGV("type=%d ", cmd->type);
	LOGV("argc=%d ", cmd->argc);
	for(i = 0; i < cmd->argc; i++) {
		LOGV("arg %d %s\n", i, cmd->argv[i]);
	}

	return;
}

static int command_dispatcher_parse(CommandDispatcher* thiz) {
	const char* cp = thiz->cursor;
	if(!*cp) return 0;
	
	const char*	end = strchr(cp, ';');
	int n = end-cp;
	thiz->cursor = end + 1;

	if(n < sizeof(thiz->cbuff)) {
		strncpy(thiz->cbuff, cp, n);
		thiz->cbuff[n] = '\0';
	}

	LOGV("%s\n", thiz->cbuff);

	char* p = thiz->cbuff;
	Command* cmd = &(thiz->cmd);
	
	cmd->type = *p++;
	int argc = 0;
	char* arg = ++p;

	while(*p) {
		if(*p == ')') {
			if(arg < p) {
				*p = '\0';
				cmd->argv[argc++] = arg;
			}

			break;
		}

		if(*p == ',') {
			*p = '\0';
			cmd->argv[argc++] = arg;
			arg = ++p;
		}

		p++;
	}
	cmd->argc = argc;
	cmd->ctx = thiz->ctx;
	cmd->dispatcher = thiz;

//	command_dump(cmd);

	return 1;
}

void command_dispatcher_init(CommandDispatcher* thiz, void* ctx) 
{
	thiz->ctx = (NVGcontext*)ctx;
	command_init_handlers(thiz->handlers);

	return;
}

void command_dispatcher_set_commands(CommandDispatcher* thiz, const char* str) 
{
	thiz->start = str;
	thiz->cursor = str;

	return;
}

static void command_dispatcher_call(CommandDispatcher* thiz) 
{
	Command* cmd = &(thiz->cmd);
	ComandHandler run = thiz->handlers[cmd->type];

	if(run) {
		run(cmd);
	}

	return;
}

void command_dispatcher_run(CommandDispatcher* thiz, int width, int height) {
	thiz->width = width;
	thiz->height = height;

	while(command_dispatcher_parse(thiz)) {
		command_dispatcher_call(thiz);
	}

	return;
}

int command_dispatcher_gen_test_image(CommandDispatcher* thiz) {
	int i = 0;
	int w = 600;
	int h = 600;
	int n = w * h;
	unsigned int *buffer = NULL;

	if(thiz->testImageID) {
		return thiz->testImageID;
	}

#ifdef STB_IMAGE_IMPLEMENTATION 
	thiz->testImageID = nvgCreateImage(thiz->ctx, "images/image6.jpg", 0);
#else
	buffer = (unsigned int*)malloc(n*sizeof(unsigned int));
	for(i = 0; i < n; i++) {
		buffer[i] = 0xFFFF0000;
	}
	thiz->testImageID = nvgCreateImageRGBA(thiz->ctx, w, h, 0, (unsigned char*)buffer);
	free(buffer);
#endif
	return thiz->testImageID;
}

