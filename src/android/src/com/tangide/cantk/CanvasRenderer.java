/*
 * File     : CanvasRenderer.java
 * Brief    : CanvasRenderer 
 * Author   : Li XianJing <xianjimli@hotmail.com>
 * Web Site : http://www.tangide.com/
 *
 * Copyright (c) 2015 - 2015 Tangram HD.
 *
**/

package com.tangide.cantk;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.os.Process; 
import android.util.Log;
import android.opengl.GLES10;
import android.opengl.GLUtils;
import android.opengl.GLSurfaceView;

class CanvasRenderer implements GLSurfaceView.Renderer {
	private int mDensity = 0;
	private long mLastRenderTime;	

    private static final String LOGTAG = "com.tangide.cantk.CanvasRenderer";

	public CanvasRenderer(int density) {
		super();
		mDensity = density;
	}

	private long getTime() {
		return System.nanoTime()/1000000;
	}

	public void onDrawFrame(GL10 gl) {
		if(true) {
			ActionInfoQueue.processActions();
			CanvasJNI.render();
		}
		else {
			long now = System.nanoTime();
			long interval = (now - mLastRenderTime)/1000000;
			long sleepTime = 12  - interval;
			
			ActionInfoQueue.processActions();

			if(sleepTime > 0) {
				try {
					Thread.sleep(sleepTime);
				} catch (final Exception e) {}
				Log.v(LOGTAG, "Render thread sleep:" + sleepTime);
			}

			CanvasJNI.render();
			mLastRenderTime = System.nanoTime();
		}
	}

	public void onSurfaceChanged(GL10 gl, int width, int height) {
		CanvasJNI.surfaceChanged(width, height, mDensity);
		StringBuilder msg = new StringBuilder();
		msg.append("{\n");
		msg.append("\t\"width\":").append(width).append(",\n");
		msg.append("\t\"height\":").append(height).append(",\n");
		msg.append("}\n");

		String js = "cordova.fireWindowEvent('surfacechanged', "+msg.toString()+");";
		RuntimePlugin.execJs(js);
	}

	public void onSurfaceCreated(GL10 gl, EGLConfig config) {
		Process.setThreadPriority(Process.THREAD_PRIORITY_LOWEST);
		//Process.setThreadPriority(Process.THREAD_PRIORITY_DEFAULT);
		CanvasJNI.surfaceCreated();
		mLastRenderTime = System.nanoTime();
	}
}
