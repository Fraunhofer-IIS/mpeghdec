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

/*!
  \page default General Overview of the AAC Decoder Implementation

  The main entry point to decode a AAC frame is CAacDecoder_DecodeFrame(). It handles the different
  transport multiplexes and bitstream formats supported by this implementation. It extracts the
  AAC_raw_data_blocks from these bitstreams to further process then in the actual decoding stages.

  Note: Click on a function of file in the above image to see details about the function. Also note,
  that this is just an overview of the most important functions and not a complete call graph.

  <h2>1 Bitstream deformatter</h2>
  The basic bit stream parser function CChannelElement_Read() is called. It uses other subcalls in
  order to parse and unpack the bitstreams. Note, that this includes huffmann decoding of the coded
  spectral data. This operation can be computational significant specifically at higher bitrates.
  Optimization is likely in CBlock_ReadSpectralData().

  The bitstream deformatter also includes many bitfield operations. Profiling on the target will
  determine required optimizations.

  <h2>2 Actual decoding to retain the time domain output</h2>
  The basic bitstream deformatter function CChannelElement_Decode() for CPE elements and SCE
  elements are called. Except for the stereo processing (2.1) which is only used for CPE elements,
  the function calls for CPE or SCE are similar, except that CPE always processes to independent
  channels while SCE only processes one channel.

  Often there is the distinction between long blocks and short blocks. However, computational
  expensive functions that ususally require optimization are being shared by these two groups,

  <h3>2.1 Stereo processing for CPE elements</h3>
  CChannelPairElement_Decode() first calles the joint stereo  tools in stereo.cpp when required.

  <h3>2.2 Scaling of spectral data</h3>
  CBlock_ScaleSpectralData().

  <h3>2.3 Apply additional coding tools</h3>
  ApplyTools() calles the PNS tools in case of MPEG-4 bitstreams, and TNS filtering CTns_Apply() for
  MPEG-2 and MPEG-4 bitstreams. The function TnsFilterIIR() which is called by CTns_Apply() (2.3.1)
  might require some optimization.

  <h2>3 Frequency-To-Time conversion</h3>
  The filterbank is called using CBlock_FrequencyToTime() using the MDCT module from the FDK Tools

*/

#include "aacdecoder.h"

#include "aac_rom.h"
#include "aac_ram.h"
#include "channel.h"
#include "FDK_audio.h"

#include "gVBAPRenderer.h"
#include "FDK_cicp2geometry.h"

#include "ac_arith_coder.h"

#include "tpdec_lib.h"

#include "conceal.h"

#include "FDK_igfDec.h"

#include "ltp_post.h"

#define PS_IS_EXPLICITLY_DISABLED(aot, flags) (0)

#include "mct.h"

#include "ui.h"

#include "FDK_formatConverter_data.h"

void CAacDecoder_SignalInterruption(HANDLE_AACDECODER self) {
  if (self->flags[0] & (AC_USAC | AC_RSVD50 | AC_MPEGH3DA)) {
    int i;

    for (i = 0; i < fMin(self->aacChannels, (28)); i++) {
      if (self->pAacDecoderStaticChannelInfo[i]) { /* number of active channels can be smaller */
        self->pAacDecoderStaticChannelInfo[i]->hArCo->m_numberLinesPrev = 0;
      }
    }
  }
}

/*!
  \brief Calculates the number of element channels

  \type  channel type
  \usacStereoConfigIndex  usac stereo config index

  \return  element channels
*/
static int CAacDecoder_GetELChannels(MP4_ELEMENT_ID type) {
  int el_channels = 0;

  switch (type) {
    case ID_USAC_CPE:
    case ID_CPE:
      el_channels = 2;
      break;
    case ID_USAC_SCE:
    case ID_USAC_LFE:
    case ID_SCE:
    case ID_LFE:
      el_channels = 1;
      break;
    default:
      el_channels = 0;
      break;
  }

  return el_channels;
}

/*!
  \brief Reset ancillary data struct. Call before parsing a new frame.

  \ancData Pointer to ancillary data structure

  \return  Error code
*/
static AAC_DECODER_ERROR CAacDecoder_AncDataReset(CAncData* ancData) {
  int i;
  for (i = 0; i < 8; i++) {
    ancData->offset[i] = 0;
  }
  ancData->nrElements = 0;

  return AAC_DEC_OK;
}

/*!
  \brief Initialize ancillary buffer

  \ancData Pointer to ancillary data structure
  \buffer Pointer to (external) anc data buffer
  \size Size of the buffer pointed on by buffer in bytes

  \return  Error code
*/
AAC_DECODER_ERROR CAacDecoder_AncDataInit(CAncData* ancData, unsigned char* buffer, int size) {
  if (size >= 0) {
    ancData->buffer = buffer;
    ancData->bufferSize = size;

    CAacDecoder_AncDataReset(ancData);

    return AAC_DEC_OK;
  }

  return AAC_DEC_ANC_DATA_ERROR;
}

/*!
  \brief Get one ancillary data element

  \ancData Pointer to ancillary data structure
  \index Index of the anc data element to get
  \ptr Pointer to a buffer receiving a pointer to the requested anc data element
  \size Pointer to a buffer receiving the length of the requested anc data element in bytes

  \return  Error code
*/
AAC_DECODER_ERROR CAacDecoder_AncDataGet(CAncData* ancData, int index, unsigned char** ptr,
                                         int* size) {
  AAC_DECODER_ERROR error = AAC_DEC_OK;

  if ((ptr == NULL) || (size == NULL)) {
    error = AAC_DEC_INVALID_PARAM;
  } else {
    *ptr = NULL;
    *size = 0;

    if (index >= 0 && index < 8 - 1 && index < ancData->nrElements) {
      *ptr = &ancData->buffer[ancData->offset[index]];
      *size = ancData->offset[index + 1] - ancData->offset[index];
    }
  }

  return error;
}

/*!
  \brief Parse PreRoll Extension Payload

  \self             Handle of AAC decoder
  \numPrerollAU     Number of preRoll AUs
  \prerollAUOffset  Offset to each preRoll AU
  \prerollAULength  Length of each preRoll AU

  \return  Error code
*/
LINKSPEC_CPP AAC_DECODER_ERROR CAacDecoder_PreRollExtensionPayloadParse(HANDLE_AACDECODER self,
                                                                        UINT* numPrerollAU,
                                                                        UINT* prerollAUOffset) {
  FDK_BITSTREAM bs;
  HANDLE_FDK_BITSTREAM hBs;
  AAC_DECODER_ERROR ErrorStatus;

  INT auStartAnchor;
  UINT independencyFlag;
  UINT extPayloadPresentFlag;
  UINT useDefaultLengthFlag;
  UINT configLength = 0;
  UINT preRollPossible = 1;
  UINT i;
  int subStreamIndex = 0;
  UCHAR configChanged = 0;
  UCHAR config[TP_USAC_MAX_CONFIG_LEN] = {0};

  ErrorStatus = AAC_DEC_OK;

  /* Copy BS context */
  bs = *transportDec_GetBitstream(self->hInput, subStreamIndex);
  hBs = &bs;

  auStartAnchor = (INT)FDKgetValidBits(hBs);
  if (auStartAnchor <= 0) {
    ErrorStatus = AAC_DEC_NOT_ENOUGH_BITS;
    goto bail;
  }

  /* Independency flag */
  FDKreadBit(hBs);

  /* Payload present flag of extension ID_EXT_ELE_AUDIOPREROLL must be one */
  extPayloadPresentFlag = FDKreadBit(hBs);
  if (!extPayloadPresentFlag) {
    preRollPossible = 0;
  }

  /* Default length flag of extension ID_EXT_ELE_AUDIOPREROLL must be zero */
  useDefaultLengthFlag = FDKreadBit(hBs);
  if (useDefaultLengthFlag) {
    preRollPossible = 0;
  }

  if (preRollPossible) { /* extPayloadPresentFlag && !useDefaultLengthFlag */
    /* Read overall ext payload length, useDefaultLengthFlag must be zero.  */
    escapedValue(hBs, 8, 16, 0);

    /* Read MPEG-H Config size */
    configLength = escapedValue(hBs, 4, 4, 8);

    /* Avoid decoding pre roll frames if there was no config change and no config is included in the
     * pre roll ext payload. */
    if ((self->flags[0] & AC_MPEGH3DA) && (configLength == 0)) {
      /* self->streamInfo.numChannels is checked additionally because for the first frame
       * buildUpStatus is not set. */
      if (self->buildUpStatus == AACDEC_BUILD_UP_OFF && self->streamInfo.numChannels != 0) {
        preRollPossible = 0;
      }
    }
  }

  /* If pre roll not possible then exit. */
  if (preRollPossible == 0) {
    /* Sanity check: if flushing is switched on, preRollPossible must be 1 */
    if (self->flushStatus != AACDEC_FLUSH_OFF) {
      /* Mismatch of current payload and flushing status */
      self->flushStatus = AACDEC_FLUSH_OFF;
      ErrorStatus = AAC_DEC_PARSE_ERROR;
    }
    goto bail;
  }

  if (self->flags[0] & AC_MPEGH3DA) {
    if (configLength > 0) {
      /* DASH IPF ATSC Config Change: Read new config and compare with current config. Apply
       * reconfiguration if config's are different. */
      for (i = 0; i < configLength; i++) {
        config[i] = FDKreadBits(hBs, 8);
      }
      TRANSPORTDEC_ERROR terr;
      terr = transportDec_InBandConfig(self->hInput, config, configLength, self->buildUpStatus,
                                       &configChanged, 0);
      if (terr != TRANSPORTDEC_OK) {
        ErrorStatus = AAC_DEC_PARSE_ERROR;
        goto bail;
      }
    }
  }

  /* For the first frame buildUpStatus is not set and no flushing is performed but preroll AU's
   * should processed. */
  /* buildUpStatus is checked additionally because if explicit cfg != implicit cfg -> idle state is
   * forced. */
  if ((self->streamInfo.numChannels == 0) && (self->flags[0] & AC_MPEGH3DA) &&
      self->buildUpStatus == AACDEC_BUILD_UP_OFF) {
    self->buildUpStatus = AACDEC_MPEGH_BUILD_UP_ON;
    /* sanity check: if buildUp status on -> flushing must be off */
    if (self->flushStatus != AACDEC_FLUSH_OFF) {
      self->flushStatus = AACDEC_FLUSH_OFF;
      ErrorStatus = AAC_DEC_PARSE_ERROR;
      goto bail;
    }
  }

  if (self->flags[0] & AC_MPEGH3DA) {
    /* We are interested in preroll AUs if an explicit or an implicit config change is signalized in
     * other words if the build up status is set. */
    if ((self->buildUpStatus == AACDEC_MPEGH_BUILD_UP_ON) ||
        (self->buildUpStatus == AACDEC_MPEGH_BUILD_UP_ON_IN_BAND)) {
      UCHAR applyCrossfade = FDKreadBit(hBs);
      if (applyCrossfade) {
        self->applyCrossfade |= AACDEC_CROSSFADE_BITMASK_PREROLL;
      } else {
        self->applyCrossfade &= ~AACDEC_CROSSFADE_BITMASK_PREROLL;
      }
      FDKreadBit(hBs); /* reserved */
      /* Read num preroll AU's */
      *numPrerollAU = escapedValue(hBs, 2, 4, 0);
      /* check limits for MPEG-H LC profile */
      if (*numPrerollAU > AACDEC_MAX_NUM_PREROLL_AU_MPEGH) {
        *numPrerollAU = 0;
        ErrorStatus = AAC_DEC_PARSE_ERROR;
        goto bail;
      }
    }
  }

  /* Implementation is limited to either one substream or one preroll frame but not both greater
   * than 1. */
  FDK_ASSERT(!(*numPrerollAU > 1 && subStreamIndex > 1));

  for (i = 0; i < *numPrerollAU; i++) {
    /* For every AU get length and offset in the bitstream */
    int prerollAULength = escapedValue(hBs, 16, 16, 0);
    if (prerollAULength > 0) {
      prerollAUOffset[i + subStreamIndex] = auStartAnchor - FDKgetValidBits(hBs);
      independencyFlag = FDKreadBit(hBs);
      if (i == 0 && !independencyFlag) {
        *numPrerollAU = 0;
        ErrorStatus = AAC_DEC_PARSE_ERROR;
        goto bail;
      }
      if (subStreamIndex == 0) {
        FDKpushFor(hBs, prerollAULength * 8 - 1);
        self->prerollAULength[i] = (prerollAULength * 8) + prerollAUOffset[i];
      }
    } else {
      *numPrerollAU = 0;
      ErrorStatus = AAC_DEC_PARSE_ERROR; /* Something is wrong */
      goto bail;
    }
  }

bail:

  return ErrorStatus;
}

/*  Stream Configuration and Information.

    This class holds configuration and information data for a stream to be decoded. It
    provides the calling application as well as the decoder with substantial information,
    e.g. profile, sampling rate, number of channels found in the bitstream etc.
*/
static void CStreamInfoInit(CStreamInfo* pStreamInfo) {
  pStreamInfo->aacSampleRate = 0;
  pStreamInfo->profile = -1;
  pStreamInfo->aot = AOT_NONE;

  pStreamInfo->channelConfig = -1;
  pStreamInfo->bitRate = 0;
  pStreamInfo->aacSamplesPerFrame = 0;

  pStreamInfo->extAot = AOT_NONE;
  pStreamInfo->extSamplingRate = 0;

  pStreamInfo->flags = 0;

  pStreamInfo->epConfig = -1; /* default: no ER */

  pStreamInfo->numChannels = 0;
  pStreamInfo->sampleRate = 0;
  pStreamInfo->frameSize = 0;

  pStreamInfo->outputDelay = 0;

  pStreamInfo->pcmChOrder = CH_ORDER_CICP; /* default: CICP */

  /* DRC */
  pStreamInfo->drcProgRefLev = -1;       /* set program reference level to not indicated */
  pStreamInfo->drcPresMode = -1;         /* default: presentation mode not indicated */
  pStreamInfo->pceMatrixMixdownIdx = -1; /* default: matrix mixdown index from PCE not indicated */
  pStreamInfo->pcePseudoSurroundEnable =
      -1; /* default: pseudo surround flag from PCE not indicated */

  pStreamInfo->outputLoudness = -1; /* default: no loudness metadata present */

  pStreamInfo->fBsRestartOk = 0;
}

/*!
  \brief Initialization of AacDecoderChannelInfo

  The function initializes the pointers to AacDecoderChannelInfo for each channel,
  set the start values for window shape and window sequence of overlap&add to zero,
  set the overlap buffer to zero and initializes the pointers to the window coefficients.
  \param bsFormat is the format of the AAC bitstream

  \return  AACDECODER instance
*/
LINKSPEC_CPP HANDLE_AACDECODER
CAacDecoder_Open(TRANSPORT_TYPE bsFormat) /*!< bitstream format (adif,adts,loas,...). */
{
  HANDLE_AACDECODER self;

  self = GetAacDecoder();
  if (self == NULL) {
    goto bail;
  }

  /* Assign channel mapping info arrays (doing so removes dependency of settings header in API
   * header). */
  self->streamInfo.pChannelIndices = self->channelIndices;
  self->streamInfo.pChannelType = self->channelType;
  self->streamInfo.pElements = self->elements;
  self->streamInfo.numElements = 0;
  self->downscaleFactor = 1;

  /* initialize anc data */
  CAacDecoder_AncDataInit(&self->ancData, NULL, 0);

  /* initialize stream info */
  CStreamInfoInit(&self->streamInfo);

  /* initialize progam config */
  CProgramConfig_Init(&self->pce);

  /* initialize error concealment common data */
  CConcealment_InitCommonData(&self->concealCommonData);
  self->concealMethodUser = ConcealMethodNone; /* undefined -> auto mode */

  self->workBufferCore2 = GetWorkBufferCore2();
  if (self->workBufferCore2 == NULL) goto bail;

  /* When MPEG-H is active use dedicated memory for core decoding */
  self->pTimeData2 = GetWorkBufferCore5();
  self->timeData2Size = GetRequiredMemWorkBufferCore5();
  if (self->pTimeData2 == NULL) {
    goto bail;
  }

  return self;

bail:
  CAacDecoder_Close(self);

  return NULL;
}

/* Revert CAacDecoder_InitRender() */
static void CAacDecoder_DeInitRenderer(HANDLE_AACDECODER self, const int subStreamIndex) {
  if (self->pFormatConverter[subStreamIndex] != NULL) {
    IIS_FormatConverter_Close(&(self->pFormatConverter[subStreamIndex]));
  }

  {
    int i, objSigGrpOffset = 0;
    for (i = 0; i < subStreamIndex; i++) {
      objSigGrpOffset += self->numObjSignalGroups[i];
    }
    for (i = 0; i < self->numObjSignalGroups[subStreamIndex]; i++) {
      if (self->hgVBAPRenderer[objSigGrpOffset + i]) {
        gVBAPRenderer_Close(self->hgVBAPRenderer[objSigGrpOffset + i]);
        self->hgVBAPRenderer[objSigGrpOffset + i] = NULL;
      }
    }
    /* Do not clear numObjSignalGroups, required for next substream close. */
    /* self->numObjSignalGroups[subStreamIndex] = 0; */
  }

  if (subStreamIndex == 0) {
    FDK_Delay_Destroy(&self->mpegH_rendered_delay);
  }
}

