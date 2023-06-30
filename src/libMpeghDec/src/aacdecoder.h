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

#ifndef AACDECODER_H
#define AACDECODER_H

#include "common_fix.h"

#include "FDK_bitstream.h"

#include "channel.h"

#include "tpdec_lib.h"
#include "FDK_audio.h"

#include "block.h"

#include "genericStds.h"

#include "mct.h"

#include "gVBAPRenderer.h"

#include "FDK_drcDecLib.h"

#include "FDK_formatConverter.h"

#include "limiter.h"

#include "FDK_igfDec.h"

#include "FDK_delay.h"

#include "TD_upsampler.h"

#include "uiManager.h"

#define TIME_DATA_FLUSH_SIZE (128)
#define TIME_DATA_FLUSH_SIZE_SF (7)
#define AACDEC_MAX_NUM_PREROLL_AU_MPEGH (1)
#define AACDEC_MAX_NUM_PREROLL_AU AACDEC_MAX_NUM_PREROLL_AU_MPEGH

#define TDL_MPEGH3DA_DEFAULT_ATTACK (5) /* default attack time in ms for peak limiter */
#define TDL_MPEGH3DA_DEFAULT_THRESHOLD \
  FL2FXCONST_DBL(0.89125094f) /* default threshold (-1 dBFS) for peak limiter */

typedef struct AAC_DECODER_INSTANCE* HANDLE_AACDECODER;

enum { L = 0, R = 1 };

typedef struct {
  unsigned char* buffer;
  int bufferSize;
  int offset[8];
  int nrElements;
} CAncData;

enum {
  AACDEC_FLUSH_OFF = 0,
  AACDEC_MPEGH_CFG_CHANGE_ATSC_FLUSH_ON = 1,
  AACDEC_MPEGH_DASH_IPF_ATSC_FLUSH_ON = 2,
  AACDEC_USAC_DASH_IPF_FLUSH_ON = 3
};

enum {
  AACDEC_BUILD_UP_OFF = 0,
  AACDEC_MPEGH_BUILD_UP_ON = 1,
  AACDEC_MPEGH_BUILD_UP_ON_IN_BAND = 2,
  AACDEC_USAC_BUILD_UP_ON = 3,
  AACDEC_MPEGH_BUILD_UP_IDLE = 4,
  AACDEC_MPEGH_BUILD_UP_IDLE_IN_BAND = 5
};

#define AACDEC_CROSSFADE_BITMASK_OFF \
  ((UCHAR)0) /*!< No cross-fade between frames shall be applied at next config change. */
#define AACDEC_CROSSFADE_BITMASK_UI                                                      \
  ((UCHAR)1 << 0) /*!< Cross-fade is needed to switch channel at next config change when \
                       applying mpegh3daElementInteraction from USERINTERACTION (UI) packet */
#define AACDEC_CROSSFADE_BITMASK_PREROLL \
  ((UCHAR)1 << 1) /*!< applyCrossfade is signaled in AudioPreRoll */
#define AACDEC_CROSSFADE_BITMASK_FORCE \
  ((UCHAR)1                            \
   << 2) /*!< Cross-fade at config change is demanded by pCtrlCFGChangeStruct->forceCrossfade */

typedef struct {
  /* Usac Extension Elements */
  USAC_EXT_ELEMENT_TYPE usacExtElementType[(3)];
  UINT usacExtElementDefaultLength[(3)];
  UCHAR usacExtElementPayloadFrag[(3)];
} CUsacCoreExtensions;

