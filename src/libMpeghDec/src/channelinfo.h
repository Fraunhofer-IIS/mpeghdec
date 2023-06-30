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

#ifndef CHANNELINFO_H
#define CHANNELINFO_H

#include "common_fix.h"

#include "aac_rom.h"
#include "aacdecoder_lib.h"
#include "FDK_bitstream.h"
#include "overlapadd.h"

#include "mdct.h"
#include "stereo.h"
#include "aacdec_tns.h"

#include "FDK_igfDec.h"

#include "ac_arith_coder.h"

#include "conceal_types.h"

#include "ltp_post.h"

#define WB_SECTION_SIZE (1024 * 2)

/* Output rendering mode */
typedef enum {
  AACDEC_RENDER_INVALID = 0,
  AACDEC_RENDER_IMDCT,
  AACDEC_RENDER_ELDFB,
  AACDEC_RENDER_LPD,
  AACDEC_RENDER_INTIMDCT
} AACDEC_RENDER_MODE;

enum { MAX_QUANTIZED_VALUE = 8191 };

typedef struct {
  const SHORT* ScaleFactorBands_Long;
  const SHORT* ScaleFactorBands_Short;
  UCHAR NumberOfScaleFactorBands_Long;
  UCHAR NumberOfScaleFactorBands_Short;
  UINT samplingRateIndex;
  UINT samplingRate;
} SamplingRateInfo;

typedef struct {
  UCHAR CommonWindow;
  UCHAR GlobalGain;

} CRawDataInfo;

typedef struct {
  UCHAR WindowGroupLength[8];
  UCHAR WindowGroups;
  UCHAR Valid;

  UCHAR WindowShape;         /* 0: sine window, 1: KBD, 2: low overlap */
  BLOCK_TYPE WindowSequence; /* mdct.h; 0: long, 1: start, 2: short, 3: stop */
  UCHAR MaxSfBands;
  UCHAR max_sfb_ste;
  UCHAR ScaleFactorGrouping;

  UCHAR TotalSfBands;

} CIcsInfo;

enum {
  ZERO_HCB = 0,
  ESCBOOK = 11,
  NSPECBOOKS = ESCBOOK + 1,
  BOOKSCL = NSPECBOOKS,
  NOISE_HCB = 13,
  INTENSITY_HCB2 = 14,
  INTENSITY_HCB = 15,
  LAST_HCB
};

/* This struct holds the persistent data shared by both channels of a CPE.
   It needs to be allocated for each CPE. */
typedef struct {
  CJointStereoPersistentData jointStereoPersistentData;
} CpePersistentData;

/*
 * This struct must be allocated one for every channel and must be persistent.
 */
typedef struct {
  FIXP_DBL* pOverlapBuffer;
  mdct_t IMdct;

  CArcoData* hArCo;

  ULONG nfRandomSeed; /* seed value for USAC noise filling random generator */

  CConcealmentInfo concealmentInfo;

  CpePersistentData* pCpeStaticData;

  IGF_PRIVATE_STATIC_DATA IGF_StaticData;

  int ltp_param[3];
  int ltp_pitch_int_past;
  int ltp_pitch_fr_past;
  FIXP_SGL ltp_gain_past;
  int ltp_gainIdx_past;
  FIXP_SGL ltp_mem_in[LTP_MEM_IN_SIZE];
  FIXP_SGL ltp_mem_out[LTP_MEM_OUT_SIZE];

  /* Frequency Domain Prediction (FDP) tool */
  short quantSpecPrev1[160];
  short quantSpecPrev2[160];
  UCHAR prevWindowShape;

} CAacDecoderStaticChannelInfo;

/*
 * This union must be allocated for every element (up to 2 channels).
 */
typedef struct {
  /* Common bit stream data */
  SHORT aScaleFactor[(8 * 16)]; /* Spectral scale factors for each sfb in each window. */
  SHORT aSfbScale[(8 * 16)];    /* could be free after ApplyTools() */
  UCHAR aCodeBook[(8 * 16)];    /* section data: codebook for each window and sfb. */
  UCHAR band_is_noise[(8 * 16)];
  CTnsData TnsData;
  CRawDataInfo RawDataInfo;

  shouldBeUnion {
    struct {
    } aac;
    struct {
      UCHAR fd_noise_level_and_offset;
      UCHAR tns_active;
    } usac;
  }
  specificTo;

  IGF_PRIVATE_DATA_COMMON IGF_Common_channel_data;

} CAacDecoderDynamicData;