static AAC_DECODER_ERROR applyDownmixMatrixPerSignalGroup(IIS_FORMATCONVERTER_INTERNAL_HANDLE _p,
                                                          HANDLE_AACDECODER self,
                                                          const CSUsacConfig* pUsacConfig,
                                                          FIXP_DBL* p_buffer) {
  int error = 0;

  if (_p->numTotalInputChannels == 0) {
    return AAC_DEC_OK;
  }

  FIXP_DMX_H* dmx_sorted = _p->fcParams->dmxMtx_sorted;
  INT* eqIndexVec_sorted = _p->fcParams->eqIndexVec_sorted;
  for (INT grp = 0; grp < (INT)pUsacConfig->bsNumSignalGroups; grp++) {
    /* Skip signal groups that are not active. */
    if (!getOnOffFlag(self, pUsacConfig->m_signalGroupType[grp].firstSigIdx)) {
      continue;
    }

    if (pUsacConfig->m_signalGroupType[grp].bUseCustomDownmixMatrix) {
      /* Decode downmix matrix per signal group */
      SpeakerInformation inputConfig[FDK_MPEGHAUDIO_DEC_MAX_OUTPUT_CHANNELS];
      CICP2GEOMETRY_CHANNEL_GEOMETRY outputConfig_geo[FDK_MPEGHAUDIO_DEC_MAX_OUTPUT_CHANNELS];

      INT numOutputCh = 0, numOutLfes = 0;
      SpeakerInformation outputConfig[FDK_MPEGHAUDIO_DEC_MAX_OUTPUT_CHANNELS];

      FDK_BITSTREAM bitbuf_init;
      HANDLE_FDK_BITSTREAM bitbuf = &bitbuf_init;

      const CSSignalGroup* signalGroupType = pUsacConfig->m_signalGroupType;
      /* groupsDownmixMatrixSet was written by aacDecoder_ParseDmxMatrixCallback() into borrowed
       * memory from self->pTimeData2 */
      FDK_DOWNMIX_GROUPS_MATRIX_SET* groupsDownmixMatrixSet =
          (FDK_DOWNMIX_GROUPS_MATRIX_SET*)self->pTimeData2;
      UCHAR dmx_index = groupsDownmixMatrixSet->downmixMatrix[grp];
      FDK_ASSERT(groupsDownmixMatrixSet->downmixMatrixSize[dmx_index] > 0);

      /* use self->workBufferCore2 as scratch memory for temporary data */
      eqConfigStruct* eqConfig = (eqConfigStruct*)p_buffer;
      FIXP_DBL* scratchBuf =
          (FIXP_DBL*)ALIGN_PTR(p_buffer + (sizeof(eqConfigStruct) / sizeof(FIXP_DBL) + 1));

      FDKinitBitStream(bitbuf, groupsDownmixMatrixSet->downmixMatrixMemory[dmx_index],
                       sizeof(groupsDownmixMatrixSet->downmixMatrixMemory[dmx_index]),
                       groupsDownmixMatrixSet->downmixMatrixSize[dmx_index], BS_READER);

      for (INT n = 0; n < (INT)signalGroupType[grp].count; n++) {
        inputConfig[n].azimuth = signalGroupType[grp].speakers[n].Az;
        inputConfig[n].elevation = signalGroupType[grp].speakers[n].El;
        inputConfig[n].isLFE = signalGroupType[grp].speakers[n].Lfe;
      }

      error = cicp2geometry_get_geometry_from_cicp(self->targetLayout, outputConfig_geo,
                                                   &numOutputCh, &numOutLfes);
      if (error != 0) {
        return AAC_DEC_UNSUPPORTED_CHANNELCONFIG;
      }

      for (INT n = 0; n < (numOutputCh + numOutLfes); n++) {
        outputConfig[n].azimuth = outputConfig_geo[n].Az;
        outputConfig[n].elevation = outputConfig_geo[n].El;
        outputConfig[n].isLFE = outputConfig_geo[n].LFE;
      }

      error = DecodeDownmixMatrix(signalGroupType[grp].Layout, signalGroupType[grp].count,
                                  inputConfig, self->targetLayout, (numOutputCh + numOutLfes),
                                  outputConfig, bitbuf, dmx_sorted, eqConfig, scratchBuf);

      if (error != 0) {
        return AAC_DEC_UNSUPPORTED_CHANNELCONFIG;
      }
      formatConverterDmxMatrixExponent(_p);
      error = formatConverterSetEQs(eqIndexVec_sorted, eqConfig->numEQs, eqConfig->eqParams,
                                    eqConfig->eqMap, grp, _p, scratchBuf);
      if (error != 0) {
        return AAC_DEC_UNSUPPORTED_FORMAT;
      }
      _p->amountOfAddedDmxMatricesAndEqualizers += _p->numInputChannels[grp];
    }
    eqIndexVec_sorted += pUsacConfig->m_signalGroupType[grp].count * _p->numOutputChannels;
    dmx_sorted += pUsacConfig->m_signalGroupType[grp].count * _p->numOutputChannels;
  }

  return AAC_DEC_OK;
}

/*!
 \brief Initialization of decoder instance

 The function initializes the decoder.

 \return  error status: 0 for success, <>0 for unsupported configurations
 */
static AAC_DECODER_ERROR CAacDecoder_InitRenderer(HANDLE_AACDECODER self, const CSUsacConfig* pUsc,
                                                  const int samplingFrequency,
                                                  const int samplesPerFrame,
                                                  const int elementOffset, FIXP_DBL* p_buffer) {
  AAC_DECODER_ERROR err = AAC_DEC_OK;
  int streamIndex = pUsc->subStreamIndex;

  if (self->targetLayout == 0) {
    if (streamIndex == 0) {
      self->targetLayout_config = self->pUsacConfig[streamIndex]->referenceLayout;
    }
  } else {
    self->targetLayout_config = self->targetLayout;
  }
  /* Make reference layout available for post processing. */
  self->streamInfo.referenceLayout = self->pUsacConfig[0]->referenceLayout;

  /* Check if the cicp and the number of rendered channels is supported. */
  if (self->targetLayout_config >= 0) {
    int numRenderedChannels = 0;
    if (cicp2geometry_get_numChannels_from_cicp(self->targetLayout_config, &numRenderedChannels)) {
      err = AAC_DEC_UNSUPPORTED_CHANNELCONFIG;
      goto bail;
    }
    if (numRenderedChannels > (24)) {
      err = AAC_DEC_UNSUPPORTED_CHANNELCONFIG;
      goto bail;
    }
  }

  if (self->targetLayout_config != -1) {
    {
      IIS_FORMATCONVERTER_MODE mode = IIS_FORMATCONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT;
      CICP2GEOMETRY_CHANNEL_GEOMETRY outGeo[32];
      CICP2GEOMETRY_CHANNEL_GEOMETRY inGeo[32];
      INT aes = 0;
      INT pas = 0;
      INT chan, lfe;
      CICP2GEOMETRY_ERROR cicpErr;

      mode = IIS_FORMATCONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT;

      /* FC error */
      int fcErr = 0;
      INT noInChannels = 0;
      INT noOutChannels = 0;

      /* m_usacConfig */

      /* Target layout */
      {
        /* cicp */
        INT cicpIndexOut = 0;

        cicpIndexOut = self->targetLayout_config;
        noOutChannels = cicp2geometry_get_numChannels_from_cicp(cicpIndexOut);

        /* Sanity check for the current implementation of the FormatConverter: Output channels */
        if (noOutChannels > FDK_FORMAT_CONVERTER_MAX_OUTPUT_CHANNELS) {
          err = AAC_DEC_UNSUPPORTED_CHANNELCONFIG;
          goto bail;
        }

        FDKmemclear(outGeo, sizeof(outGeo));
        cicpErr = cicp2geometry_get_geometry_from_cicp(cicpIndexOut, outGeo, &chan, &lfe);
        if (cicpErr) {
          err = AAC_DEC_UNSUPPORTED_CHANNELCONFIG;
          goto bail;
        }
      }

      /* create FormatConverter */
      FDK_ASSERT(self->pFormatConverter[streamIndex] == NULL);
      {
        fcErr = IIS_FormatConverter_Create(&(self->pFormatConverter[streamIndex]), mode, outGeo,
                                           noOutChannels, samplingFrequency, samplesPerFrame);
        if (fcErr) {
          err = AAC_DEC_UNSUPPORTED_CHANNELCONFIG;
          goto bail;
        }
      }

      /* Set PAS and AES */
      if (pUsc->passiveDownmixFlag !=
          0) /* This should be implemented after the downmixConfig() function is done. */
      {
        /* passive downmix config */
        aes = 0;
        pas = 0;
      } else {
        /* active downmix config */
        aes = 7;
        pas = pUsc->phaseAlignStrength;
      }

      /* Set immersive flag */
      if (pUsc->immersiveDownmixFlag != 0) {
        IIS_FormatConverter_Config_SetImmersiveDownmixFlag((self->pFormatConverter[streamIndex]),
                                                           1);
      }

      for (INT grp = 0; grp < (INT)self->pUsacConfig[streamIndex]->bsNumSignalGroups; grp++) {
        /* AudioChannel is type 0 */
        if ((self->pUsacConfig[streamIndex]->m_signalGroupType[grp].type == 0)) {
          /* Skip signal groups that are not active. */
          if (!getOnOffFlag(self,
                            self->pUsacConfig[streamIndex]->m_signalGroupType[grp].firstSigIdx)) {
            continue;
          }

          noInChannels = self->pUsacConfig[streamIndex]->m_signalGroupType[grp].count;
          self->pFormatConverter[streamIndex]->numSignalsTotal += noInChannels;

          if (self->pFormatConverter[streamIndex]->numSignalsTotal >
              FORMAT_CONVERTER_MAX_CHANNELS) {
            AUDIO_SCENE_INFO* pASI = UI_Manager_GetAsiPointer(self->hUiManager);
            err = AAC_DEC_UNSUPPORTED_CHANNELCONFIG;
            if (pASI != NULL && pASI->isMainStream[streamIndex] == 0) {
              return AAC_DEC_DECODE_FRAME_ERROR; /* Need to parse ASI first. Return OK to avoid
                                                    skipping possible ASI bit stream data. */
            }
            goto bail;
          }

          /* cicp2geometry */

          for (int i = 0; i < self->pUsacConfig[streamIndex]->m_signalGroupType[grp].count; i++) {
            /* Copy Geometry from speakers parser structure in case of unknown channel. */
            inGeo[i].cicpLoudspeakerIndex = -1;
            inGeo[i].Az = self->pUsacConfig[streamIndex]->m_signalGroupType[grp].speakers[i].Az;
            inGeo[i].El = self->pUsacConfig[streamIndex]->m_signalGroupType[grp].speakers[i].El;
            inGeo[i].LFE = self->pUsacConfig[streamIndex]->m_signalGroupType[grp].speakers[i].Lfe;
            inGeo[i].screenRelative = -1;
            inGeo[i].hasDistance = 0;
            inGeo[i].distance = 0;
            inGeo[i].hasLoudspeakerCalibrationGain = 0;
            inGeo[i].loudspeakerCalibrationGain = 0;
            inGeo[i].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_UNKNOWN;
          }

          /* config input setup */
          fcErr = IIS_FormatConverter_Config_AddInputSetup(
              self->pFormatConverter[streamIndex], inGeo, noInChannels,
              0,  /* Signal offset, only needed for multiple channel groups */
              0); /* Channel group id - needed in MPEG-H context */
          if (fcErr) {
            err = AAC_DEC_UNSUPPORTED_CHANNELCONFIG;
            goto bail;
          }
        }
      }

      /* Sanity check for the current implementation of the FormatConverter: Input channels */
      if (self->pFormatConverter[streamIndex]->numSignalsTotal >
          FDK_FORMAT_CONVERTER_MAX_INPUT_CHANNELS) {
        err = AAC_DEC_UNSUPPORTED_CHANNELCONFIG;
        goto bail;
      }

      IIS_FormatConverter_Config_SetAES(self->pFormatConverter[streamIndex], aes);
      IIS_FormatConverter_Config_SetPAS(self->pFormatConverter[streamIndex], pas);

      INT error;
      /* Open Format converter: This will generate the downmix matrix, EQs ... */
      error = IIS_FormatConverter_Open(self->pFormatConverter[streamIndex], (INT*)p_buffer,
                                       sizeof(INT) * (24) * (1024 * 3));
      if (error) {
        err = AAC_DEC_OUT_OF_MEMORY;
        goto bail;
      }

      /* Applying the decoded downmix matrix and EQs for each signal group */
      error = applyDownmixMatrixPerSignalGroup(
          (IIS_FORMATCONVERTER_INTERNAL_HANDLE)(self->pFormatConverter[streamIndex])->member, self,
          self->pUsacConfig[streamIndex], p_buffer);
      if (error) {
        err = AAC_DEC_OUT_OF_MEMORY;
        goto bail;
      }
    }

    if (streamIndex == 0) {
      INT delayErr = 0;
      INT delay;
      SHORT delayTemp = 0;
      TD_FAC_UPSAMPLE sampleRateConverter_facUpsampling;
      /* delay compensation on rendered signal for different sampling rates delay is (768*Fs/48000 -
       delayUpsampler) or (768*Fs/44100 - delayUpsampler) sampling rates are from table "Allowed
       Sampling Rates and Resampling Ratios" valid for Profile Level <= 4
       */
      delay = 3 * 256 + TD_UPSAMPLER_MAX_DELAY; /* total delay */
      switch (samplingFrequency) {
        case 14700:
        case 16000:
          delay -= 3 * 256;
          sampleRateConverter_facUpsampling = TD_FAC_UPSAMPLE_3_1;
          break;
        case 22050:
        case 24000:
          delay -= 2 * 256;
          sampleRateConverter_facUpsampling = TD_FAC_UPSAMPLE_2_1;
          break;
        case 29400:
        case 32000:
          delay -= (3 * 256) / 2;
          sampleRateConverter_facUpsampling = TD_FAC_UPSAMPLE_3_2;
          break;
        case 44100:
        case 48000:
          delay -= 256;
          sampleRateConverter_facUpsampling = TD_FAC_UPSAMPLE_1_1;
          break;
        default:
          err = AAC_DEC_UNSUPPORTED_FORMAT;
          goto bail;
      }
      for (int ch = 0; ch < (24); ch++) {
        delayTemp = TD_upsampler_init(sampleRateConverter_facUpsampling,
                                      &self->mpegH_sampleRateConverter_filterStates[ch][0]);
      }
      delay -= delayTemp;
      FDK_ASSERT(delay >= 0);
      /* Get number of rendered channels */
      int numRenderedChannels = 0;
      if (cicp2geometry_get_numChannels_from_cicp(self->targetLayout_config,
                                                  &numRenderedChannels)) {
        err = AAC_DEC_UNSUPPORTED_FORMAT;
        goto bail;
      }
      delayErr |=
          FDK_Delay_Create(&self->mpegH_rendered_delay, (USHORT)delay,
                           numRenderedChannels); /* (24) could be reduced to nOutputChannels */
      if (delayErr) {
        err = AAC_DEC_OUT_OF_MEMORY;
        goto bail;
      }
    }

    /* Open Object Renderer instance */
    {
      int numChannels;
      int numLFE;

      CICP2GEOMETRY_CHANNEL_GEOMETRY outGeometryInfo[32] =
          {}; /* [cicpLoudspeakerIndex AZ EL isLFE] */
      /* get geometry of loudspeakers by targetLayout (CICP index see also ISO-IEC_23001-8) */
      /* targetLayout_config is prepared obove in PCM_DMX module */
      {
        CICP2GEOMETRY_ERROR cErr = CICP2GEOMETRY_OK;
        cErr = cicp2geometry_get_geometry_from_cicp(self->targetLayout_config, outGeometryInfo,
                                                    &numChannels, &numLFE);
        if (cErr != CICP2GEOMETRY_OK) {
          err = AAC_DEC_UNSUPPORTED_CHANNELCONFIG;
          goto bail;
        }
      }

      /* check if production metadata config present */
      int prodMetadataPresent = 0;
      for (int _el = 0; _el < (INT)self->pUsacConfig[streamIndex]->m_usacNumElements; _el++) {
        if (self->pUsacConfig[streamIndex]->element[_el].extElement.usacExtElementType ==
            ID_EXT_ELE_PROD_METADATA) {
          prodMetadataPresent = 1;
        }
      }

      /* open opject renderer instances */
      int objGrpCnt = 0, signalsIdx = 0;
      int objGrpCntOffset = 0;

      /* Account signal groups of previous substreams */
      for (int i = 0; i < streamIndex; i++) {
        objGrpCntOffset += self->numObjSignalGroups[i];
        signalsIdx += self->pUsacConfig[i]->m_nUsacChannels;
      }

      for (int _el = 0; _el < (INT)self->pUsacConfig[streamIndex]->m_usacNumElements; _el++) {
        int el = elementOffset + _el;
        if (self->pUsacConfig[streamIndex]->element[_el].extElement.usacExtElementType ==
            ID_EXT_ELE_OBJ_METADATA) {
          FDK_ASSERT(self->hgVBAPRenderer[objGrpCnt + objGrpCntOffset] == NULL);

          /* Skip signal groups that are not active. */
          if (!getOnOffFlag(self, signalsIdx)) {
            continue;
          }

          if (gVBAPRenderer_Open(
                  &(self->hgVBAPRenderer[objGrpCnt + objGrpCntOffset]),
                  self->pUsacConfig[streamIndex]
                      ->element[_el]
                      .extElement.extConfig.oam
                      .numObjectSignals, /* number of objects for current signal group */
                  samplesPerFrame,       /* core frame length */
                  self->pUsacConfig[streamIndex]
                      ->element[_el]
                      .extElement.extConfig.oam
                      .OAMframeLength, /* oam frame length for current signal group */
                  outGeometryInfo, numChannels + numLFE, self->targetLayout_config,
                  self->pUsacConfig[streamIndex]
                      ->element[_el]
                      .extElement.extConfig.oam.hasUniformSpread,
                  prodMetadataPresent ? GVBAP_ENHANCED : GVBAP_LEGACY) != 0) {
            err = AAC_DEC_DECODE_FRAME_ERROR;
            /* save the amount of object signal groups for propper closing of object renderer
             * instances */
            self->numObjSignalGroups[streamIndex] = objGrpCnt + 1;
            goto bail;
          }
          objGrpCnt++;
        }
        if (IS_CHANNEL_ELEMENT(self->elements[el])) {
          signalsIdx += (self->elements[el] == ID_USAC_CPE) ? 2 : 1;
        }
      }
      /* save the amount of object signal groups for later closing of object renderer instances */
      self->numObjSignalGroups[streamIndex] = objGrpCnt;
      if ((self->pUsacConfig[streamIndex]->objNumSignalGroups !=
           self->numObjSignalGroups[streamIndex]) &&
          !self->useElementSkipping) {
        err = AAC_DEC_DECODE_FRAME_ERROR;
        goto bail;
      }
    }
  }

  return err;

bail:
  CAacDecoder_DeInitRenderer(self, streamIndex);

  return err;
}

