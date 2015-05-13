/*
 * File     : ActionInfo.java
 * Brief    : ActionInfo
 * Author   : Li XianJing <xianjimli@hotmail.com>
 * Web Site : http://www.tangide.com/
 *
 * Copyright (c) 2015 - 2015 Tangram HD.
 *
**/

package com.tangide.cantk;
import android.graphics.Bitmap;

import org.apache.cordova.CallbackContext;
import java.util.concurrent.BlockingQueue;

public class ActionInfo {
	public static int LOAD_IMAGE = 1;
	public static int UNLOAD_IMAGE = 2;

	public int type;
	public int imageID;
	public String url;
	public Bitmap bitmap;
	public CallbackContext callbackContext;

	public static ActionInfo createUnloadImageAction(CallbackContext callbackContext, int imageID) {
		return new ActionInfo(ActionInfo.UNLOAD_IMAGE, callbackContext, imageID);
	}

	public static ActionInfo createLoadImageAction(CallbackContext callbackContext, String url, Bitmap bitmap) {
		return new ActionInfo(ActionInfo.LOAD_IMAGE, callbackContext, url, bitmap);
	}

	public ActionInfo(int type, CallbackContext callbackContext, int imageID) {
		this.type = type;
		this.imageID = imageID;
		this.callbackContext = callbackContext;
	}

	public ActionInfo(int type, CallbackContext callbackContext, String url, Bitmap bitmap) {
		this.type = type;
		this.url = url;
		this.bitmap = bitmap;
		this.callbackContext = callbackContext;
	}
};
