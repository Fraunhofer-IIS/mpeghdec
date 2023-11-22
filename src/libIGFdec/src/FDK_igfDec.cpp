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

/********************** Intelligent gap filling library ************************

   Author(s):   Youliy Ninov, Andreas Niedermeier

   Description: Intelligent gap filling library

*******************************************************************************/

#include "FDK_igfDec.h"

#if defined(__arm__)
#include "arm/FDK_igfDec_arm.cpp"
#endif

/* Decoder library info */
#define IGF_DECODER_LIB_VL0 1
#define IGF_DECODER_LIB_VL1 0
#define IGF_DECODER_LIB_VL2 0
#define IGF_DECODER_LIB_TITLE "IGF library"
#ifdef __ANDROID__
#define IGF_DECODER_LIB_BUILD_DATE ""
#define IGF_DECODER_LIB_BUILD_TIME ""
#else
#define IGF_DECODER_LIB_BUILD_DATE __DATE__
#define IGF_DECODER_LIB_BUILD_TIME __TIME__
#endif

#define MAX_SPECTRUM_LEN 1024

#define IGF_SCF_REDUCTION_FAC_FOR_TCX_MDCT 20

/* This macro switches to a very fast processing inside the function */
#define USE_NEW_iisIGFDecLibInjectSourceSpectrumTCX_func1

static LONG randomSign(ULONG* seed) {
  LONG sign = 0;
  *seed = (ULONG)(((UINT64)(*seed) * 69069) + 5);
  if (((*seed) & 0x10000) > 0) {
    sign = -1;
  } else {
    sign = +1;
  }
  return sign;
}

static void iisIGFDecLibInjectSourceSpectrum(
    IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticData, IGF_PRIVATE_DATA_HANDLE hPrivateData,
    FIXP_DBL* pSpectralData,                     /**< ptr to spectreal window data       */
    SHORT* specScale, const INT window_sequence, /**< in: 1==SHORT 0==LONG window seq    */
    const INT frameType);

static INT get_IGF_tile(const INT igfMin, const INT sfbBgn, const INT sfbEnd, const INT bUseHighRes,
                        const SHORT* swb_offset, const INT swb_shift, INT* tile) {
  SHORT igfBgn = swb_offset[sfbBgn] << swb_shift;
  SHORT igfEnd = (SHORT)fMin((UINT)1024, (UINT)(swb_offset[sfbEnd] << swb_shift));
  INT mem = sfbBgn;
  INT igfRange = fMax(8, igfEnd - igfBgn);
  INT i, j;
  INT sbs = igfBgn;
  INT nTr = 0;

  for (i = 0; i < 4; i++) {
    tile[i] = igfRange >> 2;
  }

  if (igfBgn > igfMin && igfBgn < igfEnd) {
    for (i = 0; i < 4; i++) {
      j = 0;
      do {
        j++;
      } while (fMin(sbs + tile[i], (INT)igfEnd) > (swb_offset[j] << swb_shift));

      if (!bUseHighRes) {
        if ((((j - sfbBgn) & 1) == 1) && ((j - mem) > 1) && (i < 3)) {
          j--;
        } else if (i == 3) {
          j = sfbEnd;
        }
        mem = j;
      }

      tile[i] = fMax(2, fMin((swb_offset[j] << swb_shift) - sbs, igfEnd - sbs));

      FDK_ASSERT(tile[i] <= IGF_MAX_TILE_WIDTH);

      sbs += tile[i];
      nTr++;
      if (sbs == igfEnd) {
        break;
      }
    }
  }
  for (i = nTr; i < 4; i++) {
    tile[i] = 0;
  }
  return nTr;
}

/* ######################################################################*/
/* ######################### public functions  ##########################*/
/* ######################################################################*/

/*********************************************************************
initialization of an instance of this module, pass a ptr to a hInstance
\return returns an error code
**************************************************************************/
static void iisIGFDecLibInitGridInfo(FIXP_DBL* Spectrum_tab_array, IGF_GRID_INFO_HANDLE hGridInfo,
                                     const INT window_sequence, const SHORT* sfb_offset,
                                     const INT sfb_offset_len, const INT sfbStart,
                                     const INT sfbStop, const INT igfMin, const INT useHighRes,
                                     const INT nT, INT* tile, UCHAR* tileIdx) {
  IGF_MAP_INFO_HANDLE hMapInfo = NULL;
  SHORT igfBgn;
  INT t, sfbStep;
  INT swb_shift;

  swb_shift = 0;

  igfBgn = sfb_offset[sfbStart] << swb_shift;

  hGridInfo->iIGFNumSfb = sfbStop - sfbStart;
  hGridInfo->iIGFStartSfb = sfbStart;
  hGridInfo->iIGFStopSfb = sfbStop;
  hGridInfo->iIGFStartLine = igfBgn;
  hGridInfo->iIGFStopLine = (SHORT)fMin((UINT)1024, (UINT)(sfb_offset[sfbStop] << swb_shift));
  hGridInfo->iIGFMinLine = igfMin;

  /* Compute tile begins */
  int TileBgn[5] = {igfBgn, 0, 0, 0, 0};

  /* Calculate offsets from tile begin to mapped field from base spectrum */
  for (int j = 0; j < nT; j++) {
    TileBgn[j + 1] = TileBgn[j] + tile[j];

    /* Initialize tile mapping index tables */
    hMapInfo = &(hGridInfo->sIGFMapInfoTab[j]);
    hMapInfo->iDesStart = TileBgn[j];
    hMapInfo->iWidth = tile[j];
    hMapInfo->iIdx = j;
    hMapInfo->fSpectrumTab[0] = &Spectrum_tab_array[j * 1024];
  } /* for(int j=0;j<nT;j++) */

  /* Aggregate tile and sfb info */
  const SHORT* p2_OffsetTable = &sfb_offset[sfbStart];
  sfbStep = 1;
  if ((useHighRes == 0) && (IGF_GRID_LONG_WINDOW == window_sequence)) {
    sfbStep = 2;
  }
  /* No pairing case */
  if (sfbStep == 1) {
    for (int i = 0; i < nT; i++) {
      int sfb_count = 0;
      int tileUpperEdge = TileBgn[i + 1];
      hMapInfo = &(hGridInfo->sIGFMapInfoTab[i]);
      do {
        SHORT sfb_width = (p2_OffsetTable[1] << swb_shift) - (p2_OffsetTable[0] << swb_shift);
        hMapInfo->iSfbWidthTab[sfb_count++] = sfb_width;
        p2_OffsetTable++;
      } while (tileUpperEdge > (*p2_OffsetTable << swb_shift));

      hMapInfo->iSfbCnt = sfb_count;
      hMapInfo->iSfbSplit = 0;
    } /* for(i=0;i<nT;i++) */
  }
  /* Pairing case, i.e. sfbStep = 2. Must be checked if some tile boundary occurs in the middle of a
  sfb-pair. If yes, then sfb-pair splitting is signalled */
  else {
    int sfb_jump = sfbStep;
    int split_flag = 0;
    for (int i = 0; i < nT; i++) {
      int sfb_count = 0;
      SHORT sfb_width;
      int tileUpperEdge = TileBgn[i + 1];
      hMapInfo = &(hGridInfo->sIGFMapInfoTab[i]);
      do {
        sfb_width = (p2_OffsetTable[sfb_jump] << swb_shift) - (p2_OffsetTable[0] << swb_shift);
        hMapInfo->iSfbWidthTab[sfb_count++] = sfb_width;
        p2_OffsetTable += sfb_jump;
        /*Introduced to prevent an illegal memory access when
        the end of the table is reached (p2_OffsetTable[1])*/
        if (p2_OffsetTable[0] >= hGridInfo->iIGFStopLine) break;
        if (p2_OffsetTable[1] == TileBgn[nT]) {
          sfb_jump = 1;
        } else {
          sfb_jump = sfbStep;
        }
      } while (tileUpperEdge > (*p2_OffsetTable << swb_shift));

      hMapInfo->iSfbCnt = sfb_count;
      if (split_flag == 0)
        hMapInfo->iSfbSplit = 0;
      else {
        hMapInfo->iSfbSplit = 1;
        split_flag = 0;
      }
      /* If tile boundary is in the middle of the pair signal sfb-splitting */
      if ((*p2_OffsetTable << swb_shift) > tileUpperEdge) {
        hMapInfo->iSfbWidthTab[--sfb_count] =
            sfb_width - ((p2_OffsetTable[0] << swb_shift) - (p2_OffsetTable[-1] << swb_shift));
        split_flag = 1;
        p2_OffsetTable--;
        sfb_jump = 1;
      }
    } /* for(i=0;i<nT;i++) */
  }

  hGridInfo->iIGFNumTile = nT;
  for (t = 0; t < nT; t++) {
    hGridInfo->iIGFTileIdxTab[t] = tileIdx[t];
  }
}

/****************************************/
static void iisIGFDecLibResetIGF(IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticData,
                                 IGF_PRIVATE_DATA_HANDLE hPrivateData, const INT frameType) {
  int i;

  for (i = 0; i < IGF_MAX_TILES; i++) {
    hPrivateData->bitstreamData[frameType].iCurrTileIdxTab[i] = 3;
    hPrivateData->bitstreamData[frameType].igfUseEnfFlat = 0;
    hPrivateStaticData->prevPatchNum[i] = 3;
    hPrivateData->bitstreamData[frameType].igfWhiteningLevel[i] = WHITENING_MID;
    hPrivateStaticData->igfWhiteningLevelPrev[i] = WHITENING_MID;
  }
}
/**********************************************************************/ /**
 implements the decoding of IGF
 **************************************************************************/
int iisIGFDecLibReadIGF(IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticData,
                        IGF_PRIVATE_DATA_HANDLE hPrivateData, HANDLE_FDK_BITSTREAM hBStr,
                        const INT window_sequence,       /**< in: 1==SHORT 0==LONG window seq */
                        const INT bUsacIndependencyFlag, /**< in: USAC independency flag */
                        const INT igfUseEnf, const INT frameType) {
  INT i, nTiles;
  INT igFUsePrevTile;
  int retCode = 0;

  if (hPrivateData->bitstreamData[frameType].igf_allZero) {
    iisIGFDecLibResetIGF(hPrivateStaticData, hPrivateData, frameType);
    return (retCode);
  }

  nTiles = hPrivateStaticData->sGridInfoTab[window_sequence].iIGFNumTile;

  {
    /* read igfUsePrevTile */
    igFUsePrevTile = bUsacIndependencyFlag ? 0 : FDKreadBit(hBStr);
    /* read all igfTileNum */
    if (igFUsePrevTile) {
      for (i = 0; i < nTiles; i++) {
        hPrivateData->tileNum[i] = hPrivateStaticData->prevPatchNum[i];
      }
    } else {
      for (i = 0; i < nTiles; i++) {
        hPrivateData->tileNum[i] = FDKreadBits(hBStr, 2);
      }
    }
  }

  for (i = 0; i < nTiles; i++) {
    hPrivateStaticData->prevPatchNum[i] = hPrivateData->tileNum[i];
    hPrivateData->bitstreamData[frameType].igfWhiteningLevel[i] = WHITENING_MID;
  }

  for (; i < IGF_MAX_TILES; i++) {
    hPrivateStaticData->prevPatchNum[i] = 3;
    hPrivateData->bitstreamData[frameType].igfWhiteningLevel[i] = WHITENING_MID;
  }

  if (hPrivateStaticData->igfUseWhitening && (window_sequence != IGF_GRID_SHORT_WINDOW)) {
    INT igFUsePrevWhiteningLevel;

    igFUsePrevWhiteningLevel = bUsacIndependencyFlag ? 0 : FDKreadBit(hBStr);

    if (igFUsePrevWhiteningLevel) {
      for (i = 0; i < nTiles; i++) {
        hPrivateData->bitstreamData[frameType].igfWhiteningLevel[i] =
            hPrivateStaticData->igfWhiteningLevelPrev[i];
      }
    } else {
      INT level;

      level = FDKreadBit(hBStr);

      if (level) {
        level = FDKreadBit(hBStr);
        if (level)
          hPrivateData->bitstreamData[frameType].igfWhiteningLevel[0] = WHITENING_STRONG;
        else
          hPrivateData->bitstreamData[frameType].igfWhiteningLevel[0] = WHITENING_OFF;
      } else
        hPrivateData->bitstreamData[frameType].igfWhiteningLevel[0] = WHITENING_MID;

      if (FDKreadBit(hBStr)) {
        for (i = 1; i < nTiles; i++) {
          level = FDKreadBit(hBStr);

          if (level) {
            level = FDKreadBit(hBStr);
            if (level)
              hPrivateData->bitstreamData[frameType].igfWhiteningLevel[i] = WHITENING_STRONG;
            else
              hPrivateData->bitstreamData[frameType].igfWhiteningLevel[i] = WHITENING_OFF;
          } else
            hPrivateData->bitstreamData[frameType].igfWhiteningLevel[i] = WHITENING_MID;
        }
      } else {
        for (i = 1; i < nTiles; i++) {
          hPrivateData->bitstreamData[frameType].igfWhiteningLevel[i] =
              hPrivateData->bitstreamData[frameType].igfWhiteningLevel[0];
        }
      }
    }

  } /*  if(hPrivateStaticData->igfUseWhitening ... */

  for (i = 0; i < IGF_MAX_TILES; i++) {
    hPrivateStaticData->igfWhiteningLevelPrev[i] =
        hPrivateData->bitstreamData[frameType].igfWhiteningLevel[i];
  }

  for (i = 0; i < nTiles; i++) {
    /* map the tile indices to the current tile tab: */
    hPrivateData->bitstreamData[frameType].iCurrTileIdxTab[i] = hPrivateData->tileNum[i];
  }

  /* decode_temp_flattening_trigger */
  if (igfUseEnf && (window_sequence != IGF_GRID_SHORT_WINDOW)) {
    hPrivateData->bitstreamData[frameType].igfUseEnfFlat = FDKreadBit(hBStr);
  }

  return (retCode);
}

/*********************************************************************
resets the internal decoder memory (context memory)
**************************************************************************/
static void iisIGFDecLibResetSCF(IGF_PRIVATE_STATIC_DATA* hPrivateStaticData,
                                 IGF_PRIVATE_DATA* hPrivateData, const INT frameType) {
  FDKmemclear(hPrivateData->bitstreamData[frameType].sfe,
              sizeof(hPrivateData->bitstreamData[frameType].sfe));

  iisIGFSCFDecoderReset(&(hPrivateStaticData->hIGFSCFDecoder));
}

/*********************************************************************
implements the decoding of SCF
**************************************************************************/
void iisIGFDecLibReadSCF(
    IGF_PRIVATE_STATIC_DATA* hPrivateStaticData, IGF_PRIVATE_DATA* hPrivateData,
    HANDLE_FDK_BITSTREAM hBStr,
    const INT indepFlag,  /**< in: if  1 on input the encoder will be forced to reset
                      if  0 on input the encodder will be forced to encode without a reset */
    const INT igfWinMode, /**< in: DEC_IGF_GRID enum */
    const INT winCount,   /**< in: number of windows in the group */
    const INT frameType) {
  INT sfeReducePrecisionEnabled;
  INT allZero = 0;
  INT winFlag = 0;

  sfeReducePrecisionEnabled =
      (igfWinMode <= IGF_GRID_SHORT_WINDOW) ? !hPrivateStaticData->useHighRes : 0;

  allZero = FDKreadBit(hBStr);

  hPrivateData->bitstreamData[frameType].igf_allZero = allZero;

  winFlag =
      iisIGFSCFDecoderGetLastFrameWasShortBlock(&hPrivateStaticData->hIGFSCFDecoder) != igfWinMode;

  if (allZero || indepFlag || winFlag) {
    iisIGFDecLibResetSCF(hPrivateStaticData, hPrivateData, frameType);
  }

  if (!allZero) {
    iisIGFSCFDecoderDecode(&(hPrivateStaticData->hIGFSCFDecoder), hBStr,
                           hPrivateData->bitstreamData[frameType].sfe, indepFlag, igfWinMode,
                           winCount, sfeReducePrecisionEnabled);
  }

  /* store actual window sequence */
  iisIGFSCFDecoderSetLastFrameWasShortBlock(&hPrivateStaticData->hIGFSCFDecoder, igfWinMode);
}

