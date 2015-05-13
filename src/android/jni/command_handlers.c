/*
 * File     : command_handlers.c
 * Brief    : command handlers
 * Author   : Li XianJing <xianjimli@hotmail.com>
 * Web Site : http://www.tangide.com/
 *
 * Copyright (c) 2015 - 2015 Tangram HD.
 *
**/

#include "color_parser.h"
#include "command_handlers.h"
#include "command_dispatcher.h"

static int command_get_text_align(Command* cmd) 
{
	return cmd->dispatcher->textAlign;
}

static int command_get_text_baseline(Command* cmd) 
{
	return cmd->dispatcher->textBaseline;
}

static void command_save_text_align(Command* cmd, int textAlign) 
{
	cmd->dispatcher->textAlign = textAlign;

	return;	
}

static void command_save_text_baseline(Command* cmd, int textBaseline) 
{
	cmd->dispatcher->textBaseline = textBaseline;

	return;	
}

static float clampf(float a, float mn, float mx) { return a < mn ? mn : (a > mx ? mx : a); }

static const char* command_get_arg_str(Command* cmd, int index, const char* name) 
{
	const char* str = (index >= 0 && index < cmd->argc) ? cmd->argv[index] : NULL;

	return str;
}

static float command_get_arg_int(Command* cmd, int index, const char* name) 
{
	const char* str = command_get_arg_str(cmd, index, name);

	return str ? atoi(str) : 0;
}

static float command_get_arg_img(Command* cmd, int index, const char* name) 
{
	int id = command_get_arg_int(cmd, index, name);

	return id;
}

static float command_get_arg_float(Command* cmd, int index, const char* name) 
{
	const char* str = command_get_arg_str(cmd, index, name);

	return str ? atof(str) : 0.0f;
}

static NVGcolor command_get_arg_color(Command* cmd, int index, const char* name) 
{
	NVGcolor  color;
	const char* str = command_get_arg_str(cmd, index, name);

	ColorRGBA rgba = parseColor(str ? str : "#00000000");

	color.r = rgba.r/255.f;
	color.g = rgba.g/255.f;
	color.b = rgba.b/255.f;
	color.a = rgba.a/255.f;

	return color;
}

static int command_get_arg_bool(Command* cmd, int index, const char* name) 
{
	const char* str = command_get_arg_str(cmd, index, name);
	
	return (str && str[0] == 't') ? 1 : 0;
}

static int command_check_arg(Command* cmd, int index, const char* key, const char* name) 
{
	const char* str = command_get_arg_str(cmd, index, name);
	
	return str && strstr(str, key) != NULL;
}

static int atoi_r(const char* str, const char* start) {
	int i = 0;
	int value = 0;
	const char* p = str;

	while(p >= start) {
		char c = *p;
		if(c >= '0' && c <= '9') {
			c = c - '0';
			value = value + (i ? (i * 10 * c) :  c );
		}
		else {
			break;
		}
		i++;
		p--;
	}

	return value;
}

static int command_get_arg_font_size(Command* cmd, int index, const char* name) 
{
	int fontSize = 12;
	const char* px = NULL;
	const char* pt = NULL;
	const char* str = command_get_arg_str(cmd, index, name);

	if(str) {
		px = strstr(str, "px");
		pt = strstr(str, "pt");
		if(px) {
			fontSize = atoi_r(px-1, str);
		}
		else if(pt) {
			fontSize = atoi_r(pt-1, str) * 1.5;
		}
	}

	return fontSize;
}

static CommandResult command_set_linewidth(Command* cmd) {
	int lineWidth = atoi(cmd->argv[0]);
	LOGV("lineWidth:%d\n", lineWidth);
	nvgStrokeWidth(cmd->ctx, lineWidth);

	return CMD_OK;
}

static CommandResult command_set_fillstyle(Command* cmd) {
	NVGcolor fillStyle = command_get_arg_color(cmd, 0, "fillStyle");
	nvgFillColor(cmd->ctx, fillStyle);

	return CMD_OK;
}

