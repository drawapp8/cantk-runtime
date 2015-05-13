/*
 * File     : color_parser.c
 * Brief    : color parser
 * Author   : Li XianJing <xianjimli@hotmail.com>
 * Web Site : http://www.tangide.com/
 *
 * Copyright (c) 2015 - 2015 Tangram HD.
 *
**/

#include "color_parser.h"

static const char* colorMap[] = {"white","#ffffff",
	"azure","#f0ffff",
	"mintcream","#f5fffa",
	"snow","#fffafa",
	"ivory","#fffff0",
	"ghostwhite","#f8f8ff",
	"floralwhite","#fffaf0",
	"aliceblue","#f0f8ff",
	"lightcyan","#e0ffff",
	"honeydew","#f0fff0",
	"lightyellow","#ffffe0",
	"seashell","#fff5ee",
	"lavenderblush","#fff0f5",
	"whitesmoke","#f5f5f5",
	"oldlace","#fdf5e6",
	"cornsilk","#fff8dc",
	"linen","#faf0e6",
	"lightgoldenrodyellow","#fafad2",
	"lemonchiffon","#fffacd",
	"beige","#f5f5dc",
	"lavender","#e6e6fa",
	"papayawhip","#ffefd5",
	"mistyrose","#ffe4e1",
	"antiquewhite","#faebd7",
	"blanchedalmond","#ffebcd",
	"bisque","#ffe4c4",
	"paleturquoise","#afeeee",
	"moccasin","#ffe4b5",
	"gainsboro","#dcdcdc",
	"peachpuff","#ffdab9",
	"navajowhite","#ffdead",
	"palegoldenrod","#eee8aa",
	"wheat","#f5deb3",
	"powderblue","#b0e0e6",
	"aquamarine","#7fffd4",
	"lightgrey","#d3d3d3",
	"pink","#ffc0cb",
	"lightblue","#add8e6",
	"thistle","#d8bfd8",
	"lightpink","#ffb6c1",
	"lightskyblue","#87cefa",
	"palegreen","#98fb98",
	"lightsteelblue","#b0c4de",
	"khaki","#f0d58c",
	"skyblue","#87ceeb",
	"aqua","#00ffff",
	"cyan","#00ffff",
	"silver","#c0c0c0",
	"plum","#dda0dd",
	"gray","#bebebe",
	"lightgreen","#90ee90",
	"violet","#ee82ee",
	"yellow","#ffff00",
	"turquoise","#40e0d0",
	"burlywood","#deb887",
	"greenyellow","#adff2f",
	"tan","#d2b48c",
	"mediumturquoise","#48d1cc",
	"lightsalmon","#ffa07a",
	"mediumaquamarine","#66cdaa",
	"darkgray","#a9a9a9",
	"orchid","#da70d6",
	"darkseagreen","#8fbc8f",
	"deepskyblue","#00bfff",
	"sandybrown","#f4a460",
	"gold","#ffd700",
	"mediumspringgreen","#00fa9a",
	"darkkhaki","#bdb76b",
	"cornflowerblue","#6495ed",
	"hotpink","#ff69b4",
	"darksalmon","#e9967a",
	"darkturquoise","#00ced1",
	"springgreen","#00ff7f",
	"lightcoral","#f08080",
	"rosybrown","#bc8f8f",
	"salmon","#fa8072",
	"chartreuse","#7fff00",
	"mediumpurple","#9370db",
	"lawngreen","#7cfc00",
	"dodgerblue","#1e90ff",
	"yellowgreen","#9acd32",
	"palevioletred","#db7093",
	"mediumslateblue","#7b68ee",
	"mediumorchid","#ba55d3",
	"coral","#ff7f50",
	"cadetblue","#5f9ea0",
	"lightseagreen","#20b2aa",
	"goldenrod","#daa520",
	"orange","#ffa500",
	"lightslategray","#778899",
	"fuchsia","#ff00ff",
	"magenta","#ff00ff",
	"mediumseagreen","#3cb371",
	"peru","#cd853f",
	"steelblue","#4682b4",
	"royalblue","#4169e1",
	"slategray","#708090",
	"tomato","#ff6347",
	"darkorange","#ff8c00",
	"slateblue","#6a5acd",
	"limegreen","#32cd32",
	"lime","#00ff00",
	"indianred","#cd5c5c",
	"darkorchid","#9932cc",
	"blueviolet","#8a2be2",
	"deeppink","#ff1493",
	"darkgoldenrod","#b8860b",
	"chocolate","#d2691e",
	"darkcyan","#008b8b",
	"dimgray","#696969",
	"olivedrab","#6b8e23",
	"seagreen","#2e8b57",
	"teal","#008080",
	"darkviolet","#9400d3",
	"mediumvioletred","#c71585",
	"orangered","#ff4500",
	"olive","#808000",
	"sienna","#a0522d",
	"darkslateblue","#483d8b",
	"darkolivegreen","#556b2f",
	"forestgreen","#228b22",
	"crimson","#dc143c",
	"blue","#0000ff",
	"darkmagenta","#8b008b",
	"darkslategray","#2f4f4f",
	"saddlebrown","#8b4513",
	"brown","#a52a2a",
	"firebrick","#b22222",
	"purple","#800080",
	"green","#008000",
	"red","#ff0000",
	"mediumblue","#0000cd",
	"indigo","#4b0082",
	"midnightblue","#191970",
	"darkgreen","#006400",
	"darkblue","#00008b",
	"navy","#000080",
	"darkred","#8b0000",
	"maroon","#800000",
	"black","#000000", NULL
};

static const char* normalizeColor(const char* str) {
	int i = 0;
	if(!str) {
		return "#00000000";;
	}
	
	if(str[0] == '#') {
		return str;
	}

	while(colorMap[i]) {
		if(strcasecmp(colorMap[i], str) == 0) {
			return colorMap[i+1];
		}
		i++;
	}

	return "#00000000";
}

static int charToHex(char c) {
	if(c >= '0' && c <= '9') {
		return c - '0';
	}
	if(c >= 'a' && c <= 'f') {
		return c - 'a' + 10;
	}
	if(c >= 'A' && c <= 'F') {
		return c - 'A' + 10;
	}

	return 0;
}

static int parseColorChannel(const char* str) {
	int value = 0;
	char first = '0';
	char second = '0';
	
	if(str[0]) {
		first = str[0];
	}

	if(str[1]) {
		second = str[1];
	}

	value = charToHex(first) * 16 + charToHex(second);

	return value;
}

ColorRGBA parseColor(const char* str) {
	int r = 0;
	int g = 0;
	int b = 0;
	int a = 255;
	ColorRGBA color;
	const char* p = normalizeColor(str);

	p++;

	if(p[0] && p [1]) {
		r = parseColorChannel(p);
		p += 2;
	}
	
	if(p[0] && p [1]) {
		g = parseColorChannel(p);
		p += 2;
	}
	
	if(p[0] && p [1]) {
		b = parseColorChannel(p);
		p += 2;
	}
	
	if(p[0] && p [1]) {
		a = parseColorChannel(p);
		p += 2;
	}

	color.r = r;
	color.g = g;
	color.b = b;
	color.a = a;

	return color;
}

#ifdef TEST_COLOR
#include <stdio.h>

int main(int argc, char* argv[]) {
	const char* str = argc > 1 ? argv[1] : "White";
	ColorRGBA color = parseColor(str);

	printf("%d %d %d %d\n", color.r, color.g, color.b, color.a);
	return 0;
}
#endif
