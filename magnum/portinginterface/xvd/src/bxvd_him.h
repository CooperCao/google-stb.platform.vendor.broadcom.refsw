/***************************************************************************
 *     Copyright (c) 2006-2012, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Macros for reading/writing words to/from the Host Interface Memory.
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef _BXVD_HIM_H_
#define _BXVD_HIM_H_

#ifdef __cplusplus
extern "C" {
#endif

#define BXVD_P_HOSTINTERFACEMEMORY_DUMP_OFFSETS( _str_, _stOffsets_ )               \
{                                                                                   \
   BKNI_Printf("%s: base: %08x word: %08x shift: %08x mask: %08x inverse %08x\n",   \
                     _str_,                                                         \
                     _stOffsets_.ulByteOffset,                                      \
                     _stOffsets_.ulWordOffset,                                      \
                     _stOffsets_.ulByteShift,                                       \
                     _stOffsets_.ulByteMask,                                        \
                     _stOffsets_.ulInverseMask                                      \
                     );                                                             \
}



#define BXVD_AVD_P_HOSTINTERFACEMEMORY_READ( _hXvdCh_, _stOffset_, _pulValueFromMemory_ )    \
{                                                                                            \
   unsigned long  _ulTempValueFromMemory_ ;                                                  \
   BXVD_Handle _hXvd_ = _hXvdCh_->pXvd;                                                      \
                                                                                             \
   /* Read a word from the Host Interface Memory. */                                         \
   BXVD_P_READ_HIM( _hXvd_, _stOffset_.ulWordOffset, _ulTempValueFromMemory_ );              \
                                                                                             \
   /* Shift the value to get the appropriate byte. */                                        \
   _ulTempValueFromMemory_ >>= _stOffset_.ulByteShift ;                                      \
                                                                                             \
   /* Mask off any higher bits. */                                                           \
   _ulTempValueFromMemory_ &= _stOffset_.ulByteMask ;                                        \
                                                                                             \
   *_pulValueFromMemory_ = (uint32_t)_ulTempValueFromMemory_ ;                               \
}

#define BXVD_AVD_P_HOSTINTERFACEMEMORY_WRITE( _hXvdCh_, _stOffset_, _ulValueToMemory_ )      \
{                                                                                            \
   unsigned long  _ulTempValueToMemory_ ;                                                    \
   unsigned long  _ulCurrentValueInMemory_ ;                                                 \
   BXVD_Handle _hXvd_ = _hXvdCh_->pXvd ;                                                     \
                                                                                             \
   /* Read the current word from the Host Interface Memory. */                               \
   BXVD_P_READ_HIM( _hXvd_, _stOffset_.ulWordOffset, _ulCurrentValueInMemory_ );             \
                                                                                             \
   /* Mask off the byte about to be written */                                               \
   _ulCurrentValueInMemory_ &= _stOffset_.ulInverseMask ;                                    \
                                                                                             \
   /* Shift the value to get the appropriate byte. */                                        \
   _ulTempValueToMemory_ = _ulValueToMemory_  ;                                              \
   _ulTempValueToMemory_ <<= _stOffset_.ulByteShift ;                                        \
                                                                                             \
   /* Or the new byte with the other three current bytes. */                                 \
   _ulTempValueToMemory_ = ( _ulTempValueToMemory_ |  _ulCurrentValueInMemory_ ) ;           \
                                                                                             \
   /* Write a word to the Host Interface Memory. */                                          \
   BXVD_P_WRITE_HIM( _hXvd_, _stOffset_.ulWordOffset, _ulTempValueToMemory_ );               \
}

/*
 * Macros for managing the delivery queue.
 */
#define BXVD_AVD_P_DELIVERY_QUEUE_GET_READ_OFFSET( _hXvdCh_, _uiReadOffset_ )                                 \
{                                                                                                                 \
   BXVD_AVD_P_HOSTINTERFACEMEMORY_READ( _hXvdCh_, _hXvdCh_->stDeliveryQueue.stReadIndex, &_uiReadOffset_ );   \
}

#define BXVD_AVD_P_DELIVERY_QUEUE_SET_READ_OFFSET( _hXvdCh_, _uiReadOffset_ )                                 \
{                                                                                                                 \
   BXVD_AVD_P_HOSTINTERFACEMEMORY_WRITE( _hXvdCh_, _hXvdCh_->stDeliveryQueue.stReadIndex, _uiReadOffset_ );   \
}

