var exec = require('cordova/exec');

function Canvas() {
	this.isNative = true;
	this.listeners = {};
}

Canvas.PointerEvent = function(e) {
	this.action    = e.action;
	this.eventTime = e.eventTime
	this.downTime  = e.downTime

	this.touches = e.touches;
	this.originalEvent = e;

	return;
}

Canvas.PointerEvent.ACTION_DOWN = 0;
Canvas.PointerEvent.ACTION_UP   = 1;
Canvas.PointerEvent.ACTION_MOVE = 2;

Canvas.PointerEvent.prototype.getPointerList = function() {
	return this.touches;
}

Canvas.PointerEvent.prototype.preventDefault = function() {
}

Canvas.prototype.dispatchTouchEvent = function(e) {
	var data = new Canvas.PointerEvent(e);

	switch(e.action) {
		case Canvas.PointerEvent.ACTION_DOWN: {
			this.dispatchEvent("pointerdown", data);
			break;
		}
		case Canvas.PointerEvent.ACTION_UP: {
			this.dispatchEvent("pointerup", data);
			break;
		}
		case Canvas.PointerEvent.ACTION_MOVE: {
			this.dispatchEvent("pointermove", data);
			break;
		}
		default:break;
	}

	return;
}

Canvas.prototype.addEventListener = function(type, listener, useCapture) {
	if(!type || !listener) return this;

	if(!this.listeners[type]) {
		this.listeners[type] = [];
	}

	var listeners = this.listeners[type];
	listeners.push(listener);

	console.log("Canvas.prototype.addEventListener:" + type);

	return this;
}


Canvas.prototype.dispatchEvent = function(type, data) {
	var listeners = this.listeners[type];
	if(!listeners) {
		console.log("Canvas.prototype.dispatchEvent: no listeners for:" + type);

		return;
	}

	for(var i = 0; i < listeners.length; i++) {
		var iter = listeners[i];
		if(!iter(data)) break;
	}

	return;
}

Canvas.create = function() {
	if(Canvas.instance) {
		return Canvas.instance;
	}

	var canvas = new Canvas();
	
	canvas.style = new Object();
	canvas.isFastCanvas = true;
	canvas.context = new Canvas.Context2d();
	canvas.context.canvas = canvas;

	canvas.width = Canvas.info.width;
	canvas.height = Canvas.info.height;

	Object.defineProperty(canvas.style, "width", {
		set: function (width) {
			console.log("ignore set canvas.style.width:" + width)
		},
		get: function() {
			return Canvas.info.width;
		}
	});
	
	Object.defineProperty(canvas.style, "height", {
		set: function (height) {
			console.log("ignore set canvas.style.height:" + height)
		},
		get: function() {
			return Canvas.info.height;
		}
	});

	var pathName = window.location.protocol + "//" +window.location.hostname + window.location.pathname;
	Canvas.root = pathName.replace(/\\/g,'/').replace(/\/[^\/]*$/, '');
	Canvas.instance = canvas;

	return canvas;
}

Canvas.getInfo = function() {
	function success(info) {
		Canvas.info = info;
		console.log("getInfo success:" + JSON.stringify(info));
	}

	function error(err) {
		console.log("getInfo fail:" + JSON.stringify(err));
	}

    exec(success, error, "RuntimePlugin", "getInfo", []);
};

Canvas.onSurfaceChanged = function(width, height) {
	Canvas.info.width = width;
	Canvas.info.height = height;

	if(window.onresize) {
		window.onresize(Canvas.info);
	}
	console.log("Canvas.onSurfaceChanged:" + width + "x" + height);

	return;
}

Canvas.setShowFPS = function(value) {
	Canvas.showFPS = value;

	function onDone(msg) {
		console.log("setShowFPS:" + msg);
	}
    exec(onDone, onDone, "RuntimePlugin", "setShowFPS", [value]);

    return;
};

Canvas.getInfo();

function defaultOnSuccess(msg) {
}

function defaultOnError(msg) {
}

Canvas.prototype.render = function(cmd, success, error) {
	Canvas.fps = DirectCall.setRenderCommands(cmd);
};

Canvas.absoluteURL = function(url) {
	if(url.indexOf("data:") === 0 || url.indexOf("://") > 0) {
		return url;
	}

	return Canvas.root + "/" + url;
}

Canvas.loadImage = function(url, success, error) {
	url = Canvas.absoluteURL(url);

	exec(success, error, "RuntimePlugin", "loadImage", [url]);
}

Canvas.unloadImage = function(textureID) {
	console.log("Canvas.unloadImage:" + textureID);
	function onDone(info) {
		console.log("Canvas.unloadImage done:" + textureID);
	}
	exec(onDone, onDone, "RuntimePlugin", "unloadImage", [textureID]);
}

