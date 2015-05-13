/*
 * File     : SoundMusicPlayer.java
 * Brief    : SoundMusicPlayer
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
import android.media.MediaPlayer;
import android.content.res.AssetManager; 
import org.apache.cordova.CallbackContext;
import android.content.res.AssetFileDescriptor;

public class SoundMusicPlayer extends MediaPlayer {
	public String mURL;
	public static String LOGTAG = "com.tangide.cantk.SoundMusicPlayer";

	public SoundMusicPlayer() {
		super();
	}

	public boolean loadURL(CallbackContext callbackContext, String url) {
		if(mURL == url) return true;

		if(mURL != null) {
			this.reset();
			mURL = url;
		}

		String dataPrefix = "data:image/";
		String filePrefix = "file:///android_asset/";

		if(url.indexOf(dataPrefix) >= 0) {
			String base64Data = url.substring(22);
			byte [] data = Base64.decode(base64Data, 0);
			//TODO
		}
		else if(url.indexOf(filePrefix) >= 0) {
			int soundID = -1;
			String fileName = url.substring(filePrefix.length());
			Log.i(LOGTAG, "SoundMusicPlayer load from asset:" + fileName);
			try {
				AssetManager am = RuntimePlugin.getActivity().getAssets();
				AssetFileDescriptor afd = am.openFd(fileName);
				this.setDataSource(afd.getFileDescriptor(), afd.getStartOffset(), afd.getLength());
				afd.close();
				this.prepare();

        		callbackContext.success(soundID);
				Log.i(LOGTAG, "SoundMusicPlayer loadURL success:" + url);
			}catch(Exception e) {
				e.printStackTrace();
				Log.i(LOGTAG, "SoundMusicPlayer loadURL fail:" + e.toString());
        		callbackContext.error(e.toString());
			}
			return true;
		}
		else {
			//TODO
			Log.i(LOGTAG, "loadSoundMusic from web:" + url);
		}

		return false;
	}
	
	public boolean hookCompletionEvent() {
		final SoundMusicPlayer player = this;
		this.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
			@Override
			public void onCompletion(MediaPlayer mp) {
				String eventStr = "{\"url\":\"" +player.mURL+ "\"}";
				String js = "cordova.fireWindowEvent('soundmusicend', "+eventStr+");";
				RuntimePlugin.execJs(js);
			}
		});

		return true;
	}
	
	public boolean dispatch(String action, JSONArray args, CallbackContext callbackContext) {
		try {
			if (action.equals("loadSoundMusic")) {
				String url = args.getString(0);
				return this.loadURL(callbackContext, url);
			} else if (action.equals("playSoundMusic")) {
				boolean loop = args.getBoolean(0);
				this.hookCompletionEvent();
				this.setLooping(loop);
				this.start();
				callbackContext.success("done");
				return true;
			} else if (action.equals("stopSoundMusic")) {
				this.stop();
				callbackContext.success("done");
				return true;
			} else if (action.equals("pauseSoundMusic")) {
				this.pause();
				callbackContext.success("done");
				return true;
			} else if (action.equals("setSoundMusicVolume")) {
				float volume = (float)args.getDouble(0);
				this.setVolume(volume, volume);
				callbackContext.success("done");
				return true;
			} else if (action.equals("setSoundMusicLoop")) {
				boolean loop = args.getBoolean(0);
				this.setLooping(loop);
				callbackContext.success("done");
				return true;
			}else {
				return false;
			}
		}catch(Exception e){
			Log.e(LOGTAG, "Unexpected error " + action, e);
		}

		return false;
	}
}