/*********************************************************************/ /**

 *************************************************************************/

void iisIGF_Sync_Data(IGF_PRIVATE_DATA_HANDLE hPrivateDataL,
                      IGF_PRIVATE_DATA_HANDLE hPrivateDataR) {
  UCHAR igf_allZeroL;
  UCHAR igf_allZeroR;

  for (int i = 0; i < N_IGF_FRAME_DIVISION; i++) {
    igf_allZeroL = hPrivateDataL->bitstreamData[i].igf_allZero;
    igf_allZeroR = hPrivateDataR->bitstreamData[i].igf_allZero;

    if (1 == igf_allZeroL && 0 == igf_allZeroR) {
      for (int t = 0; t < 4; t++) {
        hPrivateDataL->bitstreamData[i].iCurrTileIdxTab[t] =
            hPrivateDataR->bitstreamData[i].iCurrTileIdxTab[t];
        hPrivateDataL->bitstreamData[i].igfWhiteningLevel[t] =
            hPrivateDataR->bitstreamData[i].igfWhiteningLevel[t];
      }
      hPrivateDataL->bitstreamData[i].igfUseEnfFlat = hPrivateDataR->bitstreamData[i].igfUseEnfFlat;

    } else if (0 == igf_allZeroL && 1 == igf_allZeroR) {
      for (int t = 0; t < 4; t++) {
        hPrivateDataR->bitstreamData[i].iCurrTileIdxTab[t] =
            hPrivateDataL->bitstreamData[i].iCurrTileIdxTab[t];
        hPrivateDataR->bitstreamData[i].igfWhiteningLevel[t] =
            hPrivateDataL->bitstreamData[i].igfWhiteningLevel[t];
      }
      hPrivateDataR->bitstreamData[i].igfUseEnfFlat = hPrivateDataL->bitstreamData[i].igfUseEnfFlat;

    } /* if (1 == igf_allZeroL && 0 == igf_allZeroR) */
  }
}

/**********************************************************************/ /**

  **************************************************************************/
static void getWhiteSpectralData(FIXP_DBL* in, FIXP_DBL* out, FIXP_DBL* working_array,
                                 IGF_WORK_MEMORY_HANDLE hWorkMem, const INT start, const INT stop) {
  INT shl1;  /* scale factor to left-align input data from in[(start-3)...(stop-1)] */
  INT shr1;  /* scale factor to right-align energies before accumulation            */
  ULONG env; /* current average of data in acc[0..6], each shifted right by shr1.   */
  INT n_min;

  /* Find available headroom in input signal */
  shl1 = getScalefactor(&in[start - 3], stop - start + 3);

  /* Generate 1e-3 in Q-10.41 */
  env = (ULONG)0x83126E98;        /* 0x4189374c<<1 */
  const INT value0_001_exp = -10; /* This exponent is even on purpose */

  /* Find the (even) exponent of the averaged env[i] variable */
  INT shift_energy = 2; /* Minimum shift amount for energy accumulation: 2, max energy:
                           7*0x2000.0000 = 0xE000.0000 (for -1.0f) */
  INT exp_env = ((hWorkMem->Initial_exp[0] - shl1) << 1) + shift_energy;

  INT delta = exp_env - value0_001_exp; /* delta is still even */
  if (delta > 0) {
    /* Samples have higher exponent than 1e-3, when scaled left, we adapt the env initial value
     * accordingly */
    if (delta <= 31)
      env >>= delta;
    else
      env = (ULONG)0;
  } else {
    /* Sample energies have lower (or same) exponent than 1e-3, when shifted by 2, we add a positive
     * (or zero) shift-right delta */
    shift_energy -= delta;
    exp_env -= delta; /* exp_env = -10 (even, same as default env) */
  }

  switch (exp_env) {
    case -10:
      /* Worst case scenario for in[i]=1.0f: env_max = 7*0x0800.0000 + 0x8312.6E98 = 0xBB12.6E98 */
      /* Least case scenario for in[i]=0.0f: env_min = 7*0x0000.0000 + 0x8312.6E98 = 0x8312.6E98 */
      /* Normalization and shift in range 0 */
      if (shift_energy == 2) /* We need a shift-right by 4 */
      {
        /* Worst case scenario for in[i]=1.0f: env_max = 7*0x0800.0000 + 0x20C4.9BA6 = 0x58C4.9BA6
         */
        /* Least case scenario for in[i]=0.0f: env_min = 7*0x0000.0000 + 0x20C4.9BA6 = 0x20C4.9BA6
         */
        /* Normalization and shift in range 1..2 */
        shift_energy = 4;
        env >>= 2;
        exp_env += 2; /* exp_env: -8 */
      }
      break;
    case -8:
      /* Worst case scenario for in[i]=1.0f: env_max = 7*0x0800.0000 + 0x20C4.9BA6 = 0x58C4.9BA6 */
      /* Least case scenario for in[i]=0.0f: env_min = 7*0x0000.0000 + 0x20C4.9BA6 = 0x20C4.9BA6 */
      /* Normalization and shift in range 1..2 */
      if (shift_energy == 2) /* We need a shift-right by 4 */
      {
        /* Worst case scenario for in[i]=1.0f: env_max = 7*0x0800.0000 + 0x0831.26E9 = 0x4031.26E9
         */
        /* Least case scenario for in[i]=0.0f: env_min = 7*0x0000.0000 + 0x0831.26E9 = 0x0831.26E9
         */
        /* Normalization and shift in range 0..4 */
        shift_energy = 4;
        env >>= 2;
        exp_env += 2; /* exp_env: -6 */
      }
      break;
    default: /* [-6, -4, .. +??]*/
      /* Worst case scenario for in[i]=1.0f: env_max = 7*0x2000.0000 + 0x0831.26E9 = 0xE831.26E9 @
       * -6 */
      /* shift_energy = 2   !!! already set !!! */
      /* Normalization and shift in range 0..4   exp_env= -6 */
      /* Normalization and shift in range 0..6   exp_env= -4 */
      /* Normalization and shift in range 0..8   exp_env= -2 */
      /* Normalization and shift in range 0..10  exp_env=  0 */
      /* Normalization and shift in range 0..12  exp_env=  2 */
      /* Normalization and shift in range 0..14  exp_env=  4 */
      /* Normalization and shift in range 0..16  exp_env=  6 */
      /* Normalization and shift in range 0..18  exp_env=  8 */
      /* Normalization and shift in range 0..20  exp_env= 10 */
      /* Normalization and shift in range 0..22  exp_env= 12 */
      /* Normalization and shift in range 0..24  exp_env= 14 */
      /* Normalization and shift in range 0..26  exp_env= 16 */
      /* Normalization and shift in range 0..28  exp_env= 18 */
      /* Normalization and shift in range 0..30  exp_env= 20 */
      /* Normalization and shift in range 0..32  exp_env= 22 or higher */
      break;
  }

    /* shr1 (limited to 31) is used to scale right the energy values. With shr1=31, each energy
     * becomes 0x0000.0000 */
    /* Subtract 1 due to use of fPow2Div2 instead of fPow2 because of issues with fract class tests
     */

#ifdef FUNCTION_getWhiteSpectralData_func1
  shr1 = fMin(31, shift_energy);
  n_min = getWhiteSpectralData_func1(in, working_array, start, stop, shl1, shr1, env, exp_env);
#else
  shr1 = fMin(31, shift_energy) - 1;
  ULONG acc[8]; /* accumulator for 7 sliding energy values plus initial value 0.001f   */
                /* On DSP, it can be implemented as register set also.                 */

  INT idx = 0; /* index to refer to next data of array acc */
  INT i;

  /* Prolog: calculate energy average using first 7 samples */
  for (i = (start - 3); i < (start + 4); i++) {
    ULONG val_in = (((ULONG)(INT)fPow2Div2(in[i] << shl1)) >> shr1);
    acc[idx++] = val_in;
    env += val_in;
  }
  /* acc[0..6] hold last 7 energy values, idx = 7 */

  INT* p1 = (INT*)working_array;

  n_min = 1000;
  /* In this loop, we evaluate the factors (shr2) to apply for the output, that is written in a
   * second run */
  for (i = start; i < (stop - 4); i++) {
    /* compute output sample: shr2 in range 0..64 */
    INT shr2 = exp_env - fNormz((FIXP_DBL)(INT)env);
    if (in[i] != (FIXP_DBL)0) {
      n_min = fMin(n_min, shr2);
    }
    *p1++ = shr2;

    /* Update env and acc[],idx with next sample */
    ULONG val_in = (((ULONG)(INT)fPow2Div2(in[i + 4] << shl1)) >> shr1);
    acc[idx++] = val_in;
    idx &= 0x7; /* idx runs 0...7, 0...7 */
    env = env - acc[idx] + val_in;
  }

  /* Epilog: compute last 4 output samples using 4x latest env */
  for (; i < stop; i++) {
    /* compute output sample: shr2 in range 0..64 */
    INT shr2 = exp_env - fNormz((FIXP_DBL)(INT)env);
    if (in[i] != (FIXP_DBL)0) n_min = fMin(n_min, shr2);
    *p1++ = shr2;
  }
#endif

  /* Check, if all in[i] are zero, ... */
  if (n_min == 1000) {
    /* Obviously all in[i] are ZERO, let's quit after clearing the output */
    FDKmemclear(&out[start], sizeof(FIXP_DBL) * (stop - start));
    hWorkMem->Initial_exp[0] = 0;
  } else {
    /* make n_min even */
    n_min = (n_min >> 1) << 1;
    INT* p2 = (INT*)working_array;

    /* Calclulate the output data exponent */
    hWorkMem->Initial_exp[0] += (SCHAR)(21 - shl1 - (n_min >> 1));

#ifdef FUNCTION_getWhiteSpectralData_func2
    getWhiteSpectralData_func2(out, in, p2, start, stop, shl1, n_min);
#else
    const FIXP_DBL constInvSqrt2 = 0x5A82799A; /* 1/sqrt(2)=0.70710678 => 0x5a82799A in Q0.31;*/
    for (INT j = start; j < stop; j++) {
      /* compute output sample: shr2 in range 0..62 */
      INT shr2 = fMax(0, *p2++ - n_min);
      FIXP_DBL val_in = in[j] << shl1;
      if (shr2 & 1) val_in = fMult(val_in, constInvSqrt2);
      val_in = val_in >> (shr2 >> 1);
      out[j] = val_in;
    }
#endif
  }
}

/**********************************************************************/ /**
   initalization of energy values
  **************************************************************************/
static void iisIGFDecLibInitFractScaleValues(IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticData,
                                             IGF_PRIVATE_DATA_HANDLE hPrivateData,
                                             const INT window_sequence) {
  IGF_GRID_INFO_HANDLE hGrid;
  IGF_WORK_MEMORY_HANDLE hWorkMem;
  INT tile;

  /* get the grid structure: */
  hGrid = &hPrivateStaticData->sGridInfoTab[window_sequence];

  int MaxWin = 1;
  if (window_sequence == IGF_GRID_SHORT_WINDOW) MaxWin = 8;

  /* over all tiles: */
  for (tile = 0; tile < hGrid->iIGFNumTile; tile++) {
    /* get the mapping structure: */
    hWorkMem = &(hPrivateData->IGF_Common_channel_data_handle->IGF_WorkingMem[tile]);

    SHORT* pSpecScaleTile = iisIGFDecLibAccessSourceSpectrum_Scale(hPrivateStaticData, hPrivateData,
                                                                   tile, window_sequence);

    SCHAR value;
    for (int win = 0; win < MaxWin; win++) {
      value = (SCHAR)pSpecScaleTile[win];
      hWorkMem->Initial_exp[win * IGF_MAX_SFB_SHORT] = value;
    }
  }
}

/**********************************************************************/ /**
  calculation of energies
  **************************************************************************/
static void iisIGFDecoderApplyWhitening(
    IGF_PRIVATE_DATA_HANDLE hPrivateData, IGF_GRID_INFO_HANDLE hGrid,
    FIXP_DBL* pSpectralData, /**< ptr to spectreal window data */
    FIXP_DBL* working_array, ULONG* randomSeed, UCHAR flag_INF_active, UCHAR mono_or_stereo_flag) {
  IGF_MAP_INFO_HANDLE hMap;
  IGF_WORK_MEMORY_HANDLE hWorkMem;
  INT tile;
  INT tb;
  INT width;
  INT run_once = 0;
  INT keep_tile_num = 0;
  INT keep_initial_value = 0;

  /* over all tiles: */
  for (tile = 0; tile < hGrid->iIGFNumTile; tile++) {
    /* get the mapping structure: */
    hMap = &hGrid->sIGFMapInfoTab[tile];
    hWorkMem = &(hPrivateData->IGF_Common_channel_data_handle->IGF_WorkingMem[tile]);

    if (WHITENING_MID == hMap->iWhiteningLevel) {
      /* If INF is active then all tiles are whitened independently. If, however, INF is not active,
       * then the first tile is whitened and copied to the rest */
      if (flag_INF_active) {
        getWhiteSpectralData(hMap->fSpectrumTab[0], hMap->fSpectrumTab[0], working_array, hWorkMem,
                             hGrid->iIGFMinLine, hGrid->iIGFStartLine);
      } else {
        if (run_once == 0) {
          getWhiteSpectralData(pSpectralData, hMap->fSpectrumTab[0], working_array, hWorkMem,
                               hGrid->iIGFMinLine, hGrid->iIGFStartLine);
          keep_initial_value = hWorkMem->Initial_exp[0];
          keep_tile_num = tile;
          run_once = 1;
        } else { /* when run once, then just copy  for the other tiles */
          FDKmemcpy(hMap->fSpectrumTab[0], (&hGrid->sIGFMapInfoTab[keep_tile_num])->fSpectrumTab[0],
                    sizeof(FIXP_DBL) * (hGrid->iIGFStartLine));
          hWorkMem->Initial_exp[0] = keep_initial_value;
        }
      }
    }
    if (WHITENING_STRONG == hMap->iWhiteningLevel) {
      /* Define a constant */
      /*2097152 is 0x4000000 in Q22.9. Maximum left alligned */
      hWorkMem->Initial_exp[0] = 22;

      /* first target line: */
      tb = hMap->iDesStart;
      /* width of the current tile: */
      width = hMap->iWidth;

      FIXP_DBL* p2_tile = hMap->fSpectrumTab[0];
      FDKmemclear(p2_tile, sizeof(FIXP_DBL) * width);

      /* The RNG acts differently for mono and stereo */
      if (mono_or_stereo_flag == 0) {
        for (int i = 0; i < width; i++) {
          if ((FIXP_DBL)0 == pSpectralData[tb]) {
            LONG rr = randomSign(randomSeed);
            *p2_tile = FIXP_DBL(rr * 0x40000000);
          }
          p2_tile++;
          tb++;
        }

      } else {
        for (int i = 0; i < width; i++) {
          LONG rr = randomSign(randomSeed);
          if ((FIXP_DBL)0 == pSpectralData[tb]) {
            *p2_tile = FIXP_DBL(rr * 0x40000000);
          }
          p2_tile++;
          tb++;
        }
      }

    } /* end else if( WHITENING_STRONG == hMap->iWhiteningLevel ) */

  } /* end for ( tile = 0.... */
}