/* Revert CAacDecoder_Init() */
static void CAacDecoder_DeInit(HANDLE_AACDECODER self, const int subStreamIndex) {
  int ch;
  int aacChannelOffset = 0, aacChannels = (28);
  int numElements = (3 * ((28) * 2) + (((28) * 2)) / 2 + 4 * (1) + 1), elementOffset = 0;

  if (self == NULL) return;

  {
    self->ascChannels[0] = 0;
    self->numUsacElements[0] = 0;
    self->elements[0] = ID_END;
  }

  for (ch = aacChannelOffset; ch < aacChannelOffset + aacChannels; ch++) {
    if (self->pAacDecoderChannelInfo[ch] != NULL) {
      if (self->pAacDecoderChannelInfo[ch]->pDynData != NULL) {
        FDKfree(self->pAacDecoderChannelInfo[ch]->pDynData);
      }
      if (self->pAacDecoderChannelInfo[ch]->pComStaticData != NULL) {
        if (self->pAacDecoderChannelInfo[ch]->pComStaticData->pWorkBufferCore1 != NULL) {
          if (ch == aacChannelOffset) {
            FreeWorkBufferCore1(
                &self->pAacDecoderChannelInfo[ch]->pComStaticData->pWorkBufferCore1);
          }
        }
        if (self->pAacDecoderChannelInfo[ch]->pComStaticData->cplxPredictionData != NULL) {
          FreeCplxPredictionData(
              &self->pAacDecoderChannelInfo[ch]->pComStaticData->cplxPredictionData);
        }
        /* Avoid double free of linked pComStaticData in case of CPE by settings pointer to NULL. */
        if (ch < (28) - 1) {
          if ((self->pAacDecoderChannelInfo[ch + 1] != NULL) &&
              (self->pAacDecoderChannelInfo[ch + 1]->pComStaticData ==
               self->pAacDecoderChannelInfo[ch]->pComStaticData)) {
            self->pAacDecoderChannelInfo[ch + 1]->pComStaticData = NULL;
          }
        }
        FDKfree(self->pAacDecoderChannelInfo[ch]->pComStaticData);
        self->pAacDecoderChannelInfo[ch]->pComStaticData = NULL;
      }
      if (self->pAacDecoderChannelInfo[ch]->pComData != NULL) {
        if (self->pAacDecoderChannelInfo[ch]->pComData->pJointStereoData != NULL) {
          FDKfree(self->pAacDecoderChannelInfo[ch]->pComData->pJointStereoData);
        }
        /* Avoid double free of linked pComData in case of CPE by settings pointer to NULL. */
        if (ch < (28) - 1) {
          if ((self->pAacDecoderChannelInfo[ch + 1] != NULL) &&
              (self->pAacDecoderChannelInfo[ch + 1]->pComData ==
               self->pAacDecoderChannelInfo[ch]->pComData)) {
            self->pAacDecoderChannelInfo[ch + 1]->pComData = NULL;
          }
        }
        if (ch == aacChannelOffset) {
          FreeWorkBufferCore6((FIXP_DBL**)&self->pAacDecoderChannelInfo[ch]->pComData);
        } else {
          FDKafree(self->pAacDecoderChannelInfo[ch]->pComData);
        }
        self->pAacDecoderChannelInfo[ch]->pComData = NULL;
      }
    }
    if (self->pAacDecoderStaticChannelInfo[ch] != NULL) {
      if (self->pAacDecoderStaticChannelInfo[ch]->concealmentInfo.spectralCoefficient != NULL) {
        FreeConcealmentSpecBuffer(
            &self->pAacDecoderStaticChannelInfo[ch]->concealmentInfo.spectralCoefficient);
      }
      if (self->pAacDecoderStaticChannelInfo[ch]->pOverlapBuffer != NULL) {
        FreeOverlapBuffer(&self->pAacDecoderStaticChannelInfo[ch]->pOverlapBuffer);
      }
      if (self->pAacDecoderStaticChannelInfo[ch]->hArCo != NULL) {
        CArco_Destroy(self->pAacDecoderStaticChannelInfo[ch]->hArCo);
      }
      FreeAacDecoderStaticChannelInfo(&self->pAacDecoderStaticChannelInfo[ch]);
    }
    if (self->pAacDecoderChannelInfo[ch] != NULL) {
      FreeAacDecoderChannelInfo(&self->pAacDecoderChannelInfo[ch]);
    }
  }

  {
    int el;
    for (el = elementOffset; el < elementOffset + numElements; el++) {
      if (self->cpeStaticData[el] != NULL) {
        FreeCpePersistentData(&self->cpeStaticData[el]);
      }
    }
  }

  if (self->flags[subStreamIndex] & AC_MPEGH3DA) {
    for (int grp = 0; grp < TP_MPEGH_MAX_SIGNAL_GROUPS; grp++) {
      if (self->pMCTdec[grp]) {
        CMct_Destroy(self->pMCTdec[grp]);
        self->pMCTdec[grp] = NULL;
      }
    }
  }

  if (self->multibandDrcPresent) {
    for (ch = 0; ch < ((28) * 2); ch++) {
      StftFilterbank_Close(&(self->stftFilterbankAnalysis[ch]));
    }
    for (ch = 0; ch < ((28) * 2); ch++) {
      StftFilterbank_Close(&(self->stftFilterbankSynthesis[ch]));
    }
  }

  if (self->flags[subStreamIndex] & AC_MPEGH3DA) {
    CAacDecoder_DeInitRenderer(self, subStreamIndex);
  }

  self->aacChannels = 0;
  self->streamInfo.aacSampleRate = 0;
  self->streamInfo.sampleRate = 0;
  /* This samplerate value is checked for configuration change, not the others above. */
  self->samplingRateInfo[subStreamIndex].samplingRate = 0;
}

/*!
 * \brief CAacDecoder_AcceptFlags Accept flags and element flags
 *
 * \param self          [o]   handle to AACDECODER structure
 * \param asc           [i]   handle to ASC structure
 * \param flags         [i]   flags
 * \param elFlags       [i]   pointer to element flags
 * \param streamIndex   [i]   stream index
 * \param elementOffset [i]   element offset
 *
 * \return void
 */
static void CAacDecoder_AcceptFlags(HANDLE_AACDECODER self, const CSAudioSpecificConfig* asc,
                                    UINT flags, UINT* elFlags, int streamIndex, int elementOffset) {
  if (flags & AC_MPEGH3DA) {
    for (int el = elementOffset; el < elementOffset + (int)asc->m_sc.m_usacConfig.m_usacNumElements;
         el++) {
      self->elFlags[el] = elFlags[el];
    }
  } else {
    FDKmemcpy(self->elFlags, elFlags,
              sizeof(*elFlags) * (3 * ((28) * 2) + (((28) * 2)) / 2 + 4 * (1) + 1));
  }

  self->flags[streamIndex] = flags;
}

/*!
 * \brief CAacDecoder_CtrlCFGChange Set config change parameters.
 *
 * \param self           [i]   handle to AACDECODER structure
 * \param flushStatus    [i]   flush status: on|off
 * \param flushCnt       [i]   flush frame counter
 * \param buildUpStatus  [i]   build up status: on|off
 * \param buildUpCnt     [i]   build up frame counter
 *
 * \return error
 */
LINKSPEC_CPP AAC_DECODER_ERROR CAacDecoder_CtrlCFGChange(HANDLE_AACDECODER self, UCHAR flushStatus,
                                                         SCHAR flushCnt, UCHAR buildUpStatus,
                                                         SCHAR buildUpCnt) {
  AAC_DECODER_ERROR err = AAC_DEC_OK;

  self->flushStatus = flushStatus;
  self->flushCnt = flushCnt;
  self->buildUpStatus = buildUpStatus;
  self->buildUpCnt = buildUpCnt;

  return (err);
}

/*!
 * \brief CAacDecoder_FreeMem Free config dependent AAC memory.
 *
 * \param self       [i]   handle to AACDECODER structure
 *
 * \return error
 */
LINKSPEC_CPP AAC_DECODER_ERROR CAacDecoder_FreeMem(HANDLE_AACDECODER self,
                                                   const int subStreamIndex) {
  AAC_DECODER_ERROR err = AAC_DEC_OK;

  CAacDecoder_DeInit(self, subStreamIndex);

  return (err);
}

/* Destroy aac decoder */
LINKSPEC_CPP void CAacDecoder_Close(HANDLE_AACDECODER self) {
  if (self == NULL) return;

  for (int i = 0; i < TPDEC_MAX_TRACKS; i++) {
    CAacDecoder_DeInit(self, i);
  }

  /* Free WorkBufferCore2 */
  if (self->workBufferCore2 != NULL) {
    FreeWorkBufferCore2(&self->workBufferCore2);
  }
  if (self->pTimeData2 != NULL) {
    FreeWorkBufferCore5(&self->pTimeData2);
  }

  FreeAacDecoder(&self);
}

