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

   Description: a rudimentary wav file interface

*******************************************************************************/

#include "wav_file.h"
#include "genericStds.h"

#include <stdlib.h>

static INT_PCM ulaw2pcm(UCHAR ulawbyte);

/*!
 *
 *  \brief Read header from a WAVEfile. Host endianess is handled accordingly.
 *  \wav->fp filepointer of type FILE*.
 *  \wavinfo SWavInfo struct where the decoded header info is stored into.
 *  \return 0 on success and non-zero on failure.
 *
 */
INT WAV_InputOpen(HANDLE_WAV* pWav, const char* filename) {
  HANDLE_WAV wav = (HANDLE_WAV)calloc(1, sizeof(struct WAV));
  INT offset;

  if (wav == NULL) {
    FDKprintfErr("WAV_InputOpen(): Unable to allocate WAV struct.\n");
    goto error;
  }

  wav->fp = FDKfopen(filename, "rb");
  if (wav->fp == NULL) {
    FDKprintfErr("WAV_InputOpen(): Unable to open wav file. %s\n", filename);
    goto error;
  }

  /* read RIFF-chunk */
  if (FDKfread(&(wav->header.riffType), 1, 4, wav->fp) != 4) {
    FDKprintfErr("WAV_InputOpen(): couldn't read RIFF_ID\n");
    goto error; /* bad error "couldn't read RIFF_ID" */
  }
  if (FDKstrncmp("RIFF", wav->header.riffType, 4) && FDKstrncmp("RF64", wav->header.riffType, 4)) {
    FDKprintfErr("WAV_InputOpen(): RIFF or RF64 descriptor not found.\n");
    goto error;
  }

  /* Read RIFF size. Ignored. */
  FDKfread_EL(&(wav->header.riffSize), 4, 1, wav->fp);

  /* read WAVE-chunk */
  if (FDKfread(&wav->header.waveType, 1, 4, wav->fp) != 4) {
    FDKprintfErr("WAV_InputOpen(): couldn't read format\n");
    goto error; /* bad error "couldn't read format" */
  }
  if (FDKstrncmp("WAVE", wav->header.waveType, 4)) {
    FDKprintfErr("WAV_InputOpen(): WAVE chunk ID not found.\n");
    goto error;
  }

  /* use ds64 */
  if (FDKstrncmp("RF64", wav->header.riffType, 4) == 0 && (wav->header.riffSize >= 0xFFFFFFFF)) {
    if (FDKfread(&(wav->header.ds64Chunk.chunkID), 1, 4, wav->fp) != 4) {
      FDKprintfErr("WAV_InputOpen(): DS64 descriptor not found.\n");
      goto error;
    }
    FDKfread_EL(&(wav->header.ds64Chunk.chunkSize), 4, 1, wav->fp);
    FDKfread_EL(&(wav->header.ds64Chunk.riffSize64), 8, 1, wav->fp);
    FDKfread_EL(&(wav->header.ds64Chunk.dataSize64), 8, 1, wav->fp);
    FDKfread_EL(&(wav->header.ds64Chunk.samplesPerCh64), 8, 1, wav->fp);
    FDKfread_EL(&(wav->header.ds64Chunk.tableLength), 4, 1, wav->fp);
  }

  /* read format-chunk */
  while (1) {
    if (FDKfread(&(wav->header.formatType), 1, 4, wav->fp) != 4) {
      FDKprintfErr("WAV_InputOpen(): couldn't read format_ID\n");
      goto error; /* bad error "couldn't read format_ID" */
    }

    if (FDKstrncmp("fmt", wav->header.formatType, 3)) {
      /* skip non fmt chunks */
      UINT chunkSize = 0;
      FDKfread_EL(&chunkSize, 4, 1, wav->fp);
      FDKfseek(wav->fp, chunkSize, FDKSEEK_CUR);
    } else {
      break;
    }
  }

  FDKfread_EL(&wav->header.formatSize, 4, 1,
              wav->fp); /* should be 16 for PCM-format (uncompressed) */

  /* read  info */
  FDKfread_EL(&(wav->header.compressionCode), 2, 1, wav->fp);
  if ((wav->header.compressionCode != 0x0001) && (wav->header.compressionCode != 0x0003) &&
      (wav->header.compressionCode != 0xFFFE)) {
    FDKprintfErr("WAV_InputOpen(): PCM-format (uncompressed) or extensible-format expected.\n");
    goto error;
  }
  FDKfread_EL(&(wav->header.numChannels), 2, 1, wav->fp);
  FDKfread_EL(&(wav->header.sampleRate), 4, 1, wav->fp);
  FDKfread_EL(&(wav->header.bytesPerSecond), 4, 1, wav->fp);
  FDKfread_EL(&(wav->header.blockAlign), 2, 1, wav->fp);
  FDKfread_EL(&(wav->header.bitsPerSample), 2, 1, wav->fp);
  if ((wav->header.compressionCode == 0x0001) &&
      ((wav->header.bitsPerSample != 8) && (wav->header.bitsPerSample != 12) &&
       (wav->header.bitsPerSample != 16) && (wav->header.bitsPerSample != 24) &&
       (wav->header.bitsPerSample != 32))) {
    FDKprintfErr(
        "WAV_InputOpen(): invalid value with given PCM-format (PCM) for bits per sample.\n");
    goto error;
  }
  if ((wav->header.compressionCode == 0x0003) && (wav->header.bitsPerSample != 32)) {
    FDKprintfErr(
        "WAV_InputOpen(): invalid value with given PCM-format (float) for bits per sample.\n");
    goto error;
  }
  if (wav->header.blockAlign != (wav->header.numChannels * ((wav->header.bitsPerSample + 7) / 8))) {
    FDKprintfErr("WAV_InputOpen(): invalid value for frame size.\n");
    goto error;
  }
  if (wav->header.bytesPerSecond != (wav->header.blockAlign * wav->header.sampleRate)) {
    FDKprintfErr("WAV_InputOpen(): invalid value for bytes per second.\n");
    goto error;
  }

  offset = wav->header.formatSize - 16;

  /* Wave format extensible */
  if (wav->header.compressionCode == 0xFFFE) {
    static const UCHAR guidPCM[16] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
                                      0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71};
    USHORT extraFormatBytes, validBitsPerSample;
    UCHAR guid[16];
    INT i;

    /* read extra bytes */
    FDKfread_EL(&(extraFormatBytes), 2, 1, wav->fp);
    offset -= 2;

    if (extraFormatBytes >= 22) {
      FDKfread_EL(&(validBitsPerSample), 2, 1, wav->fp);
      FDKfread_EL(&(wav->channelMask), 4, 1, wav->fp);
      FDKfread_EL(&(guid), 16, 1, wav->fp);

      /* check for PCM GUID */
      for (i = 1; i < 16; i++)
        if (guid[i] != guidPCM[i]) break;
      if (i == 16) {
        if (guid[0] == 0x01) wav->header.compressionCode = 0x01;
        if (guid[0] == 0x03) wav->header.compressionCode = 0x03;
      }

      offset -= 22;
    }
  }

  /* Skip rest of fmt header if any. */
  for (; offset > 0; offset--) {
    FDKfread(&wav->header.formatSize, 1, 1, wav->fp);
  }

  do {
    /* Read data chunk ID */
    if (FDKfread(wav->header.dataType, 1, 4, wav->fp) != 4) {
      FDKprintfErr("WAV_InputOpen(): Unable to read data chunk ID.\n");
      goto error;
    }

    /* Read chunk length. */
    FDKfread_EL(&offset, 4, 1, wav->fp);

    /* Check for data chunk signature. */
    if (FDKstrncmp("data", wav->header.dataType, 4) == 0) {
      wav->header.dataSize = offset;
      wav->headerSize = FDKftell(wav->fp);
      break;
    }
    /* Jump over non data chunk. */
    for (; offset > 0; offset--) {
      FDKfread(&(wav->header.dataSize), 1, 1, wav->fp);
    }
  } while (!FDKfeof(wav->fp));

  /* return success */
  *pWav = wav;
  return 0;

  /* Error path */