static CommandResult command_set_strokestyle(Command* cmd) {
	NVGcolor strokeStyle = command_get_arg_color(cmd, 0, "strokeStyle");
	nvgStrokeColor(cmd->ctx, strokeStyle);

	return CMD_OK;
}

static CommandResult command_set_globalalpha(Command* cmd) {
	float alpha = clampf(command_get_arg_float(cmd, 0, "globalAlpha"), 0, 1);

	nvgGlobalAlpha(cmd->ctx, alpha);

	return CMD_OK;
}

static CommandResult command_set_globalcompositeoperation(Command* cmd) {
	/*TODO*/	
	return CMD_OK;
}

static CommandResult command_set_textalign(Command* cmd) {
	enum NVGalign textAlign = NVG_ALIGN_LEFT;
	const char* str = command_get_arg_str(cmd, 0, "textAlign");

	if(!str || str[0] == 'l') {
		textAlign = NVG_ALIGN_LEFT;
	}
	else if(str[0] == 'r') {
		textAlign = NVG_ALIGN_RIGHT;
	}
	else {
		textAlign = NVG_ALIGN_CENTER;
	}

	LOGV("set_textalign %s %d\n", str, textAlign);
	command_save_text_align(cmd, textAlign);
	
	return CMD_OK;
}

static CommandResult command_set_textbaseline(Command* cmd) {
	enum NVGalign textBaseline = NVG_ALIGN_TOP;
	const char* str = command_get_arg_str(cmd, 0, "textBaseline");

	if(!str || str[0] == 't') {
		textBaseline = NVG_ALIGN_TOP;
	}
	else if(str[0] == 'b') {
		textBaseline = NVG_ALIGN_BOTTOM;
	}
	else {
		textBaseline = NVG_ALIGN_MIDDLE;
	}

	LOGV("set_textbaseline %s %d\n", str, textBaseline);
	command_save_text_baseline(cmd, textBaseline);
	
	return CMD_OK;
}

static CommandResult command_set_font(Command* cmd) {
	int fontSize = command_get_arg_font_size(cmd, 0, "font");
	int bold = command_check_arg(cmd, 0, "bold", "font");
	int italic = command_check_arg(cmd, 0, "italic", "font");

	LOGV("setfont: fontSize=%d bold=%d, italic=%d\n", fontSize, bold, italic);

	nvgFontSize(cmd->ctx, fontSize);
	if(bold) {
		nvgFontFace(cmd->ctx, "sans-bold");
	}
	else {
		nvgFontFace(cmd->ctx, "sans");
	}

	return CMD_OK;
}

static CommandResult command_set_shadowoffsetx(Command* cmd) 
{
}

static CommandResult command_set_shadowoffsety(Command* cmd) 
{
}

static CommandResult command_set_shadowblur(Command* cmd) 
{
}

static CommandResult command_set_shadowcolor(Command* cmd) 
{
}

CommandResult command_set_linecap(Command* cmd) 
{
	int lineCap = NVG_BUTT;
	const char* str = command_get_arg_str(cmd, 0, "lineCap");
	if(str && str[0] == 's') {
		lineCap = NVG_SQUARE;
	}
	else if(str && str[0] == 'r') {
		lineCap = NVG_ROUND;
	}

	LOGV("set_linecap: linecap=%d\n", lineCap);

	nvgLineCap(cmd->ctx, lineCap);

	return CMD_OK;
}

CommandResult command_set_linejoin(Command* cmd) 
{
	int lineJoin = NVG_MITER;
	const char* str = command_get_arg_str(cmd, 0, "lineJoin");

	if(str && str[0] == 'r') {
		lineJoin = NVG_ROUND;
	}
	else if(str && str[0] == 'b') {
		lineJoin = NVG_BEVEL;
	}

	nvgLineJoin(cmd->ctx, lineJoin);

	return CMD_OK;

}

CommandResult command_set_miterlimit(Command* cmd) 
{
	float miterLimit = command_get_arg_float(cmd, 0, "miterLimit");

	nvgMiterLimit(cmd->ctx, miterLimit);

	return CMD_OK;
}