//////////////////////////////////////////////////
Canvas.FontImage = function(text, font, color, width, height) {
	this.text = text;
	this.font = font;
	this.color = color;
	this.width = width;
	this.height = height;
	this.textureID = 0;
}

Canvas.getFontSizeInFont = function(str) {
	var fontSize = 12;
	var px = str.match(/\d+px/g);

	if(px) {
		fontSize = parseInt(px[0]);
	}
	else {
		var pt = str.match(/\d+pt/g);
		if(pt) {
			fontSize = parseInt(pt[0]) * 1.5;
		}
	}

	return fontSize;
}

Canvas.FontImage.caches = [];
Canvas.FontImage.create = function(text, font, color) {
	var canvas = Canvas.getTempCanvas();
	var ctx = canvas.getContext("2d");

	ctx.font = font;
	var width = ctx.measureText(text).width;
	var height = Canvas.getFontSizeInFont(font);

	canvas = Canvas.getTempCanvas(width, height);
	ctx = canvas.getContext("2d");
	
	ctx.clearRect(0, 0, width, height);
	ctx.font = font;
	ctx.fillStyle = color;
	ctx.textAlign = "left";
    ctx.textBaseline = "top";
    ctx.fillText(text, 0, 0);
	var src = canvas.toDataURL();

	var fontImage = new Canvas.FontImage(text, font, color, width, height);
	Canvas.loadImage(src, function(info) {
		fontImage.textureID = info.id;
		console.log("Canvas.loadFontImage success:" + JSON.stringify(info));
	}, function(info) {
		console.log("Canvas.loadImage fail:" + JSON.stringify(info));
	});

	Canvas.FontImage.caches.push(fontImage);

	return fontImage;
}

Canvas.FontImage.clearOldCache = function() {
	var caches = Canvas.FontImage.caches;
	var n = caches.length;

	if(n < 50) {
		return;
	}

	var now = Date.now();
	for(var i = 0; i < n; ) {
		var iter = caches[i];
		if((now - iter.recentUseTime) > 1000) {
			console.log("Canvas.FontImage.clearOldCache remove:" + iter.text);
			Canvas.unloadImage(iter.textureID);
			caches.splice(i, 1);
		}
		else {
			i++;
		}
	}

	return;
}

Canvas.FontImage.get = function(text, font, color) {
	var caches = Canvas.FontImage.caches;

	var n = caches.length;
	for(var i = 0; i < n; i++) {
		var iter = caches[i];
		if(iter.text === text && iter.font === font && iter.color === color) {
			iter.recentUseTime = Canvas.renderTime;
			return iter;
		}
	}

	return Canvas.FontImage.create(text, font, color);
}

//////////////////////////////////////////////////

Canvas.prototype.getScaleX = function() {
	return this.style.width/this.width;
}

Canvas.prototype.getScaleY = function() {
	return this.style.height/this.height;
}

Canvas.prototype.getContext = function(type) {
	Canvas.renderTime = Date.now();
	Canvas.FontImage.clearOldCache();

	this.context.commands = "";
	this.context.scale(this.getScaleX(), this.getScaleY())

	return this.context;
}

Canvas.prototype.flush = function() {
	var ctx = this.context;
	if(Canvas.fps) {
		ctx.font = "24px sans";
		ctx.fillStyle = "Black";
		ctx.textAlign = "left";
		ctx.textBaseline = "top";
		ctx.fillText(Canvas.fps.toString(), 10, 40);
	}

	this.render(this.context.commands, defaultOnSuccess, defaultOnError);
	this.context.commands = null;

	return this;
}

Canvas.Context2d = function() {
	this.commands = "";
	this.defineProperties();

	return;
}

Canvas.tempCanvas = null;
Canvas.getTempCanvas = function(width, height) {
	if(!Canvas.tempCanvas) {
		Canvas.tempCanvas = document.createElement("canvas");
		Canvas.tempCanvas.type = "backend_canvas";
	}

	if(width) {
		Canvas.tempCanvas.width = width;
	}

	if(height) {
		Canvas.tempCanvas.height = height;
	}

	return Canvas.tempCanvas;
}

Canvas.Context2d.prototype.measureText = function(text) {
	var canvas = Canvas.getTempCanvas(100, 100);
	var ctx = canvas.getContext("2d");
	
	ctx.font = this.font;

	return ctx.measureText(text);
}

Canvas.Context2d.prototype.drawImage = function(image, p1, p2, p3, p4, p5, p6, p7, p8) {
	if(!image || !image.textureID) {
//		console.log("drawImage invalid image");
		return this;
	}

	var len = arguments.length;
	var textureID = image.textureID;

	if(len === 3) {
		this.drawImage3(textureID, p1, p2);
	}
	else if(len === 5) {
		this.drawImage5(textureID, p1, p2, p3, p4);
	}
	else if(len === 9) {
		this.drawImage9(textureID, p1, p2, p3, p4, p5, p6, p7, p8);
	}
	else {
		console.log("invalid arguments for drawImage");
	}

	return this;
}

