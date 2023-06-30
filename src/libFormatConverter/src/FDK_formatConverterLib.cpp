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

   Author(s):   Achim Kuntz, Alfonso Pino, Thomas Blender

   Description: format converter library

*******************************************************************************/

#include "FDK_formatConverterLib.h"

#include "FDK_formatConverter_constants.h"
#include "FDK_formatConverter_constants_stft.h"
#include "FDK_formatConverter_api.h"
#include "FDK_formatConverter_init.h"
#include "FDK_formatConverter_intern.h"
#include "FDK_formatConverter_process.h"
#include "FDK_stftfilterbank_api.h"

#include "FDK_core.h"
#include "gVBAPRenderer.h"

#include "FDK_formatConverter_activeDmx_stft.h"

static INT _checkConfiguration(IIS_FORMATCONVERTER_HANDLE self);

static INT _applyPartialVbapInputFallback(IIS_FORMATCONVERTER_INTERNAL* _p, UINT numUnknownInCh,
                                          INT* unknownInCh_vec);

static INT _applyFullVbapFallback(IIS_FORMATCONVERTER_INTERNAL* _p);

static INT _applyLfeRuleSet(CICP2GEOMETRY_CHANNEL_GEOMETRY* inputChannelGeo, UINT numInputChannels,
                            CICP2GEOMETRY_CHANNEL_GEOMETRY* outputChannelGeo,
                            UINT numOutputChannels, FIXP_DMX_H** dmxMtx);

static INT _initSTFT(IIS_FORMATCONVERTER_INTERNAL* _p);

/**********************************************/
/*                                            */
/*           Initialization  Methods          */
/*                                            */
/**********************************************/

INT IIS_FormatConverter_Create(IIS_FORMATCONVERTER_HANDLE* self, IIS_FORMATCONVERTER_MODE mode,
                               CICP2GEOMETRY_CHANNEL_GEOMETRY* outGeo, UINT numOutChannels,
                               UINT sampleRate, UINT frameSize) {
  INT err = 0;
  IIS_FORMATCONVERTER_INTERNAL* _p;

  /* Other frame sizes may need a special treatment.
     see processing in phase align. */
  switch (mode) {
    case IIS_FORMATCONVERTER_MODE_PASSIVE_FREQ_DOMAIN:
    case IIS_FORMATCONVERTER_MODE_ACTIVE_FREQ_DOMAIN_PHASE_ALIGN:
    case IIS_FORMATCONVERTER_MODE_CUSTOM_FREQ_DOMAIN_PHASE_ALIGN:
    case IIS_FORMATCONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT:
    case IIS_FORMATCONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT: {
      if (frameSize != 1024) {
        return err = 1;
      }
      break;
    }
    default: {
      break;
    }
  }

  if (((*self) = (IIS_FORMATCONVERTER_HANDLE)FDKcalloc(1, sizeof **self)) == NULL) {
    return err = 1;
  }
  if (((*self)->member = FDKcalloc(1, sizeof(IIS_FORMATCONVERTER_INTERNAL))) == NULL) {
    return err = 1;
  }
  _p = (IIS_FORMATCONVERTER_INTERNAL*)(*self)->member;

  /* Initialization: Total number of signals and format converter mode. */
  (*self)->numSignalsTotal = 0;
  (*self)->fCMode = mode;

  if (mode == IIS_FORMATCONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT ||
      mode == IIS_FORMATCONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT) {
    _p->stftNumErbBands = STFT_ERB_BANDS;
    _p->stftFrameSize = 256;
    _p->stftLength = _p->stftFrameSize * 2;
    _p->fcNumFreqBands = _p->stftLength / 2 + 1;
    _p->fcCenterFrequencies = f_bands_nrm_stft_256_erb_58;
    _p->stftErbFreqIdx = erb_freq_idx_256_58;
  }
  FDKmemcpy(_p->GVH, GVH, 13 * 6 * sizeof(FIXP_DBL));
  FDKmemcpy(_p->GVH_e, GVH_e, 13 * 6 * sizeof(INT));
  FDKmemcpy(_p->GVL, GVL, 13 * 6 * sizeof(FIXP_DBL));
  FDKmemcpy(_p->GVL_e, GVL_e, 13 * 6 * sizeof(INT));

  _p->frameSize = frameSize;

  _p->outChannelVbapFlag = 0;
  _p->inChannelVbapFlag = 0;
  _p->fcParams = NULL;
  _p->fcState = NULL;

  _p->fcInputFormat = FDK_FORMAT_CONVERTER_INPUT_FORMAT_LISTOFCHANNELS;
  _p->fcOutputFormat = FDK_FORMAT_CONVERTER_OUTPUT_FORMAT_LISTOFCHANNELS;

  _p->numTotalInputChannels = 0;
  _p->numOutputChannels = numOutChannels;
  _p->samplingRate = sampleRate;
  _p->channelVec = NULL;
  _p->unknownChannelVec = NULL;

  _p->STFT_headroom_prescaling = 0;

  if (mode == IIS_FORMATCONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT ||
      mode == IIS_FORMATCONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT) {
    _p->numTimeSlots = frameSize / _p->stftFrameSize;
  }

  _p->immersiveDownmixFlag = 0;

  _p->cicpLayoutIndex = -1;
  _p->dmxMatrixValid = 1;
  _p->amountOfAddedDmxMatricesAndEqualizers = 0;

  FDK_ASSERT((mode == IIS_FORMATCONVERTER_MODE_PASSIVE_TIME_DOMAIN) ||
             (mode == IIS_FORMATCONVERTER_MODE_PASSIVE_FREQ_DOMAIN) ||
             (mode == IIS_FORMATCONVERTER_MODE_ACTIVE_FREQ_DOMAIN_PHASE_ALIGN) ||
             (mode == IIS_FORMATCONVERTER_MODE_CUSTOM_FREQ_DOMAIN_PHASE_ALIGN) ||
             (mode == IIS_FORMATCONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT) ||
             (mode == IIS_FORMATCONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT));

  _p->mode = mode;

  FDK_ASSERT(outGeo != NULL);

  FDKmemcpy(_p->outChannelGeo, outGeo, (sizeof *outGeo) * numOutChannels);
  (*self)->numLocalSpeaker = numOutChannels;

  return err;
}