typedef struct {
  EarconConfig earconConfig;
  EarconInfo earconInfo;
  UCHAR First_Frame;
  INT target_loudness_old;
  UINT m_bsPcmLoudnessValue_old;
  UINT m_bsPcmAttenuationGain_old;
  FIXP_DBL EarconGain;
  INT EarconShift;
  FIXP_DBL AttGain_store_high_precision; /* Exact attenuation value used if increment is 0. */
  INT AttGain_exp_store_high_precision;
  FIXP_DBL AttGain_intermediate; /* current processing attenuation  */
  INT AttGainShift_intermediate;
  FIXP_DBL AttGain_increment_intermediate;
  FIXP_DBL AttGain_old; /* start attenuation of current frame which ramps to AttGain_new */
  INT AttGainShift_old;
  FIXP_DBL AttGain_increment_old;
  FIXP_DBL AttGain_new; /* end attenuation of current frame */
  INT AttGainShift_new;
  FIXP_SGL EarconData[EARCON_BUFFER_SIZE];
  INT AccumulatedFrameSize; /* Amount of buffered earcon samples in EarconData. Stereo samples
                               account as 2. */
  INT BaseframeSize;
  INT StartDelay;
  INT TruncationPresent;
  INT numPcmSignalsInFrame; /* Current amount of earcon signals */
  INT numPcmSignals_old;    /* Past amount of earcon signals */
  INT numSignalsMixed;      /* Amount of decoder output channels that receive earcon mix */
  INT NumberOfRestChannels; /* Amount of decoder output channels that are unrelated to earcons. */
  INT CurrentFrameHasEarcon;
  INT LastFrameHadEarcon;
  INT speakerPosIndices[EARCON_MAX_NUM_SIGNALS];
  INT speakerPosIndices_Rest[(28)];
  INT speakerGains[EARCON_MAX_NUM_SIGNALS];
} EarconDecoder;

typedef EarconDecoder* HANDLE_EARCONDECODER;

// Add PCM earcon to pTimeData2.
TRANSPORTDEC_ERROR PcmDataPayload(EarconDecoder* earconDecoder, FIXP_DBL* TimeData,
                                  UINT BaseframeSize, SCHAR drcStatus_targetLoudness,
                                  SCHAR defaultTargetLoudness, INT targetLayout,
                                  SHORT truncateFrameSize);

/* AAC decoder (opaque toward userland) struct declaration */
struct AAC_DECODER_INSTANCE {
  INT aacChannels;                     /*!< Amount of AAC decoder channels allocated.        */
  INT ascChannels[TPDEC_MAX_TRACKS];   /*!< Amount of AAC decoder channels signalled in ASC. */
  INT allocChannels[TPDEC_MAX_TRACKS]; /*!< Amount of AAC decoder channels allocated per each
                                          track/substream. */
  INT blockNumber;                     /*!< frame counter                                    */

  INT nrOfLayers;

  INT outputInterleaved; /*!< PCM output format (interleaved/none interleaved). */

  INT aacOutDataHeadroom; /*!< Headroom of the output time signal to prevent clipping */

  HANDLE_TRANSPORTDEC hInput; /*!< Transport layer handle. */

  SamplingRateInfo samplingRateInfo[TPDEC_MAX_TRACKS]; /*!< Sampling Rate information table */

  UCHAR frameOK; /*!< Will be unset if a consistency check, e.g. CRC etc. fails */

  UINT flags[TPDEC_MAX_TRACKS]; /*!< Flags for internal decoder use. DO NOT USE
                                   self::streaminfo::flags ! */
  UINT elFlags[(3 * ((28) * 2) + (((28) * 2)) / 2 + 4 * (1) +
                1)]; /*!< Flags for internal decoder use (element specific). DO NOT USE
                        self::streaminfo::flags ! */

  MP4_ELEMENT_ID elements[(3 * ((28) * 2) + (((28) * 2)) / 2 + 4 * (1) +
                           1)]; /*!< Table where the element Id's are listed          */
  UCHAR chMapping[((28) *
                   2)]; /*!< Table of MPEG canonical order to bitstream channel order mapping. */

  MP4_ELEMENT_ID channel_elements[(3 * ((28) * 2) + (((28) * 2)) / 2 + 4 * (1) +
                                   1)]; /* Channel elements in bit stream order. */

  AUDIO_CHANNEL_TYPE channelType[(
      28)]; /*!< Audio channel type of each output audio channel (from 0 upto numChannels). */
  UCHAR channelIndices[(
      28)]; /*!< Audio channel index for each output audio channel (from 0 upto numChannels). */
  /* See ISO/IEC 13818-7:2005(E), 8.5.3.2 Explicit channel mapping using a program_config_element()
   */