static void iisIGFDecoderCollectEnergiesMonoNormalize(
    IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticData, IGF_PRIVATE_DATA_HANDLE hPrivateData,
    const INT window_sequence, /**< in: 1==SHORT 0==LONG window seq */
    const UCHAR groupLength, const INT start_win) {
  IGF_GRID_INFO_HANDLE hGrid;
  IGF_MAP_INFO_HANDLE hMap;
  IGF_WORK_MEMORY_HANDLE hWorkMem;
  INT tile;
  INT sfb;
  FIXP_DBL sE, tE, acc;
  INT temp_int;
  INT s_shift;
  INT corrected_shift;

  if (window_sequence == IGF_GRID_SHORT_WINDOW) { /*  Only for short windows */

    if (groupLength == 1) { /* IF "1" then copy */

      /* get the grid structure: */
      hGrid = &hPrivateStaticData->sGridInfoTab[window_sequence];

      /* over all tiles: */
      for (tile = 0; tile < hGrid->iIGFNumTile; tile++) {
        /* get the mapping structure: */
        hMap = &hGrid->sIGFMapInfoTab[tile];
        hWorkMem = &(hPrivateData->IGF_Common_channel_data_handle->IGF_WorkingMem[tile]);

        /* over all sfb of tile: */
        for (sfb = 0; sfb < hMap->iSfbCnt; sfb++) {
          hWorkMem->sE[sfb] = hWorkMem->fSfbSurvivedEnergyTab[start_win * IGF_MAX_SFB_SHORT + sfb];
          hWorkMem->sE_exp[sfb] =
              hWorkMem->fSfbSurvivedEnergyTab_exp[start_win * IGF_MAX_SFB_SHORT + sfb];
          hWorkMem->tE[sfb] = hWorkMem->fSfbTileEnergyTab[start_win * IGF_MAX_SFB_SHORT + sfb];
          hWorkMem->tE_exp[sfb] =
              hWorkMem->fSfbTileEnergyTab_exp[start_win * IGF_MAX_SFB_SHORT + sfb];
        } /* for sfb */

      } /* for tile */

    }
    /* If "groupLength" is a power of two */
    else if ((groupLength == 2) || (groupLength == 4) || (groupLength == 8)) {
      /* get the grid structure: */
      hGrid = &hPrivateStaticData->sGridInfoTab[window_sequence];
      int end_win = start_win + groupLength;

      int shift = groupLength >> 1;    /* cases 2 and 4 */
      if (groupLength == 8) shift = 3; /* case 8 */

      /* over all tiles: */
      for (tile = 0; tile < hGrid->iIGFNumTile; tile++) {
        /* get the mapping structure: */
        hMap = &hGrid->sIGFMapInfoTab[tile];
        hWorkMem = &(hPrivateData->IGF_Common_channel_data_handle->IGF_WorkingMem[tile]);

        /* over all sfb of tile: */
        for (sfb = 0; sfb < hMap->iSfbCnt; sfb++) {
          /* Survived energy */
          /* Find the maximum exponent to allign to */
          int max = -100;
          for (int win = start_win; win < end_win; win++) {
            if (hWorkMem->fSfbSurvivedEnergyTab_exp[win * IGF_MAX_SFB_SHORT + sfb] > max)
              max = hWorkMem->fSfbSurvivedEnergyTab_exp[win * IGF_MAX_SFB_SHORT + sfb];
          }

          /* To guarantee that no overflow occurs */
          max = max + shift;

          /* Allign and Add Survived energy values for all 8 windows */
          sE = (FIXP_DBL)0;
          for (int win = start_win; win < end_win; win++) {
            acc = hWorkMem->fSfbSurvivedEnergyTab[win * IGF_MAX_SFB_SHORT + sfb];
            s_shift = hWorkMem->fSfbSurvivedEnergyTab_exp[win * IGF_MAX_SFB_SHORT + sfb];
            /* Limit the shift to 31 */
            corrected_shift = fMax(-31, s_shift - max);
            acc = scaleValue(acc, corrected_shift);
            sE += acc;
          }
          hWorkMem->sE[sfb] = sE;
          hWorkMem->sE_exp[sfb] = max - shift; /* Move binary point to the left to show division */

          /* Tile energy */
          /* Find the maximum exponent to allign to */
          max = -100;
          for (int win = start_win; win < end_win; win++) {
            if (hWorkMem->fSfbTileEnergyTab_exp[win * IGF_MAX_SFB_SHORT + sfb] > max)
              max = hWorkMem->fSfbTileEnergyTab_exp[win * IGF_MAX_SFB_SHORT + sfb];
          }

          /* To guarantee that no overflow occurs */
          max = max + shift;

          /* Allign and Add Tile energy values for all 8 windows  */
          tE = (FIXP_DBL)0;
          for (int win = start_win; win < end_win; win++) {
            acc = hWorkMem->fSfbTileEnergyTab[win * IGF_MAX_SFB_SHORT + sfb];
            s_shift = hWorkMem->fSfbTileEnergyTab_exp[win * IGF_MAX_SFB_SHORT + sfb];
            /* Limit the shift to 31 */
            corrected_shift = fMax(-31, s_shift - max);
            acc = scaleValue(acc, corrected_shift);
            tE += acc;
          }
          hWorkMem->tE[sfb] = tE;
          hWorkMem->tE_exp[sfb] = max - shift; /* Move binary point to the left to show division */

        } /* for sfb */

      } /* for tile */

    } /* if((groupLength==2) */

    /* The rest; the general case */
    else {
      /* get the grid structure: */
      hGrid = &hPrivateStaticData->sGridInfoTab[window_sequence];
      int end_win = start_win + groupLength;

      int shift = 3;                   /* cases 5,6 and 7 */
      if (groupLength == 3) shift = 2; /* case 3 */

      /* over all tiles: */
      for (tile = 0; tile < hGrid->iIGFNumTile; tile++) {
        /* get the mapping structure: */
        hMap = &hGrid->sIGFMapInfoTab[tile];
        hWorkMem = &(hPrivateData->IGF_Common_channel_data_handle->IGF_WorkingMem[tile]);

        /* over all sfb of tile: */
        for (sfb = 0; sfb < hMap->iSfbCnt; sfb++) {
          /* Survived energy */
          /* Find the maximum exponent to allign to */
          int max = -100;
          for (int win = start_win; win < end_win; win++) {
            if (hWorkMem->fSfbSurvivedEnergyTab_exp[win * IGF_MAX_SFB_SHORT + sfb] > max)
              max = hWorkMem->fSfbSurvivedEnergyTab_exp[win * IGF_MAX_SFB_SHORT + sfb];
          }

          /* To guarantee that no overflow occurs */
          max = max + shift;

          /* Allign and Add Survived energy values for all 8 windows */
          sE = (FIXP_DBL)0;
          for (int win = start_win; win < end_win; win++) {
            acc = hWorkMem->fSfbSurvivedEnergyTab[win * IGF_MAX_SFB_SHORT + sfb];
            s_shift = hWorkMem->fSfbSurvivedEnergyTab_exp[win * IGF_MAX_SFB_SHORT + sfb];
            /*  Limit the shift to 31 */
            corrected_shift = fMax(-31, s_shift - max);
            acc = scaleValue(acc, corrected_shift);
            sE += acc;
          }
          /* Divide by the group length */
          sE = fDivNorm(sE, groupLength, &temp_int);
          hWorkMem->sE[sfb] = sE;
          hWorkMem->sE_exp[sfb] = temp_int + max - 31;

          /* Tile energy */
          /* Find the maximum exponent to allign to */
          max = -100;
          for (int win = start_win; win < end_win; win++) {
            if (hWorkMem->fSfbTileEnergyTab_exp[win * IGF_MAX_SFB_SHORT + sfb] > max)
              max = hWorkMem->fSfbTileEnergyTab_exp[win * IGF_MAX_SFB_SHORT + sfb];
          }

          /* To guarantee that no overflow occurs */
          max = max + shift;

          /* Allign and Add Tile energy values for all 8 windows */
          tE = (FIXP_DBL)0;
          for (int win = start_win; win < end_win; win++) {
            acc = hWorkMem->fSfbTileEnergyTab[win * IGF_MAX_SFB_SHORT + sfb];
            s_shift = hWorkMem->fSfbTileEnergyTab_exp[win * IGF_MAX_SFB_SHORT + sfb];
            /*  Limit the shift to 31 */
            corrected_shift = fMax(-31, s_shift - max);
            acc = scaleValue(acc, corrected_shift);
            tE += acc;
          }
          /* Divide by the group length */
          tE = fDivNorm(tE, groupLength, &temp_int);
          hWorkMem->tE[sfb] = tE;
          hWorkMem->tE_exp[sfb] = temp_int + max - 31;

        } /* for sfb */

      } /* for tile */

    } /* end else */

  }      /*     if( window_sequence ) */
  else { /* for long windows */

    /* get the grid structure: */
    hGrid = &hPrivateStaticData->sGridInfoTab[window_sequence];

    /* over all tiles: */
    for (tile = 0; tile < hGrid->iIGFNumTile; tile++) {
      /* get the mapping structure: */
      hMap = &hGrid->sIGFMapInfoTab[tile];
      hWorkMem = &(hPrivateData->IGF_Common_channel_data_handle->IGF_WorkingMem[tile]);

      /* over all sfb of tile: */
      for (sfb = 0; sfb < hMap->iSfbCnt; sfb++) {
        hWorkMem->sE[sfb] = hWorkMem->fSfbSurvivedEnergyTab[sfb];
        hWorkMem->sE_exp[sfb] = hWorkMem->fSfbSurvivedEnergyTab_exp[sfb];
        hWorkMem->tE[sfb] = hWorkMem->fSfbTileEnergyTab[sfb];
        hWorkMem->tE_exp[sfb] = hWorkMem->fSfbTileEnergyTab_exp[sfb];
      } /* for sfb */

    } /* for tile */
  }
}

static void createIGF_array_inplace(const INT particularTile, const INT particularTileIdx,
                                    const int particularTileBgn, const SHORT igfBgn,
                                    const INT igfMin, const FIXP_DBL* pSpectralData,
                                    FIXP_DBL* temp_IGF_band) {
  int width = particularTile;
  INT src = igfBgn - igfMin;
  INT raw_offset_from_igfBgn = 0;

  switch (particularTileIdx) {
    case 0: /* 2.5f */
      raw_offset_from_igfBgn = (particularTile << 1) + (particularTile >> 1);
      break;
    case 1: /* 2.0f */
      raw_offset_from_igfBgn = (particularTile << 1);
      break;
    case 2: /* 1.5f */
      raw_offset_from_igfBgn = particularTile + (particularTile >> 1);
      break;
    case 3: /* 1.0f */
      raw_offset_from_igfBgn = particularTile;
      break;
  }
  int offset = raw_offset_from_igfBgn + particularTileBgn - igfBgn;
  if ((offset & 1) == 1) offset++;

  int sb = particularTileBgn - offset;
  int delta = igfMin - sb;

  /* If mapping field is below igfMin value */
  if (delta > 0) {
    sb = (igfMin + particularTileBgn % src);

    for (; width--;) {
      *temp_IGF_band++ = pSpectralData[sb++];
      delta--;
      if ((delta == 0) || (sb % igfBgn) == 0) {
        sb = igfMin;
      }
    }
  } else {
    for (; width--;) {
      *temp_IGF_band++ = pSpectralData[sb++];
    }
  }
}