INT IIS_FormatConverter_Config_AddInputSetup(IIS_FORMATCONVERTER_HANDLE self,
                                             CICP2GEOMETRY_CHANNEL_GEOMETRY* geo, UINT numChannels,
                                             UINT channelOffset, int* channelId) {
  int numLfes = 0;
  IIS_FORMATCONVERTER_INTERNAL* _p = (IIS_FORMATCONVERTER_INTERNAL*)self->member;

  /* sanity check: check number of input channels */
  if ((_p->numTotalInputChannels + numChannels) > FDK_FORMAT_CONVERTER_MAX_INPUT_CHANNELS) {
    return -1;
  }

  /* sanity check: check number of LFE channels */
  for (UINT k = 0; k < _p->numTotalInputChannels; k++)
    if ((_p->inputChannelGeo[k]).LFE == 1) numLfes++;

  for (UINT k = 0; k < numChannels; k++)
    if ((geo[k]).LFE == 1) numLfes++;

  if (numLfes > FDK_FORMAT_CONVERTER_MAX_LFE) {
    return 1;
  }

  FDKmemcpy(&(_p->inputChannelGeo[_p->numTotalInputChannels]), geo, numChannels * sizeof *geo);

  _p->numTotalInputChannels += numChannels;

  _p->numInputChannels[_p->numInputChannelGroups] = numChannels;
  _p->inputChannelGroupOffsets[_p->numInputChannelGroups] = channelOffset;

  _p->numInputChannelGroups++;

  return 0;
}

void IIS_FormatConverter_Config_SetAES(IIS_FORMATCONVERTER_HANDLE self, UINT aes) {
  FDK_ASSERT(aes <= (UINT)7);
  self->aes = aes;
}

void IIS_FormatConverter_Config_SetPAS(IIS_FORMATCONVERTER_HANDLE self, UINT pas) {
  FDK_ASSERT(pas <= (UINT)7);
  self->pas = pas;
}

int IIS_FormatConverter_Config_SetImmersiveDownmixFlag(IIS_FORMATCONVERTER_HANDLE self,
                                                       UINT immersiveFlag) {
  IIS_FORMATCONVERTER_INTERNAL* _p = (IIS_FORMATCONVERTER_INTERNAL*)self->member;
  if (immersiveFlag != 0 && immersiveFlag != 1) return -1;
  if (_p == NULL) return -1;

  _p->immersiveDownmixFlag = immersiveFlag;

  return 0;
}

/* rendering3DTypeFlag */

int IIS_FormatConverter_Config_SetRendering3DTypeFlag(IIS_FORMATCONVERTER_HANDLE self,
                                                      UINT rendering3DtypeFlag) {
  IIS_FORMATCONVERTER_INTERNAL* _p = (IIS_FORMATCONVERTER_INTERNAL*)self->member;
  if (rendering3DtypeFlag != 0 && rendering3DtypeFlag != 1) return -1;
  if (_p == NULL) return -1;

  _p->rendering3DTypeFlag = rendering3DtypeFlag;

  return 0;
}

int IIS_FormatConverter_GetDelay(IIS_FORMATCONVERTER_HANDLE self, UINT* delay) {
  IIS_FORMATCONVERTER_INTERNAL* _p = (IIS_FORMATCONVERTER_INTERNAL*)self->member;
  /* stftFrameSize equals the delay caused by the FormatConverter.
     In case of IIS_FORMATCONVERTER_MODE_PASSIVE_TIME_DOMAIN stftFrameSize and delay are zero.
  */
  *delay = _p->stftFrameSize;
  return 0;
}
/**********************************************/
/*                                            */
/*      Start of data processing methods      */
/*                                            */
/**********************************************/