#define BXVD_AVD_P_DELIVERY_QUEUE_GET_WRITE_OFFSET( _hXvdCh_, _uiWriteOffset_ )                               \
{                                                                                                                 \
   BXVD_AVD_P_HOSTINTERFACEMEMORY_READ( _hXvdCh_, _hXvdCh_->stDeliveryQueue.stWriteIndex, &_uiWriteOffset_ ); \
}

#define BXVD_AVD_P_DELIVERY_QUEUE_GET_ADDR( _hXvdCh_, _pDeliveryQue_ )                \
{                                                                                         \
    _pDeliveryQue_ = _hXvdCh_->stChBufferConfig.AVD_DMS2PI_Buffer.pPictureDeliveryQueue;  \
}

#define BXVD_AVD_P_RELEASE_QUEUE_GET_ADDR( _hXvdCh_, _pReleaseQue_ )               \
{                                                                                      \
   _pReleaseQue_= _hXvdCh_->stChBufferConfig.AVD_DMS2PI_Buffer.pPictureReleaseQueue;   \
}

/*
 * Macros for managing the release queue.
 */
#define BXVD_AVD_P_RELEASE_QUEUE_GET_READ_OFFSET( _hXvdCh_, _uiReadOffset_ )                                   \
{                                                                                                              \
   BXVD_AVD_P_HOSTINTERFACEMEMORY_READ( _hXvdCh_, _hXvdCh_->stReleaseQueue.stReadIndex, &_uiReadOffset_ );     \
}

#define BXVD_AVD_P_RELEASE_QUEUE_GET_WRITE_OFFSET( _hXvdCh_, _uiWriteOffset_ )                                 \
{                                                                                                              \
   BXVD_AVD_P_HOSTINTERFACEMEMORY_READ( _hXvdCh_, _hXvdCh_->stReleaseQueue.stWriteIndex, &_uiWriteOffset_ );   \
}

#define BXVD_AVD_P_RELEASE_QUEUE_SET_WRITE_OFFSET( _hXvdCh_, _uiWriteOffset_ )                                 \
{                                                                                                              \
   BXVD_AVD_P_HOSTINTERFACEMEMORY_WRITE( _hXvdCh_, _hXvdCh_->stReleaseQueue.stWriteIndex, _uiWriteOffset_ );   \
}

#define BXVD_AVD_P_RELEASE_QUEUE_GET_SHADOW_WRITE_OFFSET( _hXvdCh_, _uiWriteOffset_ )                                \
{                                                                                                                    \
   BXVD_AVD_P_HOSTINTERFACEMEMORY_READ( _hXvdCh_, _hXvdCh_->stReleaseQueue.stShadowWriteIndex, &_uiWriteOffset_ );   \
}

#define BXVD_AVD_P_RELEASE_QUEUE_SET_SHADOW_WRITE_OFFSET( _hXvdCh_, _uiWriteOffset_ )                                \
{                                                                                                                    \
   BXVD_AVD_P_HOSTINTERFACEMEMORY_WRITE( _hXvdCh_, _hXvdCh_->stReleaseQueue.stShadowWriteIndex, _uiWriteOffset_ );   \
}

/*
 * Macros for accessing the drop count.
 */
#define BXVD_AVD_P_SET_DROP_COUNT( _hXvdCh_, _count_ )                                    \
{                                                                                         \
   BXVD_AVD_P_HOSTINTERFACEMEMORY_WRITE( _hXvdCh_, _hXvdCh_->stDropCountIndex, _count_ ); \
}

#define BXVD_AVD_P_GET_DROP_COUNT( _hXvdCh_, _count_ )                                     \
{                                                                                          \
   BXVD_AVD_P_HOSTINTERFACEMEMORY_READ( _hXvdCh_, _hXvdCh_->stDropCountIndex, &_count_ );  \
}

/*
 * Macro for retrieving the AVD status block.
 */
#define BXVD_AVD_P_GET_VIDEO_DECODER_STATUS( _hXvdCh_, _ulStatus_ )                             \
{                                                                                               \
   BXVD_AVD_P_HOSTINTERFACEMEMORY_READ( _hXvdCh_, _hXvdCh_->stStatusBlockIndex, &_ulStatus_ );  \
}

#ifdef __cplusplus
}
#endif

#endif /* _BXVD_HIM_H_ */
