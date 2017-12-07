/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/

#include "crypto.h"
#include "convert.h"

BDBG_MODULE(atlas_crypto);

CCrypto::CCrypto(const char * strName) :
    CMvcModel(strName),
    _size(0),
    _encrypt(false),
    _used(false),
    _keySlot(NULL),
    _securityAlgorithm(NEXUS_SecurityAlgorithm_eMax)
{
    defaultSettings();
}

CCrypto::~CCrypto()
{
}

CCrypto::CCrypto(const CCrypto & crypto) :
    CMvcModel(crypto._strName),
    _size(crypto._size),
    _encrypt(false),
    _used(false),
    _keySlot(NULL),
    _securityAlgorithm(crypto._securityAlgorithm)
{
    BKNI_Memset(_key, 0, _size);
    BKNI_Memcpy(_key, crypto._key, _size);
    _data = (uint8_t *)_key;
}

void CCrypto::setKey(uint8_t * key)
{
    if (key == NULL)
    {
        return;
    }
    else
    {
        BKNI_Memcpy(_key, key, sizeof(_key));
    }
}

void CCrypto::defaultSettings(void)
{
    static const uint8_t key[] = { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x12, 0x34, 0xfe, 0xed, 0xba, 0xbe, 0xbe, 0xef };

    _data = (uint8_t *)_key;
    _size = sizeof(_key);
    BKNI_Memset(_key, 0, _size);
    BKNI_Memcpy(_key, &key, _size);
    _securityAlgorithm = NEXUS_SecurityAlgorithm_eAes;
    _used              = false;
}

void CCryptoCP::defaultSettings(void)
{
    CCrypto::defaultSettings();
}

CCryptoCP::CCryptoCP(const char * strName) :
    CCrypto(strName)
{
    CCrypto::defaultSettings();
}

void CCrypto::operator =(CCrypto &other)
{
    _strName           = other._strName;
    _size              = other._size;
    _encrypt           = false;
    _keySlot           = NULL;
    _securityAlgorithm = other._securityAlgorithm;

    BKNI_Memset(_key, 0, _size);
    BKNI_Memcpy(_key, other._key, _size);
    _data = (uint8_t *)_key;
} /* = */

#if NEXUS_HAS_SECURITY && NEXUS_SECURITY_API_VERSION==1
void CCrypto::removeKey(NEXUS_PidChannelHandle pidChannel)
{
    if (_keySlot && pidChannel)
    {
        NEXUS_KeySlot_RemovePidChannel(_keySlot, pidChannel);
        NEXUS_Security_FreeKeySlot(_keySlot);
    }
    _used = false;
}

void CCryptoCP::removeKey(NEXUS_PidChannelHandle pidChannel)
{
    CCrypto::removeKey(pidChannel);

    _used = false;
}

CCryptoCP::~CCryptoCP()
{
}

/* Crypto CP Class */
eRet CCryptoCP::keySlotConfig(NEXUS_SecurityKeySlotSettings * keySlotSettings)
{
    NEXUS_SecurityKeySlotSettings lKeySlotSettings;
    eRet ret = eRet_Ok;

    if (keySlotSettings != NULL)
    {
        BKNI_Memcpy(&lKeySlotSettings, keySlotSettings, sizeof(lKeySlotSettings));
    }
    else
    {
        NEXUS_Security_GetDefaultKeySlotSettings(&lKeySlotSettings);
    }

    lKeySlotSettings.keySlotEngine = _encrypt ? NEXUS_SecurityEngine_eCaCp : NEXUS_SecurityEngine_eCa;
    _keySlot                       = NEXUS_Security_AllocateKeySlot(&lKeySlotSettings);
    CHECK_PTR_ERROR_GOTO("_keyslot failed", _keySlot, ret, eRet_ExternalError, error);
    BDBG_MSG(("keyslot %p %s ", (void *)_keySlot, _encrypt ? "encrypt" : "decrypt"));

error:
    return(ret);
} /* keySlotConfig */

