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

/************************* MPEG-D DRC decoder library **************************

   Author(s):

   Description:

*******************************************************************************/

#include "drcDec_types.h"
#include "drcDec_gainDecoder.h"
#include "drcGainDec_multiband.h"
#include "drcGainDec_process.h"

#define E_TGAINSTEP 12

static DRC_ERROR _prepareLnbIndex(ACTIVE_DRC* pActiveDrc, const int channelOffset,
                                  const int drcChannelOffset, const int numChannelsProcessed,
                                  const int lnbPointer) {
  SCHAR g;
  int c;
  DRC_INSTRUCTIONS_UNI_DRC* pInst = &pActiveDrc->drcInst;

  /* channelOffset: start index of physical channels
     numChannelsProcessed: number of processed channels, physical channels and DRC channels
     channelOffset + drcChannelOffset: start index of DRC channels,
        i.e. the channel order referenced in pInst.sequenceIndex */

  /* sanity checks */
  if ((channelOffset + numChannelsProcessed) > 28) return DE_NOT_OK;

  if ((channelOffset + drcChannelOffset + numChannelsProcessed) > 2 * 28) return DE_NOT_OK;

  if ((channelOffset + drcChannelOffset) < 0) return DE_NOT_OK;

  /* prepare lnbIndexForChannel, a map of indices from each channel to its corresponding
   * linearNodeBuffer instance */
  for (c = channelOffset; c < channelOffset + numChannelsProcessed; c++) {
    if (pInst->drcSetId > 0) {
      int drcChannel = c + drcChannelOffset;
      /* fallback for configuration with more physical channels than DRC channels: reuse DRC gain of
         first channel. This is necessary for HE-AAC mono with stereo output */
      if (drcChannel >= pInst->drcChannelCount) drcChannel = 0;
      g = pActiveDrc->channelGroupForChannel[drcChannel];
      if ((g >= 0) && !pActiveDrc->channelGroupIsParametricDrc[g]) {
        pActiveDrc->lnbIndexForChannel[c][lnbPointer] =
            pActiveDrc->activeDrcOffset + pActiveDrc->gainElementForGroup[g];
      }
    }
  }

  return DE_OK;
}

static DRC_ERROR _interpolateDrcGain(const GAIN_INTERPOLATION_TYPE gainInterpolationType,
                                     const SHORT timePrev,      /* time0 */
                                     const SHORT tGainStep,     /* time1 - time0 */
                                     const SHORT start,         /* start sample of processing */
                                     const SHORT stop,          /* stop sample of processing */
                                     const SHORT stepsize,      /* processing hop size */
                                     const FIXP_DBL gainLeft,   /* gain at time0, e = 7 */
                                     const FIXP_DBL gainRight,  /* gain at time1, e = 7 */
                                     const FIXP_DBL slopeLeft,  /* slope at time0, e = 7 */
                                     const FIXP_DBL slopeRight, /* slope at time1, e = 7 */
                                     FIXP_DBL* buffer) {
  int n, n_buf;
  int start_modulo, start_offset;

  if (tGainStep < 0) {
    return DE_NOT_OK;
  }
  if (tGainStep == 0) {
    return DE_OK;
  }

  if ((gainLeft == FL2FXCONST_DBL(1.0f / (float)(1 << 7))) &&
      (gainRight == FL2FXCONST_DBL(1.0f / (float)(1 << 7))) && (slopeLeft == (FIXP_DBL)0) &&
      (slopeRight == (FIXP_DBL)0)) {
    /* gain of 1.0 throughout the section. Skip processing to keep bit-identical signal. */
    return DE_OK;
  }

  /* get start index offset and buffer index for downsampled interpolation */
  /* start_modulo = (start+timePrev)%stepsize; */ /* stepsize is a power of 2 */
  start_modulo = (start + timePrev) & (stepsize - 1);
  start_offset = (start_modulo ? stepsize - start_modulo : 0);
  /* n_buf = (start + timePrev + start_offset)/stepsize; */
  n_buf = (start + timePrev + start_offset) >> (15 - fixnormz_S(stepsize));

  { /* gainInterpolationType == GIT_LINEAR */
    LONG a, a_step;
    int i;
    /* runs: Number of gain applications on buffer.
       runs = ceil((stop - start - start_offset)/stepsize). This works for stepsize = 2^N only. */
    INT runs =
        (INT)(stop - start - start_offset + stepsize - 1) >> (30 - CountLeadingBits(stepsize));
    if (runs <= 0) return DE_OK;
    /* n_min: common headroom of gainRight and gainLeft, limited by 8 */
    INT n_min = fMin(fMin(CntLeadingZeros(gainRight), CntLeadingZeros(gainLeft)) - 1, 8);
    /* a: increment per sample for linear interpolation between gainLeft and gainRight */
    a = (LONG)((gainRight << n_min) - (gainLeft << n_min)) / tGainStep;
    if (runs > 1) {
      /* a_step: increment per step for linear interpolation */
      a_step = a * stepsize;
    }
    /* n: starting position of first gain application with respect to time0 */
    n = start + start_offset;
    /* a: linear interpolation gain of first gain application */
    a = a * n + (LONG)(gainLeft << n_min);
    buffer += n_buf;
#if defined(FUNCTION_interpolateDrcGain_func1)
    interpolateDrcGain_func1(buffer, a, a_step, n_min, runs);
#else
    /* n_min: scaling value to compensate for e = 7 of the gain, and for fMultDiv2 */
    n_min = 8 - n_min;
    for (i = 0; i < runs - 1; i++) {
      buffer[i] = fMultDiv2(buffer[i], (FIXP_DBL)a) << n_min;
      a += a_step;
    }
    for (; i < runs; i++) {
      buffer[i] = fMultDiv2(buffer[i], (FIXP_DBL)a) << n_min;
    }
#endif /* defined(FUNCTION_interpolateDrcGain_func1) */
  }
  return DE_OK;
}

