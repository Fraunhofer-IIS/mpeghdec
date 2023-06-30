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

   Description: long/short-block decoding

*******************************************************************************/

#include "block.h"

#include "aac_rom.h"
#include "FDK_bitstream.h"
#include "scale.h"
#include "FDK_tools_rom.h"

#include "ac_arith_coder.h"

#if defined(__arm__)
#include "arm/block_arm.cpp"
#endif

AAC_DECODER_ERROR CBlock_ReadScaleFactorData(CAacDecoderChannelInfo* pAacDecoderChannelInfo,
                                             HANDLE_FDK_BITSTREAM bs, UINT flags) {
  int temp;
  int band;
  int group;
  int position = 0; /* accu for intensity delta coding */
  int factor = pAacDecoderChannelInfo->pDynData->RawDataInfo
                   .GlobalGain; /* accu for scale factor delta coding */
  UCHAR* pCodeBook = pAacDecoderChannelInfo->pDynData->aCodeBook;
  SHORT* pScaleFactor = pAacDecoderChannelInfo->pDynData->aScaleFactor;
  const CodeBookDescription* hcb = &AACcodeBookDescriptionSCL;

  const USHORT(*CodeBook)[HuffmanEntries] = hcb->CodeBook;

  int ScaleFactorBandsTransmitted =
      GetScaleFactorBandsTransmitted(&pAacDecoderChannelInfo->icsInfo);
  for (group = 0; group < GetWindowGroups(&pAacDecoderChannelInfo->icsInfo); group++) {
    for (band = 0; band < ScaleFactorBandsTransmitted; band++) {
      switch (pCodeBook[band]) {
        case ZERO_HCB: /* zero book */
          pScaleFactor[band] = 0;
          break;

        default: /* decode scale factor */
          if (!((flags & (AC_USAC | AC_RSVD50 | AC_MPEGH3DA)) && band == 0 && group == 0)) {
            temp = CBlock_DecodeHuffmanWordCB(bs, CodeBook);
            factor += temp - 60; /* MIDFAC 1.5 dB */
          }
          if (255 < factor || 0 > factor) {
            return AAC_DEC_PARSE_ERROR;
          }
          pScaleFactor[band] = factor - 100;
          break;

        case INTENSITY_HCB: /* intensity steering */
        case INTENSITY_HCB2:
          temp = CBlock_DecodeHuffmanWordCB(bs, CodeBook);
          position += temp - 60;
          pScaleFactor[band] = position - 100;
          break;
      }
    }
    pCodeBook += 16;
    pScaleFactor += 16;
  }

  return AAC_DEC_OK;
}

void CBlock_ScaleSpectralData(CAacDecoderChannelInfo* pAacDecoderChannelInfo, UCHAR maxSfbs,
                              SamplingRateInfo* pSamplingRateInfo) {
  int band;
  int window;
  const SHORT* RESTRICT pSfbScale = pAacDecoderChannelInfo->pDynData->aSfbScale;
  SHORT* RESTRICT pSpecScale = pAacDecoderChannelInfo->specScale;
  int groupwin, group;
  const SHORT* RESTRICT BandOffsets =
      GetScaleFactorBandOffsets(&pAacDecoderChannelInfo->icsInfo, pSamplingRateInfo);
  SPECTRAL_PTR RESTRICT pSpectralCoefficient = pAacDecoderChannelInfo->pSpectralCoefficient;

  FDKmemclear(pSpecScale, 8 * sizeof(SHORT));

  for (window = 0, group = 0; group < GetWindowGroups(&pAacDecoderChannelInfo->icsInfo); group++) {
    for (groupwin = 0; groupwin < GetWindowGroupLength(&pAacDecoderChannelInfo->icsInfo, group);
         groupwin++, window++) {
      int SpecScale_window = pSpecScale[window];
      FIXP_DBL* pSpectrum =
          SPEC(pSpectralCoefficient, window, pAacDecoderChannelInfo->granuleLength);

      /* find scaling for current window */
      for (band = 0; band < maxSfbs; band++) {
        SpecScale_window = fMax(SpecScale_window, (int)pSfbScale[window * 16 + band]);
      }

      if (pAacDecoderChannelInfo->pDynData->TnsData.Active &&
          pAacDecoderChannelInfo->pDynData->TnsData.NumberOfFilters[window] > 0) {
        int filter_index, SpecScale_window_tns;
        int tns_start, tns_stop;

        /* Find max scale of TNS bands */
        SpecScale_window_tns = 0;
        tns_start = GetMaximumTnsBands(&pAacDecoderChannelInfo->icsInfo,
                                       pSamplingRateInfo->samplingRateIndex);
        tns_stop = 0;
        for (filter_index = 0;
             filter_index < (int)pAacDecoderChannelInfo->pDynData->TnsData.NumberOfFilters[window];
             filter_index++) {
          for (band =
                   pAacDecoderChannelInfo->pDynData->TnsData.Filter[window][filter_index].StartBand;
               band <
               pAacDecoderChannelInfo->pDynData->TnsData.Filter[window][filter_index].StopBand;
               band++) {
            SpecScale_window_tns = fMax(SpecScale_window_tns, (int)pSfbScale[window * 16 + band]);
          }
          /* Find TNS line boundaries for all TNS filters */
          tns_start = fMin(
              tns_start, (int)pAacDecoderChannelInfo->pDynData->TnsData.Filter[window][filter_index]
                             .StartBand);
          tns_stop = fMax(
              tns_stop,
              (int)pAacDecoderChannelInfo->pDynData->TnsData.Filter[window][filter_index].StopBand);
        }
        SpecScale_window_tns =
            SpecScale_window_tns + pAacDecoderChannelInfo->pDynData->TnsData.GainLd;
        FDK_ASSERT(tns_stop >= tns_start);
        /* Consider existing headroom of all MDCT lines inside the TNS bands. */
        SpecScale_window_tns -= getScalefactor(pSpectrum + BandOffsets[tns_start],
                                               BandOffsets[tns_stop] - BandOffsets[tns_start]);
        if (SpecScale_window <= 17) {
          SpecScale_window_tns++;
        }
        /* Add enough mantissa head room such that the spectrum is still representable
           after applying TNS. */
        SpecScale_window = fMax(SpecScale_window, SpecScale_window_tns);
      }

      /* store scaling of current window */
      pSpecScale[window] = SpecScale_window;

#ifdef FUNCTION_CBlock_ScaleSpectralData_func1

      CBlock_ScaleSpectralData_func1(pSpectrum, maxSfbs, BandOffsets, SpecScale_window, pSfbScale,
                                     window);

#else  /* FUNCTION_CBlock_ScaleSpectralData_func1 */
      for (band = 0; band < maxSfbs; band++) {
        int scale = fMin(DFRACT_BITS - 1, SpecScale_window - pSfbScale[window * 16 + band]);
        if (scale) {
          FDK_ASSERT(scale > 0);

          /* following relation can be used for optimizations: (BandOffsets[i]%4) == 0 for all i */
          int max_index = BandOffsets[band + 1];
          DWORD_ALIGNED(pSpectrum);
          for (int index = BandOffsets[band]; index < max_index; index++) {
            pSpectrum[index] >>= scale;
          }
        }
      }
#endif /* FUNCTION_CBlock_ScaleSpectralData_func1 */
    }
  }
}