static CommandResult command_save(Command* cmd) {
	LOGV("save\n");
	nvgSave(cmd->ctx);

	return CMD_OK;
}

static CommandResult command_restore(Command* cmd) {
	LOGV("restore\n");
	nvgRestore(cmd->ctx);

	return CMD_OK;
}

static CommandResult command_clip(Command* cmd) {
	int xx = 0;
	int yy = 0;
	int ww = 0;
	int hh = 0;

	NVGcontext* ctx = cmd->ctx;
	float x = cmd->dispatcher->rectX;
	float y = cmd->dispatcher->rectY;
	float w = cmd->dispatcher->rectW;
	float h = cmd->dispatcher->rectH;
	float r = x + w;
	float b = y + h;
	
	nvgTransformPointWithXForm(ctx, x, y, &x, &y);
	nvgTransformPointWithXForm(ctx, r, b, &r, &b);

	xx = x;
	yy = y;
	ww = r - x;
	hh = b - y;

	LOGI("clip: %d %d %d %d\n", xx, yy, ww, hh);
	nvgScissor(ctx, xx, yy, ww, hh);

//for debug
//	nvgStrokeColor(ctx, nvgRGBA(0,0,255,255));
//	nvgStrokeWidth(ctx, 3.0f);
//	nvgStroke(ctx);

	return CMD_OK;
}

static CommandResult command_resetclip(Command* cmd) {
	LOGV("resetClip\n");
	nvgResetScissor(cmd->ctx);

	return CMD_OK;
}


static CommandResult command_fill(Command* cmd) {
	LOGV("fill\n");
	nvgFill(cmd->ctx);

	return CMD_OK;
}


static CommandResult command_stroke(Command* cmd) {
	LOGV("stroke\n");
	nvgStroke(cmd->ctx);

	return CMD_OK;
}


static CommandResult command_beginpath(Command* cmd) {
	LOGV("beginpath\n");
	nvgBeginPath(cmd->ctx);

	return CMD_OK;
}


static CommandResult command_closepath(Command* cmd) {
	LOGV("closepath\n");
	nvgClosePath(cmd->ctx);

	return CMD_OK;
}


static CommandResult command_moveto(Command* cmd) {
	int x = command_get_arg_int(cmd, 0, "x");
	int y = command_get_arg_int(cmd, 1, "y");
	LOGV("moveto x=%d y=%d\n", x, y);

	nvgMoveTo(cmd->ctx, x, y);

	return CMD_OK;
}


static CommandResult command_lineto(Command* cmd) {
	int x = command_get_arg_int(cmd, 0, "x");
	int y = command_get_arg_int(cmd, 1, "y");
	LOGV("lineto x=%d y=%d\n", x, y);

	nvgLineTo(cmd->ctx, x, y);

	return CMD_OK;
}


static CommandResult command_rotate(Command* cmd) {
	float angle = command_get_arg_float(cmd, 0, "angle");
	LOGV("rotate angle%f\n", angle);

	nvgRotate(cmd->ctx, angle);

	return CMD_OK;
}


static CommandResult command_scale(Command* cmd) {
	float x = command_get_arg_float(cmd, 0, "x");
	float y = command_get_arg_float(cmd, 1, "y");
	LOGV("scale x=%f y=%f\n", x, y);
	nvgScale(cmd->ctx, x, y);

	return CMD_OK;
}

static CommandResult command_skewx(Command* cmd) {
	float angle = command_get_arg_float(cmd, 0, "angle");
	LOGV("skeyx angle=%f\n", angle);
	nvgSkewX(cmd->ctx, angle);

	return CMD_OK;
}


static CommandResult command_skewy(Command* cmd) {
	float angle = command_get_arg_float(cmd, 0, "angle");
	LOGV("skeyy angle=%f\n", angle);
	nvgSkewY(cmd->ctx, angle);

	return CMD_OK;
}