typedef shouldBeUnion {
  /* Common signal data, can be used once the bit stream data from above is not used anymore. */
  FIXP_DBL mdctOutTemp[1024];

  FIXP_DBL workBuffer[WB_SECTION_SIZE];
}
CWorkBufferCore1;

/* Common data referenced by all channels */
typedef struct {
  CAacDecoderDynamicData pAacDecoderDynamicData[2];

  CJointStereoData jointStereoData; /* One for one element */

  shouldBeUnion {
    struct {
    } aac;
  }
  overlay;

} CAacDecoderCommonData;

typedef struct {
  CWorkBufferCore1* pWorkBufferCore1;
  CCplxPredictionData* cplxPredictionData;
} CAacDecoderCommonStaticData;

/*
 * This struct must be allocated one for every channel of every element and must be persistent.
 * Among its members, the following memory areas can be overwritten under the given conditions:
 *  - pSpectralCoefficient The memory pointed to can be overwritten after time signal rendering.
 *  - data can be overwritten after time signal rendering.
 *  - pDynData memory pointed to can be overwritten after each CChannelElement_Decode() call.
 *  - pComData->overlay memory pointed to can be overwritten after each CChannelElement_Decode()
 * call..
 */
typedef struct {
  shouldBeUnion {
    struct {
    } usac;

    struct {
    } aac;
  }
  data;

  SPECTRAL_PTR pSpectralCoefficient; /* Spectral coefficients of each window */
  SHORT specScale[8];                /* Scale shift values of each spectrum window */
  CIcsInfo icsInfo;
  INT granuleLength; /* Size of smallest spectrum piece */
  UCHAR ElementInstanceTag;

  AACDEC_RENDER_MODE renderMode; /* Output signal rendering mode */

  CAacDecoderDynamicData* pDynData; /* Data required for one element and discarded after decoding */
  CAacDecoderCommonData* pComData;  /* Data required for one channel at a time during decode */
  CAacDecoderCommonStaticData*
      pComStaticData; /* Persistent data required for one channel at a time during decode */

  int currAliasingSymmetry;         /* required for MPEG-H MCT */
  UCHAR transform_splitting_active; /* flag signalling transform splitting */

  IGF_PRIVATE_DATA IGFdata;

  /* Frequency Domain Prediction (FDP) tool */
  int fdp_data_present;
  int fdp_spacing_index;

} CAacDecoderChannelInfo;

/* channelinfo.cpp */

AAC_DECODER_ERROR getSamplingRateInfo(SamplingRateInfo* t, UINT samplesPerFrame,
                                      UINT samplingRateIndex, UINT samplingRate);

/**
 * \brief Read max SFB from bit stream and assign TotalSfBands according
 *        to the window sequence and sample rate.
 * \param hBs bit stream handle as data source
 * \param pIcsInfo IcsInfo structure to read the window sequence and store MaxSfBands and
 * TotalSfBands
 * \param pSamplingRateInfo read only
 */
AAC_DECODER_ERROR IcsReadMaxSfb(HANDLE_FDK_BITSTREAM hBs, CIcsInfo* pIcsInfo,
                                const SamplingRateInfo* pSamplingRateInfo);

AAC_DECODER_ERROR IcsRead(HANDLE_FDK_BITSTREAM bs, CIcsInfo* pIcsInfo,
                          const SamplingRateInfo* SamplingRateInfoTable, const UINT flags);

/* stereo.cpp, only called from this file */

/*!
  \brief Applies MS stereo.

  The function applies MS stereo.

  \param pAacDecoderChannelInfo aac channel info.
  \param pScaleFactorBandOffsets pointer to scalefactor band offsets.
  \param pWindowGroupLength pointer to window group length array.
  \param windowGroups number of window groups.
  \param scaleFactorBandsTransmittedL number of transmitted scalefactor bands in left channel.
  \param scaleFactorBandsTransmittedR number of transmitted scalefactor bands in right channel.
                                      May differ from scaleFactorBandsTransmittedL only for USAC.
  \return  none
*/
void CJointStereo_ApplyMS(CAacDecoderChannelInfo* pAacDecoderChannelInfo[2],
                          CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo[2],
                          FIXP_DBL* spectrumL, FIXP_DBL* spectrumR, SHORT* SFBleftScale,
                          SHORT* SFBrightScale, SHORT* specScaleL, SHORT* specScaleR,
                          const SHORT* pScaleFactorBandOffsets, const UCHAR* pWindowGroupLength,
                          const int windowGroups, const int max_sfb_ste_outside,
                          const int scaleFactorBandsTransmittedL,
                          const int scaleFactorBandsTransmittedR, FIXP_DBL* store_dmx_re_prev,
                          SHORT* store_dmx_re_prev_e, const int mainband_flag);

