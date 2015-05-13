/*
 * File     : command_dispatcher.h
 * Brief    : command dispatcher
 * Author   : Li XianJing <xianjimli@hotmail.com>
 * Web Site : http://www.tangide.com/
 *
 * Copyright (c) 2015 - 2015 Tangram HD.
 *
**/

#include "command_handlers.h"

#ifndef COMMAND_DISPATCHER_H
#define COMMAND_DISPATCHER_H

#ifdef __cplusplus
extern "C" {
#endif/*__cplusplus*/

struct _CommandDispatcher {
	NVGcontext* ctx;
	const char* start;
	const char* cursor;
	
	Command cmd;
	char cbuff[1024];
	int textAlign;
	int textBaseline;
	int testImageID;
	ComandHandler handlers[255];
	int rectX;
	int rectY;
	int rectW;
	int rectH;

	int width;
	int height;
};

void command_dispatcher_init(CommandDispatcher* thiz, void* ctx);

void command_dispatcher_set_commands(CommandDispatcher* thiz, const char* str);
void command_dispatcher_run(CommandDispatcher* thiz, int width, int height);

void command_dispatcher_load_image(CommandDispatcher* thiz, const char* url, int urlLength, 
	const char* data, int dataLength, int*id, int* width, int* height);

int command_dispatcher_gen_test_image(CommandDispatcher* thiz);

#ifdef __cplusplus
}
#endif/*__cplusplus*/
#endif/*COMMAND_DISPATCHER_H*/