int IIS_FormatConverter_Open(IIS_FORMATCONVERTER_HANDLE self, INT* p_buffer, UINT buf_size) {
  int err = 0;
  UINT i;
  IIS_FORMATCONVERTER_INTERNAL* _p;

  int numChannels = self->numLocalSpeaker;
  UINT numLfes;
  AFC_FORMAT_CONVERTER_CHANNEL_ID channelIn_vec[FDK_FORMAT_CONVERTER_MAX_INPUT_CHANNELS];
  AFC_FORMAT_CONVERTER_CHANNEL_ID channelOut_vec[FDK_FORMAT_CONVERTER_MAX_OUTPUT_CHANNELS];
  UINT numUnknownOutCh = 0;
  UINT numUnknownInCh = 0;

  INT unknownOutCh_vec[FDK_FORMAT_CONVERTER_MAX_OUTPUT_CHANNELS];
  INT unknownInCh_vec[FDK_FORMAT_CONVERTER_MAX_INPUT_CHANNELS];

  _p = (IIS_FORMATCONVERTER_INTERNAL*)self->member;
  FDK_ASSERT(_p != NULL);

  if (buf_size < sizeof(INT) * (6 * IN_OUT_N + 4 * (IN_OUT_N * sizeof(FIXP_DMX_H)) / sizeof(INT) +
                                MAX_NUM_DMX_RULES * 8))
    return -1;

  _p->openSuccess = 0;
  _p->aes = self->aes;
  _p->pas = self->pas;

  /* match known channels in channel config */
  err = formatConverterMatchChConfig2Channels(_p->inputChannelGeo, 0, _p->numTotalInputChannels,
                                              channelIn_vec, &numUnknownInCh, unknownInCh_vec);

  if (0 != err) {
    goto FC_OPEN_CLEANUP_AND_RETURN;
  }

  if ((numUnknownInCh > 0) && (_p->fcOutputFormat != FDK_FORMAT_CONVERTER_OUTPUT_FORMAT_GENERIC)) {
    _p->inChannelVbapFlag = 1;
  }

  _p->numTotalInputChannels = 0;

  for (i = 0; i < _p->numInputChannelGroups; i++) {
    _p->numTotalInputChannels += _p->numInputChannels[i];
  }

  cicp2geometry_get_number_of_lfes(_p->outChannelGeo, self->numLocalSpeaker, &numLfes);

  /* Error can be ignored, because _p->cicpLayoutIndex is only required to detect target layout 5
   * and 6 for immersive rendering. */
  cicp2geometry_get_cicpIndex_from_geometry(_p->outChannelGeo, numChannels - numLfes, numLfes,
                                            &(_p->cicpLayoutIndex));

  /* OUTPUT: match known channels in channel config */
  err = formatConverterMatchChConfig2Channels(_p->outChannelGeo, 1, self->numLocalSpeaker,
                                              channelOut_vec, &numUnknownOutCh, unknownOutCh_vec);

  if (0 != err) {
    goto FC_OPEN_CLEANUP_AND_RETURN;
  }

  if (numUnknownOutCh > 0) {
    _p->fcInputFormat = FDK_FORMAT_CONVERTER_INPUT_FORMAT_LISTOFCHANNELS;
    _p->fcOutputFormat = FDK_FORMAT_CONVERTER_OUTPUT_FORMAT_LISTOFCHANNELS;
    _p->outChannelVbapFlag = 1;
  }

  /* apply channel config as output setup in format converter and clean up */
  if (_p->fcOutputFormat != FDK_FORMAT_CONVERTER_OUTPUT_FORMAT_GENERIC) {
    formatConverterSetInOutFormat(_p, 1, numChannels, channelOut_vec);
  }

  /* apply channel config as input setup in format converter and clean up */
  formatConverterSetInOutFormat(_p, 0, _p->numTotalInputChannels, channelIn_vec);

  err = formatConverterOpen(_p->mode, _p);
  if (err != 0) {
    goto FC_OPEN_CLEANUP_AND_RETURN;
  }

  if (_p->mode == IIS_FORMATCONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT ||
      _p->mode == IIS_FORMATCONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT) {
    err = _initSTFT(_p);
    if (err) {
      goto FC_OPEN_CLEANUP_AND_RETURN;
    }
  }

  err = formatConverterInit(_p, _p->fcCenterFrequencies, p_buffer);

  switch (err) {
    case 0: {
      /* Everything's fine */
      /* Generic/Immersive control code */
      formatConverterDmxMatrixControl(_p);
      /* Exponent calculation */
      formatConverterDmxMatrixExponent(_p);
      break;
    }
    case -2:
    case -8: {
      _p->fcInputFormat = FDK_FORMAT_CONVERTER_INPUT_FORMAT_GENERIC;
      _p->fcOutputFormat = FDK_FORMAT_CONVERTER_OUTPUT_FORMAT_GENERIC;
      _p->outChannelVbapFlag = 1;

      /* close, open, initialize for generic input/output setups: */
      formatConverterClose(_p);
      /*_p->fcParams->formatConverterMode = IIS_FORMATCONVERTER_MODE_CUSTOM_FREQ_DOMAIN;*/
      switch (_p->mode) {
        case IIS_FORMATCONVERTER_MODE_ACTIVE_FREQ_DOMAIN_PHASE_ALIGN:
        case IIS_FORMATCONVERTER_MODE_CUSTOM_FREQ_DOMAIN_PHASE_ALIGN: {
          _p->mode = IIS_FORMATCONVERTER_MODE_CUSTOM_FREQ_DOMAIN_PHASE_ALIGN;
          break;
        }
        case IIS_FORMATCONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT:
        case IIS_FORMATCONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT: {
          _p->mode = IIS_FORMATCONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT;
          break;
        }
        default:
          break;
      }

      err = formatConverterOpen(_p->mode, _p);
      if (err != 0) {
        goto FC_OPEN_CLEANUP_AND_RETURN;
      }
      _p->fcParams->genericIOFmt = 1;
      err = formatConverterInit(_p, _p->fcCenterFrequencies, p_buffer);
      if (err != 0) {
        goto FC_OPEN_CLEANUP_AND_RETURN;
      }
      break;
    }
    default: {
      goto FC_OPEN_CLEANUP_AND_RETURN;
    }
  }

  setCustomDownmixParameters(_p, self->aes, self->pas);

  {
    if (_p->outChannelVbapFlag) {
      err = _applyFullVbapFallback(_p);
      if (err) {
        goto FC_OPEN_CLEANUP_AND_RETURN;
      }
      /* Generic/Immersive control code */
      formatConverterDmxMatrixControl(_p);
      /* Exponent calculation */
      formatConverterDmxMatrixExponent(_p);
    }

    /*if( numUnknownCh > 0 )*/
    if (_p->inChannelVbapFlag && (!_p->outChannelVbapFlag)) {
      err = _applyPartialVbapInputFallback(_p, numUnknownInCh, unknownInCh_vec);
      if (err) {
        goto FC_OPEN_CLEANUP_AND_RETURN;
      }
      /* Generic/Immersive control code */
      formatConverterDmxMatrixControl(_p);
      /* Exponent calculation */
      formatConverterDmxMatrixExponent(_p);
    }
  }

FC_OPEN_CLEANUP_AND_RETURN:

  if (err == 0) {
    err = _checkConfiguration(self);
    if (err == 0) _p->openSuccess = 1;
    return err;
  } else {
    return err;
  }
}