Canvas.Context2d.prototype.fillText = function(text, x, y) {
	if(!text || !text.length) {
		console.log("Canvas.Context2d.prototype.fillText invalid text");
		return;
	}

	var fontImage = Canvas.FontImage.get(text, this.font, this.fillStyle);

	if(!fontImage) {
		console.log("Canvas.Context2d.prototype.fillText not found.");
		return;
	}

	if(!fontImage.textureID) {
		console.log("Canvas.Context2d.prototype.fillText not ready.");
		return;
	}

	var tx = x;
	var ty = y;
	switch(this.textAlign) {
		case "right": {
			tx = x - fontImage.width;
			break;
		}
		case "center": {
			tx = x - (fontImage.width >> 1);
			break;
		}
		default:break;
	}

	switch(this.textBaseline) {
		case "bottom": {
			ty = y - fontImage.height;
			break;
		}
		case "middle": {
			ty = y - (fontImage.height>>1);
			break;
		}
		default:break;
	}

	return this.drawImage3(fontImage.textureID, tx, ty);
}

Canvas.Context2d.prototype.strokeText = function(text, x, y) {
	return this.strokeText3(btoa(text), x, y);
}

///////////////////////////////////////////////////////////////////////////////

function SoundEffect() {
	this.soundID = -1;
	this.volume = 1;

	return;
}

SoundEffect.audios = {};

SoundEffect.prototype.addEventListener = function(event, listener) {
	//TODO
}

SoundEffect.prototype.play = function() {
	if(this.soundID < 0) {
		console.log("invalid sound effect:" + this.url);
		return this;
	}

	exec(function(msg) {
		console.log("SoundEffect.prototype.play success:" + msg);
	}, function(msg) {
		console.log("SoundEffect.prototype.play fail:" + msg);
	}, "RuntimePlugin", "playSoundEffect", [this.soundID, this.volume]);

	return this;
}

SoundEffect.prototype.pause = function() {
	if(this.soundID < 0) {
		console.log("invalid sound effect:" + this.url);
		return this;
	}

	exec(function(msg) {
		console.log("SoundEffect.prototype.pause success:" + msg);
	}, function(msg) {
		console.log("SoundEffect.prototype.pause fail:" + msg);
	}, "RuntimePlugin", "pauseSoundEffect", [this.soundID]);

	return this;
}

SoundEffect.prototype.stop = function() {
	if(this.soundID < 0) {
		console.log("invalid sound effect:" + this.url);
		return this;
	}

	exec(function(msg) {
		console.log("SoundEffect.prototype.stop success:" + msg);
	}, function(msg) {
		console.log("SoundEffect.prototype.stop fail:" + msg);
	}, "RuntimePlugin", "stopSoundEffect", [this.soundID]);

	return this;
}

SoundEffect.prototype.unload = function() {
	if(this.soundID < 0) {
		console.log("invalid sound effect:" + this.url);
		return this;
	}

	exec(function(msg) {
		console.log("SoundEffect.prototype.unload success:" + msg);
	}, function(msg) {
		console.log("SoundEffect.prototype.unload fail:" + msg);
	}, "RuntimePlugin", "unloadSoundEffect", [this.soundID]);

	return this;
}

SoundEffect.prototype.setSrc = function(src) {
	if(this.src === src) {
		return this;
	}

	var me = this;
	this.src = src;

	exec(function(soundID) {
		me.soundID = soundID;
		if(me.onload) {
			me.onload(me);
		}
		console.log("SoundEffect.prototype.setSrc success:" + src + " soundID:" + soundID);
	}, function(msg) {
		if(me.onerror) {
			me.onerror();
		}
		console.log("SoundEffect.prototype.setSrc fail:" + src + " err:" + msg);
	}, "RuntimePlugin", "loadSoundEffect", [src]);
	return this;
}

///////////////////////////////////////////////////////////////////////////////
function SoundMusic() {
	this.status = "";
	this._loop = true;

	Object.defineProperty(this, "volume", {
		set: function (volume) {
			this._volume = volume;
			return this.setVolume(volume);
		},
		get: function() {
			return this._volume;
		}
	});

	Object.defineProperty(this, "loop", {
		set: function (loop) {
			this._loop = !!loop;
			return this.setLoop(this._loop);
		},
		get: function() {
			return this._loop;
		}
	});

	return;
}

SoundMusic.audios = {};

SoundMusic.prototype.addEventListener = function(event, listener) {
	//TODO
}

