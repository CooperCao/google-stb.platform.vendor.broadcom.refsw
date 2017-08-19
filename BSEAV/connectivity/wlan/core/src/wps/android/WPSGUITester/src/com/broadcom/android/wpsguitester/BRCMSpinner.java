package com.broadcom.android.wpsguitester;

import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.util.AttributeSet;
import android.view.View;
import android.content.Context;
import android.widget.AdapterView;

public class BRCMSpinner extends Spinner {

	static final int WPS_MODE_STA_CONFIGURE_AP = 0;
	static final int WPS_MODE_STA_ENROLLEE_JOIN = 1;
	static final int WPS_MODE_STA_REGISTRAR_JOIN = 2;
	private int lastSelectedPosition = WPS_MODE_STA_ENROLLEE_JOIN;

	public BRCMSpinner(Context context, AttributeSet attrs) {
		super(context, attrs);     
		ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(
			getContext(), R.array.wpsmode, android.R.layout.simple_spinner_item);
		adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
		setAdapter(adapter); 
		setSelection(WPS_MODE_STA_ENROLLEE_JOIN);
	}

	public void setListener() {
		setOnItemSelectedListener(
			new OnItemSelectedListener() {
				public void onItemSelected(
					AdapterView<?> parent, View view, int position, long id) {

						if (position != lastSelectedPosition) {
							lastSelectedPosition = position;
							BRCMMessageBox msgBox = new BRCMMessageBox(getContext());
							msgBox.Display("2", BRCMMessageBox.DIALOG_YES_NO_MESSAGE);
						}
				}

				public void onNothingSelected(AdapterView<?> parent) {
				}
		});
	}

}