static DRC_ERROR _processNodeSegments(
    const int frameSize, const GAIN_INTERPOLATION_TYPE gainInterpolationType, const int nNodes,
    const FIXP_DBL* pNodeLinGain, const SHORT* pNodeLinTime, const int offset, const SHORT stepsize,
    const FIXP_DBL nodeLinGainPrevious, /* the last node gain of the previous frame */
    const SHORT nodeLinTimePrevious,    /* the last node time of the previous frame */
    const FIXP_DBL channelGain,         /* e = 8 */
    FIXP_DBL* pChannelGainPrevious,     /* e = 8 */
    FIXP_DBL* buffer) {
  DRC_ERROR err = DE_OK;
  SHORT timePrev, duration, start, stop, time;
  int n;
  FIXP_DBL gainLin = FL2FXCONST_DBL(1.0f / (float)(1 << 7)), gainLinPrev; /* e = 7 */
  FIXP_DBL gainLinChan, gainLinChanPrev;                                  /* e = 7 */
  FIXP_DBL slopeLin = (FIXP_DBL)0, slopeLinPrev = (FIXP_DBL)0;            /* e = 7 */

  timePrev = nodeLinTimePrevious + offset;
  gainLinPrev = nodeLinGainPrevious;
  for (n = 0; n < nNodes; n++) {
    time = pNodeLinTime[n] + offset;
    duration = time - timePrev;
    gainLin = pNodeLinGain[n];

    /* skip over invalid sections with negative duration */
    if (duration < 0) continue;

    if ((timePrev >= (frameSize - 1)) || (time < 0)) { /* This segment (between previous and current
                                                          node) lies outside of this audio frame */
      timePrev = time;
      gainLinPrev = gainLin;
      continue;
    }

    /* start and stop are the boundaries of the region of this segment that lie within this audio
       frame. Their values are relative to the beginning of this segment. stop is the first sample
       that isn't processed any more. */
    start = fMax(-timePrev, 1);
    stop = fMin(time, (SHORT)(frameSize - 1)) - timePrev + 1;

    gainLinChanPrev = fMultNorm(gainLinPrev, 7, *pChannelGainPrevious, 8, 7);

    if (start == 1) { /* This is a new segment. Apply new channelGain. */
      gainLinChan = fMultNorm(gainLin, 7, channelGain, 8, 7);
      *pChannelGainPrevious = channelGain;
    } else { /* This is a segment already processed last frame. Needs to be completed with old
                channelGain. */
      gainLinChan = fMultNorm(gainLin, 7, *pChannelGainPrevious, 8, 7);
    }

    err = _interpolateDrcGain(gainInterpolationType, timePrev, duration, start, stop, stepsize,
                              gainLinChanPrev, gainLinChan, slopeLinPrev, slopeLin, buffer);
    if (err) return err;

    timePrev = time;
    gainLinPrev = gainLin;
  }
  return err;
}