SoundMusic.prototype.setVolume = function(volume) {
	exec(function(msg) {
		console.log("SoundMusic.prototype.setVolume success:" + msg);
	}, function(msg) {
		console.log("SoundMusic.prototype.setVolume fail:" + msg);
	}, "RuntimePlugin", "setSoundMusicVolume", [volume]);

	return this;
}

SoundMusic.prototype.setLoop = function(loop) {
	exec(function(msg) {
		console.log("SoundMusic.prototype.setLoop success:" + msg);
	}, function(msg) {
		console.log("SoundMusic.prototype.setLoop fail:" + msg);
	}, "RuntimePlugin", "setSoundMusicLoop", [loop]);

	return this;
}

SoundMusic.prototype.play = function() {
	exec(function(msg) {
		console.log("SoundMusic.prototype.play success:" + msg);
	}, function(msg) {
		console.log("SoundMusic.prototype.play fail:" + msg);
	}, "RuntimePlugin", "playSoundMusic", [this.loop]);

	return this;
}

SoundMusic.prototype.pause = function() {
	exec(function(msg) {
		console.log("SoundMusic.prototype.pause success:" + msg);
	}, function(msg) {
		console.log("SoundMusic.prototype.pause fail:" + msg);
	}, "RuntimePlugin", "pauseSoundMusic", []);

	return this;
}

SoundMusic.prototype.stop = function() {
	exec(function(msg) {
		console.log("SoundMusic.prototype.stop success:" + msg);
	}, function(msg) {
		console.log("SoundMusic.prototype.stop fail:" + msg);
	}, "RuntimePlugin", "stopSoundMusic", [this.soundID]);

	return this;
}

SoundMusic.prototype.setSrc = function(src) {
	if(this.src === src) {
		return this;
	}

	var me = this;
	this.src = src;

	exec(function(soundID) {
		me.soundID = soundID;
		if(me.onload) {
			me.onload(me);
		}
		console.log("SoundMusic.prototype.setSrc success:" + src);
	}, function(msg) {
		if(me.onerror) {
			me.onerror();
		}
		console.log("SoundMusic.prototype.setSrc fail:" + src);
	}, "RuntimePlugin", "loadSoundMusic", [src]);

	return this;
}

///////////////////////////////////////////////////////////////////////////////

function TextEditor() {
	this.x = 0;
	this.y = 0;
	this.w = 100;
	this.h = 40;
	this.inputType = "text";
	this.singleLine = true;
	return;
}

TextEditor.prototype.setSingleLine = function(singleLine) {
	this.singleLine = singleLine;

	return this;
}

TextEditor.prototype.setInputType = function(inputType) {
	this.inputType = inputType;

	return this;
}

TextEditor.prototype.setFontSize = function(fontSize) {
	this.fontSize = fontSize;

	return this;
}

TextEditor.prototype.move = function(x, y) {
	this.x = x;
	this.y = y;

	return this;
}

TextEditor.prototype.resize = function(w, h) {
	this.w = w;
	this.h = h;

	return this;
}

TextEditor.prototype.show = function() {
	var x = this.x;
	var w = this.w;
	var y = this.y;
	var h = this.h;
	var singleLine = this.singleLine;
	var inputType = this.inputType ? this.inputType : "text";
	
    exec(function(info) {
    	console.log("showEditor done.");
    }, function(info) {
    	console.log("showEditor fail.");
    }, "RuntimePlugin", "showEditor", [inputType, x, y, w, h, singleLine]);
	
	TextEditor.textEdotr = this;

    return this;
}

TextEditor.prototype.hide = function() {
    exec(function(info) {
    	console.log("hideEditor done.");
    }, function(info) {
    	console.log("hideEditor fail.");
    }, "RuntimePlugin", "hideEditor", []);

    return;
}

TextEditor.prototype.setText = function(text) {
	this.text = text;

    exec(function(info) {
    	console.log("setEditorContent done.");
    }, function(info) {
    	console.log("setEditorContent fail.");
    }, "RuntimePlugin", "setEditorContent", [text]);

	return;
}

TextEditor.prototype.getText = function() {
	return this.text;
}

TextEditor.prototype.notifyChanged = function(text) {
	if(this.onChanged) {
		this.onChanged(text);
	}

	return this;
}

TextEditor.prototype.setOnChangedHandler = function(onChanged) {
	this.onChanged = onChanged;

	return;
}

TextEditor.prototype.setOnChangeHandler = function(onChange) {
	this.onChange = onChange;

	return;
}

TextEditor.createSingleLineTextEditor = function() {
	if(!TextEditor.singleLineTextEditor) {
		TextEditor.singleLineTextEditor = new TextEditor();
		TextEditor.singleLineTextEditor.setSingleLine(true);
	}

	return TextEditor.singleLineTextEditor;
}

