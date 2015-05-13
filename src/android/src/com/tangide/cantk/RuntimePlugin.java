/*
 * File     : RuntimePlugin.java
 * Brief    : RuntimePlugin
 * Author   : Li XianJing <xianjimli@hotmail.com>
 * Web Site : http://www.tangide.com/
 *
 * Copyright (c) 2015 - 2015 Tangram HD.
 *
**/

package com.tangide.cantk;

import android.os.Process; 
import android.util.Log;
import org.json.JSONArray;
import org.json.JSONObject;
import org.json.JSONException;
import org.apache.cordova.CordovaWebView;
import org.apache.cordova.CordovaInterface;
import org.apache.cordova.CordovaPlugin;
import org.apache.cordova.CallbackContext;
import android.util.DisplayMetrics;
import android.content.Context;

import android.app.Activity;
import android.util.Log;
import android.view.View;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.widget.FrameLayout;    
import android.widget.RelativeLayout;
import android.widget.AbsoluteLayout;
import android.webkit.JavascriptInterface;

public class RuntimePlugin extends CordovaPlugin {
	private static RuntimePlugin thePlugin = null;
	public static DisplayMetrics displayMetrics = new DisplayMetrics();
    private static final String LOGTAG = "com.tangide.cantk.RuntimePlugin";

	private Activity mActivity;
	private CanvasView mCanvasView;
	private TextEditor mTextEditor;
	private CordovaWebView mCordovaView;
	private SoundEffectPlayer mSoundEffectPlayer;
	private SoundMusicPlayer mSoundMusicPlayer;

    @Override
    public void initialize(CordovaInterface cordova, CordovaWebView webView) {
        super.initialize(cordova, webView);

		mActivity = cordova.getActivity();
		mCanvasView = new CanvasView(mActivity);
		mTextEditor = new TextEditor(mActivity);
		mSoundMusicPlayer = new SoundMusicPlayer();
		mSoundEffectPlayer = new SoundEffectPlayer();

		thePlugin = this;
		mCordovaView = webView;
		DirectCall dr = new DirectCall();
		webView.addJavascriptInterface(dr, "DirectCall");
		
		final RuntimePlugin me = this;
		mActivity.runOnUiThread(new Runnable() {
			@Override
			public void run() {	me.doInitView();}
		}); 

        Log.i(LOGTAG, "RuntimePlugin initialize done");
    }

	@Override
    public void onDestroy() {
		mActivity = null;
		mCordovaView = null;
		mCanvasView = null;
		mTextEditor = null;
		thePlugin = null;
		mSoundMusicPlayer.stop();
		mSoundMusicPlayer = null;
		mSoundEffectPlayer = null;
		
		Log.i(LOGTAG, "RuntimePlugin onDestroy");
    }
	
	public void doInitView() {
		Log.i(LOGTAG, "RuntimePlugin initView");
		final RelativeLayout top = new RelativeLayout(mActivity);
		top.setLayoutParams(new RelativeLayout.LayoutParams(
				RelativeLayout.LayoutParams.MATCH_PARENT,
				RelativeLayout.LayoutParams.MATCH_PARENT));
		top.addView(mCanvasView);

		mActivity.setContentView(top);

		AbsoluteLayout.LayoutParams params = new AbsoluteLayout.LayoutParams(200, 80, 300, 300);
		mActivity.addContentView(mTextEditor, params); 	
		mTextEditor.setVisibility(View.INVISIBLE);

		mActivity.getWindowManager().getDefaultDisplay().getMetrics(RuntimePlugin.displayMetrics);
		Process.setThreadPriority(Process.THREAD_PRIORITY_DISPLAY);
	}

    public boolean dispatch(String action, JSONArray args, CallbackContext callbackContext) throws JSONException {
		if (action.equals("render")) {
			return this.setRenderCommands(callbackContext, args.getString(0));
		} else if (action.equals("setShowFPS")) {
			return this.setShowFPS(callbackContext, args.getBoolean(0));
		} else if (action.equals("getInfo")) {
			return this.getInfo(callbackContext);
		} else {
			if(mSoundEffectPlayer.dispatch(action, args, callbackContext)) {
				return true;
			}
			else if(mSoundMusicPlayer.dispatch(action, args, callbackContext)) {
				return true;
			}
			else if(mTextEditor.dispatch(action, args, callbackContext)) {
				return true;
			}
			else if(ImageLoader.dispatch(action, args, callbackContext)) {
				return true;
			}
			else {
				callbackContext.error("invalid action:" + action);
				Log.i(LOGTAG, "invalid action:" + action );
			}
    	}

		return false;
    }
    