static CommandResult command_translate(Command* cmd) {
	int x = command_get_arg_int(cmd, 0, "x");
	int y = command_get_arg_int(cmd, 1, "y");
	LOGV("translate x=%d y=%d\n", x, y);

	nvgTranslate(cmd->ctx, x, y);

	return CMD_OK;
}


static CommandResult command_rect(Command* cmd) {
	int x = command_get_arg_int(cmd, 0, "x");
	int y = command_get_arg_int(cmd, 1, "y");
	int w = command_get_arg_int(cmd, 2, "w");
	int h = command_get_arg_int(cmd, 3, "h");

	w = w > 0 ? w : 0;
	h = h > 0 ? h : 0;

	cmd->dispatcher->rectX = x;
	cmd->dispatcher->rectY = y;
	cmd->dispatcher->rectW = w;
	cmd->dispatcher->rectH = h;

	LOGV("rect: x=%d y=%d, w=%d, h=%d\n", x, y, w, h);
	nvgRect(cmd->ctx, x, y, w, h);

	return CMD_OK;
}

static CommandResult command_circle(Command* cmd) {
	int x = command_get_arg_int(cmd, 0, "x");
	int y = command_get_arg_int(cmd, 1, "y");
	int r = command_get_arg_int(cmd, 2, "r");
	
	LOGV("circle: x=%d y=%d, r=%d\n", x, y, r);
	nvgCircle(cmd->ctx, x, y, r);

	return CMD_OK;
}


static CommandResult command_ellipse(Command* cmd) {
	int x = command_get_arg_int(cmd, 0, "x");
	int y = command_get_arg_int(cmd, 1, "y");
	int rx = command_get_arg_int(cmd, 2, "rx");
	int ry = command_get_arg_int(cmd, 3, "ry");
	
	LOGV("ellipse: x=%d y=%d, rx=%d ry=%d\n", x, y, rx, ry);
	nvgEllipse(cmd->ctx, x, y, rx, ry);

	return CMD_OK;
}


static CommandResult command_roundrect(Command* cmd) {
	int x = command_get_arg_int(cmd, 0, "x");
	int y = command_get_arg_int(cmd, 1, "y");
	int w = command_get_arg_int(cmd, 2, "w");
	int h = command_get_arg_int(cmd, 3, "h");
	int r = command_get_arg_int(cmd, 4, "r");

	LOGV("roundrect: x=%d y=%d, w=%d, h=%d r=%d\n", x, y, w, h, r);
	nvgRoundedRect(cmd->ctx, x, y, w, h, r);

	return CMD_OK;
}


static CommandResult command_arcto(Command* cmd) {
	int x1 = command_get_arg_int(cmd, 0, "x1");
	int y1 = command_get_arg_int(cmd, 1, "y1");
	int x2 = command_get_arg_int(cmd, 2, "x2");
	int y2 = command_get_arg_int(cmd, 3, "y2");
	int r = command_get_arg_int(cmd, 4, "r");

	LOGV("arcto x1=%d y1=%d x2=%d y2=%d r=%d\n", x1, y1, x2, y2, r);
	nvgArcTo(cmd->ctx, x1, y1, x2, y2, r);

	return CMD_OK;
}


static CommandResult command_quadto(Command* cmd) {
	int cx = command_get_arg_int(cmd, 0, "cx");
	int cy = command_get_arg_int(cmd, 1, "cy");
	int x  = command_get_arg_int(cmd, 2, "x");
	int y  = command_get_arg_int(cmd, 3, "y");

	LOGV("quadto cx=%d cy=%d x=%d y=%d\n", cx, cy, x, y);
	nvgQuadTo(cmd->ctx, cx, cy, x, y);

	return CMD_OK;
}


static CommandResult command_bezierto(Command* cmd) {
	int cx1 = command_get_arg_int(cmd, 0, "cx1");
	int cy1 = command_get_arg_int(cmd, 1, "cy1");
	int cx2 = command_get_arg_int(cmd, 2, "cx2");
	int cy2 = command_get_arg_int(cmd, 3, "cy2");
	int x  = command_get_arg_int(cmd, 4, "x");
	int y  = command_get_arg_int(cmd, 5, "y");

	LOGV("bezierto cx1=%d cy1=%d  cx2=%d cy2=%d x=%d y=%d\n", cx1, cy1, cx2, cy2, x, y);
	nvgBezierTo(cmd->ctx, cx1, cy1, cx2, cy2, x, y);

	return CMD_OK;
}


