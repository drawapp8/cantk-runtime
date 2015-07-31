package com.tangide.cantk;

import android.util.Log;
import java.io.InputStream;
import java.io.FileInputStream;
import android.util.Base64;
import org.json.JSONArray;
import org.json.JSONObject;
import org.json.JSONException;
import java.io.IOException;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.content.res.AssetManager; 
import org.apache.cordova.CallbackContext;

public class ImageLoader {
    private static final String LOGTAG = "com.tangide.cantk.ImageLoader";
	private static final String FILE_URL_PREFIX = "file://";
	private static final String DATA_URL_PREFIX = "data:image/";
	private static final String ASSETS_URL_PREFIX = "file:///android_asset/";

	private static Bitmap getBitmapFromAsset(String filePath) {
		AssetManager assetManager = RuntimePlugin.getActivity().getAssets();

		Bitmap bitmap = null;
		try {
			InputStream istr = assetManager.open(filePath);
			bitmap = BitmapFactory.decodeStream(istr);
		} catch (IOException e) {
			e.printStackTrace();
			Log.i(LOGTAG, "Load Asset Bitmap Failed:" + e.toString());
		}

		return bitmap;
	}
	
	private static Bitmap getBitmapFromFile(String filePath) {
		Bitmap bitmap = null;
		try {
			FileInputStream istr = new FileInputStream(filePath);
			bitmap = BitmapFactory.decodeStream(istr);
		} catch (IOException e) {
			e.printStackTrace();
			Log.i(LOGTAG, "Load Asset Bitmap Failed:" + e.toString());
		}

		return bitmap;
	}

	public static boolean loadImage(CallbackContext callbackContext, String url) {
		Bitmap bitmap = null;

		if(url.indexOf(DATA_URL_PREFIX) >= 0) {
			String base64Data = url.substring(22);
			byte [] data = Base64.decode(base64Data, 0);
			bitmap = BitmapFactory.decodeByteArray(data, 0, data.length, null);
			Log.i(LOGTAG, "loadImage from base64 data.");
			ActionInfoQueue.addActionInfo(ActionInfo.createLoadImageAction(callbackContext, url, bitmap));
		}
		else if(url.indexOf(ASSETS_URL_PREFIX) >= 0) {
			String fileName = url.substring(ASSETS_URL_PREFIX.length());
			bitmap = ImageLoader.getBitmapFromAsset(fileName);
			Log.i(LOGTAG, "loadImage from asset:" + fileName);
			ActionInfoQueue.addActionInfo(ActionInfo.createLoadImageAction(callbackContext, url, bitmap));
		}
		else if(url.indexOf(FILE_URL_PREFIX) >= 0) {
			String fileName = url.substring(FILE_URL_PREFIX.length());
			bitmap = ImageLoader.getBitmapFromFile(fileName);
			Log.i(LOGTAG, "loadImage from file:" + fileName);
			ActionInfoQueue.addActionInfo(ActionInfo.createLoadImageAction(callbackContext, url, bitmap));
		}
		else {
			new DownloadImageTask(callbackContext, url).execute(url);
			Log.i(LOGTAG, "loadImage from web:" + url);
		}

		return true;
	}

	public static boolean unloadImage(CallbackContext callbackContext, int imageID) {
		ActionInfoQueue.addActionInfo(ActionInfo.createUnloadImageAction(callbackContext, imageID));

		return true;
	}
	
	public static boolean dispatch(String action, JSONArray args, CallbackContext callbackContext) {
		try {
			if (action.equals("loadImage")) {
				return ImageLoader.loadImage(callbackContext, args.getString(0));
			} else if (action.equals("unloadImage")) {
				return ImageLoader.unloadImage(callbackContext, args.getInt(0));
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
