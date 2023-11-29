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

#include <stdint.h>
#include <stdbool.h>

#ifndef MPEGH_DECODER_TIMING_HANDLING_WRAPPER
#define MPEGH_DECODER_TIMING_HANDLING_WRAPPER

/**
 * @file   mpeghdecoder.h
 * @brief  MPEG-H 3DA decoder library interface header file.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  MPEG-H decoder error codes.
 */
typedef enum {
  MPEGH_DEC_OK,                /*!< No error occured. */
  MPEGH_DEC_FEED_DATA,         /*!< No more output samples are available. Feed more input data. */
  MPEGH_DEC_NULLPTR_ERROR,     /*!< A NULL pointer was provided as a function parameter. */
  MPEGH_DEC_OUT_OF_MEMORY,     /*!< Heap returned NULL pointer. */
  MPEGH_DEC_BUFFER_ERROR,      /*!< The current input data could not be filled because the buffer
                                    is full or the size of a provided output buffer is too small. */
  MPEGH_DEC_PROCESS_ERROR,     /*!< The provided input data could not be processed. */
  MPEGH_DEC_UNSUPPORTED_PARAM, /*!< An unsupported parameter was provided as a function
                                    parameter. */
  MPEGH_DEC_NEEDS_RESTART      /*!< A restart of the decoder is necessary due to a change of the
                                    output configuration (samplerate, number of channels). */
} MPEGH_DECODER_ERROR;

/**
 * @brief  Options for handling of Dynamic Range Control (DRC) parameters.
 */
typedef enum {
  MPEGH_DEC_PARAM_TARGET_REFERENCE_LEVEL =
      0x0000, /*!< MPEG-D DRC: Target reference level / decoder target loudness.\n
                   Defines the level below full-scale (quantized in steps of 0.25 LU) to which the
                   output audio signal will be normalized to by the DRC module.\n
                   The parameter controls loudness normalization for MPEG-D DRC.
                   The valid values range from 40 (-10 LKFS) to 127 (-31.75 LKFS).\n
                   Example values:\n
                   124 (-31 LKFS) for audio/video receivers (AVR) or other devices allowing audio
                   playback with high dynamic range,\n
                    96 (-24 LKFS) for TV sets or equivalent devices (default),\n
                    64 (-16 LKFS) for mobile devices where the dynamic range of audio playback is
                   restricted. */
  MPEGH_DEC_PARAM_EFFECT_TYPE =
      0x0001, /*!< MPEG-D DRC: Request a DRC effect type for selection of a DRC set.\n
                   Supported indices are:\n
                   -1: DRC off. Completely disables MPEG-D DRC.\n
                    0: None (default). Disables MPEG-D DRC, but automatically enables DRC if
                       necessary to prevent clipping.\n
                    1: Late night\n
                    2: Noisy environment\n
                    3: Limited playback range\n
                    4: Low playback level\n
                    5: Dialog enhancement\n
                    6: General compression. Used for generally enabling MPEG-D DRC without
                       particular request. */
  MPEGH_DEC_PARAM_BOOST_FACTOR =
      0x0002, /*!< MPEG-D DRC: Scaling factor for boosting gain values.\n
                   Defines how the boosting DRC gains (conveyed in the
                   bitstream) will be applied to the decoded signal.
                   The valid values range from 0 (don't apply boost gains)
                   to 127 (fully apply boost gains). Default value is 127. */
  MPEGH_DEC_PARAM_ATTENUATION_FACTOR =
      0x0003, /*!< MPEG-D DRC: Scaling factor for attenuating gain values.\n
                   Same as ::MPEGH_DEC_PARAM_BOOST_FACTOR but for attenuating DRC gains. */
  MPEGH_DEC_PARAM_ALBUM_MODE =
      0x0004 /*!< MPEG-D DRC: Enable album mode.\n
                  0: Disabled (default),\n
                  1: Enabled.\n
                  Disabled album mode leads to application of gain sequences for fading in and out,
                  if provided in the bitstream.\n
                  Enabled album mode makes use of dedicated album loudness information, if provided
                  in the bitstream. */
} MPEGH_DECODER_PARAMETER;

typedef struct MPEGH_DECODER_CONTEXT*
    HANDLE_MPEGH_DECODER_CONTEXT; /*!< Pointer to a MPEG-H decoder instance. */

/**
 * @brief  This structure gives information about the currently decoded audio data. All fields are
 *         read-only.
 */
typedef struct MPEGH_DECODER_OUTPUT_INFO {
  int numSamplesPerChannel; /*!< The number of samples per channel of the decoded PCM audio
                                 signal. */
  int numChannels;          /*!< The number of audio channels of the decoded PCM audio signal. */
  int sampleRate;           /*!< The sample rate in Hz of the decoded PCM audio signal. */
  uint64_t pts;             /*!< The presentation timestamp in nano seconds of the decoded
                                 PCM audio signal. */
  int loudness;             /*!< Audio output loudness in steps of -0.25 LU.
                                 Range: 0 (0 LKFS) to 231 (-57.75 LKFS).\n
                                 A value of -1 indicates that no loudness metadata is present.\n
                                 If loudness normalization is active, the value corresponds
                                 to the target loudness value set with
                                 ::MPEGH_DEC_PARAM_TARGET_REFERENCE_LEVEL. */
  bool isConcealed;         /*!< Flag to signal if the decoded PCM audio signal is concealed. */
} MPEGH_DECODER_OUTPUT_INFO;

