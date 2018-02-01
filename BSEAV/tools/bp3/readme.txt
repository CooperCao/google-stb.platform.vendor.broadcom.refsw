bp3 is a tool which directly provisions a Broadcom chip by securely enabling/disabling IP features on the Broadcom SOC.

Source Location:
    BSEAV/tools/bp3

Source dependency location:
    BSEAV/opensource/openssl
    BSEAV/opensource/zlib
    BSEAV/opensource/curl

How to build:
    make install (this builds bp3 app)

How to run:
	1) For example, to provision licenses 3 and 4 from broadcom bp3 portal site,
       on the set top box, type:
       a) STB> cd ../nexus/bin
       b) STB> nexus nxserver &
       b) STB> ./nexus.client bp3 provision -portal https://bp3.broadcom.com -user < your username> -password <your password> -license 3,4

    You should see sample following messages on the console:
            00:00:00.035 bp3_curl: Server: https://bp3.broadcom.com
        *** 00:00:00.168 srai: _srai_get_heap: given heap (nil) is not valid. Consider using SRAI_SetSettings() with proper heap indexes.
        *** 00:00:00.168 srai: _srai_init_settings: export secure heap is not configured
            00:00:00.178 bp3_curl: License ID: 3
            00:00:00.178 bp3_curl: License ID: 4
            00:00:00.213 bp3_main: pBp3BinBuff 0x817ad000
            00:00:00.213 bp3_main: bp3bin filePath ./bp3.bin
            00:00:00.214 bp3_main: oem provisioning
            00:00:00.214 bp3_main: pEncryptedCcfBuff 0x817ab000, size 912
        *** 00:00:00.315 srai: BKNI_WaitForEvent TIMEOUT 100 ms
        *** 00:00:00.415 srai: BKNI_WaitForEvent TIMEOUT 200 ms
        *** 00:00:00.515 srai: BKNI_WaitForEvent TIMEOUT 300 ms
        *** 00:00:00.615 srai: BKNI_WaitForEvent TIMEOUT 400 ms
            00:00:00.617 bp3_module_host: bp3.bin size=720, log size=316, rc=0
            00:00:00.617 bp3_module_host: CCF block 0 or header status 0
            00:00:00.617 bp3_module_host: CCF block 1 status 0
            00:00:00.617 bp3_module_host: CCF block 2 status 0
            00:00:00.617 bp3_module_host: CCF block 3 status 0
            00:00:00.617 bp3_module_host: CCF block 4 status 0
        *** 00:00:00.625 kernelinterface: Event 0x156bd6b0 still in the group 0x156bcdb0, removing it
        *** 00:00:00.625 kernelinterface: Event 0x156bd610 still in the group 0x156bcdb0, removing it
        *** 00:00:00.625 kernelinterface: Event 0x156bd3e0 still in the group 0x156bcdb0, removing it
        *** 00:00:00.625 kernelinterface: Event 0x156bd340 still in the group 0x156bcdb0, removing it
        Provision succeeded

    2) For example, to check what features has been provisioned on the set top box, type:
        a) STB> ./nexus.client bp3 status

    You should see sample following messages on the console:
        Broadcom - H264/AVC
        Broadcom - MPEG-2
        Broadcom - AVS
        Broadcom - H265 (HEVC)
        Broadcom - VP9
        Broadcom - Dolby Vision HDR Activation ($)
        Broadcom - CA Multi2
        Broadcom - CA DVB-CSA3
        Dolby - Post Proc: DAP
        Dolby - Decode Dolby Digital
        Dolby - Decode Dolby Digital Plus
        Dolby - Decode AC4
        Dolby - Decode TrueHD
        Dolby - MS10/11
        Dolby - MS12 v1
        Dolby - MS12 v2
        Dolby - Decode Dolby Vision