void CBlock_ScaleTileData(CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo,
                          CAacDecoderChannelInfo* pAacDecoderChannelInfo, UCHAR maxSfbs,
                          SamplingRateInfo* pSamplingRateInfo) {
  int band;
  int window;

  SHORT* RESTRICT pSpecScale;
  int groupwin, group;
  const SHORT* RESTRICT BandOffsets =
      GetScaleFactorBandOffsets(&pAacDecoderChannelInfo->icsInfo, pSamplingRateInfo);

  /* Get the number of tiles */
  INT NumberOfTiles = iisIGFDecLibGetNumberOfTiles(
      &(pAacDecoderStaticChannelInfo->IGF_StaticData),
      GetWindowSequence(&pAacDecoderChannelInfo->icsInfo) == BLOCK_SHORT);

  /* Over all tiles */
  for (int tileIdx = 0; tileIdx < NumberOfTiles; tileIdx++) {
    for (window = 0, group = 0; group < GetWindowGroups(&pAacDecoderChannelInfo->icsInfo);
         group++) {
      for (groupwin = 0; groupwin < GetWindowGroupLength(&pAacDecoderChannelInfo->icsInfo, group);
           groupwin++, window++) {
        pSpecScale = iisIGFDecLibAccessSourceSpectrum_Scale(
            &pAacDecoderStaticChannelInfo->IGF_StaticData, &pAacDecoderChannelInfo->IGFdata,
            tileIdx, GetWindowSequence(&pAacDecoderChannelInfo->icsInfo) == BLOCK_SHORT);
        int SpecScale_window = 0;

        FIXP_DBL* tile_spectrum = iisIGFDecLibAccessSourceSpectrum(
            &pAacDecoderStaticChannelInfo->IGF_StaticData, &pAacDecoderChannelInfo->IGFdata,
            tileIdx, GetWindowSequence(&pAacDecoderChannelInfo->icsInfo) == BLOCK_SHORT, window);

        SHORT* sfbScale = iisIGFDecLibAccessSourceSpectrum_exponent(
            &pAacDecoderStaticChannelInfo->IGF_StaticData, tileIdx,
            GetWindowSequence(&pAacDecoderChannelInfo->icsInfo) == BLOCK_SHORT);

        /* find scaling for current window */
        for (band = 0; band < maxSfbs; band++) {
          SpecScale_window = fMax(SpecScale_window, (int)sfbScale[window * 16 + band]);
        }

        if (pAacDecoderChannelInfo->pDynData->TnsData.Active &&
            pAacDecoderChannelInfo->pDynData->TnsData.NumberOfFilters[window] > 0) {
          int filter_index, SpecScale_window_tns;
          int tns_start, tns_stop;

          /* Find max scale of TNS bands */
          SpecScale_window_tns = 0;
          tns_start = GetMaximumTnsBands(&pAacDecoderChannelInfo->icsInfo,
                                         pSamplingRateInfo->samplingRateIndex);
          tns_stop = 0;
          for (filter_index = 0;
               filter_index <
               (int)pAacDecoderChannelInfo->pDynData->TnsData.NumberOfFilters[window];
               filter_index++) {
            for (band = pAacDecoderChannelInfo->pDynData->TnsData.Filter[window][filter_index]
                            .StartBand;
                 band <
                 pAacDecoderChannelInfo->pDynData->TnsData.Filter[window][filter_index].StopBand;
                 band++) {
              SpecScale_window_tns = fMax(SpecScale_window_tns, (int)sfbScale[window * 16 + band]);
            }
            /* Find TNS line boundaries for all TNS filters */
            tns_start = fMin(tns_start, (int)pAacDecoderChannelInfo->pDynData->TnsData
                                            .Filter[window][filter_index]
                                            .StartBand);
            tns_stop = fMax(tns_stop, (int)pAacDecoderChannelInfo->pDynData->TnsData
                                          .Filter[window][filter_index]
                                          .StopBand);
          }
          SpecScale_window_tns =
              SpecScale_window_tns + pAacDecoderChannelInfo->pDynData->TnsData.GainLd;
          FDK_ASSERT(tns_stop >= tns_start);
          /* Consider existing headroom of all MDCT lines inside the TNS bands. */
          SpecScale_window_tns -= getScalefactor(tile_spectrum + BandOffsets[tns_start],
                                                 BandOffsets[tns_stop] - BandOffsets[tns_start]);
          if (SpecScale_window <= 17) {
            SpecScale_window_tns++;
          }
          /* Add enough mantissa head room such that the spectrum is still representable
             after applying TNS. */
          SpecScale_window = fMax(SpecScale_window, SpecScale_window_tns);
        }

        /* store scaling of current window */
        pSpecScale[window] = (SHORT)SpecScale_window;

#ifdef FUNCTION_CBlock_ScaleSpectralData_func1

        CBlock_ScaleSpectralData_func1(tile_spectrum, maxSfbs, BandOffsets, SpecScale_window,
                                       sfbScale, window);

#else  /* FUNCTION_CBlock_ScaleSpectralData_func1 */
        for (band = 0; band < maxSfbs; band++) {
          int scale = fMin(DFRACT_BITS - 1, SpecScale_window - sfbScale[window * 16 + band]);
          if (scale) {
            FDK_ASSERT(scale > 0);

            /* following relation can be used for optimizations: (BandOffsets[i]%4) == 0 for all i
             */
            int max_index = BandOffsets[band + 1];
            for (int index = BandOffsets[band]; index < max_index; index++) {
              tile_spectrum[index] >>= scale;
            }
          }
        }
#endif /* FUNCTION_CBlock_ScaleSpectralData_func1 */

      } /*  for (groupwin=0.... */
    }   /* for (window=0 ....*/

  } /* for(int tileIdx=0... */
}

/**
 * \brief inverse quantize one sfb.
 *        Each value of the sfb is processed according to the formula:
 *        spectrum[i] = Sign(spectrum[i]) * Matissa(spectrum[i])^(4/3) * 2^(lsb/4).
 *
 * \param spectrum         pointer to first line of the sfb to be inverse quantized.
 * \param sfbScalefactor   pointer to maximum scalefactor of sfb.
 * \param noLines          number of quantized values in sfb.
 * \param scf              scalefactor of sfb.
 */
static inline void InverseQuantizeBand(FIXP_DBL* RESTRICT spectrum, SHORT* RESTRICT sfbScalefactor,
                                       FIXP_DBL maxVal, INT noLines, INT scf) {
  INT i, q;
  INT scale;
  INT msb = scf >> 2;
  INT lsb = scf & 3;
  FIXP_DBL fac = a2ToPow0p25Mant[lsb];
  FIXP_DBL spec;

  if (maxVal < (FIXP_DBL)INV_QUANT_TABLESIZE) {
    q = (INT)maxVal;
    maxVal = fMult(InverseQuantTableMant[q], fac);
    scale = InverseQuantTableExp[q] - (CntLeadingZeros(maxVal) - 2);
    *sfbScalefactor = scale + a2ToPow0p25Exp + msb;

    for (i = noLines; i--;) {
      q = (INT)*spectrum++;
      if (q < 0) {
        q = -q;
        spec = fMult(-InverseQuantTableMant[q], fac);
        spectrum[-1] = spec >> (scale - InverseQuantTableExp[q]);
      } else if (q > 0) {
        spec = fMult(InverseQuantTableMant[q], fac);
        spectrum[-1] = spec >> (scale - InverseQuantTableExp[q]);
      }
    }
  } else {
    INT abs_q;
    scale = EvaluatePower43(&maxVal, lsb);
    scale = scale - (fNormz(maxVal) - 2);
    *sfbScalefactor = scale + a2ToPow0p25Exp + msb;

    for (i = 0; i < noLines; i++) {
      q = (INT)spectrum[i];
      abs_q = fAbs(q);
      if (abs_q < INV_QUANT_TABLESIZE) {
        if (q < 0) {
          q = -q;
          spec = fMult(-InverseQuantTableMant[q], fac);
          spectrum[i] = spec >> (scale - InverseQuantTableExp[q]);
        } else if (q > 0) {
          spec = fMult(InverseQuantTableMant[q], fac);
          spectrum[i] = spec >> (scale - InverseQuantTableExp[q]);
        }
      } else {
        INT e0, e1, idx;
        FIXP_DBL t0, t1, td;

        idx = (abs_q >> 3);
        e0 = InverseQuantTableExp[idx];
        e1 = InverseQuantTableExp[idx + 1];

        t0 = InverseQuantTableMant[idx] >> (e1 - e0);
        t1 = InverseQuantTableMant[idx + 1];

        td = fMult(t1 - t0, InverseQuantInterpolationTable[abs_q & 7]);

        spec = fMult(t0 + td, fac);

        if (q < 0) {
          spec = -spec;
        }

        spectrum[i] = spec >> (scale - e1 - InverseQuantInterpolationExp);
      }
    }
  }
}