static void iisIGFDecoderCollectEnergiesMono(
    IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticData, IGF_PRIVATE_DATA_HANDLE hPrivateData,
    FIXP_DBL* pSpectralData, /**< ptr to spectreal window data */
    const SHORT specScale, FIXP_DBL** temp_IGF_band,
    const INT window_sequence, /**< in: 1==SHORT 0==LONG window seq */
    const INT win) {
  IGF_GRID_INFO_HANDLE hGrid;
  IGF_MAP_INFO_HANDLE hMap, hMap1;
  IGF_WORK_MEMORY_HANDLE hWorkMem, hWorkMem1;
  INT tile;
  INT tb;
  INT sfb;
  INT width;
#ifndef FUNCTION_iisIGFDecoderCollectEnergiesMono_func1
  FIXP_DBL val;
#endif
  FIXP_DBL sE, tE;
  INT shift, shift1, temp;

  /* get the grid structure: */
  hGrid = &hPrivateStaticData->sGridInfoTab[window_sequence];

  /* over all tiles: */
  for (tile = 0; tile < hGrid->iIGFNumTile; tile++) {
    /* get the mapping structure: */
    hMap = &hGrid->sIGFMapInfoTab[tile];
    hWorkMem = &(hPrivateData->IGF_Common_channel_data_handle->IGF_WorkingMem[tile]);

    /* first target line: */
    tb = hMap->iDesStart;

    /*Assign pointer */
    FIXP_DBL* pSpectralData_tb = &pSpectralData[tb];

    for (sfb = 0; sfb < hMap->iSfbCnt; sfb++) {
      /* width of the current sfb: */
      width = hMap->iSfbWidthTab[sfb];

      FIXP_DBL* p2_temp_IGF_band = *temp_IGF_band;

      *temp_IGF_band += width;

      shift = getScalefactor(pSpectralData_tb, width);
      shift1 = hWorkMem->intermediate_shift[win * IGF_MAX_SFB_SHORT + sfb];

      /*Calculate the normalization necessary due to addition*/
      /* Check for power of two /special case */
      INT width_shift = (INT)(fNormz((FIXP_DBL)width));
      /* Get the number of bits necessary minus one, because we need one sign bit only */
      width_shift = 31 - width_shift;

#ifdef FUNCTION_iisIGFDecoderCollectEnergiesMono_func1
      iisIGFDecoderCollectEnergiesMono_func1(pSpectralData_tb, p2_temp_IGF_band, width, shift,
                                             shift1, width_shift, &sE, &tE);
      pSpectralData_tb += width;
      p2_temp_IGF_band += width;
#else
      sE = (FIXP_DBL)0;
      tE = (FIXP_DBL)0;

      /* over all lines in sfb of tile: */
      while (width--) {
        FIXP_DBL sE_temp, tE_temp;
        FIXP_SGL val_SGL, val1_SGL;
        FIXP_DBL val1;

        /* sE += val * val;*/
        val = *pSpectralData_tb++ << shift;
        val_SGL = FX_DBL2FX_SGL(val);
        sE_temp = fPow2Div2(val_SGL);

        /* tE += val * val;*/
        val1 = *p2_temp_IGF_band++ << shift1;
        val1_SGL = FX_DBL2FX_SGL(val1);
        tE_temp = fPow2Div2(val1_SGL);

        if ((FIXP_SGL)0 != val_SGL) {
          tE_temp = (FIXP_DBL)0;
        }

        /* summation of survived energy: */
        sE += (sE_temp >> width_shift);
        /* summation of tile energy: */
        tE += (tE_temp >> width_shift);

      } /* width */
#endif
      /* Store results */
      hWorkMem->fSfbSurvivedEnergyTab[win * IGF_MAX_SFB_SHORT + sfb] = sE;
      hWorkMem->fSfbTileEnergyTab[win * IGF_MAX_SFB_SHORT + sfb] = tE;

      /* Calculating exponent of sE */
      temp = 1 + ((specScale - shift) << 1) + width_shift;
      hWorkMem->fSfbSurvivedEnergyTab_exp[win * IGF_MAX_SFB_SHORT + sfb] = temp;

      /* Calculating exponent of tE */
      temp = 1 +
             ((hWorkMem->Initial_exp[win * IGF_MAX_SFB_SHORT + sfb] -
               hWorkMem->intermediate_shift[win * IGF_MAX_SFB_SHORT + sfb])
              << 1) +
             width_shift;
      hWorkMem->fSfbTileEnergyTab_exp[win * IGF_MAX_SFB_SHORT + sfb] = temp;

    } /* for sfb */

  } /* for tile */

  /* Correction for the tile/sfb-pair split case */
  /* The ref soft finds the energy of the sfb-pair without splitting it. For this reason we mimic it
  here, by adding the two energy parts and dividing by two */
  if ((hPrivateStaticData->useHighRes == 0) && (IGF_GRID_LONG_WINDOW == window_sequence)) {
    /* over all tiles: */
    for (tile = 1; tile < hGrid->iIGFNumTile; tile++) {
      /* get the mapping structure: */
      hMap = &hGrid->sIGFMapInfoTab[tile];
      hWorkMem = &(hPrivateData->IGF_Common_channel_data_handle->IGF_WorkingMem[tile]);

      if (hMap->iSfbSplit) {
        FIXP_DBL sE1, sE2;
        FIXP_DBL tE1, tE2;
        SHORT sE1_exp, sE2_exp;
        SHORT tE1_exp, tE2_exp;

        sE2 = hWorkMem->fSfbSurvivedEnergyTab[win * IGF_MAX_SFB_SHORT];
        sE2_exp = hWorkMem->fSfbSurvivedEnergyTab_exp[win * IGF_MAX_SFB_SHORT];
        tE2 = hWorkMem->fSfbTileEnergyTab[win * IGF_MAX_SFB_SHORT];
        tE2_exp = hWorkMem->fSfbTileEnergyTab_exp[win * IGF_MAX_SFB_SHORT];

        /* get the mapping structure of the previous tile */
        hMap1 = &hGrid->sIGFMapInfoTab[tile - 1];
        hWorkMem1 = &(hPrivateData->IGF_Common_channel_data_handle->IGF_WorkingMem[tile - 1]);

        int last_sfb = hMap1->iSfbCnt - 1;
        int out_exp;

        sE1 = hWorkMem1->fSfbSurvivedEnergyTab[win * IGF_MAX_SFB_SHORT + last_sfb];
        sE1_exp = hWorkMem1->fSfbSurvivedEnergyTab_exp[win * IGF_MAX_SFB_SHORT + last_sfb];
        tE1 = hWorkMem1->fSfbTileEnergyTab[win * IGF_MAX_SFB_SHORT + last_sfb];
        tE1_exp = hWorkMem1->fSfbTileEnergyTab_exp[win * IGF_MAX_SFB_SHORT + last_sfb];

        /* Find if a difference between the scalings exists.*/
        /* Add 1 so that we multiply by 0.5 */
        int tmp = sE1_exp - sE2_exp;
        int lScale, rScale;

        if (tmp >= 0) {
          lScale = 1;
          rScale = fMin(tmp + 1, 31);
          out_exp = sE1_exp;
        } else {
          lScale = fMin(-tmp + 1, 31);
          rScale = 1;
          out_exp = sE2_exp;
        }

        FIXP_DBL out_res;
        out_res = (sE1 >> lScale) + (sE2 >> rScale);

        hWorkMem1->fSfbSurvivedEnergyTab[win * IGF_MAX_SFB_SHORT + last_sfb] = out_res;
        hWorkMem1->fSfbSurvivedEnergyTab_exp[win * IGF_MAX_SFB_SHORT + last_sfb] = out_exp;

        hWorkMem->fSfbSurvivedEnergyTab[win * IGF_MAX_SFB_SHORT] = out_res;
        hWorkMem->fSfbSurvivedEnergyTab_exp[win * IGF_MAX_SFB_SHORT] = out_exp;

        /* Find if a difference between the scalings exists.*/
        /* Add 1 so that we multiply by 0.5 */
        tmp = tE1_exp - tE2_exp;

        if (tmp >= 0) {
          lScale = 1;
          rScale = fMin(tmp + 1, 31);
          out_exp = tE1_exp;
        } else {
          lScale = fMin(-tmp + 1, 31);
          rScale = 1;
          out_exp = tE2_exp;
        }

        out_res = (tE1 >> lScale) + (tE2 >> rScale);

        hWorkMem1->fSfbTileEnergyTab[win * IGF_MAX_SFB_SHORT + last_sfb] = out_res;
        hWorkMem1->fSfbTileEnergyTab_exp[win * IGF_MAX_SFB_SHORT + last_sfb] = out_exp;

        hWorkMem->fSfbTileEnergyTab[win * IGF_MAX_SFB_SHORT] = out_res;
        hWorkMem->fSfbTileEnergyTab_exp[win * IGF_MAX_SFB_SHORT] = out_exp;

      } /* if(hMap->iSfbSplit) */

    } /* for tile */
  } /*  if ( (hPrivateStaticData->useHighRes == 0) && (IGF_GRID_LONG_WINDOW == window_sequence) ) */
}

/********************************************************************
**********************************************************************/
static void iisIGFDecoderMappingMono(
    IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticData, IGF_PRIVATE_DATA_HANDLE hPrivateData,
    FIXP_DBL** temp_IGF_band, const INT window_sequence, /**< in: 1==SHORT 0==LONG window seq */
    const INT win, const INT frameType) {
  IGF_GRID_INFO_HANDLE hGrid;
  IGF_MAP_INFO_HANDLE hMap;
  IGF_WORK_MEMORY_HANDLE hWorkMem;
  INT tile;
  INT sfb;
  INT width;

  /* get the grid structure: */
  hGrid = &hPrivateStaticData->sGridInfoTab[window_sequence];

  /* over all tiles: */
  for (tile = 0; tile < hGrid->iIGFNumTile; tile++) {
    /* get the mapping structure: */
    hMap = &hGrid->sIGFMapInfoTab[tile];
    hWorkMem = &(hPrivateData->IGF_Common_channel_data_handle->IGF_WorkingMem[tile]);

    width = hMap->iWidth;

    if (hMap->iWhiteningLevel != WHITENING_STRONG) {
      createIGF_array_inplace(width, hPrivateData->bitstreamData[frameType].iCurrTileIdxTab[tile],
                              hMap->iDesStart, hGrid->iIGFStartLine, hGrid->iIGFMinLine,
                              &hMap->fSpectrumTab[0][0] + win * 128, *temp_IGF_band);
    } else {
      FDKmemcpy(*temp_IGF_band, &hMap->fSpectrumTab[0][0] + win * 128,
                sizeof(FIXP_DBL) * (int)width);
    }

    for (sfb = 0; sfb < hMap->iSfbCnt; sfb++) {
      /* width of the current sfb: */
      width = hMap->iSfbWidthTab[sfb];

      FIXP_DBL* p2_temp_IGF_band = *temp_IGF_band;

      hWorkMem->intermediate_shift[win * IGF_MAX_SFB_SHORT + sfb] =
          getScalefactor(p2_temp_IGF_band, width);

      hWorkMem->Initial_exp[win * IGF_MAX_SFB_SHORT + sfb] =
          hWorkMem->Initial_exp[win * IGF_MAX_SFB_SHORT];

      *temp_IGF_band += width;
    }
  }
}

/************************************************************************
***************************************************************************/
static void iisIGFDecoderMappingAndMSStereo(
    IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticDataL,
    IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticDataR, IGF_PRIVATE_DATA_HANDLE hPrivateDataL,
    IGF_PRIVATE_DATA_HANDLE hPrivateDataR, FIXP_DBL** temp_IGF_bandL, FIXP_DBL** temp_IGF_bandR,
    const UCHAR* iUseMSTab, UCHAR* TNF_mask,
    const INT window_sequence, /**< in: 1==SHORT 0==LONG window seq */
    const INT win, const INT frameType) {
  IGF_GRID_INFO_HANDLE hGridL;
  IGF_GRID_INFO_HANDLE hGridR;
  IGF_MAP_INFO_HANDLE hMapL;
  IGF_MAP_INFO_HANDLE hMapR;
  IGF_WORK_MEMORY_HANDLE hWorkMemL;
  IGF_WORK_MEMORY_HANDLE hWorkMemR;
  INT tile;
  INT sfb;
  INT width;
  INT sfbStep;

  /* get the grid structure: */
  hGridL = &hPrivateStaticDataL->sGridInfoTab[window_sequence];
  hGridR = &hPrivateStaticDataR->sGridInfoTab[window_sequence];

  /*make sure it is a common window*/
  FDK_ASSERT(hGridL->iIGFNumTile == hGridR->iIGFNumTile);

  /* Adjust iUseMSTab to the IGF range and set pairing if active */
  iUseMSTab += hGridL->iIGFStartSfb;
  TNF_mask += hGridL->iIGFStartLine;

  sfbStep = 1;
  if ((hPrivateStaticDataL->useHighRes == 0) && (IGF_GRID_LONG_WINDOW == window_sequence)) {
    sfbStep = 2;
  }

  /* over all tiles: */
  for (tile = 0; tile < hGridL->iIGFNumTile; tile++) {
    /* get the mapping structure: */
    hMapL = &hGridL->sIGFMapInfoTab[tile];
    hMapR = &hGridR->sIGFMapInfoTab[tile];

    hWorkMemL = &(hPrivateDataL->IGF_Common_channel_data_handle->IGF_WorkingMem[tile]);
    hWorkMemR = &(hPrivateDataR->IGF_Common_channel_data_handle->IGF_WorkingMem[tile]);

    /* make sure it is common window: */
    FDK_ASSERT(hMapL->iDesStart == hMapR->iDesStart);

    /*Just for MS/stereo case*/
    /* Calculate the decimal point difference for the particular window*/
    INT delta = hWorkMemL->Initial_exp[win * IGF_MAX_SFB_SHORT] -
                hWorkMemR->Initial_exp[win * IGF_MAX_SFB_SHORT];
    /* Effectively Multiply by 0.5*/
    INT shiftL = 1, shiftR = 1;
    if (delta > 0)
      shiftR = fMin(31, shiftR + delta);
    else
      shiftL = fMin(31, shiftL - delta);

    /* Map tile values according to mapping tables */
    width = hMapL->iWidth;

    if (hMapL->iWhiteningLevel != WHITENING_STRONG) {
      createIGF_array_inplace(width, hPrivateDataL->bitstreamData[frameType].iCurrTileIdxTab[tile],
                              hMapL->iDesStart, hGridL->iIGFStartLine, hGridL->iIGFMinLine,
                              &hMapL->fSpectrumTab[0][0] + win * 128, *temp_IGF_bandL);
    } else {
      FDKmemcpy(*temp_IGF_bandL, &hMapL->fSpectrumTab[0][0] + win * 128,
                sizeof(FIXP_DBL) * (int)width);
    }

    width = hMapR->iWidth;

    if (hMapR->iWhiteningLevel != WHITENING_STRONG) {
      createIGF_array_inplace(width, hPrivateDataR->bitstreamData[frameType].iCurrTileIdxTab[tile],
                              hMapL->iDesStart, hGridR->iIGFStartLine, hGridR->iIGFMinLine,
                              &hMapR->fSpectrumTab[0][0] + win * 128, *temp_IGF_bandR);
    } else {
      FDKmemcpy(*temp_IGF_bandR, &hMapR->fSpectrumTab[0][0] + win * 128,
                sizeof(FIXP_DBL) * (int)width);
    }

    /* When tile boundary is in the middle of a sfb pair */
    INT sfb_jump;
    sfb_jump = 0;
    if (hMapL->iSfbSplit) {
      sfb_jump = 1;
      iUseMSTab--;
    }

    for (sfb = 0; sfb < hMapL->iSfbCnt; sfb++) {
      /* width of the current sfb: */
      width = hMapL->iSfbWidthTab[sfb];

      hWorkMemL->Initial_exp[win * IGF_MAX_SFB_SHORT + sfb] =
          hWorkMemL->Initial_exp[win * IGF_MAX_SFB_SHORT];
      hWorkMemR->Initial_exp[win * IGF_MAX_SFB_SHORT + sfb] =
          hWorkMemR->Initial_exp[win * IGF_MAX_SFB_SHORT];

      FIXP_DBL* p2_temp_IGF_bandL = *temp_IGF_bandL;
      FIXP_DBL* p2_temp_IGF_bandR = *temp_IGF_bandR;

      /*Just for MS/stereo case*/
      if (*iUseMSTab) {
        INT width_count = width;
        while (width_count--) {
          /*Shift left to get maximum resolution of the output.*/
          FIXP_DBL valL = *p2_temp_IGF_bandL >> shiftL;
          FIXP_DBL valR = *p2_temp_IGF_bandR >> shiftR;

          FIXP_DBL val1L = valL + valR;
          FIXP_DBL val1R = valL - valR;

          *p2_temp_IGF_bandL++ = val1L;
          *p2_temp_IGF_bandR++ = val1R;
        } /* while(width--) */

        FDKmemset(TNF_mask, 0x01, sizeof(UCHAR) * width);

        if (delta >= 0) {
          hWorkMemR->Initial_exp[win * IGF_MAX_SFB_SHORT + sfb] += delta;
        } else {
          hWorkMemL->Initial_exp[win * IGF_MAX_SFB_SHORT + sfb] -= delta;
        }

      } /* if ( iUseMSTab[sfbOffset] ) */

      TNF_mask += width;

      /* Find leading zeroes */
      hWorkMemL->intermediate_shift[win * IGF_MAX_SFB_SHORT + sfb] =
          getScalefactor(*temp_IGF_bandL, width);
      hWorkMemR->intermediate_shift[win * IGF_MAX_SFB_SHORT + sfb] =
          getScalefactor(*temp_IGF_bandR, width);

      *temp_IGF_bandL += width;
      *temp_IGF_bandR += width;

      if (sfb_jump) {
        iUseMSTab++;
        sfb_jump = 0;
      } else {
        iUseMSTab += sfbStep;
      }
    }
  }
}

/**********************************************************************/ /**

  **************************************************************************/

