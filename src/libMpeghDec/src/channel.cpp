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

   Description:

*******************************************************************************/

#include "channel.h"
#include "aacdecoder.h"
#include "block.h"
#include "aacdec_tns.h"
#include "FDK_bitstream.h"

#include "conceal.h"

#include "ltp_post.h"

#include "fdp.h"

static void Clean_Complex_Prediction_coefficients(
    CJointStereoPersistentData* pJointStereoPersistentData, int windowGroups, const int low_limit,
    const int high_limit) {
  for (int group = 0; group < windowGroups; group++) {
    for (int sfb = low_limit; sfb < high_limit; sfb++) {
      pJointStereoPersistentData->alpha_q_re_prev[group][sfb] = 0;
      pJointStereoPersistentData->alpha_q_im_prev[group][sfb] = 0;
    }
  }
}

/*!
  \brief Decode channel pair element

  The function decodes a channel pair element.

  \return  none
*/
void CChannelElement_Decode(
    CAacDecoderChannelInfo* pAacDecoderChannelInfo[2], /*!< pointer to aac decoder channel info */
    CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo[2],
    SamplingRateInfo* pSamplingRateInfo, UINT flags, UINT elFlags, int el_channels) {
  int ch = 0;

  int maxSfBandsL = 0, maxSfBandsR = 0;
  int maybe_jstereo = (el_channels > 1);

  if (maybe_jstereo) {
    maxSfBandsL = GetScaleFactorBandsTransmitted(&pAacDecoderChannelInfo[L]->icsInfo);
    maxSfBandsR = GetScaleFactorBandsTransmitted(&pAacDecoderChannelInfo[R]->icsInfo);

    /* apply ms */
    if (pAacDecoderChannelInfo[L]->pDynData->RawDataInfo.CommonWindow) {
      {
        int max_sfb_ste = (INT)(pAacDecoderChannelInfo[L]->icsInfo.max_sfb_ste);

        CJointStereo_ApplyMS(
            pAacDecoderChannelInfo, pAacDecoderStaticChannelInfo,
            pAacDecoderChannelInfo[L]->pSpectralCoefficient,
            pAacDecoderChannelInfo[R]->pSpectralCoefficient,
            pAacDecoderChannelInfo[L]->pDynData->aSfbScale,
            pAacDecoderChannelInfo[R]->pDynData->aSfbScale, pAacDecoderChannelInfo[L]->specScale,
            pAacDecoderChannelInfo[R]->specScale,
            GetScaleFactorBandOffsets(&pAacDecoderChannelInfo[L]->icsInfo, pSamplingRateInfo),
            GetWindowGroupLengthTable(&pAacDecoderChannelInfo[L]->icsInfo),
            GetWindowGroups(&pAacDecoderChannelInfo[L]->icsInfo), max_sfb_ste, maxSfBandsL,
            maxSfBandsR, pAacDecoderChannelInfo[L]->pComData->jointStereoData.store_dmx_re_prev,
            &(pAacDecoderChannelInfo[L]->pComData->jointStereoData.store_dmx_re_prev_e), 1);

        if ((elFlags & AC_EL_ENHANCED_NOISE) && (elFlags & AC_EL_IGF_USE_ENF)) {
          /* Performs joint-stereo/MS for the IGF tiles (up to 4) */
          CJointStereo_ApplyMSIGFcore(
              pAacDecoderChannelInfo, pAacDecoderStaticChannelInfo,
              GetScaleFactorBandOffsets(&pAacDecoderChannelInfo[L]->icsInfo, pSamplingRateInfo),
              GetWindowGroupLengthTable(&pAacDecoderChannelInfo[L]->icsInfo),
              GetWindowGroups(&pAacDecoderChannelInfo[L]->icsInfo), max_sfb_ste, maxSfBandsL,
              maxSfBandsR);
        }

      } /* if ( ((elFlags & AC_EL_USAC_CP_POSSIBLE).... */
    }   /* if (pAacDecoderChannelInfo[L]->pDynData->RawDataInfo.CommonWindow)*/
  }     /* maybe_stereo */

  for (ch = 0; ch < el_channels; ch++) {
    {
      UCHAR noSfbs = GetScaleFactorBandsTransmitted(&pAacDecoderChannelInfo[ch]->icsInfo);
      /* For USAC common window: max_sfb of both channels may differ (common_max_sfb == 0). */
      if ((maybe_jstereo == 1) &&
          (pAacDecoderChannelInfo[L]->pDynData->RawDataInfo.CommonWindow == 1)) {
        noSfbs = fMax(maxSfBandsL, maxSfBandsR);
      }
      int CP_active = 0;
      if (elFlags & AC_EL_USAC_CP_POSSIBLE) {
        CP_active = pAacDecoderChannelInfo[ch]->pComData->jointStereoData.cplx_pred_flag;
      }

      /* Omit writing of pAacDecoderChannelInfo[ch]->specScale for complex stereo prediction
         since scaling has already been carried out. */
      int max_sfb_ste = (INT)(pAacDecoderChannelInfo[L]->icsInfo.max_sfb_ste);

      if (!(CP_active && (max_sfb_ste == noSfbs)) ||
          !(CP_active && !(pAacDecoderChannelInfo[ch]->pDynData->TnsData.Active))) {
        CBlock_ScaleSpectralData(pAacDecoderChannelInfo[ch], noSfbs, pSamplingRateInfo);

        if ((elFlags & AC_EL_ENHANCED_NOISE) && (elFlags & AC_EL_IGF_USE_ENF)) {
          /* Scale tiles if Complex Prediction has not already been run */
          CBlock_ScaleTileData(pAacDecoderStaticChannelInfo[ch], pAacDecoderChannelInfo[ch], noSfbs,
                               pSamplingRateInfo);
        }
      }
    }
  } /* End "for (ch = 0; ch < el_channels; ch++)" */

  if (maybe_jstereo) {
    /* apply ms */
    if (pAacDecoderChannelInfo[L]->pDynData->RawDataInfo.CommonWindow) {
      /* Apply IGF stereo */
      if ((elFlags & AC_EL_ENHANCED_NOISE) && !(elFlags & AC_EL_IGF_AFTER_TNS) &&
          !(elFlags & AC_EL_IGF_INDEP_TILING)) {
        UCHAR* iUseMSTab =
            (UCHAR*)pAacDecoderChannelInfo[0]->pComStaticData->pWorkBufferCore1->mdctOutTemp;

        for (int group = 0; group < GetWindowGroups(&pAacDecoderChannelInfo[L]->icsInfo); group++) {
          UCHAR groupMask = (UCHAR)1 << group;
          UCHAR* p2_MsUsed = pAacDecoderChannelInfo[L]->pComData->jointStereoData.MsUsed;
          UCHAR* p2_iUseMSTab = &iUseMSTab[group << 6];
          for (int band = 0; band < 64; band++) {
            UCHAR temp = 0;
            if (*p2_MsUsed++ & groupMask) temp = 1;
            *p2_iUseMSTab++ = temp;
          }
        }

        UCHAR* TNF_maskL =
            (UCHAR*)pAacDecoderChannelInfo[0]->pComStaticData->pWorkBufferCore1->mdctOutTemp + 512;
        UCHAR* TNF_maskR =
            (UCHAR*)pAacDecoderChannelInfo[0]->pComStaticData->pWorkBufferCore1->mdctOutTemp + 512 +
            1024;
        FDKmemclear(TNF_maskL, 1024);
        FDKmemclear(TNF_maskR, 1024);

        CIgf_apply_stereo(
            &(pAacDecoderStaticChannelInfo[L]->IGF_StaticData),
            &(pAacDecoderStaticChannelInfo[R]->IGF_StaticData),
            &(pAacDecoderChannelInfo[L]->IGFdata), &(pAacDecoderChannelInfo[R]->IGFdata),
            (FIXP_DBL*)(pAacDecoderChannelInfo[L]->pSpectralCoefficient),
            (FIXP_DBL*)(pAacDecoderChannelInfo[R]->pSpectralCoefficient),
            pAacDecoderChannelInfo[L]->specScale, pAacDecoderChannelInfo[R]->specScale,
            (IsLongBlock(&pAacDecoderChannelInfo[L]->icsInfo) == 1) ? IGF_GRID_LONG_WINDOW
                                                                    : IGF_GRID_SHORT_WINDOW,
            GetWindowGroups(&pAacDecoderChannelInfo[L]->icsInfo),
            IsLongBlock(&pAacDecoderChannelInfo[L]->icsInfo) ? 1 : 8,
            GetWindowGroupLengthTable(&pAacDecoderChannelInfo[L]->icsInfo),
            &(pAacDecoderStaticChannelInfo[L]->nfRandomSeed),
            &(pAacDecoderStaticChannelInfo[R]->nfRandomSeed), iUseMSTab, TNF_maskL, TNF_maskR,
            (UCHAR)(elFlags & AC_EL_IGF_USE_ENF ? 1 : 0), IGF_FRAME_DIVISION_AAC_OR_TCX_LONG);

        int igfStartSfb;

        if (IsLongBlock(&pAacDecoderChannelInfo[L]->icsInfo)) {
          igfStartSfb = pAacDecoderStaticChannelInfo[L]->IGF_StaticData.igfStartSfbLB;
        } else {
          igfStartSfb = pAacDecoderStaticChannelInfo[L]->IGF_StaticData.igfStartSfbSB;
        }

        if (pAacDecoderChannelInfo[L]->pComData->jointStereoData.IGF_MsMaskPresent != (UCHAR)3) {
          Clean_Complex_Prediction_coefficients(
              &pAacDecoderStaticChannelInfo[L]->pCpeStaticData->jointStereoPersistentData,
              GetWindowGroups(&pAacDecoderChannelInfo[L]->icsInfo), igfStartSfb, 64);
        }

        CJointStereo_ApplyMS_IGF(
            pAacDecoderStaticChannelInfo, pAacDecoderChannelInfo,
            pAacDecoderChannelInfo[L]->pSpectralCoefficient,
            pAacDecoderChannelInfo[R]->pSpectralCoefficient,
            GetScaleFactorBandOffsets(&pAacDecoderChannelInfo[L]->icsInfo, pSamplingRateInfo),
            GetWindowGroupLengthTable(&pAacDecoderChannelInfo[L]->icsInfo),
            GetWindowGroups(&pAacDecoderChannelInfo[L]->icsInfo));

        if (IsLongBlock(&pAacDecoderChannelInfo[L]->icsInfo)) {
          /* Apply CP/MS over tha auxiliary array in case of TNF */
          if ((pAacDecoderChannelInfo[L]->IGFdata.bitstreamData[0].igfUseEnfFlat) ||
              (pAacDecoderChannelInfo[R]->IGFdata.bitstreamData[0].igfUseEnfFlat)) {
            CJointStereo_ApplyMS_IGF(
                pAacDecoderStaticChannelInfo, pAacDecoderChannelInfo,
                pAacDecoderChannelInfo[L]->IGFdata.IGF_Common_channel_data_handle->virtualSpec,
                pAacDecoderChannelInfo[R]->IGFdata.IGF_Common_channel_data_handle->virtualSpec,
                GetScaleFactorBandOffsets(&pAacDecoderChannelInfo[L]->icsInfo, pSamplingRateInfo),
                GetWindowGroupLengthTable(&pAacDecoderChannelInfo[L]->icsInfo),
                GetWindowGroups(&pAacDecoderChannelInfo[L]->icsInfo));
          }

          /* Apply IGF - TNF */
          CIgf_TNF_apply(&pAacDecoderStaticChannelInfo[L]->IGF_StaticData,
                         &pAacDecoderChannelInfo[L]->IGFdata,
                         pAacDecoderChannelInfo[L]->pSpectralCoefficient,
                         pAacDecoderChannelInfo[L]->specScale, TNF_maskL, 0, 0);
          /* Apply IGF - TNF */
          CIgf_TNF_apply(&pAacDecoderStaticChannelInfo[L]->IGF_StaticData,
                         &pAacDecoderChannelInfo[R]->IGFdata,
                         pAacDecoderChannelInfo[R]->pSpectralCoefficient,
                         pAacDecoderChannelInfo[R]->specScale, TNF_maskR, 0, 0);
        }
      }

    } /* CommonWindow */
    else {
      if (elFlags & AC_EL_USAC_CP_POSSIBLE) {
        FDKmemclear(pAacDecoderStaticChannelInfo[L]
                        ->pCpeStaticData->jointStereoPersistentData.alpha_q_re_prev,
                    JointStereoMaximumGroups * JointStereoMaximumBands * sizeof(SHORT));
        FDKmemclear(pAacDecoderStaticChannelInfo[L]
                        ->pCpeStaticData->jointStereoPersistentData.alpha_q_im_prev,
                    JointStereoMaximumGroups * JointStereoMaximumBands * sizeof(SHORT));
      }
    }

  } /* if (maybe_jstereo) */

  for (ch = 0; ch < el_channels; ch++) {
    {
      if ((!(flags & (AC_USAC))) ||
          ((flags & (AC_USAC)) &&
           (pAacDecoderChannelInfo[L]->pDynData->specificTo.usac.tns_active == 1)) ||
          (maybe_jstereo == 0)) {
        ApplyTools(pAacDecoderStaticChannelInfo, pAacDecoderChannelInfo, pSamplingRateInfo, flags,
                   elFlags, ch, pAacDecoderChannelInfo[L]->pDynData->RawDataInfo.CommonWindow);
      }
    } /* End "} else" */
  }   /* End "for (ch = 0; ch < el_channels; ch++)" */

  if (maybe_jstereo) {
    /* apply ms */
    if (pAacDecoderChannelInfo[L]->pDynData->RawDataInfo.CommonWindow) {
      /* Apply IGF stereo */
      if ((elFlags & AC_EL_ENHANCED_NOISE) && (elFlags & AC_EL_IGF_AFTER_TNS) &&
          !(elFlags & AC_EL_IGF_INDEP_TILING)) {
        UCHAR* iUseMSTab =
            (UCHAR*)pAacDecoderChannelInfo[0]->pComStaticData->pWorkBufferCore1->mdctOutTemp;

        for (int group = 0; group < GetWindowGroups(&pAacDecoderChannelInfo[L]->icsInfo); group++) {
          UCHAR groupMask = (UCHAR)1 << group;
          UCHAR* p2_MsUsed = pAacDecoderChannelInfo[L]->pComData->jointStereoData.MsUsed;
          UCHAR* p2_iUseMSTab = &iUseMSTab[group << 6];
          for (int band = 0; band < 64; band++) {
            UCHAR temp = 0;
            if (*p2_MsUsed++ & groupMask) temp = 1;
            *p2_iUseMSTab++ = temp;
          }
        }

        UCHAR* TNF_maskL =
            (UCHAR*)pAacDecoderChannelInfo[0]->pComStaticData->pWorkBufferCore1->mdctOutTemp + 512;
        UCHAR* TNF_maskR =
            (UCHAR*)pAacDecoderChannelInfo[0]->pComStaticData->pWorkBufferCore1->mdctOutTemp + 512 +
            1024;
        FDKmemclear(TNF_maskL, 1024);
        FDKmemclear(TNF_maskR, 1024);

        CIgf_apply_stereo(
            &(pAacDecoderStaticChannelInfo[L]->IGF_StaticData),
            &(pAacDecoderStaticChannelInfo[R]->IGF_StaticData),
            &(pAacDecoderChannelInfo[L]->IGFdata), &(pAacDecoderChannelInfo[R]->IGFdata),
            (FIXP_DBL*)(pAacDecoderChannelInfo[L]->pSpectralCoefficient),
            (FIXP_DBL*)(pAacDecoderChannelInfo[R]->pSpectralCoefficient),
            pAacDecoderChannelInfo[L]->specScale, pAacDecoderChannelInfo[R]->specScale,
            (IsLongBlock(&pAacDecoderChannelInfo[L]->icsInfo) == 1) ? IGF_GRID_LONG_WINDOW
                                                                    : IGF_GRID_SHORT_WINDOW,
            GetWindowGroups(&pAacDecoderChannelInfo[L]->icsInfo),
            IsLongBlock(&pAacDecoderChannelInfo[L]->icsInfo) ? 1 : 8,
            GetWindowGroupLengthTable(&pAacDecoderChannelInfo[L]->icsInfo),
            &(pAacDecoderStaticChannelInfo[L]->nfRandomSeed),
            &(pAacDecoderStaticChannelInfo[R]->nfRandomSeed), iUseMSTab, TNF_maskL, TNF_maskR,
            (UCHAR)(elFlags & AC_EL_IGF_USE_ENF ? 1 : 0), IGF_FRAME_DIVISION_AAC_OR_TCX_LONG);

        int igfStartSfb;

        if (IsLongBlock(&pAacDecoderChannelInfo[L]->icsInfo)) {
          igfStartSfb = pAacDecoderStaticChannelInfo[L]->IGF_StaticData.igfStartSfbLB;
        } else {
          igfStartSfb = pAacDecoderStaticChannelInfo[L]->IGF_StaticData.igfStartSfbSB;
        }

        if (pAacDecoderChannelInfo[L]->pComData->jointStereoData.IGF_MsMaskPresent != (UCHAR)3) {
          Clean_Complex_Prediction_coefficients(
              &pAacDecoderStaticChannelInfo[L]->pCpeStaticData->jointStereoPersistentData,
              GetWindowGroups(&pAacDecoderChannelInfo[L]->icsInfo), igfStartSfb, 64);
        }

        CJointStereo_ApplyMS_IGF(
            pAacDecoderStaticChannelInfo, pAacDecoderChannelInfo,
            pAacDecoderChannelInfo[L]->pSpectralCoefficient,
            pAacDecoderChannelInfo[R]->pSpectralCoefficient,
            GetScaleFactorBandOffsets(&pAacDecoderChannelInfo[L]->icsInfo, pSamplingRateInfo),
            GetWindowGroupLengthTable(&pAacDecoderChannelInfo[L]->icsInfo),
            GetWindowGroups(&pAacDecoderChannelInfo[L]->icsInfo));

        if (IsLongBlock(&pAacDecoderChannelInfo[L]->icsInfo)) {
          /* Apply CP/MS over tha auxiliary array in case of TNF */
          if ((pAacDecoderChannelInfo[L]->IGFdata.bitstreamData[0].igfUseEnfFlat) ||
              (pAacDecoderChannelInfo[R]->IGFdata.bitstreamData[0].igfUseEnfFlat)) {
            CJointStereo_ApplyMS_IGF(
                pAacDecoderStaticChannelInfo, pAacDecoderChannelInfo,
                pAacDecoderChannelInfo[L]->IGFdata.IGF_Common_channel_data_handle->virtualSpec,
                pAacDecoderChannelInfo[R]->IGFdata.IGF_Common_channel_data_handle->virtualSpec,
                GetScaleFactorBandOffsets(&pAacDecoderChannelInfo[L]->icsInfo, pSamplingRateInfo),
                GetWindowGroupLengthTable(&pAacDecoderChannelInfo[L]->icsInfo),
                GetWindowGroups(&pAacDecoderChannelInfo[L]->icsInfo));
          }

          /* Apply IGF - TNF */
          CIgf_TNF_apply(&pAacDecoderStaticChannelInfo[L]->IGF_StaticData,
                         &pAacDecoderChannelInfo[L]->IGFdata,
                         pAacDecoderChannelInfo[L]->pSpectralCoefficient,
                         pAacDecoderChannelInfo[L]->specScale, TNF_maskL, 0, 0);
          /* Apply IGF - TNF */
          CIgf_TNF_apply(&pAacDecoderStaticChannelInfo[L]->IGF_StaticData,
                         &pAacDecoderChannelInfo[R]->IGFdata,
                         pAacDecoderChannelInfo[R]->pSpectralCoefficient,
                         pAacDecoderChannelInfo[R]->specScale, TNF_maskR, 0, 0);
        }
      }
    } /* if (pAacDecoderChannelInfo[L]->pDynData->RawDataInfo.CommonWindow) */

  } /* if (maybe_jstereo) */

  for (ch = 0; ch < el_channels; ch++) {
    if (elFlags & AC_EL_USAC_CP_POSSIBLE) {
      pAacDecoderStaticChannelInfo[L]
          ->pCpeStaticData->jointStereoPersistentData.clearSpectralCoeffs = 0;
      if (pAacDecoderChannelInfo[L]->pDynData->RawDataInfo.CommonWindow) {
        if ((pAacDecoderChannelInfo[L]->transform_splitting_active) !=
            (pAacDecoderChannelInfo[R]->transform_splitting_active)) {
          pAacDecoderStaticChannelInfo[L]
              ->pCpeStaticData->jointStereoPersistentData.clearSpectralCoeffs = 1;
        }
      }
    }
  }
}

