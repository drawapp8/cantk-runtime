/*
 * File     : CanvasView.java
 * Brief    : CanvasView 
 * Author   : Li XianJing <xianjimli@hotmail.com>
 * Web Site : http://www.tangide.com/
 *
 * Copyright (c) 2015 - 2015 Tangram HD.
 *
**/

package com.tangide.cantk;

import android.content.Context;
import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.util.DisplayMetrics;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.opengles.GL10;

class CanvasView extends GLSurfaceView {
    private static final boolean DEBUG = false;
    private static String LOGTAG = "com.tangide.canvas";

    public CanvasView(Context context) {
        super(context);
        
        this.init();
    }

	public CanvasView(Context context, boolean translucent, int depth, int stencil) {
		super(context);
		this.init();
	}

	@Override
	public boolean onKeyDown (int keyCode, KeyEvent event) {
		synchronized (this) {
			if (RuntimePlugin.dispatchKeyDown(keyCode, event)) {
				return true;
			} else {
				return super.onKeyDown(keyCode, event);
			}
		}
	}

	@Override
	public boolean onKeyUp (int keyCode, KeyEvent event) {
		synchronized (this) {
			if (RuntimePlugin.dispatchKeyUp(keyCode, event)) {
				return true;
			} else {
				return super.onKeyDown(keyCode, event);
			}
		}
	}

	@Override
	public boolean onTouchEvent (MotionEvent event) {
		synchronized (this) {
			if (!RuntimePlugin.dispatchTouchEvent(event)) {
				// tear down.
			}
		}
		return true;
	}

//////////////////////////////////////////////////////////////
    private void init() {
        // Pick an EGLConfig with RGB8 color, 16-bit depth, no stencil,
        // supporting OpenGL ES 2.0 or later backwards-compatible versions.
		setEGLConfigChooser(8, 8, 8, 0, 16, 0);
        setEGLContextClientVersion(2);
	
		setRenderer(new CanvasRenderer(RuntimePlugin.displayMetrics.densityDpi));
//		setRenderMode(RENDERMODE_WHEN_DIRTY);
		setRenderMode(RENDERMODE_CONTINUOUSLY);

        this.setFocusableInTouchMode(true);
        this.requestFocus();
    }
}
