/*
 * File     : SoundEffectPlayer.java
 * Brief    : SoundEffectPlayer
 * Author   : Li XianJing <xianjimli@hotmail.com>
 * Web Site : http://www.tangide.com/
 *
 * Copyright (c) 2015 - 2015 Tangram HD.
 *
**/

package com.tangide.cantk;

import android.util.Log;
import android.util.Base64;
import org.json.JSONArray;
import org.json.JSONObject;
import org.json.JSONException;
import android.media.SoundPool;
import android.media.AudioManager;
import android.content.res.AssetManager; 
import org.apache.cordova.CallbackContext;
import android.content.res.AssetFileDescriptor;

public class SoundEffectPlayer extends SoundPool {
	public String mURL;
	public static String LOGTAG = "com.tangide.cantk.SoundEffectPlayer";
	private static final String FILE_URL_PREFIX = "file://";
	private static final String DATA_URL_PREFIX = "data:image/";
	private static final String ASSETS_URL_PREFIX = "file:///android_asset/";

	public SoundEffectPlayer() {
		super(10, AudioManager.STREAM_MUSIC, 0);
	}

	public boolean loadURL(CallbackContext callbackContext, String url) {
		if(mURL == url) return true;

		if(mURL != null) {
			mURL = url;
		}

		if(url.indexOf(DATA_URL_PREFIX) >= 0) {
			String base64Data = url.substring(22);
			byte [] data = Base64.decode(base64Data, 0);
			//TODO
		}
		else if(url.indexOf(ASSETS_URL_PREFIX) >= 0) {
			int soundID = -1;
			String fileName = url.substring(ASSETS_URL_PREFIX.length());
			Log.i(LOGTAG, "SoundEffectPlayer load from asset:" + fileName);
			try {
				AssetManager am = RuntimePlugin.getActivity().getAssets();
				AssetFileDescriptor afd = am.openFd(fileName);
				soundID = this.load(afd, 0);
				afd.close();

        		callbackContext.success(soundID);
				Log.i(LOGTAG, "SoundEffectPlayer loadURL success:" + url);
			}catch(Exception e) {
				e.printStackTrace();
				Log.i(LOGTAG, "SoundEffectPlayer loadURL fail:" + e.toString());
        		callbackContext.error(e.toString());
			}
			return true;
		}
		else if(url.indexOf(FILE_URL_PREFIX) >= 0) {
			int soundID = -1;
			String fileName = url.substring(FILE_URL_PREFIX.length());
			Log.i(LOGTAG, "SoundEffectPlayer load from file:" + fileName);
			try {
				soundID = this.load(fileName, 0);

        		callbackContext.success(soundID);
				Log.i(LOGTAG, "SoundEffectPlayer loadURL success:" + url);
			}catch(Exception e) {
				e.printStackTrace();
				Log.i(LOGTAG, "SoundEffectPlayer loadURL fail:" + e.toString());
        		callbackContext.error(e.toString());
			}
			return true;
		}
		else {
			//TODO
			Log.i(LOGTAG, "loadSoundEffect from web:" + url);
		}

		return false;
	}
	
	public boolean dispatch(String action, JSONArray args, CallbackContext callbackContext) {
		try {
			if (action.equals("loadSoundEffect")) {
				String url = args.getString(0);
				return this.loadURL(callbackContext, url);
			} else if (action.equals("playSoundEffect")) {
				int soundID = args.getInt(0);
				float volume = (float)args.getDouble(1);
				this.play(soundID, volume, volume, 0, 0, 1.0f);
				callbackContext.success("done");
				return true;
			} else if (action.equals("stopSoundEffect")) {
				int soundID = args.getInt(0);
				this.stop(soundID);
				callbackContext.success("done");
				return true;
			} else if (action.equals("pauseSoundEffect")) {
				int soundID = args.getInt(0);
				this.pause(soundID);
				callbackContext.success("done");
				return true;
			} else if (action.equals("unloadSoundEffect")) {
				int soundID = args.getInt(0);
				this.unload(soundID);
				callbackContext.success("done");
				return true;
			}
			else {
				return false;
			}
		}catch(Exception e){
			Log.e(LOGTAG, "Unexpected error " + action, e);
		}

		return false;
	}
}

