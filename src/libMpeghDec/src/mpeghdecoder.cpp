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

   Author(s):

   Description:

*******************************************************************************/

#include <math.h>

#include "FDK_cicp2geometry.h"
#include "aacdecoder_lib.h"
#include "deque.h"
#include "mpeghdecoder.h"

// The following threshold determines the maximally allowed time difference (in milliseconds) of
// two consecutively provided MPEG-H frames. If this threshold is exceeded the decoding process
// will be restarted, i.e. the decoding core will be reset to its initial start-up state.
#define THRESHOLD 200000000  // 200 milliseconds ~ 9.375 frames of 1024 samples at 48kHz

#define TIMESTAMP_ARRAY_SIZE (10)
#define FADE_ARRAY_SIZE (10)
#define NUM_FADE_SAMPLES_PER_CHANNEL (128)

#define MAX_NUM_FRAME_SAMPLES (3072)
#define MAX_NUM_OUTPUT_CHANNELS (24)

#define TOLERANCE (5)

#define DEFAULT_DRC_SETTING_TARGET_REFERENCE_LEVEL (96)
#define DEFAULT_DRC_SETTING_EFFECT_TYPE (0)
#define DEFAULT_DRC_SETTING_BOOST_FACTOR (127)
#define DEFAULT_DRC_SETTING_ATTENUATION_FACTOR (127)
#define DEFAULT_DRC_SETTING_ALBUM_MODE (0)

typedef struct AUInfo {
  int auSize;  // samples per channel
  bool concealed;
  int outputLoudness;
} AUInfo;

typedef struct OutputInfo {
  int size;  // samples * channel
  bool concealed;
  int outputLoudness;
} OutputInfo;

typedef struct MPEGH_DECODER_CONTEXT {
  int sampleRate;
  int numberOfChannels;
  int32_t cicpIndex;
  bool zeroSignal;

  HANDLE_AACDECODER mpeghdec;
  INT_PCM* tmpSamples;

  uint8_t* mhaConfig;
  uint32_t mhaConfigLength;

  deque timestampInQueue;
  deque timestampOutQueue;

  deque auInfoQueue;

  deque decodedSamplesQueue;
  deque outputInfoQueue;
  deque outputSamplesQueue;

  deque fadeoutIdxQueue;
  deque fadeinIdxQueue;

  uint64_t frameNumber;

  unsigned int maxDecoderOutputSamples;

  bool drcUpdate;
  /* Desired DRC values (set by user). */
  int desiredDrcTarget;
  int desiredDrcEffectType;
  int desiredDrcBoostFactor;
  int desiredDrcAttFactor;
  int desiredDrcAlbumMode;

  /* DRC values from last time. */
  int lastDrcTarget;
  int lastDrcEffectType;
  int lastDrcBoostFactor;
  int lastDrcAttFactor;
  int lastDrcAlbumMode;
} MPEGH_DECODER_CONTEXT;

/*
 * Method:    fade
 * called to fadein/fadeout a signal
 */
static void fade(int32_t* sample, int index, int fadelen, bool fadein);

/*
 * Method:    clearQueues
 * called to clear all queues
 */
static void clearQueues(HANDLE_MPEGH_DECODER_CONTEXT hCtx);

/*
 * Method:    restartDecoder
 * called to restart the decoder
 */
static MPEGH_DECODER_ERROR restartDecoder(HANDLE_MPEGH_DECODER_CONTEXT hCtx);

static void updateDrcSettings(HANDLE_MPEGH_DECODER_CONTEXT hCtx);

