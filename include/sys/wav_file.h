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

/************************* System integration library **************************

   Author(s):   Eric Allamanche

   Description:

*******************************************************************************/

/** \file   wav_file.h
 *  \brief  Rudimentary WAVE file read/write support.
 *
 *  The WAVE file reader/writer is intented to be used in the codec's example
 *  framework for easily getting started with encoding/decoding. Therefore
 *  it serves mainly for helping quickly understand how a codec's API actually
 *  works.
 *  Being a WAVE file reader/writer with very basic functionality, it may not be
 *  able to read WAVE files that come with unusual configurations.
 *  Details on how to use the interface functions can be found in every
 *  (encoder/decoder) example framework.
 */

#ifndef WAV_FILE_H
#define WAV_FILE_H

#include "genericStds.h"
#include "mpeghexport.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SPEAKER_FRONT_LEFT 0x1
#define SPEAKER_FRONT_RIGHT 0x2
#define SPEAKER_FRONT_CENTER 0x4
#define SPEAKER_LOW_FREQUENCY 0x8
#define SPEAKER_BACK_LEFT 0x10
#define SPEAKER_BACK_RIGHT 0x20
#define SPEAKER_FRONT_LEFT_OF_CENTER 0x40
#define SPEAKER_FRONT_RIGHT_OF_CENTER 0x80
#define SPEAKER_BACK_CENTER 0x100
#define SPEAKER_SIDE_LEFT 0x200
#define SPEAKER_SIDE_RIGHT 0x400
#define SPEAKER_TOP_CENTER 0x800
#define SPEAKER_TOP_FRONT_LEFT 0x1000
#define SPEAKER_TOP_FRONT_CENTER 0x2000
#define SPEAKER_TOP_FRONT_RIGHT 0x4000
#define SPEAKER_TOP_BACK_LEFT 0x8000
#define SPEAKER_TOP_BACK_CENTER 0x10000
#define SPEAKER_TOP_BACK_RIGHT 0x20000
#define SPEAKER_RESERVED 0x80000000

#define NBITS_FLOAT (-32)
#define NBITS_DOUBLE (-64)

/*!
 * RF64 WAVE file struct, for being able to read WAV files >4GB.
 */
typedef struct Ds64Header {
  char chunkID[4];
  UINT chunkSize;
  UINT64 riffSize64;
  UINT64 dataSize64;
  UINT64 samplesPerCh64;
  UINT tableLength;
} Ds64Header;

#define WAV_HEADER_SIZE 80 /* including DS64Header */

/*!
 * RIFF WAVE file struct.
 * For details see WAVE file format documentation (for example at http://www.wotsit.org).
 */
typedef struct WAV_HEADER {
  char riffType[4];
  UINT riffSize;
  char waveType[4];
  char formatType[4];
  UINT formatSize;
  USHORT compressionCode;
  USHORT numChannels;
  UINT sampleRate;
  UINT bytesPerSecond;
  USHORT blockAlign;
  USHORT bitsPerSample;
  char dataType[4];
  UINT dataSize;
  Ds64Header ds64Chunk;
} WAV_HEADER;

struct WAV {
  WAV_HEADER header;
  FDKFILE* fp;
  UINT channelMask;
  UINT headerSize;
};

typedef struct WAV* HANDLE_WAV;

/**
 * \brief  Open a WAV file handle for reading.
 *
 * \param pWav      Pointer to a memory location, where a WAV handle is returned.
 * \param filename  File name to be opened.
 *
 * \return  0 on success and non-zero on failure.
 */
MPEGHDEC_EXPORT INT WAV_InputOpen(HANDLE_WAV* pWav, const char* filename);

/**
 * \brief  Read samples from a WAVE file. The samples are automatically re-ordered to the
 *         native host endianess and scaled to full scale of the INT_PCM type, from
 *         whatever BPS the WAVE file had specified in its header data.
 *
 *  \param wav           Handle of WAV file.
 *  \param sampleBuffer  Pointer to store audio data.
 *  \param numSamples    Desired number of samples to read.
 *  \param nBufBits      Size in bit of each audio sample of sampleBuffer.
 *
 *  \return  Number of samples actually read.
 */
MPEGHDEC_EXPORT INT WAV_InputRead(HANDLE_WAV wav, void* sampleBuffer, UINT numSamples,
                                  int nBufBits);

/**
 * \brief       Close a WAV file reading handle.
 * \param pWav  Pointer to a WAV file reading handle.
 */
MPEGHDEC_EXPORT void WAV_InputClose(HANDLE_WAV* pWav);

/**
 * \brief  Open WAV output/writer handle.
 *
 * \param pWav            Pointer to WAV handle to be returned.
 * \param outputFilename  File name of the file to be written to.
 * \param sampleRate      Desired samplerate of the resulting WAV file.
 * \param numChannels     Desired number of audio channels of the resulting WAV file.
 * \param bitsPerSample   Desired number of bits per audio sample of the resulting WAV file.
 *
 * \return  0: ok; -1: error
 */
MPEGHDEC_EXPORT INT WAV_OutputOpen(HANDLE_WAV* pWav, const char* outputFilename, INT sampleRate,
                                   INT numChannels, INT bitsPerSample);

/**
 * \brief  Write data to WAV file asociated to WAV handle.
 *
 * \param wav              Handle of WAV file
 * \param sampleBuffer     Pointer to audio samples, right justified integer values.
 * \param numberOfSamples  The number of individual audio sample valuesto be written.
 * \param nBufBits         Size in bits of each audio sample in sampleBuffer.
 * \param nSigBits         Amount of significant bits of each nBufBits in sampleBuffer.
 *
 * \return 0: ok; -1: error
 */
MPEGHDEC_EXPORT INT WAV_OutputWrite(HANDLE_WAV wav, void* sampleBuffer, UINT numberOfSamples,
                                    int nBufBits, int nSigBits);

/**
 * \brief       Close WAV output handle.
 * \param pWav  Pointer to WAV handle. *pWav is set to NULL.
 */
MPEGHDEC_EXPORT void WAV_OutputClose(HANDLE_WAV* pWav);

#ifdef __cplusplus
}
#endif

#endif /* WAV_FILE_H */