static void iisIGFDecoderCalculateGainsNew(
    IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticData, IGF_PRIVATE_DATA_HANDLE hPrivateData,
    const INT window_sequence /**< in: 1==SHORT 0==LONG window seq */
) {
  IGF_GRID_INFO_HANDLE hGrid;
  IGF_MAP_INFO_HANDLE hMap;
  IGF_WORK_MEMORY_HANDLE hWorkMem;
  INT tile;
  INT sfb;
  INT exp_dE, exp_tE, exp_sE, shift_sE, temp_int;
  FIXP_SGL width;
  FIXP_DBL dE, temp_FIXP_DBL;
  FIXP_DBL sE, tE, mE, gain;
  INT exp_mE;

  /* get the grid structure: */
  hGrid = &hPrivateStaticData->sGridInfoTab[window_sequence];

  /* over all tiles: */
  for (tile = 0; tile < hGrid->iIGFNumTile; tile++) {
    /* get the mapping structure: */
    hMap = &hGrid->sIGFMapInfoTab[tile];
    hWorkMem = &(hPrivateData->IGF_Common_channel_data_handle->IGF_WorkingMem[tile]);

    /* over all sfb of tile: */
    for (sfb = 0; sfb < hMap->iSfbCnt; sfb++) {
      /* Allign the width of the current sfb to the left for maximum precision */
      width = (FIXP_SGL)hMap->iSfbWidthTab[sfb];
      int headroom = fNormz(width) - 1;
      width <<= headroom;

      /* destination energy: (is transmitted in bitstream) */
      dE = hWorkMem->fSfbDestinEnergyTab[sfb];
      /*Get the exponent of the energy value*/
      exp_dE = hWorkMem->fSfbDestinEnergyTab_exp[sfb];

      /* survived energy: (survived lines in destination region) */
      sE = hWorkMem->sE[sfb];
      exp_sE = hWorkMem->sE_exp[sfb];

      /* tile energy: (lines in source region) */
      tE = hWorkMem->tE[sfb];
      exp_tE = hWorkMem->tE_exp[sfb];

      /* missing energy: (energy to restore in destination region) */
      /* mE = dE * dE * width - sE;*/
      temp_FIXP_DBL = fPow2(dE);

      /*mE = dE * dE * width*/
      mE = fMultDiv2(temp_FIXP_DBL, width);

      /*Calculate exp of mE*/
      exp_mE = (exp_dE << 1) + (16 - headroom);

      /*mEtE_shift*/
      INT mEtE_shift = exp_mE - exp_tE;

      if (sE != (FIXP_DBL)0) {
        /*sE shift calculation*/
        shift_sE = exp_sE - exp_mE;

        /* Limit shift_sE to 31*/
        shift_sE = fMax(-31, shift_sE);

        /*Check sE leading bits */
        INT temp = fNorm(sE);

        /* Check if sE and mE can be alligned */
        if (shift_sE > temp) {
          /* Obviously sE is greater than mE, so sE is set to max because of the substraction later
           */
          sE = (FIXP_DBL)MAXVAL_DBL;
        } else {
          /*Shifting so that the output result of the next calcluation will be in the proper format
           */
          sE = scaleValue(sE, shift_sE);
        }
      }

      /* mE = dE * dE * width - sE;*/
      mE = mE - sE;

      /* calculate gain: */
      if ((mE > (FIXP_DBL)0) && (tE > (FIXP_DBL)0)) {
        /*Calculate
        hMap->fSfbGainTab[ sfb ] = MIN( 10.f, (float)sqrt( mE / tE) );*/

        /* result= mE / tE */
        temp_FIXP_DBL = fDivNorm(mE, tE, &temp_int);

        /* add to shift */
        temp_int += mEtE_shift;

        /* sqrt(result) */
        gain = sqrtFixp_lookup(temp_FIXP_DBL, &temp_int);

        if (temp_int > 3) {
          /* If value > 8 then generate 10.0 */
          temp_FIXP_DBL = scaleValue((FIXP_DBL)10, fMax(-31, (31 - temp_int)));
          /* MIN( 10.f, (float)sqrt( mE / tE) )
          gain=fMin(gain,temp_FIXP_DBL);*/
          if (gain > temp_FIXP_DBL) {
            /* 10.0 in Q4.27 */
            gain = (FIXP_DBL)0x50000000;
            temp_int = 4;
          }
        }

        /* Store the result */
        hWorkMem->fSfbGainTab[sfb] = gain;
        hWorkMem->fSfbGainTab_exp[sfb] = temp_int;

      } else {
        hWorkMem->fSfbGainTab[sfb] = (FIXP_DBL)0;
        hWorkMem->fSfbGainTab_exp[sfb] = 0;
      }

    } /* for sfb */

  } /* for tile */
}

/**********************************************************************/ /**
   checks if a new tile index set was transmitted and configures tilemapping
  **************************************************************************/
static void iisIGFDecLibCheckTileTransition(
    IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticData, IGF_PRIVATE_DATA_HANDLE hPrivateData,
    const INT window_sequence, /**< in: 1==SHORT 0==LONG window seq */
    const INT frameType) {
  IGF_GRID_INFO_HANDLE hGrid;
  IGF_MAP_INFO_HANDLE hMap;
  INT tile;
  INT doInit;

  hGrid = &hPrivateStaticData->sGridInfoTab[window_sequence];

  /*  check if the grid/mapping information is still valid: */
  doInit = 0;
  for (tile = 0; tile < hGrid->iIGFNumTile; tile++) {
    if (hGrid->iIGFTileIdxTab[tile] !=
        hPrivateData->bitstreamData[frameType].iCurrTileIdxTab[tile]) {
      doInit = 1;
    }
  }

  /* do initialization, if needed: */
  if (doInit) {
    switch (window_sequence) {
      case IGF_GRID_LONG_WINDOW:
        /* AAC long blocks */
        iisIGFDecLibInitGridInfo(
            hPrivateData->IGF_Common_channel_data_handle->Spectrum_tab_array, hGrid,
            IGF_GRID_LONG_WINDOW, hPrivateStaticData->sfbOffsetLB, hPrivateStaticData->numSfbLB,
            hPrivateStaticData->igfStartSfbLB, hPrivateStaticData->igfStopSfbLB,
            hPrivateStaticData->igfMinSubbandLB, hPrivateStaticData->useHighRes,
            hPrivateStaticData->igfNTilesLB, hPrivateStaticData->igfTilesLB,
            hPrivateData->bitstreamData[frameType].iCurrTileIdxTab);
        break;
      case IGF_GRID_SHORT_WINDOW:
        /* AAC short blocks */
        iisIGFDecLibInitGridInfo(
            hPrivateData->IGF_Common_channel_data_handle->Spectrum_tab_array, hGrid,
            IGF_GRID_SHORT_WINDOW, (const SHORT*)hPrivateStaticData->sfbOffsetSB,
            hPrivateStaticData->numSfbSB, hPrivateStaticData->igfStartSfbSB,
            hPrivateStaticData->igfStopSfbSB, hPrivateStaticData->igfMinSubbandSB,
            hPrivateStaticData->useHighRes, hPrivateStaticData->igfNTilesSB,
            hPrivateStaticData->igfTilesSB, hPrivateData->bitstreamData[frameType].iCurrTileIdxTab);
        break;
      default:
        FDK_ASSERT(0);
        break;
    } /* switch window_sequence */
  }   /* if doInit */

  /* map whitening levels: */
  for (tile = 0; tile < hGrid->iIGFNumTile; tile++) {
    hMap = &hGrid->sIGFMapInfoTab[tile];

    hMap->iWhiteningLevel = hPrivateData->bitstreamData[frameType].igfWhiteningLevel[tile];
  }
}

/**********************************************************************/ /**
  mapping of source to destination and application of gain values
  **************************************************************************/
static void iisIGFDecoderApplyGainsMonoNew(
    IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticData, const IGF_PRIVATE_DATA_HANDLE hPrivateData,
    FIXP_DBL* pSpectralData, /**< inout: ptr to spectral window data */
    SHORT* specScale, FIXP_DBL** pSpectralDataReshuffle,
    const INT window_sequence, /**< in: 1==SHORT 0==LONG window seq */
    const INT win              /**< in: 0..7 window number */
) {
  IGF_GRID_INFO_HANDLE hGrid;
  IGF_MAP_INFO_HANDLE hMap;
  IGF_WORK_MEMORY_HANDLE hWorkMem;
  INT tile;
  INT tb;
  INT sfb;
  INT width;
  FIXP_DBL* virtualSpec;

  virtualSpec = hPrivateData->IGF_Common_channel_data_handle->virtualSpec;

  /* get the grid structure: */
  hGrid = &hPrivateStaticData->sGridInfoTab[window_sequence];

  int MaxExponent = -1000;

  /* Find if an exponent adjustment  for the whole sequence is necessary */
  for (tile = 0; tile < hGrid->iIGFNumTile; tile++) {
    /* get the mapping structure: */
    hMap = &hGrid->sIGFMapInfoTab[tile];
    hWorkMem = &(hPrivateData->IGF_Common_channel_data_handle->IGF_WorkingMem[tile]);

    /* over all sfb of tile: */
    for (sfb = 0; sfb < hMap->iSfbCnt; sfb++) {
      if (hWorkMem->fSfbGainTab[sfb] > (FIXP_DBL)0) {
        /* Find headroom of the gain */
        INT lead_zeros = (INT)(fNormz(hWorkMem->fSfbGainTab[sfb]) - 1);
        lead_zeros = fMax(0, lead_zeros);

        /* Left allign the gain */
        hWorkMem->fSfbGainTab[sfb] = (hWorkMem->fSfbGainTab[sfb] << lead_zeros);

        /* Adjust the gain exponent */
        hWorkMem->fSfbGainTab_exp[sfb] -= lead_zeros;

        /* Headroom of data */
        int shift1 = hWorkMem->intermediate_shift[win * IGF_MAX_SFB_SHORT + sfb];

        /* Calcuation of output exponent for the particular sfb */
        int shift = (hWorkMem->Initial_exp[win * IGF_MAX_SFB_SHORT + sfb] - shift1) +
                    hWorkMem->fSfbGainTab_exp[sfb];

        /*  Limit shift_sE to 31 */
        shift = fMax(-31, shift);

        /* Find maximum exaponent */
        if (shift > MaxExponent) MaxExponent = shift;

      } /* if(hWorkMem->fSfbGainTab[sfb] > 0) */

    } /* for(sfb = 0; sfb < hMap->iSfbCnt; sfb++) */

  } /* for (tile = 0; tile < hGrid->iIGFNumTile; tile++)*/

  /* Rescale the whole frame when the exponent in the IGF range is bigger than the specScale */
  int FrameLength = 1024;
  if (window_sequence) FrameLength = 128;

  if (MaxExponent > *specScale) {
    /* Rescale main band */
    int diff = fMin((MaxExponent - *specScale), 31);
    for (int i = 0; i < FrameLength; i++) {
      pSpectralData[i] >>= diff;
    }
    /* Change the exponent */
    *specScale = MaxExponent;
  } else {
    MaxExponent = *specScale;
  }

  /* Must be defined here */
  FIXP_DBL* p2_virtualSpec_tb;

  /* over all tiles: */
  for (tile = 0; tile < hGrid->iIGFNumTile; tile++) {
    /* get the mapping structure: */
    hMap = &hGrid->sIGFMapInfoTab[tile];
    hWorkMem = &(hPrivateData->IGF_Common_channel_data_handle->IGF_WorkingMem[tile]);

    /* first target line: */
    tb = hMap->iDesStart;

    p2_virtualSpec_tb = &virtualSpec[tb];

    /* over all sfb of tile: */
    for (sfb = 0; sfb < hMap->iSfbCnt; sfb++) {
      /* width of the current sfb: */
      width = hMap->iSfbWidthTab[sfb];

      int shift1 = 0;
      int shift = 0;

      /* Make the gain a 16 bit value */
      FIXP_SGL hMap_fSfbGainTab_sfb = FX_DBL2FX_SGL(hWorkMem->fSfbGainTab[sfb]);

      if (hMap_fSfbGainTab_sfb > (FIXP_SGL)0) {
        shift1 = hWorkMem->intermediate_shift[win * IGF_MAX_SFB_SHORT + sfb];

        /* Calcuation of shift value for output */
        shift = MaxExponent - ((hWorkMem->Initial_exp[win * IGF_MAX_SFB_SHORT + sfb] - shift1) +
                               hWorkMem->fSfbGainTab_exp[sfb]);

        shift = fMin(31, shift);
      }

      FIXP_DBL* p2_pSpectralDataReshuffle = *pSpectralDataReshuffle;

      *pSpectralDataReshuffle += width;

      FDK_ASSERT((width & 3) == 0);
#ifdef FUNCTION_iisIGFDecoderApplyGainsMonoNew_func1
      iisIGFDecoderApplyGainsMonoNew_func1(p2_pSpectralDataReshuffle, p2_virtualSpec_tb, shift1,
                                           shift, hMap_fSfbGainTab_sfb, width);
      p2_virtualSpec_tb += width;
#else
      /* over all lines in sfb of tile: */
      for (; width--;) {
        /* Operation */
        /* virtualSpec[tb] = hMap->fSfbGainTab[sfb] * hMap->fSpectrumTab[win][sb]; */
        /* When we flatten all values are filled from the main spectum, i.e. not only the zero ones
          We create an additional array to the main spectral array and pass it to the TNF function
        */
        FIXP_DBL temp;
        temp = *p2_pSpectralDataReshuffle++ << shift1;
        *p2_virtualSpec_tb++ = fMult(hMap_fSfbGainTab_sfb, temp) >> shift;
      } /* width */
#endif
    } /* for sfb */

  } /* for tile */
}
/**********************************************************************/ /**

 **************************************************************************/
static void iisIGFDecoderFillMono(FIXP_DBL* spectrum, FIXP_DBL* virtualSpec, int IGFstartLine,
                                  int IGFstopLine, UCHAR* TNF_mask, UCHAR flag_use) {
  /* When TNF in mono is used */
  if (flag_use) {
    /* Add data generated from IGF over the real data*/
    for (int i = IGFstartLine; i < IGFstopLine; i++) {
      UCHAR temp = 1;
      if (spectrum[i] == (FIXP_DBL)0) {
        spectrum[i] = virtualSpec[i];
        temp = (UCHAR)0;
      }
      TNF_mask[i] = temp;
    } /* for(int i=IGFstartLineL;i<IGFstopLineL;i++) */
  }
  /* When TNF in mono is not used */
  else {
    for (int i = IGFstartLine; i < IGFstopLine; i++) {
      if (spectrum[i] == (FIXP_DBL)0) {
        spectrum[i] = virtualSpec[i];
      }
    } /* for(int i=IGFstartLineL;i<IGFstopLineL;i++) */
  }   /* if(flag_use) */
}
/**********************************************************************/ /**

 **************************************************************************/
static void iisIGFDecoderFillStereo(FIXP_DBL* spectrumL, FIXP_DBL* spectrumR,
                                    FIXP_DBL* virtualSpecL, FIXP_DBL* virtualSpecR,
                                    int IGFstartLineL, int IGFstopLineL, UCHAR* TNF_mask,
                                    UCHAR* TNF_maskL, UCHAR* TNF_maskR, UCHAR flag_use) {
  /* When TNF in stereo is used */
  if (flag_use) {
    /* Add data generated from IGF over the real data*/
    for (int i = IGFstartLineL; i < IGFstopLineL; i++) {
      UCHAR tempL = 1, tempR = 1;
      if (spectrumL[i] == (FIXP_DBL)0) {
        spectrumL[i] = virtualSpecL[i];
        tempL = (UCHAR)0;
      }
      if (spectrumR[i] == (FIXP_DBL)0) {
        spectrumR[i] = virtualSpecR[i];
        tempR = (UCHAR)0;
      }
      if (TNF_mask[i]) {
        tempL = tempR = (tempL || tempR);
      }
      TNF_maskL[i] = tempL;
      TNF_maskR[i] = tempR;
    } /* for(int i=IGFstartLineL;i<IGFstopLineL;i++) */
  }
  /* When TNF in stereo is not used */
  else {
    for (int i = IGFstartLineL; i < IGFstopLineL; i++) {
      if (spectrumL[i] == (FIXP_DBL)0) {
        spectrumL[i] = virtualSpecL[i];
      }
      if (spectrumR[i] == (FIXP_DBL)0) {
        spectrumR[i] = virtualSpecR[i];
      }
    } /* for(int i=IGFstartLineL;i<IGFstopLineL;i++) */
  }   /* if(flag_use) */
}

/**********************************************************************/ /**

 **************************************************************************/

