package com.broadcom.android.wpsguitester;


import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;


public class BRCMMessageBox extends AlertDialog.Builder {
	public static final int DIALOG_YES_NO_MESSAGE = 1;
	public static final int DIALOG_OK = 2;

	public static final int RESULT_YES = 1;
	public static final int RESULT_NO = 2;

	int result;

	public BRCMMessageBox(Context context) {
		super(context); 
		result = RESULT_YES;
	}

	public int getResult() {
		return  result;
	}

	public void Display(String message, int dialogType) {
		setTitle("WPS GUI Tester");
		setMessage(message);
		setIcon(R.drawable.alert_dialog_icon);

		switch(dialogType) {
		 case DIALOG_YES_NO_MESSAGE:
			 setPositiveButton("Yes", new DialogInterface.OnClickListener() {
				 public void onClick(DialogInterface dialog, int whichButton) {
					 result = RESULT_YES;
				 }
			 });
			 setNegativeButton("No", new DialogInterface.OnClickListener() {
				 public void onClick(DialogInterface dialog, int whichButton) {
					 result = RESULT_NO;
				 }
			 }); 
			 break;
		 default:
			 setNeutralButton("OK", new DialogInterface.OnClickListener() {
				 public void onClick(DialogInterface dialog, int whichButton) {
				 }
			 });
			 break;
		}
		show();
	}
}