HANDLE_MPEGH_DECODER_CONTEXT mpeghdecoder_init(int32_t cicpSetup) {
  int dequeError = 0;
  AAC_DECODER_ERROR ErrorStatus;
  int numOutChannels = cicp2geometry_get_numChannels_from_cicp(cicpSetup);

  /* Check for allowed target layouts */
  if ((cicpSetup <= 0) || (cicpSetup == 8) || ((cicpSetup > 20) && (cicpSetup < 100)) ||
      ((cicpSetup > 351) && (cicpSetup < 400)) || (cicpSetup > 422)) {
    return NULL;
  }

  MPEGH_DECODER_CONTEXT* ctx = (MPEGH_DECODER_CONTEXT*)FDKcalloc(1, sizeof(MPEGH_DECODER_CONTEXT));
  if (ctx == NULL) {
    goto bail;
  }

  ctx->maxDecoderOutputSamples = numOutChannels * MAX_NUM_FRAME_SAMPLES;
  ctx->tmpSamples = (INT_PCM*)FDKcalloc(ctx->maxDecoderOutputSamples, sizeof(INT_PCM));
  if (ctx->tmpSamples == NULL) {
    goto bail;
  }

  ctx->mpeghdec = aacDecoder_Open(TT_MHAS_PACKETIZED, 1);
  if (ctx->mpeghdec == NULL) {
    goto bail;
  }

  ctx->mhaConfigLength = 0;
  ctx->mhaConfig = NULL;

  // set device target speaker setup according to CICP index
  ctx->cicpIndex = cicpSetup;
  ErrorStatus = aacDecoder_SetParam(ctx->mpeghdec, AAC_TARGET_LAYOUT_CICP, ctx->cicpIndex);
  if (ErrorStatus != AAC_DEC_OK) {
    goto bail;
  }

  // context variables initialization
  dequeError = deque_alloc(&ctx->timestampInQueue, TIMESTAMP_ARRAY_SIZE, sizeof(uint64_t));
  if (dequeError < 0) {
    goto bail;
  }
  dequeError = deque_alloc(&ctx->timestampOutQueue, TIMESTAMP_ARRAY_SIZE, sizeof(uint64_t));
  if (dequeError < 0) {
    goto bail;
  }
  dequeError = deque_alloc(&ctx->auInfoQueue, TIMESTAMP_ARRAY_SIZE, sizeof(AUInfo));
  if (dequeError < 0) {
    goto bail;
  }
  dequeError = deque_alloc(&ctx->decodedSamplesQueue,
                           TIMESTAMP_ARRAY_SIZE * ctx->maxDecoderOutputSamples, sizeof(INT_PCM));
  if (dequeError < 0) {
    goto bail;
  }
  dequeError = deque_alloc(&ctx->outputInfoQueue, TIMESTAMP_ARRAY_SIZE, sizeof(OutputInfo));
  if (dequeError < 0) {
    goto bail;
  }
  dequeError = deque_alloc(&ctx->outputSamplesQueue,
                           TIMESTAMP_ARRAY_SIZE * ctx->maxDecoderOutputSamples, sizeof(INT_PCM));
  if (dequeError < 0) {
    goto bail;
  }
  dequeError = deque_alloc(&ctx->fadeoutIdxQueue, FADE_ARRAY_SIZE, sizeof(int));
  if (dequeError < 0) {
    goto bail;
  }
  dequeError = deque_alloc(&ctx->fadeinIdxQueue, FADE_ARRAY_SIZE, sizeof(int));
  if (dequeError < 0) {
    goto bail;
  }

  ctx->sampleRate = -1;
  ctx->numberOfChannels = -1;

  ctx->zeroSignal = false;

  ctx->frameNumber = 0;

  ctx->drcUpdate = true;
  /* Desired DRC values (set by user). Initialized to default values to be
   * applied at first processing step. */
  ctx->desiredDrcTarget = DEFAULT_DRC_SETTING_TARGET_REFERENCE_LEVEL;
  ctx->desiredDrcEffectType = DEFAULT_DRC_SETTING_EFFECT_TYPE;
  ctx->desiredDrcBoostFactor = DEFAULT_DRC_SETTING_BOOST_FACTOR;
  ctx->desiredDrcAttFactor = DEFAULT_DRC_SETTING_ATTENUATION_FACTOR;
  ctx->desiredDrcAlbumMode = DEFAULT_DRC_SETTING_ALBUM_MODE;

  /* DRC values from last time. Initialized to values out of range so that above
   * default values are used */
  ctx->lastDrcTarget = -2;     /* not -1, as it is a valid parameter value */
  ctx->lastDrcEffectType = -2; /* not -1, as it is a valid parameter value */
  ctx->lastDrcBoostFactor = -1;
  ctx->lastDrcAttFactor = -1;
  ctx->lastDrcAlbumMode = -1;

  return ctx;

bail:
  mpeghdecoder_destroy(ctx);
  return NULL;
}

MPEGH_DECODER_ERROR mpeghdecoder_setMhaConfig(HANDLE_MPEGH_DECODER_CONTEXT hCtx,
                                              const uint8_t* config, uint32_t configSize) {
  if (hCtx == NULL || config == NULL) {
    return MPEGH_DEC_NULLPTR_ERROR;
  }
  if (configSize == 0) {
    return MPEGH_DEC_UNSUPPORTED_PARAM;
  }
  if (hCtx->mhaConfig != NULL) {
    FDKfree(hCtx->mhaConfig);
    hCtx->mhaConfig = NULL;
  }
  MPEGH_DECODER_ERROR retval = MPEGH_DEC_OK;
  hCtx->mhaConfigLength = configSize;
  hCtx->mhaConfig = (uint8_t*)FDKcalloc(hCtx->mhaConfigLength, sizeof(uint8_t));
  if (hCtx->mhaConfig == NULL) {
    return MPEGH_DEC_OUT_OF_MEMORY;
  }
  FDKmemcpy(hCtx->mhaConfig, config, configSize * sizeof(uint8_t));

  retval = restartDecoder(hCtx);
  return retval;
}