static void iisIGFDecoderDecodeDestinEnergies(
    const IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticData,
    const IGF_PRIVATE_DATA_HANDLE hPrivateData,
    const INT window_sequence, /**< in: 1==SHORT 0==LONG window seq */
    const INT group,           /**< in: group number, used with SHORT windows */
    const INT frameType) {
  IGF_GRID_INFO_HANDLE hGrid;
  IGF_MAP_INFO_HANDLE hMap;
  IGF_WORK_MEMORY_HANDLE hWorkMem;
  INT tile;
  INT sfb;
  INT exp_DBL;
  SCHAR* p2sfe;
  INT sfb_step;
  FIXP_DBL temp_FIXP_DBL;

  /* get the grid structure: */
  hGrid = &hPrivateStaticData->sGridInfoTab[window_sequence];

  p2sfe = &(hPrivateData->bitstreamData[frameType].sfe[group * hGrid->iIGFNumSfb]);

  sfb_step = 1;
  if ((hPrivateStaticData->useHighRes == 0) && (IGF_GRID_LONG_WINDOW == window_sequence)) {
    sfb_step = 2;
  }

  /* over all tiles: */
  for (tile = 0; tile < hGrid->iIGFNumTile; tile++) {
    /* get the mapping structure: */
    hMap = &hGrid->sIGFMapInfoTab[tile];
    hWorkMem = &(hPrivateData->IGF_Common_channel_data_handle->IGF_WorkingMem[tile]);

    /* When tile boundary is in the middle of a sfb pair */
    INT sfb_jump;
    sfb_jump = 0;
    if (hMap->iSfbSplit) {
      sfb_jump = 1;
      p2sfe--;
    }

    /* over all sfb of tile: */
    for (sfb = 0; sfb < hMap->iSfbCnt; sfb++) {
      /*Data is in Q7.0, i.e from 0 to 127
      0.25 * hMap->fSfbDestinEnergyTab[ sfb ]
      Multiply implicitly by 0.25
      Assuming result in Q5.2*/
      INT sfe = *p2sfe;

      /* pow( 2.0, 0.25 * hMap->fSfbDestinEnergyTab[ sfb ] ) */
      temp_FIXP_DBL = f2Pow((FIXP_DBL)sfe, 29, &exp_DBL);

      if (sfb_jump) {
        p2sfe++;
        sfb_jump = 0;
      } else {
        p2sfe += sfb_step;
      }

      /* Store result and exponent of the result */
      hWorkMem->fSfbDestinEnergyTab[sfb] = temp_FIXP_DBL;

      hWorkMem->fSfbDestinEnergyTab_exp[sfb] = exp_DBL;

      /*Overall operation
      hMap->fSfbDestinEnergyTab[ sfb ] = (FIXP_SGL) pow( 2.0, 0.25 * hMap->fSfbDestinEnergyTab[ sfb
      ] );*/

    } /* for sfb */

  } /* for tile */
}

/**********************************************************************/ /**

  **************************************************************************/
void CIgf_apply(IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticData,
                IGF_PRIVATE_DATA_HANDLE hPrivateData, FIXP_DBL* p2_spectrum, SHORT* specScale,
                const INT window_sequence, const INT numOfGroups, const INT NumberOfSpectra,
                const UCHAR* groupLength, ULONG* randomSeed, UCHAR* TNF_mask,
                const UCHAR flag_INF_active, const INT frameType) {
  IGF_GRID_INFO_HANDLE hGrid;
  INT group;
  INT startWin;
  INT winOffset = 0;
  INT win;
  FIXP_DBL temp_IGF_band[1024];
  FIXP_DBL* p2_temp_IGF_band;

  /* Skip IGF processing if no data is transmitted*/
  if (hPrivateData->bitstreamData[frameType].igf_allZero) {
    return;
  }

  hGrid = &hPrivateStaticData->sGridInfoTab[window_sequence];

  INT numMdctLines;
  /* Set parameters for long and short blocks */
  if (window_sequence != IGF_GRID_SHORT_WINDOW) {
    numMdctLines = hPrivateStaticData->longBlockSize;
  } else {
    numMdctLines = hPrivateStaticData->shortBlockSize;
  }

  /* Clean computational error in the IGF frequency range so that computed zeroes be recognized by
   * the algo. Non-MPEG/Out of standard procedure */
  int IGFstartLine = (int)hGrid->iIGFStartLine;
  int IGFstopLine = (int)hGrid->iIGFStopLine;

  for (win = 0; win < NumberOfSpectra; win++) {
    FIXP_DBL* pointer = &p2_spectrum[win * numMdctLines] + IGFstartLine;
    for (int i = IGFstartLine; i < IGFstopLine; i++) {
      FIXP_DBL value = *pointer;
      if ((value > -(FIXP_DBL)IGF_CLEAN_THRESHOLD) && (value < (FIXP_DBL)IGF_CLEAN_THRESHOLD)) {
        *pointer = (FIXP_DBL)0;
      }
      pointer++;
    }
  }

  if ((window_sequence == IGF_GRID_LONG_WINDOW) || (window_sequence == IGF_GRID_SHORT_WINDOW)) {
    /* If INF is active=> tiles have been initialized (injected) and have undergone processing
    earlier. When INF is not active tiles can be initialized just before the IGF processing. In this
    way we save a lot of processing power. */
    if (flag_INF_active) {
      /* Initialize scale factors for windows and tiles*/
      iisIGFDecLibInitFractScaleValues(hPrivateStaticData, hPrivateData, window_sequence);

    } else { /* INF not active */

      /* Copy spectrum and scale factors to tiles for further processing */
      iisIGFDecLibInjectSourceSpectrum(hPrivateStaticData, hPrivateData, p2_spectrum, specScale,
                                       window_sequence, frameType);

    } /* (flag_INF_active)*/
  }

  /* Applicable just for long windows; When whitening is used */
  if ((IGF_GRID_SHORT_WINDOW != window_sequence) && (hPrivateStaticData->igfUseWhitening)) {
    /* "temp_IGF_band" is passed as a working/intermediate result array */
    iisIGFDecoderApplyWhitening(hPrivateData, hGrid, p2_spectrum, temp_IGF_band, randomSeed,
                                flag_INF_active, 0);
  }

  /* Check if TNF is active in mono */
  UCHAR flag_TNF_active = (UCHAR)((IGF_GRID_SHORT_WINDOW != window_sequence) &&
                                  (hPrivateData->bitstreamData[frameType].igfUseEnfFlat));

  for (group = 0; group < numOfGroups; group++) {
    /* memorize the offset of windows in a group: */
    startWin = winOffset;

    p2_temp_IGF_band = &temp_IGF_band[0];

    /* collect energies for each window in the group */
    for (win = 0; win < groupLength[group]; win++) {
      FIXP_DBL* keep_p2_temp_IGF_band = p2_temp_IGF_band;

      /* Map data from source to IGF band */
      iisIGFDecoderMappingMono(hPrivateStaticData, hPrivateData, &p2_temp_IGF_band, window_sequence,
                               winOffset, frameType);

      /* Calculate survived and tile energies */
      iisIGFDecoderCollectEnergiesMono(hPrivateStaticData, hPrivateData,
                                       &p2_spectrum[winOffset * 128], specScale[winOffset],
                                       &keep_p2_temp_IGF_band, window_sequence, winOffset);
      winOffset++;
    }

    /* Normalize energy to the length of the group /Just for short windows */
    iisIGFDecoderCollectEnergiesMonoNormalize(hPrivateStaticData, hPrivateData, window_sequence,
                                              groupLength[group], startWin);

    /* decode the transmitted destination energies: */
    iisIGFDecoderDecodeDestinEnergies(hPrivateStaticData, hPrivateData, window_sequence, group,
                                      frameType);

    /* Calculate the gains for each sfb and tile */
    iisIGFDecoderCalculateGainsNew(hPrivateStaticData, hPrivateData, window_sequence);

    /* apply IGF with computed energies: */
    winOffset = startWin;

    p2_temp_IGF_band = &temp_IGF_band[0];

    for (win = 0; win < groupLength[group]; win++) {
      /* Apply calculated gains to mapped data */
      iisIGFDecoderApplyGainsMonoNew(hPrivateStaticData, hPrivateData,
                                     &p2_spectrum[winOffset * 128], &specScale[winOffset],
                                     &p2_temp_IGF_band, window_sequence, winOffset);

      iisIGFDecoderFillMono(&p2_spectrum[winOffset * 128],
                            hPrivateData->IGF_Common_channel_data_handle->virtualSpec, IGFstartLine,
                            IGFstopLine, TNF_mask, flag_TNF_active);

      winOffset++;
    } /* for win */

  } /* for grp */
}

/**********************************************************************/ /**

  **************************************************************************/
void CIgf_apply_stereo(IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticDataL,
                       IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticDataR,
                       IGF_PRIVATE_DATA_HANDLE hPrivateDataL, IGF_PRIVATE_DATA_HANDLE hPrivateDataR,
                       FIXP_DBL* p2_spectrumL, FIXP_DBL* p2_spectrumR, SHORT* specScaleL,
                       SHORT* specScaleR, const INT window_sequence, const INT numOfGroups,
                       const INT NumberOfSpectra, const UCHAR* groupLength, ULONG* randomSeedL,
                       ULONG* randomSeedR, const UCHAR* iUseMSTab, UCHAR* TNF_maskL,
                       UCHAR* TNF_maskR, const UCHAR flag_INF_active, const INT frameType) {
  IGF_GRID_INFO_HANDLE hGridL;
  IGF_GRID_INFO_HANDLE hGridR;
  INT group;
  INT startWin;
  INT winOffset = 0;
  INT win;
  FIXP_DBL temp_IGF_bandL[1024];
  FIXP_DBL temp_IGF_bandR[1024];
  FIXP_DBL* p2_temp_IGF_bandL;
  FIXP_DBL* p2_temp_IGF_bandR;

  /* Skip IGF processing if no data is transmitted*/
  if (hPrivateDataL->bitstreamData[frameType].igf_allZero &&
      hPrivateDataR->bitstreamData[frameType].igf_allZero) {
    return;
  }

  hGridL = &hPrivateStaticDataL->sGridInfoTab[window_sequence];
  hGridR = &hPrivateStaticDataR->sGridInfoTab[window_sequence];

  INT numMdctLines;
  /*Set parameters for long and short blocks*/
  if (window_sequence != IGF_GRID_SHORT_WINDOW) {
    numMdctLines = hPrivateStaticDataL->longBlockSize;
  } else {
    numMdctLines = hPrivateStaticDataL->shortBlockSize;
  }

  /* Clean computational error in the IGF frequency range so that computed zeroes be recognized by
   * the algo. Non-MPEG/Out of standard procedure */
  int IGFstartLine = (int)hGridL->iIGFStartLine;
  int IGFstopLine = (int)hGridL->iIGFStopLine;

  for (win = 0; win < NumberOfSpectra; win++) {
    FIXP_DBL* pointer = &p2_spectrumL[win * numMdctLines] + IGFstartLine;
    for (int i = IGFstartLine; i < IGFstopLine; i++) {
      FIXP_DBL value = *pointer;
      if ((value > -(FIXP_DBL)IGF_CLEAN_THRESHOLD) && (value < (FIXP_DBL)IGF_CLEAN_THRESHOLD)) {
        *pointer = (FIXP_DBL)0;
      }
      pointer++;
    }
  }

  /* Clean computational error in the IGF frequency range so that computed zeroes be recognized by
   * the algo. Non-MPEG/Out of standard procedure */
  IGFstartLine = (int)hGridR->iIGFStartLine;
  IGFstopLine = (int)hGridR->iIGFStopLine;

  for (win = 0; win < NumberOfSpectra; win++) {
    FIXP_DBL* pointer = &p2_spectrumR[win * numMdctLines] + IGFstartLine;
    for (int i = IGFstartLine; i < IGFstopLine; i++) {
      FIXP_DBL value = *pointer;
      if ((value > -(FIXP_DBL)IGF_CLEAN_THRESHOLD) && (value < (FIXP_DBL)IGF_CLEAN_THRESHOLD)) {
        *pointer = (FIXP_DBL)0;
      }
      pointer++;
    }
  }

  /* If INF is active=> tiles have been initialized (injected) and have undergone processing
   earlier. When INF is not active tiles can be initialized just before the IGF processing. In this
   way we save a lot of processing power. */
  if (flag_INF_active) {
    /*Initialize scale factors for windows and tiles*/
    iisIGFDecLibInitFractScaleValues(hPrivateStaticDataL, hPrivateDataL, window_sequence);
    iisIGFDecLibInitFractScaleValues(hPrivateStaticDataR, hPrivateDataR, window_sequence);

  } else { /* INF not active */

    /* Copy spectrum and scale factors to tiles for further processing */
    iisIGFDecLibInjectSourceSpectrum(hPrivateStaticDataL, hPrivateDataL, p2_spectrumL, specScaleL,
                                     window_sequence, IGF_FRAME_DIVISION_AAC_OR_TCX_LONG);
    iisIGFDecLibInjectSourceSpectrum(hPrivateStaticDataR, hPrivateDataR, p2_spectrumR, specScaleR,
                                     window_sequence, IGF_FRAME_DIVISION_AAC_OR_TCX_LONG);
  }

  /*Applicable just for long windows; When whitening is used*/
  if ((IGF_GRID_SHORT_WINDOW != window_sequence) && (hPrivateStaticDataL->igfUseWhitening)) {
    /* "temp_IGF_bandL" is passed as a working/intermediate result array*/
    iisIGFDecoderApplyWhitening(hPrivateDataL, hGridL, p2_spectrumL, temp_IGF_bandL, randomSeedL,
                                flag_INF_active, 1);
  }
  if ((IGF_GRID_SHORT_WINDOW != window_sequence) && (hPrivateStaticDataR->igfUseWhitening)) {
    /*"temp_IGF_bandR" is passed as a working/intermediate result array*/
    iisIGFDecoderApplyWhitening(hPrivateDataR, hGridR, p2_spectrumR, temp_IGF_bandR, randomSeedR,
                                flag_INF_active, 1);
  }

  /* Check if TNF is active in stereo */
  UCHAR flag_TNF_active = (UCHAR)((IGF_GRID_SHORT_WINDOW != window_sequence) &&
                                  ((hPrivateDataL->bitstreamData[frameType].igfUseEnfFlat) ||
                                   (hPrivateDataR->bitstreamData[frameType].igfUseEnfFlat)));

  UCHAR TNF_mask[1024];
  FDKmemclear(TNF_mask + IGFstartLine, sizeof(UCHAR) * (IGFstopLine - IGFstartLine));

  for (group = 0; group < numOfGroups; group++) {
    /* memorize the offset of windows in a group: */
    startWin = winOffset;

    p2_temp_IGF_bandL = &temp_IGF_bandL[0];
    p2_temp_IGF_bandR = &temp_IGF_bandR[0];

    /* collect energies for each window in the group */
    for (win = 0; win < groupLength[group]; win++) {
      FIXP_DBL* keep_p2_temp_IGF_bandL = p2_temp_IGF_bandL;
      FIXP_DBL* keep_p2_temp_IGF_bandR = p2_temp_IGF_bandR;

      /* Map data from source to IGF band and implement MS */
      iisIGFDecoderMappingAndMSStereo(hPrivateStaticDataL, hPrivateStaticDataR, hPrivateDataL,
                                      hPrivateDataR, &p2_temp_IGF_bandL, &p2_temp_IGF_bandR,
                                      iUseMSTab + group * 64, TNF_mask, window_sequence, winOffset,
                                      frameType);

      /* Calculate survived and tile energies */
      iisIGFDecoderCollectEnergiesMono(hPrivateStaticDataL, hPrivateDataL,
                                       &p2_spectrumL[winOffset * 128], specScaleL[winOffset],
                                       &keep_p2_temp_IGF_bandL, window_sequence, winOffset);

      /* Calculate survived and tile energies */
      iisIGFDecoderCollectEnergiesMono(hPrivateStaticDataR, hPrivateDataR,
                                       &p2_spectrumR[winOffset * 128], specScaleR[winOffset],
                                       &keep_p2_temp_IGF_bandR, window_sequence, winOffset);

      winOffset++;
    }

    /*Normalize energy to the length of the group Just for short windows*/
    iisIGFDecoderCollectEnergiesMonoNormalize(hPrivateStaticDataL, hPrivateDataL, window_sequence,
                                              groupLength[group], startWin);
    iisIGFDecoderCollectEnergiesMonoNormalize(hPrivateStaticDataR, hPrivateDataR, window_sequence,
                                              groupLength[group], startWin);

    /*Decode the transmitted destination energies*/
    iisIGFDecoderDecodeDestinEnergies(hPrivateStaticDataL, hPrivateDataL, window_sequence, group,
                                      frameType);

    /*Decode the transmitted destination energies*/
    iisIGFDecoderDecodeDestinEnergies(hPrivateStaticDataR, hPrivateDataR, window_sequence, group,
                                      frameType);

    /* Calculate the gains for each sfb and tile */
    iisIGFDecoderCalculateGainsNew(hPrivateStaticDataL, hPrivateDataL, window_sequence);

    /* Calculate the gains for each sfb and tile */
    iisIGFDecoderCalculateGainsNew(hPrivateStaticDataR, hPrivateDataR, window_sequence);

    /* apply IGF with computed energies: */
    winOffset = startWin;

    p2_temp_IGF_bandL = &temp_IGF_bandL[0];
    p2_temp_IGF_bandR = &temp_IGF_bandR[0];

    for (win = 0; win < groupLength[group]; win++) {
      /* Apply calculated gains to mapped data */
      iisIGFDecoderApplyGainsMonoNew(hPrivateStaticDataL, hPrivateDataL,
                                     &p2_spectrumL[winOffset * 128], &specScaleL[winOffset],
                                     &p2_temp_IGF_bandL, window_sequence, winOffset);
      /* Apply calculated gains to mapped data */
      iisIGFDecoderApplyGainsMonoNew(hPrivateStaticDataR, hPrivateDataR,
                                     &p2_spectrumR[winOffset * 128], &specScaleR[winOffset],
                                     &p2_temp_IGF_bandR, window_sequence, winOffset);

      iisIGFDecoderFillStereo(&p2_spectrumL[winOffset * 128], &p2_spectrumR[winOffset * 128],
                              hPrivateDataL->IGF_Common_channel_data_handle->virtualSpec,
                              hPrivateDataR->IGF_Common_channel_data_handle->virtualSpec,
                              IGFstartLine, IGFstopLine, TNF_mask, TNF_maskL, TNF_maskR,
                              flag_TNF_active);

      winOffset++;
    } /* for win */

  } /* for grp */
}

