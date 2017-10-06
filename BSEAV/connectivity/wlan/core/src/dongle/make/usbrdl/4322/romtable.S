# ROM table definition.
#
# $Id: romtable.S,v 1.5 2009-09-23 17:58:30 $
#
# NOTE: This file is parsed by the replace-rommap script.
#
# - add '# .rodata' to symbol declaration line to designate the symbol is read-only data
#   in the text section
# - add '# .abandon' to symbol declaration line to designate the symbol is no longer
#   used although it is in the ROM
#
	.text

	# aes.o
	.extern	aes_cbc_decrypt
	.extern	aes_cbc_encrypt
	.extern	aes_ccm_decrypt
	.extern	aes_ccm_encrypt
	.extern	aes_ccm_mac
	.extern	aes_ccmp_cal_params
	.extern	aes_ccmp_decrypt
	.extern	aes_ccmp_encrypt
	.extern	aes_ctr_crypt
	.extern	aes_decrypt
	.extern	aes_encrypt

	# aeskeywrap.o
	.extern	aes_unwrap
	.extern	aes_wrap

	# bcmccx.o
	.extern	bcm_ccx_hashpwd
	.extern	bcm_ccx_leap_response
	.extern	bcm_ccx_session_key
	.extern	bcm_ckip_rxseq_check

	# bcmwap.o
	.extern	bcm_find_wpaie
	.extern	bcmwpa_akm2WPAauth
	.extern	bcmwpa_cipher2wsec
	.extern	wpa2_cipher
	.extern	wpa_array_cmp
	.extern	wpa_calc_pmkid
	.extern	wpa_calc_ptk
	.extern	wpa_check_mic
	.extern	wpa_cipher
	.extern	wpa_decr_gtk
	.extern	wpa_decr_key_data
	.extern	wpa_find_gtk_encap
	.extern	wpa_find_kde
	.extern	wpa_is_gtk_encap
	.extern	wpa_make_mic
	.extern	wpa_incr_array

	# ccx.o
	.extern	CKIP_key_permute
	.extern	wsec_ckip_mic_check
	.extern	wsec_ckip_mic_compute

	# des.o
	.extern Spbox	#.rodata
	.extern	des
	.extern	deskey

	# hmac.o
	.extern	hmac_md5

	# md4.o
	.extern	MD4Final
	.extern	MD4Init
	.extern	MD4Update

	# md5.o
	.extern	MD5Final
	.extern	MD5Init
	.extern	MD5Update

	# passhash.o
	.extern	passhash

	# prf.o
	.extern	PRF
	.extern	fPRF
	.extern	hmac_sha1

	# rc4.o
	.extern	prepare_key
	.extern	rc4

	# rijndael-alg-fst.o
	.extern	rijndaelDecrypt
	.extern	rijndaelEncrypt
	.extern	rijndaelKeySetupDec
	.extern	rijndaelKeySetupEnc

	# sha.o
	.extern	SHA1Input
	.extern	SHA1Reset
	.extern	SHA1Result

	# tkhash.o
	.extern	tkhash_phase1
	.extern	tkhash_phase2

	# tkmic.o
	.extern	tkip_mic
	.extern	tkip_mic_eom

	# wep.o
	.extern	wep_decrypt
	.extern	wep_encrypt

	# bcmutils.o
	.extern bcm_ctype	# .rodata
	.extern	bcm_atoi
	.extern	bcm_bitcount
	.extern	bcm_ether_atoe
	.extern	bcm_mw_to_qdbm
	.extern	bcm_next_tlv
	.extern	bcm_parse_ordered_tlvs
	.extern	bcm_parse_tlvs
	.extern	bcm_qdbm_to_mw
	.extern	bcm_strtoul
	.extern	bcmstrcat
	.extern	bcmstrstr
	.extern	hndcrc16
	.extern	hndcrc32
	.extern	hndcrc8

	# bcmstdlib.o
	.extern	index
	.extern	memchr
	.extern	memcmp
	.extern	memmove
	.extern	snprintf
	.extern	sprintf
	.extern	strcat
	.extern	strchr
	.extern	strcmp
	.extern	strcpy
	.extern	strcspn
	.extern	strlen
	.extern	strncmp
	.extern	strncpy
	.extern	strrchr
	.extern	strspn
	.extern	strstr
	.extern	strtoul
	.extern	vsnprintf
	.extern	vsprintf


	.global	_romtable
_romtable:
	.word	_romtable_end
	.word	aes_cbc_decrypt
	.word	aes_cbc_encrypt
	.word	aes_ccm_decrypt
	.word	aes_ccm_encrypt
	.word	aes_ccm_mac
	.word	aes_ccmp_cal_params
	.word	aes_ccmp_decrypt
	.word	aes_ccmp_encrypt
	.word	aes_ctr_crypt
	.word	aes_decrypt
	.word	aes_encrypt
	.word	aes_unwrap
	.word	aes_wrap
	.word	bcm_ccx_hashpwd
	.word	bcm_ccx_leap_response
	.word	bcm_ccx_session_key
	.word	bcm_ckip_rxseq_check
	.word	bcm_find_wpaie
	.word	bcmwpa_akm2WPAauth
	.word	bcmwpa_cipher2wsec
	.word	wpa2_cipher
	.word	wpa_array_cmp
	.word	wpa_calc_pmkid
	.word	wpa_calc_ptk
	.word	wpa_check_mic
	.word	wpa_cipher
	.word	wpa_decr_gtk
	.word	wpa_decr_key_data
	.word	wpa_find_gtk_encap
	.word	wpa_find_kde
	.word	wpa_is_gtk_encap
	.word	wpa_make_mic
	.word	wpa_incr_array
	.word	CKIP_key_permute
	.word	wsec_ckip_mic_check
	.word	wsec_ckip_mic_compute
	.word	des
	.word	deskey
	.word	hmac_md5
	.word	MD4Final
	.word	MD4Init
	.word	MD4Update
	.word	MD5Final
	.word	MD5Init
	.word	MD5Update
	.word	passhash
	.word	PRF
	.word	fPRF
	.word	hmac_sha1
	.word	prepare_key
	.word	rc4
	.word	rijndaelDecrypt
	.word	rijndaelEncrypt
	.word	rijndaelKeySetupDec
	.word	rijndaelKeySetupEnc
	.word	SHA1Input
	.word	SHA1Reset
	.word	SHA1Result
	.word	tkhash_phase1
	.word	tkhash_phase2
	.word	tkip_mic
	.word	tkip_mic_eom
	.word	wep_decrypt
	.word	wep_encrypt
	.word	bcm_atoi
	.word	bcm_bitcount
	.word	bcm_ether_atoe
	.word	bcm_mw_to_qdbm
	.word	bcm_next_tlv
	.word	bcm_parse_ordered_tlvs
	.word	bcm_parse_tlvs
	.word	bcm_qdbm_to_mw
	.word	bcm_strtoul
	.word	bcmstrcat
	.word	bcmstrstr
	.word	hndcrc16
	.word	hndcrc32
	.word	hndcrc8
	.word	index
	.word	memchr
	.word	memcmp
	.word	memmove
	.word	snprintf
	.word	sprintf
	.word	strcat
	.word	strchr
	.word	strcmp
	.word	strcpy
	.word	strcspn
	.word	strlen
	.word	strncmp
	.word	strncpy
	.word	strrchr
	.word	strspn
	.word	strstr
	.word	strtoul
	.word	vsnprintf
	.word	vsprintf
	.word	0
_romtable_end:
