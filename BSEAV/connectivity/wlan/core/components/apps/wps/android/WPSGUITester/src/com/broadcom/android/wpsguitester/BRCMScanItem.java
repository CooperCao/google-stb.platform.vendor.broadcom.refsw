package com.broadcom.android.wpsguitester;

public final class BRCMScanItem 
{
	public BRCMScanItem(String ssid, String bssid, int wep, int band) {
		m_ssid = ssid;
		m_bssid = bssid;
		m_wep = wep;
		m_band = band;

	}
	
	public String getSsid() { return m_ssid ; }
	public String getBssid() { return m_bssid ;}
	public int getWep() { return m_wep ;}
	public int getBand() { return m_band ;}
	
    private String m_ssid;
    private String m_bssid;
    private int m_wep;
    private int m_band;
}
