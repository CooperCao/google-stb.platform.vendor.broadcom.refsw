package com.broadcom.android.wpsguitester;

import java.lang.ref.WeakReference;

import android.app.Activity;
import android.content.Intent;

import android.os.Bundle;
import android.widget.CheckBox;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.content.Context;

public class WPSGUITester extends Activity {
	static BRCMSpinner wps_mode;
	static CheckBox pin_mode;
	static BRCMEditText pin_value;
	static BRCMScanList scan_list;
	static BRCMTextView status;
	static BRCMButton btnScan;
	static BRCMButton btnEnroll;
	static IWLWPSClient wlWPSClient;
	static public Context mContext;

	/** Called when the activity is first created. */
	@Override
		public void onCreate(Bundle savedInstanceState) {
			super.onCreate(savedInstanceState);
			setContentView(R.layout.main);

			mContext = this;
			// WPS Mode 
			wps_mode = (BRCMSpinner) findViewById(R.id.spinner_mode);
			wps_mode.setListener();

			// Pin/PBC Mode 
			pin_mode = (CheckBox) findViewById(R.id.checkbox_pin);
			pin_mode.setChecked(true);

			//* Pin Value 
			pin_value = (BRCMEditText) findViewById(R.id.editbox_pin);
			pin_value.setText("12345670");

			// List view 
			scan_list = (BRCMScanList) findViewById(R.id.listview_scanlist);


			// status 
			status = (BRCMTextView) findViewById(R.id.textview_status);
			status.setText("Pin Mode");

			// buttons 
			btnScan = (BRCMButton) findViewById(R.id.button_scan);
			btnScan.setText("Scan");
			btnScan.setEnabled(false);

			btnEnroll = (BRCMButton) findViewById(R.id.button_enroll);
			btnEnroll.setText("Enroll");
			btnEnroll.setEnabled(false); 

			wlWPSClient = new IWLWPSClient(status);
			Log.e("onDestroy", "ok");	 	

			if (!wlWPSClient.InitWPS())
			{
				BRCMMessageBox msgBox = new BRCMMessageBox(this);
				msgBox.Display("Unable to initialize WPS.", BRCMMessageBox.DIALOG_OK);
				finish();
			}
			else
			{
				btnScan.setEnabled(true);
			}

	}


	@Override public void onDestroy() {
		Log.e("onDestroy", "Start");	 	
		super.onDestroy();
		wlWPSClient.UninitWPS();
		wlWPSClient = null;
		Log.e("onDestroy", "End");	
	}

	static void UpdateEnrollButton()
	{
		if (scan_list.getCount() > 0)
		{
			btnEnroll.setEnabled(true);
		}
		btnScan.setEnabled(true); 
	}

}

class IWLWPSClient {
	private EventHandler mEventHandler;
	static final int  WPS_STATUS_SUCCESS = 0;
	static final int  WPS_STATUS_ERROR  = 1;
	static BRCMTextView mStatus;
	
	static final int REFRESH_SCAN_SUCCESS = 200;
	static final int REFRESH_SCAN_NO_APS = 201;
	static final int REFRESH_SCAN_MULTIPLE_PBC = 202;
	static final int REFRESH_SCAN_CONNECTION_ERROR = 203;

	static final int ENROLL_WPS_SUCCESS = 100;
	static final int ENROLL_WPS_CANCELED = ENROLL_WPS_SUCCESS + 1;
	static final int ENROLL_WPS_PROTOCOL_ERROR = ENROLL_WPS_SUCCESS + 2;
	static final int ENROLL_WPS_UNKNOWN_ERROR = ENROLL_WPS_SUCCESS + 3;
	static final int ENROLL_WPS_PROTOCOL_FAILED = ENROLL_WPS_SUCCESS + 4;
	static final int ENROLL_WPS_UNABLE_TO_JOIN = ENROLL_WPS_SUCCESS + 5;
	static final int ENROLL_NO_AP_MATCH = ENROLL_WPS_SUCCESS + 6;
	static final int ENROLL_WPS_TIMEDOUT = ENROLL_WPS_SUCCESS + 7;
	static final int ENROLL_WPS_INPROGRESS = ENROLL_WPS_SUCCESS + 8;
	
	static final int WPS_ENCRYPT_NONE = 1;
	static final int WPS_ENCRYPT_WEP = 2;
	static final int WPS_ENCRYPT_TKIP = 4;
	static final int WPS_ENCRYPT_AES = 8;
	      
    static {
    	// The runtime will add "lib" on the front and ".o" on the end of
    	// the name supplied to loadLibrary.
    	System.load("libwlwpsjni.so");
     }
    
    public IWLWPSClient(BRCMTextView status) {
    	      
        Looper looper = null;
        if ((looper = Looper.myLooper()) != null) {
            mEventHandler = new EventHandler(this, looper);
        } else if ((looper = Looper.getMainLooper()) != null) {
            mEventHandler = new EventHandler(this, looper);
        } else {
            mEventHandler = null;
        }	
        mStatus = status;
        iWPSInitalized = false;
        
        /* Native setup requires a weak reference to our object.
         * It's easier to create it here than in C++.
         */
        native_setup(new WeakReference<IWLWPSClient>(this));
    }
     
    private class EventHandler extends Handler
    {
        private IWLWPSClient mWLClient;

        public EventHandler(IWLWPSClient wlclient, Looper looper) {
            super(looper);
            mWLClient = wlclient;
        }