	private long getTime() {
		return System.nanoTime()/1000000;
	}
	
	private boolean setRenderCommands(CallbackContext callbackContext, String commands) {
		int fps = CanvasJNI.setRenderCommands(commands);
        callbackContext.success(fps);

        return true;
	}
	
	private boolean setShowFPS(CallbackContext callbackContext, boolean showFPS) {
		CanvasJNI.setShowFPS(showFPS?1:0);
		callbackContext.success("done");

		return true;
	}

	private boolean getInfo(CallbackContext callbackContext) {
		DisplayMetrics dm = RuntimePlugin.displayMetrics;
        int width   =     dm.widthPixels;
        int height  =     dm.heightPixels;
        int density =     dm.densityDpi;

		JSONObject info = new JSONObject();
		try {
			info.put("width",   width);
			info.put("height",  height);
			info.put("density", density);
		} catch (JSONException e) {
			callbackContext.error("Unexpected JSONException.");
			e.printStackTrace();
		}
        callbackContext.success(info);

        return true;
    }

    @Override
    public boolean execute(String action, JSONArray args, CallbackContext callbackContext) throws JSONException {
    	boolean ret = false;
		long start = this.getTime();
		try {
			ret = this.dispatch(action, args, callbackContext);	
		}catch(Exception e){
			Log.e(LOGTAG, "Unexpected error " + action, e);
		}
//		Log.i(LOGTAG, action + " cost:" + (this.getTime() - start));

        return ret;
    }
	
	public static String touchEventToString (MotionEvent event) {
		int action = event.getAction();
		StringBuilder msg = new StringBuilder();
		msg.append("{\n");
		msg.append("\t\"action\":").append(action).append(",\n");
		msg.append("\t\"eventTime\":").append(event.getEventTime()).append(",\n");
		msg.append("\t\"downTime\":").append(event.getDownTime()).append(",\n");
		msg.append("\t\"touches\":[");
		
		final int pointerCount = event.getPointerCount();
		for (int i = 0; i < pointerCount; i++) {
			float x = event.getX(i);
			float y = event.getY(i);
			if(i > 0) {
				msg.append(",");
			}
			msg.append("{");
			msg.append("\"x\":").append(x).append(",");
			msg.append("\"y\":").append(y).append(",");
			msg.append("\"pageX\":").append(x).append(",");
			msg.append("\"pageY\":").append(y).append("");
			msg.append("}");
		}
		msg.append("]\n");
		msg.append("}\n");

		return msg.toString();
	}

	public static boolean execJs(String js) {
		thePlugin.mCordovaView.sendJavascript(js);

		return true;
	}

	public static boolean dispatchTouchEvent (MotionEvent event) {
		if ( thePlugin == null || thePlugin.mCanvasView == null ) {
			return false;				
		}
		else {
			thePlugin.mTextEditor.setVisibility(View.INVISIBLE);
			thePlugin.mCanvasView.requestFocus();
			String eventStr = RuntimePlugin.touchEventToString(event);
			String js = "cordova.fireWindowEvent('canvastouchevent', "+eventStr+");";
			RuntimePlugin.execJs(js);
		}

		return true;
	}
	
	public static boolean dispatchKeyDown (int keyCode, KeyEvent event) {
		if ( thePlugin == null || thePlugin.mCordovaView == null ) {
			return false;				
		}
		else {
			return thePlugin.mCordovaView.onKeyDown(keyCode, event);
		}
	}
	
	public static boolean dispatchKeyUp (int keyCode, KeyEvent event) {
		if ( thePlugin == null || thePlugin.mCordovaView == null ) {
			return false;				
		}
		else {
			return thePlugin.mCordovaView.onKeyUp(keyCode, event);
		}
	}
	
	public static Activity getActivity() {
		Activity theActivity = null;
		if (thePlugin != null) {
			theActivity = thePlugin.mActivity;
		}
		return theActivity;
	}
	
	class DirectCall {
		@JavascriptInterface
		public int setRenderCommands(String commands) {
			int fps = CanvasJNI.setRenderCommands(commands);
//			thePlugin.mCanvasView.requestRender();

			return fps;
		}
	}

}