eRet CCryptoCP::setAlgorithmSettings(NEXUS_SecurityAlgorithmSettings * algConfig)
{
    NEXUS_SecurityAlgorithmSettings AlgConfig;
    NEXUS_Error                     rc = NEXUS_SUCCESS;
    eRet ret                           = eRet_Ok;

    if (algConfig != NULL)
    {
        BKNI_Memcpy(&AlgConfig, algConfig, sizeof(AlgConfig));
    }
    else
    {
        NEXUS_Security_GetDefaultAlgorithmSettings(&AlgConfig);
    }

    /* Config AV algorithms */
    AlgConfig.algorithm       = _securityAlgorithm;
    AlgConfig.algorithmVar    = NEXUS_SecurityAlgorithmVariant_eEcb;
    AlgConfig.terminationMode = NEXUS_SecurityTerminationMode_eClear;
    AlgConfig.ivMode          = NEXUS_SecurityIVMode_eRegular;
    AlgConfig.solitarySelect  = NEXUS_SecuritySolitarySelect_eClear;
    AlgConfig.caVendorID      = 0x1234;
    AlgConfig.askmModuleID    = NEXUS_SecurityAskmModuleID_eModuleID_4;
    AlgConfig.otpId           = NEXUS_SecurityOtpId_eOtpVal;
    AlgConfig.key2Select      = NEXUS_SecurityKey2Select_eReserved1;
    AlgConfig.dest            = _encrypt ? NEXUS_SecurityAlgorithmConfigDestination_eCps : NEXUS_SecurityAlgorithmConfigDestination_eCa;
    AlgConfig.operation       = _encrypt ? NEXUS_SecurityOperation_eEncrypt : NEXUS_SecurityOperation_eDecrypt;
    if (_encrypt)
    {
        AlgConfig.bRestrictEnable    = false;
        AlgConfig.bEncryptBeforeRave = false;
        AlgConfig.bGlobalEnable      = true;
        AlgConfig.modifyScValue[NEXUS_SecurityPacketType_eRestricted] = true;
        AlgConfig.modifyScValue[NEXUS_SecurityPacketType_eGlobal]     = true;
        AlgConfig.keyDestEntryType = NEXUS_SecurityKeyType_eClear;
    }

    if (_encrypt)
    {
        AlgConfig.scValue[NEXUS_SecurityPacketType_eRestricted] =
            AlgConfig.scValue[NEXUS_SecurityPacketType_eGlobal] = NEXUS_SecurityAlgorithmScPolarity_eEven;
        rc = NEXUS_Security_ConfigAlgorithm(_keySlot, &AlgConfig);
        CHECK_NEXUS_ERROR_GOTO("Key Slot Config Failure", ret, rc, error);

        AlgConfig.scValue[NEXUS_SecurityPacketType_eRestricted] =
            AlgConfig.scValue[NEXUS_SecurityPacketType_eGlobal] = NEXUS_SecurityAlgorithmScPolarity_eOdd;
        rc = NEXUS_Security_ConfigAlgorithm(_keySlot, &AlgConfig);
        CHECK_NEXUS_ERROR_GOTO("Key Slot Config Failure", ret, rc, error);
    }
    else
    {
        rc = NEXUS_Security_ConfigAlgorithm(_keySlot, &AlgConfig);
        CHECK_NEXUS_ERROR_GOTO("Security Config Algorithm Failure Failure", ret, rc, error);
    }

error:
    return(ret);
} /* setAlgorithmSettings */

eRet CCryptoCP::readXML(MXmlElement * xmlElement)
{
    eRet    ret = eRet_Ok;
    MString strEncType;
    MString strEncKey;

    strEncKey = xmlElement->attrValue(XML_ATT_ENCKEY);
    if (strEncKey)
    {
        BKNI_Memset(_key, 0, sizeof(_key));
        BDBG_MSG(("StrnEncKey: %s", strEncKey.s()));
        if (strEncKey)
        {
            /* coverity[secure_coding] */
            sscanf(strEncKey.s(), "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                    (int *)&_key[0],
                    (int *)&_key[1],
                    (int *)&_key[2],
                    (int *)&_key[3],
                    (int *)&_key[4],
                    (int *)&_key[5],
                    (int *)&_key[6],
                    (int *)&_key[7],
                    (int *)&_key[8],
                    (int *)&_key[9],
                    (int *)&_key[10],
                    (int *)&_key[11],
                    (int *)&_key[12],
                    (int *)&_key[13],
                    (int *)&_key[14],
                    (int *)&_key[15]);
        }

        BDBG_MSG(("StrnEncKey: %x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n",
                  _key[0],
                  _key[1],
                  _key[2],
                  _key[3],
                  _key[4],
                  _key[5],
                  _key[6],
                  _key[7],
                  _key[8],
                  _key[9],
                  _key[10],
                  _key[11],
                  _key[12],
                  _key[13],
                  _key[14],
                  _key[15]));
    }

    strEncType = xmlElement->attrValue(XML_ATT_ENCTYPE);
    if (strEncType)
    {
        _securityAlgorithm = stringToSecurityAlgorithm(strEncType);
    }
    _data = (uint8_t *)_key;

    return(ret);
} /* readXML */

