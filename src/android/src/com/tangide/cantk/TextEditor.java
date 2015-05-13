/*
 * File     : TextEditor.java
 * Brief    : TextEditor
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
import android.content.Context;
import org.apache.cordova.CallbackContext;

import android.view.View;
import android.view.Gravity;
import android.app.Activity;
import android.widget.EditText;    
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;

public class TextEditor extends EditText {
	public String mURL;
	private Activity mActivity;
	public static String LOGTAG = "com.tangide.cantk.TextEditor";

	public TextEditor(Activity activity) {
		super(activity);
		mActivity = activity;

		this.setOnFocusChangeListener(new View.OnFocusChangeListener() {
			@Override
			public void onFocusChange(View v, boolean hasFocus) {
			    if(!hasFocus){
			    	TextEditor edit = (TextEditor)v;
			    	Activity activity = edit.getActivity();
			    	String text = edit.getText().toString();
			    	edit.notifyChanged(text);	
					InputMethodManager imm = (InputMethodManager)activity.getSystemService(Context.INPUT_METHOD_SERVICE);
					imm.hideSoftInputFromWindow(v.getWindowToken(), 0);
					Log.i(LOGTAG, "imm.hideSoftInputFromWindow");
			    }
		   }
		});
	}

	public Activity getActivity() {
		return mActivity;
	}

	public void notifyChanged(String text) {
		String js = "cordova.fireWindowEvent('editorchanged', { 'text':'" + text + "'});";
		RuntimePlugin.execJs(js);
	}

	public void doShow(String inputType, int x, int y, int w, int h, boolean singleLine) {
		if(inputType.equals("email")) {
			this.setInputType(EditorInfo.TYPE_CLASS_TEXT|EditorInfo.TYPE_TEXT_VARIATION_EMAIL_ADDRESS);
		}
		else if(inputType.equals("number")) {
			this.setInputType(EditorInfo.TYPE_CLASS_NUMBER);
		}
		else if(inputType.equals("password")) {
			this.setInputType(EditorInfo.TYPE_CLASS_TEXT|EditorInfo.TYPE_TEXT_VARIATION_PASSWORD);
		}
		else {
			this.setInputType(EditorInfo.TYPE_CLASS_TEXT);
		}

		this.getLayoutParams().width = w;
		this.getLayoutParams().height = h;

		Log.i(LOGTAG, "TextEditor.doShow singleLine=" + singleLine);
		if(singleLine) {
			this.setSingleLine(true);
		}
		else {
			this.setSingleLine(false);
			this.setGravity(Gravity.TOP|Gravity.LEFT);
			this.setHorizontalScrollBarEnabled(false);
			this.setVerticalScrollBarEnabled(true);
			this.setMinLines(2);  
		}

		this.setX(x);
		this.setY(y);
		this.setWidth(w);
		this.setHeight(h);
		this.setVisibility(View.VISIBLE);
		this.requestFocus();

		return;
	}

	public boolean dispatch(String action, JSONArray args, CallbackContext callbackContext) {
		final TextEditor me = this;
		final Activity activity = this.getActivity();

		try {
			if (action.equals("setEditorContent")) {
				final String text = args.getString(0);
				activity.runOnUiThread(new Runnable() {
					@Override
					public void run() {	
						me.setText(text);
					}
				}); 
				callbackContext.success("done");
				
				return true;
			} else if (action.equals("showEditor")) {
				final String inputType = args.getString(0);
				final int x = args.getInt(1);
				final int y = args.getInt(2);
				final int w = args.getInt(3);
				final int h = args.getInt(4);
				final boolean singleLine = args.getBoolean(5);

				activity.runOnUiThread(new Runnable() {
					@Override
					public void run() {	
						me.doShow(inputType, x, y, w, h, singleLine);
					}
				}); 
				callbackContext.success("done");

				return true;
			} else if (action.equals("hideEditor")) {
				activity.runOnUiThread(new Runnable() {
					@Override
					public void run() {	
						me.setVisibility(View.INVISIBLE);
					}
				}); 
				callbackContext.success("done");

				return true;
			}
		}catch(Exception e){
			Log.e(LOGTAG, "Unexpected error " + action, e);
		}

		return false;
	}
}