void mpeghdecoder_destroy(HANDLE_MPEGH_DECODER_CONTEXT hCtx) {
  if (hCtx == NULL) {
    return;
  }

  if (hCtx->mhaConfig != NULL) {
    FDKfree(hCtx->mhaConfig);
    hCtx->mhaConfig = NULL;
  }

  if (hCtx->tmpSamples != NULL) {
    FDKfree(hCtx->tmpSamples);
    hCtx->tmpSamples = NULL;
  }

  // destroy the MPEG-H decoder instance
  if (hCtx->mpeghdec != NULL) {
    aacDecoder_Close(hCtx->mpeghdec);
    hCtx->mpeghdec = NULL;
  }

  // free memory for queues
  deque_free(&hCtx->timestampInQueue);
  deque_free(&hCtx->timestampOutQueue);
  deque_free(&hCtx->auInfoQueue);
  deque_free(&hCtx->decodedSamplesQueue);
  deque_free(&hCtx->outputInfoQueue);
  deque_free(&hCtx->outputSamplesQueue);
  deque_free(&hCtx->fadeoutIdxQueue);
  deque_free(&hCtx->fadeinIdxQueue);

  FDKfree(hCtx);
  hCtx = NULL;
}

MPEGH_DECODER_ERROR mpeghdecoder_process(HANDLE_MPEGH_DECODER_CONTEXT hCtx, const uint8_t* inData,
                                         uint32_t inLength, uint64_t timestamp) {
  if (hCtx == NULL || inData == NULL) {
    return MPEGH_DEC_NULLPTR_ERROR;
  }
  if (inLength == 0) {
    return MPEGH_DEC_UNSUPPORTED_PARAM;
  }
  if (deque_full(&hCtx->timestampInQueue)) {
    return MPEGH_DEC_BUFFER_ERROR;
  }
  if (deque_space(&hCtx->decodedSamplesQueue) < hCtx->maxDecoderOutputSamples) {
    return MPEGH_DEC_BUFFER_ERROR;
  }

  bool isDone = false;
  unsigned int validBytes = inLength;
  bool decodingSuccessful = false;
  bool doConceal = false;

  // update the DRC settings if necessary
  updateDrcSettings(hCtx);

  CStreamInfo* p_si = aacDecoder_GetStreamInfo(hCtx->mpeghdec);

  // store the presentation timestamp associated with this MHAS frame; two
  // consecutive timestamps have to differ!
  if (deque_empty(&hCtx->timestampInQueue) ||
      *(uint64_t*)deque_back(&hCtx->timestampInQueue) != timestamp) {
    deque_push_back(&hCtx->timestampInQueue, &timestamp);
  }

  if (deque_size(&hCtx->timestampInQueue) > 1) {
    uint64_t b =
        *(uint64_t*)deque_at(&hCtx->timestampInQueue, deque_size(&hCtx->timestampInQueue) - 1);
    uint64_t a =
        *(uint64_t*)deque_at(&hCtx->timestampInQueue, deque_size(&hCtx->timestampInQueue) - 2);
    uint64_t duration = b - a;

    if (duration > THRESHOLD) {
      MPEGH_DECODER_ERROR retval = restartDecoder(hCtx);
      if (retval != MPEGH_DEC_OK) {
        return retval;
      }
      p_si = aacDecoder_GetStreamInfo(hCtx->mpeghdec);

      // store the presentation timestamp associated with this MHAS frame; two
      // consecutive timestamps have to differ! restartDecoder also cleared all
      // queues!
      deque_push_back(&hCtx->timestampInQueue, &timestamp);
    }
  }

  while (validBytes != 0) {
    bool concealed = false;
    isDone = false;

    // pass the MHAS frame to the MPEG-H decoder
    AAC_DECODER_ERROR err =
        aacDecoder_Fill(hCtx->mpeghdec, &inData, (uint32_t*)&inLength, &validBytes);

    if (err != AAC_DEC_OK) {
      deque_pop_back(&hCtx->timestampInQueue);  // remove timestamp as it was not
                                                // possible to fill in current AU
      return MPEGH_DEC_PROCESS_ERROR;
    }

    while (!isDone) {
      // run FDK decoding process
      UINT flags = 0;
      if (doConceal) {  // force conceal of the decoder
        concealed = true;
        flags |= AACDEC_CONCEAL;
      }
      err = aacDecoder_DecodeFrame(hCtx->mpeghdec, hCtx->tmpSamples, hCtx->maxDecoderOutputSamples,
                                   flags);
      doConceal = false;  // do not conceal anymore

      switch (err) {
        case AAC_DEC_OK:
          // get information for the decoded PCM
          if (p_si != NULL) {
            decodingSuccessful = true;

            if (hCtx->sampleRate == -1 && hCtx->numberOfChannels == -1) {
              hCtx->sampleRate = p_si->sampleRate;
              hCtx->numberOfChannels = p_si->numChannels;
            } else if (hCtx->sampleRate != p_si->sampleRate ||
                       hCtx->numberOfChannels != p_si->numChannels) {
              return MPEGH_DEC_NEEDS_RESTART;
            }
          }
          break;

        case AAC_DEC_NOT_ENOUGH_BITS:
        case AAC_DEC_TRANSPORT_SYNC_ERROR:

          if (decodingSuccessful) {
            // Do nothing.
            // This just indicates the end of a frame and new data has to be fed
            // into the decoder.
            isDone = true;
            break;
          }

          if (hCtx->sampleRate == -1 || hCtx->numberOfChannels == -1) {
            // The decoder has no valid output configuration.
            // Remove the queued presentation timestamp until first frame decoded
            // successfully and sampling rate and number of channels is known.
            deque_pop_front(&hCtx->timestampInQueue);
            isDone = true;
            break;
          }

          if (err == AAC_DEC_TRANSPORT_SYNC_ERROR) {
            // We ran into a sync error, but the decoder has a valid output
            // configuration. Therefore it is possible to obtain a force concealed
            // frame in next call to decodeFrame.
            doConceal = true;
          } else {
            // There are not enough bits, but the decoder has a valid output
            // configuration. Therefore we can end the loop and new data has to be
            // fed into the decoder.
            isDone = true;
          }
          break;

        case AAC_DEC_INTERMEDIATE_OK:
          err = AAC_DEC_OK;
          break;

        default:  // unhandled error codes
                  // we ran into a decoding error
          if (IS_DECODE_ERROR(err)) {
            isDone = true;
            concealed = true;
          } else {
            decodingSuccessful = false;
            deque_pop_front(&hCtx->timestampInQueue);
            isDone = true;
          }
          break;
      }  // end of switch(err)

      // if the output is valid, add the PCM samples to the sample queue
      if (IS_OUTPUT_VALID(err)) {
        if (p_si != NULL) {
          // add decoded PCM samples to the sample queue
          if (p_si->frameSize > 0) {
            deque_bulk_push_back(&hCtx->decodedSamplesQueue, hCtx->tmpSamples,
                                 p_si->frameSize * p_si->numChannels);
          }
          // add the MPEG-H AU size info received from decoder
          if (p_si->mpeghAUSize > 0) {
            AUInfo auInfo;
            auInfo.auSize = p_si->mpeghAUSize;
            auInfo.concealed = concealed;
            auInfo.outputLoudness = p_si->outputLoudness;

            deque_push_back(&hCtx->auInfoQueue, &auInfo);
          }
        }
      }
    }  // end of while(!isDone)
  }    // end of while(validBytes != 0)
  return MPEGH_DEC_OK;
}