/**********************************************************************/ /**
 copies spectral data to the internal IGF representation
 - applies INF
 - applies whitening
 **************************************************************************/

static void iisIGFDecLibInjectSourceSpectrum(
    IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticData, IGF_PRIVATE_DATA_HANDLE hPrivateData,
    FIXP_DBL* pSpectralData,                     /**< ptr to spectral window data       */
    SHORT* specScale, const INT window_sequence, /**< in: 1==SHORT 0==LONG window seq    */
    const INT frameType) {
  IGF_GRID_INFO_HANDLE hGrid;
  IGF_MAP_INFO_HANDLE hMap;
  IGF_WORK_MEMORY_HANDLE hWorkMem;
  short numWindows;
  int copyNr;

  iisIGFDecLibCheckTileTransition(hPrivateStaticData, hPrivateData, window_sequence, frameType);

  hGrid = &hPrivateStaticData->sGridInfoTab[window_sequence];

  /* Set parameters for long and short blocks */
  if (window_sequence == IGF_GRID_SHORT_WINDOW) {
    numWindows = 8;
    copyNr = 1024;
  } else {
    numWindows = 1;
    copyNr = hGrid->iIGFStartLine;
  }

  for (UCHAR t = 0; t < hGrid->iIGFNumTile; t++) {
    hMap = &hGrid->sIGFMapInfoTab[t];

    /* copy from pSpectralData to the internal representation: */
    FDKmemcpy(hMap->fSpectrumTab[0], pSpectralData, sizeof(FIXP_DBL) * copyNr);
  }

  for (UCHAR t = 0; t < hGrid->iIGFNumTile; t++) {
    /* Copy the scale values for windows in an internal array */
    hWorkMem = &(hPrivateData->IGF_Common_channel_data_handle->IGF_WorkingMem[t]);

    for (short win = 0; win < numWindows; win++) {
      hWorkMem->Initial_exp[win * IGF_MAX_SFB_SHORT] = (SCHAR)specScale[win];
    }
  }
}

void iisIGFDecLibInjectSourceSpectrumNew(
    IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticData, IGF_PRIVATE_DATA_HANDLE hPrivateData,
    FIXP_DBL* pSpectralData,                             /**< ptr to spectral window data       */
    SHORT* pSpectralData_exp, const INT window_sequence, /**< in: 1==SHORT 0==LONG window seq    */
    const INT frameType) {
  IGF_GRID_INFO_HANDLE hGrid;
  IGF_MAP_INFO_HANDLE hMap;
  INT t;

  iisIGFDecLibCheckTileTransition(hPrivateStaticData, hPrivateData, window_sequence, frameType);

  hGrid = &hPrivateStaticData->sGridInfoTab[window_sequence];

  for (t = 0; t < hGrid->iIGFNumTile; t++) {
    /* copy from pSpectralData scaling to the internal representation: */
    FDKmemcpy(hPrivateStaticData->fSpectrumTab_sfb_exp[t], pSpectralData_exp,
              sizeof(SHORT) * IGF_MAX_WIN * IGF_MAX_SFB_SHORT);

  } /* for( t = 0; t < hGrid->iIGFNumTile; t++ ) */

  for (t = 0; t < hGrid->iIGFNumTile; t++) {
    hMap = &hGrid->sIGFMapInfoTab[t];

    /* copy from pSpectralData to the internal representation: */
    FDKmemcpy(hMap->fSpectrumTab[0], pSpectralData, sizeof(FIXP_DBL) * 1024);

  } /* for( t = 0; t < hGrid->iIGFNumTile; t++ ) */
}

/*=======================================================================*/
/* Encoder Functions                                                    */
/*=======================================================================*/
/**********************************************************************/ /**
 pass sfb_offset[len] for shortblocks!
 \return returns IGF start sfb for short blocks
 **************************************************************************/
static INT iisIGFGetStopSfbSB(
    const INT igfStopSubbandLB, /**< in: igf stop subband for long blocks! */
    const SHORT* sfb_offset,    /**< in: scalefactor band offset table; short block table! */
    const INT len               /**< in: length of scalefactor band offset table */
) {
  INT sfb;
  INT igfStopSfbSB = len;

  for (sfb = 0; sfb < len; sfb++) {
    if (sfb_offset[sfb] >= igfStopSubbandLB >> 3) {
      igfStopSfbSB = sfb;
      break;
    }
  }

  return igfStopSfbSB;
}

/**********************************************************************/ /**
 pass sfb_offset[len] for longblocks!
 \return returns IGF stop sfb
 **************************************************************************/
static INT iisIGFGetStopSfbLB(
    const INT igfStartSfbLB, /**< in: calculated start sfb for long blocks */
    const INT igfStopIdx,    /**< in: igfStopIdx from bitsream */
    const SHORT* sfb_offset, /**< in: scalefactor band offset table */
    const INT len            /**< in: length of scalefactor band offset table */
) {
  INT retVal = -1;

  if (igfStopIdx != 15) {
    retVal = fMin(len, fMax(igfStartSfbLB + (((len - (igfStartSfbLB + 1)) * (igfStopIdx + 2)) >> 4),
                            igfStartSfbLB + 1));
  } else {
    retVal = len;
  }
  return retVal;
}

/**********************************************************************/ /**
 pass sfb_offset[len] for longblocks!
 \return returns IGF start sfb
 **************************************************************************/
static INT iisIGFGetStartSfbLB(const INT igfStartIdx, /**< in: igfStartIdx from bitsream */
                               const INT len /**< in: length of scalefactor band offset table */
) {
  INT retVal = -1;

  retVal = fMin(11 + igfStartIdx, len - 5);

  return retVal;
}

/**********************************************************************/ /**
 pass sfb_offset[len] for shortblocks!
 \return returns IGF start sfb for short blocks
 **************************************************************************/
static INT iisIGFGetStartSfbSB(
    const INT igfStartSubbandLB, /**< in: igf start subband for long blocks! */
    const SHORT* sfb_offset,     /**< in: scalefactor band offset table; short block table! */
    const INT len                /**< in: length of scalefactor band offset table */
) {
  INT sfb;
  INT igfStartSfbSB = len;

  for (sfb = 0; sfb < len; sfb++) {
    if (sfb_offset[sfb] >= igfStartSubbandLB >> 3) {
      igfStartSfbSB = sfb;
      break;
    }
  }

  return igfStartSfbSB;
}

/* Initialize */
void iisIGFDecLibInit(
    IGF_PRIVATE_STATIC_DATA* hPrivateStaticData, /**< inout: instance handle of MPEG-H */
    IGF_PRIVATE_DATA* hPrivateData,              /**< inout: instance handle of MPEG-H */
    IGF_PRIVATE_DATA_COMMON* hPrivateCommonData, const UCHAR igfStartIndex,
    const UCHAR igfStopIndex, const UCHAR igfUseHighRes, const UCHAR igfUseWhitening,
    const UINT aacSampleRate, const INT aacFrameLength, const SHORT* sfb_offset_LB,
    const INT len_LB, const SHORT* sfb_offset_SB, const INT len_SB,
    const UCHAR enhancedNoiseFilling, const UCHAR igfAfterTnsSynth,
    const UCHAR igfIndependentTiling) {
  /*Assign pointer to common data memory for all channel elements*/
  hPrivateData->IGF_Common_channel_data_handle = hPrivateCommonData;

  UCHAR fixedTileIdx[IGF_MAX_TILES] = {3, 3, 3, 3};

  hPrivateStaticData->aacFrameLength = aacFrameLength;

  hPrivateStaticData->useHighRes = igfUseHighRes;

  hPrivateStaticData->igfStartSfbLB = (UCHAR)iisIGFGetStartSfbLB(igfStartIndex, len_LB);

  hPrivateStaticData->igfStopSfbLB = (UCHAR)iisIGFGetStopSfbLB(hPrivateStaticData->igfStartSfbLB,
                                                               igfStopIndex, sfb_offset_LB, len_LB);

  SHORT startLine = sfb_offset_LB[hPrivateStaticData->igfStartSfbLB];

  SHORT stopLine = sfb_offset_LB[hPrivateStaticData->igfStopSfbLB];

  hPrivateStaticData->igfStartSfbSB = (UCHAR)iisIGFGetStartSfbSB(startLine, sfb_offset_SB, len_SB);

  hPrivateStaticData->igfStopSfbSB = (UCHAR)iisIGFGetStopSfbSB(stopLine, sfb_offset_SB, len_SB);

  hPrivateStaticData->shortBlockSize = 128;
  hPrivateStaticData->longBlockSize = 1024;

  hPrivateStaticData->numSfbLB = len_LB;
  hPrivateStaticData->sfbOffsetLB = sfb_offset_LB;

  hPrivateStaticData->numSfbSB = len_SB;
  hPrivateStaticData->sfbOffsetSB = sfb_offset_SB;

  FIXP_DBL temp;
  int exp;

  /* hPrivateStaticData->igfMinSubbandLB  = INT( (1125 * aacFrameLength ) / (aacSampleRate >> 1) );
   */
  temp = fDivNorm((FIXP_DBL)(1125 * aacFrameLength), (FIXP_DBL)(aacSampleRate >> 1), &exp);
  hPrivateStaticData->igfMinSubbandLB = INT(temp >> (31 - exp));
  hPrivateStaticData->igfMinSubbandLB += hPrivateStaticData->igfMinSubbandLB % 2;

  /* hPrivateStaticData->igfMinSubbandSB  = INT( (1125 * aacFrameLength >> 3 ) / (aacSampleRate >>
   * 1) ); */
  temp = fDivNorm((FIXP_DBL)(1125 * (aacFrameLength >> 3)), (FIXP_DBL)(aacSampleRate >> 1), &exp);
  hPrivateStaticData->igfMinSubbandSB = INT(temp >> (31 - exp));
  hPrivateStaticData->igfMinSubbandSB += hPrivateStaticData->igfMinSubbandSB % 2;

  hPrivateStaticData->igfUseWhitening = igfUseWhitening;

  hPrivateStaticData->enhancedNoiseFilling = enhancedNoiseFilling;
  hPrivateStaticData->igfAfterTnsSynth = igfAfterTnsSynth;
  hPrivateStaticData->igfIndependentTiling = igfIndependentTiling;

  hPrivateStaticData->igfNTilesLB =
      get_IGF_tile(hPrivateStaticData->igfMinSubbandLB, hPrivateStaticData->igfStartSfbLB,
                   hPrivateStaticData->igfStopSfbLB, hPrivateStaticData->useHighRes,
                   hPrivateStaticData->sfbOffsetLB, 0, /* swb shift factor */
                   hPrivateStaticData->igfTilesLB);

  hPrivateStaticData->igfNTilesSB =
      get_IGF_tile(hPrivateStaticData->igfMinSubbandSB, hPrivateStaticData->igfStartSfbSB,
                   hPrivateStaticData->igfStopSfbSB, hPrivateStaticData->useHighRes,
                   hPrivateStaticData->sfbOffsetSB, 0, /* swb shift factor */
                   hPrivateStaticData->igfTilesSB);

  iisIGFSCFDecLibInit(&(hPrivateStaticData->hIGFSCFDecoder),
                      hPrivateStaticData->igfStopSfbLB - hPrivateStaticData->igfStartSfbLB,
                      hPrivateStaticData->igfStopSfbSB - hPrivateStaticData->igfStartSfbSB,
                      hPrivateStaticData->igfStopSfbSB - hPrivateStaticData->igfStartSfbSB);

  /* AAC long blocks */
  iisIGFDecLibInitGridInfo(hPrivateData->IGF_Common_channel_data_handle->Spectrum_tab_array,
                           &hPrivateStaticData->sGridInfoTab[IGF_GRID_LONG_WINDOW],
                           IGF_GRID_LONG_WINDOW, hPrivateStaticData->sfbOffsetLB,
                           hPrivateStaticData->numSfbLB, hPrivateStaticData->igfStartSfbLB,
                           hPrivateStaticData->igfStopSfbLB, hPrivateStaticData->igfMinSubbandLB,
                           hPrivateStaticData->useHighRes, hPrivateStaticData->igfNTilesLB,
                           hPrivateStaticData->igfTilesLB, fixedTileIdx);

  /* AAC short blocks */
  iisIGFDecLibInitGridInfo(hPrivateData->IGF_Common_channel_data_handle->Spectrum_tab_array,
                           &hPrivateStaticData->sGridInfoTab[IGF_GRID_SHORT_WINDOW],
                           IGF_GRID_SHORT_WINDOW, hPrivateStaticData->sfbOffsetSB,
                           hPrivateStaticData->numSfbSB, hPrivateStaticData->igfStartSfbSB,
                           hPrivateStaticData->igfStopSfbSB, hPrivateStaticData->igfMinSubbandSB,
                           hPrivateStaticData->useHighRes, hPrivateStaticData->igfNTilesSB,
                           hPrivateStaticData->igfTilesSB, fixedTileIdx);

  /* reset IGF */
  iisIGFDecLibResetIGF(hPrivateStaticData, hPrivateData, IGF_FRAME_DIVISION_AAC_OR_TCX_LONG);

  iisIGFDecLibResetIGF(hPrivateStaticData, hPrivateData, IGF_FRAME_DIVISION_TCX_SHORT_1);
}