error:

  if (wav) {
    if (wav->fp) {
      FDKfclose(wav->fp);
      wav->fp = NULL;
    }
    free(wav);
  }

  *pWav = NULL;

  return -1;
}

/*!
 *
 *  \brief Read samples from a WAVEfile. The samples are automatically reorder to the native
 *    host endianess and scaled to full scale of the INT_PCM type, from whatever bps the WAVEfile
 *    had specified in its haader data.
 *
 *  \wav HANDLE_WAV of the wav file.
 *  \buffer Pointer to store read data.
 *  \numSamples Desired number of samples to read.
 *  \nBits sample size in bits to be used for the buffer
 *
 *  \return Number of samples actually read.
 *
 */

INT WAV_InputRead(HANDLE_WAV wav, void* buffer, UINT numSamples, int nBits) {
  UINT result = 0;
  UINT i;
  SCHAR* bptr = (SCHAR*)buffer;
  LONG* lptr = (LONG*)buffer;
  SHORT* sptr = (SHORT*)buffer;
  float* fptr = (float*)buffer;
  double* dptr = (double*)buffer;

  /* read until end of audio only, ignore possibly following non-audio data */
  if (FDKstrncmp("RF64", wav->header.riffType, 4) == 0) {
    if ((UINT64)(FDKftell64(wav->fp) + (wav->header.bitsPerSample >> 3) * numSamples) >
        wav->headerSize + wav->header.ds64Chunk.dataSize64) {
      if ((UINT64)FDKftell64(wav->fp) < wav->headerSize + wav->header.ds64Chunk.dataSize64) {
        numSamples = (UINT)((wav->headerSize + wav->header.ds64Chunk.dataSize64 -
                             (UINT64)FDKftell64(wav->fp)) /
                            (wav->header.bitsPerSample >> 3));
      } else {
        numSamples = 0;
      }
    }
  } else {
    if (FDKftell(wav->fp) + (wav->header.bitsPerSample >> 3) * numSamples >
        wav->headerSize + wav->header.dataSize) {
      if ((UINT)FDKftell(wav->fp) < wav->headerSize + wav->header.dataSize) {
        numSamples = (wav->headerSize + wav->header.dataSize - FDKftell(wav->fp)) /
                     (wav->header.bitsPerSample >> 3);
      } else {
        numSamples = 0;
      }
    }
  }

  switch (wav->header.compressionCode) {
    case 0x01: /* PCM uncompressed */
      if (nBits == wav->header.bitsPerSample) {
        result = FDKfread_EL(buffer, wav->header.bitsPerSample >> 3, numSamples, wav->fp);
      } else {
        result = 0;
        for (i = 0; i < numSamples; i++) {
          LONG tmp = 0;
          result += FDKfread_EL(&tmp, wav->header.bitsPerSample >> 3, 1, wav->fp);

          /* Move read bits to lower bits of LONG. */
          if (!IS_LITTLE_ENDIAN() && wav->header.bitsPerSample != 24 &&
              wav->header.bitsPerSample < 32) {
            tmp >>= (32 - wav->header.bitsPerSample);
          }

          if (nBits == NBITS_FLOAT) {
            *fptr++ = (float)(tmp << (32 - wav->header.bitsPerSample)) / 0x80000000U;
          } else if (nBits == NBITS_DOUBLE) {
            *dptr++ = (double)(tmp << (32 - wav->header.bitsPerSample)) / 0x80000000U;
          } else {
            /* Full scale */
            if (wav->header.bitsPerSample > nBits)
              tmp >>= (wav->header.bitsPerSample - nBits);
            else
              tmp <<= (nBits - wav->header.bitsPerSample);

            if (nBits == 8) *bptr++ = (SCHAR)tmp;
            if (nBits == 16) *sptr++ = (SHORT)tmp;
            if (nBits == 32) *lptr++ = (LONG)tmp;
          }
        }
      }
      break;

    case 0x03: /* IEEE float */
      if ((wav->header.bitsPerSample != 32) && (wav->header.bitsPerSample != 64)) {
        FDKprintf("WAV_InputRead(): unsupported format!!");
        break;
      }

      if (((nBits == NBITS_FLOAT) && (wav->header.bitsPerSample == 32)) ||
          ((nBits == NBITS_DOUBLE) && (wav->header.bitsPerSample == 64))) {
        result = FDKfread_EL(buffer, wav->header.bitsPerSample >> 3, numSamples, wav->fp);
      } else {
        result = 0;
        for (i = 0; i < numSamples; i++) {
          double tmp = 0;

          if (wav->header.bitsPerSample == 32) {
            float f = 0;
            result += FDKfread_EL(&f, 4, 1, wav->fp);
            tmp = (double)f;
          } else {
            result += FDKfread_EL(&tmp, 8, 1, wav->fp);
          }

          if (nBits == NBITS_FLOAT) {
            *fptr++ = (float)tmp;
          } else if (nBits == NBITS_DOUBLE) {
            *dptr++ = tmp;
          } else {
            double scl = (double)(1U << (nBits - 1));

            tmp *= scl;

            if (tmp >= 0)
              tmp += 0.5;
            else
              tmp -= 0.5;

            if (tmp >= scl) tmp = scl - 1;
            if (tmp < -scl) tmp = -scl;

            switch (nBits) {
              case 8:
                *bptr++ = (SCHAR)tmp;
                break;
              case 16:
                *sptr++ = (SHORT)tmp;
                break;
              case 32:
                *lptr++ = (LONG)tmp;
                break;
              default:
                return 0;
            }
          }
        }
      }
      break;

    case 0x07: /* u-Law compression */
      for (i = 0; i < numSamples; i++) {
        result += FDKfread(&(bptr[i << 1]), 1, 1, wav->fp);
        sptr[i] = (SHORT)ulaw2pcm(bptr[i << 1]);
      }
      break;

    default:
      FDKprintf("WAV_InputRead(): unsupported data-compression!!");
      break;
  }
  return result;
}