/*!
  \brief Initialization of decoder instance

  The function initializes the decoder.

  \return  error status: 0 for success, <>0 for unsupported configurations
*/
LINKSPEC_CPP AAC_DECODER_ERROR CAacDecoder_Init(HANDLE_AACDECODER self,
                                                const CSAudioSpecificConfig* asc, UCHAR configMode,
                                                UCHAR* configChanged) {
  AAC_DECODER_ERROR err = AAC_DEC_OK;
  INT ascChannels, ascChanged = 0;
  AACDEC_RENDER_MODE initRenderMode = AACDEC_RENDER_INVALID;
  int elementOffset, aacChannelsOffset, aacChannelsOffsetIdx;
  int streamIndex = 0;
  if (asc->m_aot == AOT_MPEGH3DA) {
    streamIndex = asc->m_sc.m_usacConfig.subStreamIndex;
  }

  UINT flags;
  UINT elFlags[(3 * ((28) * 2) + (((28) * 2)) / 2 + 4 * (1) + 1)];

  if (!self) return AAC_DEC_INVALID_HANDLE;

  UCHAR downscaleFactor = self->downscaleFactor;

  self->aacOutDataHeadroom = (3);

  if (asc->m_aot == AOT_MPEGH3DA) {
    self->aacOutDataHeadroom = (8);
  }

  // set profile and check for supported aot
  // leave profile on default (=-1) for all other supported MPEG-4 aot's except aot=2 (=AAC-LC)
  switch (asc->m_aot) {
    case AOT_MPEGH3DA:
      initRenderMode = AACDEC_RENDER_IMDCT;
      break;
    default:
      return AAC_DEC_UNSUPPORTED_AOT;
  }

  { CProgramConfig_Init(&self->pce); }

  /* set channels */
  switch (asc->m_channelConfiguration) {
    case 0:
      switch (asc->m_aot) {
        case AOT_MPEGH3DA:
          ascChannels = asc->m_sc.m_usacConfig.numAudioChannels +
                        asc->m_sc.m_usacConfig.numAudioObjects +
                        asc->m_sc.m_usacConfig.numHOATransportChannels;
          break;
        default:
          return AAC_DEC_UNSUPPORTED_CHANNELCONFIG;
      }
      break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
      ascChannels = asc->m_channelConfiguration;
      break;
    case 11:
      ascChannels = 7;
      break;
    case 7:
    case 12:
    case 14:
      ascChannels = 8;
      break;
    case 13: /* 22.2 setup */
      ascChannels = 24;
      break;
    default:
      return AAC_DEC_UNSUPPORTED_CHANNELCONFIG;
  }

  if ((asc->m_aot == AOT_USAC) || (asc->m_aot == AOT_MPEGH3DA)) {
    /* Check if Transport Decoder element list will fit into Audio decoder element list. */
    if (asc->m_sc.m_usacConfig.m_usacNumElements >
        (3 * ((28) * 2) + (((28) * 2)) / 2 + 4 * (1) + 1)) {
      goto bail;
    }

    for (int el = 0; el < (INT)asc->m_sc.m_usacConfig.m_usacNumElements; el++) {
    }
  }

  aacChannelsOffset = 0;
  aacChannelsOffsetIdx = 0;
  elementOffset = 0;
  if (asc->m_aot == AOT_MPEGH3DA) {
    updateOnOffFlags(self);
    for (int i = 0; i < streamIndex; i++) {
      if (self->pUsacConfig[i] == NULL) {
        /* Its not possible to allocate sub streams before a main stream has been allocated. */
        return AAC_DEC_UNSUPPORTED_FORMAT;
      }
      for (int j = 0; j < self->pUsacConfig[i]->m_nUsacChannels; j++) {
        if (getOnOffFlag(self, aacChannelsOffsetIdx + j)) {
          aacChannelsOffset++;
        }
      }
      aacChannelsOffsetIdx += self->pUsacConfig[i]->m_nUsacChannels;
    }
    for (int i = 0; i < streamIndex; i++) {
      elementOffset += self->numUsacElements[i];
    }
  }
  if (ascChannels <= 0) {
    return AAC_DEC_UNSUPPORTED_CHANNELCONFIG;
  }
  if ((ascChannels + aacChannelsOffsetIdx) > ((28) * 2)) {
    return AAC_DEC_UNSUPPORTED_CHANNELCONFIG;
  }
  if (configMode & AC_CM_ALLOC_MEM) {
    if (asc->m_aot == AOT_MPEGH3DA) {
      /* Check max signal count for channels and objects. */
      int n_other = 0, n_obj = 0;

      for (int sb = 0; sb < streamIndex - 1; sb++) {
        for (int sg = 0; sg < self->pUsacConfig[sb]->bsNumSignalGroups; sg++) {
          int n_sig;

          n_sig = self->pUsacConfig[sb]->m_signalGroupType[sg].count;
          if (self->pUsacConfig[sb]->m_signalGroupType[sg].type == 1) {
            n_obj += n_sig;
          } else {
            n_other += n_sig;
          }
        }
      }
      for (int sg = 0; sg < asc->m_sc.m_usacConfig.bsNumSignalGroups; sg++) {
        int n_sig;

        n_sig = asc->m_sc.m_usacConfig.m_signalGroupType[sg].count;
        if (asc->m_sc.m_usacConfig.m_signalGroupType[sg].type == 1) {
          n_obj += n_sig;
        } else {
          n_other += n_sig;
        }
      }
      if ((n_other == 0 && n_obj > 24) || ((n_other > 0) && ((n_obj + n_other) > 16))) {
        if (n_obj + n_other > ((28) * 2)) {
          return AAC_DEC_UNSUPPORTED_CHANNELCONFIG;
        }

        self->useElementSkipping = 1;
        updateOnOffFlags(self);
      } else {
        self->useElementSkipping = 0;
        updateOnOffFlags(self);
      }
      transportDec_SetParam(self->hInput, TPDEC_PARAM_USE_ELEM_SKIPPING, self->useElementSkipping);
    }
  }

  /* Set syntax flags */
  flags = 0;
  if (asc->m_aot == AOT_MPEGH3DA) {
    for (int el = elementOffset; el < elementOffset + (int)asc->m_sc.m_usacConfig.m_usacNumElements;
         el++) {
      elFlags[el] = 0;
    }
  } else {
    FDKmemclear(elFlags, sizeof(elFlags));
  }

  if ((asc->m_channelConfiguration > 0) || (asc->m_aot == AOT_MPEGH3DA) ||
      (asc->m_aot == AOT_USAC)) {
    if ((asc->m_aot == AOT_USAC) || (asc->m_aot == AOT_MPEGH3DA)) {
      /* copy pointer to usac config
        (this is preliminary since there's an ongoing discussion about storing the config-part of
        the bitstream rather than the complete decoded configuration) */
      self->pUsacConfig[streamIndex] = &asc->m_sc.m_usacConfig;
      updateOnOffFlags(self);
      if (self->numUsacElements[streamIndex] != (int)asc->m_sc.m_usacConfig.m_usacNumElements) {
        ascChanged = 1;
      }

      if (configMode & AC_CM_ALLOC_MEM) {
        self->numUsacElements[streamIndex] = (int)asc->m_sc.m_usacConfig.m_usacNumElements;
      }

      for (int _el = 0; _el < (int)self->pUsacConfig[streamIndex]->m_usacNumElements; _el++) {
        int el = _el + elementOffset;
        if (self->elements[el] != self->pUsacConfig[streamIndex]->element[_el].usacElementType) {
          ascChanged = 1;
        }
        if (configMode & AC_CM_ALLOC_MEM) {
          self->elements[el] = self->pUsacConfig[streamIndex]->element[_el].usacElementType;
        }

        elFlags[el] |= (asc->m_sc.m_usacConfig.element[_el].m_noiseFilling) ? AC_EL_USAC_NOISE : 0;
        elFlags[el] |= (asc->m_sc.m_usacConfig.element[_el].usacElementType == ID_USAC_LFE)
                           ? AC_EL_USAC_LFE
                           : 0;
        elFlags[el] |=
            (asc->m_sc.m_usacConfig.element[_el].enhancedNoiseFilling) ? AC_EL_ENHANCED_NOISE : 0;
        elFlags[el] |= (asc->m_sc.m_usacConfig.element[_el].igfUseEnf) ? AC_EL_IGF_USE_ENF : 0;
        elFlags[el] |=
            (asc->m_sc.m_usacConfig.element[_el].igfAfterTnsSynth) ? AC_EL_IGF_AFTER_TNS : 0;
        elFlags[el] |=
            (asc->m_sc.m_usacConfig.element[_el].igfIndependentTiling) ? AC_EL_IGF_INDEP_TILING : 0;
        elFlags[el] |= (asc->m_sc.m_usacConfig.element[_el].fullbandLpd) ? AC_EL_FULLBANDLPD : 0;
        elFlags[el] |=
            (asc->m_sc.m_usacConfig.element[_el].lpdStereoIndex) ? AC_EL_LPDSTEREOIDX : 0;
        elFlags[el] |=
            (asc->m_sc.m_usacConfig.element[_el].usacElementType == ID_USAC_LFE) ? AC_EL_LFE : 0;
        if ((asc->m_sc.m_usacConfig.element[_el].usacElementType == ID_USAC_CPE)) {
          elFlags[el] |= AC_EL_USAC_CP_POSSIBLE;
        }
      }

      self->hasAudioPreRoll = 0;
      if (self->pUsacConfig[streamIndex]->m_usacNumElements) {
        self->hasAudioPreRoll =
            asc->m_sc.m_usacConfig.element[0].extElement.usacExtElementHasAudioPreRoll;
      }
      if (configMode & AC_CM_ALLOC_MEM) {
        self->elements[elementOffset + self->pUsacConfig[streamIndex]->m_usacNumElements] = ID_END;
      }
    }

    if (asc->m_aot == AOT_MPEGH3DA) {
      const CSUsacConfig* usc = &(asc->m_sc.m_usacConfig);
      int i = aacChannelsOffsetIdx, j, shift;

      if (streamIndex == 0) {
        for (int ch = 0; ch < ((28) * 2); ch++) {
          self->chMapping[ch] = 255;
        }
      }

      for (int _el = 0; _el < (INT)usc->m_usacNumElements; _el++) {
        const CSUsacElementConfig* elCfg = &(usc->element[_el]);

        switch (elCfg->usacElementType) {
          case ID_USAC_SCE:
          case ID_USAC_LFE:
            /* assign to next free index */
            for (j = 0; j < ((28) * 2); j++) {
              if (self->chMapping[j] == 255) {
                self->chMapping[j] = i;
                i++;
                break;
              }
            }
            break;

          case ID_USAC_CPE:
            /* first CPE channel, assign to next free index + shift */
            for (j = 0; j < ((28) * 2); j++) {
              if (self->chMapping[j] == 255) {
                self->chMapping[j] = i;
                i++;
                break;
              }
            }

            /* second CPE channel, assign to next free index + shift */
            shift = elCfg->shiftIndex1 ? (elCfg->shiftChannel1 + 1) : 0;
            for (j = 0; j < ((28) * 2); j++) {
              if (self->chMapping[j] == 255) {
                while (shift > 0) {
                  j++;
                  if ((j + shift) >= ((28) * 2)) {
                    return AAC_DEC_UNSUPPORTED_CHANNELCONFIG;
                  }
                  if (self->chMapping[j] == 255) shift--;
                }
                self->chMapping[j] = i;
                i++;
                break;
              }
            }

            break;
          default:
            break;
        }
      }
    }
  }

  self->streamInfo.channelConfig = asc->m_channelConfiguration;

  if (self->streamInfo.aot != asc->m_aot) {
    if (configMode & AC_CM_ALLOC_MEM) {
      self->streamInfo.aot = asc->m_aot;
    }
    ascChanged = 1;
  }

  if ((INT)asc->m_samplesPerFrame % downscaleFactor != 0) {
    return AAC_DEC_UNSUPPORTED_SAMPLINGRATE; /* frameSize/dsf must be an integer number */
  }

  self->streamInfo.bitRate = 0;

  /* --------- vcb11 ------------ */
  flags |= (asc->m_vcb11Flag) ? AC_ER_VCB11 : 0;
  if (asc->m_vcb11Flag != 0) {
    return AAC_DEC_UNSUPPORTED_ER_FORMAT;
  }

  /* ---------- rvlc ------------ */
  flags |= (asc->m_rvlcFlag) ? AC_ER_RVLC : 0;
  if (asc->m_rvlcFlag != 0) {
    return AAC_DEC_UNSUPPORTED_ER_FORMAT;
  }

  /* ----------- hcr ------------ */
  flags |= (asc->m_hcrFlag) ? AC_ER_HCR : 0;
  if (asc->m_hcrFlag != 0) {
    return AAC_DEC_UNSUPPORTED_ER_FORMAT;
  }

  flags |= (asc->m_epConfig >= 0) ? AC_ER : 0;

  if (asc->m_aot == AOT_MPEGH3DA) {
    flags |= AC_MPEGH3DA;
  }

  if ((asc->m_epConfig >= 0) && (asc->m_channelConfiguration <= 0)) {
    /* we have to know the number of channels otherwise no decoding is possible */
    return AAC_DEC_UNSUPPORTED_ER_FORMAT;
  }

  self->streamInfo.epConfig = asc->m_epConfig;
  /* self->hInput->asc.m_epConfig = asc->m_epConfig; */

  if (asc->m_epConfig > 1) return AAC_DEC_UNSUPPORTED_ER_FORMAT;

  /* Check if samplerate changed. */
  if ((self->samplingRateInfo[streamIndex].samplingRate != asc->m_samplingFrequency) ||
      (self->streamInfo.aacSamplesPerFrame != (INT)asc->m_samplesPerFrame / downscaleFactor)) {
    AAC_DECODER_ERROR error;

    ascChanged = 1;

    if (configMode & AC_CM_ALLOC_MEM) {
      /* Update samplerate info. */
      error = getSamplingRateInfo(&self->samplingRateInfo[streamIndex], asc->m_samplesPerFrame,
                                  asc->m_samplingFrequencyIndex, asc->m_samplingFrequency);
      if (error != AAC_DEC_OK) {
        return error;
      }
      self->streamInfo.aacSampleRate =
          self->samplingRateInfo[0].samplingRate / self->downscaleFactor;
      self->streamInfo.aacSamplesPerFrame = asc->m_samplesPerFrame / self->downscaleFactor;
      if (self->streamInfo.aacSampleRate <= 0) {
        return AAC_DEC_UNSUPPORTED_SAMPLINGRATE;
      }
    }
  }

  /* Check if amount of channels has changed. */
  if (self->ascChannels[streamIndex] != ascChannels) {
    ascChanged = 1;
  }

  /* detect config change */
  if (configMode & AC_CM_DET_CFG_CHANGE) {
    if (ascChanged != 0) {
      *configChanged = 1;
    }

    CAacDecoder_AcceptFlags(self, asc, flags, elFlags, streamIndex, elementOffset);

    return err;
  }

  if (*configChanged) {
    /* Allocate all memory structures for each channel */
    {
      int ch = aacChannelsOffset;
      for (int _ch = 0; _ch < ascChannels; _ch++) {
        if (asc->m_aot == AOT_MPEGH3DA) {
          int chIdx = aacChannelsOffsetIdx + _ch;

          if (!getOnOffFlag(self, chIdx)) {
            continue;
          }
        }
        if (ch >= (28)) {
          goto bail;
        }
        self->pAacDecoderChannelInfo[ch] = GetAacDecoderChannelInfo(ch);
        /* This is temporary until the DynamicData is split into two or more regions!
           The memory could be reused after completed core decoding. */
        if (self->pAacDecoderChannelInfo[ch] == NULL) {
          goto bail;
        }
        self->pAacDecoderChannelInfo[ch]->pDynData =
            (CAacDecoderDynamicData*)FDKmalloc(sizeof(CAacDecoderDynamicData));
        if (self->pAacDecoderChannelInfo[ch]->pDynData == NULL) {
          goto bail;
        }

        ch++;
      }

      int chIdx = aacChannelsOffsetIdx;
      ch = aacChannelsOffset;
      int _numElements;
      _numElements = (((28)) + ((28)));
      if (flags & (AC_MPEGH3DA | AC_USAC)) {
        _numElements = (int)asc->m_sc.m_usacConfig.m_usacNumElements;
      }
      for (int _el = 0; _el < _numElements; _el++) {
        int el_channels = 0;
        int el = elementOffset + _el;

        switch (self->elements[el]) {
          case ID_SCE:
          case ID_CPE:
          case ID_LFE:
          case ID_USAC_SCE:
          case ID_USAC_CPE:
          case ID_USAC_LFE:

            el_channels = CAacDecoder_GetELChannels(self->elements[el]);

            if (el_channels == 2) {
              /* The signal skip flag of both channels must be the same. */
              if (getOnOffFlag(self, chIdx) != getOnOffFlag(self, chIdx + 1)) {
                goto bail;
              }
            }

            if (getOnOffFlag(self, chIdx)) {
              if (ch >= (28)) {
                goto bail;
              }
              if (self->pAacDecoderChannelInfo[ch] == NULL) {
                goto bail;
              }
              self->pAacDecoderChannelInfo[ch]->pComStaticData =
                  (CAacDecoderCommonStaticData*)FDKcalloc(1, sizeof(CAacDecoderCommonStaticData));
              if (self->pAacDecoderChannelInfo[ch]->pComStaticData == NULL) {
                goto bail;
              }
              if (ch == aacChannelsOffset) {
                self->pAacDecoderChannelInfo[ch]->pComData =
                    (CAacDecoderCommonData*)GetWorkBufferCore6();
                self->pAacDecoderChannelInfo[ch]->pComStaticData->pWorkBufferCore1 =
                    GetWorkBufferCore1();
              } else {
                self->pAacDecoderChannelInfo[ch]->pComData = (CAacDecoderCommonData*)FDKaalloc(
                    sizeof(CAacDecoderCommonData), ALIGNMENT_DEFAULT);
                self->pAacDecoderChannelInfo[ch]->pComStaticData->pWorkBufferCore1 =
                    self->pAacDecoderChannelInfo[aacChannelsOffset]
                        ->pComStaticData->pWorkBufferCore1;
              }
              if ((self->pAacDecoderChannelInfo[ch]->pComData == NULL) ||
                  (self->pAacDecoderChannelInfo[ch]->pComStaticData->pWorkBufferCore1 == NULL)) {
                goto bail;
              }
              if (el_channels == 2) {
                self->pAacDecoderChannelInfo[ch]->pComData->pJointStereoData =
                    (CJointStereoData*)FDKmalloc(sizeof(CJointStereoData));
                if (self->pAacDecoderChannelInfo[ch]->pComData->pJointStereoData == NULL) {
                  goto bail;
                }
              } else { /* memory required only for CPEs */
                FDK_ASSERT(self->pAacDecoderChannelInfo[ch]->pComData->pJointStereoData == NULL);
              }

              self->pAacDecoderChannelInfo[ch]->pSpectralCoefficient =
                  (SPECTRAL_PTR)&self->workBufferCore2[ch * 1024];

              if (el_channels == 2) {
                if (ch >= (28) - 1) {
                  goto bail;
                }
                if (self->pAacDecoderChannelInfo[ch + 1] == NULL) {
                  goto bail;
                }
                self->pAacDecoderChannelInfo[ch + 1]->pComData =
                    self->pAacDecoderChannelInfo[ch]->pComData;
                self->pAacDecoderChannelInfo[ch + 1]->pComStaticData =
                    self->pAacDecoderChannelInfo[ch]->pComStaticData;
                self->pAacDecoderChannelInfo[ch + 1]->pComStaticData->pWorkBufferCore1 =
                    self->pAacDecoderChannelInfo[ch]->pComStaticData->pWorkBufferCore1;
                self->pAacDecoderChannelInfo[ch + 1]->pSpectralCoefficient =
                    (SPECTRAL_PTR)&self->workBufferCore2[(ch + 1) * 1024];
              }

              ch += el_channels;
            }
            chIdx += el_channels;
            break;

          case ID_USAC_EXT:
            if (self->pUsacConfig[streamIndex]->element[el].extElement.usacExtElementType ==
                ID_EXT_ELE_MCT) {
              if (getOnOffFlag(self, chIdx)) {
                int grp = self->pUsacConfig[streamIndex]->sigIdx2GrpIdx[chIdx];
                int mctErr = 0;
                int firstSigIdx = 0;
                int signalsInGroup = self->pUsacConfig[streamIndex]->m_signalGroupType[grp].count;

                for (int i = 0;
                     i < self->pUsacConfig[streamIndex]->m_signalGroupType[grp].firstSigIdx; i++) {
                  if (getOnOffFlag(self, i)) firstSigIdx++;
                }

                mctErr = CMct_Initialize(&self->pMCTdec[grp],
                                         self->pUsacConfig[streamIndex]
                                             ->element[el]
                                             .extElement.extConfig.mct.mctChanMask,
                                         firstSigIdx, signalsInGroup);
                if (mctErr != 0) {
                  goto bail;
                }
              }
            }
            break;
          default:
            break;
        }

        if (self->elements[el] == ID_END) {
          break;
        }

        el++;
      }

      chIdx = aacChannelsOffsetIdx;
      ch = aacChannelsOffset;
      for (int _ch = 0; _ch < ascChannels; _ch++) {
        /* Allocate persistent channel memory */
        if (getOnOffFlag(self, chIdx)) {
          self->pAacDecoderStaticChannelInfo[ch] = GetAacDecoderStaticChannelInfo(ch);
          if (self->pAacDecoderStaticChannelInfo[ch] == NULL) {
            goto bail;
          }
          self->pAacDecoderStaticChannelInfo[ch]->concealmentInfo.spectralCoefficient =
              GetConcealmentSpecBuffer(ch);
          if (self->pAacDecoderStaticChannelInfo[ch]->concealmentInfo.spectralCoefficient == NULL) {
            goto bail;
          }
          self->pAacDecoderStaticChannelInfo[ch]->pOverlapBuffer =
              GetOverlapBuffer(ch); /* This area size depends on the AOT */
          if (self->pAacDecoderStaticChannelInfo[ch]->pOverlapBuffer == NULL) {
            goto bail;
          }
          if (flags & (AC_USAC | AC_RSVD50 | AC_MPEGH3DA /*|AC_BSAC*/)) {
            self->pAacDecoderStaticChannelInfo[ch]->hArCo = CArco_Create();
            if (self->pAacDecoderStaticChannelInfo[ch]->hArCo == NULL) {
              goto bail;
            }
          }

          ch++;
        }
        chIdx++;
      }

      if (flags & (AC_USAC | AC_MPEGH3DA)) {
        int complexStereoPredPossible = 0;
        ch = aacChannelsOffset;
        chIdx = aacChannelsOffsetIdx;
        for (int _el2 = 0; _el2 < (int)asc->m_sc.m_usacConfig.m_usacNumElements; _el2++) {
          int el2 = elementOffset + _el2;
          int elCh = 0, ch2;

          if (self->elements[el2] == ID_USAC_CPE) {
            elCh = 2;
          } else if (IS_CHANNEL_ELEMENT(self->elements[el2])) {
            elCh = 1;
          }

          if (elCh && !getOnOffFlag(self, chIdx)) {
            chIdx += elCh;
            continue;
          }

          if (elFlags[el2] & AC_EL_USAC_CP_POSSIBLE) {
            complexStereoPredPossible = 1;
            if (self->cpeStaticData[el2] == NULL) {
              self->cpeStaticData[el2] = GetCpePersistentData();
              if (self->cpeStaticData[el2] == NULL) {
                goto bail;
              }
              self->cpeStaticData[el2]->jointStereoPersistentData.winGroupsPrev = 1;
            }
          }

          for (ch2 = 0; ch2 < elCh; ch2++) {
            /* IGF decoder config */
            self->igf_private_data_common[ch2].virtualSpec =
                &self->pTimeData2[1024 *
                                  ch2]; /* required: 1024 for each channel of a channel element */
            if (asc->m_sc.m_usacConfig.element[_el2].enhancedNoiseFilling) {
              iisIGFDecLibInit(
                  &(self->pAacDecoderStaticChannelInfo[ch]->IGF_StaticData),
                  &(self->pAacDecoderChannelInfo[ch]->IGFdata), &self->igf_private_data_common[ch2],
                  asc->m_sc.m_usacConfig.element[_el2].igfStartIndex,   /* igfStartIndex        */
                  asc->m_sc.m_usacConfig.element[_el2].igfStopIndex,    /* igfStopIndex         */
                  asc->m_sc.m_usacConfig.element[_el2].igfUseHighRes,   /* igfUseHighRes        */
                  asc->m_sc.m_usacConfig.element[_el2].igfUseWhitening, /* igfUseWhitening      */
                  self->samplingRateInfo[streamIndex].samplingRate,     /* aacSampleRate        */
                  self->streamInfo.aacSamplesPerFrame,                  /* aacFrameLength       */
                  self->samplingRateInfo[streamIndex].ScaleFactorBands_Long, /* *sfb_offset_LB */
                  self->samplingRateInfo[streamIndex].NumberOfScaleFactorBands_Long, /* len_LB */
                  self->samplingRateInfo[streamIndex].ScaleFactorBands_Short, /* *sfb_offset_SB */
                  self->samplingRateInfo[streamIndex].NumberOfScaleFactorBands_Short, /* len_SB */
                  asc->m_sc.m_usacConfig.element[_el2]
                      .enhancedNoiseFilling,                             /* enhancedNoiseFilling */
                  asc->m_sc.m_usacConfig.element[_el2].igfAfterTnsSynth, /* igfAfterTnsSynth     */
                  asc->m_sc.m_usacConfig.element[_el2]
                      .igfIndependentTiling /* igfIndependentTiling */
              );
            }
            /* Hook element specific cpeStaticData into channel specific aacDecoderStaticChannelInfo
             */
            self->pAacDecoderStaticChannelInfo[ch]->pCpeStaticData = self->cpeStaticData[el2];
            if (self->pAacDecoderStaticChannelInfo[ch]->pCpeStaticData != NULL) {
              self->pAacDecoderStaticChannelInfo[ch]
                  ->pCpeStaticData->jointStereoPersistentData.spectralCoeffs[ch2] =
                  self->pAacDecoderStaticChannelInfo[ch]->concealmentInfo.spectralCoefficient;
              self->pAacDecoderStaticChannelInfo[ch]
                  ->pCpeStaticData->jointStereoPersistentData.specScale[ch2] =
                  self->pAacDecoderStaticChannelInfo[ch]->concealmentInfo.specScale;
              self->pAacDecoderStaticChannelInfo[ch]
                  ->pCpeStaticData->jointStereoPersistentData.scratchBuffer =
                  self->pAacDecoderChannelInfo[ch]->pComStaticData->pWorkBufferCore1->workBuffer;
              self->pAacDecoderStaticChannelInfo[ch]
                  ->pCpeStaticData->jointStereoPersistentData.scratchBuffer2 =
                  &self->pTimeData2[2 * 1024]; /* required size: 1024 */
            }
            chIdx++;
            ch++;
          } /* for each channel in current element */
          if (complexStereoPredPossible && (elCh == 2)) {
            /* needed once for all channels */
            if (self->pAacDecoderChannelInfo[ch - 1]->pComStaticData->cplxPredictionData == NULL) {
              self->pAacDecoderChannelInfo[ch - 1]->pComStaticData->cplxPredictionData =
                  GetCplxPredictionData();
            }
            if (self->pAacDecoderChannelInfo[ch - 1]->pComStaticData->cplxPredictionData == NULL) {
              goto bail;
            }
          }
          if (elCh > 0) {
            self->pAacDecoderStaticChannelInfo[ch - elCh]->nfRandomSeed = (ULONG)0x3039;
            if (self->elements[el2] == ID_USAC_CPE) {
              self->pAacDecoderStaticChannelInfo[ch - elCh + 1]->nfRandomSeed = (ULONG)0x10932;
            }
          }
        } /* for each element */
      }

      /* Make allocated channel count persistent in decoder context. */
      self->aacChannels = ch;
      self->allocChannels[streamIndex] = ch - aacChannelsOffset;
    }

    /* Make amount of signalled channels persistent in decoder context. */
    self->ascChannels[streamIndex] = ascChannels;
    /* Init the previous channel count values. This is required to avoid a mismatch of memory
       accesses in the error concealment module and the allocated channel structures in this
       function. */
    self->aacChannelsPrev = 0;
  }

  /* Update structures */
  if (*configChanged) {
    /* Things to be done for each channel, which do not involve allocating memory.
       Doing these things only on the channels needed for the current configuration
       (ascChannels) could lead to memory access violation later (error concealment). */
    int ch = aacChannelsOffset;
    int chIdx = aacChannelsOffsetIdx;
    for (int _ch = 0; _ch < self->ascChannels[streamIndex]; _ch++) {
      if (!getOnOffFlag(self, chIdx)) {
        chIdx++;
        continue;
      }

      self->pAacDecoderChannelInfo[ch]->granuleLength = self->streamInfo.aacSamplesPerFrame / 8;
      self->pAacDecoderChannelInfo[ch]->renderMode = initRenderMode;

      mdct_init(&self->pAacDecoderStaticChannelInfo[ch]->IMdct,
                self->pAacDecoderStaticChannelInfo[ch]->pOverlapBuffer, OverlapBufferSize);

      /* Reset concealment only if ASC changed. Otherwise it will be done with any config callback.
         E.g. every time the LATM SMC is present. */
      CConcealment_InitChannelData(&self->pAacDecoderStaticChannelInfo[ch]->concealmentInfo,
                                   &self->concealCommonData, initRenderMode,
                                   self->streamInfo.aacSamplesPerFrame);
      ch++;
      chIdx++;
    }
  }

  if (*configChanged) {
    int drcDecSampleRate, drcDecFrameSize;

    if (self->streamInfo.extSamplingRate != 0) {
      drcDecSampleRate = self->streamInfo.extSamplingRate;
      drcDecFrameSize = (self->streamInfo.aacSamplesPerFrame * self->streamInfo.extSamplingRate) /
                        self->streamInfo.aacSampleRate;
    } else {
      drcDecSampleRate = self->streamInfo.aacSampleRate;
      drcDecFrameSize = self->streamInfo.aacSamplesPerFrame;
    }

    if (FDK_drcDec_Init(self->hUniDrcDecoder, drcDecFrameSize, drcDecSampleRate,
                        self->aacChannels) != 0)
      goto bail;
  }
  self->multibandDrcPresent = 0;
  if (flags & AC_MPEGH3DA) {
    self->multibandDrcPresent =
        (UCHAR)FDK_drcDec_GetParam(self->hUniDrcDecoder, DRC_DEC_IS_MULTIBAND_DRC_1);
  }

  if (self->multibandDrcPresent) {
    /* Create filterbanks for Multiband DRC for object and/or HOA signal groups */

    STFT_FILTERBANK_CONFIG stftFilterbankConfigAnalysis;
    STFT_FILTERBANK_CONFIG stftFilterbankConfigSynthesis;
    int stft_err, grp;

    self->stftFrameSize = 256;
    self->numTimeSlots = self->streamInfo.aacSamplesPerFrame / self->stftFrameSize;

    stftFilterbankConfigAnalysis.frameSize = self->stftFrameSize;
    stftFilterbankConfigAnalysis.fftSize = 2 * self->stftFrameSize;
    stftFilterbankConfigAnalysis.stftFilterbankMode = STFT_FILTERBANK_MODE_TIME_TO_FREQ;

    stftFilterbankConfigSynthesis.frameSize = self->stftFrameSize;
    stftFilterbankConfigSynthesis.fftSize = 2 * self->stftFrameSize;
    stftFilterbankConfigSynthesis.stftFilterbankMode = STFT_FILTERBANK_MODE_FREQ_TO_TIME;

    for (grp = 0; grp < self->pUsacConfig[streamIndex]->bsNumSignalGroups; grp++) {
      int transportStartChannel =
          self->pUsacConfig[streamIndex]->m_signalGroupType[grp].firstSigIdx;
      int transportNumChannels = self->pUsacConfig[streamIndex]->m_signalGroupType[grp].count;

      for (int ch = 0; ch < transportNumChannels; ch++) {
        stft_err = StftFilterbank_Open(&stftFilterbankConfigAnalysis,
                                       &(self->stftFilterbankAnalysis[transportStartChannel + ch]));
        if (stft_err) {
          goto bail;
        }

        stft_err =
            StftFilterbank_Open(&stftFilterbankConfigSynthesis,
                                &(self->stftFilterbankSynthesis[transportStartChannel + ch]));
        if (stft_err) {
          goto bail;
        }
      }
      self->STFT_headroom_prescaling[grp] = 0;
    }
  }

  if ((asc->m_aot == AOT_MPEGH3DA) || (asc->m_aot == AOT_USAC)) {
    pcmLimiter_SetAttack(self->hLimiter, TDL_MPEGH3DA_DEFAULT_ATTACK);
    pcmLimiter_SetThreshold(self->hLimiter, TDL_MPEGH3DA_DEFAULT_THRESHOLD);
  }

  if (*configChanged) {
    if (asc->m_aot == AOT_MPEGH3DA) {
      err = CAacDecoder_InitRenderer(
          self, self->pUsacConfig[streamIndex], self->samplingRateInfo[streamIndex].samplingRate,
          self->streamInfo.aacSamplesPerFrame, elementOffset, self->workBufferCore2);
      if (err != AAC_DEC_OK) {
        goto bail;
      }
    }
  }

  CAacDecoder_AcceptFlags(self, asc, flags, elFlags, streamIndex, elementOffset);

  /* Update externally visible copy of flags */
  self->streamInfo.flags = self->flags[0];

  return err;

bail:
  CAacDecoder_DeInit(self, streamIndex);
  return AAC_DEC_OUT_OF_MEMORY;
}

