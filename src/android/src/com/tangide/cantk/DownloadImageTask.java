/*
 * File     : DownloadImageTask.java
 * Brief    : DownloadImageTask 
 * Author   : Li XianJing <xianjimli@hotmail.com>
 * Web Site : http://www.tangide.com/
 *
 * Copyright (c) 2015 - 2015 Tangram HD.
 *
**/

package com.tangide.cantk;

import java.net.URL;
import android.util.Log;
import android.os.AsyncTask;
import java.io.IOException;
import java.io.InputStream;
import android.graphics.Bitmap;
import java.net.HttpURLConnection;
import android.graphics.BitmapFactory;
import org.apache.cordova.CallbackContext;

public class DownloadImageTask extends AsyncTask<String, String, String> {
    private static final String LOGTAG = "com.tangide.cantk.DownloadImageTask";

	private String url;
	private CallbackContext callbackContext;

	public DownloadImageTask(CallbackContext callbackContext, String url) {
		this.url = url;
		this.callbackContext = callbackContext;
	}

	@Override  
	protected String doInBackground(String... params) {
		String url = this.url;
		Bitmap bitmap = this.createBitmapFromWeb(url);
		CallbackContext callbackContext = this.callbackContext;
	
		if(bitmap != null) {
			ActionInfoQueue.addActionInfo(ActionInfo.createLoadImageAction(callbackContext, url, bitmap));
		}
		else {
			callbackContext.error("Not Found");
		}
		return "Done";
	}

	private Bitmap createBitmapFromWeb(String url) {
		Bitmap bmp =null;
		try{
			Log.i(LOGTAG, "AsyncTask:" + url);
			URL ulrn = new URL(url);
			HttpURLConnection con = (HttpURLConnection)ulrn.openConnection();
			InputStream is = con.getInputStream();
			bmp = BitmapFactory.decodeStream(is);
		}
		catch(Exception e){
			Log.i(LOGTAG, "AsyncTask fail:" + e.toString());
			e.printStackTrace();
		}

		return bmp;
	}
}