#if !defined(FUNCTION_maxabs_D)
static inline FIXP_DBL maxabs_D(const FIXP_DBL* pSpectralCoefficient, const int noLines) {
  /* Find max spectral line value of the current sfb */
  FIXP_DBL locMax = (FIXP_DBL)0;
  int i;

  DWORD_ALIGNED(pSpectralCoefficient);
  FDK_PRAGMA_MUST_ITERATE(noLines, 4, 2048, 4)

  for (i = noLines; i-- > 0;) {
    /* Expensive memory access */
    locMax = fMax(fixp_abs(pSpectralCoefficient[i]), locMax);
  }

  return locMax;
}
#endif

AAC_DECODER_ERROR CBlock_InverseQuantizeSpectralData(CAacDecoderChannelInfo* pAacDecoderChannelInfo,
                                                     SamplingRateInfo* pSamplingRateInfo,
                                                     UCHAR* band_is_noise,
                                                     UCHAR active_band_search) {
  int window, group, groupwin, band;
  int ScaleFactorBandsTransmitted =
      GetScaleFactorBandsTransmitted(&pAacDecoderChannelInfo->icsInfo);
  UCHAR* RESTRICT pCodeBook = pAacDecoderChannelInfo->pDynData->aCodeBook;
  SHORT* RESTRICT pSfbScale = pAacDecoderChannelInfo->pDynData->aSfbScale;
  SHORT* RESTRICT pScaleFactor = pAacDecoderChannelInfo->pDynData->aScaleFactor;
  const SHORT* RESTRICT BandOffsets =
      GetScaleFactorBandOffsets(&pAacDecoderChannelInfo->icsInfo, pSamplingRateInfo);
  const SHORT total_bands = GetScaleFactorBandsTotal(&pAacDecoderChannelInfo->icsInfo);

  FDKmemclear(pAacDecoderChannelInfo->pDynData->aSfbScale, (8 * 16) * sizeof(SHORT));

  for (window = 0, group = 0; group < GetWindowGroups(&pAacDecoderChannelInfo->icsInfo); group++) {
    for (groupwin = 0; groupwin < GetWindowGroupLength(&pAacDecoderChannelInfo->icsInfo, group);
         groupwin++, window++) {
      /* inverse quantization */
      for (band = 0; band < ScaleFactorBandsTransmitted; band++) {
        FIXP_DBL* pSpectralCoefficient = SPEC(pAacDecoderChannelInfo->pSpectralCoefficient, window,
                                              pAacDecoderChannelInfo->granuleLength) +
                                         BandOffsets[band];
        FIXP_DBL locMax;

        const int noLines = BandOffsets[band + 1] - BandOffsets[band];
        const int bnds = group * 16 + band;

        if ((pCodeBook[bnds] == ZERO_HCB) || (pCodeBook[bnds] == INTENSITY_HCB) ||
            (pCodeBook[bnds] == INTENSITY_HCB2))
          continue;

        locMax = maxabs_D(pSpectralCoefficient, noLines);

        if (active_band_search) {
          if (locMax != FIXP_DBL(0)) {
            band_is_noise[group * 16 + band] = 0;
          }
        }

        /* Cheap robustness improvement - Do not remove!!! */
        if (fixp_abs(locMax) > (FIXP_DBL)MAX_QUANTIZED_VALUE) {
          return AAC_DEC_PARSE_ERROR;
        }

        /* Added by Youliy Ninov:
        The inverse quantization operation is given by (ISO/IEC 14496-3:2009(E)) by:

        x_invquant=Sign(x_quant). abs(x_quant)^(4/3)

        We apply a gain, derived from the scale factor for the particular sfb, according to the
        following function:

        gain=2^(0.25*ScaleFactor)

        So, after scaling we have:

        x_rescale=gain*x_invquant=Sign(x_quant)*2^(0.25*ScaleFactor)*abs(s_quant)^(4/3)

        We could represent the ScaleFactor as:

        ScaleFactor= (ScaleFactor >> 2)*4 + ScaleFactor %4

        When we substitute it we get:

        x_rescale=Sign(x_quant)*2^(ScaleFactor>>2)* ( 2^(0.25*(ScaleFactor%4))*abs(s_quant)^(4/3))

        When we set: msb=(ScaleFactor>>2) and lsb=(ScaleFactor%4), we obtain:

        x_rescale=Sign(x_quant)*(2^msb)* ( 2^(lsb/4)*abs(s_quant)^(4/3))

        The rescaled output can be represented by:
           mantissa : Sign(x_quant)*( 2^(lsb/4)*abs(s_quant)^(4/3))
           exponent :(2^msb)

        */

        int msb = pScaleFactor[bnds] >> 2;

        /* Inverse quantize band only if it is not empty */
        if (locMax != FIXP_DBL(0)) {
          InverseQuantizeBand(pSpectralCoefficient, &pSfbScale[window * 16 + band], locMax, noLines,
                              pScaleFactor[bnds]);
        } else {
          pSfbScale[window * 16 + band] = msb;
        }

      } /* for (band=0; band < ScaleFactorBandsTransmitted; band++) */

      /* Make sure the array is cleared to the end */
      SHORT start_clear = BandOffsets[ScaleFactorBandsTransmitted];
      SHORT end_clear = BandOffsets[total_bands];
      int diff_clear = (int)(end_clear - start_clear);
      FIXP_DBL* pSpectralCoefficient = SPEC(pAacDecoderChannelInfo->pSpectralCoefficient, window,
                                            pAacDecoderChannelInfo->granuleLength) +
                                       start_clear;
      FDKmemclear(pSpectralCoefficient, diff_clear * sizeof(FIXP_DBL));

    } /* for (groupwin=0; groupwin < GetWindowGroupLength(&pAacDecoderChannelInfo->icsInfo,group);
         groupwin++, window++) */
  } /* for (window=0, group=0; group < GetWindowGroups(&pAacDecoderChannelInfo->icsInfo); group++)*/

  return AAC_DEC_OK;
}

static const FIXP_SGL noise_level_tab[8] = {
    /* FDKpow(2, (float)(noise_level-14)/3.0f) * 2; (*2 to compensate for fMultDiv2)
       noise_level_tab(noise_level==0) == 0 by definition
    */
    FX_DBL2FXCONST_SGL(0x00000000 /*0x0a145173*/),
    FX_DBL2FXCONST_SGL(0x0cb2ff5e),
    FX_DBL2FXCONST_SGL(0x10000000),
    FX_DBL2FXCONST_SGL(0x1428a2e7),
    FX_DBL2FXCONST_SGL(0x1965febd),
    FX_DBL2FXCONST_SGL(0x20000000),
    FX_DBL2FXCONST_SGL(0x28514606),
    FX_DBL2FXCONST_SGL(0x32cbfd33)};