TextEditor.createMultiLineTextEditor = function() {
	if(!TextEditor.multiLineTextEditor) {
		TextEditor.multiLineTextEditor = new TextEditor();
		TextEditor.multiLineTextEditor.setSingleLine(false);
	}

	return TextEditor.multiLineTextEditor;
}

///////////////////////////////////////////////////////////////////////////////

exports.getFPS = function () {
	return Canvas.fps;
}

exports.getViewPort = function () {
	return {width:Canvas.info.width, height:Canvas.info.height};
}

exports.setShowFPS = function (value) {
	return Canvas.setShowFPS(value);
}

exports.createCanvas = function () {
	return Canvas.create();
}

exports.createImage = function (src, onload, onerror) {
	var image = new Image(); 

	image.src = src;
	image.info = {};
	image.loadStartTime = Date.now();
	Canvas.loadImage(src, function(info) {
		image.info = info;
//		image.width = info.width;
//		image.height = info.height;
		image.textureID = info.id;

		if(onload) {
			onload(image);
		}
		
		info.loadCost = Date.now() - image.loadStartTime;
		if(src.indexOf("data:") >= 0) {
			info.url = null;
			console.log("Canvas.loadImage success:" + JSON.stringify(info));
		}
		else {
			console.log("Canvas.loadImage success:" + JSON.stringify(info));
		}
	}, function(info) {
		image.info = info;
		if(onerror) {
			onerror(null);
		}
		console.log("Canvas.loadImage fail:" + JSON.stringify(info));
	});

	image.unload = function() {
		if(this.textureID && this.textureID > 0) {
			Canvas.unloadImage(this.textureID);
			this.textureID = -1;
		}
	}

	return image;
}

exports.createSoundEffect = function(url, onload, onerror) {
	url = Canvas.absoluteURL(url);

	var soundEffect = SoundEffect.audios[url];
	if(soundEffect) {
		if(onload) {
			onload.call(soundEffect);
		}
		return soundEffect;
	}

	soundEffect = new SoundEffect();

	soundEffect.onload = onload;
	soundEffect.onerror = onerror;
	soundEffect.setSrc(url);

	SoundEffect.audios[url] = soundEffect;

	console.log("exports.createSoundEffect:" + url);

	return soundEffect;
}

exports.createSoundMusic = function(url, onload, onerror) {
	url = Canvas.absoluteURL(url);

	var soundMusic = SoundMusic.audios[url];
	if(soundMusic) {
		if(onload) {
			onload.call(soundMusic);
		}
		return soundEffect;
	}

	soundMusic = new SoundMusic();

	soundMusic.onload = onload;
	soundMusic.onerror = onerror;
	soundMusic.setSrc(url);

	SoundMusic.audios[url] = soundMusic;

	console.log("exports.createSoundMusic:" + url);

	return soundMusic;
}

exports.createSingleLineTextEditor = function() {
	return TextEditor.createSingleLineTextEditor();
}

exports.createMultiLineTextEditor = function() {
	return TextEditor.createMultiLineTextEditor();
}

////////////////////////////////////////////////////////////////////////////