void CCryptoCP::writeXML(MXmlElement * xmlElement)
{
    MString       strEncKey;
    MXmlElement * xmlElemSecurity = new MXmlElement(xmlElement, XML_TAG_SECURITY);

    strEncKey  = MString(_key[0]) + ",";
    strEncKey += MString(_key[1]) + ",";
    strEncKey += MString(_key[2]) + ",";
    strEncKey += MString(_key[3]) + ",";
    strEncKey += MString(_key[4]) + ",";
    strEncKey += MString(_key[5]) + ",";
    strEncKey += MString(_key[6]) + ",";
    strEncKey += MString(_key[7]) + ",";
    strEncKey += MString(_key[8]) + ",";
    strEncKey += MString(_key[9]) + ",";
    strEncKey += MString(_key[10]) + ",";
    strEncKey += MString(_key[11]) + ",";
    strEncKey += MString(_key[12]) + ",";
    strEncKey += MString(_key[13]) + ",";
    strEncKey += MString(_key[14]) + ",";
    strEncKey += MString(_key[15]);

    BDBG_MSG((" key is %s", strEncKey.s()));
    BDBG_MSG(("Encryption Key %s, size of tmp key %d ", strEncKey.s(), sizeof(_key)));
    xmlElemSecurity->addAttr(XML_ATT_ENCKEY, strEncKey);
    xmlElemSecurity->addAttr(XML_ATT_ENCTYPE, securityAlgorithmToString(_securityAlgorithm));
} /* writeXML */

/* use local key if key is null */
eRet CCryptoCP::loadKey(
        NEXUS_SecurityClearKey * key,
        NEXUS_PidChannelHandle   pidChannel
        )
{
    NEXUS_SecurityClearKey lKey;
    NEXUS_Error            rc  = NEXUS_SUCCESS;
    eRet                   ret = eRet_Ok;

    if (_used)
    {
        /* remove key if you want to load another key first. Cannot load multiple keys to the key slot */
        goto error;
    }

    /* Default Key */
    if (key == NULL)
    {
        BDBG_MSG(("Use default Key Settings"));
        NEXUS_Security_GetDefaultClearKey(&lKey);
        lKey.dest         = _encrypt ? NEXUS_SecurityAlgorithmConfigDestination_eCps : NEXUS_SecurityAlgorithmConfigDestination_eCa;
        lKey.keyEntryType = _encrypt ? NEXUS_SecurityKeyType_eClear : NEXUS_SecurityKeyType_eOddAndEven;
        lKey.keySize      = _size;
        lKey.keyIVType    = NEXUS_SecurityKeyIVType_eNoIV;
        BDBG_MSG(("keysize %d", lKey.keySize));
        BDBG_MSG(("keysize %d", lKey.keySize));
        BDBG_MSG(("keysize %d", lKey.keySize));
        BDBG_MSG(("keysize %d", lKey.keySize));
        BDBG_MSG(("keysize %d", lKey.keySize));
        BKNI_Memcpy(lKey.keyData, _data, _size);
        BDBG_MSG(("2keysize %d", lKey.keySize));
        BDBG_MSG(("2keysize %d", lKey.keySize));
        BDBG_MSG(("2keysize %d", lKey.keySize));
    }
    else
    {
        BKNI_Memcpy(&lKey, key, sizeof(lKey));
    }

    BDBG_MSG(("keysize %d", lKey.keySize));

    rc = NEXUS_Security_LoadClearKey(_keySlot, &lKey);
    CHECK_NEXUS_ERROR_GOTO("Security Load Key Failure", ret, rc, error);

    NEXUS_KeySlot_AddPidChannel(_keySlot, pidChannel);

    _used = true;

error:
    return(ret);
} /* loadKey */

#else /* NEXUS_HAS_SECURITY */
void CCrypto::removeKey(NEXUS_PidChannelHandle pidChannel)
{
    BSTD_UNUSED(pidChannel);
}

CCryptoCP::~CCryptoCP()
{
}

eRet CCryptoCP::loadKey(
        NEXUS_SecurityClearKey * key,
        NEXUS_PidChannelHandle   pidChannel
        )
{
    BSTD_UNUSED(key);
    BSTD_UNUSED(pidChannel);
    return(eRet_NotSupported);
}

eRet CCryptoCP::keySlotConfig(NEXUS_SecurityKeySlotSettings * keySlotSettings)
{
    BSTD_UNUSED(keySlotSettings);
    return(eRet_NotSupported);
}

eRet CCryptoCP::setAlgorithmSettings(NEXUS_SecurityAlgorithmSettings * AlgConfig)
{
    BSTD_UNUSED(AlgConfig);
    return(eRet_NotSupported);
}

eRet CCryptoCP::readXML(MXmlElement * xmlElement)
{
    BSTD_UNUSED(xmlElement);
    return(eRet_NotSupported);
}

void CCryptoCP::writeXML(MXmlElement * xmlElement)
{
    BSTD_UNUSED(xmlElement);
}

void CCryptoCP::removeKey(NEXUS_PidChannelHandle pidChannel)
{
    BSTD_UNUSED(pidChannel);
}

#endif /* NEXUS_HAS_SECURITY*/