void CBlock_ApplyNoise(CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo,
                       CAacDecoderChannelInfo* pAacDecoderChannelInfo,
                       SamplingRateInfo* pSamplingRateInfo, ULONG* nfRandomSeed,
                       UCHAR* band_is_noise, INT flag_INFactive, INT igf_StereoFilling,
                       INT max_noise_sfb) {
  const SHORT* swb_offset =
      GetScaleFactorBandOffsets(&pAacDecoderChannelInfo->icsInfo, pSamplingRateInfo);
  int g, win, gwin, sfb, noiseFillingStartOffset, nfStartOffset_sfb;

  /* Obtain noise level and scale factor offset. */
  int noise_level =
      pAacDecoderChannelInfo->pDynData->specificTo.usac.fd_noise_level_and_offset >> 5;
  const FIXP_SGL noiseVal_pos = noise_level_tab[noise_level];

  /* noise_offset can change even when noise_level=0. Neccesary for IGF stereo filling */
  const int noise_offset =
      (pAacDecoderChannelInfo->pDynData->specificTo.usac.fd_noise_level_and_offset & 0x1f) - 16;

  noiseFillingStartOffset =
      (GetWindowSequence(&pAacDecoderChannelInfo->icsInfo) == BLOCK_SHORT) ? 20 : 160;
  if (pAacDecoderChannelInfo->granuleLength == 96) {
    noiseFillingStartOffset = (3 * noiseFillingStartOffset) /
                              4; /* scale offset with 3/4 for coreCoderFrameLength == 768 */
  }

  /* determine sfb from where on noise filling is applied */
  for (sfb = 0; swb_offset[sfb] < noiseFillingStartOffset; sfb++)
    ;
  nfStartOffset_sfb = sfb;

  int IGFNumTile = iisIGFDecLibGetNumberOfTiles(
      &(pAacDecoderStaticChannelInfo->IGF_StaticData),
      GetWindowSequence(&pAacDecoderChannelInfo->icsInfo) == BLOCK_SHORT);
  FIXP_DBL* Tile_pointer_Array[4];

  if ((flag_INFactive) && (!pAacDecoderChannelInfo->IGFdata.bitstreamData[0].igf_allZero) &&
      (IGFNumTile > 0)) {
    /* Create an array of tile begin pointers */
    for (int tileIdx = 0; tileIdx < IGFNumTile; tileIdx++) {
      FIXP_DBL* pSpec_Tile = iisIGFDecLibAccessSourceSpectrum(
          &pAacDecoderStaticChannelInfo->IGF_StaticData, &pAacDecoderChannelInfo->IGFdata, tileIdx,
          GetWindowSequence(&pAacDecoderChannelInfo->icsInfo) == BLOCK_SHORT, 0);

      Tile_pointer_Array[tileIdx] = pSpec_Tile;
    }
  }

  /* if (noise_level!=0) */
  {
    for (g = 0, win = 0; g < GetWindowGroups(&pAacDecoderChannelInfo->icsInfo); g++) {
      int windowGroupLength = GetWindowGroupLength(&pAacDecoderChannelInfo->icsInfo, g);
      for (sfb = nfStartOffset_sfb; sfb < max_noise_sfb; sfb++) {
        int bin_start = swb_offset[sfb];
        int bin_stop = swb_offset[sfb + 1];

        int flagN = band_is_noise[g * 16 + sfb];

        /* if all bins of one sfb in one window group are zero modify the scale factor by
         * noise_offset */
        if (flagN) {
          /* Change scaling factors for empty signal bands */
          pAacDecoderChannelInfo->pDynData->aScaleFactor[g * 16 + sfb] += noise_offset;
          /* scale factor "sf" implied gain "g" is g = 2^(sf/4) */
          for (gwin = 0; gwin < windowGroupLength; gwin++) {
            pAacDecoderChannelInfo->pDynData->aSfbScale[(win + gwin) * 16 + sfb] +=
                (noise_offset >> 2);
          }
        }

        ULONG seed = *nfRandomSeed;
        /* + 1 because exponent of MantissaTable[lsb][0] is always 1. */
        int scale = (pAacDecoderChannelInfo->pDynData->aScaleFactor[g * 16 + sfb] >> 2) + 1;
        int lsb = pAacDecoderChannelInfo->pDynData->aScaleFactor[g * 16 + sfb] & 3;
        FIXP_DBL mantissa = MantissaTable[lsb][0];

        for (gwin = 0; gwin < windowGroupLength; gwin++) {
          FIXP_DBL* pSpec = SPEC(pAacDecoderChannelInfo->pSpectralCoefficient, win + gwin,
                                 pAacDecoderChannelInfo->granuleLength);

          int scale1 = scale - pAacDecoderChannelInfo->pDynData->aSfbScale[(win + gwin) * 16 + sfb];
          FIXP_DBL scaled_noiseVal_pos = scaleValue(fMultDiv2(noiseVal_pos, mantissa), scale1);
          FIXP_DBL scaled_noiseVal_neg = -scaled_noiseVal_pos;

          /* If the whole band is zero, just fill without checking */
          if (flagN) {
            if (igf_StereoFilling) {
              /* If Stereo Filling is active and this band has been zeroed-out, then do not scale
               * noise data */
              scaled_noiseVal_pos = FX_SGL2FX_DBL(noiseVal_pos);
              scaled_noiseVal_neg = -scaled_noiseVal_pos;
            }
            for (int bin = bin_start; bin < bin_stop; bin++) {
              seed = (ULONG)((UINT64)seed * 69069 +
                             5); /* Inlined: UsacRandomSign - origin in usacdec_lpd.h */
              pSpec[bin] = (seed & 0x10000) ? scaled_noiseVal_neg : scaled_noiseVal_pos;
            } /* for (bin...) */
          }
          /*If band is sparsely filled, check for 0 and fill */
          else {
            for (int bin = bin_start; bin < bin_stop; bin++) {
              if (pSpec[bin] == (FIXP_DBL)0) {
                seed = (ULONG)((UINT64)seed * 69069 +
                               5); /* Inlined: UsacRandomSign - origin in usacdec_lpd.h */
                pSpec[bin] = (seed & 0x10000) ? scaled_noiseVal_neg : scaled_noiseVal_pos;
              }
            } /* for (bin...) */
          }

          /* Apply Independent noise filling of tiles  or just copy them if INF is not active */

          if ((flag_INFactive) && (!pAacDecoderChannelInfo->IGFdata.bitstreamData[0].igf_allZero) &&
              (IGFNumTile > 0)) {
            /* Initialization to prevent warnings */
            FIXP_DBL* Win_Tile_pointer_Array[4] = {Tile_pointer_Array[0], Tile_pointer_Array[1],
                                                   Tile_pointer_Array[2], Tile_pointer_Array[3]};

            for (int tileIdx = 0; tileIdx < IGFNumTile; tileIdx++) {
              Win_Tile_pointer_Array[tileIdx] = SPEC(Tile_pointer_Array[tileIdx], win + gwin,
                                                     pAacDecoderChannelInfo->granuleLength) +
                                                bin_start;
            }

            /* If the whole band is zero, just fill without checking */
            if (flagN) {
              if (igf_StereoFilling) {
                /* If Stereo Filling is active and this band has been zeroed-out, then do not scale
                 * noise data */
                scaled_noiseVal_pos = FX_SGL2FX_DBL(noiseVal_pos);
                scaled_noiseVal_neg = -scaled_noiseVal_pos;
              }

              for (int bin = bin_start; bin < bin_stop; bin++) {
                for (int tileIdx = 0; tileIdx < IGFNumTile; tileIdx++) {
                  seed = (ULONG)((UINT64)seed * 69069 +
                                 5); /* Inlined: UsacRandomSign - origin in usacdec_lpd.h */
                  *(Win_Tile_pointer_Array[tileIdx])++ =
                      (seed & 0x10000) ? scaled_noiseVal_neg : scaled_noiseVal_pos;
                }
              } /* for (bin...) */
            }
            /*If band is sparsely filled, check for 0 and fill */
            else {
              for (int bin = bin_start; bin < bin_stop; bin++) {
                if (*(Win_Tile_pointer_Array[0]) == (FIXP_DBL)0) {
                  for (int tileIdx = 0; tileIdx < IGFNumTile; tileIdx++) {
                    seed = (ULONG)((UINT64)seed * 69069 + 5);
                    *(Win_Tile_pointer_Array[tileIdx])++ =
                        (seed & 0x10000) ? scaled_noiseVal_neg : scaled_noiseVal_pos;
                  } /*for( int tileIdx = 0; */

                } else {
                  for (int tileIdx = 0; tileIdx < IGFNumTile; tileIdx++) {
                    Win_Tile_pointer_Array[tileIdx]++;
                  } /*for( int tileIdx = 0; */
                }

              } /* for (bin...) */
            }

          } /* if(flag_INFactive) */

        } /* for (gwin...) */
        *nfRandomSeed = seed;
      } /* for (sfb...) */
      win += windowGroupLength;
    } /* for (g...) */

    if (flag_INFactive) {
      for (int tileIdx = 0; tileIdx < IGFNumTile; tileIdx++) {
        /* If the scale factors have changed due to empty signal bands, reflect these changes in
         * each tile  */
        IGF_GRID_INFO_HANDLE hGrid =
            &pAacDecoderStaticChannelInfo->IGF_StaticData
                 .sGridInfoTab[GetWindowSequence(&pAacDecoderChannelInfo->icsInfo) == BLOCK_SHORT];
        IGF_MAP_INFO_HANDLE hMap = &hGrid->sIGFMapInfoTab[tileIdx];
        FDKmemcpy(pAacDecoderStaticChannelInfo->IGF_StaticData.fSpectrumTab_sfb_exp[tileIdx],
                  pAacDecoderChannelInfo->pDynData->aSfbScale,
                  sizeof(SHORT) * IGF_MAX_WIN * IGF_MAX_SFB_SHORT);

        if (pAacDecoderChannelInfo->IGFdata.bitstreamData[0].igf_allZero) {
          /* copy noise filled spectrum to the internal representation: */
          FDKmemcpy(hMap->fSpectrumTab[0], pAacDecoderChannelInfo->pSpectralCoefficient,
                    sizeof(FIXP_DBL) * 1024);
        }

      } /* for (tileIdx) */
    }

  } /* ... */
}