LINKSPEC_CPP AAC_DECODER_ERROR CAacDecoder_DecodeFrame(HANDLE_AACDECODER self, const UINT flags,
                                                       PCM_DEC* pTimeData, const INT timeDataSize,
                                                       const int timeDataChannelOffset) {
  AAC_DECODER_ERROR ErrorStatus = AAC_DEC_OK;

  CProgramConfig* pce;
  HANDLE_FDK_BITSTREAM bs = transportDec_GetBitstream(self->hInput, 0);

  MP4_ELEMENT_ID type = ID_NONE; /* Current element type */
  INT aacChannels = 0;           /* Channel counter for channels found in the bitstream */
  int streamIndex = 0;           /* index of the current substream */

  INT auStartAnchor =
      (INT)FDKgetValidBits(bs); /* AU start bit buffer position for AU byte alignment */

  INT checkSampleRate = self->streamInfo.aacSampleRate;

  INT CConceal_TDFading_Applied[(28)] = {0}; /* Initialize status of Time Domain fading */

  /* Init fBsRestartOk. For USAC/MPEG-H defaults to zero because AC_INDEP is required.
     For all other formats every frame is independent. */
  if (self->flags[streamIndex] & (AC_MPEGH3DA | AC_USAC)) {
    self->streamInfo.fBsRestartOk = 0;
  } else {
    self->streamInfo.fBsRestartOk = 1;
  }

  if (self->aacChannels <= 0) {
    return AAC_DEC_UNSUPPORTED_CHANNELCONFIG;
  }

  /* Any supported base layer valid AU will require more than 16 bits. */
  if ((transportDec_GetAuBitsRemaining(self->hInput, 0) < 15) &&
      (flags & (AACDEC_CONCEAL | AACDEC_FLUSH)) == 0) {
    self->frameOK = 0;
    ErrorStatus = AAC_DEC_DECODE_FRAME_ERROR;
  }

  if (self->flags[0] & AC_MPEGH3DA) {
    updateOnOffFlags(self);
    if (self->useElementSkipping) {
      int ch, cnt = 0;

      for (ch = 0; ch < self->ascChannels[0]; ch++) {
        cnt += getOnOffFlag(self, ch);
      }
      if (cnt > (28)) return AAC_DEC_UNSUPPORTED_CHANNELCONFIG;
    }
  }

  /* Reset Program Config structure */
  pce = &self->pce;
  CProgramConfig_Reset(pce);

  CAacDecoder_AncDataReset(&self->ancData);
  if (!(flags & (AACDEC_CONCEAL | AACDEC_FLUSH)) && !(self->flags[0] & (AC_USAC | AC_MPEGH3DA))) {
    int ch;
    if (self->streamInfo.channelConfig == 0) {
      /* Init Channel/Element mapping table */
      for (ch = 0; ch < (28); ch++) {
        self->chMapping[ch] = 255;
      }
      if (!CProgramConfig_IsValid(pce)) {
        int el;
        for (el = 0; el < (3 * ((28) * 2) + (((28) * 2)) / 2 + 4 * (1) + 1); el++) {
          self->elements[el] = ID_NONE;
        }
      }
    }
  }

  /* Check sampling frequency  */
  if (self->streamInfo.aacSampleRate <= 0) {
    /* Instance maybe uninitialized! */
    return AAC_DEC_UNSUPPORTED_SAMPLINGRATE;
  }
  switch (checkSampleRate) {
    case 96000:
    case 88200:
    case 64000:
    case 16000:
    case 12000:
    case 11025:
    case 8000:
    case 7350:
    case 48000:
    case 44100:
    case 32000:
    case 24000:
    case 22050:
      break;
    default:
      if (!(self->flags[0] & (AC_USAC | AC_RSVD50 | AC_MPEGH3DA))) {
        return AAC_DEC_UNSUPPORTED_SAMPLINGRATE;
      }
      break;
  }

  if (flags & AACDEC_CLRHIST) {
    int ch;
    /* Clear history */
    for (ch = 0; ch < self->aacChannels; ch++) {
      /* Reset concealment */
      CConcealment_InitChannelData(
          &self->pAacDecoderStaticChannelInfo[ch]->concealmentInfo, &self->concealCommonData,
          self->pAacDecoderChannelInfo[0]->renderMode, self->streamInfo.aacSamplesPerFrame);
      /* Clear overlap-add buffers to avoid clicks. */
      FDKmemclear(self->pAacDecoderStaticChannelInfo[ch]->pOverlapBuffer,
                  OverlapBufferSize * sizeof(FIXP_DBL));
    }
    if (self->streamInfo.channelConfig > 0) {
      /* Declare the possibly adopted old PCE (with outdated metadata) invalid. */
      CProgramConfig_Init(pce);
    }
  }

  UCHAR previous_element_index = 0; /* Canonical index of last element */
  int element_count = 0;            /* Element counter for elements found in the bitstream */
  int channel_element_count = 0;    /* Channel element counter */
  UINT MCT_elFlags[(3 * ((28) * 2) + (((28) * 2)) / 2 + 4 * (1) + 1)];
  UINT* p2_MCT_elFlags;
  int el_cnt[ID_LAST] = {0};          /* element counter ( robustness ) */
  int element_count_prev_streams = 0; /* Element count of all previous sub streams. */
  int aacChannelsIdx = 0;

  if (self->flags[streamIndex] & AC_MPEGH3DA) {
    /* Clear the MCT element flags */
    FDKmemclear(MCT_elFlags, sizeof(UINT) * (3 * ((28) * 2) + (((28) * 2)) / 2 + 4 * (1) + 1));
    /* Assign a pointer to MCT_elFlags array*/
    p2_MCT_elFlags = MCT_elFlags;
  }

  while ((type != ID_END) && (!(flags & (AACDEC_CONCEAL | AACDEC_FLUSH))) && self->frameOK) {
    int el_channels;
    int mpeghElementLength = -1, elStartBitPos = 0;

    {
      if (element_count >= (3 * ((28) * 2) + (((28) * 2)) / 2 + 4 * (1) + 1)) {
        self->frameOK = 0;
        ErrorStatus = AAC_DEC_PARSE_ERROR;
        break;
      }
      type = self->elements[element_count];
    }

    /* Determine current sub stream */
    if (self->flags[streamIndex] & (AC_MPEGH3DA | AC_USAC | AC_RSVD50)) {
      int numElements = 0;

      for (int i = 0; i <= streamIndex; i++) {
        numElements += self->numUsacElements[i];
      }
      if (element_count >= numElements) {
        if ((streamIndex < TPDEC_MAX_TRACKS - 1) && self->ascChannels[streamIndex + 1] > 0) {
          element_count_prev_streams = element_count;
          streamIndex++;
          numElements += self->numUsacElements[streamIndex];
        }
      }
      /* Detect first element of sub stream to switch bit stream and parse indep flag. */
      if (element_count == (numElements - self->numUsacElements[streamIndex])) {
        /* Do byte alignment of current bit stream before switching to next. */
        /* Only prerollAUs are byteAligned with respect to the first bit */
        /* Byte alignment with respect to the first bit of the raw_data_block(). */
        if (!(self->flags[streamIndex] & (AC_RSVD50 | AC_USAC)) ||
            (self->prerollAULength[self->accessUnit]) /* indicates preroll */
        ) {
          FDKbyteAlign(bs, auStartAnchor);
        }
        /* Switch to next sub stream */
        bs = transportDec_GetBitstream(self->hInput, streamIndex);
        auStartAnchor = (INT)FDKgetValidBits(bs);

        if ((transportDec_GetAuBitsTotal(self->hInput, streamIndex) > 0) ||
            (transportDec_GetFormat(self->hInput) == TT_MP4_ADIF)) {
          if ((self->flags[streamIndex] & (AC_USAC | AC_RSVD50) && element_count == 0) ||
              (self->flags[streamIndex] & AC_MPEGH3DA)) {
            self->flags[streamIndex] &= ~AC_INDEP;

            if (FDKreadBit(bs)) {
              self->flags[streamIndex] |= AC_INDEP;
              self->streamInfo.fBsRestartOk = 1;
            } else if (self->aacChannelsPrev == 0) {
              self->frameOK =
                  0; /* decoding cannot start on a frame which is no independent frame */
            }

            int ch = aacChannels;
            for (int chIdx = aacChannels; chIdx < self->ascChannels[streamIndex]; chIdx++) {
              if (getOnOffFlag(self, chIdx)) {
                /* Robustness check */
                if (ch >= self->aacChannels) {
                  return AAC_DEC_UNKNOWN;
                }

                /* if last frame was broken and this frame is no independent frame, correct decoding
                 * is impossible we need to trigger concealment */
                if ((CConcealment_GetLastFrameOk(
                         &self->pAacDecoderStaticChannelInfo[ch]->concealmentInfo, 1) == 0) &&
                    !(self->flags[streamIndex] & AC_INDEP)) {
                  self->frameOK = 0;
                }
                ch++;
              }
            }
          }
        }
      }
      if ((transportDec_GetAuBitsTotal(self->hInput, streamIndex) <= 0) &&
          (transportDec_GetFormat(self->hInput) != TT_MP4_ADIF)) {
        el_channels = CAacDecoder_GetELChannels(type);

        if (el_channels > 0) {
          self->channel_elements[channel_element_count++] = type;
        }
        aacChannels += el_channels;
        aacChannelsIdx += el_channels;
        element_count++;
        continue;
      }
    } else {
      streamIndex = 0;
    }

    if ((INT)FDKgetValidBits(bs) < 0) {
      self->frameOK = 0;
    }

    switch (type) {
      case ID_SCE:
      case ID_CPE:
      case ID_LFE:
      case ID_USAC_SCE:
      case ID_USAC_CPE:
      case ID_USAC_LFE:
        if (element_count >= (3 * ((28) * 2) + (((28) * 2)) / 2 + 4 * (1) + 1)) {
          self->frameOK = 0;
          ErrorStatus = AAC_DEC_PARSE_ERROR;
          break;
        }

        if (self->flags[streamIndex] & AC_MPEGH3DA) {
          if (self->pUsacConfig[streamIndex]->elementLengthPresent) {
            /* The value of the element length can be used to skip elements containing signals that
             * are not to be rendered. */
            mpeghElementLength = FDKreadBits(bs, 16);
            elStartBitPos = (INT)FDKgetValidBits(bs);
          }
        }

        el_channels = CAacDecoder_GetELChannels(type);

        if (self->flags[streamIndex] & AC_MPEGH3DA) {
          /* Create an elFlag array for use in MCT */
          *p2_MCT_elFlags++ = self->elFlags[element_count];
          if (el_channels == 2) *p2_MCT_elFlags++ = self->elFlags[element_count];

          if (!getOnOffFlag(self, aacChannelsIdx)) {
            if (mpeghElementLength == -1) {
              self->frameOK = 0;
              ErrorStatus = AAC_DEC_PARSE_ERROR;
              break;
            }
            /* Skip signals that are off. */
            aacChannelsIdx += el_channels;
            FDKpushFor(bs, mpeghElementLength);
            break;
          }
        } /*if (self->flags[streamIndex] & AC_MPEGH3DA) */
        /*
          Consistency check
         */
        {
          int totalAscChannels = 0;

          for (int i = 0; i < TPDEC_MAX_TRACKS; i++) {
            totalAscChannels += self->ascChannels[i];
          }
          if ((el_cnt[type] >= (totalAscChannels >> (el_channels - 1))) ||
              (aacChannels > (totalAscChannels - el_channels))) {
            ErrorStatus = AAC_DEC_DECODE_FRAME_ERROR;
            self->frameOK = 0;
            break;
          }
        }

        if (self->frameOK) {
          ErrorStatus = CChannelElement_Read(
              bs, &self->pAacDecoderChannelInfo[aacChannels],
              &self->pAacDecoderStaticChannelInfo[aacChannels], self->streamInfo.aot,
              &self->samplingRateInfo[streamIndex], self->flags[streamIndex],
              self->elFlags[element_count], self->streamInfo.aacSamplesPerFrame, el_channels,
              self->streamInfo.epConfig, self->hInput);
          if (ErrorStatus != AAC_DEC_OK) {
            self->frameOK = 0;
          }
        }

        if (self->frameOK) {
          /* Lookup the element and decode it only if it belongs to the current program */
          if (CProgramConfig_LookupElement(
                  pce, self->streamInfo.channelConfig,
                  self->pAacDecoderChannelInfo[aacChannels]->ElementInstanceTag, aacChannels,
                  self->chMapping, self->channelType, self->channelIndices, (28),
                  &previous_element_index, self->elements, type)) {
            self->channel_elements[channel_element_count++] = type;
            aacChannels += el_channels;
            aacChannelsIdx += el_channels;
          } else {
            self->frameOK = 0;
          }
        }

        el_cnt[type]++;
        break;

      case ID_USAC_EXT: {
        if ((element_count - element_count_prev_streams) >=
            (3 * ((28) * 2) + (((28) * 2)) / 2 + 4 * (1) + 1)) {
          self->frameOK = 0;
          ErrorStatus = AAC_DEC_PARSE_ERROR;
          break;
        }
        /* parse extension element payload
           q.v. mpegh3daExtElement() ISO/IEC DIS 23008-3  Table 30
           or   UsacExElement() ISO/IEC FDIS 23003-3:2011(E)  Table 21
         */
        int usacExtElementPayloadLength;
        /* int usacExtElementStart, usacExtElementStop; */

        if (FDKreadBit(bs)) {   /* usacExtElementPresent */
          if (FDKreadBit(bs)) { /* usacExtElementUseDefaultLength */
            usacExtElementPayloadLength = self->pUsacConfig[streamIndex]
                                              ->element[element_count - element_count_prev_streams]
                                              .extElement.usacExtElementDefaultLength;
          } else {
            usacExtElementPayloadLength = FDKreadBits(bs, 8);
            if (usacExtElementPayloadLength == (UINT)(1 << 8) - 1) {
              UINT valueAdd = FDKreadBits(bs, 16);
              usacExtElementPayloadLength += (INT)valueAdd - 2;
            }
          }
          if (usacExtElementPayloadLength > 0) {
            int usacExtBitPos;

            if (self->pUsacConfig[streamIndex]
                    ->element[element_count - element_count_prev_streams]
                    .extElement.usacExtElementPayloadFrag) {
              /* usacExtElementStart = */ FDKreadBit(bs);
              /* usacExtElementStop = */ FDKreadBit(bs);
            } else {
              /* usacExtElementStart = 1; */
              /* usacExtElementStop = 1; */
            }

            usacExtBitPos = FDKgetValidBits(bs);

            USAC_EXT_ELEMENT_TYPE usacExtElementType =
                self->pUsacConfig[streamIndex]
                    ->element[element_count - element_count_prev_streams]
                    .extElement.usacExtElementType;

            switch (usacExtElementType) {
              case ID_EXT_ELE_OBJ_METADATA:
                if (self->targetLayout_config > -1) {
                  const OAMCONFIG* oam;
                  int objectMetadataParseError;
                  int objectGroup_count, objectGroup_index = 0;

                  for (objectGroup_count = 0;
                       objectGroup_count < (int)self->pUsacConfig[streamIndex]->objNumSignalGroups;
                       objectGroup_count++) {
                    int grpIdx =
                        self->pUsacConfig[streamIndex]->objSignalGroupsIndices[objectGroup_count];

                    if (!getOnOffFlag(self, self->pUsacConfig[streamIndex]
                                                ->m_signalGroupType[grpIdx]
                                                .firstSigIdx)) {
                      continue;
                    }
                    if (grpIdx == self->pUsacConfig[streamIndex]->sigIdx2GrpIdx[aacChannelsIdx])
                      break;
                    objectGroup_index++;
                  }

                  /* FDK_ASSERT(objectGroup_count < (int)self->pUsacConfig->objNumSignalGroups); */
                  if (objectGroup_count >=
                          (int)self->pUsacConfig[streamIndex]->objNumSignalGroups &&
                      self->hgVBAPRenderer[objectGroup_index] !=
                          NULL) /* Check amount of objectGroups counted */
                  {
                    ErrorStatus = AAC_DEC_PARSE_ERROR;
                    self->frameOK = 0;
                    break;
                  }

                  if (self->hgVBAPRenderer[objectGroup_index] != NULL) {
                    oam = &self->pUsacConfig[streamIndex]
                               ->element[element_count - element_count_prev_streams]
                               .extElement.extConfig.oam;
                    objectMetadataParseError = objectMetadataFrame(
                        self->hgVBAPRenderer[objectGroup_index], bs, usacExtElementPayloadLength,
                        oam->lowDelayMetadataCoding,
                        self->hgVBAPRenderer[objectGroup_index]->numObjects,
                        oam->hasDynamicObjectPriority, oam->hasUniformSpread);
                    if (objectMetadataParseError != 0) {
                      ErrorStatus = AAC_DEC_PARSE_ERROR;
                      self->frameOK = 0;
                    }
                  }
                }
                break;

              case ID_EXT_ELE_PROD_METADATA:
                if (self->targetLayout_config > -1) {
                  int sigGrp, objGrp = 0, gvbapIdx = 0;

                  for (sigGrp = 0; sigGrp < self->pUsacConfig[streamIndex]->bsNumSignalGroups;
                       sigGrp++) {
                    if (self->pUsacConfig[streamIndex]->m_signalGroupType[sigGrp].type == 1) {
                      INT bsReferenceDistance =
                          self->pUsacConfig[streamIndex]
                              ->element[element_count - element_count_prev_streams]
                              .extElement.extConfig.prodMetadata.bsReferenceDistance;
                      INT hasObjectDistance =
                          self->pUsacConfig[streamIndex]
                              ->element[element_count - element_count_prev_streams]
                              .extElement.extConfig.prodMetadata.hasObjectDistance &
                          ((ULONG)1 << (31 - objGrp));
                      INT err;

                      if (getOnOffFlag(self, self->pUsacConfig[streamIndex]
                                                 ->m_signalGroupType[sigGrp]
                                                 .firstSigIdx)) {
                        if (self->hgVBAPRenderer[gvbapIdx] != NULL) {
                          err = prodMetadataFrameGroup(self->hgVBAPRenderer[gvbapIdx], bs,
                                                       self->hgVBAPRenderer[gvbapIdx]->numObjects,
                                                       bsReferenceDistance, hasObjectDistance);
                          gvbapIdx++;
                        } else {
                          err = 1;
                        }
                      } else {
                        err = prodMetadataFrameGroup(
                            NULL, bs,
                            self->pUsacConfig[streamIndex]->m_signalGroupType[sigGrp].count,
                            bsReferenceDistance, hasObjectDistance);
                      }

                      if (err != 0) {
                        ErrorStatus = AAC_DEC_PARSE_ERROR;
                        self->frameOK = 0;
                        break;
                      }

                      objGrp++;
                    }
                  }
                }
                break;
              case ID_EXT_ELE_MCT: {
                int mctErr;
                int grp = self->pUsacConfig[streamIndex]->sigIdx2GrpIdx[aacChannelsIdx];

                if (self->pMCTdec[grp]) {
                  /* Store initial value for counting bits */
                  int bits = (int)FDKgetValidBits(bs);

                  mctErr = CMct_inverseMctParseBS(
                      self->pMCTdec[grp], bs, self->flags[streamIndex] & AC_INDEP,
                      self->pUsacConfig[streamIndex]->m_signalGroupType[grp].count);

                  self->pMCTdec[grp]->Mct_group_parsed = 1;

                  /*Find bits consumed */
                  int bits_left = bits - (int)FDKgetValidBits(bs);
                  /* Check if number of bits read corresponds to the number of bytes transmitted.
                   * Max 7 bits can be wasted,i.e. the last byte can be partially used */
                  int diff = (usacExtElementPayloadLength << 3) - bits_left;
                  if ((diff > 7) || (diff < 0)) {
                    mctErr = -1;
                  }
                  if (mctErr != 0) {
                    self->frameOK = 0;
                    ErrorStatus = AAC_DEC_PARSE_ERROR;
                  }
                }
              } break;
              case ID_EXT_ELE_FMT_CNVRTR: /* FormatConverterFrame() */
                if (self->pFormatConverter[streamIndex] != NULL) {
                  FormatConverterFrame(self->pFormatConverter[streamIndex], bs);
                }
                break;
              case ID_EXT_ELE_UNI_DRC: /* uniDrcGain() */
              {
                DRC_DEC_ERROR drcErr;

                drcErr = FDK_drcDec_ReadUniDrcGain(self->hUniDrcDecoder, bs, streamIndex);
                if (drcErr != DRC_DEC_OK) {
                  ErrorStatus = AAC_DEC_PARSE_ERROR;
                }
              } break;

              default:
                break;
            }

            /* Skip any remaining bits of extension payload */
            usacExtBitPos =
                (usacExtElementPayloadLength * 8) - (usacExtBitPos - FDKgetValidBits(bs));
            if (usacExtBitPos < 0) {
              self->frameOK = 0;
              ErrorStatus = AAC_DEC_PARSE_ERROR;
            }
            FDKpushBiDirectional(bs, usacExtBitPos);
          }
        }
      } break;
      case ID_END:
      case ID_USAC_END:
        break;

      default:
        ErrorStatus = AAC_DEC_DECODE_FRAME_ERROR;
        self->frameOK = 0;
        break;
    }

    if (self->flags[streamIndex] & AC_MPEGH3DA) {
      if (mpeghElementLength != -1) {
        int bitsOffset = mpeghElementLength - (elStartBitPos - FDKgetValidBits(bs));
        if (bitsOffset != 0) {
          ErrorStatus = AAC_DEC_PARSE_ERROR;
          self->frameOK = 0;
        }
        /* FDKpushBiDirectional(bs, bitsOffset); */
      }
    }

    /* Support arbitrary amount of FIL elements for General Audio bitstreams by not increasing
     * element_count */
    if ((self->flags[0] & (AC_USAC | AC_RSVD50 | AC_MPEGH3DA | AC_ELD | AC_SCALABLE | AC_ER))) {
      element_count++;
    }

  } /* while ( (type != ID_END) ... ) */

  if (!(flags & (AACDEC_CONCEAL | AACDEC_FLUSH)) && (self->frameOK == 1) &&
      !(self->flags[streamIndex] & AC_MPEGH3DA)) {
    {
      if ((self->aacChannels != aacChannels)) {
        self->frameOK = 0;

        if (ErrorStatus == AAC_DEC_OK) {
          ErrorStatus = AAC_DEC_PARSE_ERROR;
        }
      }
    }
  }

  if (!(flags & (AACDEC_CONCEAL | AACDEC_FLUSH))) {
    /* float decoder checks if bitsLeft is in range 0-7; only prerollAUs are byteAligned with
     * respect to the first bit */
    /* Byte alignment with respect to the first bit of the raw_data_block(). */
    if (!(self->flags[streamIndex] & (AC_RSVD50 | AC_USAC)) ||
        (self->prerollAULength[self->accessUnit]) /* indicates preroll */
    ) {
      FDKbyteAlign(bs, auStartAnchor);
    }

    /* Check if all bits of the raw_data_block() have been read. */
    if (transportDec_GetAuBitsTotal(self->hInput, 0) > 0) {
      INT unreadBits = transportDec_GetAuBitsRemaining(self->hInput, 0);
      /* for pre-roll frames pre-roll length has to be used instead of total AU lenght */
      /* unreadBits regarding preroll bounds */
      if (self->prerollAULength[self->accessUnit]) {
        unreadBits = unreadBits - transportDec_GetAuBitsTotal(self->hInput, 0) +
                     (INT)self->prerollAULength[self->accessUnit];
      }

      if ((unreadBits != 0 && !(self->flags[streamIndex] & (AC_RSVD50 | AC_USAC)) &&
           streamIndex == 0) ||
          (self->prerollAULength[self->accessUnit] && 0 < unreadBits && unreadBits <= 7)) {
        self->frameOK = 0;
        /* Do not overwrite current error */
        if (ErrorStatus == AAC_DEC_OK && self->frameOK == 0) {
          ErrorStatus = AAC_DEC_PARSE_ERROR;
        }
        /* Always put the bitbuffer at the right position after the current Access Unit. */
        FDKpushBiDirectional(bs, unreadBits);
      }
    }

    /* Check the last element. The terminator (ID_END) has to be the last one (even if ER syntax is
     * used). */
    if (self->frameOK && type != ID_END) {
      /* Do not overwrite current error */
      if (ErrorStatus == AAC_DEC_OK) {
        ErrorStatus = AAC_DEC_PARSE_ERROR;
      }
      self->frameOK = 0;
    }

    streamIndex = 0;

    /* MCT */
    if (self->flags[streamIndex] & AC_MPEGH3DA) {
      for (int grp = 0; grp < self->pUsacConfig[streamIndex]->bsNumSignalGroups; grp++) {
        if (self->pMCTdec[grp]) {
          if ((self->frameOK) && (self->pMCTdec[grp]->Mct_group_parsed)) {
            int err = 0;

            err = CMct_MCT_StereoFilling(
                self->pMCTdec[grp], &self->streamInfo, self->pAacDecoderChannelInfo,
                self->pAacDecoderStaticChannelInfo, &self->samplingRateInfo[streamIndex],
                MCT_elFlags, (self->flags[streamIndex] & AC_INDEP) ? 1 : 0);
            if (err) {
              self->frameOK = 0;
              break;
            }

            /* Save MCT output spectra for usage in the next frame stereo filling calculations */
            CMct_StereoFilling_save_prev(self->pMCTdec[grp], self->pAacDecoderChannelInfo);
          } else {
            /* Clear MCT output spectra if frame not ok */
            CMct_StereoFilling_clear_prev(self->pMCTdec[grp], self->pAacDecoderChannelInfo);
          }

          self->pMCTdec[grp]->Mct_group_parsed = 0;

        } /* if (self->pMCTdec[grp]) */

      } /* for (int grp = 0; grp < self->pUsacConfig->bsNumSignalGroups; grp++) */
    }   /* if (self->flags & AC_MPEGH3DA)  */
  }

  if (!(flags & (AACDEC_CONCEAL | AACDEC_FLUSH)) && self->frameOK) {
    self->channel_elements[channel_element_count++] = ID_END;
  }
  element_count = 0;
  aacChannels = 0;
  aacChannelsIdx = 0;
  type = ID_NONE;
  previous_element_index = 0;

  while (type != ID_END && element_count < (3 * ((28) * 2) + (((28) * 2)) / 2 + 4 * (1) + 1)) {
    int el_channels;

    if ((flags & (AACDEC_CONCEAL | AACDEC_FLUSH)) || !self->frameOK) {
      self->channel_elements[element_count] = self->elements[element_count];
      if (self->channel_elements[element_count] == ID_NONE) {
        self->channel_elements[element_count] = ID_END;
      }
    }

    /* Determine current sub stream */
    if (self->flags[streamIndex] & (AC_MPEGH3DA | AC_USAC)) {
      int numElements = 0;

      for (int i = 0; i <= streamIndex; i++) {
        numElements += self->numUsacElements[i];
      }
      if (element_count >= numElements) {
        if ((streamIndex < TPDEC_MAX_TRACKS - 1) && self->pUsacConfig[streamIndex + 1] != NULL) {
          streamIndex++;
        }
      }
    } else {
      streamIndex = 0;
    }

    if (self->flags[streamIndex] & (AC_USAC | AC_MPEGH3DA | AC_BSAC)) {
      type = self->elements[element_count];
    } else {
      type = self->channel_elements[element_count];
    }

    if (!(flags & (AACDEC_CONCEAL | AACDEC_FLUSH)) && self->frameOK) {
      /* Skip sub streams not being present. */
      if ((transportDec_GetAuBitsTotal(self->hInput, streamIndex) <= 0) &&
          (transportDec_GetFormat(self->hInput) != TT_MP4_ADIF)) {
        el_channels = CAacDecoder_GetELChannels(type);
        aacChannels += el_channels;
        aacChannelsIdx += el_channels;
        element_count++;
        continue;
      }

      switch (type) {
        case ID_SCE:
        case ID_CPE:
        case ID_LFE:
        case ID_USAC_SCE:
        case ID_USAC_CPE:
        case ID_USAC_LFE:

          el_channels = CAacDecoder_GetELChannels(type);

          /* Skip signals that are not enabled */
          if (!getOnOffFlag(self, aacChannelsIdx)) {
            aacChannelsIdx += el_channels;
            break;
          }
          {
            CChannelElement_Decode(&self->pAacDecoderChannelInfo[aacChannels],
                                   &self->pAacDecoderStaticChannelInfo[aacChannels],
                                   &self->samplingRateInfo[streamIndex], self->flags[streamIndex],
                                   self->elFlags[element_count], el_channels);
          }
          aacChannels += el_channels;
          aacChannelsIdx += el_channels;
          break;
        case ID_NONE:
          type = ID_END;
          break;
        default:
          break;
      }
    }
    element_count++;
  }

  /* More AAC channels than specified by the ASC not allowed. */
  if ((aacChannels == 0 || aacChannels > self->aacChannels) &&
      !(flags & (AACDEC_CONCEAL | AACDEC_FLUSH))) {
    /* Do not overwrite current error */
    if (ErrorStatus == AAC_DEC_OK) {
      ErrorStatus = AAC_DEC_DECODE_FRAME_ERROR;
    }
    self->frameOK = 0;
    aacChannels = 0;
  }

  if (!(flags & (AACDEC_CONCEAL | AACDEC_FLUSH))) {
    if (TRANSPORTDEC_OK != transportDec_CrcCheck(self->hInput)) {
      ErrorStatus = AAC_DEC_CRC_ERROR;
      self->frameOK = 0;
    }
  }

  /* Ensure that in case of concealment a proper error status is set. */
  if ((self->frameOK == 0) && (ErrorStatus == AAC_DEC_OK)) {
    ErrorStatus = AAC_DEC_DECODE_FRAME_ERROR;
  }

  if (self->frameOK && (flags & AACDEC_FLUSH)) {
    aacChannels = self->aacChannelsPrev;
    /* Because the downmix could be active, its necessary to restore the channel type and indices.
     */
    FDKmemcpy(self->channelType, self->channelTypePrev,
              (28) * sizeof(AUDIO_CHANNEL_TYPE));                                    /* restore */
    FDKmemcpy(self->channelIndices, self->channelIndicesPrev, (28) * sizeof(UCHAR)); /* restore */
  } else {
    /* store or restore the number of channels and the corresponding info */
    if (self->frameOK && !(flags & AACDEC_CONCEAL)) {
      self->aacChannelsPrev = aacChannels; /* store */
      FDKmemcpy(self->channelTypePrev, self->channelType,
                (28) * sizeof(AUDIO_CHANNEL_TYPE));                                    /* store */
      FDKmemcpy(self->channelIndicesPrev, self->channelIndices, (28) * sizeof(UCHAR)); /* store */
    } else {
      if (self->aacChannels > 0) {
        if ((self->buildUpStatus == AACDEC_MPEGH_BUILD_UP_ON) ||
            (self->buildUpStatus == AACDEC_MPEGH_BUILD_UP_ON_IN_BAND) ||
            (self->buildUpStatus == AACDEC_USAC_BUILD_UP_ON)) {
          aacChannels = self->aacChannels;
          self->aacChannelsPrev = aacChannels; /* store */
        } else {
          aacChannels = self->aacChannelsPrev; /* restore */
        }
        FDKmemcpy(self->channelType, self->channelTypePrev,
                  (28) * sizeof(AUDIO_CHANNEL_TYPE)); /* restore */
        FDKmemcpy(self->channelIndices, self->channelIndicesPrev,
                  (28) * sizeof(UCHAR)); /* restore */
      }
    }
  }

  /* Update number of output channels */
  self->streamInfo.aacNumChannels = aacChannels;

  /* Ensure consistency of IS_OUTPUT_VALID() macro. */
  if (aacChannels == 0) {
    ErrorStatus = AAC_DEC_UNKNOWN;
  }

  /* If there is no valid data to transform into time domain, return. */
  if (!IS_OUTPUT_VALID(ErrorStatus)) {
    return ErrorStatus;
  }

  /*
    Inverse transform
  */
  {
    int c, cIdx;

    /* Create a reverse mapping table */
    UCHAR Reverse_chMapping[((28) * 2)];
    for (c = 0; c < aacChannels; c++) {
      int d;
      for (d = 0; d < aacChannels - 1; d++) {
        if (self->chMapping[d] == c) {
          break;
        }
      }
      Reverse_chMapping[c] = d;
    }

    int el;
    int el_channels;
    streamIndex = 0;
    c = 0;
    cIdx = 0;
    el_channels = 0;
    for (el = 0; el < element_count; el++) {
      int frameOk_butConceal = 0; /* Force frame concealment during mute release active state. */
      int concealApplyReturnCode;

      /* Determine current sub stream */
      if (self->flags[streamIndex] & (AC_MPEGH3DA | AC_USAC)) {
        int numElements = 0;

        for (int i = 0; i <= streamIndex; i++) {
          numElements += self->numUsacElements[i];
        }
        if (el >= numElements) {
          if ((streamIndex < TPDEC_MAX_TRACKS - 1) && self->pUsacConfig[streamIndex + 1] != NULL) {
            streamIndex++;
          }
        }
      } else {
        streamIndex = 0;
      }

      if (self->flags[streamIndex] & (AC_USAC | AC_MPEGH3DA | AC_BSAC)) {
        type = self->elements[el];
      } else {
        type = self->channel_elements[el];
      }

      {
        int nElementChannels;

        nElementChannels = CAacDecoder_GetELChannels(type);

        el_channels += nElementChannels;

        if (nElementChannels == 0) {
          continue;
        }
      }

      int offset;
      int elCh = 0;
      /* "c" iterates in canonical MPEG channel order */
      for (; cIdx < el_channels; c++, cIdx++, elCh++) {
        if (!getOnOffFlag(self, cIdx)) {
          c--;
          continue;
        }
        /* Robustness check */
        if (c >= aacChannels) {
          return AAC_DEC_UNKNOWN;
        }

        CAacDecoderChannelInfo* pAacDecoderChannelInfo = self->pAacDecoderChannelInfo[c];
        CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo =
            self->pAacDecoderStaticChannelInfo[c];

        /* Setup offset for time buffer traversal. */
        offset = Reverse_chMapping[c] * timeDataChannelOffset;

        if (self->flags[streamIndex] & AC_MPEGH3DA) {
          /* Clear audio data for sub streams which are currently not available. */
          if (transportDec_GetAuBitsTotal(self->hInput, streamIndex) <= 0 &&
              !(flags & (AACDEC_FLUSH | AACDEC_CONCEAL))) {
            FDKmemclear(pTimeData + offset, sizeof(PCM_DEC) * self->streamInfo.aacSamplesPerFrame);
            continue;
          }
        }

        if (flags & AACDEC_FLUSH) {
          /* Clear pAacDecoderChannelInfo->pSpectralCoefficient because with AACDEC_FLUSH set it
           * contains undefined data. */
          FDKmemclear(pAacDecoderChannelInfo->pSpectralCoefficient,
                      sizeof(FIXP_DBL) * self->streamInfo.aacSamplesPerFrame);
        }

        /* if The ics info is not valid and it will be stored and used in the following concealment
         * method, mark the frame as erroneous */
        {
          CIcsInfo* pIcsInfo = &pAacDecoderChannelInfo->icsInfo;
          CConcealmentInfo* hConcealmentInfo = &pAacDecoderStaticChannelInfo->concealmentInfo;
          const int mute_release_active =
              (self->frameOK && !(flags & AACDEC_CONCEAL)) &&
              ((hConcealmentInfo->concealState >= ConcealState_Mute) &&
               (hConcealmentInfo->cntValidFrames + 1 <=
                hConcealmentInfo->pConcealParams->numMuteReleaseFrames));
          const int icsIsInvalid =
              (GetScaleFactorBandsTransmitted(pIcsInfo) > GetScaleFactorBandsTotal(pIcsInfo));
          const int icsInfoUsedinFadeOut = 1;
          if (icsInfoUsedinFadeOut && icsIsInvalid && !mute_release_active) {
            self->frameOK = 0;
          }
        }

        /*
          Conceal defective spectral data
        */
        {
          CAacDecoderChannelInfo** ppAacDecoderChannelInfo = &pAacDecoderChannelInfo;
          CAacDecoderStaticChannelInfo** ppAacDecoderStaticChannelInfo =
              &pAacDecoderStaticChannelInfo;
          {
            concealApplyReturnCode = CConcealment_Apply(
                &(*ppAacDecoderStaticChannelInfo)->concealmentInfo, *ppAacDecoderChannelInfo,
                *ppAacDecoderStaticChannelInfo, &self->samplingRateInfo[streamIndex],
                self->streamInfo.aacSamplesPerFrame, 0,
                (self->frameOK && !(flags & AACDEC_CONCEAL)), self->flags[streamIndex]);
          }
        }
        if (concealApplyReturnCode == -1) {
          frameOk_butConceal = 1;
        }

        if (timeDataSize < timeDataChannelOffset * self->aacChannels) {
          ErrorStatus = AAC_DEC_OUTPUT_BUFFER_TOO_SMALL;
          break;
        }
        if (self->flushStatus && (self->flushCnt > 0) && !(flags & AACDEC_CONCEAL)) {
          FDKmemclear(pTimeData + offset, sizeof(PCM_DEC) * self->streamInfo.aacSamplesPerFrame);
        } else
          switch (pAacDecoderChannelInfo->renderMode) {
            case AACDEC_RENDER_IMDCT:

              CBlock_FrequencyToTime(
                  pAacDecoderStaticChannelInfo, pAacDecoderChannelInfo, pTimeData + offset,
                  self->streamInfo.aacSamplesPerFrame,
                  (self->frameOK && !(flags & AACDEC_CONCEAL) && !frameOk_butConceal),
                  pAacDecoderChannelInfo->pComStaticData->pWorkBufferCore1->mdctOutTemp,
                  self->aacOutDataHeadroom, self->elFlags[el], elCh);
              if (self->flags[streamIndex] & AC_MPEGH3DA) {
                ltp_post(pTimeData + offset, self->streamInfo.aacSamplesPerFrame,
                         self->streamInfo.aacSampleRate, pAacDecoderStaticChannelInfo->ltp_param,
                         &(pAacDecoderStaticChannelInfo->ltp_pitch_int_past),
                         &(pAacDecoderStaticChannelInfo->ltp_pitch_fr_past),
                         &(pAacDecoderStaticChannelInfo->ltp_gain_past),
                         &(pAacDecoderStaticChannelInfo->ltp_gainIdx_past),
                         pAacDecoderStaticChannelInfo->ltp_mem_in,
                         pAacDecoderStaticChannelInfo->ltp_mem_out);
              }

              break;
            default:
              ErrorStatus = AAC_DEC_UNKNOWN;
              break;
          }
        /* TimeDomainFading */
        if (!CConceal_TDFading_Applied[c]) {
          CConceal_TDFading_Applied[c] = CConcealment_TDFading(
              self->streamInfo.aacSamplesPerFrame, &self->pAacDecoderStaticChannelInfo[c],
              self->aacOutDataHeadroom, pTimeData + offset, 0);
          if (c + 1 < (28) && c < aacChannels - 1) {
            /* update next TDNoise Seed to avoid muting in case of Parametric Stereo */
            self->pAacDecoderStaticChannelInfo[c + 1]->concealmentInfo.TDNoiseSeed =
                self->pAacDecoderStaticChannelInfo[c]->concealmentInfo.TDNoiseSeed;
          }
        }
      }
    }
  }

  /* Add additional concealment delay */
  self->streamInfo.outputDelay +=
      CConcealment_GetDelay(&self->concealCommonData) * self->streamInfo.aacSamplesPerFrame;

  if (!(self->flags[streamIndex] & (AC_MPEGH3DA | AC_USAC | AC_RSVD50))) {
    int el = 0;
    for (el = 0; el < (((28)) + ((28))); el++) {
      if (self->streamInfo.pElements[el] == ID_NONE) break;
    }
    self->streamInfo.numElements = el;
  }

  self->blockNumber++;

  return ErrorStatus;
}

