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

/******************* Library for basic calculation routines ********************

   Author(s):   Dirk Mahne, Arthur Tritthart

   Description:

*******************************************************************************/

#include "FDK_tools_rom.h"

#include "FDK_stftfilterbank_process.h"
#include "rfft.h"

#if defined(__arm__)
#include "arm/FDK_stftfilterbank_process_arm.cpp"
#endif

/**********************************************************************************************************************************/
/* static function declatations */
/**********************************************************************************************************************************/

/** Short time analysis using DFT.
    Analysis time input signals using DFT with 50 percent overlap.
    Comprises blocking of new and old time input samples, windowing and applying DFT.
    \param audioInputTime One frame of audio input data, size [2*frameSize]
    \param audioInputTimePrev audio data of previous iteration.
    \param auxVecOfTransformSize Auxiliary vector, size transformSize.
    \param audioOutputFreq DFT domain output, size [fftLength].
    \param fftSize FFT size.
    \param frameSize Frame size.
    \return none  */
static void filterbankAnalysisSTFT(const FIXP_DBL* audioInputTime, FIXP_DBL* audioInputTimePrev,
                                   FIXP_DBL* audioOutputFreq, const int fftSize,
                                   const int frameSize);

/**********************************************************************************************************************************/

/** Short time synthesis using inverse DFT.
    Synthesize frequency input signals using inverse DFT.
    \param audioInputOutput DFT domain input, size [fftSize]
                            Audio output buffer, size [fftSize]
    \param numAudioChans Number of channels.
    \param numFreqBands number of frequency bands.
    \return none  */
static void filterbankSynthesisSTFT(FIXP_DBL* audioInputOutput, const UINT fftLength);

/**********************************************************************************************************************************/

/** Windowing and Overlap/Add of audio frames.
    Windowing and 50 percent Overlap/Add of audio frames.
    \param audioInput audio input, size [fftSize]
    \param prevAudioInput Audio input of previous iteration [frameSize]
    \param audioOutput Audio output, [frameSize]
    \param fftSize FFT size.
    \return Returns 0 on success, otherwise 1.  */
static void filterbankOverlapAddAudioFrames(FIXP_DBL* audioInput, FIXP_DBL* prevAudioInput,
                                            FIXP_DBL* audioOutput, const UINT fftSize,
                                            const UINT frameSize);

/**********************************************************************************************************************************/
/**********************************************************************************************************************************/

INT processAnalysisSTFT(const FIXP_DBL* inputBuffer, FIXP_DBL* outputBuffer,
                        HANDLE_STFT_FILTERBANK h_stft_filterbank) {
  FDK_ASSERT(inputBuffer != NULL);
  FDK_ASSERT(outputBuffer != NULL);
  FDK_ASSERT(h_stft_filterbank != NULL);

  filterbankAnalysisSTFT(inputBuffer, h_stft_filterbank->prevAudioInput, outputBuffer,
                         h_stft_filterbank->stftFilterbankConfig.fftSize,
                         h_stft_filterbank->stftFilterbankConfig.frameSize);
  return 0;
}

/**********************************************************************************************************************************/

INT processSynthesisSTFT(FIXP_DBL* RESTRICT inputBuffer, FIXP_DBL* RESTRICT outputBuffer,
                         HANDLE_STFT_FILTERBANK h_stft_filterbank) {
  INT status = 0;

  filterbankSynthesisSTFT(inputBuffer, h_stft_filterbank->stftFilterbankConfig.fftSize);

  filterbankOverlapAddAudioFrames(inputBuffer, h_stft_filterbank->prevAudioInput, outputBuffer,
                                  h_stft_filterbank->stftFilterbankConfig.fftSize,
                                  h_stft_filterbank->stftFilterbankConfig.frameSize);

  return status;
}

/**********************************************************************************************************************************/
/* static function implementations */
/**********************************************************************************************************************************/

static void filterbankSineWindowingSTFT(const FIXP_DBL* RESTRICT audioInputTime,
                                        FIXP_DBL* RESTRICT audioInputTimePrev,
                                        FIXP_DBL* RESTRICT audioOutputFreq, UINT fftSize) {
  const FIXP_WTP* sinetab = FDKgetWindowSlope(fftSize / 2, 0 /* Shape = SINE */);

#ifdef FUNCTION_filterbankSineWindowingSTFT_func1
  filterbankSineWindowingSTFT_func1(audioInputTime, audioInputTimePrev, audioOutputFreq, fftSize,
                                    sinetab);
#else
  /* Apply windowing with a sine window in range ]0-180[ degrees */
  /* The table contains the range im=]0-45], re=]90-45] degrees */
  for (UINT j = 0; j < fftSize / 4; j++) {
    FIXP_DBL time0, time1;
    FIXP_WTP w = *sinetab++;
    audioOutputFreq[j] = fMult(audioInputTimePrev[j], w.v.im);
    audioOutputFreq[fftSize / 2 - 1 - j] = fMult(audioInputTimePrev[fftSize / 2 - 1 - j], w.v.re);
    time0 = audioInputTime[j];
    time1 = audioInputTime[fftSize / 2 - 1 - j];
    audioOutputFreq[fftSize / 2 + j] = fMult(time0, w.v.re);
    audioOutputFreq[fftSize - 1 - j] = fMult(time1, w.v.im);
    audioInputTimePrev[j] = time0;
    audioInputTimePrev[fftSize / 2 - 1 - j] = time1;
  }
#endif /* FUNCTION_filterbankSineWindowingSTFT_func1 */
}