void WAV_InputClose(HANDLE_WAV* pWav) {
  HANDLE_WAV wav = *pWav;

  if (wav != NULL) {
    if (wav->fp != NULL) {
      FDKfclose(wav->fp);
      wav->fp = NULL;
    }
    free(wav);
  }
  *pWav = NULL;
}

/* conversion of u-law to linear coding */
static INT_PCM ulaw2pcm(UCHAR ulawbyte) {
  static const INT exp_lut[8] = {0, 132, 396, 924, 1980, 4092, 8316, 16764};
  INT sign, exponent, mantissa, sample;

  ulawbyte = (UCHAR)~ulawbyte;
  sign = (ulawbyte & 0x80);
  exponent = (ulawbyte >> 4) & 0x07;
  mantissa = ulawbyte & 0x0F;

  sample = exp_lut[exponent] + (mantissa << (exponent + 3));
  if (sign != 0) sample = -sample;

  return (INT_PCM)sample;
}

/************** Writer ***********************/

static UINT64 LittleEndian64(UINT64 v) {
  if (IS_LITTLE_ENDIAN())
    return v;
  else
    return (v & 0x00000000000000FFLL) << 56 | (v & 0x000000000000FF00LL) << 40 |
           (v & 0x0000000000FF0000LL) << 24 | (v & 0x00000000FF000000LL) << 8 |
           (v & 0x000000FF00000000LL) >> 8 | (v & 0x0000FF0000000000LL) >> 24 |
           (v & 0x00FF000000000000LL) >> 40 | (v & 0xFF00000000000000LL) >> 56;
}