MPEGH_DECODER_ERROR mpeghdecoder_processTimescale(HANDLE_MPEGH_DECODER_CONTEXT hCtx,
                                                  const uint8_t* inData, uint32_t inLength,
                                                  uint64_t timestamp, uint32_t timescale) {
  uint64_t timestampNs = (uint64_t)((double)timestamp * 1e9 / timescale + 0.5);
  return mpeghdecoder_process(hCtx, inData, inLength, timestampNs);
}

MPEGH_DECODER_ERROR
mpeghdecoder_getSamples(HANDLE_MPEGH_DECODER_CONTEXT hCtx, int32_t* outData, uint32_t outLength,
                        MPEGH_DECODER_OUTPUT_INFO* outInfo) {
  if (hCtx == NULL || outData == NULL || outInfo == NULL) {
    return MPEGH_DEC_NULLPTR_ERROR;
  }
  if (outLength < hCtx->maxDecoderOutputSamples) {
    return MPEGH_DEC_BUFFER_ERROR;
  }
  int durationSamples;
  int outNumSamples = 0;
  uint64_t duration = 0;
  MPEGH_DECODER_ERROR retVal = MPEGH_DEC_FEED_DATA;
  int i;
  unsigned int k;

  outInfo->numChannels = hCtx->numberOfChannels;
  outInfo->sampleRate = hCtx->sampleRate;
  outInfo->numSamplesPerChannel = 0;
  outInfo->pts = 0;
  outInfo->loudness = 0;
  outInfo->isConcealed = false;

  if (deque_size(&hCtx->timestampInQueue) < 2) {
    return MPEGH_DEC_FEED_DATA;
  }
  // calculate the duration if enough timestamps are available
  uint64_t b = *(uint64_t*)deque_at(&hCtx->timestampInQueue, 1);
  uint64_t a = *(uint64_t*)deque_at(&hCtx->timestampInQueue, 0);
  duration = b - a;
  durationSamples = (int)(((double)duration * hCtx->sampleRate / 1e9) + 0.5);

  int fadelen = NUM_FADE_SAMPLES_PER_CHANNEL * hCtx->numberOfChannels;

  // check whether a sample queue contains a complete PCM block to be sent to
  // for playback/postprocessing
  if (!deque_empty(&hCtx->auInfoQueue)) {
    int numDecodedSamples =
        ((AUInfo*)deque_front(&hCtx->auInfoQueue))->auSize * hCtx->numberOfChannels;

    if (numDecodedSamples > 0 &&
        deque_size(&hCtx->decodedSamplesQueue) >= (unsigned int)numDecodedSamples) {
      // enough samples available
      uint64_t pts = *(uint64_t*)deque_pop_front(&hCtx->timestampInQueue);
      AUInfo* info = (AUInfo*)deque_pop_front(&hCtx->auInfoQueue);
      int AUSize = info->auSize;
      bool concealed = info->concealed;
      int outputLoudnessTmp = info->outputLoudness;

      deque_push_back(&hCtx->timestampOutQueue, &pts);

      int numSamplesToRemove = 0;
      int numSamplesToAdd = 0;
      if (AUSize <= durationSamples + TOLERANCE && AUSize >= durationSamples - TOLERANCE) {
        // do nothing
      } else {
        if (AUSize > durationSamples + TOLERANCE) {
          // remove samples from queue
          numSamplesToRemove = (AUSize - durationSamples) * hCtx->numberOfChannels;
        } else {
          // add samples to queue
          numSamplesToAdd = (durationSamples - AUSize) * hCtx->numberOfChannels;
        }
      }

      if (hCtx->zeroSignal && !concealed) {
        // the previous samples were zero and now there is an unconcealed signal
        // available -> fade in set fadein start index
        unsigned int fadeInLength = deque_size(&hCtx->outputSamplesQueue);
        deque_push_back(&hCtx->fadeinIdxQueue, &fadeInLength);
        hCtx->zeroSignal = false;
      }

      if (!hCtx->zeroSignal) {
        // the previous samples were non zero
        if (numSamplesToAdd > 0) {
          // we need to add zero samples -> fade out
          // set fadeout start index
          int idx = (int)deque_size(&hCtx->outputSamplesQueue) + numDecodedSamples - fadelen;
          hCtx->zeroSignal = true;
          if (idx >= 0) {
            deque_push_back(&hCtx->fadeoutIdxQueue, &idx);
          }
        }
        if (numSamplesToRemove > 0) {
          // we need to remove samples -> fade out
          // set fadeout start index
          int idx = (int)deque_size(&hCtx->outputSamplesQueue) - 1 + numDecodedSamples -
                    numSamplesToRemove - fadelen;
          if (idx >= 0) {
            deque_push_back(&hCtx->fadeoutIdxQueue, &idx);
          }
        }
      }

      // copy from sample queue to outputSamplesQueue
      if (numDecodedSamples > 0) {
        unsigned int samplesToCopy = numDecodedSamples - numSamplesToRemove;
        deque_bulk_pop_front_copy(&hCtx->decodedSamplesQueue, hCtx->tmpSamples, numDecodedSamples);
        if (concealed && hCtx->zeroSignal) {
          // the previous samples were zero and the current signal is concealed
          // -> replace samples with zero samples
          deque_bulk_push_back_zeros(&hCtx->outputSamplesQueue, samplesToCopy);
        } else {
          deque_bulk_push_back(&hCtx->outputSamplesQueue, hCtx->tmpSamples, samplesToCopy);
        }
      }

      // adjust the number of decoded samples after removing samples
      numDecodedSamples -= numSamplesToRemove;

      if (numSamplesToAdd > 0) {
        // add zero samples
        deque_bulk_push_back_zeros(&hCtx->outputSamplesQueue, numSamplesToAdd);
        // adjust the number of decoded samples after adding zero samples
        numDecodedSamples += numSamplesToAdd;
      }

      // add number of samples to outputSampleSizeQueue
      OutputInfo decInfo;
      decInfo.size = numDecodedSamples;
      decInfo.concealed = concealed || (numSamplesToAdd > 0) || (numSamplesToRemove > 0);
      decInfo.outputLoudness = outputLoudnessTmp;
      deque_push_back(&hCtx->outputInfoQueue, &decInfo);

    }  // end of if (deque_size(&hCtx->decodedSamplesQueue) >= numDecodedSamples)
  }    // end of if (!deque_empty(&hCtx->auInfoQueue))

  // check if enough samples to output are available and apply fadein/fadeout if
  // necessary
  if (!deque_empty(&hCtx->outputInfoQueue)) {
    OutputInfo* info = (OutputInfo*)deque_front(&hCtx->outputInfoQueue);
    if (info->size > 0 && info->size > (int)deque_size(&hCtx->outputSamplesQueue)) {
      // not enough samples available to output a frame
      return MPEGH_DEC_FEED_DATA;
    }

    outNumSamples = info->size;

    // apply fadeout
    int numIndexesToRemove = 0;
    for (k = 0; k < deque_size(&hCtx->fadeoutIdxQueue); k++) {
      unsigned int index = *(unsigned int*)deque_front(&hCtx->fadeoutIdxQueue);
      if (index + fadelen < deque_size(&hCtx->outputSamplesQueue)) {
        for (i = 0; i < fadelen; i++) {
          fade((int32_t*)deque_at(&hCtx->outputSamplesQueue, index + i), i, fadelen, false);
        }
        numIndexesToRemove++;
      }
    }
    // remove used indexes from fadeoutIdxQueue
    deque_bulk_pop_front(&hCtx->fadeoutIdxQueue, numIndexesToRemove);

    // apply fadein
    numIndexesToRemove = 0;
    for (k = 0; k < deque_size(&hCtx->fadeinIdxQueue); k++) {
      unsigned int index = *(unsigned int*)deque_front(&hCtx->fadeinIdxQueue);
      if (index + fadelen < deque_size(&hCtx->outputSamplesQueue)) {
        for (i = 0; i < fadelen; i++) {
          fade((int32_t*)deque_at(&hCtx->outputSamplesQueue, index + i), i, fadelen, true);
        }
        numIndexesToRemove++;
      }
    }
    // remove used indexes from fadeinIdxQueue
    deque_bulk_pop_front(&hCtx->fadeinIdxQueue, numIndexesToRemove);

    uint64_t pts;
    int tmpFrameSize = outNumSamples / hCtx->numberOfChannels;
    // check if the samples fit into the output buffer
    if (tmpFrameSize > MAX_NUM_FRAME_SAMPLES) {
      // not all samples can be written to output buffer
      // split it to consecutive calls
      int newFrameSize = MAX_NUM_FRAME_SAMPLES;
      outNumSamples = newFrameSize * hCtx->numberOfChannels;
      info->size -= outNumSamples;
      uint64_t* tmpPts = (uint64_t*)deque_front(&hCtx->timestampOutQueue);
      pts = *tmpPts;
      *tmpPts += (uint64_t)((double)newFrameSize * 1e9 / hCtx->sampleRate + 0.5);
    } else {
      // all samples fit into output buffer
      pts = *(uint64_t*)deque_pop_front(&hCtx->timestampOutQueue);
      info = (OutputInfo*)deque_pop_front(&hCtx->outputInfoQueue);
    }

    // copy samples to outData
    deque_bulk_pop_front_copy(&hCtx->outputSamplesQueue, outData, outNumSamples);

    // adjust fadein indexes
    int size = deque_size(&hCtx->fadeinIdxQueue);
    for (i = 0; i < size; i++) {
      int idx = *(int*)deque_pop_front(&hCtx->fadeinIdxQueue);
      idx -= outNumSamples;
      if (idx >= 0) {
        deque_push_back(&hCtx->fadeinIdxQueue, &idx);
      }
    }

    // adjust fadeout indexes
    size = deque_size(&hCtx->fadeoutIdxQueue);
    for (i = 0; i < size; i++) {
      int idx = *(int*)deque_pop_front(&hCtx->fadeoutIdxQueue);
      idx -= outNumSamples;
      if (idx >= 0) {
        deque_push_back(&hCtx->fadeoutIdxQueue, &idx);
      }
    }

    outInfo->numSamplesPerChannel = outNumSamples / outInfo->numChannels;
    outInfo->pts = pts;
    outInfo->ticks = (uint64_t)((double)pts * outInfo->sampleRate / 1e9 + 0.5);
    outInfo->loudness = info->outputLoudness;
    outInfo->isConcealed = info->concealed;

    retVal = MPEGH_DEC_OK;
  }  // end of if (!deque_empty(&hCtx->outputInfoQueue))

  return retVal;
}

