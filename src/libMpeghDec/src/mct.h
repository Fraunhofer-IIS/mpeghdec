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

/************************* MPEG-H 3DA decoder library **************************

   Author(s):   Matthias Neusinger

   Description: multi-channel tool

*******************************************************************************/

#ifndef MCT_H
#define MCT_H

#include "aacdecoder.h"

#define MAX_NUM_MCT_CHANNELS (28)
#define MAX_NUM_MCT_BOXES (28) /* MPEG-H LC, see DAM3 Study on Table P9 */
#define MAX_NUM_MCT_BANDS 64

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CMctWork {
  UCHAR keepTree;
  UCHAR numPairs;
  UCHAR codePairs[MAX_NUM_MCT_BOXES][2];
  UCHAR bDeltaTime[MAX_NUM_MCT_BOXES];
  UCHAR pairCoeffDeltaSfb[MAX_NUM_MCT_BOXES][MAX_NUM_MCT_BANDS];
  UCHAR pairCoeffDeltaFb[MAX_NUM_MCT_BOXES];
  UCHAR numMctMaskBands[MAX_NUM_MCT_BOXES];
  UINT64 mctMask[MAX_NUM_MCT_BOXES];
  UCHAR bHasMctMask[MAX_NUM_MCT_BOXES];
  UCHAR bHasBandwiseCoeffs[MAX_NUM_MCT_BOXES];
  UCHAR predDir[MAX_NUM_MCT_BOXES];
  UCHAR hasStereoFilling[MAX_NUM_MCT_CHANNELS];
} CMctWork, *CMctWorkPtr;

typedef struct CMct {
  CMctWorkPtr mctWork;
  UCHAR MCCSignalingType;
  UCHAR MCTSignalingTypePrev;
  UCHAR windowSequenceIsShortPrev[MAX_NUM_MCT_BOXES];
  UCHAR channelMask[MAX_NUM_MCT_CHANNELS];
  UCHAR channelMap[MAX_NUM_MCT_CHANNELS];
  UCHAR numMctChannels;
  SHORT pairCoeffQSfbPrev[MAX_NUM_MCT_BOXES][MAX_NUM_MCT_BANDS];
  SHORT pairCoeffQFbPrev[MAX_NUM_MCT_BOXES]; /* fullband coeff */
  FIXP_DBL* prevOutSpec;
  SHORT* prevOutSpec_exp;
  UCHAR Mct_group_parsed;
} CMct, *CMctPtr;

int CMct_Initialize(CMctPtr* pCMctPtr, const CSUsacExtElementConfig* extElementConfig,
                    int firstSigIdx, int signalsInGroup);

void CMct_Destroy(CMctPtr self);

int CMct_inverseMctParseBS(CMctPtr self, HANDLE_FDK_BITSTREAM hbitBuffer, int usacIndepFlag,
                           int nChannels);

int CMct_MCT_StereoFilling(CMctPtr self, CStreamInfo* streamInfo,
                           CAacDecoderChannelInfo** pAacDecoderChannelInfo,
                           CAacDecoderStaticChannelInfo** pAacDecoderStaticChannelInfo,
                           SamplingRateInfo* samplingRateInfo, const UINT* MCT_elFlags,
                           const UINT usacIndepFrame);

void CMct_StereoFilling_save_prev(CMctPtr self, CAacDecoderChannelInfo** pAacDecoderChannelInfo);

void CMct_StereoFilling_clear_prev(CMctPtr self, CAacDecoderChannelInfo** pAacDecoderChannelInfo);

#ifdef __cplusplus
}
#endif

#endif /* __STEREO_H__ */