void CChannel_CodebookTableInit(CAacDecoderChannelInfo* pAacDecoderChannelInfo) {
  int b, w, maxBands, maxWindows;
  int maxSfb = GetScaleFactorBandsTransmitted(&pAacDecoderChannelInfo->icsInfo);
  UCHAR* pCodeBook = pAacDecoderChannelInfo->pDynData->aCodeBook;

  if (IsLongBlock(&pAacDecoderChannelInfo->icsInfo)) {
    maxBands = 64;
    maxWindows = 1;
  } else {
    maxBands = 16;
    maxWindows = 8;
  }

  for (w = 0; w < maxWindows; w++) {
    for (b = 0; b < maxSfb; b++) {
      pCodeBook[b] = ESCBOOK;
    }
    for (; b < maxBands; b++) {
      pCodeBook[b] = ZERO_HCB;
    }
    pCodeBook += maxBands;
  }
}

static void EnhancedNoiseFilling(HANDLE_FDK_BITSTREAM hBs,
                                 CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo,
                                 CAacDecoderChannelInfo* pAacDecoderChannelInfo, const UINT flags,
                                 const UINT elFlags) {
  INT bUsacIndepFlag = 0;
  INT isShortWin;
  INT WindowGroups;

  if (IsLongBlock(&pAacDecoderChannelInfo->icsInfo)) {
    isShortWin = 0;
    WindowGroups = 1;
  } else {
    isShortWin = 1;
    WindowGroups = pAacDecoderChannelInfo->icsInfo.WindowGroups;
  }

  if (flags & AC_INDEP) {
    bUsacIndepFlag = 1;
  }

  iisIGFDecLibReadSCF(&(pAacDecoderStaticChannelInfo->IGF_StaticData),
                      &(pAacDecoderChannelInfo->IGFdata), hBs, bUsacIndepFlag, isShortWin,
                      WindowGroups, IGF_FRAME_DIVISION_AAC_OR_TCX_LONG);

  iisIGFDecLibReadIGF(&(pAacDecoderStaticChannelInfo->IGF_StaticData),
                      &(pAacDecoderChannelInfo->IGFdata), hBs, isShortWin, bUsacIndepFlag,
                      ((elFlags & AC_EL_IGF_USE_ENF) ? 1 : 0), IGF_FRAME_DIVISION_AAC_OR_TCX_LONG);
}

