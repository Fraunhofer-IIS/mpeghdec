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

   Author(s):   Josef Hoepfl

   Description: individual channel stream info

*******************************************************************************/

#include "channelinfo.h"
#include "aac_rom.h"
#include "aac_ram.h"
#include "FDK_bitstream.h"

AAC_DECODER_ERROR IcsReadMaxSfb(HANDLE_FDK_BITSTREAM bs, CIcsInfo* pIcsInfo,
                                const SamplingRateInfo* pSamplingRateInfo) {
  AAC_DECODER_ERROR ErrorStatus = AAC_DEC_OK;
  int nbits;

  if (IsLongBlock(pIcsInfo)) {
    nbits = 6;
    pIcsInfo->TotalSfBands = pSamplingRateInfo->NumberOfScaleFactorBands_Long;
  } else {
    nbits = 4;
    pIcsInfo->TotalSfBands = pSamplingRateInfo->NumberOfScaleFactorBands_Short;
  }
  pIcsInfo->MaxSfBands = (UCHAR)FDKreadBits(bs, nbits);

  if (pIcsInfo->MaxSfBands > pIcsInfo->TotalSfBands) {
    ErrorStatus = AAC_DEC_PARSE_ERROR;
  }

  return ErrorStatus;
}

AAC_DECODER_ERROR IcsRead(HANDLE_FDK_BITSTREAM bs, CIcsInfo* pIcsInfo,
                          const SamplingRateInfo* pSamplingRateInfo, const UINT flags) {
  AAC_DECODER_ERROR ErrorStatus = AAC_DEC_OK;

  pIcsInfo->Valid = 0;

  {
    if (!(flags & (AC_USAC | AC_RSVD50 | AC_MPEGH3DA))) {
      FDKreadBit(bs);
    }
    pIcsInfo->WindowSequence = (BLOCK_TYPE)FDKread2Bits(bs);
    pIcsInfo->WindowShape = (UCHAR)FDKreadBit(bs);
  }

  ErrorStatus = IcsReadMaxSfb(bs, pIcsInfo, pSamplingRateInfo);
  if (ErrorStatus != AAC_DEC_OK) {
    goto bail;
  }

  if (IsLongBlock(pIcsInfo)) {
    if (!(flags & (AC_ELD | AC_SCALABLE | AC_BSAC | AC_USAC | AC_RSVD50 |
                   AC_MPEGH3DA))) /* If not ELD nor Scalable nor BSAC nor USAC syntax then ... */
    {
      if ((UCHAR)FDKreadBit(bs) != 0) /* UCHAR PredictorDataPresent */
      {
        ErrorStatus = AAC_DEC_UNSUPPORTED_PREDICTION;
        goto bail;
      }
    }

    pIcsInfo->WindowGroups = 1;
    pIcsInfo->WindowGroupLength[0] = 1;
  } else {
    INT i;
    UINT mask;

    pIcsInfo->ScaleFactorGrouping = (UCHAR)FDKreadBits(bs, 7);

    pIcsInfo->WindowGroups = 0;

    for (i = 0; i < (8 - 1); i++) {
      mask = 1 << (6 - i);
      pIcsInfo->WindowGroupLength[i] = 1;

      if (pIcsInfo->ScaleFactorGrouping & mask) {
        pIcsInfo->WindowGroupLength[pIcsInfo->WindowGroups]++;
      } else {
        pIcsInfo->WindowGroups++;
      }
    }

    /* loop runs to i < 7 only */
    pIcsInfo->WindowGroupLength[8 - 1] = 1;
    pIcsInfo->WindowGroups++;
  }

bail:
  if (ErrorStatus == AAC_DEC_OK) pIcsInfo->Valid = 1;

  return ErrorStatus;
}

/*
  interleave codebooks the following way

    9 (84w) |  1 (51w)
   10 (82w) |  2 (39w)
  SCL (65w) |  4 (38w)
    3 (39w) |  5 (41w)
            |  6 (40w)
            |  7 (31w)
            |  8 (31w)
     (270w)     (271w)
*/

/*
  Table entries are sorted as following:
  | num_swb_long_window | sfbands_long | num_swb_short_window | sfbands_short |
*/
AAC_DECODER_ERROR getSamplingRateInfo(SamplingRateInfo* t, UINT samplesPerFrame,
                                      UINT samplingRateIndex, UINT samplingRate) {
  int index = 0;

  /* Search closest samplerate according to ISO/IEC 13818-7:2005(E) 8.2.4 (Table 38): */
  if ((samplingRateIndex >= 15)) {
    const UINT borders[] = {(UINT)-1, 92017, 75132, 55426, 46009, 37566,
                            27713,    23004, 18783, 13856, 11502, 9391};
    UINT i, samplingRateSearch = samplingRate;

    for (i = 0; i < 11; i++) {
      if (borders[i] > samplingRateSearch && samplingRateSearch >= borders[i + 1]) {
        break;
      }
    }
    samplingRateIndex = i;
  }

  t->samplingRateIndex = samplingRateIndex;
  t->samplingRate = samplingRate;

  switch (samplesPerFrame) {
    case 1024:
      index = 0;
      break;

    default:
      return AAC_DEC_UNSUPPORTED_FORMAT;
  }

  t->ScaleFactorBands_Long = sfbOffsetTables[index][samplingRateIndex].sfbOffsetLong;
  t->ScaleFactorBands_Short = sfbOffsetTables[index][samplingRateIndex].sfbOffsetShort;
  t->NumberOfScaleFactorBands_Long = sfbOffsetTables[index][samplingRateIndex].numberOfSfbLong;
  t->NumberOfScaleFactorBands_Short = sfbOffsetTables[index][samplingRateIndex].numberOfSfbShort;

  if (t->ScaleFactorBands_Long == NULL || t->NumberOfScaleFactorBands_Long == 0) {
    t->samplingRate = 0;
    return AAC_DEC_UNSUPPORTED_FORMAT;
  }

  FDK_ASSERT((UINT)t->ScaleFactorBands_Long[t->NumberOfScaleFactorBands_Long] == samplesPerFrame);
  FDK_ASSERT(t->ScaleFactorBands_Short == NULL ||
             (UINT)t->ScaleFactorBands_Short[t->NumberOfScaleFactorBands_Short] * 8 ==
                 samplesPerFrame);

  return AAC_DEC_OK;
}
