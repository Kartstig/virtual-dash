package com.singh.VirtualDashPro;

import com.singh.VirtualDashPro.R.id;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class MenuActivity extends Activity {
	// Debugging
	private static final String TAG = "SetupActivity";
	private static final boolean D = true;

	// GUI elements
	private Button connectButton;
	private static Button dashButton;
	private static Button logButton;
	private static Button plotButton;

	TextView bluetoothStatusMenu;
	View connectIndicator;

	BluetoothAdapter bluetoothAdapter;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		if (D)
			Log.e(TAG, "+++ ON CREATE +++");
		
		// Initialize GUI
		initializeGUI();

		// Set Click listeners
		setClickListeners();

		// Check Status
//		updateBluetoothStatus();

	}

	public void onResume(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		if (D)
			Log.e(TAG, "+++ ON RESUME +++");
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
		if (D)
			Log.e(TAG, "--- ON DESTROY ---");

	}
	
	// Click Listeners
	private void setClickListeners() {
		connectButton.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				Intent myIntent = new Intent(v.getContext(),
						SetupActivity.class);
				startActivityForResult(myIntent, 0);
			}
		});

		dashButton.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				Intent intent = new Intent(v.getContext(), DashActivity.class);
				startActivityForResult(intent, 0);
			}
		});

		logButton.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				Intent intent = new Intent(v.getContext(), LoggerActivity.class);
				startActivityForResult(intent, 0);
			}
		});

		plotButton.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				Intent intent = new Intent(v.getContext(),
						PlotterActivity.class);
				startActivityForResult(intent, 0);
			}
		});
	}
	
	// GUI	
	private void initializeGUI() {
		setContentView(R.layout.menu_activity);
		
		connectButton = (Button) findViewById(R.id.connect);
		dashButton = (Button) findViewById(R.id.dash);
		logButton = (Button) findViewById(R.id.log);
		plotButton = (Button) findViewById(R.id.plot);
		
		bluetoothStatusMenu = (TextView) findViewById(R.id.bluetoothStatusMenu);
		
	}


	// Menu
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.menu, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		case R.id.menu_exit:
			Intent iExit = new Intent(Intent.ACTION_MAIN);
			iExit.addCategory(Intent.CATEGORY_HOME);
			iExit.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
			startActivity(iExit);
			return true;
		case R.id.menu_disconnect:
			Intent iStopService = new Intent(this, BluetoothService.class);
			stopService(iStopService);
			return true;
		case R.id.menu_setup:
			Intent iSetup = new Intent(this, SetupActivity.class);
			startActivity(iSetup);
			return true;
		default:
			return false;
		}

	}

}