/**
 * @brief  Open an MPEG-H decoder instance.
 *
 * @param[in] cicpSetup The CICP index of the desired target layout.
 * @return              MPEG-H decoder handle.
 */
HANDLE_MPEGH_DECODER_CONTEXT mpeghdecoder_init(int32_t cicpSetup);

/**
 * @brief  Explicitly configure the decoder by passing the MHA config contained in a binary buffer.
 *         This is required for MPEG-H MHA format bitstreams which have no in-band config.
 *
 * @param[in] hCtx        MPEG-H decoder handle.
 * @param[in] config      Pointer to an unsigned char buffer containing the MHA configuration
 *                        buffer.
 * @param[in] configSize  Length of the configuration buffer in bytes.
 * @return                Error code.
 */
MPEGH_DECODER_ERROR mpeghdecoder_setMhaConfig(HANDLE_MPEGH_DECODER_CONTEXT hCtx,
                                              const uint8_t* config, uint32_t configSize);

/**
 * @brief  De-allocate all resources of an MPEG-H decoder instance.
 *
 * @param[in] hCtx  MPEG-H decoder handle.
 * @return          void.
 */
void mpeghdecoder_destroy(HANDLE_MPEGH_DECODER_CONTEXT hCtx);

/**
 * @brief  Fill MPEG-H decoder's internal input buffer with bitstream data from the external input
 *         buffer and adds the corresponding timestamp to a timestamp queue. Furthermore, it
 *         decodes as many frames as possible and pushes the decoded samples into a samples queue.
 *         The decoded PCM samples can then be obtained by calling the mpeghdecoder_getSamples()
 *         function.
 *
 * @param[in] hCtx       MPEG-H decoder handle.
 * @param[in] inData     Pointer to external input buffer.
 * @param[in] inLength   Size of external input buffer.
 * @param[in] timestamp  Presentation timestamp of the external input buffer (in nano seconds).
 * @return               Error code.
 */
MPEGH_DECODER_ERROR mpeghdecoder_process(HANDLE_MPEGH_DECODER_CONTEXT hCtx, const uint8_t* inData,
                                         uint32_t inLength, uint64_t timestamp);

/**
 * @brief  Get a decoded audio frame
 *
 * @param[in]  hCtx       MPEG-H decoder handle.
 * @param[out] outData    Pointer to external output buffer where the decoded PCM samples will be
 *                        stored into. Needs space to hold up to 3072 samples per rendered output
 *                        channel.
 * @param[in]  outLength  Size of external output buffer in samples. Needs space to hold up to 3072
 *                        samples per rendered output channel. The amount of valid bytes after
 *                        invocation, can be otained from the outInfo.
 * @param[out] outInfo    Pointer to an OUTPUT_INFO structure holding information about the current
 *                        output frame.
 * @return                Error code.
 */
MPEGH_DECODER_ERROR mpeghdecoder_getSamples(HANDLE_MPEGH_DECODER_CONTEXT hCtx, int32_t* outData,
                                            uint32_t outLength, MPEGH_DECODER_OUTPUT_INFO* outInfo);

/**
 * @brief  Flush the decoder and push the flushed PCM samples into a samples queue. The decoded PCM
 *         samples can then be obtained by calling the mpeghdecoder_getSamples() function. This
 *         function should only be used to trigger the output of all pending samples in the EOF/EOS
 *         case.
 *
 * @param[in] hCtx  MPEG-H decoder handle.
 * @return          Error code.
 */
MPEGH_DECODER_ERROR mpeghdecoder_flushAndGet(HANDLE_MPEGH_DECODER_CONTEXT hCtx);

/**
 * @brief  Flush the decoder and discard all pending samples in the samples queue. This function
 *         should be used in case of seeking operations.
 *
 * @param[in] hCtx  MPEG-H decoder handle.
 * @return          Error code.
 */
MPEGH_DECODER_ERROR mpeghdecoder_flush(HANDLE_MPEGH_DECODER_CONTEXT hCtx);

/**
 * @brief  Set decoder parameter.
 *
 * @param[in] hCtx   MPEG-H decoder handle.
 * @param[in] param  Parameter to be set.
 * @param[in] value  Parameter value.
 * @return           Error code.
 */
MPEGH_DECODER_ERROR mpeghdecoder_setParam(HANDLE_MPEGH_DECODER_CONTEXT hCtx,
                                          MPEGH_DECODER_PARAMETER param, int value);

#ifdef __cplusplus
}
#endif

#endif