void IGF_StereoFillingPrepare(CAacDecoderChannelInfo* pAacDecoderChannelInfo,
                              SamplingRateInfo* pSamplingRateInfo, const INT max_noise_sfb,
                              FIXP_DBL* dmx_prev_modified, SHORT* dmx_prev_modified_exp,
                              const UCHAR* band_is_noise) {
  const SHORT* swb_offset =
      GetScaleFactorBandOffsets(&pAacDecoderChannelInfo->icsInfo, pSamplingRateInfo);
  int sfb, nfStartOffset_sfb;

  /* Fixed number for long windows */
  const int noiseFillingStartOffset = 160;

  /* determine sfb from where on noise filling is applied */
  for (sfb = 0; swb_offset[sfb] < noiseFillingStartOffset; sfb++)
    ;
  nfStartOffset_sfb = sfb;

  for (sfb = nfStartOffset_sfb; sfb < max_noise_sfb; sfb++) {
    /* Run algorithm only if the particular band has been fully noise filled, i.e. that it has been
     empty before the noise-filling process */
    if (!band_is_noise[sfb]) {
      continue;
    }

    short bin_start = swb_offset[sfb];
    short bin_stop = swb_offset[sfb + 1];

    /* Calculate the widht of the particular sfb */
    short sfb_width = bin_stop - bin_start;

    /* Left-allign "sfb_width", so that it is suitable for multiplication calculations */
    int bits = fNormz((FIXP_SGL)sfb_width) - 1;
    FIXP_SGL sfb_width_modified = (FIXP_SGL)sfb_width << bits;

    /* Find a suitable shift for dmx addition */
    INT shift = 0;
    while (1) {
      INT temp = (INT)1 << shift;
      if (temp >= sfb_width)
        break;
      else {
        shift++;
      }
    }

    /* Get the spectrum of the particular window */
    FIXP_DBL* pSpec = pAacDecoderChannelInfo->pSpectralCoefficient;

    /* Get the previous downmix of the particular window */
    FIXP_DBL* dmx_prev = dmx_prev_modified;

    /* Find the available headroom of the downmix signal and a suitable shift value*/
    INT head_shift = getScalefactor(&dmx_prev[bin_start], sfb_width);

    /* Avoiding noise boosting. Cleaning noise*/
    if (head_shift > 25) {
      dmx_prev_modified_exp[sfb] = -31;
      for (int bin = bin_start; bin < bin_stop; bin++) {
        dmx_prev[bin] = (FIXP_DBL)0;
      }
      continue;
    }

    /* Calculate the energy of the previous downmix */
    /* Make sure energy is never zero, so that the division later is possible */
    FIXP_DBL energy_dmx = 1;
    for (int bin = bin_start; bin < bin_stop; bin++) {
      energy_dmx += (fPow2Div2(dmx_prev[bin] << head_shift) >> shift);
    } /* for (bin...) */

    /* Calculate "energy_dmx" exponent of the particular window */
    INT energy_dmx_e = (((INT)dmx_prev_modified_exp[sfb] - head_shift) << 1) + 1 + shift;

    /* We need to calculate the energy per line (constant) of the scaled, noise-filled spectrum.
    Then energy in the whole band is obtained by multiplying the energy per line by the widht of
    the band */

    /* When we Pow, we get Q4.27->noise_energy_per_line */
    FIXP_DBL noise_energy_per_line_sfb = fPow2(pSpec[bin_start]);

    /* pSpec is in Q-1.32, but since we multiply it by 4 (2 bits;see algo) it becomes Q1.30.
    The power is in Q2.29. In short: exp= ((-1 + 2) << 1); */
    const INT noise_energy_per_line_sfb_exp = 2;

    /* We need to compute "sfbWidth - factor", which is equal to "sfbWidth*(1-factor)" */

    /* Get the value of one with the same exponent. It is:" 1<< (31 -
     * noise_energy_per_line_sfb_exp)" */
    const FIXP_DBL Value_of_One = 0x20000000;

    /* Calculate "(1-factor)" */
    FIXP_DBL Intermediate_Result = Value_of_One - noise_energy_per_line_sfb;

    /* We multiply by sfbWidth, i.e. calculate "sfbWidth*(1-factor)" */
    FIXP_DBL factor = fMult(Intermediate_Result, sfb_width_modified);
    int factor_exp = noise_energy_per_line_sfb_exp + (15 - bits);

    /* Divide by energy_sfb. Calculate (sfb_width * (1 - factor))/energy_dmx[sfb] */
    int temp_exp;
    FIXP_DBL temp_FIXP_DBL = fDivNorm(factor, energy_dmx, &temp_exp);

    /* Correct the output exponent */
    temp_exp += factor_exp - energy_dmx_e;

    /* Calculate sqrt((sfb_width * (1 - factor))/energy_dmx[sfb]) */
    temp_FIXP_DBL = sqrtFixp_lookup(temp_FIXP_DBL, &temp_exp);

    /* Allign max left */
    INT tmp_scale = fNormz(temp_FIXP_DBL) - 1;
    FIXP_DBL tmp = temp_FIXP_DBL << tmp_scale;
    temp_exp -= tmp_scale;
    /* Limit the output value to 10 */
    if (temp_exp > 3) {
      /* limit factor to 10.0f */
      tmp = (temp_exp > 4) ? FL2FXCONST_DBL(0.625f) : fMin(tmp, FL2FXCONST_DBL(0.625f));
      temp_exp = 4;
    }

    /* Modify the downmix so that it can be added to the spectrum later: "dmx_prev*tmp" */
    for (int bin = bin_start; bin < bin_stop; bin++) {
      dmx_prev[bin] = fMult(dmx_prev[bin] << head_shift, tmp);
    } /* for (bin...) */

    /* "dmx_prev*tmp" exponent for every sfb and window */
    dmx_prev_modified_exp[sfb] = dmx_prev_modified_exp[sfb] - head_shift + temp_exp;

  } /* for (sfb...) */
}

