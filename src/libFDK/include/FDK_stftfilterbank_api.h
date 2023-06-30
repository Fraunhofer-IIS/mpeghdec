/*-----------------------------------------------------------------------------
Software License for The Fraunhofer FDK MPEG-H Software

Copyright (c) 2018 - 2023 Fraunhofer-Gesellschaft zur Förderung der angewandten
Forschung e.V. and Contributors
All rights reserved.

1. INTRODUCTION

The "Fraunhofer FDK MPEG-H Software" is software that implements the ISO/MPEG
MPEG-H 3D Audio standard for digital audio or related system features. Patent
licenses for necessary patent claims for the Fraunhofer FDK MPEG-H Software
(including those of Fraunhofer), for the use in commercial products and
services, may be obtained from the respective patent owners individually and/or
from Via LA (www.via-la.com).

Fraunhofer supports the development of MPEG-H products and services by offering
additional software, documentation, and technical advice. In addition, it
operates the MPEG-H Trademark Program to ease interoperability testing of end-
products. Please visit www.mpegh.com for more information.

2. COPYRIGHT LICENSE

Redistribution and use in source and binary forms, with or without modification,
are permitted without payment of copyright license fees provided that you
satisfy the following conditions:

* You must retain the complete text of this software license in redistributions
of the Fraunhofer FDK MPEG-H Software or your modifications thereto in source
code form.

* You must retain the complete text of this software license in the
documentation and/or other materials provided with redistributions of
the Fraunhofer FDK MPEG-H Software or your modifications thereto in binary form.
You must make available free of charge copies of the complete source code of
the Fraunhofer FDK MPEG-H Software and your modifications thereto to recipients
of copies in binary form.

* The name of Fraunhofer may not be used to endorse or promote products derived
from the Fraunhofer FDK MPEG-H Software without prior written permission.

* You may not charge copyright license fees for anyone to use, copy or
distribute the Fraunhofer FDK MPEG-H Software or your modifications thereto.

* Your modified versions of the Fraunhofer FDK MPEG-H Software must carry
prominent notices stating that you changed the software and the date of any
change. For modified versions of the Fraunhofer FDK MPEG-H Software, the term
"Fraunhofer FDK MPEG-H Software" must be replaced by the term "Third-Party
Modified Version of the Fraunhofer FDK MPEG-H Software".

3. No PATENT LICENSE

NO EXPRESS OR IMPLIED LICENSES TO ANY PATENT CLAIMS, including without
limitation the patents of Fraunhofer, ARE GRANTED BY THIS SOFTWARE LICENSE.
Fraunhofer provides no warranty of patent non-infringement with respect to this
software. You may use this Fraunhofer FDK MPEG-H Software or modifications
thereto only for purposes that are authorized by appropriate patent licenses.

4. DISCLAIMER

This Fraunhofer FDK MPEG-H Software is provided by Fraunhofer on behalf of the
copyright holders and contributors "AS IS" and WITHOUT ANY EXPRESS OR IMPLIED
WARRANTIES, including but not limited to the implied warranties of
merchantability and fitness for a particular purpose. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE for any direct, indirect,
incidental, special, exemplary, or consequential damages, including but not
limited to procurement of substitute goods or services; loss of use, data, or
profits, or business interruption, however caused and on any theory of
liability, whether in contract, strict liability, or tort (including
negligence), arising in any way out of the use of this software, even if
advised of the possibility of such damage.

5. CONTACT INFORMATION

Fraunhofer Institute for Integrated Circuits IIS
Attention: Division Audio and Media Technologies - MPEG-H FDK
Am Wolfsmantel 33
91058 Erlangen, Germany
www.iis.fraunhofer.de/amm
amm-info@iis.fraunhofer.de
-----------------------------------------------------------------------------*/

/******************* Library for basic calculation routines ********************

   Author(s):   Dirk Mahne, Arthur Tritthart

   Description:

*******************************************************************************/

#ifndef FDK_STFTFILTERBANK_API_H
#define FDK_STFTFILTERBANK_API_H

#include "common_fix.h"

/** STFT Filterbank handle.
    Handle to a STFT Filterbank instance. */
typedef struct T_STFT_FILTERBANK* HANDLE_STFT_FILTERBANK;