  CProgramConfig pce;
  CStreamInfo streamInfo; /*!< Pointer to StreamInfo data (read from the bitstream) */
  CAacDecoderChannelInfo* pAacDecoderChannelInfo[(28)]; /*!< Temporal channel memory */
  CAacDecoderStaticChannelInfo*
      pAacDecoderStaticChannelInfo[(28)]; /*!< Persistent channel memory */

  FIXP_DBL* workBufferCore2;
  PCM_DEC* pTimeData2;
  INT timeData2Size;

  CpePersistentData* cpeStaticData[(3 * ((28) * 2) + (((28) * 2)) / 2 + 4 * (1) +
                                    1)]; /*!< Pointer to persistent data shared by both channels of
                            a CPE. This structure is allocated once for each CPE. */

  CConcealParams concealCommonData;
  CConcealmentMethod concealMethodUser;

  CUsacCoreExtensions
      usacCoreExt; /*!< Data and handles to extend USAC FD/LPD core decoder (SBR, MPS, ...) */
  INT numUsacElements[TPDEC_MAX_TRACKS];
  UCHAR usacStereoConfigIndex[(3 * ((28) * 2) + (((28) * 2)) / 2 + 4 * (1) + 1)];
  const CSUsacConfig* pUsacConfig[TPDEC_MAX_TRACKS];
  INT nbDiv; /*!< number of frame divisions in LPD-domain */

  INT aacChannelsPrev; /*!< The amount of AAC core channels of the last successful decode call. */
  AUDIO_CHANNEL_TYPE channelTypePrev[(
      28)]; /*!< Array holding the channelType values of the last successful decode call.    */
  UCHAR channelIndicesPrev[(
      28)]; /*!< Array holding the channelIndices values of the last successful decode call. */

  UCHAR downscaleFactor; /*!< Variable to store a supported ELD downscale factor of 1, 2, 3 or 4 */

  CAncData ancData; /*!< structure to handle ancillary data         */

  TDLimiterPtr hLimiter;   /*!< Handle of time domain limiter.             */
  UCHAR limiterEnableUser; /*!< The limiter configuration requested by the library user */
  UCHAR limiterEnableCurr; /*!< The current limiter configuration.         */

  INT discardSamplesAtStartCnt; /*!< Counter for discarding samples of delay at start */

  HANDLE_GVBAPRENDERER
  hgVBAPRenderer[TP_MPEGH_MAX_SIGNAL_GROUPS];     /*!< handle to gVBAP data structure           */
  UCHAR numObjSignalGroups[TPDEC_MAX_TRACKS + 1]; /* +1 is just to avoid a useless gcc warning  */

  HANDLE_DRC_DECODER hUniDrcDecoder;
  UCHAR multibandDrcPresent;
  UCHAR numTimeSlots;
  USHORT stftFrameSize;
  HANDLE_STFT_FILTERBANK stftFilterbankAnalysis[((28) * 2)];
  HANDLE_STFT_FILTERBANK stftFilterbankSynthesis[((28) * 2)];
  SCHAR STFT_headroom_prescaling[TP_MPEGH_MAX_SIGNAL_GROUPS];
  INT loudnessInfoSetPosition[3];
  SCHAR defaultDrcSetEffect;
  SCHAR defaultTargetLoudness;

  INT targetLayout; /*!< Requested Target layout index based on table 95 of ISO/IEC DIS 23008-3.
                       Value 0 means no rendering process. See table 95 for indexes from 1 to 19. */
  INT targetLayout_config; /*!< Applied Target layout index which can be either equal targetLayout
                              or referenceLayout (if targetLayout is 0). */
  IIS_FORMATCONVERTER_HANDLE pFormatConverter[TPDEC_MAX_TRACKS]; /*!< Format converter instances. */
  FIXP_SGL downmixMatrix[TP_MPEGH_MAX_SIGNAL_GROUPS][FDK_FORMAT_CONVERTER_MAX_INPUT_CHANNELS *
                                                     FDK_FORMAT_CONVERTER_MAX_OUTPUT_CHANNELS];
  INT downmixId;
  eqConfigStruct eqConfig[TP_MPEGH_MAX_SIGNAL_GROUPS];