/* process DRC on time-domain signal */
DRC_ERROR
processDrcTime(HANDLE_DRC_GAIN_DECODER hGainDec, const int activeDrcIndex,
               const int activeDrcLocation, const int delaySamples, const int channelOffset,
               const int drcChannelOffset, const int numChannelsProcessed,
               const int timeDataChannelOffset, FIXP_DBL* deinterleavedAudio) {
  DRC_ERROR err = DE_OK;
  int c, b, i;
  ACTIVE_DRC* pActiveDrc = &(hGainDec->activeDrc[activeDrcLocation][activeDrcIndex]);
  DRC_GAIN_BUFFERS* pDrcGainBuffers = &(hGainDec->drcGainBuffers);
  int lnbPointer = pDrcGainBuffers->lnbPointer, lnbIx;
  LINEAR_NODE_BUFFER* pLinearNodeBuffer = pDrcGainBuffers->linearNodeBuffer;
  LINEAR_NODE_BUFFER* pDummyLnb = &(pDrcGainBuffers->dummyLnb);
  int offset = 0;

  if (hGainDec->delayMode == DM_REGULAR_DELAY) {
    offset = hGainDec->frameSize;
  }

  if ((delaySamples + offset) >
      (NUM_LNB_FRAMES - 2) *
          hGainDec->frameSize) /* if delaySamples is too big, NUM_LNB_FRAMES should be increased */
    return DE_NOT_OK;

  err = _prepareLnbIndex(pActiveDrc, channelOffset, drcChannelOffset, numChannelsProcessed,
                         lnbPointer);
  if (err) return err;

  deinterleavedAudio += channelOffset * timeDataChannelOffset; /* apply channelOffset */

  /* signal processing loop */
  for (c = channelOffset; c < channelOffset + numChannelsProcessed; c++) {
    FIXP_DBL channelGain, *pChannelGainPrev;
    FIXP_DBL gainOne = FL2FXCONST_DBL(1.0f / (float)(1 << 8));
    if (activeDrcLocation == 0 && activeDrcIndex == hGainDec->channelGainActiveDrcIndex) {
      channelGain = hGainDec->channelGain[c];
      pChannelGainPrev = &(hGainDec->channelGainPrev[c]);
    } else {
      channelGain = gainOne;
      pChannelGainPrev = &gainOne;
    }

    b = 0;
    {
      LINEAR_NODE_BUFFER *pLnb, *pLnbPrevious;
      FIXP_DBL nodeGainPrevious;
      SHORT nodeTimePrevious;

      int lnbPointerDiff;
      /* get pointer to oldest linearNodes */
      lnbIx = lnbPointer + 1 - NUM_LNB_FRAMES;
      while (lnbIx < 0) lnbIx += NUM_LNB_FRAMES;

      /* Loop over all node buffers in linearNodeBuffer.
         All nodes which are not relevant for the current frame are sorted out inside
         _processNodeSegments. */
      for (i = 0; i < NUM_LNB_FRAMES - 1; i++) {
        /* Prepare previous node */
        if (pActiveDrc->lnbIndexForChannel[c][lnbIx] >= 0)
          pLnbPrevious = &(pLinearNodeBuffer[pActiveDrc->lnbIndexForChannel[c][lnbIx] + b]);
        else
          pLnbPrevious = pDummyLnb;
        nodeGainPrevious = pLnbPrevious->linearNodeGain[lnbIx][pLnbPrevious->nNodes[lnbIx] - 1];
        nodeTimePrevious = pLnbPrevious->linearNodeTime[lnbIx][pLnbPrevious->nNodes[lnbIx] - 1];
        nodeTimePrevious -= hGainDec->frameSize;

        /* Prepare current linearNodeBuffer instance */
        lnbIx++;
        if (lnbIx >= NUM_LNB_FRAMES) lnbIx = 0;

        /* if lnbIndexForChannel changes over time, use the old indices for smooth transitions */
        if (pActiveDrc->lnbIndexForChannel[c][lnbIx] >= 0)
          pLnb = &(pLinearNodeBuffer[pActiveDrc->lnbIndexForChannel[c][lnbIx] + b]);
        else /* lnbIndexForChannel = -1 means "no DRC processing", due to drcInstructionsIndex < 0,
                drcSetId < 0 or channel group < 0 */
          pLnb = pDummyLnb;

        /* number of frames of offset with respect to lnbPointer */
        lnbPointerDiff = i - (NUM_LNB_FRAMES - 2);

        err = _processNodeSegments(
            hGainDec->frameSize, pLnb->gainInterpolationType, pLnb->nNodes[lnbIx],
            pLnb->linearNodeGain[lnbIx], pLnb->linearNodeTime[lnbIx],
            lnbPointerDiff * hGainDec->frameSize + delaySamples + offset, 1, nodeGainPrevious,
            nodeTimePrevious, channelGain, pChannelGainPrev, deinterleavedAudio);
        if (err) return err;
      }
      deinterleavedAudio += timeDataChannelOffset; /* proceed to next channel */
    }
  }
  return DE_OK;
}

