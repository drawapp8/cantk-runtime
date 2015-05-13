LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := libCanvasJNI 
LOCAL_CFLAGS    := -Werror -DNANOVG_GLES2_IMPLEMENTATION -DDEBUG -DANDROID  -Os \
	-I/work/android/android-4.4/frameworks/native/include
LOCAL_SRC_FILES := canvas_jni.cpp nanovg.c color_parser.c command_handlers.c command_dispatcher.c
LOCAL_LDLIBS    := -llog -lGLESv2 -lc -ljnigraphics

include $(BUILD_SHARED_LIBRARY)