/** STFT Filterbank mode, analysis, synthesis.
    STFT_FILTERBANK_MODE_INVALID         : invalid stft filterbank mode, \n
    STFT_FILTERBANK_MODE_TIME_TO_FREQ    : time to frequency transform, analysis, \n
    STFT_FILTERBANK_MODE_FREQ_TO_TIME    : frequency to time transform, synthesis */
typedef enum {
  STFT_FILTERBANK_MODE_INVALID = 0,
  STFT_FILTERBANK_MODE_TIME_TO_FREQ,
  STFT_FILTERBANK_MODE_FREQ_TO_TIME
} STFT_FILTERBANK_MODE;

/** STFT Filterbank configuration.
    Comprises all input parameters to be handled to ::StftFilterbank_Open(). \n \param
   stftFilterbankMode       STFT Filterbank mode STFT_FILTERBANK_MODE. \n \param frameSize Frame
   size, number of samples, per channel, per processing call.        \n \param fftSize FFT size, at
   least 2 * frameSize, if greater, zero padding is performed \n \param ph_stft_filterbank A pointer
   to a handle to STFT Filterbank instance. \return Returns 0 on success, otherwise an error code.
 */
typedef struct T_STFT_FILTERBANK_CONFIG {
  STFT_FILTERBANK_MODE stftFilterbankMode;
  UINT frameSize;
  UINT fftSize;
} STFT_FILTERBANK_CONFIG;

/** Open STFT Filterbank.
    Open a STFT Filterbank instance.
    \param stftFilterbankConfig A pointer to STFT Filterbank configuration.
    \param ph_stft_filterbank A pointer to a handle to STFT Filterbank instance.
    \return Returns 0 on success, otherwise an error code.    */
int StftFilterbank_Open(const STFT_FILTERBANK_CONFIG* stftFilterbankConfig,
                        HANDLE_STFT_FILTERBANK* ph_stft_filterbank);

/** STFT Filterbank processing.
    STFT Filterbank processing, transform of time signal to frequency signal, or vice versa,
   dependent on filterbank mode STFT_FILTERBANK_MODE. \param inputBuffer Input signal, time or
   frequency, dependent on filterbankMode STFT_FILTERBANK_MODE. \param outputBuffer Output signal,
   time or frequency, dependent on filterbankMode STFT_FILTERBANK_MODE. \n For STFT synthesis, the
   scaled output signal is accumulated on outputBuffer. \param h_stft_filterbank A handle to STFT
   Filterbank instance. \return none  */
void StftFilterbank_Process(FIXP_DBL* RESTRICT inputBuffer, FIXP_DBL* RESTRICT outputBuffer,
                            HANDLE_STFT_FILTERBANK h_stft_filterbank, INT STFT_headroom);

/** Close STFT Filterbank.
    Close STFT Filterbank instance.
    \param ph_stft_filterbank A pointer to a handle to STFT Filterbank instance.
    \return Returns 0 on success, otherwise an error code.    */
int StftFilterbank_Close(HANDLE_STFT_FILTERBANK* ph_stft_filterbank);

/** Get STFT Filterbank configuration.
    Returns STFT Filterbank configuration.
    \param stftFilterbankConfig A pointer to STFT Filterbank configuration.
    \param ph_stft_filterbank A pointer to a handle to STFT Filterbank instance.
    \return Returns 0 on success, otherwise an error code.    */
int StftFilterbank_GetConfig(STFT_FILTERBANK_CONFIG* stftFilterbankConfig,
                             const HANDLE_STFT_FILTERBANK h_stft_filterbank);

/** Display STFT Filterbank configuration.
    Display STFT Filterbank configuration.
    \param ph_stft_filterbank A pointer to a handle to STFT Filterbank instance.
    \return Returns 0 on success, otherwise an error code.    */
int StftFilterbank_DisplayConfig(const HANDLE_STFT_FILTERBANK h_stft_filterbank);

/** Convert STFT Filterbank error code to info string.
    Returns an information string for a given STFT Filterbank error code.
    \param stftFilterbankErrorCode  A STFT Filterbank error code, returned by API functions.
    \return Returns an info string according to error code.    */
const char* StftFilterbank_ErrorCodeToStr(const int stftFilterbankErrorCode);

#endif /* #ifndef FDK_STFTFILTERBANK_API_H */
