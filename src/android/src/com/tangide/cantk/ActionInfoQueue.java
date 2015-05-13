/*
 * File     : ActionInfoQueue.java
 * Brief    : ActionInfoQueue
 * Author   : Li XianJing <xianjimli@hotmail.com>
 * Web Site : http://www.tangide.com/
 *
 * Copyright (c) 2015 - 2015 Tangram HD.
 *
**/

package com.tangide.cantk;

import android.util.Log;
import org.json.JSONArray;
import org.json.JSONObject;
import org.json.JSONException;

import org.apache.cordova.CallbackContext;

import android.util.Base64;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

public class ActionInfoQueue {
    private static final String LOGTAG = "com.tangide.canvas.queue";
	private static BlockingQueue<ActionInfo> actionInfoQueue = new LinkedBlockingQueue<ActionInfo>();

	public static void addActionInfo(ActionInfo actionInfo) {
		actionInfoQueue.add(actionInfo);

		return;
	}
	
	public static void handleUnloadImage(ActionInfo actionInfo) {
		int imageID = actionInfo.imageID;
		CallbackContext callbackContext = actionInfo.callbackContext;
		
		CanvasJNI.deleteImage(imageID);

		callbackContext.success("done");
		Log.i(LOGTAG, "handleUnloadImage: " + imageID);

		return;
	}

	public static void loadImageAndReturnInfo(String url, Bitmap bitmap, CallbackContext callbackContext) {
		ImageInfo imgInfo = new ImageInfo();
		
		CanvasJNI.loadImage(url, bitmap, imgInfo);
		bitmap.recycle();
		Log.i(LOGTAG, "CanvasJNI.loadImage: " + imgInfo.width + "x" + imgInfo.height + " " + imgInfo.id);

		JSONObject info = new JSONObject();
		try {
			info.put("url",     url);
			info.put("id",      imgInfo.id);
			info.put("width",   imgInfo.width);
			info.put("height",  imgInfo.height);
		} catch (JSONException e) {
			callbackContext.error("Unexpected JSONException.");
			e.printStackTrace();
		}
		callbackContext.success(info);
		Log.i(LOGTAG, "decodeImage success");
	}

	public static void handleLoadImage(ActionInfo actionInfo) {
		String url = actionInfo.url;
		Bitmap bitmap = actionInfo.bitmap;
		CallbackContext callbackContext = actionInfo.callbackContext;

		try {
			ActionInfoQueue.loadImageAndReturnInfo(url, bitmap, callbackContext);
		}catch(Exception e){
			Log.e(LOGTAG, "decodeImage Unexpected error " + url, e);
			callbackContext.error("error");
		}

		return;
	}

	public static void processActions() {
		ActionInfo actionInfo = null;
		if(actionInfoQueue.size() < 1) {
			return;
		}

		while((actionInfo = actionInfoQueue.poll()) != null) {
			if(actionInfo.type == ActionInfo.LOAD_IMAGE) {
				ActionInfoQueue.handleLoadImage(actionInfo);
			}
			else if(actionInfo.type == ActionInfo.UNLOAD_IMAGE) {
				ActionInfoQueue.handleUnloadImage(actionInfo);
			}
		}

		return;
	}
};