static CommandResult command_clearrect(Command* cmd) {
	int x = command_get_arg_int(cmd, 0, "x");
	int y = command_get_arg_int(cmd, 1, "y");
	int w = command_get_arg_int(cmd, 2, "w");
	int h = command_get_arg_int(cmd, 3, "h");

	LOGV("clearrect: x=%d y=%d, w=%d, h=%d\n", x, y, w, h);
	nvgBeginPath(cmd->ctx);
	nvgFillColor(cmd->ctx, nvgRGBA(0,0,0,255));
	nvgRect(cmd->ctx, x, y, w, h);
	nvgFill(cmd->ctx);
	nvgBeginPath(cmd->ctx);

	return CMD_OK;
}

static CommandResult command_fillrect(Command* cmd) {
	int x = command_get_arg_int(cmd, 0, "x");
	int y = command_get_arg_int(cmd, 1, "y");
	int w = command_get_arg_int(cmd, 2, "w");
	int h = command_get_arg_int(cmd, 3, "h");

	LOGV("clearrect: x=%d y=%d, w=%d, h=%d\n", x, y, w, h);
	nvgBeginPath(cmd->ctx);
	nvgRect(cmd->ctx, x, y, w, h);
	nvgFill(cmd->ctx);
	nvgBeginPath(cmd->ctx);

	return CMD_OK;
}

static CommandResult command_strokerect(Command* cmd) {
	int x = command_get_arg_int(cmd, 0, "x");
	int y = command_get_arg_int(cmd, 1, "y");
	int w = command_get_arg_int(cmd, 2, "w");
	int h = command_get_arg_int(cmd, 3, "h");

	LOGV("strokerect: x=%d y=%d, w=%d, h=%d\n", x, y, w, h);
	nvgBeginPath(cmd->ctx);
	nvgRect(cmd->ctx, x, y, w, h);
	nvgStroke(cmd->ctx);
	nvgBeginPath(cmd->ctx);

	return CMD_OK;
}

static CommandResult command_arc(Command* cmd) 
{
	int x = command_get_arg_int(cmd, 0, "x");
	int y = command_get_arg_int(cmd, 1, "y");
	int r = command_get_arg_int(cmd, 2, "r");
	float angle1 = command_get_arg_float(cmd, 3, "angle1");
	float angle2 = command_get_arg_float(cmd, 4, "angle2");
	int   ccw    = command_get_arg_bool(cmd,  5, "ccw");

	LOGV("arc: x=%d y=%d, r=%d, a1=%f a2=%f ccw=%d\n", x, y, r, angle1, angle2, ccw);
	nvgArc(cmd->ctx, x, y, r, angle1, angle2, ccw ? NVG_CCW : NVG_CW);

	return CMD_OK;
}

static CommandResult command_transform(Command* cmd) 
{
	float a = command_get_arg_float(cmd, 0, "a");
	float b = command_get_arg_float(cmd, 1, "b");
	float c = command_get_arg_float(cmd, 2, "c");
	float d = command_get_arg_float(cmd, 3, "d");
	float e = command_get_arg_float(cmd, 4, "e");
	float f = command_get_arg_float(cmd, 5, "f");

	LOGV("transform: a=%f b=%f c=%f d=%f e=%f f=%f\n", a, b, c, d, e, f);

	nvgTransform(cmd->ctx, a, b, c, d, e, f);

	return CMD_OK;
}

static CommandResult command_settransform(Command* cmd) 
{
	nvgResetTransform(cmd->ctx);

	return command_transform(cmd);
}

static CommandResult command_resettransform(Command* cmd) 
{
	nvgResetTransform(cmd->ctx);

	return CMD_OK;
}