static UINT LittleEndian32(UINT v) {
  if (IS_LITTLE_ENDIAN())
    return v;
  else
    return (v & 0x000000FF) << 24 | (v & 0x0000FF00) << 8 | (v & 0x00FF0000) >> 8 |
           (v & 0xFF000000) >> 24;
}

static SHORT LittleEndian16(SHORT v) {
  if (IS_LITTLE_ENDIAN())
    return v;
  else
    return (SHORT)(((v << 8) & 0xFF00) | ((v >> 8) & 0x00FF));
}

static USHORT Unpack(USHORT v) {
  if (IS_LITTLE_ENDIAN())
    return v;
  else
    return (SHORT)(((v << 8) & 0xFF00) | ((v >> 8) & 0x00FF));
}

UINT WAV_WriteHeader(HANDLE_WAV wav) {
  UINT size = 0;

  size += (FDKfwrite_EL(&wav->header.riffType, 1, 4, wav->fp)) *
          sizeof(wav->header.riffType[0]);  // RIFF Type
  size += (FDKfwrite_EL(&wav->header.riffSize, 4, 1, wav->fp)) *
          sizeof(wav->header.riffSize);  // RIFF Size
  size += (FDKfwrite_EL(&wav->header.waveType, 1, 4, wav->fp)) *
          sizeof(wav->header.waveType[0]);  // WAVE Type
  size += (FDKfwrite_EL(&wav->header.ds64Chunk.chunkID, 1, 4, wav->fp)) *
          sizeof(wav->header.ds64Chunk.chunkID[0]);  // JUNK/RF64 Type
  size += (FDKfwrite_EL(&wav->header.ds64Chunk.chunkSize, 4, 1, wav->fp)) *
          sizeof(wav->header.ds64Chunk.chunkSize);  // JUNK/RF64 Size
  size += (FDKfwrite_EL(&wav->header.ds64Chunk.riffSize64, 8, 1, wav->fp)) *
          sizeof(wav->header.ds64Chunk.riffSize64);  // Riff Size (64 bit)
  size += (FDKfwrite_EL(&wav->header.ds64Chunk.dataSize64, 8, 1, wav->fp)) *
          sizeof(wav->header.ds64Chunk.dataSize64);  // Data Size (64 bit)
  size += (FDKfwrite_EL(&wav->header.ds64Chunk.samplesPerCh64, 8, 1, wav->fp)) *
          sizeof(wav->header.ds64Chunk.samplesPerCh64);  // Number of Samples
  size += (FDKfwrite_EL(&wav->header.ds64Chunk.tableLength, 4, 1, wav->fp)) *
          sizeof(wav->header.ds64Chunk.tableLength);  // Number of valid entries in array table
  size += (FDKfwrite_EL(&wav->header.formatType, 1, 4, wav->fp)) *
          sizeof(wav->header.formatType[0]);  // format Type
  size += (FDKfwrite_EL(&wav->header.formatSize, 4, 1, wav->fp)) *
          sizeof(wav->header.formatSize);  // format Size
  size += (FDKfwrite_EL(&wav->header.compressionCode, 2, 1, wav->fp)) *
          sizeof(wav->header.compressionCode);  // compression Code
  size += (FDKfwrite_EL(&wav->header.numChannels, 2, 1, wav->fp)) *
          sizeof(wav->header.numChannels);  // numChannels
  size += (FDKfwrite_EL(&wav->header.sampleRate, 4, 1, wav->fp)) *
          sizeof(wav->header.sampleRate);  // sampleRate
  size += (FDKfwrite_EL(&wav->header.bytesPerSecond, 4, 1, wav->fp)) *
          sizeof(wav->header.bytesPerSecond);  // bytesPerSecond
  size += (FDKfwrite_EL(&wav->header.blockAlign, 2, 1, wav->fp)) *
          sizeof(wav->header.blockAlign);  // blockAlign
  size += (FDKfwrite_EL(&wav->header.bitsPerSample, 2, 1, wav->fp)) *
          sizeof(wav->header.bitsPerSample);  // bitsPerSample
  size += (FDKfwrite_EL(&wav->header.dataType, 1, 4, wav->fp)) *
          sizeof(wav->header.dataType[0]);  // dataType
  size += (FDKfwrite_EL(&wav->header.dataSize, 4, 1, wav->fp)) *
          sizeof(wav->header.dataSize);  // dataSize

  return size;
}

