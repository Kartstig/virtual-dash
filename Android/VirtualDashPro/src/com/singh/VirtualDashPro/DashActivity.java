package com.singh.VirtualDashPro;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class DashActivity extends Activity {
	// Debugging
	private static final String TAG = "DashActivity";
	private static final boolean D = true;
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		if(D) Log.e(TAG, "+++ ON CREATE +++");
		
		// Set Content Layout
		setContentView(R.layout.dash_activity);
	}
	
	@Override
	protected void onDestroy() {
		super.onDestroy();
		if (D)
			Log.e(TAG, "--- ON DESTROY ---");

	}
} 
