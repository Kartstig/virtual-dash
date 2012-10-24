package com.singh.VirtualDashPro;

import java.io.DataInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.UUID;
import android.app.IntentService;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Binder;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.Process;
import android.os.RemoteException;
import android.util.Log;
import android.view.Gravity;
import android.widget.Toast;

public class BluetoothService extends Service {
	private static final String TAG = "BluetoothService";
	private static final boolean D = true;

	private NotificationManager mNM;

	// Unique Identification Number for the Notification.
	// We use it on Notification start, and to cancel it.
	private int NOTIFICATION = R.string.app_name;

	private static final UUID MY_UUID = UUID
			.fromString("00001101-0000-1000-8000-00805F9B34FB");

	private BluetoothAdapter bluetoothAdapter;
	private BluetoothSocket bluetoothSocket;
	private BluetoothDevice bluetoothDevice;

	private String selectedDevice = null;
	private String status = null;
	private String ACTION_REQUEST = null;

	private Bundle startData = new Bundle();

	public static boolean isRunning = false;

	private final IBluetoothService.Stub mBinder = new IBluetoothService.Stub() {

		public int getPid(int pid) throws RemoteException {
			pid = Process.myPid();
			return pid;
		}

		public String Status(String status) throws RemoteException {

			if (bluetoothAdapter.isEnabled()) {
				if (bluetoothAdapter.isDiscovering()) {
					status = "DISCOVERING";
				} else {
					status = "ENABLED";
				}
			} else {
				status = "DISABLED";
			}

			Log.i(TAG, status);
			return status;
		}
	};

	@Override
	public IBinder onBind(Intent intent) {
		if (D)
			Log.e(TAG, "+++ ON BIND +++");

		Toast toast = Toast.makeText(getApplicationContext(),
				"Bluetooth Service Bound", Toast.LENGTH_SHORT);
		toast.setGravity(Gravity.BOTTOM, 0, 10);
		toast.show();

		return mBinder;
	}

	@Override
	public void onCreate() {
		if (D)
			Log.e(TAG, "+++ ON CREATE +++");

		mNM = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);

		// Display a notification about us starting. We put an icon in the
		// status bar.
		showNotification();
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startid) {
		Log.i("LocalService", "Received start id " + startid + ": " + intent);
		Log.d(TAG, "BluetoothService is started.");

		isRunning = true;

		// Get device information
		startData = intent.getExtras();

		selectedDevice = startData.getString("selectedDevice");
		ACTION_REQUEST = startData.getString("ACTION");
		Log.d(TAG, selectedDevice);
		Log.d(TAG, ACTION_REQUEST);

		// Connect to device if not null
		if (ACTION_REQUEST == "SERVICE_REQUEST_CONNECT") {
			if (!(selectedDevice == "") && !(selectedDevice == null)) {
				Toast.makeText(getApplicationContext(),
						"Attempting to connect to " + selectedDevice,
						Toast.LENGTH_LONG).show();
				// bluetoothDevice =
				bluetoothAdapter.getRemoteDevice(selectedDevice);
				connectToDevice();
			}
			else{
				Toast.makeText(getApplicationContext(),"No device!", Toast.LENGTH_LONG).show();
			}
		}
		return START_STICKY;
	}

	// Establish a connection to the device
	private void connectToDevice() {
		try {
			System.out.println("making connection to remote device");
			bluetoothSocket = bluetoothDevice
					.createRfcommSocketToServiceRecord(MY_UUID);
			bluetoothSocket.connect();
		} catch (IOException e) {
			Log.e("Bluetooth Socket",
					"Bluetooth not available, or insufficient permissions");
		} catch (NullPointerException e) {
			Log.e("Bluetooth Socket", "NPE - connectToDevice");
		} finally {
			Log.d(TAG, "Connected to: " + selectedDevice);
			Toast.makeText(getApplicationContext(),
					"Connected to: " + selectedDevice, Toast.LENGTH_LONG)
					.show();
			readData();
		}
	}

	public String readData() {
		String result = "";
		try {
			InputStream input = bluetoothSocket.getInputStream();
			DataInputStream dinput = new DataInputStream(input);
			result = new String(dinput.readLine());
			if (!(result == "" || result == null || result.contains(",") == false)) {
				return result;
			}
		} catch (IOException e) {
			// TODO Auto-generated catch block
			Log.e("Bluetooth Input Stream Creation",
					"Could not open input stream to device: "
							+ bluetoothDevice.getName());
		}
		return result;
	}

	private void showNotification() {
		// In this sample, we'll use the same text for the ticker and the
		// expanded notification
		CharSequence text = getText(R.string.app_name);

		// Set the icon, scrolling text and timestamp
		Notification notification = new Notification(R.drawable.ic_launcher,
				text, System.currentTimeMillis());

		// The PendingIntent to launch our activity if the user selects this
		// notification
		PendingIntent contentIntent = PendingIntent.getActivity(this, 0,
				new Intent(this, MenuActivity.class), 0);

		// Set the info for the views that show in the notification panel.
		notification.setLatestEventInfo(this, getText(R.string.app_name), text,
				contentIntent);

		// Send the notification.
		mNM.notify(NOTIFICATION, notification);
	}

	@Override
	public void onDestroy() {
		if (D)
			Log.e(TAG, "--- ON DESTROY ---");
		Toast.makeText(getApplicationContext(), "Bluetooth Service Stopped",
				Toast.LENGTH_LONG).show();

		isRunning = false;

		// Cancel the persistent notification.
		mNM.cancel(NOTIFICATION);
	}
}