  CMctPtr pMCTdec[TP_MPEGH_MAX_SIGNAL_GROUPS];

  UCHAR flushStatus;     /*!< Indicates flush status: on|off */
  SCHAR flushCnt;        /*!< Flush frame counter */
  UCHAR buildUpStatus;   /*!< Indicates build up status: on|off */
  SCHAR buildUpCnt;      /*!< Build up frame counter */
  UCHAR hasAudioPreRoll; /*!< Indicates preRoll status: on|off */
  UINT prerollAULength[AACDEC_MAX_NUM_PREROLL_AU +
                       1]; /*!< Relative offset of the prerollAU end position to the AU start
                              position in the bitstream */
  INT accessUnit;          /*!< Number of the actual processed preroll accessUnit */
  UCHAR
  applyCrossfade; /*!< If any bit is set, cross-fade for seamless stream switching is applied */

  SHORT truncateStartOffset, truncateStopOffset; /*!< Truncation point offsets in samples relative
                                                    to start of current frame */
  SHORT truncateFrameSize;
  SHORT truncateSampleCount;
  UCHAR truncateFromEndFlag;
  PCM_DEC crossfadeMem[128 * (24)]; /*!< Memory for saving samples for MPEG-H crossfade */

  HANDLE_UI_MANAGER hUiManager;

  USER_INTERACTIVITY_STATUS uiStatus;     /* Currently active UI status */
  USER_INTERACTIVITY_STATUS uiStatusNext; /* Next scheduled UI status. */
  UI_DRC_LOUDNESS_STATUS drcStatus;
  SCHAR uiManagerEnabled; /* Flag indicating if the embedded UI manager is enabled. If zero,
                             then UI events are managed through MHAS packets, if greater than zero
                             UI events are managed through the XML message interface. If less than
                             zero, the integrated UI manager is selected automatically according to
                             MHAS UI input. */
  UCHAR uiManagerActive;  /* Flag indicating if the integrated UI manager is currently active. */
  UCHAR uiStatusValid, drcStatusValid; /* Flags indicating if UI or DRC loudness UI Data for
                                          uiManagerEnabled=0 case are valid. */
  UCHAR uiSignalChanged;    /* Flag indicating that the next scheduled UI status uiStatusNext
                               has a different signal setup and needs to be delayed until the
                               next RAP so that it can be applied. */
  UCHAR useElementSkipping; /* Flag indicating if skipping of inactive elements is used */
  UCHAR signalSkipped[TP_MPEGH_MAX_SIGNAL_GROUPS];

  FDK_SignalDelay
      mpegH_rendered_delay; /*!< MPEG-H delay compensation for rendering chain (constant delay). */
  PCM_DEC delayBuffer[(28)][256];

  FIXP_DBL mpegH_sampleRateConverter_filterStates[(
      24)][TD_STATES_MEM_SIZE]; /*!< MPEG-H sample rate converter for upsampling to output sample
                                   rate */
  EarconDecoder earconDecoder;
};

#define AAC_DEBUG_EXTHLP \
  "\
--- AAC-Core ---\n\
    0x00010000 Header data\n\
    0x00020000 CRC data\n\
    0x00040000 Channel info\n\
    0x00080000 Section data\n\
    0x00100000 Scalefactor data\n\
    0x00200000 Pulse data\n\
    0x00400000 Tns data\n\
    0x00800000 Quantized spectrum\n\
    0x01000000 Requantized spectrum\n\
    0x02000000 Time output\n\
    0x04000000 Fatal errors\n\
    0x08000000 Buffer fullness\n\
    0x10000000 Average bitrate\n\
    0x20000000 Synchronization\n\
    0x40000000 Concealment\n\
    0x7FFF0000 all AAC-Core-Info\n\
"
/**
 * \brief Signal a bit stream interruption to the decoder
 * \param self decoder handle
 */
