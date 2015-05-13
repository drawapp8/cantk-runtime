/*
 * File     : command_handlers.h
 * Brief    : command handlers
 * Author   : Li XianJing <xianjimli@hotmail.com>
 * Web Site : http://www.tangide.com/
 *
 * Copyright (c) 2015 - 2015 Tangram HD.
 *
**/

#include "nanovg.h"
#include "color_parser.h"

#ifndef COMMAND_HANDLERS_H
#define COMMAND_HANDLERS_H

#ifdef __cplusplus
extern "C" {
#endif/*__cplusplus*/

struct _CommandDispatcher;
typedef struct _CommandDispatcher CommandDispatcher;

typedef struct _Command {
	int type;
	int argc;
	char* argv[10];
	NVGcontext* ctx;
	CommandDispatcher* dispatcher;
}Command;

typedef enum _CommandResult {
	CMD_OK = 0,
	CMD_ERR = -1
}CommandResult;

typedef CommandResult (*ComandHandler)(Command* cmd);

void command_init_handlers(ComandHandler* handlers);

#define CAPTURE_TEXTURE_1 0xff6666
#define CAPTURE_TEXTURE_2 0xff8888

#ifdef __cplusplus
}
#endif/*__cplusplus*/
#endif/*COMMAND_HANDLERS_H*/