/***************************************
 * FormatConverter with STFT Processing *
 ****************************************/
static int IIS_FormatConverter_Process_STFT(IIS_FORMATCONVERTER_HANDLE self,
                                            HANDLE_DRC_DECODER hDrcDec, FIXP_DBL** deinBuffer,
                                            FIXP_DBL** deoutBuffer) {
  UINT ch, i;

  IIS_FORMATCONVERTER_INTERNAL* _p;
  _p = (IIS_FORMATCONVERTER_INTERNAL*)self->member;

  for (i = 0; i < _p->numTimeSlots; i++) {
    int STFT_headroom = 31;
    int STFT_headroom_prescaling = 0;
    for (ch = 0; ch < _p->numTotalInputChannels; ch++) {
      STFT_headroom = fMin(STFT_headroom, getScalefactor(&(deinBuffer[ch][i * _p->stftFrameSize]),
                                                         (_p->stftFrameSize)));
    }
    for (ch = 0; ch < _p->numOutputChannels; ch++) {
      STFT_headroom = fMin(STFT_headroom, getScalefactor(&(deoutBuffer[ch][i * _p->stftFrameSize]),
                                                         (_p->stftFrameSize)));
    }

    /* Ensure 8 bit headroom */
    STFT_headroom_prescaling = fMax(STFT_headroom - 8, 0);
    /* Don't prescale more than necessary */
    STFT_headroom_prescaling = fMin(STFT_headroom_prescaling, 8);
    /* Ensure at least one bit headroom for FFT */
    if (STFT_headroom == 0) STFT_headroom_prescaling = -1;

    /* Store for use in  the format converter. The minimum is used, because the previous
       signal may be with less headroom than the current one and saturation may occur
       (the previous signal is overlapped with the current one) */
    _p->STFT_headroom_prescaling = fMin(_p->STFT_headroom_prescaling, STFT_headroom_prescaling);

    /* Transform into frequency domain*/
    for (ch = 0; ch < _p->numTotalInputChannels; ch++) {
      StftFilterbank_Process(&(deinBuffer[ch][i * _p->stftFrameSize]), _p->inputBufferStft[ch],
                             _p->stftFilterbankAnalysis[ch], _p->STFT_headroom_prescaling);
    }

    if (hDrcDec) {
      /* Delay of DRC gains: Delay of STFT Analysis (256) - 128 because MPEG-D DRC downsamples at
       * the middle of each timeslot*/
      FDK_drcDec_ProcessFreq(hDrcDec, 128, DRC_DEC_DRC1, 0, 0, _p->numTotalInputChannels, i,
                             _p->inputBufferStft, NULL);
    }

    formatConverter_process_STFT(_p);

    /* Transform into time domain */
    for (ch = 0; ch < _p->numOutputChannels; ch++) {
      StftFilterbank_Process(_p->outputBufferStft[ch], &(deoutBuffer[ch][i * (_p->stftFrameSize)]),
                             _p->stftFilterbankSynthesis[ch], _p->STFT_headroom_prescaling);
    }

    /* Keep the current signal headroom correction for use in the next frame */
    _p->STFT_headroom_prescaling = STFT_headroom_prescaling;
  }
  return 0;
}