static CommandResult command_filltext3(Command* cmd) 
{
	const char* str = command_get_arg_str(cmd, 0, "text");
	int x = command_get_arg_int(cmd, 1, "x");
	int y = command_get_arg_int(cmd, 2, "y");

	int textAlign = command_get_text_baseline(cmd) | command_get_text_align(cmd);

	LOGV("filltext: x=%d y=%d, textalign=%04x str=%s\n", x, y, textAlign, str);
	
	nvgTextAlign(cmd->ctx, textAlign);
	nvgText(cmd->ctx, x, y, str, NULL);

	return CMD_OK;
}

static CommandResult command_stroketext3(Command* cmd) {
	int x = command_get_arg_int(cmd, 0, "x");
	int y = command_get_arg_int(cmd, 1, "y");
	const char* str = command_get_arg_str(cmd, 2, "text");
	LOGV("filltext: x=%d y=%d, str=%s\n", x, y, str);
	nvgText(cmd->ctx, x, y, str, NULL);

	return CMD_OK;
}

static CommandResult command_drawimage3(Command* cmd) {
	int iw = 0;
	int ih = 0;
	NVGpaint imgPaint;
	NVGcontext* vg = cmd->ctx;
	int img = command_get_arg_img(cmd, 0, "img");
	int x = command_get_arg_int(cmd, 1, "x");
	int y = command_get_arg_int(cmd, 2, "y");

#ifdef USE_TEST_IMAGE
	img = command_dispatcher_gen_test_image(cmd->dispatcher);
#endif

	nvgImageSize(vg, img, &iw, &ih);
	
	LOGV("drawimage3: img=%d x=%d y=%d iw=%d ih=%d\n", img, x, y, iw, ih);
	if(iw < 1 || ih < 1) {
		LOGV("drawimage3: not found img %d\n", img);
		return CMD_ERR;
	}

	//imgPaint = nvgImagePattern(vg, 0, 0, iw, ih, 0, img, 1);
	imgPaint = nvgImagePatternSimple(vg, img);

	nvgSave(vg);
	nvgBeginPath(vg);
	nvgTranslate(vg, x, y);
	nvgRect(vg, 0, 0, iw, ih);	
	nvgFillPaint(vg, imgPaint);
	nvgClosePath(vg);
	nvgFill(vg);
	nvgRestore(vg);

	return CMD_OK;
}


static CommandResult command_drawimage5(Command* cmd) {
	int iw = 0;
	int ih = 0;
	float scaleX = 1;
	float scaleY = 1;
	NVGpaint imgPaint;
	NVGcontext* vg = cmd->ctx;
	int img = command_get_arg_img(cmd, 0, "img");
	int x = command_get_arg_int(cmd, 1, "x");
	int y = command_get_arg_int(cmd, 2, "y");
	int w = command_get_arg_int(cmd, 3, "w");
	int h = command_get_arg_int(cmd, 4, "h");
#ifdef USE_TEST_IMAGE
	img = cmd->dispatcher->testImageID;
#endif
	nvgImageSize(vg, img, &iw, &ih);
	
	if(iw < 1 || ih < 1) {
		LOGV("drawimage5: not found img %d\n", img);
		return CMD_ERR;
	}

	scaleX = (float)w/iw;
	scaleY = (float)h/ih;
	
	LOGV("drawimage5: img=%d x=%d y=%d iw=%d ih=%d w=%d h=%d scaleX=%f scaleY=%f\n", 
		img, x, y, iw, ih, w, h, scaleX, scaleY);
	//imgPaint = nvgImagePattern(vg, 0, 0, iw, ih, 0, img, 1);
	imgPaint = nvgImagePatternSimple(vg, img);

	nvgSave(vg);
	nvgBeginPath(vg);
	nvgTranslate(vg, x, y);
	nvgScale(vg, scaleX, scaleY);
	nvgRect(vg, 0, 0, iw, ih);	
	nvgFillPaint(vg, imgPaint);
	nvgClosePath(vg);
	nvgFill(vg);
	nvgRestore(vg);

	return CMD_OK;
}

