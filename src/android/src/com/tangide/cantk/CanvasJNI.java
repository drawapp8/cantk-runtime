/*
 * File     : CanvasJNI.java
 * Brief    : canvas jni 
 * Author   : Li XianJing <xianjimli@hotmail.com>
 * Web Site : http://www.tangide.com/
 *
 * Copyright (c) 2015 - 2015 Tangram HD inc.
 *
**/

package com.tangide.cantk;

import android.graphics.Bitmap;

public class CanvasJNI {
    public static native void render();
	public static native void setShowFPS(int showFPS);
	public static native void deleteImage(int imageID);
	public static native void surfaceCreated();
	public static native void surfaceChanged(int width, int height, int dpi);
    public static native void loadImage(String url, Bitmap bitmap, ImageInfo size);
    public static native int setRenderCommands(String renderCommands);
	
	static {
		System.loadLibrary("CanvasJNI");
	}
}