INT IIS_FormatConverter_Process(IIS_FORMATCONVERTER_HANDLE self, HANDLE_DRC_DECODER hDrcDec,
                                FIXP_DBL* inBuffer, FIXP_DBL* outBuffer,
                                const int inputBufferChannelOffset) {
  IIS_FORMATCONVERTER_INTERNAL* _p;
  INT error = 0;

  _p = (IIS_FORMATCONVERTER_INTERNAL*)self->member;

  activeDownmixer* h = (activeDownmixer*)_p->fcState->handleActiveDmxStft;

  switch (_p->mode) {
    case IIS_FORMATCONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT: {
      FIXP_DBL* deinBuffer[FDK_FORMAT_CONVERTER_MAX_INPUT_CHANNELS];
      FIXP_DBL* deoutBuffer[FDK_FORMAT_CONVERTER_MAX_OUTPUT_CHANNELS];

      UINT frameSize = _p->frameSize;

      UINT i;

      deinBuffer[0] = inBuffer;
      for (i = 1; i < h->numInChans; i++) {
        deinBuffer[i] = &deinBuffer[i - 1][inputBufferChannelOffset];
      }

      deoutBuffer[0] = outBuffer;
      for (i = 1; i < h->numOutChans; i++) {
        deoutBuffer[i] = &deoutBuffer[i - 1][frameSize];
      }

      error = IIS_FormatConverter_Process_STFT(self, hDrcDec, deinBuffer, deoutBuffer);
    } break;
    case IIS_FORMATCONVERTER_MODE_PASSIVE_TIME_DOMAIN:
      formatConverterProcess_passive_timeDomain_frameLength(inBuffer, outBuffer, _p,
                                                            inputBufferChannelOffset);
      break;
    default:
      error = -1;
      break;
  } /* switch */

  return error;
}

int IIS_FormatConverter_Close(IIS_FORMATCONVERTER_HANDLE* self) {
  UINT i;

  if (self == NULL) return 0;

  IIS_FORMATCONVERTER_INTERNAL* _p = (IIS_FORMATCONVERTER_INTERNAL*)(*self)->member;
  if (_p == NULL) {
    FDKfree(*self);
    *self = NULL;
    return 0;
  }

  /*********  STFT **************/
  for (i = 0; i < _p->numTotalInputChannels; i++) {
    StftFilterbank_Close(&_p->stftFilterbankAnalysis[i]);
  }
  for (i = 0; i < _p->numOutputChannels; i++) {
    StftFilterbank_Close(&_p->stftFilterbankSynthesis[i]);
  }

  if (_p->prevInputBufferStft) {
    for (i = 0; i < (TFRA + 1); i++) {
      if (_p->prevInputBufferStft[i]) {
        FDKafree(_p->prevInputBufferStft[i]);
      }
    }
    FDKfree(_p->prevInputBufferStft);
  }

  if (_p->inputBufferStft) {
    for (i = 0; i < _p->numTotalInputChannels; i++) {
      if (_p->inputBufferStft[i]) {
        FDKafree(_p->inputBufferStft[i]);
      }
    }
    FDKfree(_p->inputBufferStft);
  }

  if (_p->outputBufferStft) {
    for (i = 0; i < _p->numOutputChannels; i++) {
      if (_p->outputBufferStft[i]) {
        FDKafree(_p->outputBufferStft[i]);
      }
    }
    FDKfree(_p->outputBufferStft);
  }
  formatConverterClose(_p);

  if (_p->channelVec) FDKfree(_p->channelVec);

  FDKfree(_p);

  FDKfree(*self);
  *self = NULL;

  return 0;
}

/**********************************************/
/*                                            */
/*              Private Methods               */
/*                                            */
/**********************************************/

static int _checkConfiguration(IIS_FORMATCONVERTER_HANDLE self) {
  switch (((IIS_FORMATCONVERTER_INTERNAL*)self->member)->mode) {
    case IIS_FORMATCONVERTER_MODE_PASSIVE_TIME_DOMAIN:
    case IIS_FORMATCONVERTER_MODE_PASSIVE_FREQ_DOMAIN: {
      self->aes = 0;
      self->pas = 0;
      break;
    }
    case IIS_FORMATCONVERTER_MODE_ACTIVE_FREQ_DOMAIN_PHASE_ALIGN: {
      self->aes = 7;
      self->pas = 3;
      break;
    }
    case IIS_FORMATCONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT:
    case IIS_FORMATCONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT:
    case IIS_FORMATCONVERTER_MODE_CUSTOM_FREQ_DOMAIN_PHASE_ALIGN: {
      if (self->aes < 0 || self->aes > 7) {
        return -1;
      }
      if (self->pas < 0 || self->pas > 7) {
        return -1;
      }
      break;
    }
    default: {
      return -1;
    }
  }

  return 0;
}