/*
 * Arbitrary order bitstream parser
 */
AAC_DECODER_ERROR CChannelElement_Read(HANDLE_FDK_BITSTREAM hBs,
                                       CAacDecoderChannelInfo* pAacDecoderChannelInfo[],
                                       CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo[],
                                       const AUDIO_OBJECT_TYPE aot,
                                       SamplingRateInfo* pSamplingRateInfo, const UINT flags,
                                       const UINT elFlags, const UINT frame_length,
                                       const UCHAR numberOfChannels, const SCHAR epConfig,
                                       HANDLE_TRANSPORTDEC pTpDec) {
  AAC_DECODER_ERROR error = AAC_DEC_OK;
  const element_list_t* list;
  int i, ch, decision_bit;
  int crcReg1 = -1, crcReg2 = -1;
  int cplxPred;
  int common_ltpf = 0;

  FDK_ASSERT((numberOfChannels == 1) || (numberOfChannels == 2));

  /* Get channel element sequence table */
  list = getBitstreamElementList(aot, epConfig, numberOfChannels, 0, elFlags);
  if (list == NULL) {
    error = AAC_DEC_UNSUPPORTED_FORMAT;
    goto bail;
  }

  CTns_Reset(&pAacDecoderChannelInfo[0]->pDynData->TnsData);
  /* Set common window to 0 by default. If signalized in the bit stream it will be overwritten later
   * explicitely */
  pAacDecoderChannelInfo[0]->pDynData->RawDataInfo.CommonWindow = 0;
  if (flags & (AC_USAC | AC_RSVD50 | AC_MPEGH3DA)) {
    pAacDecoderChannelInfo[0]->pDynData->specificTo.usac.tns_active = 0;
  }
  if (numberOfChannels == 2) {
    CTns_Reset(&pAacDecoderChannelInfo[1]->pDynData->TnsData);
    pAacDecoderChannelInfo[1]->pDynData->RawDataInfo.CommonWindow = 0;
  }

  cplxPred = 0;
  if (pAacDecoderStaticChannelInfo != NULL) {
    if (elFlags & AC_EL_USAC_CP_POSSIBLE) {
      pAacDecoderChannelInfo[0]->pComData->jointStereoData.cplx_pred_flag = 0;
      cplxPred = 1;
    }
  }

  /* Iterate through sequence table */
  i = 0;
  ch = 0;
  decision_bit = 0;
  do {
    switch (list->id[i]) {
      case enhancedNoiseFilling:
        if (elFlags & AC_EL_ENHANCED_NOISE) {
          if (pAacDecoderStaticChannelInfo[ch]->IGF_StaticData.enhancedNoiseFilling == 0) {
            return AAC_DEC_UNKNOWN; /* robustness: unrecoverable init error */
          }
          EnhancedNoiseFilling(hBs, pAacDecoderStaticChannelInfo[ch], pAacDecoderChannelInfo[ch],
                               flags, elFlags);
        }
        break;
      case element_instance_tag:
        pAacDecoderChannelInfo[0]->ElementInstanceTag = FDKreadBits(hBs, 4);
        if (numberOfChannels == 2) {
          pAacDecoderChannelInfo[1]->ElementInstanceTag =
              pAacDecoderChannelInfo[0]->ElementInstanceTag;
        }
        break;
      case common_window:
        decision_bit = pAacDecoderChannelInfo[ch]->pDynData->RawDataInfo.CommonWindow =
            FDKreadBit(hBs);
        if (numberOfChannels == 2) {
          pAacDecoderChannelInfo[1]->pDynData->RawDataInfo.CommonWindow =
              pAacDecoderChannelInfo[0]->pDynData->RawDataInfo.CommonWindow;
        }
        break;
      case ics_info:
        /* store last window sequence (utilized in complex stereo prediction) before reading new
         * channel-info */
        if (cplxPred) {
          if (pAacDecoderChannelInfo[0]->pDynData->RawDataInfo.CommonWindow) {
            pAacDecoderStaticChannelInfo[0]->pCpeStaticData->jointStereoPersistentData.winSeqPrev =
                pAacDecoderChannelInfo[0]->icsInfo.WindowSequence;
            pAacDecoderStaticChannelInfo[0]
                ->pCpeStaticData->jointStereoPersistentData.winShapePrev =
                pAacDecoderChannelInfo[0]->icsInfo.WindowShape;
          }
        }
        /* Read individual channel info */
        if (flags & AC_MPEGH3DA) {
          pAacDecoderStaticChannelInfo[ch]->prevWindowShape =
              pAacDecoderChannelInfo[ch]->icsInfo.WindowShape;
        }
        error = IcsRead(hBs, &pAacDecoderChannelInfo[ch]->icsInfo, pSamplingRateInfo, flags);

        if (elFlags & AC_EL_LFE &&
            GetWindowSequence(&pAacDecoderChannelInfo[ch]->icsInfo) != BLOCK_LONG) {
          error = AAC_DEC_PARSE_ERROR;
          break;
        }

        if (numberOfChannels == 2 &&
            pAacDecoderChannelInfo[0]->pDynData->RawDataInfo.CommonWindow) {
          pAacDecoderChannelInfo[1]->icsInfo = pAacDecoderChannelInfo[0]->icsInfo;
        }
        break;

      case common_max_sfb:
        if (FDKreadBit(hBs) == 0) {
          error = AAC_DEC_PARSE_ERROR;
          break;
        }
        break;

      case ltp_data_present:
        if (FDKreadBit(hBs) != 0) {
          error = AAC_DEC_UNSUPPORTED_PREDICTION;
        }
        break;

      case ms:

        INT max_sfb_ste;
        INT max_sfb_ste_clear;
        INT igfStartSfb;
        INT igfStopSfb;
        INT igfUseEnhancesNoise;

        max_sfb_ste = GetScaleMaxFactorBandsTransmitted(&pAacDecoderChannelInfo[0]->icsInfo,
                                                        &pAacDecoderChannelInfo[1]->icsInfo);

        max_sfb_ste_clear = 64;

        igfUseEnhancesNoise =
            (elFlags & AC_EL_ENHANCED_NOISE) && !(elFlags & AC_EL_IGF_INDEP_TILING);
        if (igfUseEnhancesNoise) {
          if (IsLongBlock(&pAacDecoderChannelInfo[ch]->icsInfo)) {
            igfStartSfb = pAacDecoderStaticChannelInfo[ch]->IGF_StaticData.igfStartSfbLB;
            igfStopSfb = pAacDecoderStaticChannelInfo[ch]->IGF_StaticData.igfStopSfbLB;
          } else {
            igfStartSfb = pAacDecoderStaticChannelInfo[ch]->IGF_StaticData.igfStartSfbSB;
            igfStopSfb = pAacDecoderStaticChannelInfo[ch]->IGF_StaticData.igfStopSfbSB;
          }

          max_sfb_ste = fMin(max_sfb_ste, igfStartSfb);
          max_sfb_ste_clear = fMin(max_sfb_ste_clear, igfStartSfb);

          if ((max_sfb_ste > pAacDecoderChannelInfo[0]->icsInfo.MaxSfBands) ||
              (max_sfb_ste > pAacDecoderChannelInfo[1]->icsInfo.MaxSfBands)) {
            error = AAC_DEC_PARSE_ERROR;
          }
        }

        pAacDecoderChannelInfo[0]->icsInfo.max_sfb_ste = (UCHAR)max_sfb_ste;
        pAacDecoderChannelInfo[1]->icsInfo.max_sfb_ste = (UCHAR)max_sfb_ste;

        if (flags & (AC_USAC | AC_MPEGH3DA) &&
            pAacDecoderChannelInfo[ch]->pDynData->RawDataInfo.CommonWindow == 0) {
          Clean_Complex_Prediction_coefficients(
              &pAacDecoderStaticChannelInfo[0]->pCpeStaticData->jointStereoPersistentData,
              GetWindowGroups(&pAacDecoderChannelInfo[0]->icsInfo), 0, 64);
        }

        if (CJointStereo_Read(
                hBs, &pAacDecoderChannelInfo[0]->pComData->jointStereoData,
                GetWindowGroups(&pAacDecoderChannelInfo[0]->icsInfo), max_sfb_ste,
                max_sfb_ste_clear,
                /* jointStereoPersistentData and cplxPredictionData are only available/allocated if
                   cplxPred is active. */
                ((cplxPred == 0) || (pAacDecoderStaticChannelInfo == NULL))
                    ? NULL
                    : &pAacDecoderStaticChannelInfo[0]->pCpeStaticData->jointStereoPersistentData,
                ((cplxPred == 0) || (pAacDecoderChannelInfo[0] == NULL))
                    ? NULL
                    : pAacDecoderChannelInfo[0]->pComStaticData->cplxPredictionData,
                cplxPred, GetScaleFactorBandsTotal(&pAacDecoderChannelInfo[0]->icsInfo),
                GetWindowSequence(&pAacDecoderChannelInfo[0]->icsInfo), flags)) {
          error = AAC_DEC_PARSE_ERROR;
        }

        if (igfUseEnhancesNoise) {
          if (CJointStereo_ReadIGF(
                  hBs, &pAacDecoderChannelInfo[0]->pComData->jointStereoData,
                  GetWindowGroups(&pAacDecoderChannelInfo[0]->icsInfo), igfStartSfb, igfStopSfb,
                  pAacDecoderStaticChannelInfo[ch]->IGF_StaticData.useHighRes,
                  &pAacDecoderStaticChannelInfo[0]->pCpeStaticData->jointStereoPersistentData,
                  pAacDecoderChannelInfo[0]->pComStaticData->cplxPredictionData,
                  GetWindowSequence(&pAacDecoderChannelInfo[0]->icsInfo), flags)) {
            error = AAC_DEC_PARSE_ERROR;
          }
        } else if (elFlags & AC_EL_USAC_CP_POSSIBLE) {
          INT group;
          INT runs = GetScaleFactorBandsTotal(&pAacDecoderChannelInfo[0]->icsInfo) - max_sfb_ste;
          if (runs > 0) {
            CCplxPredictionData* cPD =
                pAacDecoderChannelInfo[0]->pComStaticData->cplxPredictionData;
            for (group = 0; group < 8; group++) {
              FDKmemclear(&cPD->alpha_q_re[group][max_sfb_ste], runs * sizeof(SHORT));
              FDKmemclear(&cPD->alpha_q_im[group][max_sfb_ste], runs * sizeof(SHORT));
            }
          }
        }

        break;

      case global_gain:
        pAacDecoderChannelInfo[ch]->pDynData->RawDataInfo.GlobalGain = (UCHAR)FDKreadBits(hBs, 8);
        break;

      case scale_factor_data_usac:
        if (aot == AOT_MPEGH3DA) {
          if (!common_ltpf) {
            if (readLtpParam(hBs, pAacDecoderStaticChannelInfo[ch]->ltp_param,
                             elFlags & AC_EL_LFE)) {
              error = AAC_DEC_PARSE_ERROR;
              break;
            }
          } else if (ch == 1) {
            FDKmemcpy(pAacDecoderStaticChannelInfo[1]->ltp_param,
                      pAacDecoderStaticChannelInfo[0]->ltp_param,
                      sizeof(pAacDecoderStaticChannelInfo[0]->ltp_param));
          }
          pAacDecoderChannelInfo[ch]->fdp_spacing_index = -1;
          if (!(flags & AC_INDEP) &&
              (GetWindowSequence(&pAacDecoderChannelInfo[ch]->icsInfo) != BLOCK_SHORT)) {
            pAacDecoderChannelInfo[ch]->fdp_data_present = FDKreadBit(hBs); /* fdp_data_present */
            if (pAacDecoderChannelInfo[ch]->fdp_data_present) {
              if (elFlags & AC_EL_LFE) {
                error = AAC_DEC_PARSE_ERROR;
                break;
              }
              pAacDecoderChannelInfo[ch]->fdp_spacing_index =
                  FDKreadBits(hBs, 8); /* fdp_spacing_index */
            }
          }
          if (flags & AC_INDEP) {
            pAacDecoderStaticChannelInfo[ch]->IMdct.prevAliasSymmetry =
                FDKreadBit(hBs); /* prev_aliasing_symmetry */
          }
          pAacDecoderChannelInfo[ch]->currAliasingSymmetry =
              FDKreadBit(hBs); /* curr_aliasing_symmetry */
          if (elFlags & AC_EL_LFE && (pAacDecoderStaticChannelInfo[ch]->IMdct.prevAliasSymmetry ||
                                      pAacDecoderChannelInfo[ch]->currAliasingSymmetry)) {
            error = AAC_DEC_PARSE_ERROR;
            break;
          }
        }
        /* Set active sfb codebook indexes to HCB_ESC to make them "active" */
        CChannel_CodebookTableInit(
            pAacDecoderChannelInfo[ch]); /*  equals ReadSectionData(self, bs) in float soft. block.c
                                            line: ~599 */
        /* Note: The missing "break" is intentional here, since we need to call
         * CBlock_ReadScaleFactorData(). */
        FDK_FALLTHROUGH;

      case scale_factor_data:
        if (flags & AC_ER_RVLC) {
        } else {
          error = CBlock_ReadScaleFactorData(pAacDecoderChannelInfo[ch], hBs, flags);
        }
        break;

      case tns_data_present:
        CTns_ReadDataPresentFlag(hBs, &pAacDecoderChannelInfo[ch]->pDynData->TnsData);
        if (elFlags & AC_EL_LFE && pAacDecoderChannelInfo[ch]->pDynData->TnsData.DataPresent) {
          error = AAC_DEC_PARSE_ERROR;
        }
        break;
      case tns_data:
        /* tns_data_present is checked inside CTns_Read(). */
        error = CTns_Read(hBs, &pAacDecoderChannelInfo[ch]->pDynData->TnsData,
                          &pAacDecoderChannelInfo[ch]->icsInfo, flags);

        break;

      case gain_control_data:
        break;

      case gain_control_data_present:
        if (FDKreadBit(hBs)) {
          error = AAC_DEC_UNSUPPORTED_GAIN_CONTROL_DATA;
        }
        break;

      case tw_data:
        break;
      case common_tw:
        if (aot == AOT_MPEGH3DA) {
          common_ltpf = FDKreadBit(hBs);
          if (common_ltpf) {
            if (readLtpParam(hBs, pAacDecoderStaticChannelInfo[ch]->ltp_param,
                             elFlags & AC_EL_LFE)) {
              error = AAC_DEC_PARSE_ERROR;
              break;
            }
          }
        }
        break;
      case tns_data_present_usac:
        if (pAacDecoderChannelInfo[0]->pDynData->specificTo.usac.tns_active) {
          UCHAR tns_on_lr;
          CTns_ReadDataPresentUsac(hBs, &pAacDecoderChannelInfo[0]->pDynData->TnsData,
                                   &pAacDecoderChannelInfo[1]->pDynData->TnsData, &tns_on_lr,
                                   &pAacDecoderChannelInfo[0]->icsInfo, flags, elFlags,
                                   pAacDecoderChannelInfo[0]->pDynData->RawDataInfo.CommonWindow);
          if (tns_on_lr == 0) {
            error = AAC_DEC_PARSE_ERROR;
            break;
          }
        }
        break;
      case core_mode:
        decision_bit = FDKreadBit(hBs);
        if (decision_bit) {
          error = AAC_DEC_PARSE_ERROR;
          break;
        }
        break;
      case tns_active:
        pAacDecoderChannelInfo[0]->pDynData->specificTo.usac.tns_active = FDKreadBit(hBs);
        break;
      case noise:
        if (elFlags & AC_EL_USAC_NOISE) {
          pAacDecoderChannelInfo[ch]->pDynData->specificTo.usac.fd_noise_level_and_offset =
              FDKreadBits(hBs, 3 + 5); /* Noise level */
        }
        break;
      case fac_data: {
        int fFacDatPresent = FDKreadBit(hBs);

        if (fFacDatPresent) {
          error = AAC_DEC_PARSE_ERROR;
          break;
        }
      } break;
      case esc2_rvlc:
        if (flags & AC_ER_RVLC) {
        }
        break;

      case esc1_hcr:
        if (flags & AC_ER_HCR) {
        }
        break;

      case ac_spectral_data:
        error = CBlock_ReadAcSpectralData(hBs, pAacDecoderChannelInfo[ch],
                                          pAacDecoderStaticChannelInfo[ch], pSamplingRateInfo,
                                          frame_length, flags);
        pAacDecoderChannelInfo[ch]->renderMode = AACDEC_RENDER_IMDCT;
        break;

        /* CRC handling */
      case adtscrc_start_reg1:
        if (pTpDec != NULL) {
          crcReg1 = transportDec_CrcStartReg(pTpDec, 192);
        }
        break;
      case adtscrc_start_reg2:
        if (pTpDec != NULL) {
          crcReg2 = transportDec_CrcStartReg(pTpDec, 128);
        }
        break;
      case adtscrc_end_reg1:
      case drmcrc_end_reg:
        if (pTpDec != NULL) {
          transportDec_CrcEndReg(pTpDec, crcReg1);
          crcReg1 = -1;
        }
        break;
      case adtscrc_end_reg2:
        if (crcReg1 != -1) {
          error = AAC_DEC_DECODE_FRAME_ERROR;
        } else if (pTpDec != NULL) {
          transportDec_CrcEndReg(pTpDec, crcReg2);
          crcReg2 = -1;
        }
        break;
      case drmcrc_start_reg:
        if (pTpDec != NULL) {
          crcReg1 = transportDec_CrcStartReg(pTpDec, 0);
        }
        break;

        /* Non data cases */
      case next_channel:
        ch = (ch + 1) % numberOfChannels;
        break;
      case link_sequence:
        list = list->next[decision_bit];
        i = -1;
        break;

      default:
        error = AAC_DEC_UNSUPPORTED_FORMAT;
        break;
    }

    if (error != AAC_DEC_OK) {
      goto bail;
    }

    i++;

  } while (list->id[i] != end_of_sequence);

  for (ch = 0; ch < numberOfChannels; ch++) {
    if (pAacDecoderChannelInfo[ch]->renderMode == AACDEC_RENDER_IMDCT ||
        pAacDecoderChannelInfo[ch]->renderMode == AACDEC_RENDER_ELDFB) {
      /* Copy lower part of coded spectrum for later use in FD Prediction */
      FIXP_DBL quantSpecCurr[172];
      FDKmemcpy(quantSpecCurr, pAacDecoderChannelInfo[ch]->pSpectralCoefficient,
                172 * sizeof(quantSpecCurr[0]));

      /* IGF Stereo Filling start/stop */
      UCHAR igf_StereoFilling = 0;
      /* Check for stereo filling only when MPEG-H stream is available */
      if (aot == AOT_MPEGH3DA) {
        pAacDecoderChannelInfo[ch]->transform_splitting_active = 0;

        if (elFlags & AC_EL_USAC_NOISE) {
          UCHAR temp;
          temp = pAacDecoderChannelInfo[ch]->pDynData->specificTo.usac.fd_noise_level_and_offset;
          /* When "noise_level"=0 and "noise_offset"!=0 => check if transform splitting or stereo
           * filling and rearrange noise bits */
          if ((temp > 0) && (temp < 32)) {
            /* Activate stereo filling when the window of the two channels is common and this is the
             * right (second) channel */
            if ((pAacDecoderChannelInfo[0]->pDynData->RawDataInfo.CommonWindow == 1) && (ch == 1)) {
              temp <<= 3;
              pAacDecoderChannelInfo[ch]->pDynData->specificTo.usac.fd_noise_level_and_offset =
                  temp;
              /* Activate Stereo Filling */
              igf_StereoFilling = 1;
            }

            /* Activate transform splitting for LONG_START_SEQUENCE and STOP_START_SEQUENCE only. */
            if (pAacDecoderChannelInfo[ch]->icsInfo.WindowSequence == BLOCK_START && (ch == 0)) {
              temp <<= 3;
              pAacDecoderChannelInfo[ch]->pDynData->specificTo.usac.fd_noise_level_and_offset =
                  temp;
              /* Signal transform splitting */
              pAacDecoderChannelInfo[ch]->transform_splitting_active = 1;
            }

          } /* if((temp > 0)&&(temp < 32)) */

          if ((pAacDecoderChannelInfo[0]->pDynData->RawDataInfo.CommonWindow) && (ch == 1)) {
            pAacDecoderChannelInfo[ch]->transform_splitting_active =
                pAacDecoderChannelInfo[0]->transform_splitting_active;
          }

        } /* if (elFlags & AC_EL_USAC_NOISE) */

      } /* if(aot==AOT_MPEGH3DA) */

      /* Shows which bands are empty. */
      UCHAR* band_is_noise = pAacDecoderChannelInfo[ch]->pDynData->band_is_noise;
      FDKmemset(band_is_noise, (UCHAR)1, sizeof(UCHAR) * (8 * 16));

      error = CBlock_InverseQuantizeSpectralData(pAacDecoderChannelInfo[ch], pSamplingRateInfo,
                                                 band_is_noise, 1);
      if (error != AAC_DEC_OK) {
        return error;
      }

      /* Syncronize channel tiling and whitening info for IGF if necessary */
      /* Just for stereo */
      if ((elFlags & AC_EL_ENHANCED_NOISE) && !(elFlags & AC_EL_IGF_INDEP_TILING)) {
        /* Sync once */
        if ((numberOfChannels == 2) && (ch == L)) {
          iisIGF_Sync_Data(&pAacDecoderChannelInfo[L]->IGFdata,
                           &pAacDecoderChannelInfo[R]->IGFdata);
        }
      }

      /* Copy the base spectrum into tiles (up to 4) for subsequent IGF processing in case INF is
      active. Copying is implemented before the noise filling stage, i.e. tiles are sparsely
      populated */
      if ((elFlags & AC_EL_ENHANCED_NOISE) && (elFlags & AC_EL_IGF_USE_ENF)) {
        iisIGFDecLibInjectSourceSpectrumNew(
            &pAacDecoderStaticChannelInfo[ch]->IGF_StaticData, &pAacDecoderChannelInfo[ch]->IGFdata,
            pAacDecoderChannelInfo[ch]->pSpectralCoefficient,
            pAacDecoderChannelInfo[ch]->pDynData->aSfbScale,
            !IsLongBlock(&pAacDecoderChannelInfo[ch]->icsInfo), IGF_FRAME_DIVISION_AAC_OR_TCX_LONG);
      }

      UCHAR max_noise_sfb = 0;
      if (elFlags & AC_EL_USAC_NOISE) {
        max_noise_sfb = GetScaleFactorBandsTransmitted(&pAacDecoderChannelInfo[ch]->icsInfo);

        if (elFlags & AC_EL_ENHANCED_NOISE) {
          if (pAacDecoderStaticChannelInfo[ch]->IGF_StaticData.enhancedNoiseFilling) {
            if (IsLongBlock(&pAacDecoderChannelInfo[ch]->icsInfo)) {
              if (max_noise_sfb > pAacDecoderStaticChannelInfo[ch]->IGF_StaticData.igfStartSfbLB) {
                max_noise_sfb =
                    (UCHAR)pAacDecoderStaticChannelInfo[ch]->IGF_StaticData.igfStartSfbLB;
              }
            } else {
              if (max_noise_sfb > pAacDecoderStaticChannelInfo[ch]->IGF_StaticData.igfStartSfbSB) {
                max_noise_sfb =
                    (UCHAR)pAacDecoderStaticChannelInfo[ch]->IGF_StaticData.igfStartSfbSB;
              }
            }
          }

          /* Noise filling is applied after the noise params for the empty sfb-bands have been
           * modified. */
          /* If INF is active, then fill the separate tiles with noise independently */
          CBlock_ApplyNoise(pAacDecoderStaticChannelInfo[ch], pAacDecoderChannelInfo[ch],
                            pSamplingRateInfo, &pAacDecoderStaticChannelInfo[ch]->nfRandomSeed,
                            band_is_noise, ((elFlags & AC_EL_IGF_USE_ENF) ? 1 : 0),
                            igf_StereoFilling, max_noise_sfb);

        } /* if(elFlags & AC_EL_ENHANCED_NOISE) */
        else {
          CBlock_ApplyNoise(pAacDecoderStaticChannelInfo[ch], pAacDecoderChannelInfo[ch],
                            pSamplingRateInfo, &pAacDecoderStaticChannelInfo[ch]->nfRandomSeed,
                            band_is_noise, 0, 0, max_noise_sfb);
        }

        /*Entered only for particular noise values and when Channel=1 */
        if (igf_StereoFilling) {
          /* Stereo filling is active only for long frames*/
          if (IsLongBlock(&pAacDecoderChannelInfo[ch]->icsInfo)) {
            /* We use this array as a temporary one in "StereoFilling_GetPreviousDmx_IGF" and we
             * fill it again in "IGF_StereoFillingPrepare" */
            FIXP_DBL* dmx_prev_modified =
                pAacDecoderChannelInfo[0]->pComStaticData->pWorkBufferCore1->mdctOutTemp;
            FDKmemclear(dmx_prev_modified, 1024 * sizeof(LONG));
            SHORT dmx_prev_modified_exp[64];
            FDKmemclear(dmx_prev_modified_exp, 64 * sizeof(SHORT));

            /* We calculate the previous downmix for the fully noise-filled bands only and put it in
             * "dmx_prev_modified" */
            IGF_StereoFilling_GetPreviousDmx(
                pAacDecoderStaticChannelInfo, pAacDecoderChannelInfo, dmx_prev_modified,
                dmx_prev_modified_exp,
                GetScaleFactorBandOffsets(&pAacDecoderChannelInfo[L]->icsInfo, pSamplingRateInfo),
                GetWindowGroupLengthTable(&pAacDecoderChannelInfo[L]->icsInfo),
                GetWindowGroups(&pAacDecoderChannelInfo[L]->icsInfo), max_noise_sfb, band_is_noise);

            /* Change frame-wise scaling factors to sfb-wise ones by copying them up for each and
             * every sfb */
            for (i = 1; i < 64; i++) {
              dmx_prev_modified_exp[i] = dmx_prev_modified_exp[0];
            }

            /* Stereo Filling parameter calculation */
            IGF_StereoFillingPrepare(pAacDecoderChannelInfo[R], pSamplingRateInfo, max_noise_sfb,
                                     dmx_prev_modified, dmx_prev_modified_exp, band_is_noise);

            if (elFlags & AC_EL_IGF_USE_ENF) {
              /* Get the number of tiles */
              INT NumberOfTiles =
                  iisIGFDecLibGetNumberOfTiles(&pAacDecoderStaticChannelInfo[R]->IGF_StaticData, 0);

              /* Over all tiles */
              for (int tileIdx = 0; tileIdx < NumberOfTiles; tileIdx++) {
                /* Get the spectrum of the particular tile */
                FIXP_DBL* p2_tile_spectrum = iisIGFDecLibAccessSourceSpectrum(
                    &pAacDecoderStaticChannelInfo[R]->IGF_StaticData,
                    &pAacDecoderChannelInfo[R]->IGFdata, tileIdx, 0, 0);

                /* Get pointer to the exponent of the particular tile */
                SHORT* p2_tile_spectrum_exp = iisIGFDecLibAccessSourceSpectrum_exponent(
                    &pAacDecoderStaticChannelInfo[R]->IGF_StaticData, tileIdx, 0);

                /* Apply Stereo Filling to the particular tile */
                IGF_StereoFillingApply(pAacDecoderChannelInfo[R], p2_tile_spectrum,
                                       p2_tile_spectrum_exp, pSamplingRateInfo, max_noise_sfb,
                                       dmx_prev_modified, dmx_prev_modified_exp, band_is_noise);
              }
            }

            /* Get the sfb scale data */
            SHORT* p2_SpectralCoefficient_exp = pAacDecoderChannelInfo[R]->pDynData->aSfbScale;

            /* Stereo Filling application */
            IGF_StereoFillingApply(pAacDecoderChannelInfo[R],
                                   pAacDecoderChannelInfo[R]->pSpectralCoefficient,
                                   p2_SpectralCoefficient_exp, pSamplingRateInfo, max_noise_sfb,
                                   dmx_prev_modified, dmx_prev_modified_exp, band_is_noise);

          } /* if(IsLongBlock(&pAacDecoderChannelInfo[ch]->icsInfo)) */
        }

      } /* if (elFlags & AC_EL_USAC_NOISE) */

      if (flags & AC_MPEGH3DA) {
        const int predictionBandwidth =
            (frame_length != 768) && (pSamplingRateInfo->samplingRate >= 44100)
                ? 132
                : (160 * frame_length) / 1024;

        FDP_DecodeBins(
            pAacDecoderChannelInfo[ch], pAacDecoderStaticChannelInfo[ch], flags, quantSpecCurr,
            GetScaleFactorBandOffsets(&pAacDecoderChannelInfo[ch]->icsInfo, pSamplingRateInfo),
            predictionBandwidth, 0, /* isTcx */
            0,                      /* tcx => i_gain_m */
            0,                      /* tcx => i_gain_e */
            0,                      /* tcx => g_div64 */
            0                       /* tcx => lg */
        );

        /* FDP for IGF tiles when INF is active */
        if ((elFlags & AC_EL_ENHANCED_NOISE) && (elFlags & AC_EL_IGF_USE_ENF)) {
          if (!(elFlags & AC_EL_USAC_NOISE)) {
            max_noise_sfb = GetScaleFactorBandsTransmitted(&pAacDecoderChannelInfo[ch]->icsInfo);
          }
          if ((IsLongBlock(&pAacDecoderChannelInfo[ch]->icsInfo)) &&
              (pAacDecoderChannelInfo[ch]->fdp_spacing_index >= 0) && (max_noise_sfb > 0)) {
            int NumberOfTiles = iisIGFDecLibGetNumberOfTiles(
                &(pAacDecoderStaticChannelInfo[ch]->IGF_StaticData), 0);
            const SHORT* sfb_offset =
                GetScaleFactorBandOffsets(&pAacDecoderChannelInfo[ch]->icsInfo, pSamplingRateInfo);
            int max_num_bins_copied = fMin(predictionBandwidth, (int)sfb_offset[max_noise_sfb - 1]);

            int sfb_max_num_bins_copied, sfb;
            /* determine sfb for max_num_bins_copied */
            for (sfb = 0; sfb_offset[sfb] < max_num_bins_copied; sfb++) {
              ;
            }
            sfb_max_num_bins_copied = sfb;

            for (int tileIdx = 0; tileIdx < NumberOfTiles; tileIdx++) {
              /* Get the spectrum of the particular tile */
              FIXP_DBL* p2_tile_spectrum = iisIGFDecLibAccessSourceSpectrum(
                  &pAacDecoderStaticChannelInfo[ch]->IGF_StaticData,
                  &pAacDecoderChannelInfo[ch]->IGFdata, tileIdx, 0, 0);

              /* Get pointer to the exponent of the particular tile */
              SHORT* p2_tile_spectrum_exp = iisIGFDecLibAccessSourceSpectrum_exponent(
                  &pAacDecoderStaticChannelInfo[ch]->IGF_StaticData, tileIdx, 0);

              FDKmemcpy(p2_tile_spectrum, pAacDecoderChannelInfo[ch]->pSpectralCoefficient,
                        sizeof(FIXP_DBL) * sfb_offset[sfb_max_num_bins_copied]);

              FDKmemcpy(p2_tile_spectrum_exp, pAacDecoderChannelInfo[ch]->pDynData->aSfbScale,
                        sizeof(SHORT) * sfb_max_num_bins_copied);
            }
          }

        } /* if((elFlags & AC_EL_ENHANCED_NOISE) && (elFlags & AC_EL_IGF_USE_ENF)) */

      } /* if (flags & AC_MPEGH3DA) */
    }
  }

bail:
  if (crcReg1 != -1 || crcReg2 != -1) {
    if (error == AAC_DEC_OK) {
      error = AAC_DEC_DECODE_FRAME_ERROR;
    }
    if (crcReg1 != -1) {
      transportDec_CrcEndReg(pTpDec, crcReg1);
    }
    if (crcReg2 != -1) {
      transportDec_CrcEndReg(pTpDec, crcReg2);
    }
  }
  return error;
}