/**
 * WAV_OutputOpen
 * \brief Open WAV output/writer handle
 * \param pWav pointer to WAV handle to be returned
 * \param sampleRate desired samplerate of the resulting WAV file
 * \param numChannels desired number of audio channels of the resulting WAV file
 * \param bitsPerSample desired number of bits per audio sample of the resulting WAV file
 *
 * \return value:   0: ok
 *                 -1: error
 */
INT WAV_OutputOpen(HANDLE_WAV* pWav, const char* outputFilename, INT sampleRate, INT numChannels,
                   INT bitsPerSample) {
  HANDLE_WAV wav = (HANDLE_WAV)calloc(1, sizeof(struct WAV));
  UINT size = 0;

  if (wav == NULL) {
    FDKprintfErr("WAV_OutputOpen(): Unable to allocate WAV struct.\n");
    goto bail;
  }

  if (bitsPerSample != 16 && bitsPerSample != 24 && bitsPerSample != 32 &&
      bitsPerSample != NBITS_FLOAT && bitsPerSample != NBITS_DOUBLE) {
    FDKprintfErr("WAV_OutputOpen(): Invalid argument (bitsPerSample).\n");
    goto bail;
  }

  wav->fp = FDKfopen(outputFilename, "wb");
  if (wav->fp == NULL) {
    FDKprintfErr("WAV_OutputOpen(): unable to create file %s\n", outputFilename);
    goto bail;
  }

  FDKstrncpy(wav->header.riffType, "RIFF", 4);
  wav->header.riffSize =
      LittleEndian32(0x7fffffff); /* in case fseek() doesn't work later in WAV_OutputClose() */
  FDKstrncpy(wav->header.waveType, "WAVE", 4);

  /* Create 'JUNK' chunk for possible transition to RF64  */
  FDKstrncpy(wav->header.ds64Chunk.chunkID, "JUNK", 4);
  wav->header.ds64Chunk.chunkSize = LittleEndian32(28);
  wav->header.ds64Chunk.riffSize64 = LittleEndian64(0);
  wav->header.ds64Chunk.dataSize64 = LittleEndian64(0);
  wav->header.ds64Chunk.samplesPerCh64 = LittleEndian64(0);
  wav->header.ds64Chunk.tableLength = LittleEndian32(0);

  FDKstrncpy(wav->header.formatType, "fmt ", 4);
  wav->header.formatSize = LittleEndian32(16);

  wav->header.compressionCode = LittleEndian16(0x01);

  if (bitsPerSample == NBITS_FLOAT) {
    wav->header.compressionCode = LittleEndian16(0x03);
    bitsPerSample = 32;
  }
  if (bitsPerSample == NBITS_DOUBLE) {
    wav->header.compressionCode = LittleEndian16(0x03);
    bitsPerSample = 64;
  }

  wav->header.bitsPerSample = LittleEndian16((SHORT)bitsPerSample);
  wav->header.numChannels = LittleEndian16((SHORT)numChannels);
  wav->header.blockAlign = LittleEndian16((SHORT)(numChannels * (bitsPerSample >> 3)));
  wav->header.sampleRate = LittleEndian32(sampleRate);
  wav->header.bytesPerSecond = LittleEndian32(sampleRate * wav->header.blockAlign);
  FDKstrncpy(wav->header.dataType, "data", 4);
  wav->header.dataSize = LittleEndian32(
      0x7fffffff - WAV_HEADER_SIZE - (sizeof(wav->header.riffType) + sizeof(wav->header.riffSize)));

  size = WAV_WriteHeader(wav);

  if (size != WAV_HEADER_SIZE) {
    FDKprintfErr("WAV_OutputOpen(): error writing to output file %s\n", outputFilename);
    goto bail;
  }

  wav->header.dataSize = wav->header.riffSize = 0;

  *pWav = wav;

  return 0;

bail:
  if (wav) {
    if (wav->fp) {
      FDKfclose(wav->fp);
    }
    free(wav);
  }

  *pWav = NULL;

  return -1;
}