FIXP_DBL* iisIGFDecLibAccessSourceSpectrum(
    IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticData, IGF_PRIVATE_DATA_HANDLE hPrivateData,
    const INT tileIdx, const INT window_sequence, /**< in: 1==SHORT 0==LONG window seq */
    const INT win) {
  IGF_GRID_INFO_HANDLE hGrid = NULL;
  IGF_MAP_INFO_HANDLE hMap = NULL;

  hGrid = &hPrivateStaticData->sGridInfoTab[window_sequence];

  FDK_ASSERT(win >= 0);
  if (window_sequence == IGF_GRID_LONG_WINDOW) {
    FDK_ASSERT(win == 0);
  }
  if (window_sequence == IGF_GRID_SHORT_WINDOW) {
    FDK_ASSERT(win <= 7);
  }

  if (tileIdx >= hGrid->iIGFNumTile) return NULL;

  hMap = &hGrid->sIGFMapInfoTab[tileIdx];
  return hMap->fSpectrumTab[0] + win * 128;
}

SHORT* iisIGFDecLibAccessSourceSpectrum_exponent(
    IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticData, const INT tileIdx,
    const INT window_sequence /**< in: 1==SHORT 0==LONG window seq */
) {
  FDK_ASSERT(tileIdx <= hPrivateStaticData->sGridInfoTab[window_sequence].iIGFNumTile);

  return hPrivateStaticData->fSpectrumTab_sfb_exp[tileIdx];
}

SHORT* iisIGFDecLibAccessSourceSpectrum_Scale(
    IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticData, IGF_PRIVATE_DATA_HANDLE hPrivateData,
    const INT tileIdx, const INT window_sequence /**< in: 1==SHORT 0==LONG window seq */
) {
  FDK_ASSERT(tileIdx < hPrivateStaticData->sGridInfoTab[window_sequence].iIGFNumTile);

  return hPrivateStaticData->fSpectrumScale[tileIdx];
}

INT iisIGFDecLibGetNumberOfTiles(IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticData,
                                 const INT window_sequence /**< in: 1==SHORT 0==LONG window seq */
) {
  return hPrivateStaticData->sGridInfoTab[window_sequence].iIGFNumTile;
}

/********************************************************
   IGF TNF functions
*********************************************************/
#define TNF_MAX_FILTER_ORDER 16
#define TNF_MAX_SUBDIVISIONS 3
#define TNF_MAX_SUBDIVISIONS_EXP 2

static void DotProduct(FIXP_DBL* Output, INT* Output_exp, FIXP_DBL* Input, INT Input_exp,
                       INT length) {
  INT shift = getScalefactor(Input, length);

  const int loop_shift = DFRACT_BITS - fNormz((FIXP_DBL)length);

  FIXP_DBL acc;
#ifdef FUNCTION_DotProduct_func1
  acc = DotProduct_func1(Input, shift, loop_shift, length);
#else
  acc = (FIXP_DBL)0;
  for (int i = 0; i < length; i++) {
    FIXP_DBL temp = fPow2Div2(Input[i] << shift);
    acc += (temp >> loop_shift);
  }
#endif
  *Output = acc;
  *Output_exp = ((Input_exp - shift) << 1) + 1 + loop_shift;
}

static FIXP_DBL Same_Exponent_Correlation(FIXP_DBL* X, FIXP_DBL* Y, INT* Exp, INT signal_length,
                                          INT correlation_length) {
  INT shift = getScalefactor(X, signal_length);

  /* Find a suitable shift for dmx addition */
  INT loop_shift = 31 - fixnorm_D(correlation_length);

  if (loop_shift >= 1) loop_shift--;

  FIXP_DBL acc;
#ifdef FUNCTION_Same_Exponent_Correlation_func1
  acc = Same_Exponent_Correlation_func1(X, Y, shift, loop_shift, correlation_length);
#else
  acc = (FIXP_DBL)0;
  for (int i = 0; i < correlation_length; i++) {
    FIXP_DBL temp = fMultDiv2(X[i] << shift, Y[i] << shift);
    acc += (temp >> loop_shift);
  }
#endif
  *Exp = ((*Exp - shift) << 1) + 1 + loop_shift;

  return acc;
}

/*************************************************************************
static data for this module
*************************************************************************/
static FIXP_DBL const tnsAcfWindow[8] = {
    /* 0.997803f,   0.991211f,  0.980225f,  0.964844f,  0.945068f,  0.920898f,  0.892334f, 0.859375f
     */
    /* In Q0.31 */
    0x7fb80200, 0x7ee00080, 0x7d780380, 0x7b800200, 0x78f7fd00,
    0x75dffc80, 0x72380000, 0x6e000000

};

#include "FDK_lpc.h"

FIXP_DBL Autocorrelation(FIXP_DBL* x, INT* x_exp, INT n, INT lag) {
  FIXP_DBL Output;

  Output = Same_Exponent_Correlation(x, x + lag, x_exp, n, n - lag);

  return Output;
}

static void iisIGF_TNFdetect(FIXP_DBL* p2_spectrum, const SHORT* specScale, const SHORT igfBgn,
                             const SHORT igfEnd, const UCHAR maxOrder, FIXP_DBL* Aarray,
                             INT* AarrayExp, UCHAR* curr_order) {
  FIXP_DBL norms[TNF_MAX_SUBDIVISIONS] = {0};
  INT norms_exp[TNF_MAX_SUBDIVISIONS] = {0};
  const int nSubdivisions = TNF_MAX_SUBDIVISIONS;
  int iSubdivisions;
  int lines[4];
  int line_diffs[3];

  /* Calculate line spacing */
  /* iStartLine = igfBgn + (igfEnd-igfBgn)*iSubdivisions/nSubdivisions;
     int const iEndLine = igfBgn + (igfEnd-igfBgn)*(iSubdivisions+1)/nSubdivisions; */

  /* 1/3 in Q0.31 = 0x2aaaaaab; 2/3 in Q0.31 = 0x55555555 */
  const FIXP_DBL OneThird = 0x2aaaaaab;

  FIXP_SGL span = (FIXP_SGL)(igfEnd - igfBgn);
  lines[0] = igfBgn; /* iSubdivisions =0 */
  int comp = (int)(fMult(OneThird, span) >> 16);
  lines[1] = lines[0] + comp;
  comp = (int)(fMult(OneThird, (FIXP_SGL)(span << 1)) >> 16);
  lines[2] = lines[0] + comp;
  lines[3] = igfEnd; /* iSubdivisions +1 = 3 */

  line_diffs[0] = lines[1] - lines[0];
  line_diffs[1] = lines[2] - lines[1];
  line_diffs[2] = lines[3] - lines[2];

  /* Calculate norms for each spectrum part */
  for (iSubdivisions = 0; iSubdivisions < nSubdivisions; iSubdivisions++) {
    DotProduct(&norms[iSubdivisions], &norms_exp[iSubdivisions],
               (p2_spectrum + lines[iSubdivisions]), specScale[0], line_diffs[iSubdivisions]);
  }

  /* If a norm is lower than threshold, return */
  INT exit_flag;
  for (iSubdivisions = 0; iSubdivisions < nSubdivisions; iSubdivisions++) {
    /* Minimum value is 1 in Q3.28: 0.0000000037252902984619140625 */
    exit_flag = fIsLessThan(norms[iSubdivisions], norms_exp[iSubdivisions], 1, 3);
    if (exit_flag) break;
  }
  /* Return if the condition is met*/
  if (exit_flag) return;

  /* Calculate normalized autocorrelation for spectrum subdivision */
  FIXP_DBL rxx[TNF_MAX_SUBDIVISIONS][TNF_MAX_FILTER_ORDER + 1];
  INT rxx_exp[TNF_MAX_SUBDIVISIONS][TNF_MAX_FILTER_ORDER + 1];
  const INT spectrumLength = igfEnd - igfBgn;

  for (iSubdivisions = 0; iSubdivisions < nSubdivisions; iSubdivisions++) {
    FIXP_DBL norm_value = norms[iSubdivisions];
    INT fac_exp = norms_exp[iSubdivisions];

    FIXP_DBL const fac = invFixp(norm_value, &fac_exp);

    const FIXP_DBL* pWindow = tnsAcfWindow;

    for (INT lag = 1; lag <= (INT)maxOrder; lag++) {
      FIXP_DBL temp = fMult(fac, *pWindow++);

      INT autocorr_exp = specScale[0];
      FIXP_DBL autocorr = Autocorrelation(p2_spectrum + lines[iSubdivisions], &autocorr_exp,
                                          line_diffs[iSubdivisions], lag);

      rxx[iSubdivisions][lag] = fMult(temp, autocorr);
      rxx_exp[iSubdivisions][lag] = autocorr_exp + fac_exp;

      if ((LONG)rxx[iSubdivisions][lag] == 0) rxx_exp[iSubdivisions][lag] = -1000;
    } /* for(INT lag=1; */

  } /* for(iSubdivisions=0 ...*/

  /* Make the exponents for different lag values equal and add the results */
  FIXP_DBL StoreValues[9];
  int maxExp = -1000;
  INT rxx_headroom;
  int min_rxx_headroom = 31;
  INT StoreValues_Exp;
  for (INT lag = 1; lag <= (INT)maxOrder; lag++) {
    /* Find the maximum exponent */
    for (iSubdivisions = 0; iSubdivisions < nSubdivisions; iSubdivisions++) {
      if (maxExp < rxx_exp[iSubdivisions][lag]) {
        maxExp = rxx_exp[iSubdivisions][lag];
      }
      rxx_headroom = fixnorm_D(rxx[iSubdivisions][lag]);
      if (((LONG)rxx[iSubdivisions][lag] != 0) && (rxx_headroom < min_rxx_headroom)) {
        min_rxx_headroom = rxx_headroom;
      }
    }
  }

  maxExp = maxExp + 2;

  for (INT lag = 1; lag <= (INT)maxOrder; lag++) {
    /* Shift and add values and store the exponent */
    StoreValues[lag] = (FIXP_DBL)0;
    for (iSubdivisions = 0; iSubdivisions < nSubdivisions; iSubdivisions++) {
      rxx[iSubdivisions][lag] <<= min_rxx_headroom;
      int shift = fMin(31, (maxExp - rxx_exp[iSubdivisions][lag]));
      rxx[iSubdivisions][lag] >>= shift;
      StoreValues[lag] += rxx[iSubdivisions][lag];
    }
  }

  StoreValues_Exp = maxExp - min_rxx_headroom;

  /* Allign maximum left for high precision */
  INT shiftStV = getScalefactor(StoreValues + 1, (INT)maxOrder);
  for (int i = 1; i <= (INT)maxOrder; i++) {
    StoreValues[i] <<= shiftStV;
  }
  StoreValues_Exp -= shiftStV;

  /* Adjust exponent of the rest of the values to fit the maximum (TNF_MAX_SUBDIVISIONS_EXP) */
  INT diff = (INT)TNF_MAX_SUBDIVISIONS_EXP - StoreValues_Exp;
  diff = fMin(31, diff);

  if (diff > 0) {
    for (int i = 1; i <= (INT)maxOrder; i++) {
      StoreValues[i] >>= diff;
    }
    StoreValues_Exp = (INT)TNF_MAX_SUBDIVISIONS_EXP;
  }

  /* Assign value and scale to fit the rest of the values */
  StoreValues[0] = scaleValue((FIXP_DBL)nSubdivisions, 31 - StoreValues_Exp);

  INT FilterOrder = fMin((INT)maxOrder, spectrumLength >> 2);
  CLpc_AutoToLpcIGF(Aarray, AarrayExp, StoreValues, StoreValues_Exp, FilterOrder, NULL);

  *curr_order = maxOrder;
}

#ifndef FUNCTION_iisIGF_TNFfilter
static void iisIGF_TNFfilter(FIXP_DBL* p2_spectrum, const SHORT specScale, const SHORT numOfLines,
                             const UCHAR FilterOrder, const FIXP_DBL* Aarray, const INT AarrayExp) {
  /* Find the available headroom of the downmix signal and a suitable shift value*/
  INT head_shift = getScalefactor(p2_spectrum, numOfLines);

  if (head_shift == 31) head_shift = 0;

  INT OutputShift = AarrayExp - head_shift + 2;
  if (OutputShift > 31) OutputShift = 31;
  if (OutputShift < -31) OutputShift = -31;

  FDK_ASSERT(FilterOrder == 8); /* if assert fails, FUNCTION_iisIGF_TNFfilter_func1 optimization
                                   needs to be undefined before removing assert */
#ifdef FUNCTION_iisIGF_TNFfilter_func1
  iisIGF_TNFfilter_func1(p2_spectrum, OutputShift, head_shift, Aarray, (INT)numOfLines);
#else
  FIXP_DBL acc, tmp;
  INT i, j;

  /* First part is running backwards with full filter length */
  for (j = numOfLines - 1; j >= (FilterOrder - 1); j--) {
    acc = (FIXP_DBL)0;
    for (i = 0; i < FilterOrder; i++) {
      tmp = fMult(p2_spectrum[j - i] << head_shift, Aarray[i]);
      acc += tmp >> 2;
    }
    p2_spectrum[j] = scaleValue(acc, OutputShift);
  }

  /* second part is running with decreasing filter length */
  for (; j >= 0; j--) {
    acc = (FIXP_DBL)0;
    for (i = 0; i <= j; i++) {
      tmp = fMult(p2_spectrum[j - i] << head_shift, Aarray[i]);
      acc += tmp >> 2;
    }
    p2_spectrum[j] = scaleValue(acc, OutputShift);
  }
#endif
}
#endif /* #ifndef FUNCTION_iisIGF_TNFfilter */

void CIgf_TNF_apply(IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticData,
                    IGF_PRIVATE_DATA_HANDLE hPrivateData, FIXP_DBL* p2_spectrum, SHORT* specScale,
                    const UCHAR* TNF_mask, const INT window_sequence, const INT frameType) {
  /* Start only if triggered */
  if (hPrivateData->bitstreamData[frameType].igfUseEnfFlat) {
    FIXP_DBL* virtualSpec = hPrivateData->IGF_Common_channel_data_handle->virtualSpec;
    FIXP_DBL Aarray[TNF_MAX_FILTER_ORDER + 1];
    INT AarrayExp;
    SHORT igfBgn = hPrivateStaticData->sGridInfoTab[window_sequence].iIGFStartLine;
    SHORT igfEnd = hPrivateStaticData->sGridInfoTab[window_sequence].iIGFStopLine;
    const UCHAR maxOrder = 8;
    UCHAR curr_order = 0;

    /* Filter coefficient array. The first value is always 1 */
    FDKmemclear(Aarray, (TNF_MAX_FILTER_ORDER + 1) * sizeof(FIXP_DBL));

    /* "virtualSpec" contains a modified IGF spectrum values.  Typically IGF fills the zero-gaps
    with values from the base spectrum and leaves the non-zero values intact. In this spectrum
    however all values are filled from the lower spectrum. Just later (at the end of the algorithm)
    the non-zero values are put back in the spectrum. */

    iisIGF_TNFdetect(virtualSpec, specScale, igfBgn, igfEnd, maxOrder, Aarray + 1, &AarrayExp,
                     &curr_order);

    if (curr_order > 0) {
      /* Assign 1 to the first value of the array */
      Aarray[0] = (FIXP_DBL)0x7fffffff;
      if (AarrayExp > 0) {
        Aarray[0] = (FIXP_DBL)1 << (31 - AarrayExp);
      }

      iisIGF_TNFfilter(&virtualSpec[igfBgn], specScale[0], (igfEnd - igfBgn), curr_order, Aarray,
                       AarrayExp);
    }

    /* We make sure all values created by IGF (which were zero before IGF) are added to the non-zero
    values of the spectrum */
    for (short i = igfBgn; i < igfEnd; i++) {
      if (TNF_mask[i] == (UCHAR)0) {
        p2_spectrum[i] = virtualSpec[i];
      }
    }

  } /* if(hPrivateData->igfUseEnfFlat) */
}