MPEGH_DECODER_ERROR
mpeghdecoder_flushAndGet(HANDLE_MPEGH_DECODER_CONTEXT hCtx) {
  if (hCtx == NULL) {
    return MPEGH_DEC_NULLPTR_ERROR;
  }
  if (deque_full(&hCtx->timestampInQueue)) {
    return MPEGH_DEC_BUFFER_ERROR;
  }
  if (deque_space(&hCtx->decodedSamplesQueue) < hCtx->maxDecoderOutputSamples) {
    return MPEGH_DEC_BUFFER_ERROR;
  }
  // flush the decoder
  AAC_DECODER_ERROR err = aacDecoder_DecodeFrame(hCtx->mpeghdec, hCtx->tmpSamples,
                                                 hCtx->maxDecoderOutputSamples, AACDEC_FLUSH);
  CStreamInfo* p_si = aacDecoder_GetStreamInfo(hCtx->mpeghdec);
  if (IS_OUTPUT_VALID(err) && p_si != NULL) {
    if (hCtx->sampleRate == -1 && hCtx->numberOfChannels == -1) {
      hCtx->sampleRate = p_si->sampleRate;
      hCtx->numberOfChannels = p_si->numChannels;
    } else if (hCtx->sampleRate != p_si->sampleRate ||
               hCtx->numberOfChannels != p_si->numChannels) {
      return MPEGH_DEC_NEEDS_RESTART;
    }
    // add decoded PCM samples to the sample queue
    if (p_si->frameSize > 0) {
      deque_bulk_push_back(&hCtx->decodedSamplesQueue, hCtx->tmpSamples,
                           p_si->frameSize * p_si->numChannels);
    }
    // add the MPEG-H AU size info received from decoder
    if (p_si->mpeghAUSize > 0) {
      AUInfo auInfo;
      auInfo.auSize = p_si->mpeghAUSize;
      auInfo.concealed = false;
      auInfo.outputLoudness = p_si->outputLoudness;
      deque_push_back(&hCtx->auInfoQueue, &auInfo);
    }

    // add the ending pts
    if (!deque_empty(&hCtx->timestampInQueue)) {
      uint64_t pts = *(uint64_t*)deque_back(&hCtx->timestampInQueue);
      int last_frame_size = p_si->frameSize;
      if (!deque_empty(&hCtx->auInfoQueue)) {
        AUInfo* auInfo = (AUInfo*)deque_back(&hCtx->auInfoQueue);
        last_frame_size = auInfo->auSize;
      }
      pts += (uint64_t)((double)last_frame_size * 1e9 / p_si->sampleRate + 0.5);
      deque_push_back(&hCtx->timestampInQueue, &pts);
    }
  }
  return MPEGH_DEC_OK;
}