/*
  Parameter description in case of MPEG-H
  ---------------------------------------
  const FIXP_DBL  *audioInputTime               256 time domain samples => current subframe
  inputBuffer FIXP_DBL        **audioInputTimePrev          512 samples, 2nd half contains last
  input    h_stft_filterbank->timeInputBufferPrev FIXP_DBL        *auxVecOfTransformSize        512
  (buffer is unused)                       h_stft_filterbank->auxBufferFftSize FIXP_DBL
  *audioOutputFreq              512 freq domain samples => current subframe  outputBuffer const int
  fftSize                       always 512 const int       frameSize                     always 256
*/

static void filterbankAnalysisSTFT(const FIXP_DBL* audioInputTime, FIXP_DBL* audioInputTimePrev,
                                   FIXP_DBL* audioOutputFreq, const int fftSize,
                                   const int frameSize) {
  FDK_ASSERT(audioInputTime != NULL);
  FDK_ASSERT(audioInputTimePrev != NULL);
  FDK_ASSERT(fftSize > 0);
  FDK_ASSERT(frameSize > 0);
  FDK_ASSERT(fftSize == 2 * frameSize);

  INT dummy_scale = 0;

  filterbankSineWindowingSTFT((const FIXP_DBL*)audioInputTime, audioInputTimePrev, audioOutputFreq,
                              fftSize);

  /* IRFFT */
  rfft(fftSize, (FIXP_DBL*)audioOutputFreq, sizeof(FIXP_DBL) * fftSize, &dummy_scale);
}

/**********************************************************************************************************************************/

static void filterbankSynthesisSTFT(FIXP_DBL* audioInputOutput, const UINT fftLength)

{
  INT dummy_scale = 0;
  irfft(fftLength, audioInputOutput, sizeof(FIXP_DBL) * fftLength, &dummy_scale);
}

/***********************************************************************************************************/

static void filterbankOverlapAddAudioFrames(FIXP_DBL* audioInput, FIXP_DBL* prevAudioInput,
                                            FIXP_DBL* audioOutput, const UINT fftSize,
                                            const UINT frameSize) {
  FDK_ASSERT(audioInput != NULL);
  FDK_ASSERT(prevAudioInput != NULL);
  FDK_ASSERT(audioOutput != NULL);
  FDK_ASSERT(frameSize > 0);
  FDK_ASSERT(fftSize > 0);
  FDK_ASSERT(fftSize == 2 * frameSize);

  FDK_ASSERT(fftSize ==
             512); /* shift by 8 below is only correct for this length, has to be fixed! */

  FIXP_DBL* RESTRICT audioInputTime = audioInput;
  FIXP_DBL* RESTRICT audioInputTimePrev = prevAudioInput;

  const FIXP_WTP* sinetab = FDKgetWindowSlope(fftSize / 2, 0 /* Shape = SINE */);

#ifdef FUNCTION_filterbankOverlapAddAudioFrames_func1
  filterbankOverlapAddAudioFrames_func1(audioInputTime, audioInputTimePrev, audioOutput, fftSize,
                                        sinetab);
#else
  /* Apply windowing with a sine window in range [0 till 180[ degrees upon data from audioInputTime
   */
  /* For first half, add windowing result to previous input, store all in output              */
  /* For 2nd half, store windowing result in previous input                                   */
  /* The table contains only the range ]0 till 45] degrees                                    */

  for (UINT j = 0; j < fftSize / 4; j++) {
    FIXP_WTP w = *sinetab++;
    audioOutput[j] += fAddSaturate(fMult(audioInputTime[j] << 8, w.v.im), audioInputTimePrev[j]);
    audioInputTimePrev[j] = fMult(audioInputTime[fftSize / 2 + j] << 8, w.v.re);

    audioOutput[fftSize / 2 - 1 - j] +=
        fAddSaturate(fMult(audioInputTime[fftSize / 2 - 1 - j] << 8, w.v.re),
                     audioInputTimePrev[fftSize / 2 - 1 - j]);
    audioInputTimePrev[fftSize / 2 - 1 - j] = fMult(audioInputTime[fftSize - 1 - j] << 8, w.v.im);
  }
#endif
  return;
}

/**********************************************************************************************************************************/
/**********************************************************************************************************************************/