/*!
  \brief returns the streaminfo pointer

  The function hands back a pointer to the streaminfo structure

  \return pointer to the struct
*/
LINKSPEC_CPP CStreamInfo* CAacDecoder_GetStreamInfo(HANDLE_AACDECODER self) {
  if (!self) {
    return NULL;
  }
  return &self->streamInfo;
}

void EarconDecoder_Init(HANDLE_EARCONDECODER hEarconDecoderH) {
  hEarconDecoderH->earconConfig.m_numPcmSignals = 0;
  hEarconDecoderH->earconInfo.m_numEarcons = 0;
  hEarconDecoderH->First_Frame = 1;
  hEarconDecoderH->AccumulatedFrameSize = 0;
  hEarconDecoderH->BaseframeSize = 1024;
  hEarconDecoderH->numPcmSignals_old = 0;
  hEarconDecoderH->CurrentFrameHasEarcon = 0;
  hEarconDecoderH->LastFrameHadEarcon = 0;
}

TRANSPORTDEC_ERROR PcmDataPayload(EarconDecoder* earconDecoder, FIXP_DBL* TimeData,
                                  UINT BaseframeSize, SCHAR drcStatus_targetLoudness,
                                  SCHAR defaultTargetLoudness, INT targetLayout, SHORT truncStart,
                                  SHORT truncStop) {
  FIXP_DBL EarconGain, AttGain;
  FIXP_DBL AttGain_increment;
  INT EarconShift, AttGainShift;
  EarconConfig* config = &earconDecoder->earconConfig;

  if (earconDecoder->AccumulatedFrameSize <= 0) {
    /*Prepare for the next time*/
    earconDecoder->AccumulatedFrameSize = 0;
    if (earconDecoder->BaseframeSize - truncStop + truncStart > 0) earconDecoder->First_Frame = 1;
    return TRANSPORTDEC_OK;
  }

  if ((config->m_pcmHasAttenuationGain != 0) &&
      (config->m_bsPcmAttenuationGain != earconDecoder->m_bsPcmAttenuationGain_old)) {
    /*We calculate y=-0.25*m_bsPcmAttenuationGain */
    int headroom = fNorm((FIXP_DBL)(-(INT)config->m_bsPcmAttenuationGain));

    /*Left allign negated value */
    AttGain = (FIXP_DBL)(-(INT)config->m_bsPcmAttenuationGain) << headroom;
    /*The exponent is 31 (i.e. int)  - headroom  -2 (from multiplication by 0.25)*/
    INT AttGain_exp = 31 - headroom - 2;

    /* Calculate  y=10^(x/20), which can be represented as:
    y=2^(x*log2(10)*0.05), where:
    log2(10)*0.05 is 0x550a9685 in Q-2.33 format */
    const FIXP_DBL scaledValue = 0x550a9685;
    AttGain = fMult(AttGain, scaledValue);
    /*Adjust exponent*/
    AttGain_exp = AttGain_exp - 2;
    /*Calculate pow2()*/
    AttGain = f2Pow(AttGain, AttGain_exp, &AttGain_exp);

    /*AttGain is less or equal to 1 (0x40000000 with exp=1). Make sure the exponent is always <=0 to
     * simplify the calcuations later*/
    if ((AttGain >= (FIXP_DBL)0x40000000) && (AttGain_exp > 0)) {
      AttGain = (FIXP_DBL)MAXVAL_DBL;
      AttGain_exp = 0;
    } else {
      headroom = fNorm(AttGain);
      AttGain = AttGain << headroom;
      AttGain_exp -= headroom;
    }
    if (config->m_bsPcmAttenuationGain == 255) {
      AttGain = (FIXP_DBL)0; /* special case: map -63.75 dB => -inf dB */
    }

    /*Calculate ramp up/down. Compare present and past gains and calculate increment/decrement */

    /*Keep the old PRECISE values for calculation purposes*/
    FIXP_DBL AttGain_old = earconDecoder->AttGain_store_high_precision;
    int AttGain_exp_old = earconDecoder->AttGain_exp_store_high_precision;
    earconDecoder->AttGain_store_high_precision = AttGain;
    earconDecoder->AttGain_exp_store_high_precision = AttGain_exp;

    /*Calculate increment/decrement and adjust exponent */
    int diff = AttGain_exp - AttGain_exp_old;
    if (diff >= 0) {
      AttGain_old >>= diff;
      AttGainShift = -AttGain_exp;
    } else {
      AttGain >>= (-diff);
      AttGainShift = -AttGain_exp_old;
    }

    /*Calculate gain increment value*/
    if (BaseframeSize == 1024) {
      AttGain_increment = (AttGain - AttGain_old) >> 10;
    } else if (BaseframeSize == 2048) {
      AttGain_increment = (AttGain - AttGain_old) >> 11;
    } else {
      /*Calculate ramp-up/down multiplicative factor.
      Needed for truncations and for frame lengths that
      are not a power of two */
      FIXP_DBL factor = fDivNorm((FIXP_DBL)1, (FIXP_DBL)BaseframeSize);
      AttGain_increment = fMult((AttGain - AttGain_old), factor);
    }

    /*Present-past adjustment*/
    earconDecoder->AttGain_old = AttGain_old;
    earconDecoder->AttGainShift_old = AttGainShift;
    earconDecoder->AttGain_increment_old = AttGain_increment;

    /*Store the present Attenuation parameters*/
    earconDecoder->AttGain_new = AttGain;
    earconDecoder->AttGainShift_new = AttGainShift;

    /*Store the last attenuation gain*/
    earconDecoder->m_bsPcmAttenuationGain_old = config->m_bsPcmAttenuationGain;
  }

  /*Calculate the gain applied to the Earcon data*/
  EarconGain = earconDecoder->EarconGain;
  EarconShift = earconDecoder->EarconShift;

  INT target_loudness;
  if (drcStatus_targetLoudness == UI_MANAGER_USE_DEFAULT_TARGET_LOUDNESS)
    target_loudness = -defaultTargetLoudness;
  else
    target_loudness = 4 * drcStatus_targetLoudness;

  if ((earconDecoder->target_loudness_old != target_loudness) ||
      (earconDecoder->m_bsPcmLoudnessValue_old != config->m_bsPcmLoudnessValue)) {
    FIXP_DBL EarconGainNew;
    int EarconShiftNew;

    if (target_loudness <= 0) {
      /*Set constant 57.75 in Q29.2 */
      INT const fiftySeven = 231;

      /* Loudness Normalization Gain = targetLoudness (LKFS) - PCM loudness (LKFS) */
      /* Calculate: "targetLoudness(LKFS) -(-57.75f + 0.25f *
       * hEarconDecoder->pcmConfig.bsPcmLoudnessValue)" in Q29.2 format */
      EarconGainNew =
          (FIXP_DBL)(target_loudness + fiftySeven - (INT)(config->m_bsPcmLoudnessValue));
      INT EarconGain_exp = 29;

      /*Adjust precision*/
      int headroom = fNorm(EarconGainNew);
      EarconGainNew = EarconGainNew << headroom;
      EarconGain_exp -= headroom;

      /* Calculate  y=10^(x/20), which can be represented as:
      y=2^(x*log2(10)*0.05), where:
      log2(10)*0.05 is 0x550a9685 in Q-2.33 format */
      const FIXP_DBL scaledValue = 0x550a9685;
      EarconGainNew = fMult(EarconGainNew, scaledValue);
      /*Adjust exponent*/
      EarconGain_exp = EarconGain_exp - 2;
      /*Calculate pow2()*/
      EarconGainNew = f2Pow(EarconGainNew, EarconGain_exp, &EarconGain_exp);

      /*Adjust precision*/
      headroom = fNorm(EarconGainNew);
      EarconGainNew = EarconGainNew << headroom;
      EarconGain_exp -= headroom;

      /*Shift to adjust to the output format*/
      EarconShiftNew = PCM_OUT_HEADROOM - EarconGain_exp;
    } else {
      /* Loudness Normalization off. Use gain of 1.0 */
      EarconGainNew = (FIXP_DBL)0x40000000;
      EarconShiftNew = PCM_OUT_HEADROOM - 1;
    }

    /*Store for the future*/
    earconDecoder->EarconGain = EarconGainNew;
    earconDecoder->EarconShift = EarconShiftNew;
    earconDecoder->m_bsPcmLoudnessValue_old = config->m_bsPcmLoudnessValue;
    earconDecoder->target_loudness_old = target_loudness;
  }

  /* Process delayed past signal LastFrameSamples and part of current NewFrameSamples */
  {
    /*Calculate old/new signal lengths and startPoint */
    INT LastFrameSamples;
    INT NewFrameSamples;
    INT startPoint = 0;

    if (truncStart > 0) {
      NewFrameSamples = fMax(0, truncStart - 775);
      LastFrameSamples = fMin((int)truncStart, 775);
    } else {
      NewFrameSamples = earconDecoder->BaseframeSize - fMax(775, (INT)truncStop);
      LastFrameSamples = fMax(0, 775 - truncStop);
    }

    if (earconDecoder->First_Frame == 1) {
      startPoint = LastFrameSamples;
      LastFrameSamples = 0;
    }

    earconDecoder->First_Frame = 0;

    {
      INT numSpeakers = (INT)earconDecoder->numPcmSignals_old;
      INT numSignalsMixed = earconDecoder->numSignalsMixed;
      INT* speakerGains = earconDecoder->speakerGains;
      INT NumberOfRestChannels = earconDecoder->NumberOfRestChannels;
      INT* speakerPosIndices = earconDecoder->speakerPosIndices;
      INT* speakerPosIndices_Rest = earconDecoder->speakerPosIndices_Rest;

      INT i = 0;
      INT LoopCounterValue = LastFrameSamples;
      FIXP_SGL* EarcondDataPointer = earconDecoder->EarconData;

      /*Set initial gains*/
      AttGain = earconDecoder->AttGain_intermediate;
      AttGainShift = earconDecoder->AttGainShift_intermediate;
      AttGain_increment = earconDecoder->AttGain_increment_intermediate;

      for (INT j = 0; j < 2; j++) {
        /*Apply MPEGH attenuation gain on the non-Earcon channels*/
        {
          if (earconDecoder->numPcmSignals_old != numSpeakers) {
            INT numTotalChannels = 0;

            cicp2geometry_get_numChannels_from_cicp(targetLayout, &numTotalChannels);

            if (cicp2geometry_get_front_speakers(targetLayout, numSpeakers, &numSignalsMixed,
                                                 speakerPosIndices,
                                                 speakerGains) != CICP2GEOMETRY_OK) {
              goto bail;
            }

            /*Generate a signal matrix with non-Earcon signal positions*/
            int k = 0;
            for (int i2 = 0; i2 < numTotalChannels; i2++) {
              int found = 0;
              for (int j2 = 0; j2 < numSignalsMixed; j2++) {
                if (speakerPosIndices[j2] == i2) {
                  found = 1;
                  break;
                }
              }
              if (found == 0) {
                speakerPosIndices_Rest[k] = i2;
                k++;
              }
            }

            earconDecoder->numSignalsMixed = numSignalsMixed;
            NumberOfRestChannels = numTotalChannels - numSignalsMixed;
            earconDecoder->NumberOfRestChannels = NumberOfRestChannels;
          }

          /*If gain is always 1.0 skip processing.*/
          // if ((earconDecoder->AttGain_intermediate == (FIXP_DBL)0x7fffffff) &&
          // (earconDecoder->AttGain_old == (FIXP_DBL)0x7fffffff) &&
          // (earconDecoder->AttGainShift_intermediate == 0) && (earconDecoder->AttGainShift_old ==
          // 0) && (earconDecoder->AttGain_increment_intermediate == (FIXP_DBL)0) &&
          // (earconDecoder->AttGain_increment_old == (FIXP_DBL)0)) {
          if ((AttGain == (FIXP_DBL)0x7fffffff) && (AttGainShift == 0) &&
              (AttGain_increment == (FIXP_DBL)0)) {
            NumberOfRestChannels = 0;
          }

          for (int k = 0; k < NumberOfRestChannels; k++) {
            /*Pointer to the respective channel*/
            FIXP_DBL* DecodedData = &TimeData[BaseframeSize * speakerPosIndices_Rest[k]];

            int ir = i;
            {
              FIXP_DBL AttGain_temp = AttGain;
              for (; ir < LoopCounterValue; ir++) {
                DecodedData[ir] = (fMult(DecodedData[ir], AttGain_temp) >> AttGainShift);
                AttGain_temp += AttGain_increment;
              }
            }
          }
        }
        /*Adjust for the changed speaker gain*/
        FIXP_DBL EarconGain2 = fMult(EarconGain, (FIXP_DBL)speakerGains[0]);

        /*Set target pointers*/
        FIXP_DBL* DecodedData1 = &TimeData[BaseframeSize * speakerPosIndices[0]];
        FIXP_DBL* DecodedData2 = &TimeData[BaseframeSize * speakerPosIndices[1]];

        /* Limit samples read */
        if (LoopCounterValue > 0 && numSpeakers > 0) {
          LoopCounterValue = fMin(LoopCounterValue,
                                  startPoint + earconDecoder->AccumulatedFrameSize / numSpeakers);
        }
        if ((numSpeakers == 1) && (numSignalsMixed == 1)) {
          for (; i < LoopCounterValue; i++) {
            FIXP_SGL tco = *EarcondDataPointer++;

            DecodedData1[i] = (fMult(DecodedData1[i], AttGain) >> AttGainShift) +
                              (fMult(tco, EarconGain2) >> EarconShift);

            AttGain += AttGain_increment;
          }
        } else if ((numSpeakers == 2) && (numSignalsMixed == 2)) {
          for (; i < LoopCounterValue; i++) {
            FIXP_SGL tco1 = *EarcondDataPointer++;
            FIXP_SGL tco2 = *EarcondDataPointer++;

            DecodedData1[i] = (fMult(DecodedData1[i], AttGain) >> AttGainShift) +
                              (fMult(tco1, EarconGain2) >> EarconShift);
            DecodedData2[i] = (fMult(DecodedData2[i], AttGain) >> AttGainShift) +
                              (fMult(tco2, EarconGain2) >> EarconShift);

            AttGain += AttGain_increment;
          }
        } else if ((numSpeakers == 1) && (numSignalsMixed == 2)) {
          /*Adjust for the changed speaker gain. The gains are the same for the two channels */
          EarconGain2 = fMult(EarconGain2, (FIXP_DBL)speakerGains[0]);

          for (; i < LoopCounterValue; i++) {
            FIXP_SGL tco = *EarcondDataPointer++;
            FIXP_DBL scaledInput = fMult(tco, EarconGain2) >> EarconShift;

            DecodedData1[i] = (fMult(DecodedData1[i], AttGain) >> AttGainShift) + scaledInput;
            DecodedData2[i] = (fMult(DecodedData2[i], AttGain) >> AttGainShift) + scaledInput;

            AttGain += AttGain_increment;
          }
        } else if ((numSpeakers == 2) && (numSignalsMixed == 1)) {
          /*Adjust for the changed speaker gain*/
          EarconGain2 = fMult(EarconGain2, (FIXP_DBL)speakerGains[0]);

          for (; i < LoopCounterValue; i++) {
            FIXP_SGL tco;

            tco = *EarcondDataPointer++;
            DecodedData1[i] = (fMult(DecodedData1[i], AttGain) >> AttGainShift) +
                              (fMult(tco, EarconGain2) >> EarconShift);
            tco = *EarcondDataPointer++;
            DecodedData1[i] += (fMult(tco, EarconGain2) >> EarconShift);
            AttGain += AttGain_increment;
          }
        }
        i = fMax(i, startPoint);
        LoopCounterValue = i + NewFrameSamples;

        if (j == 0) {
          /* Switch to gains of current frame */
          AttGain = earconDecoder->AttGain_old;
          AttGainShift = earconDecoder->AttGainShift_old;
          AttGain_increment = earconDecoder->AttGain_increment_old;

          EarconGain = earconDecoder->EarconGain;
          EarconShift = earconDecoder->EarconShift;
        }
        numSpeakers = config->m_numPcmSignals;
      }

      /*Left align buffer*/
      INT OverallSamplesUsed =
          LastFrameSamples * earconDecoder->numPcmSignals_old + NewFrameSamples * numSpeakers;
      earconDecoder->AccumulatedFrameSize -= OverallSamplesUsed;
      if (earconDecoder->AccumulatedFrameSize < 0) {
        earconDecoder->AccumulatedFrameSize = 0;
      }
      FDK_ASSERT((earconDecoder->AccumulatedFrameSize + OverallSamplesUsed) <= EARCON_BUFFER_SIZE);
      FDKmemmove(&earconDecoder->EarconData[0], &earconDecoder->EarconData[OverallSamplesUsed],
                 sizeof(FIXP_SGL) * earconDecoder->AccumulatedFrameSize);
      earconDecoder->numPcmSignals_old = numSpeakers;

      /* Save intermediate gain for next time to finish the current frame */
      earconDecoder->AttGain_intermediate = AttGain;
      earconDecoder->AttGainShift_intermediate = AttGainShift;
      earconDecoder->AttGain_increment_intermediate = earconDecoder->AttGain_increment_old;

      /* Pass new to old gains */
      earconDecoder->AttGain_old = earconDecoder->AttGain_new;
      earconDecoder->AttGainShift_old = earconDecoder->AttGainShift_new;
      earconDecoder->AttGain_increment_old = (FIXP_DBL)0;
    }

    earconDecoder->TruncationPresent = 0;
    if (config->TruncationFlag) {
      earconDecoder->TruncationPresent = 1;
      config->TruncationFlag = 0;
    }
  }

bail:

  return TRANSPORTDEC_OK;
}