void CJointStereo_ApplyMSIGFcore(CAacDecoderChannelInfo* pAacDecoderChannelInfo[2],
                                 CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo[2],
                                 const short* pScaleFactorBandOffsets,
                                 const UCHAR* pWindowGroupLength, const int windowGroups,
                                 const int max_sfb_ste, const int scaleFactorBandsTransmittedL,
                                 const int scaleFactorBandsTransmittedR);

void CJointStereo_ApplyMS_IGF(CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo[2],
                              CAacDecoderChannelInfo* pAacDecoderChannelInfo[2], FIXP_DBL* pSpecL,
                              FIXP_DBL* pSpecR, const short* pScaleFactorBandOffsets,
                              const UCHAR* pWindowGroupLength, const int windowGroups);

void IGF_StereoFilling_GetPreviousDmx(CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo[2],
                                      CAacDecoderChannelInfo* pAacDecoderChannelInfo[2],
                                      FIXP_DBL* dmx_prev, SHORT* dmx_prev_win_exp,
                                      const SHORT* pScaleFactorBandOffsets,
                                      const UCHAR* pWindowGroupLength, const int windowGroups,
                                      const INT max_noise_sfb, const UCHAR* band_is_noise);

/****************** inline functions ******************/

inline UCHAR IsValid(const CIcsInfo* pIcsInfo) {
  return pIcsInfo->Valid;
}

inline UCHAR IsLongBlock(const CIcsInfo* pIcsInfo) {
  return (pIcsInfo->WindowSequence != BLOCK_SHORT);
}

inline UCHAR GetWindowShape(const CIcsInfo* pIcsInfo) {
  return pIcsInfo->WindowShape;
}

inline BLOCK_TYPE GetWindowSequence(const CIcsInfo* pIcsInfo) {
  return pIcsInfo->WindowSequence;
}

inline const SHORT* GetScaleFactorBandOffsets(const CIcsInfo* pIcsInfo,
                                              const SamplingRateInfo* samplingRateInfo) {
  if (IsLongBlock(pIcsInfo)) {
    return samplingRateInfo->ScaleFactorBands_Long;
  } else {
    return samplingRateInfo->ScaleFactorBands_Short;
  }
}

inline UCHAR GetNumberOfScaleFactorBands(const CIcsInfo* pIcsInfo,
                                         const SamplingRateInfo* samplingRateInfo) {
  if (IsLongBlock(pIcsInfo)) {
    return samplingRateInfo->NumberOfScaleFactorBands_Long;
  } else {
    return samplingRateInfo->NumberOfScaleFactorBands_Short;
  }
}

inline int GetWindowsPerFrame(const CIcsInfo* pIcsInfo) {
  return (pIcsInfo->WindowSequence == BLOCK_SHORT) ? 8 : 1;
}

inline UCHAR GetWindowGroups(const CIcsInfo* pIcsInfo) {
  return pIcsInfo->WindowGroups;
}

inline UCHAR GetWindowGroupLength(const CIcsInfo* pIcsInfo, const INT index) {
  return pIcsInfo->WindowGroupLength[index];
}

inline const UCHAR* GetWindowGroupLengthTable(const CIcsInfo* pIcsInfo) {
  return pIcsInfo->WindowGroupLength;
}

inline UCHAR GetScaleFactorBandsTransmitted(const CIcsInfo* pIcsInfo) {
  return pIcsInfo->MaxSfBands;
}

inline UCHAR GetScaleMaxFactorBandsTransmitted(const CIcsInfo* pIcsInfo0,
                                               const CIcsInfo* pIcsInfo1) {
  return fMax(pIcsInfo0->MaxSfBands, pIcsInfo1->MaxSfBands);
}

inline UCHAR GetScaleFactorBandsTotal(const CIcsInfo* pIcsInfo) {
  return pIcsInfo->TotalSfBands;
}

/* Note: This function applies to AAC-LC only ! */
inline UCHAR GetMaximumTnsBands(const CIcsInfo* pIcsInfo, const int samplingRateIndex) {
  return tns_max_bands_tbl[samplingRateIndex][!IsLongBlock(pIcsInfo)];
}

#endif /* #ifndef CHANNELINFO_H */