MPEGH_DECODER_ERROR mpeghdecoder_flush(HANDLE_MPEGH_DECODER_CONTEXT hCtx) {
  if (hCtx == NULL) {
    return MPEGH_DEC_NULLPTR_ERROR;
  }
  return restartDecoder(hCtx);
}

MPEGH_DECODER_ERROR mpeghdecoder_setParam(HANDLE_MPEGH_DECODER_CONTEXT hCtx,
                                          MPEGH_DECODER_PARAMETER param, int value) {
  if (hCtx == NULL) {
    return MPEGH_DEC_NULLPTR_ERROR;
  }
  MPEGH_DECODER_ERROR result = MPEGH_DEC_OK;
  switch (param) {
    case MPEGH_DEC_PARAM_TARGET_REFERENCE_LEVEL:
      if ((value >= 40 && value <= 127)) {
        hCtx->desiredDrcTarget = value;
      } else {
        result = MPEGH_DEC_UNSUPPORTED_PARAM;
      }
      break;
    case MPEGH_DEC_PARAM_EFFECT_TYPE:
      if (value >= -1 && value <= 6) {
        hCtx->desiredDrcEffectType = value;
      } else {
        result = MPEGH_DEC_UNSUPPORTED_PARAM;
      }
      break;
    case MPEGH_DEC_PARAM_BOOST_FACTOR:
      if (value >= 0 && value <= 127) {
        hCtx->desiredDrcBoostFactor = value;
      } else {
        result = MPEGH_DEC_UNSUPPORTED_PARAM;
      }
      break;
    case MPEGH_DEC_PARAM_ATTENUATION_FACTOR:
      if (value >= 0 && value <= 127) {
        hCtx->desiredDrcAttFactor = value;
      } else {
        result = MPEGH_DEC_UNSUPPORTED_PARAM;
      }
      break;
    case MPEGH_DEC_PARAM_ALBUM_MODE:
      if (value == 0 || value == 1) {
        hCtx->desiredDrcAlbumMode = value;
      } else {
        result = MPEGH_DEC_UNSUPPORTED_PARAM;
      }
      break;
    default:
      result = MPEGH_DEC_UNSUPPORTED_PARAM;
      break;
  }

  if (result == MPEGH_DEC_OK) {
    hCtx->drcUpdate = true;
  }
  return result;
}