static CommandResult command_drawimage9(Command* cmd) {
	int iw = 0;
	int ih = 0;
	float scaleX = 1;
	float scaleY = 1;
	NVGpaint imgPaint;
	NVGcontext* vg = cmd->ctx;
	int img = command_get_arg_img(cmd, 0, "img");
	int sx = command_get_arg_int(cmd, 1, "sx");
	int sy = command_get_arg_int(cmd, 2, "sy");
	int sw = command_get_arg_int(cmd, 3, "sw");
	int sh = command_get_arg_int(cmd, 4, "sh");
	int x = command_get_arg_int(cmd, 5, "x");
	int y = command_get_arg_int(cmd, 6, "y");
	int w = command_get_arg_int(cmd, 7, "w");
	int h = command_get_arg_int(cmd, 8, "h");
#ifdef USE_TEST_IMAGE
	img = cmd->dispatcher->testImageID;
#endif
	nvgImageSize(vg, img, &iw, &ih);
	
	LOGV("drawimage9: img=%d sx=%d sy=%d sw=%d sh=%d dx=%d dy=%d dw=%d dh=%d iw=%d ih=%d\n", 
		img, sx, sy, sw, sh, x, y, w, h, iw, ih);
	if(iw < 1 || ih < 1) {
		LOGV("drawimage9: not found img %d\n", img);
		return CMD_ERR;
	}

	if(sx >= iw || sy >= ih || (sx + sw) > iw || (sy + sh) > ih) {
		LOGV("drawimage9: invalid args\n");
		return CMD_ERR;
	}

	scaleX = (float)w/sw;
	scaleY = (float)h/sh;
	//imgPaint = nvgImagePattern(vg, 0, 0, iw, ih, 0, img, 1);
	imgPaint = nvgImagePatternSimple(vg, img);

	nvgSave(vg);
	nvgBeginPath(vg);
	nvgTranslate(vg, x-(sx*scaleX), y-(sy*scaleY));
	nvgScale(vg, scaleX, scaleY);
	nvgRect(vg, sx, sy, sw, sh);	
	nvgFillPaint(vg, imgPaint);
	nvgClosePath(vg);
	nvgFill(vg);
	nvgRestore(vg);

	return CMD_OK;
}

//////////////////////////////////////////////////////////////////////

typedef enum _CommandCode {
	CMD_START_CODE = 65,
	CMD_SET_LINEWIDTH = 65,
	CMD_SET_FILLSTYLE = 66,
	CMD_SET_STROKESTYLE = 67,
	CMD_SET_GLOBALALPHA = 68,
	CMD_SET_GLOBALCOMPOSITEOPERATION = 69,
	CMD_SET_TEXTALIGN = 70,
	CMD_SET_TEXTBASELINE = 71,
	CMD_SET_FONT = 72,
	CMD_SET_SHADOWOFFSETX = 73,
	CMD_SET_SHADOWOFFSETY = 74,
	CMD_SET_SHADOWBLUR = 75,
	CMD_SET_SHADOWCOLOR = 76,
	CMD_SET_LINECAP = 77,
	CMD_SET_LINEJOIN = 78,
	CMD_SET_MITERLIMIT = 79,
	CMD_SAVE = 80,
	CMD_RESTORE = 81,
	CMD_CLIP = 82,
	CMD_RESETCLIP = 83,
	CMD_FILL = 84,
	CMD_STROKE = 85,
	CMD_BEGINPATH = 86,
	CMD_CLOSEPATH = 87,
	CMD_MOVETO = 88,
	CMD_LINETO = 89,
	CMD_ROTATE = 90,
	CMD_SCALE = 91,
	CMD_SKEWX = 92,
	CMD_SKEWY = 93,
	CMD_TRANSLATE = 94,
	CMD_RECT = 95,
	CMD_CIRCLE = 96,
	CMD_ELLIPSE = 97,
	CMD_ROUNDRECT = 98,
	CMD_ARCTO = 99,
	CMD_QUADTO = 100,
	CMD_BEZIERTO = 101,
	CMD_CLEARRECT = 102,
	CMD_FILLRECT = 103,
	CMD_STROKERECT = 104,
	CMD_ARC = 105,
	CMD_TRANSFORM = 106,
	CMD_SETTRANSFORM = 107,
	CMD_RESETTRANSFORM = 108,
	CMD_FILLTEXT3 = 109,
	CMD_STROKETEXT3 = 110,
	CMD_DRAWIMAGE3 = 111,
	CMD_DRAWIMAGE5 = 112,
	CMD_DRAWIMAGE9 = 113,
	CMD_END_CODE = 114
} CommandCode;