static INT _applyLfeRuleSet(CICP2GEOMETRY_CHANNEL_GEOMETRY* inputChannelGeo, UINT numInputChannels,
                            CICP2GEOMETRY_CHANNEL_GEOMETRY* outputChannelGeo,
                            UINT numOutputChannels, FIXP_DMX_H** dmxMtx) {
  UINT in, out;
  UINT numInLfes = 0;
  UINT numOutLfes = 0;

  cicp2geometry_get_number_of_lfes(inputChannelGeo, numInputChannels, &numInLfes);
  cicp2geometry_get_number_of_lfes(outputChannelGeo, numOutputChannels, &numOutLfes);

  if (numOutLfes == 0 || numInLfes == 0) {
    return 0;
  }

  if (numOutLfes == 1) {
    INT lfeIndex = -1;
    for (in = 0; in < numInputChannels; in++) {
      for (out = 0; out < numOutputChannels; out++) {
        if (inputChannelGeo[in].LFE || outputChannelGeo[out].LFE) {
          dmxMtx[in][out] = (FIXP_DMX_H)0;
        }
      }
    }

    for (out = 0; out < numOutputChannels; out++) {
      if (outputChannelGeo[out].LFE) lfeIndex = out;
    }

    for (in = 0; in < numInputChannels; in++) {
      if (inputChannelGeo[in].LFE) {
        dmxMtx[in][lfeIndex] = (FIXP_DMX_H)MAXVAL_DMX_H;
      }
    }
    return 0;

  } /* End: ( numOutLfes == 1 ) */

  return 0;
}

/*
 * @brief Creates the gVBAP DMX gains.
 * @param phgVBAPRenderer       Pointer to gVBAPRenderer handle
 * @param numObjects            Numer of Objects which have to be rendered
 * @param outGeometryInfo       Loudspeaker geometry information for output setting
 * @param outChannels           Number of real speaker including number of LFE
 */
static INT gVBAPRenderer_GetStaticGains(CICP2GEOMETRY_CHANNEL_GEOMETRY* inGeometryInfo,
                                        INT numObjects, INT outCICPIndex,
                                        CICP2GEOMETRY_CHANNEL_GEOMETRY* outGeometryInfo,
                                        INT outChannels, FIXP_DMX_H** staticGainsMatrix) {
  INT i, j, err = 0;
  HANDLE_GVBAPRENDERER hgVBAPRenderer = NULL; /* handle to gVBAP data structure                   */
  FIXP_DBL* gains = NULL;
  /* Initialization of the renderization parameters in order to create a new downmix matrix. */

  if (gVBAPRenderer_Open(&hgVBAPRenderer, numObjects, 1, 1, outGeometryInfo, outChannels,
                         outCICPIndex, 1, 1)) {
    err = 1; /* memory allocation error */
    goto bail;
  }

  /* Calculate inverse matrices */
  /* generateInverseMatrices(*phgVBAPRenderer); */

  if ((gains = (FIXP_DBL*)FDKcalloc(outChannels, sizeof(FIXP_DBL))) == NULL) {
    err = 1; /* memory allocation error */
    goto bail;
  }
  for (i = 0; i < numObjects; ++i) {
    OAM_SAMPLE sample = {};
    sample.sph.azi = FIXP_DBL((INT)((INT64)inGeometryInfo[i].Az * (INT)11930464));
    sample.sph.ele = FIXP_DBL((INT)(inGeometryInfo[i].El * (INT)11930464));
    sample.sph.rad = FL2FXCONST_DBL(1.0 / 16);
    sample.gain = (FIXP_DBL)MAXVAL_DBL;

    /* calculate cartesian stop position from spherical to cartesian */
    sample.cart = sphericalToCartesian(sample.sph); /* This does the conversion */
    /* calculate gains */
    calculateVbap(hgVBAPRenderer, sample, gains, 0); /* This will do the calculation */

    for (j = 0; j < outChannels; ++j) {
      int mappedChannel = 0;
      if (hgVBAPRenderer->speakerSetup.mapping) {
        mappedChannel = hgVBAPRenderer->speakerSetup.mapping[j];
      } else {
        mappedChannel = j;
      }
      staticGainsMatrix[i][mappedChannel] = FX_DBL2FX_DMX_H(gains[j]);
    }
  }

bail:
  if (hgVBAPRenderer) {
    gVBAPRenderer_Close(hgVBAPRenderer);
    hgVBAPRenderer = NULL;
  }

  if (gains) FDKfree(gains);

  return err;
}