void IGF_StereoFillingApply(CAacDecoderChannelInfo* pAacDecoderChannelInfo,
                            SPECTRAL_PTR pSpectralCoefficient, SHORT* p2_tile_spectrum_exp,
                            SamplingRateInfo* pSamplingRateInfo, const INT max_noise_sfb,
                            FIXP_DBL* dmx_prev_modified, const SHORT* dmx_prev_modified_e,
                            const UCHAR* band_is_noise) {
  const SHORT* swb_offset =
      GetScaleFactorBandOffsets(&pAacDecoderChannelInfo->icsInfo, pSamplingRateInfo);
  int sfb, nfStartOffset_sfb, bits;

  /*Constant for long blocks*/
  const int noiseFillingStartOffset = 160;

  /* determine sfb from where on noise filling is applied */
  for (sfb = 0; swb_offset[sfb] < noiseFillingStartOffset; sfb++)
    ;
  nfStartOffset_sfb = sfb;

  for (sfb = nfStartOffset_sfb; sfb < max_noise_sfb; sfb++) {
    /* if all bins of one sfb in one window group are zero modify the scale factor by noise_offset
     */
    if (!band_is_noise[sfb]) {
      continue;
    }

    int bin_start = swb_offset[sfb];
    int bin_stop = swb_offset[sfb + 1];

    /* Calculate the width of the particular sfb */
    short sfb_width = bin_stop - bin_start;

    /*Find suitable headroom for subsequent additions */
    int shift = 0;
    while (1) {
      INT temp = (INT)1 << shift;
      if (temp >= sfb_width)
        break;
      else {
        shift++;
      }
    }

    /* Get the spectrum of the particular window */
    FIXP_DBL* pSpec = pSpectralCoefficient;

    /* Get the previous downmix of the particular window */
    FIXP_DBL* dmx_tmp_pointer = dmx_prev_modified;

    /* Get the scale value of dmx */
    int scaleConst = dmx_prev_modified_e[sfb];

    /* Add the scaled and noise filled values to the scaled downmix from the previous frame.
    For that purpose we need to adjust the binary point position */

    /* pSpec is in Q-1.32, but since we multiply it by 4 (2 bits;see algo) it becomes Q1.30.*/
    const int scale_pSpec = 1;

    /* Resulting exponent */
    int pSpec_exp;

    /* Set to some minimum value to prevent divison by 0 */
    FIXP_DBL factor = 1;
    int diff_1 = scaleConst - scale_pSpec;
    /* Adjust binary point and add */
    if (diff_1 >= 0) {
      diff_1 = fMin(diff_1 + 1, 31);
      for (int bin = bin_start; bin < bin_stop; bin++) {
        FIXP_DBL temp = (dmx_tmp_pointer[bin] >> 1) + (pSpec[bin] >> diff_1);
        pSpec[bin] = temp;
        factor += fPow2(temp) >> shift;
      }
      pSpec_exp = scaleConst + 1;
    } else {
      diff_1 = fMin(-diff_1 + 1, 31);
      for (int bin = bin_start; bin < bin_stop; bin++) {
        FIXP_DBL temp = (dmx_tmp_pointer[bin] >> diff_1) + (pSpec[bin] >> 1);
        pSpec[bin] = temp;
        factor += fPow2(temp) >> shift;
      }
      pSpec_exp = scale_pSpec + 1;
    }

    /* Stereo filling changes the dynamic range of the signal,despite keeping the energy of
    the signal constant. */
    INT head_shift = getScalefactor(&pSpec[bin_start], sfb_width);

    /* Do not calculate zeros or -1s */
    if (head_shift == 31) {
      p2_tile_spectrum_exp[sfb] = -31;
      continue;
    }

    /* Exponent of the resulting power value */
    int factor_exp;
    factor_exp = (pSpec_exp << 1) + shift;

    /* Left-allign "sfb_width", so that it is suitable for multiplication calculations */
    bits = fNormz((FIXP_DBL)sfb_width) - 1;
    FIXP_DBL sfb_width_modified = (FIXP_DBL)sfb_width << bits;
    INT sfb_width_modified_exp = (31 - bits);

    /*Calculate Scaled_Width/factor */
    INT temp_int;
    FIXP_DBL temp_FIXP_DBL = fDivNorm(sfb_width_modified, factor, &temp_int);
    temp_int += (sfb_width_modified_exp - factor_exp);

    /*Calculate sqrt(Scaled_Width/factor) */
    factor = sqrtFixp_lookup(temp_FIXP_DBL, &temp_int);

    INT tmp_scale = fNormz(factor) - 1;
    factor <<= tmp_scale;
    temp_int -= tmp_scale;
    if (temp_int > 3) {
      /* limit factor to 10.0f */
      factor = (temp_int > 4) ? FL2FXCONST_DBL(0.625f) : fMin(factor, FL2FXCONST_DBL(0.625f));
      temp_int = 4;
    }

    /* Generate scaling factor */
    int lsb = pAacDecoderChannelInfo->pDynData->aScaleFactor[sfb] & 3;
    int msb = pAacDecoderChannelInfo->pDynData->aScaleFactor[sfb] >> 2;
    FIXP_DBL Scaled_One = MantissaTable[lsb][0];
    SHORT Scaled_One_exp = msb + 1;

    /* Modify spectrum and scale it with the orginal scaling factor*/
    for (int bin = bin_start; bin < bin_stop; bin++) {
      FIXP_DBL temp = fMult(pSpec[bin] << head_shift, factor);
      pSpec[bin] = fMult(temp, Scaled_One);
    } /* for (bin...) */

    /* Calculate output exponent including the output scaling which was avoided in the noise filling
     * stage */
    p2_tile_spectrum_exp[sfb] = (SHORT)(pSpec_exp - head_shift + temp_int) + Scaled_One_exp;

  } /* for (sfb...) */
}

AAC_DECODER_ERROR CBlock_ReadAcSpectralData(
    HANDLE_FDK_BITSTREAM hBs, CAacDecoderChannelInfo* pAacDecoderChannelInfo,
    CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo,
    const SamplingRateInfo* pSamplingRateInfo, const UINT frame_length, const UINT flags) {
  AAC_DECODER_ERROR errorAAC = AAC_DEC_OK;
  ARITH_CODING_ERROR error = ARITH_CODER_OK;
  int arith_reset_flag, lg, numWin, win, winLen;
  const SHORT* RESTRICT BandOffsets;

  /* number of transmitted spectral coefficients */
  BandOffsets = GetScaleFactorBandOffsets(&pAacDecoderChannelInfo->icsInfo, pSamplingRateInfo);
  lg = BandOffsets[GetScaleFactorBandsTransmitted(&pAacDecoderChannelInfo->icsInfo)];

  numWin = GetWindowsPerFrame(&pAacDecoderChannelInfo->icsInfo);
  winLen = (IsLongBlock(&pAacDecoderChannelInfo->icsInfo)) ? (int)frame_length
                                                           : (int)frame_length / numWin;

  if (flags & AC_INDEP) {
    arith_reset_flag = 1;
  } else {
    arith_reset_flag = (USHORT)FDKreadBit(hBs);
  }

  for (win = 0; win < numWin; win++) {
    error = CArco_DecodeArithData(pAacDecoderStaticChannelInfo->hArCo, hBs,
                                  SPEC(pAacDecoderChannelInfo->pSpectralCoefficient, win,
                                       pAacDecoderChannelInfo->granuleLength),
                                  lg, winLen, arith_reset_flag && (win == 0));
    if (error != ARITH_CODER_OK) {
      goto bail;
    }
  }

bail:
  if (error == ARITH_CODER_ERROR) {
    errorAAC = AAC_DEC_PARSE_ERROR;
  }

  return errorAAC;
}