/**
 * WAV_OutputWrite
 * \brief Write data to WAV file asociated to WAV handle
 *
 * \param wav handle of wave file
 * \param sampleBuffer pointer to audio samples, right justified integer values
 * \param nBufBits size in bits of each audio sample in sampleBuffer
 * \param nSigBits amount of significant bits of each nBufBits in sampleBuffer
 *
 * \return value:    0: ok
 *                  -1: error
 */
INT WAV_OutputWrite(HANDLE_WAV wav, void* sampleBuffer, UINT numberOfSamples, int nBufBits,
                    int nSigBits) {
  SCHAR* bptr = (SCHAR*)sampleBuffer;
  SHORT* sptr = (SHORT*)sampleBuffer;
  LONG* lptr = (LONG*)sampleBuffer;
  float* fptr = (float*)sampleBuffer;
  double* dptr = (double*)sampleBuffer;
  LONG tmp;

  int bps = Unpack(wav->header.bitsPerSample);
  int wavIEEEfloat = (Unpack(wav->header.compressionCode) == 0x03);
  UINT i;

  if ((nBufBits == NBITS_FLOAT) || (nSigBits == NBITS_FLOAT)) {
    nBufBits = 32;
    nSigBits = NBITS_FLOAT;
  }
  if ((nBufBits == NBITS_DOUBLE) || (nSigBits == NBITS_DOUBLE)) {
    nBufBits = 64;
    nSigBits = NBITS_DOUBLE;
  }

  /* Pack samples if required */
  if ((!wavIEEEfloat && bps == nBufBits && bps == nSigBits) ||
      (wavIEEEfloat && bps == nBufBits && nSigBits < 0)) {
    if (FDKfwrite_EL(sampleBuffer, (bps >> 3), numberOfSamples, wav->fp) != numberOfSamples) {
      FDKprintfErr("WAV_OutputWrite(): error: unable to write to file %d\n", wav->fp);
      return -1;
    }
  } else if (!wavIEEEfloat) {
    for (i = 0; i < numberOfSamples; i++) {
      int result;
      int shift;

      if ((nSigBits == NBITS_FLOAT) || (nSigBits == NBITS_DOUBLE)) {
        double d;
        double scl = (double)(1UL << (bps - 1));

        if (nSigBits == NBITS_FLOAT)
          d = *fptr++;
        else
          d = *dptr++;

        d *= scl;
        if (d >= 0)
          d += 0.5;
        else
          d -= 0.5;

        if (d >= scl) d = scl - 1;
        if (d < -scl) d = -scl;

        tmp = (LONG)d;
      } else {
        switch (nBufBits) {
          case 8:
            tmp = *bptr++;
            break;
          case 16:
            tmp = *sptr++;
            break;
          case 32:
            tmp = *lptr++;
            break;
          default:
            return -1;
        }
        /* Adapt sample size */
        shift = bps - nSigBits;

        /* Correct alignment difference between 32 bit data buffer "tmp" and 24 bits to be written.
         */
        if (!IS_LITTLE_ENDIAN() && bps == 24) {
          shift += 8;
        }

        if (shift < 0)
          tmp >>= -shift;
        else
          tmp <<= shift;
      }

      /* Write sample */
      result = FDKfwrite_EL(&tmp, bps >> 3, 1, wav->fp);
      if (result <= 0) {
        FDKprintfErr("WAV_OutputWrite(): error: unable to write to file %d\n", wav->fp);
        return -1;
      }
    }
  } else {
    for (i = 0; i < numberOfSamples; i++) {
      int result;
      double d;

      if (nSigBits == NBITS_FLOAT) {
        d = *fptr++;
      } else if (nSigBits == NBITS_DOUBLE) {
        d = *dptr++;
      } else {
        switch (nBufBits) {
          case 8:
            tmp = *bptr++;
            break;
          case 16:
            tmp = *sptr++;
            break;
          case 32:
            tmp = *lptr++;
            break;
          default:
            return -1;
        }

        d = (double)tmp / (1U << (nSigBits - 1));
      }

      /* Write sample */
      if (bps == 64) {
        result = FDKfwrite_EL(&d, 8, 1, wav->fp);
      } else {
        float f = (float)d;
        result = FDKfwrite_EL(&f, 4, 1, wav->fp);
      }

      if (result <= 0) {
        FDKprintfErr("WAV_OutputWrite(): error: unable to write to file %d\n", wav->fp);
        return -1;
      }
    }
  }

  wav->header.ds64Chunk.dataSize64 += (numberOfSamples * (bps >> 3));
  return 0;
}

