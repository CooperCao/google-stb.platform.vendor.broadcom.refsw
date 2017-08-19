package com.broadcom.android.wpsguitester;

import android.widget.ListView;
import android.content.Context;
import android.widget.SimpleAdapter;
import android.util.AttributeSet;
import java.util.ArrayList;
import java.util.HashMap;
import android.widget.AdapterView;
import android.view.View;
import android.util.Log;
public class BRCMScanList extends ListView {
	ArrayList<HashMap<String,String>> list = new ArrayList<HashMap<String,String>>();
	public static int iSelectedRow = -1;
	public BRCMScanList(Context context, AttributeSet attrs) {
		super(context, attrs);   
		setBackgroundColor(R.drawable.translucent_background);   
		setChoiceMode(ListView.CHOICE_MODE_SINGLE);
		setItemsCanFocus(false);

		setOnItemClickListener(new OnItemClickListener() {
			public void onItemClick(AdapterView <?> parent, View v, int position, long id) {
				iSelectedRow = position;
				WPSGUITester.btnEnroll.setEnabled(true);
				Log.e("onItemClick", Integer.toString(position));	 	
			}
		});  

		setOnItemSelectedListener(new OnItemSelectedListener() {
			public void onItemSelected(AdapterView <?> parent, View v, int position, long id) {
				iSelectedRow = position;
				WPSGUITester.btnEnroll.setEnabled(true);
				Log.e("onItemSelected", Integer.toString(position));	 	

			}

			public void onNothingSelected(AdapterView <?> parent) {
				//	iSelectedRow = -1;
				Log.e("onNothingSelected","1");	 	

			}
		});
	} 

	public void addItem(BRCMScanItem scanItem) {
		HashMap<String,String> item = new HashMap<String,String>();
		item.put( "ssid",scanItem.getSsid());
		item.put( "bssid",scanItem.getBssid());
		item.put( "wep",Integer.toString(scanItem.getWep()));
		item.put( "band",Integer.toString(scanItem.getBand()));
		list.add( item );
		SimpleAdapter notes = new SimpleAdapter( 
			getContext(), 
			list,
			R.layout.scan_list,
			new String[] { "ssid", "bssid" },
			new int[] { R.id.ssid, R.id.bssid }  );
		setAdapter( notes );

	}

	public void removeAllItems() {
		while (list.size() != 0)
		{
			Log.e("Removing Scan Items", Integer.toString(list.size()));	 	
			list.remove(0);
		}
	}

	public String getSelectedBssid()
	{
		Log.e(getClass().getSimpleName(), Integer.toString(iSelectedRow));	 	
		if (iSelectedRow < list.size())
		{
			HashMap<String,String> item = list.get(iSelectedRow);
			Log.e("1", Integer.toString(iSelectedRow));	 	
			Log.e("2",  item.get("bssid"));	 	

			return item.get("bssid");
		}
		return null;
	}
}

//				android.R.layout.simple_list_item_single_choice,