exports.init = function() {
	console.log("hook canvastouchevent ");
	cordova.addWindowEventHandler('canvastouchevent');
	window.addEventListener('canvastouchevent', function(e) {
		if(Canvas.instance) {
			Canvas.instance.dispatchTouchEvent(e);
		}
	});

	console.log("hook soundmusicend");
	cordova.addWindowEventHandler('soundmusicend');
	window.addEventListener('soundmusicend', function(e) {
		var url = e.url;
		var audio = SoundMusic.audios[url];
		if(audio) {
			if(audio.onend) {
				audio.onend();
			}
		}
		console.log("soundmusicend:" + url);
	});

	cordova.addWindowEventHandler('editorchanged');
	window.addEventListener('editorchanged', function(e) {
		console.log("editorchanged:" + JSON.stringify(e));
		TextEditor.textEdotr.notifyChanged(e.text);
	}, false);

	function onPause() {
		console.log("onPause");
	}

	function onResume() {
		console.log("onResume");
	}

	document.addEventListener("pause", onPause, false);
	document.addEventListener("resume", onResume, false);

	cordova.addWindowEventHandler('surfacechanged');
	window.addEventListener('surfacechanged', function(e) {
		console.log("surfacechanged:" + JSON.stringify(e));
		Canvas.onSurfaceChanged(e.width, e.height);
	}, false);

	return;
}
Canvas.CMD_SET_LINEWIDTH = String.fromCharCode(65);
Canvas.CMD_SET_FILLSTYLE = String.fromCharCode(66);
Canvas.CMD_SET_STROKESTYLE = String.fromCharCode(67);
Canvas.CMD_SET_GLOBALALPHA = String.fromCharCode(68);
Canvas.CMD_SET_GLOBALCOMPOSITEOPERATION = String.fromCharCode(69);
Canvas.CMD_SET_TEXTALIGN = String.fromCharCode(70);
Canvas.CMD_SET_TEXTBASELINE = String.fromCharCode(71);
Canvas.CMD_SET_FONT = String.fromCharCode(72);
Canvas.CMD_SET_SHADOWOFFSETX = String.fromCharCode(73);
Canvas.CMD_SET_SHADOWOFFSETY = String.fromCharCode(74);
Canvas.CMD_SET_SHADOWBLUR = String.fromCharCode(75);
Canvas.CMD_SET_SHADOWCOLOR = String.fromCharCode(76);
Canvas.CMD_SET_LINECAP = String.fromCharCode(77);
Canvas.CMD_SET_LINEJOIN = String.fromCharCode(78);
Canvas.CMD_SET_MITERLIMIT = String.fromCharCode(79);
Canvas.CMD_SAVE = String.fromCharCode(80);
Canvas.CMD_RESTORE = String.fromCharCode(81);
Canvas.CMD_CLIP = String.fromCharCode(82);
Canvas.CMD_RESETCLIP = String.fromCharCode(83);
Canvas.CMD_FILL = String.fromCharCode(84);
Canvas.CMD_STROKE = String.fromCharCode(85);
Canvas.CMD_BEGINPATH = String.fromCharCode(86);
Canvas.CMD_CLOSEPATH = String.fromCharCode(87);
Canvas.CMD_MOVETO = String.fromCharCode(88);
Canvas.CMD_LINETO = String.fromCharCode(89);
Canvas.CMD_ROTATE = String.fromCharCode(90);
Canvas.CMD_SCALE = String.fromCharCode(91);
Canvas.CMD_SKEWX = String.fromCharCode(92);
Canvas.CMD_SKEWY = String.fromCharCode(93);
Canvas.CMD_TRANSLATE = String.fromCharCode(94);
Canvas.CMD_RECT = String.fromCharCode(95);
Canvas.CMD_CIRCLE = String.fromCharCode(96);
Canvas.CMD_ELLIPSE = String.fromCharCode(97);
Canvas.CMD_ROUNDRECT = String.fromCharCode(98);
Canvas.CMD_ARCTO = String.fromCharCode(99);
Canvas.CMD_QUADTO = String.fromCharCode(100);
Canvas.CMD_BEZIERTO = String.fromCharCode(101);
Canvas.CMD_CLEARRECT = String.fromCharCode(102);
Canvas.CMD_FILLRECT = String.fromCharCode(103);
Canvas.CMD_STROKERECT = String.fromCharCode(104);
Canvas.CMD_ARC = String.fromCharCode(105);
Canvas.CMD_TRANSFORM = String.fromCharCode(106);
Canvas.CMD_SETTRANSFORM = String.fromCharCode(107);
Canvas.CMD_RESETTRANSFORM = String.fromCharCode(108);
Canvas.CMD_FILLTEXT3 = String.fromCharCode(109);
Canvas.CMD_STROKETEXT3 = String.fromCharCode(110);
Canvas.CMD_DRAWIMAGE3 = String.fromCharCode(111);
Canvas.CMD_DRAWIMAGE5 = String.fromCharCode(112);
Canvas.CMD_DRAWIMAGE9 = String.fromCharCode(113);