static INT _applyPartialVbapInputFallback(IIS_FORMATCONVERTER_INTERNAL* _p, UINT numUnknownInCh,
                                          INT* unknownInCh_vec) {
  CICP2GEOMETRY_CHANNEL_GEOMETRY* unknownChannelsGeo = NULL;
  INT ch, err = 0;
  FIXP_DMX_H** tmpDmxMtx = NULL;
  FIXP_DMX_H** dmxMtx = NULL;

  if ((dmxMtx = (FIXP_DMX_H**)FDKcalloc(_p->numTotalInputChannels, sizeof *dmxMtx)) == NULL) {
    err = 1; /* memory allocation error */
    goto bail;
  }
  for (ch = 0; ch < (INT)_p->numTotalInputChannels; ch++) {
    if ((dmxMtx[ch] = (FIXP_DMX_H*)FDKcalloc(_p->numOutputChannels, sizeof *dmxMtx[ch])) == NULL) {
      err = 1; /* memory allocation error */
      goto bail;
    }
  }

  /* allocate temporary memory */
  if ((unknownChannelsGeo = (CICP2GEOMETRY_CHANNEL_GEOMETRY*)FDKcalloc(
           numUnknownInCh, sizeof *unknownChannelsGeo)) == NULL) {
    err = 1; /* memory allocation error */
    goto bail;
  }

  if ((tmpDmxMtx = (FIXP_DMX_H**)FDKcalloc(numUnknownInCh, sizeof *tmpDmxMtx)) == NULL) {
    err = 1; /* memory allocation error */
    goto bail;
  }
  for (ch = 0; ch < (INT)numUnknownInCh; ch++) {
    if ((tmpDmxMtx[ch] = (FIXP_DMX_H*)FDKcalloc(_p->numOutputChannels, sizeof *tmpDmxMtx[ch])) ==
        NULL) {
      err = 1; /* memory allocation error */
      goto bail;
    }
  }
  for (ch = 0; ch < (INT)numUnknownInCh; ch++) {
    unknownChannelsGeo[ch].Az = _p->inputChannelGeo[unknownInCh_vec[ch]].Az;
    unknownChannelsGeo[ch].El = _p->inputChannelGeo[unknownInCh_vec[ch]].El;
    unknownChannelsGeo[ch].LFE = _p->inputChannelGeo[unknownInCh_vec[ch]].LFE;
  }

  /* obtain gVBAP DMX gains: */
  err = gVBAPRenderer_GetStaticGains(unknownChannelsGeo, numUnknownInCh, -1, _p->outChannelGeo,
                                     _p->numOutputChannels, tmpDmxMtx);
  if (err != 0) {
    err = 1;
    goto bail;
  }

  /* copy gains for unknown channels into full-size DMX matrix */
  for (ch = 0; ch < (INT)numUnknownInCh; ch++) {
    INT j;
    for (j = 0; j < (INT)_p->numOutputChannels; j++) {
      dmxMtx[unknownInCh_vec[ch]][j] = tmpDmxMtx[ch][j];
    }
  }

  _applyLfeRuleSet(_p->inputChannelGeo, _p->numTotalInputChannels, _p->outChannelGeo,
                   _p->numOutputChannels, dmxMtx);

  /* post-process gains to reduce phantom sources */
  formatConverterPostprocessDmxMtx(dmxMtx, _p->numTotalInputChannels, _p->numOutputChannels);

  err = formatConverterAddDmxMtx(dmxMtx, _p);
  if (0 != err) {
    err = -1;
    goto bail;
  }

bail:
  /* free memory */
  if (unknownChannelsGeo != NULL) {
    FDKfree(unknownChannelsGeo);
    unknownChannelsGeo = NULL;
  }
  if (tmpDmxMtx != NULL) {
    for (ch = 0; ch < (INT)numUnknownInCh; ch++) {
      if (tmpDmxMtx[ch] != NULL) {
        FDKfree(tmpDmxMtx[ch]);
      }
    }
    FDKfree(tmpDmxMtx);
    tmpDmxMtx = NULL;
  }

  if (dmxMtx != NULL) {
    for (ch = 0; ch < (INT)_p->numTotalInputChannels; ch++) {
      if (dmxMtx[ch] != NULL) {
        FDKfree(dmxMtx[ch]);
      }
    }
    FDKfree(dmxMtx);
    dmxMtx = NULL;
  }

  return err;
}

