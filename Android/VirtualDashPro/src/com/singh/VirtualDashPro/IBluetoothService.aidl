package com.singh.VirtualDashPro;

interface IBluetoothService {
	// Get Process ID of service
	int getPid(int pid);
	
	// Get Bluetooth Status
	String Status(out String status);

}