void fade(int32_t* sample, int index, int fadelen, bool fadein) {
  float factor = 0;
  if (fadein) {
    factor = (float)(-cos(M_PI * (float)index / (float)fadelen) + 1.0f) / 2.0f;
  } else {
    factor = (float)(cos(M_PI * (float)(index + 1) / (float)fadelen) + 1) / 2.0f;
  }

  *sample = (int32_t)(*sample * factor);
}

void clearQueues(HANDLE_MPEGH_DECODER_CONTEXT hCtx) {
  deque_clear(&hCtx->timestampInQueue);
  deque_clear(&hCtx->auInfoQueue);
  deque_clear(&hCtx->timestampOutQueue);
  deque_clear(&hCtx->decodedSamplesQueue);
  deque_clear(&hCtx->outputSamplesQueue);
  deque_clear(&hCtx->outputInfoQueue);
  deque_clear(&hCtx->fadeinIdxQueue);
  deque_clear(&hCtx->fadeoutIdxQueue);
}

MPEGH_DECODER_ERROR restartDecoder(HANDLE_MPEGH_DECODER_CONTEXT hCtx) {
  if (hCtx == NULL) {
    return MPEGH_DEC_NULLPTR_ERROR;
  }

  if (hCtx->mpeghdec != NULL) {
    aacDecoder_Close(hCtx->mpeghdec);
    hCtx->mpeghdec = NULL;
  }

  if (hCtx->mhaConfigLength > 0 && hCtx->mhaConfig != NULL) {
    hCtx->mpeghdec = aacDecoder_Open(TT_MHA_RAW, 1);
  } else {
    hCtx->mpeghdec = aacDecoder_Open(TT_MHAS_PACKETIZED, 1);
  }
  if (hCtx->mpeghdec == NULL) {
    return MPEGH_DEC_OUT_OF_MEMORY;
  }

  // set device target speaker setup according to CICP index
  AAC_DECODER_ERROR ErrorStatus =
      aacDecoder_SetParam(hCtx->mpeghdec, AAC_TARGET_LAYOUT_CICP, hCtx->cicpIndex);
  if (ErrorStatus != AAC_DEC_OK) {
    return MPEGH_DEC_UNSUPPORTED_PARAM;
  }

  // set an out-of-band config if it was provided
  if (hCtx->mhaConfigLength > 0 && hCtx->mhaConfig != NULL) {
    ErrorStatus = aacDecoder_ConfigRaw(hCtx->mpeghdec, &hCtx->mhaConfig, &hCtx->mhaConfigLength);
    if (ErrorStatus != AAC_DEC_OK) {
      return MPEGH_DEC_UNSUPPORTED_PARAM;
    }
  }

  clearQueues(hCtx);
  hCtx->zeroSignal = false;

  hCtx->sampleRate = -1;
  hCtx->numberOfChannels = -1;

  // force an update to the last desired DRC values after restarting the decoder
  hCtx->drcUpdate = true;
  if (hCtx->lastDrcTarget == hCtx->desiredDrcTarget) {
    hCtx->lastDrcTarget = -2;
  }
  if (hCtx->lastDrcEffectType == hCtx->desiredDrcEffectType) {
    hCtx->lastDrcEffectType = -2;
  }
  if (hCtx->lastDrcBoostFactor == hCtx->desiredDrcBoostFactor) {
    hCtx->lastDrcBoostFactor = -1;
  }
  if (hCtx->lastDrcAttFactor == hCtx->desiredDrcAttFactor) {
    hCtx->lastDrcAttFactor = -1;
  }
  if (hCtx->lastDrcAlbumMode == hCtx->desiredDrcAlbumMode) {
    hCtx->lastDrcAlbumMode = -1;
  }

  return MPEGH_DEC_OK;
}

