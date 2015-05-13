package com.tangide.cantk;

import android.util.Log;
import java.io.InputStream;
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

	public static boolean loadImage(CallbackContext callbackContext, String url) {
		Bitmap bitmap = null;
		String dataPrefix = "data:image/";
		String filePrefix = "file:///android_asset/";

		if(url.indexOf(dataPrefix) >= 0) {
			String base64Data = url.substring(22);
			byte [] data = Base64.decode(base64Data, 0);
			bitmap = BitmapFactory.decodeByteArray(data, 0, data.length, null);
			Log.i(LOGTAG, "loadImage from base64 data.");
			ActionInfoQueue.addActionInfo(ActionInfo.createLoadImageAction(callbackContext, url, bitmap));
		}
		else if(url.indexOf(filePrefix) >= 0) {
			String fileName = url.substring(filePrefix.length());
			bitmap = ImageLoader.getBitmapFromAsset(fileName);
			Log.i(LOGTAG, "loadImage from asset:" + fileName);
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