void CAacDecoder_SignalInterruption(HANDLE_AACDECODER self);

/*!
  \brief Initialize ancillary buffer

  \ancData Pointer to ancillary data structure
  \buffer Pointer to (external) anc data buffer
  \size Size of the buffer pointed on by buffer

  \return  Error code
*/
AAC_DECODER_ERROR CAacDecoder_AncDataInit(CAncData* ancData, unsigned char* buffer, int size);

/*!
  \brief Get one ancillary data element

  \ancData Pointer to ancillary data structure
  \index Index of the anc data element to get
  \ptr Pointer to a buffer receiving a pointer to the requested anc data element
  \size Pointer to a buffer receiving the length of the requested anc data element

  \return  Error code
*/
AAC_DECODER_ERROR CAacDecoder_AncDataGet(CAncData* ancData, int index, unsigned char** ptr,
                                         int* size);

/* initialization of aac decoder */
LINKSPEC_H HANDLE_AACDECODER CAacDecoder_Open(TRANSPORT_TYPE bsFormat);

/* Initialization of channel elements */
LINKSPEC_H AAC_DECODER_ERROR CAacDecoder_Init(HANDLE_AACDECODER self,
                                              const CSAudioSpecificConfig* asc, UCHAR configMode,
                                              UCHAR* configChanged);
/*!
  \brief Decodes one aac frame

  The function decodes one aac frame. The decoding of coupling channel
  elements are not supported. The transport layer might signal, that the
  data of the current frame is invalid, e.g. as a result of a packet
  loss in streaming mode.
  The bitstream position of transportDec_GetBitstream(self->hInput) must
  be exactly the end of the access unit, including all byte alignment bits.
  For this purpose, the variable auStartAnchor is used.

  \return  error status
*/
LINKSPEC_H AAC_DECODER_ERROR CAacDecoder_DecodeFrame(HANDLE_AACDECODER self, const UINT flags,
                                                     PCM_DEC* pTimeData, const INT timeDataSize,
                                                     const int timeDataChannelOffset);

/* Free config dependent AAC memory */
LINKSPEC_H AAC_DECODER_ERROR CAacDecoder_FreeMem(HANDLE_AACDECODER self, const int subStreamIndex);

/* Prepare crossfade for USAC DASH IPF config change */
LINKSPEC_H AAC_DECODER_ERROR CAacDecoder_PrepareCrossFade(const PCM_DEC* pTimeData,
                                                          PCM_DEC** pTimeDataFlush,
                                                          const INT numChannels,
                                                          const INT frameSize);

/* Apply crossfade for USAC DASH IPF config change */
LINKSPEC_H AAC_DECODER_ERROR CAacDecoder_ApplyCrossFade(PCM_DEC* pTimeData,
                                                        PCM_DEC** pTimeDataFlush,
                                                        const INT numChannels, const INT frameSize);

/* Set flush and build up mode */
LINKSPEC_H AAC_DECODER_ERROR CAacDecoder_CtrlCFGChange(HANDLE_AACDECODER self, UCHAR flushStatus,
                                                       SCHAR flushCnt, UCHAR buildUpStatus,
                                                       SCHAR buildUpCnt);

/* Parse preRoll Extension Payload */
LINKSPEC_H AAC_DECODER_ERROR CAacDecoder_PreRollExtensionPayloadParse(HANDLE_AACDECODER self,
                                                                      UINT* numPrerollAU,
                                                                      UINT* prerollAUOffset);

/* Destroy aac decoder */
LINKSPEC_H void CAacDecoder_Close(HANDLE_AACDECODER self);

/* get streaminfo handle from decoder */
LINKSPEC_H CStreamInfo* CAacDecoder_GetStreamInfo(HANDLE_AACDECODER self);

void EarconDecoder_Init(HANDLE_EARCONDECODER pEarconDecoderH);

#endif /* #ifndef AACDECODER_H */
