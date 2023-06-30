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

/******************** MPEG-H 3DA channel rendering library *********************

   Author(s):

   Description:

*******************************************************************************/

/***********************************************************************************

 This software module was originally developed by

 Alexander Adami (AudioLabs) AND Fraunhofer IIS

 in the course of development of the ISO/IEC 23008-3 for reference purposes and its
 performance may not have been optimized. This software module is an implementation
 of one or more tools as specified by the ISO/IEC 23008-3 standard. ISO/IEC gives
 you a royalty-free, worldwide, non-exclusive, copyright license to copy, distribute,
 and make derivative works of this software module or modifications  thereof for use
 in implementations or products claiming conformance to the ISO/IEC 23008-3 standard
 and which satisfy any specified conformance criteria. Those intending to use this
 software module in products are advised that its use may infringe existing patents.
 ISO/IEC have no liability for use of this software module or modifications thereof.
 Copyright is not released for products that do not conform to the ISO/IEC 23008-3
 standard.

 Fraunhofer IIS retains full right to modify and use the code for its own purpose,
 assign or donate the code to a third party and to inhibit third parties from using
 the code for products that do not conform to MPEG-related ITU Recommendations and/or
 ISO/IEC International Standards.

 This copyright notice must be included in all copies or derivative works.

 Copyright (c) ISO/IEC 2015.

 ***********************************************************************************/

#include "FDK_formatConverter_constants_stft.h"
#include "FDK_formatConverter_intern.h"
#include "FDK_formatConverter_data.h"

#ifndef FDK_FORMATCONVERTER_ACTIVEDMX_STFT_H
#define FDK_FORMATCONVERTER_ACTIVEDMX_STFT_H

#ifndef MAX_ERB_BANDS
#define MAX_ERB_BANDS (58)
#endif

typedef struct {
  FIXP_DBL** targetEnePrev;
  FIXP_DBL** realizedEnePrev;

  UINT numInChans;             /* number input channels */
  UINT numOutChans;            /* number output channels */
  UINT numStftBands;           /* number of fft bands */
  UINT numErbBands;            /* number of ERB bands */
  UINT* erbFreqIdx;            /* frequency band stop indices corresponding to ERB bands */
  FIXP_DBL** inputBufferStft;  /* STFT input data with interleaved real and imaginary parts */
  FIXP_DBL** outputBufferStft; /* STFT output data with interleaved real and imaginary parts */
  FIXP_DBL**
      prevInputBufferStft; /* previous STFT input data with interleaved real and imaginary parts */
  UINT frameSize;          /* frame size*/
  UINT fftLength;          /* fft transform length*/
  FIXP_DBL eqLimitMax;     /* format: Q3.29 */
  FIXP_DBL eqLimitMin;     /* format: Q3.29 */
  FIXP_DBL fixpAES;        /* format: Q3.29 */
  INT* targetEnePrevExp;
  INT inBufStftHeadroomPrev;
  INT realizedSigHeadroomPrev;
  FIXP_DBL eneSmoothingAlpha;
} activeDownmixer;

void activeDmxProcess_STFT(void* handle);

INT activeDmxStftInit(void** handle, UINT numInChans, UINT numOutChans, FIXP_DBL** inputBufferStft,
                      FIXP_DBL** prevInputBufferStft, FIXP_DBL** outputBufferStft, UINT frameSize,
                      UINT fftLength, FIXP_DBL eqLimitMax, FIXP_DBL eqLimitMin, UINT numStftBands,
                      UINT numErbBands, UINT* stftErbFreqIdx, INT aes);

void activeDmxSetAES(INT AES, void* handle);
void activeDmxClose_STFT(void* handle);

#endif /*FDK_FORMATCONVERTER_ACTIVEDMX_STFT_H*/