void updateDrcSettings(HANDLE_MPEGH_DECODER_CONTEXT hCtx) {
  if (hCtx->drcUpdate) {
    AAC_DECODER_ERROR error;
    if (hCtx->desiredDrcTarget != hCtx->lastDrcTarget) {
      error = aacDecoder_SetParam(hCtx->mpeghdec, AAC_DRC_REFERENCE_LEVEL, hCtx->desiredDrcTarget);
      if (error == AAC_DEC_OK) {
        hCtx->lastDrcTarget = hCtx->desiredDrcTarget;
      }
    }

    if (hCtx->desiredDrcEffectType != hCtx->lastDrcEffectType) {
      error =
          aacDecoder_SetParam(hCtx->mpeghdec, AAC_UNIDRC_SET_EFFECT, hCtx->desiredDrcEffectType);
      if (error == AAC_DEC_OK) {
        hCtx->lastDrcEffectType = hCtx->desiredDrcEffectType;
      }
    }

    if (hCtx->desiredDrcBoostFactor != hCtx->lastDrcBoostFactor) {
      error =
          aacDecoder_SetParam(hCtx->mpeghdec, AAC_DRC_BOOST_FACTOR, hCtx->desiredDrcBoostFactor);
      if (error == AAC_DEC_OK) {
        hCtx->lastDrcBoostFactor = hCtx->desiredDrcBoostFactor;
      }
    }

    if (hCtx->desiredDrcAttFactor != hCtx->lastDrcAttFactor) {
      error = aacDecoder_SetParam(hCtx->mpeghdec, AAC_DRC_ATTENUATION_FACTOR,
                                  hCtx->desiredDrcAttFactor);
      if (error == AAC_DEC_OK) {
        hCtx->lastDrcAttFactor = hCtx->desiredDrcAttFactor;
      }
    }

    if (hCtx->desiredDrcAlbumMode != hCtx->lastDrcAlbumMode) {
      error = aacDecoder_SetParam(hCtx->mpeghdec, AAC_UNIDRC_ALBUM_MODE, hCtx->desiredDrcAlbumMode);
      if (error == AAC_DEC_OK) {
        hCtx->lastDrcAlbumMode = hCtx->desiredDrcAlbumMode;
      }
    }

    hCtx->drcUpdate = false;
  }
}