static int getLowPassLine(CAacDecoderChannelInfo* pAacDecoderChannelInfo,
                          CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo,
                          const SamplingRateInfo* pSamplingRateInfo, const UINT elFlags) {
  int lowpass_sfb = GetScaleFactorBandsTransmitted(&pAacDecoderChannelInfo->icsInfo);

  if (pAacDecoderChannelInfo->pDynData->RawDataInfo.CommonWindow) {
    lowpass_sfb = pAacDecoderChannelInfo->icsInfo.max_sfb_ste;
  }
  if (elFlags & AC_EL_ENHANCED_NOISE) {
    lowpass_sfb = fMax(lowpass_sfb, (int)pAacDecoderStaticChannelInfo->IGF_StaticData.igfStopSfbLB);
  }
  return GetScaleFactorBandOffsets(&pAacDecoderChannelInfo->icsInfo,
                                   pSamplingRateInfo)[lowpass_sfb] /
         2;
}

void ApplyTools(CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo[],
                CAacDecoderChannelInfo* pAacDecoderChannelInfo[],
                const SamplingRateInfo* pSamplingRateInfo, const UINT flags, const UINT elFlags,
                const int channel, const int common_window) {
  if (flags & (AC_MPEGH3DA)) {
    /* IGF processing before TNS */

    if ((elFlags & AC_EL_ENHANCED_NOISE) && (!(elFlags & AC_EL_IGF_AFTER_TNS)) &&
        ((elFlags & AC_EL_IGF_INDEP_TILING) || !common_window)) {
      UCHAR TNF_mask[1024];
      FDKmemclear(TNF_mask, 1024);

      /* Run main IGF routine */
      CIgf_apply(&pAacDecoderStaticChannelInfo[channel]->IGF_StaticData,
                 &pAacDecoderChannelInfo[channel]->IGFdata,
                 pAacDecoderChannelInfo[channel]->pSpectralCoefficient,
                 pAacDecoderChannelInfo[channel]->specScale,
                 (IsLongBlock(&pAacDecoderChannelInfo[channel]->icsInfo) == 1)
                     ? IGF_GRID_LONG_WINDOW
                     : IGF_GRID_SHORT_WINDOW,
                 (INT)GetWindowGroups(&pAacDecoderChannelInfo[channel]->icsInfo),
                 GetWindowsPerFrame(&pAacDecoderChannelInfo[channel]->icsInfo),
                 GetWindowGroupLengthTable(&pAacDecoderChannelInfo[channel]->icsInfo),
                 &(pAacDecoderStaticChannelInfo[channel]->nfRandomSeed), TNF_mask,
                 ((elFlags & AC_EL_IGF_USE_ENF) ? 1 : 0), IGF_FRAME_DIVISION_AAC_OR_TCX_LONG);

      if (IsLongBlock(&pAacDecoderChannelInfo[channel]->icsInfo)) {
        /* Apply IGF - TNF */
        CIgf_TNF_apply(&pAacDecoderStaticChannelInfo[channel]->IGF_StaticData,
                       &pAacDecoderChannelInfo[channel]->IGFdata,
                       pAacDecoderChannelInfo[channel]->pSpectralCoefficient,
                       pAacDecoderChannelInfo[channel]->specScale, TNF_mask, 0, 0);
      }
    }
  }

  UCHAR nbands = GetScaleFactorBandsTransmitted(&pAacDecoderChannelInfo[channel]->icsInfo);

  if (elFlags & AC_EL_ENHANCED_NOISE) {
    UCHAR islong = IsLongBlock(&pAacDecoderChannelInfo[channel]->icsInfo);
    IGF_PRIVATE_STATIC_DATA* IGFdata = &pAacDecoderStaticChannelInfo[channel]->IGF_StaticData;

    if (!(elFlags & AC_EL_IGF_AFTER_TNS)) {
      if (islong) {
        if (IGFdata->igfStopSfbLB > nbands) nbands = IGFdata->igfStopSfbLB;
      } else {
        if (IGFdata->igfStopSfbSB > nbands) nbands = IGFdata->igfStopSfbSB;
      }
    } else {
      if (islong) {
        if (IGFdata->igfStartSfbLB < nbands) nbands = IGFdata->igfStartSfbLB;
      } else {
        if (IGFdata->igfStartSfbSB < nbands) nbands = IGFdata->igfStartSfbSB;
      }
    }
  }

  /* Deinterleave */
  if (pAacDecoderChannelInfo[channel]->transform_splitting_active) {
    int low_pass_line =
        getLowPassLine(pAacDecoderChannelInfo[channel], pAacDecoderStaticChannelInfo[channel],
                       pSamplingRateInfo, elFlags);
    FIXP_DBL tmp[512];

    FDK_ASSERT(low_pass_line <= 512);
    for (int i = 0; i < low_pass_line; i++) {
      tmp[i] = pAacDecoderChannelInfo[channel]->pSpectralCoefficient[i * 2 + 1];
    }
    for (int i = 0; i < low_pass_line; i++) {
      pAacDecoderChannelInfo[channel]->pSpectralCoefficient[i] =
          pAacDecoderChannelInfo[channel]->pSpectralCoefficient[i * 2];
    }
    for (int i = 0; i < low_pass_line; i++) {
      pAacDecoderChannelInfo[channel]->pSpectralCoefficient[i + low_pass_line] = tmp[i];
    }
  }

  CTns_Apply(&pAacDecoderChannelInfo[channel]->pDynData->TnsData,
             &pAacDecoderChannelInfo[channel]->icsInfo,
             pAacDecoderChannelInfo[channel]->pSpectralCoefficient, pSamplingRateInfo,
             pAacDecoderChannelInfo[channel]->granuleLength, nbands,
             (elFlags & AC_EL_ENHANCED_NOISE) ? 1 : 0, flags);

  /* Interleave again */
  if (pAacDecoderChannelInfo[channel]->transform_splitting_active) {
    int low_pass_line =
        getLowPassLine(pAacDecoderChannelInfo[channel], pAacDecoderStaticChannelInfo[channel],
                       pSamplingRateInfo, elFlags);
    FIXP_DBL tmp[512];

    FDK_ASSERT(low_pass_line <= 512);
    for (int i = 0; i < low_pass_line; i++) {
      tmp[i] = pAacDecoderChannelInfo[channel]->pSpectralCoefficient[i + low_pass_line];
    }
    for (int i = low_pass_line - 1; i >= 0; i--) {
      pAacDecoderChannelInfo[channel]->pSpectralCoefficient[i * 2] =
          pAacDecoderChannelInfo[channel]->pSpectralCoefficient[i];
    }
    for (int i = 0; i < low_pass_line; i++) {
      pAacDecoderChannelInfo[channel]->pSpectralCoefficient[i * 2 + 1] = tmp[i];
    }
    for (int i = (low_pass_line << 1); i < 1024; i++) {
      pAacDecoderChannelInfo[channel]->pSpectralCoefficient[i] = (FIXP_DBL)0;
    }
  }

  if ((elFlags & AC_EL_ENHANCED_NOISE) && (elFlags & AC_EL_IGF_AFTER_TNS) &&
      (elFlags & AC_EL_IGF_USE_ENF)) {
    INT NumberOfTiles = iisIGFDecLibGetNumberOfTiles(
        &pAacDecoderStaticChannelInfo[channel]->IGF_StaticData,
        GetWindowSequence(&pAacDecoderChannelInfo[channel]->icsInfo) == BLOCK_SHORT);

    /* Apply TNS over tiles */
    for (int tileIdx = 0; tileIdx < NumberOfTiles; tileIdx++) {
      /* Get tile spectrum */
      FIXP_DBL* tile_spectrum = iisIGFDecLibAccessSourceSpectrum(
          &pAacDecoderStaticChannelInfo[channel]->IGF_StaticData,
          &pAacDecoderChannelInfo[channel]->IGFdata, tileIdx,
          GetWindowSequence(&pAacDecoderChannelInfo[channel]->icsInfo) == BLOCK_SHORT, 0);

      /* Apply TNS over the tiles */
      CTns_Apply(&pAacDecoderChannelInfo[channel]->pDynData->TnsData,
                 &pAacDecoderChannelInfo[channel]->icsInfo, tile_spectrum, pSamplingRateInfo,
                 pAacDecoderChannelInfo[channel]->granuleLength, nbands,
                 (elFlags & AC_EL_ENHANCED_NOISE) ? 1 : 0, flags);

    } /* for(int tileIdx=0;tileIdx<NumberOfTiles;tileIdx++) */
  }

  if (flags & (AC_MPEGH3DA)) {
    /* When IGF is run after TNS */

    if ((elFlags & AC_EL_ENHANCED_NOISE) && (elFlags & AC_EL_IGF_AFTER_TNS) &&
        ((elFlags & AC_EL_IGF_INDEP_TILING) || !common_window)) {
      UCHAR TNF_mask[1024];
      FDKmemclear(TNF_mask, 1024);

      /* Run main IGF routine */
      CIgf_apply(&pAacDecoderStaticChannelInfo[channel]->IGF_StaticData,
                 &pAacDecoderChannelInfo[channel]->IGFdata,
                 pAacDecoderChannelInfo[channel]->pSpectralCoefficient,
                 pAacDecoderChannelInfo[channel]->specScale,
                 (IsLongBlock(&pAacDecoderChannelInfo[channel]->icsInfo) == 1)
                     ? IGF_GRID_LONG_WINDOW
                     : IGF_GRID_SHORT_WINDOW,
                 (INT)GetWindowGroups(&pAacDecoderChannelInfo[channel]->icsInfo),
                 GetWindowsPerFrame(&pAacDecoderChannelInfo[channel]->icsInfo),
                 GetWindowGroupLengthTable(&pAacDecoderChannelInfo[channel]->icsInfo),
                 &(pAacDecoderStaticChannelInfo[channel]->nfRandomSeed), TNF_mask,
                 ((elFlags & AC_EL_IGF_USE_ENF) ? 1 : 0), IGF_FRAME_DIVISION_AAC_OR_TCX_LONG);

      if (IsLongBlock(&pAacDecoderChannelInfo[channel]->icsInfo)) {
        /* Apply IGF - TNF */
        CIgf_TNF_apply(&pAacDecoderStaticChannelInfo[channel]->IGF_StaticData,
                       &pAacDecoderChannelInfo[channel]->IGFdata,
                       pAacDecoderChannelInfo[channel]->pSpectralCoefficient,
                       pAacDecoderChannelInfo[channel]->specScale, TNF_mask, 0, 0);
      }
    }
  }
}