/**
 * WAV_OutputClose
 * \brief Close WAV Output handle
 * \param pWav pointer to WAV handle. *pWav is set to NULL.
 */
void WAV_OutputClose(HANDLE_WAV* pWav) {
  HANDLE_WAV wav = *pWav;
  UINT size = 0;

  if (wav == NULL) {
    return;
  }

  if (wav->header.ds64Chunk.dataSize64 + WAV_HEADER_SIZE >= (1LL << 32)) {
    /* File is >=4GB --> write RF64 Header */
    FDKstrncpy(wav->header.riffType, "RF64", 4);
    FDKstrncpy(wav->header.ds64Chunk.chunkID, "ds64", 4);
    wav->header.ds64Chunk.riffSize64 =
        LittleEndian64(wav->header.ds64Chunk.dataSize64 + WAV_HEADER_SIZE -
                       (sizeof(wav->header.riffType) + sizeof(wav->header.riffSize)));
    wav->header.ds64Chunk.samplesPerCh64 =
        LittleEndian64(wav->header.ds64Chunk.dataSize64 /
                       (wav->header.numChannels * (wav->header.bitsPerSample >> 3)));
    wav->header.ds64Chunk.dataSize64 = LittleEndian64(wav->header.ds64Chunk.dataSize64);
    wav->header.dataSize = LittleEndian32(0xFFFFFFFF);
    wav->header.riffSize = LittleEndian32(0xFFFFFFFF);
  } else {
    /* File is <4GB --> write RIFF Header */
    FDKstrncpy(wav->header.riffType, "RIFF", 4);
    FDKstrncpy(wav->header.ds64Chunk.chunkID, "JUNK", 4);
    wav->header.dataSize = (UINT)(wav->header.ds64Chunk.dataSize64 & 0x00000000FFFFFFFFLL);
    wav->header.riffSize =
        LittleEndian32(wav->header.dataSize + WAV_HEADER_SIZE -
                       (sizeof(wav->header.riffType) + sizeof(wav->header.riffSize)));
    wav->header.dataSize = LittleEndian32(wav->header.dataSize);
    wav->header.ds64Chunk.chunkSize = LittleEndian32(28);
    wav->header.ds64Chunk.riffSize64 = LittleEndian64(0);
    wav->header.ds64Chunk.dataSize64 = LittleEndian64(0);
    wav->header.ds64Chunk.samplesPerCh64 = LittleEndian64(0);
    wav->header.ds64Chunk.tableLength = LittleEndian32(0);
  }

  if (wav->fp != NULL) {
    if (FDKfseek(wav->fp, 0, FDKSEEK_SET)) {
      FDKprintf("WAV_OutputClose(): fseek() failed.\n");
    }

    size = WAV_WriteHeader(wav);

    if (size != WAV_HEADER_SIZE) {
      FDKprintfErr("WAV_OutputClose(): unable to write header\n");
    }

    if (FDKfclose(wav->fp)) {
      FDKprintfErr("WAV_OutputClose(): unable to close wav file\n");
    }
    wav->fp = NULL;
  }

  free(wav);
  *pWav = NULL;
}