Canvas.Context2d.prototype.defineProperties = function() {
	var ctx = this;

	Object.defineProperty(ctx, "lineWidth", {
		set: function (lineWidth) {
			var cmd = Canvas.CMD_SET_LINEWIDTH+"("+lineWidth+");";
			this._lineWidth = lineWidth;
			this.commands += cmd;
			return this;

		},
		get: function () {
			return this._lineWidth;
		},
		enumerable: true,
		configurable: true
	});
	Object.defineProperty(ctx, "fillStyle", {
		set: function (fillStyle) {
			var cmd = Canvas.CMD_SET_FILLSTYLE+"("+fillStyle+");";
			this._fillStyle = fillStyle;
			this.commands += cmd;
			return this;

		},
		get: function () {
			return this._fillStyle;
		},
		enumerable: true,
		configurable: true
	});
	Object.defineProperty(ctx, "strokeStyle", {
		set: function (strokeStyle) {
			var cmd = Canvas.CMD_SET_STROKESTYLE+"("+strokeStyle+");";
			this._strokeStyle = strokeStyle;
			this.commands += cmd;
			return this;

		},
		get: function () {
			return this._strokeStyle;
		},
		enumerable: true,
		configurable: true
	});
	Object.defineProperty(ctx, "globalAlpha", {
		set: function (globalAlpha) {
			var cmd = Canvas.CMD_SET_GLOBALALPHA+"("+globalAlpha+");";
			this._globalAlpha = globalAlpha;
			this.commands += cmd;
			return this;

		},
		get: function () {
			return this._globalAlpha;
		},
		enumerable: true,
		configurable: true
	});
	Object.defineProperty(ctx, "globalCompositeOperation", {
		set: function (globalCompositeOperation) {
			var cmd = Canvas.CMD_SET_GLOBALCOMPOSITEOPERATION+"("+globalCompositeOperation+");";
			this._globalCompositeOperation = globalCompositeOperation;
			this.commands += cmd;
			return this;

		},
		get: function () {
			return this._globalCompositeOperation;
		},
		enumerable: true,
		configurable: true
	});
	Object.defineProperty(ctx, "textAlign", {
		set: function (textAlign) {
			var cmd = Canvas.CMD_SET_TEXTALIGN+"("+textAlign+");";
			this._textAlign = textAlign;
			this.commands += cmd;
			return this;

		},
		get: function () {
			return this._textAlign;
		},
		enumerable: true,
		configurable: true
	});
	Object.defineProperty(ctx, "textBaseline", {
		set: function (textBaseline) {
			var cmd = Canvas.CMD_SET_TEXTBASELINE+"("+textBaseline+");";
			this._textBaseline = textBaseline;
			this.commands += cmd;
			return this;

		},
		get: function () {
			return this._textBaseline;
		},
		enumerable: true,
		configurable: true
	});
	Object.defineProperty(ctx, "font", {
		set: function (font) {
			var cmd = Canvas.CMD_SET_FONT+"("+font+");";
			this._font = font;
			this.commands += cmd;
			return this;

		},
		get: function () {
			return this._font;
		},
		enumerable: true,
		configurable: true
	});
	Object.defineProperty(ctx, "shadowOffsetX", {
		set: function (shadowOffsetX) {
			var cmd = Canvas.CMD_SET_SHADOWOFFSETX+"("+shadowOffsetX+");";
			this._shadowOffsetX = shadowOffsetX;
			this.commands += cmd;
			return this;

		},
		get: function () {
			return this._shadowOffsetX;
		},
		enumerable: true,
		configurable: true
	});
	Object.defineProperty(ctx, "shadowOffsetY", {
		set: function (shadowOffsetY) {
			var cmd = Canvas.CMD_SET_SHADOWOFFSETY+"("+shadowOffsetY+");";
			this._shadowOffsetY = shadowOffsetY;
			this.commands += cmd;
			return this;

		},
		get: function () {
			return this._shadowOffsetY;
		},
		enumerable: true,
		configurable: true
	});
	Object.defineProperty(ctx, "shadowBlur", {
		set: function (shadowBlur) {
			var cmd = Canvas.CMD_SET_SHADOWBLUR+"("+shadowBlur+");";
			this._shadowBlur = shadowBlur;
			this.commands += cmd;
			return this;

		},
		get: function () {
			return this._shadowBlur;
		},
		enumerable: true,
		configurable: true
	});
	Object.defineProperty(ctx, "shadowColor", {
		set: function (shadowColor) {
			var cmd = Canvas.CMD_SET_SHADOWCOLOR+"("+shadowColor+");";
			this._shadowColor = shadowColor;
			this.commands += cmd;
			return this;

		},
		get: function () {
			return this._shadowColor;
		},
		enumerable: true,
		configurable: true
	});
	Object.defineProperty(ctx, "lineCap", {
		set: function (lineCap) {
			var cmd = Canvas.CMD_SET_LINECAP+"("+lineCap+");";
			this._lineCap = lineCap;
			this.commands += cmd;
			return this;

		},
		get: function () {
			return this._lineCap;
		},
		enumerable: true,
		configurable: true
	});
	Object.defineProperty(ctx, "lineJoin", {
		set: function (lineJoin) {
			var cmd = Canvas.CMD_SET_LINEJOIN+"("+lineJoin+");";
			this._lineJoin = lineJoin;
			this.commands += cmd;
			return this;

		},
		get: function () {
			return this._lineJoin;
		},
		enumerable: true,
		configurable: true
	});
	Object.defineProperty(ctx, "miterLimit", {
		set: function (miterLimit) {
			var cmd = Canvas.CMD_SET_MITERLIMIT+"("+miterLimit+");";
			this._miterLimit = miterLimit;
			this.commands += cmd;
			return this;

		},
		get: function () {
			return this._miterLimit;
		},
		enumerable: true,
		configurable: true
	});
}