        @Override
        public void handleMessage(Message msg) {
            if (mWLClient.mNativeContext == 0) {
               return;
            }
            
            mWLClient.mStatus.setText(msg.obj.toString());
        	Log.e("handleMessage", msg.obj.toString());	 	
 
            switch(msg.what) {
            case IWLWPSClient.REFRESH_SCAN_SUCCESS:
           	    for (int i=0;i<mWLClient.native_wpsGetScanCount(); i++)
               	{
               		BRCMScanItem item = new BRCMScanItem(mWLClient.native_wpsGetScanSsid(i),mWLClient.native_wpsGetScanBssid(i), 1, 2);
               		WPSGUITester.scan_list.addItem(item);
               	} 
       		    WPSGUITester.btnScan.setEnabled(true);	
       		    WPSGUITester.UpdateEnrollButton();
       		break;
            case IWLWPSClient.REFRESH_SCAN_NO_APS:
      		    WPSGUITester.btnScan.setEnabled(true);	
            	break;
            case IWLWPSClient.REFRESH_SCAN_MULTIPLE_PBC:
      		    WPSGUITester.btnScan.setEnabled(true);	
            	break;	
            case IWLWPSClient.REFRESH_SCAN_CONNECTION_ERROR:
            	WPSGUITester.btnScan.setEnabled(true);	
            	break;
            case ENROLL_WPS_SUCCESS:
            	String ssid = mWLClient.native_wpsGetSsid();
            	Log.e("ssid", ssid);	
            	String keyMgmt = mWLClient.native_wpsGetKeyMgmt();
            	Log.e("keyMgmt", keyMgmt);	
            	String key = mWLClient.native_wpsGetKey();
            	if (key != null)
            		Log.e("key", key);	
            	else
            		key = "<no key>";
            	String encryption = mWLClient.native_wpsGetEncryption();
            	
            	Log.e("encryption", encryption);
            	
            	String display = new String("WPS Credentials details:\nSSID : " + ssid + "\nKey Management: " + keyMgmt + "\nKey: " + key + "\nEncryption: " + encryption);
         		BRCMMessageBox msgBox = new BRCMMessageBox(WPSGUITester.mContext);
        		msgBox.Display(display, BRCMMessageBox.DIALOG_OK);
               	WPSGUITester.UpdateEnrollButton();
            	break;
            	
            case ENROLL_WPS_CANCELED :
            	WPSGUITester.UpdateEnrollButton();
            	break;
            	
            case ENROLL_WPS_PROTOCOL_ERROR:
            	WPSGUITester.UpdateEnrollButton();
            	break;
            	
            case ENROLL_WPS_UNKNOWN_ERROR:
            	WPSGUITester.UpdateEnrollButton();
            	break;
            	
            case ENROLL_WPS_PROTOCOL_FAILED:
            	WPSGUITester.UpdateEnrollButton();
            	break;
            	
            case ENROLL_WPS_UNABLE_TO_JOIN:
            	WPSGUITester.UpdateEnrollButton();
            	break;
            	
            case ENROLL_NO_AP_MATCH :
            	WPSGUITester.UpdateEnrollButton();
            	break;
            	
            case ENROLL_WPS_TIMEDOUT:
            	WPSGUITester.UpdateEnrollButton();
            	break;
            
            case ENROLL_WPS_INPROGRESS:
            	WPSGUITester.UpdateEnrollButton();
            	break;
               	
            default:     	
                return;
            }
        }
    }
   
    @SuppressWarnings("unused")
    private static void postEventFromNative(Object wlclient_this,
            int what, int arg1, int arg2, Object obj)
	{
    	IWLWPSClient wlclient = (IWLWPSClient)((WeakReference)wlclient_this).get();
    	if (wlclient == null) {
    		return;
    	}
	
   		Message m = wlclient.mEventHandler.obtainMessage(what, arg1, arg2, obj);
   		wlclient.mEventHandler.sendMessage(m);
 		Log.e("postEventFromNative", obj.toString());	 	

   	}

   
    private native final void native_setup(Object wlclient_this);
    private native final void native_finalize();
    public native int native_wpsOpen();
    public native int native_wpsClose();
    public native int native_wpsRefreshScanList(String pin);
    public native int native_wpsGetScanCount();
    public native String native_wpsGetScanSsid(int iIndex);
    public native String native_wpsGetScanBssid(int iIndex);
    public native int native_wpsEnroll(String ssid);
    public native String native_wpsGetSsid();   
    public native String native_wpsGetKeyMgmt();
    public native String native_wpsGetKey();
    public native String native_wpsGetEncryption();
    public boolean InitWPS()
    {
    	if (!iWPSInitalized )
    	{
    		iWPSInitalized = native_wpsOpen() == 1 ? true :false;
    	}
    	return iWPSInitalized;
    }
    
    public boolean UninitWPS()
    {
    	if (iWPSInitalized)
    	{
    		native_wpsClose();
    		iWPSInitalized = false;
    		
    	}
    	return iWPSInitalized;
    }
    
    public boolean IsWPSIntialized() { return iWPSInitalized; }
    
    //@Override
    protected void finalize() { native_finalize(); mEventHandler = null;}
    public native String[] native_testArray(int command);
    private int mNativeContext; // accessed by native methods
    private int mListenerContext; // accessed by native methods
	private boolean iWPSInitalized;
}

