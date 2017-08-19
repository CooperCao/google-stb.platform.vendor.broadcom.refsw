package com.broadcom.android.wpsguitester;


import android.widget.Button;
import android.util.AttributeSet;
import android.content.Context;
import android.view.View;
import com.broadcom.android.wpsguitester.IWLWPSClient;

public class BRCMButton extends Button {

	public BRCMButton(Context context, AttributeSet attrs) {
		super(context, attrs);  
		setOnClickListener(mButtonListener);
	}

	private OnClickListener mButtonListener = new OnClickListener()
	{
		public void onClick(View v) {
			//do something 
			if (v.getId() == R.id.button_scan)
			{
				WPSGUITester.btnEnroll.setEnabled(false);
				WPSGUITester.scan_list.removeAllItems();
				WPSGUITester.btnScan.setEnabled(false); 
				Thread scanThread = new Thread (null, mScan, "WPSScanThread"); 
				scanThread.start();
			}
			else if (v.getId() == R.id.button_enroll)
			{
				WPSGUITester.btnEnroll.setEnabled(false);
				WPSGUITester.btnScan.setEnabled(false); 
				Thread enrolThread = new Thread (null, mEnroll, "WPSEnrollThread"); 
				enrolThread.start();
			}
		}
	};

	private Runnable mScan = new Runnable() {
		public void run() {
			String pin = new String();
			if (WPSGUITester.pin_mode.isChecked())
			{
				pin = WPSGUITester.pin_value.getText().toString();
			}

			WPSGUITester.wlWPSClient.native_wpsRefreshScanList(pin);
		}
	};

	private Runnable mEnroll = new Runnable() {
		public void run() {
			String bssid = WPSGUITester.scan_list.getSelectedBssid();
			if (bssid != null)
			{
				WPSGUITester.wlWPSClient.native_wpsEnroll(bssid);
			}
		}
	};

} 
    
    