Canvas.Context2d.prototype.save = function() {
	var cmd = Canvas.CMD_SAVE+"();"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.restore = function() {
	var cmd = Canvas.CMD_RESTORE+"();"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.clip = function() {
	var cmd = Canvas.CMD_CLIP+"();"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.resetClip = function() {
	var cmd = Canvas.CMD_RESETCLIP+"();"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.fill = function() {
	var cmd = Canvas.CMD_FILL+"();"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.stroke = function() {
	var cmd = Canvas.CMD_STROKE+"();"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.beginPath = function() {
	var cmd = Canvas.CMD_BEGINPATH+"();"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.closePath = function() {
	var cmd = Canvas.CMD_CLOSEPATH+"();"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.moveTo = function(x, y) {
	var cmd = Canvas.CMD_MOVETO+"("+x+","+y+");"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.lineTo = function(x, y) {
	var cmd = Canvas.CMD_LINETO+"("+x+","+y+");"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.rotate = function(angle) {
	var cmd = Canvas.CMD_ROTATE+"("+angle+");"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.scale = function(x, y) {
	var cmd = Canvas.CMD_SCALE+"("+x+","+y+");"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.skewX = function(angle) {
	var cmd = Canvas.CMD_SKEWX+"("+angle+");"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.skewY = function(angle) {
	var cmd = Canvas.CMD_SKEWY+"("+angle+");"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.translate = function(x, y) {
	var cmd = Canvas.CMD_TRANSLATE+"("+x+","+y+");"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.rect = function(x, y, w, h) {
	var cmd = Canvas.CMD_RECT+"("+x+","+y+","+w+","+h+");"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.circle = function(x, y, r) {
	var cmd = Canvas.CMD_CIRCLE+"("+x+","+y+","+r+");"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.ellipse = function(x, y, rx, ry) {
	var cmd = Canvas.CMD_ELLIPSE+"("+x+","+y+","+rx+","+ry+");"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.roundRect = function(x, y, w, h, r) {
	var cmd = Canvas.CMD_ROUNDRECT+"("+x+","+y+","+w+","+h+","+r+");"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.arcTo = function(x1, y1, x2, y2, r) {
	var cmd = Canvas.CMD_ARCTO+"("+x1+","+y1+","+x2+","+y2+","+r+");"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.quadTo = function(cx, cy, x, y) {
	var cmd = Canvas.CMD_QUADTO+"("+cx+","+cy+","+x+","+y+");"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.bezierTo = function(cx1, cy1, cx2, cy2, x, y) {
	var cmd = Canvas.CMD_BEZIERTO+"("+cx1+","+cy1+","+cx2+","+cy2+","+x+","+y+");"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.clearRect = function(x, y, w, h) {
	var cmd = Canvas.CMD_CLEARRECT+"("+x+","+y+","+w+","+h+");"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.fillRect = function(x, y, w, h) {
	var cmd = Canvas.CMD_FILLRECT+"("+x+","+y+","+w+","+h+");"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.strokeRect = function(x, y, w, h) {
	var cmd = Canvas.CMD_STROKERECT+"("+x+","+y+","+w+","+h+");"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.arc = function(x, y, r, start, end, clockwide) {
	var cmd = Canvas.CMD_ARC+"("+x+","+y+","+r+","+start+","+end+","+clockwide+");"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.transform = function(a, b, c, d, e, f) {
	var cmd = Canvas.CMD_TRANSFORM+"("+a+","+b+","+c+","+d+","+e+","+f+");"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.setTransform = function(a, b, c, d, e, f) {
	var cmd = Canvas.CMD_SETTRANSFORM+"("+a+","+b+","+c+","+d+","+e+","+f+");"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.resetTransform = function() {
	var cmd = Canvas.CMD_RESETTRANSFORM+"();"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.fillText3 = function(text, x, y) {
	var cmd = Canvas.CMD_FILLTEXT3+"("+text+","+x+","+y+");"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.strokeText3 = function(text, x, y) {
	var cmd = Canvas.CMD_STROKETEXT3+"("+text+","+x+","+y+");"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.drawImage3 = function(image, dx, dy) {
	var cmd = Canvas.CMD_DRAWIMAGE3+"("+image+","+dx+","+dy+");"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.drawImage5 = function(image, dx, dy, dw, dh) {
	var cmd = Canvas.CMD_DRAWIMAGE5+"("+image+","+dx+","+dy+","+dw+","+dh+");"
	this.commands += cmd;

	return this;
}

Canvas.Context2d.prototype.drawImage9 = function(image, sx, sy, sw, sh, dx, dy, dw, dh) {
	var cmd = Canvas.CMD_DRAWIMAGE9+"("+image+","+sx+","+sy+","+sw+","+sh+","+dx+","+dy+","+dw+","+dh+");"
	this.commands += cmd;

	return this;
}