static int getWindow2Nr(int length, int shape) {
  int nr = 0;

  if (shape == 2) {
    /* Low Overlap, 3/4 zeroed */
    nr = (length * 3) >> 2;
  }

  return nr;
}

void CBlock_FrequencyToTime(CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo,
                            CAacDecoderChannelInfo* pAacDecoderChannelInfo, PCM_DEC outSamples[],
                            const SHORT frameLen, const int frameOk, FIXP_DBL* pWorkBuffer1,
                            const INT aacOutDataHeadroom, UINT elFlags, INT elCh) {
  int fr, fl, tl, nSpec;

#if defined(FDK_ASSERT_ENABLE)
  LONG nSamples;
#endif

  /* Determine left slope length (fl), right slope length (fr) and transform length (tl).
     USAC: The slope length may mismatch with the previous frame in case of LPD / FD
           transitions. The adjustment is handled by the imdct implementation.
  */
  tl = frameLen;
  nSpec = 1;

  switch (pAacDecoderChannelInfo->icsInfo.WindowSequence) {
    default:
    case BLOCK_LONG:
      fl = frameLen;
      fr = frameLen - getWindow2Nr(frameLen, GetWindowShape(&pAacDecoderChannelInfo->icsInfo));
      /* New startup needs differentiation between sine shape and low overlap shape.
         This is a special case for the LD-AAC transformation windows, because
         the slope length can be different while using the same window sequence. */
      if (pAacDecoderStaticChannelInfo->IMdct.prev_tl == 0) {
        fl = fr;
      }
      break;
    case BLOCK_STOP:
      fl = frameLen >> 3;
      fr = frameLen;
      break;
    case BLOCK_START: /* or StopStartSequence */
      fl = frameLen;
      fr = frameLen >> 3;
      if (pAacDecoderChannelInfo->transform_splitting_active) {
        tl >>= 1;
        nSpec = 2;
        fl = fr;
        pAacDecoderChannelInfo->specScale[0]--;
        pAacDecoderChannelInfo->specScale[1] = pAacDecoderChannelInfo->specScale[0];

        /* Deinterleave */
        FIXP_DBL tmp[512];
        for (int i = 0; i < 512; i++) {
          tmp[i] = pAacDecoderChannelInfo->pSpectralCoefficient[i * 2 + 1];
        }
        for (int i = 0; i < 512; i++) {
          pAacDecoderChannelInfo->pSpectralCoefficient[i] =
              pAacDecoderChannelInfo->pSpectralCoefficient[i * 2];
        }
        for (int i = 0; i < 512; i++) {
          pAacDecoderChannelInfo->pSpectralCoefficient[i + 512] = tmp[i];
        }
      }
      break;
    case BLOCK_SHORT:
      fl = fr = frameLen >> 3;
      tl >>= 3;
      nSpec = 8;
      break;
  }

  {
    {
      FIXP_DBL* tmp = pAacDecoderChannelInfo->pComStaticData->pWorkBufferCore1->mdctOutTemp;
#if defined(FDK_ASSERT_ENABLE)
      nSamples =
#endif
          imlt_block(
              &pAacDecoderStaticChannelInfo->IMdct, tmp,
              SPEC_LONG(pAacDecoderChannelInfo->pSpectralCoefficient),
              pAacDecoderChannelInfo->specScale, nSpec, frameLen, tl,
              FDKgetWindowSlope(fl, GetWindowShape(&pAacDecoderChannelInfo->icsInfo)), fl,
              FDKgetWindowSlope(fr, GetWindowShape(&pAacDecoderChannelInfo->icsInfo)), fr,
              (FIXP_DBL)0,
              pAacDecoderChannelInfo->currAliasingSymmetry ? MLT_FLAG_CURR_ALIAS_SYMMETRY : 0);

#if defined(FUNCTION_scale_imdct_samples)
      scale_imdct_samples(tmp, outSamples, frameLen, 1);
#else
      scaleValuesSaturate(outSamples, tmp, frameLen, MDCT_OUT_HEADROOM - aacOutDataHeadroom);
#endif /* defined(FUNCTION_scale_imdct_samples) */
    }
  }

  FDK_ASSERT(nSamples == frameLen);

  pAacDecoderStaticChannelInfo->prevWindowShape = pAacDecoderChannelInfo->icsInfo.WindowShape;
}