/* process DRC on subband-domain signal */
DRC_ERROR
processDrcSubband(HANDLE_DRC_GAIN_DECODER hGainDec, const int activeDrcIndex,
                  const int activeDrcLocation, const int delaySamples, const int channelOffset,
                  const int drcChannelOffset, const int numChannelsProcessed,
                  const int processSingleTimeslot, FIXP_DBL* deinterleavedAudioReal[],
                  FIXP_DBL* deinterleavedAudioImag[]) {
  DRC_ERROR err = DE_OK;
  int b, c, m, m_start, m_stop, s, i;
  FIXP_DBL gainSb;
  FIXP_DBL gainLr;
  OVERLAP_PARAMETERS(*pOverlap)[4] = hGainDec->overlap;
  ACTIVE_DRC* pActiveDrc = &(hGainDec->activeDrc[activeDrcLocation][activeDrcIndex]);
  DRC_INSTRUCTIONS_UNI_DRC* pInst = &pActiveDrc->drcInst;
  DRC_GAIN_BUFFERS* pDrcGainBuffers = &(hGainDec->drcGainBuffers);
  int activeDrcOffset = pActiveDrc->activeDrcOffset;
  int lnbPointer = pDrcGainBuffers->lnbPointer, lnbIx;
  LINEAR_NODE_BUFFER* pLinearNodeBuffer = pDrcGainBuffers->linearNodeBuffer;
  FIXP_DBL(*subbandGains)[4 * (1024 / 256)] = hGainDec->subbandGains;
  FIXP_DBL* dummySubbandGains = hGainDec->dummySubbandGains;
  SUBBAND_DOMAIN_MODE subbandDomainMode = hGainDec->subbandDomainSupported;
  int signalIndex = 0;
  int frameSizeSb = 0;
  int nDecoderSubbands;
  SHORT L = 0; /* L: downsampling factor */
  int offset = 0;
  FIXP_DBL *audioReal = NULL, *audioImag = NULL;

  if (hGainDec->delayMode == DM_REGULAR_DELAY) {
    offset = hGainDec->frameSize;
  }

  if ((delaySamples + offset) >
      (NUM_LNB_FRAMES - 2) *
          hGainDec->frameSize) /* if delaySamples is too big, NUM_LNB_FRAMES should be increased */
    return DE_NOT_OK;

  switch (subbandDomainMode) {
    case SDM_QMF64:
    case SDM_QMF71:
      /* QMF domain processing is not supported. */
      return DE_NOT_OK;
    case SDM_STFT256:
      nDecoderSubbands = SUBBAND_NUM_BANDS_STFT256;
      L = 256;
      /* analysisDelay = SUBBAND_ANALYSIS_DELAY_STFT256; */
      break;
    default:
      return DE_NOT_OK;
  }

  /* frameSizeSb = hGainDec->frameSize/L; */                 /* L is a power of 2 */
  frameSizeSb = hGainDec->frameSize >> (15 - fixnormz_S(L)); /* timeslots per frame */

  if ((processSingleTimeslot < 0) || (processSingleTimeslot >= frameSizeSb)) {
    m_start = 0;
    m_stop = frameSizeSb;
  } else {
    m_start = processSingleTimeslot;
    m_stop = m_start + 1;
  }

  err = _prepareLnbIndex(pActiveDrc, channelOffset, drcChannelOffset, numChannelsProcessed,
                         lnbPointer);
  if (err) return err;

  if (!pActiveDrc->subbandGainsReady) /* only for the first time per frame that processDrcSubband is
                                         called */
  {
    FIXP_DBL gainOne = FL2FXCONST_DBL(1.0f / (float)(1 << 8));
    UCHAR g;
    /* write subbandGains */
    for (g = 0; g < pInst->nDrcChannelGroups; g++) {
      for (b = 0; b < pActiveDrc->bandCountForChannelGroup[g]; b++) {
        LINEAR_NODE_BUFFER* pLnb =
            &(pLinearNodeBuffer[activeDrcOffset + pActiveDrc->gainElementForGroup[g] + b]);
        FIXP_DBL nodeGainPrevious;
        SHORT nodeTimePrevious;
        int lnbPointerDiff;

        for (m = 0; m < frameSizeSb; m++) {
          subbandGains[activeDrcOffset + g][b * frameSizeSb + m] =
              FL2FXCONST_DBL(1.0f / (float)(1 << 7));
        }

        lnbIx = lnbPointer - (NUM_LNB_FRAMES - 1);
        while (lnbIx < 0) lnbIx += NUM_LNB_FRAMES;

        /* Loop over all node buffers in linearNodeBuffer.
           All nodes which are not relevant for the current frame are sorted out inside
           _processNodeSegments. */
        for (i = 0; i < NUM_LNB_FRAMES - 1; i++) {
          /* Prepare previous node */
          nodeGainPrevious = pLnb->linearNodeGain[lnbIx][pLnb->nNodes[lnbIx] - 1];
          nodeTimePrevious = pLnb->linearNodeTime[lnbIx][pLnb->nNodes[lnbIx] - 1];
          nodeTimePrevious -= hGainDec->frameSize;

          lnbIx++;
          if (lnbIx >= NUM_LNB_FRAMES) lnbIx = 0;

          /* number of frames of offset with respect to lnbPointer */
          lnbPointerDiff = i - (NUM_LNB_FRAMES - 2);

          err = _processNodeSegments(
              hGainDec->frameSize, pLnb->gainInterpolationType, pLnb->nNodes[lnbIx],
              pLnb->linearNodeGain[lnbIx], pLnb->linearNodeTime[lnbIx],
              lnbPointerDiff * hGainDec->frameSize + delaySamples + offset - (L - 1) / 2, L,
              nodeGainPrevious, nodeTimePrevious, gainOne, &gainOne,
              &(subbandGains[activeDrcOffset + g][b * frameSizeSb]));
          if (err) return err;
        }
      }
    }
    pActiveDrc->subbandGainsReady = 1;
  }

  for (c = channelOffset; c < channelOffset + numChannelsProcessed; c++) {
    FIXP_DBL* thisSubbandGainsBuffer;
    SCHAR g;
    int bandCount = 1;
    if (pInst->drcSetId > 0)
      g = pActiveDrc->channelGroupForChannel[c + drcChannelOffset];
    else
      g = -1;

    audioReal = deinterleavedAudioReal[signalIndex];
    if (deinterleavedAudioImag != NULL) {
      audioImag = deinterleavedAudioImag[signalIndex];
    }

    if ((g >= 0) && !pActiveDrc->channelGroupIsParametricDrc[g]) {
      thisSubbandGainsBuffer = subbandGains[activeDrcOffset + g];
      bandCount = pActiveDrc->bandCountForChannelGroup[g];
    } else {
      thisSubbandGainsBuffer = dummySubbandGains;
    }

    for (m = m_start; m < m_stop; m++) {
      INT n_min = 8;
      if (bandCount > 1) { /* multiband DRC */
        FIXP_DBL gainSbs[MAX_NUM_SUBBANDS];
        /* normalize gainLr for keeping signal precision */
        for (b = 0; b < bandCount; b++) {
          gainLr = thisSubbandGainsBuffer[b * frameSizeSb + m];
          if (activeDrcLocation == 0 && activeDrcIndex == hGainDec->channelGainActiveDrcIndex) {
            gainLr = fMultNorm(gainLr, 7, hGainDec->channelGain[c], 8, 7);
          }
          n_min = fMin(CntLeadingZeros(gainLr) - 1, n_min);
        }
        for (b = bandCount;
             b > 0;) { /* count reverse as the last band contains the most nonzero overlapWeights */
          b--;
          gainLr = thisSubbandGainsBuffer[b * frameSizeSb + m];
          if (activeDrcLocation == 0 && activeDrcIndex == hGainDec->channelGainActiveDrcIndex) {
            gainLr = fMultNorm(gainLr, 7, hGainDec->channelGain[c], 8, 7);
          }
          gainLr <<= n_min;
          if (b == bandCount - 1) {
            for (s = 0; s < nDecoderSubbands; s++) {
              gainSbs[s] = fMult(pOverlap[g][b].overlapWeight[s], gainLr);
            }
            for (; s < MAX_NUM_SUBBANDS; s++) {
              gainSbs[s] = (FIXP_DBL)0;
            }
          } else {
            for (s = pOverlap[g][b].start_index; s < pOverlap[g][b].stop_index; s++) {
              gainSbs[s] += fMult(pOverlap[g][b].overlapWeight[s], gainLr);
            }
          }
        }
        n_min = 8 - n_min;
        if (deinterleavedAudioImag == NULL) { /* real and imaginary parts are interleaved */
          *audioReal = fMultDiv2(*audioReal, gainSbs[0]) << n_min;
          audioReal++;
          if (subbandDomainMode ==
              SDM_STFT256) { /* For STFT filterbank, the real value of the nyquist band is stored at
                                the second array entry */
            *audioReal = fMultDiv2(*audioReal, gainSbs[nDecoderSubbands - 1]) << n_min;
          } else {
            *audioReal = fMultDiv2(*audioReal, gainSbs[0]) << n_min;
          }
          audioReal++;
          for (s = 1; s < nDecoderSubbands; s++) {
            *audioReal = fMultDiv2(*audioReal, gainSbs[s]) << n_min;
            audioReal++;
            *audioReal = fMultDiv2(*audioReal, gainSbs[s]) << n_min;
            audioReal++;
          }
        } else {
          *audioReal = fMultDiv2(*audioReal, gainSbs[0]) << n_min;
          audioReal++;
          if (subbandDomainMode ==
              SDM_STFT256) { /* For STFT filterbank, the real value of the nyquist band is stored at
                                the second array entry */
            *audioImag = fMultDiv2(*audioImag, gainSbs[nDecoderSubbands - 1]) << n_min;
          } else {
            *audioImag = fMultDiv2(*audioImag, gainSbs[0]) << n_min;
          }
          audioImag++;
          for (s = 1; s < nDecoderSubbands; s++) {
            *audioReal = fMultDiv2(*audioReal, gainSbs[s]) << n_min;
            audioReal++;
            *audioImag = fMultDiv2(*audioImag, gainSbs[s]) << n_min;
            audioImag++;
          }
        }
      } else { /* single-band DRC */
        gainSb = thisSubbandGainsBuffer[m];
        if (activeDrcLocation == 0 && activeDrcIndex == hGainDec->channelGainActiveDrcIndex)
          gainSb = fMultNorm(gainSb, 7, hGainDec->channelGain[c], 8, 7);
        /* normalize gainSb for keeping signal precision */
        n_min = fMin(CntLeadingZeros(gainSb) - 1, n_min);
        gainSb <<= n_min;
        n_min = 8 - n_min;
        if (deinterleavedAudioImag == NULL) { /* real and imaginary parts are interleaved */
          for (s = 0; s < nDecoderSubbands; s++) {
            *audioReal = fMultDiv2(*audioReal, gainSb) << n_min;
            audioReal++;
            *audioReal = fMultDiv2(*audioReal, gainSb) << n_min;
            audioReal++;
          }
        } else {
          for (s = 0; s < nDecoderSubbands; s++) {
            *audioReal = fMultDiv2(*audioReal, gainSb) << n_min;
            audioReal++;
            *audioImag = fMultDiv2(*audioImag, gainSb) << n_min;
            audioImag++;
          }
        }
      }
    }
    signalIndex++;
  }
  return DE_OK;
}
