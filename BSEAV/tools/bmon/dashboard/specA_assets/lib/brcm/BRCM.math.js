/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/
/**
* The BRCM global namespace object.  If BRCM is already defined, the
* existing BRCM object will not be overwritten so that defined
* namespaces are preserved.  This also allows you to include any
* BRCM libraries in any order.
* @class BRCM
* @static
*/
var BRCM = BRCM || {};

BRCM.math = (function () {

    var _brcmUtil = BRCM.util;

    var Complex = function (real, imag) {
        this.real = real;
        this.imag = imag;
    };

    var logx = function (value, base) {
        return Math.log(value) / Math.log(base);
    };

    var log2 = function (value) {
        return Math.log(value) / Math.log(2);
    };

    var log10 = function (value) {
        return Math.log(value) / Math.log(10);
    };

    var swap = function (data, a, b) {
        var temp = data[a];
        data[a] = data[b];
        data[b] = temp;
    };

    /**
    * Algorithm taken from numerical recipes
    */
    var fourierTransform = function (data, fftSize, inv) {
        var n, mmax, m, j, istep, i, sign;
        var wtemp, wr, wpr, wpi, wi, theta, tempr, tempi;

        // 0 pad the data up to 2*fftSize (2*fftSize because array contains real an imag on separate indices)
        for (var i = data.length; i < (2 * fftSize); i++) {
            data[i] = 0;
        }

        // Ensure fftSize is a power of 2
        if (fftSize < 2 || (fftSize & (fftSize - 1) != 0)) {
            _brcmUtil.printMsg("fftSize has to be a power of 2");
            return;
        }
        sign = inv ? -1 : 1;
        n = fftSize << 1;
        j = 1;
        for (i = 1; i < n; i += 2) {
            if (j > i) {
                swap(data, j - 1, i - 1);
                swap(data, j, i);
            }
            m = fftSize;
            while (m >= 2 && j > m) {
                j -= m;
                m >>>= 1;
            }
            j += m;
        }
        mmax = 2;
        while (n > mmax) {
            istep = mmax << 1;
            theta = sign * (6.28318530717959 / mmax);
            wtemp = Math.sin(0.5 * theta);
            wpr = -2.0 * wtemp * wtemp;
            wpi = Math.sin(theta);
            wr = 1.0;
            wi = 0.0;
            for (m = 1; m < mmax; m += 2) {
                for (i = m; i <= n; i += istep) {
                    j = i + mmax;
                    tempr = wr * data[j - 1] - wi * data[j];
                    tempi = wr * data[j] + wi * data[j - 1];
                    data[j - 1] = data[i - 1] - tempr;
                    data[j] = data[i] - tempi;
                    data[i - 1] += tempr;
                    data[i] += tempi;
                }
                wr = (wtemp = wr) * wpr - wi * wpi + wr;
                wi = wi * wpr + wtemp * wpi + wi;
            }
            mmax = istep;
        }
    };

    var fft = function (data, fftSize) {
        fourierTransform(data, fftSize, false);
    };

    var ifft = function (data, fftSize) {
        fourierTransform(data, fftSize, true);
    };

    return {
        "logx": logx,
        "log2": log2,
        "log10": log10,
        "fft": fft,
        "ifft": ifft
    };
} ());