void command_init_handlers(ComandHandler* handlers) {

	handlers[CMD_SET_LINEWIDTH] = command_set_linewidth;
	handlers[CMD_SET_FILLSTYLE] = command_set_fillstyle;
	handlers[CMD_SET_STROKESTYLE] = command_set_strokestyle;
	handlers[CMD_SET_GLOBALALPHA] = command_set_globalalpha;
	handlers[CMD_SET_GLOBALCOMPOSITEOPERATION] = command_set_globalcompositeoperation;
	handlers[CMD_SET_TEXTALIGN] = command_set_textalign;
	handlers[CMD_SET_TEXTBASELINE] = command_set_textbaseline;
	handlers[CMD_SET_FONT] = command_set_font;
	handlers[CMD_SET_SHADOWOFFSETX] = command_set_shadowoffsetx;
	handlers[CMD_SET_SHADOWOFFSETY] = command_set_shadowoffsety;
	handlers[CMD_SET_SHADOWBLUR] = command_set_shadowblur;
	handlers[CMD_SET_SHADOWCOLOR] = command_set_shadowcolor;
	handlers[CMD_SET_LINECAP] = command_set_linecap;
	handlers[CMD_SET_LINEJOIN] = command_set_linejoin;
	handlers[CMD_SET_MITERLIMIT] = command_set_miterlimit;
	handlers[CMD_SAVE] = command_save;
	handlers[CMD_RESTORE] = command_restore;
	handlers[CMD_CLIP] = command_clip;
	handlers[CMD_RESETCLIP] = command_resetclip;
	handlers[CMD_FILL] = command_fill;
	handlers[CMD_STROKE] = command_stroke;
	handlers[CMD_BEGINPATH] = command_beginpath;
	handlers[CMD_CLOSEPATH] = command_closepath;
	handlers[CMD_MOVETO] = command_moveto;
	handlers[CMD_LINETO] = command_lineto;
	handlers[CMD_ROTATE] = command_rotate;
	handlers[CMD_SCALE] = command_scale;
	handlers[CMD_SKEWX] = command_skewx;
	handlers[CMD_SKEWY] = command_skewy;
	handlers[CMD_TRANSLATE] = command_translate;
	handlers[CMD_RECT] = command_rect;
	handlers[CMD_CIRCLE] = command_circle;
	handlers[CMD_ELLIPSE] = command_ellipse;
	handlers[CMD_ROUNDRECT] = command_roundrect;
	handlers[CMD_ARCTO] = command_arcto;
	handlers[CMD_QUADTO] = command_quadto;
	handlers[CMD_BEZIERTO] = command_bezierto;
	handlers[CMD_CLEARRECT] = command_clearrect;
	handlers[CMD_FILLRECT] = command_fillrect;
	handlers[CMD_STROKERECT] = command_strokerect;
	handlers[CMD_ARC] = command_arc;
	handlers[CMD_TRANSFORM] = command_transform;
	handlers[CMD_SETTRANSFORM] = command_settransform;
	handlers[CMD_RESETTRANSFORM] = command_resettransform;
	handlers[CMD_FILLTEXT3] = command_filltext3;
	handlers[CMD_STROKETEXT3] = command_stroketext3;
	handlers[CMD_DRAWIMAGE3] = command_drawimage3;
	handlers[CMD_DRAWIMAGE5] = command_drawimage5;
	handlers[CMD_DRAWIMAGE9] = command_drawimage9;
}