static INT _applyFullVbapFallback(IIS_FORMATCONVERTER_INTERNAL* _p) {
  INT err = 0;
  FIXP_DMX_H** tmpDmxMtx = NULL;

  /* obtain gVBAP DMX gains: */
  err =
      gVBAPRenderer_GetStaticGains(_p->inputChannelGeo, _p->numTotalInputChannels, -1,
                                   _p->outChannelGeo, _p->numOutputChannels, _p->fcParams->dmxMtx);
  if (err != 0) {
    return err;
  }

  /* post-process gains to reduce phantom sources */
  formatConverterPostprocessDmxMtx(_p->fcParams->dmxMtx, _p->numTotalInputChannels,
                                   _p->numOutputChannels);

  if ((tmpDmxMtx = (FIXP_DMX_H**)FDKcalloc(_p->numTotalInputChannels, sizeof(tmpDmxMtx))) == NULL) {
    err = 1; /* memory allocation error */
    goto bail;
  }
  for (int ch = 0; ch < (INT)_p->numTotalInputChannels; ch++) {
    if ((tmpDmxMtx[ch] = (FIXP_DMX_H*)FDKcalloc(_p->numOutputChannels, sizeof(tmpDmxMtx[ch]))) ==
        NULL) {
      err = 1; /* memory allocation error */
      goto bail;
    }
  }

  _applyLfeRuleSet(_p->inputChannelGeo, _p->numTotalInputChannels, _p->outChannelGeo,
                   _p->numOutputChannels, tmpDmxMtx);

  err = formatConverterSetDmxMtx(_p);

bail:
  if (tmpDmxMtx != NULL) {
    for (int ch = 0; ch < (INT)_p->numTotalInputChannels; ch++) {
      if (tmpDmxMtx[ch] != NULL) {
        FDKfree(tmpDmxMtx[ch]);
      }
    }
    FDKfree(tmpDmxMtx);
    tmpDmxMtx = NULL;
  }

  return err;
}

INT FormatConverterFrame(IIS_FORMATCONVERTER_HANDLE FormatConverter, HANDLE_FDK_BITSTREAM hBitStr) {
  UINT rendering3DType = (UINT)FDKreadBit(hBitStr);

  IIS_FormatConverter_Config_SetRendering3DTypeFlag(FormatConverter, rendering3DType);

  return 1;
}

static INT _initSTFT(IIS_FORMATCONVERTER_INTERNAL* _p) {
  UINT ch;
  INT err, status = 0;
  _p->stftFilterbankConfigAnalysis.frameSize = _p->stftFrameSize;
  _p->stftFilterbankConfigAnalysis.fftSize = _p->stftLength;
  _p->stftFilterbankConfigAnalysis.stftFilterbankMode = STFT_FILTERBANK_MODE_TIME_TO_FREQ;

  FDKmemcpy(&_p->stftFilterbankConfigSynthesis, &_p->stftFilterbankConfigAnalysis,
            sizeof(_p->stftFilterbankConfigAnalysis));
  _p->stftFilterbankConfigSynthesis.stftFilterbankMode = STFT_FILTERBANK_MODE_FREQ_TO_TIME;

  /*Initialize Analysis filterbank*/
  _p->inputBufferStft = (FIXP_DBL**)FDKcalloc(_p->numTotalInputChannels, sizeof(FIXP_DBL*));
  if (_p->inputBufferStft == NULL) {
    return status = -1;
  }
  for (ch = 0; ch < _p->numTotalInputChannels; ch++) {
    _p->inputBufferStft[ch] =
        (FIXP_DBL*)FDKaalloc(_p->stftLength * sizeof(FIXP_DBL), ALIGNMENT_DEFAULT);
    if (_p->inputBufferStft[ch] == NULL) {
      return status = -1;
    }
  }

  /* Initialize prevInputBufferStft */
  _p->prevInputBufferStft = (FIXP_DBL**)FDKcalloc((TFRA + 1), sizeof(FIXP_DBL*));
  if (_p->prevInputBufferStft == NULL) {
    return status = -1;
  }
  for (ch = 0; ch < (TFRA + 1); ch++) {
    _p->prevInputBufferStft[ch] =
        (FIXP_DBL*)FDKaalloc(_p->stftLength * sizeof(FIXP_DBL), ALIGNMENT_DEFAULT);
    if (_p->prevInputBufferStft[ch] == NULL) {
      status = -1;
    }
  }

  for (ch = 0; ch < _p->numTotalInputChannels; ch++) {
    err = StftFilterbank_Open(&_p->stftFilterbankConfigAnalysis, &_p->stftFilterbankAnalysis[ch]);
    if (err) {
      return -1;
    }
  }

  /*Initialize Synthesis filterbank*/
  _p->outputBufferStft = (FIXP_DBL**)FDKcalloc(_p->numOutputChannels, sizeof(FIXP_DBL*));
  if (_p->outputBufferStft == NULL) {
    return status = -1;
  }
  for (ch = 0; ch < _p->numOutputChannels; ch++) {
    _p->outputBufferStft[ch] =
        (FIXP_DBL*)FDKaalloc(_p->stftLength * sizeof(FIXP_DBL), ALIGNMENT_DEFAULT);
    if (_p->outputBufferStft[ch] == NULL) {
      status = -1;
    }
  }

  for (ch = 0; ch < _p->numOutputChannels; ch++) {
    err = StftFilterbank_Open(&_p->stftFilterbankConfigSynthesis, &_p->stftFilterbankSynthesis[ch]);
    if (err) {
      return -1;
    }
  }
  return status;
}
