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

   Author(s):   M. Lohwasser

   Description: bitstream interface to bitbuffer routines

*******************************************************************************/

#ifndef FDK_BITSTREAM_H
#define FDK_BITSTREAM_H

#include "FDK_bitbuffer.h"
#include "machine_type.h"

#include "genericStds.h"

#define CACHE_BITS 32

#define BUFSIZE_DUMMY_VALUE MAX_BUFSIZE_BYTES

typedef enum { BS_READER, BS_WRITER } FDK_BS_CFG;

typedef struct {
  UINT CacheWord;
  UINT BitsInCache;
  FDK_BITBUF hBitBuf;
  UINT ConfigCache;
} FDK_BITSTREAM;

typedef FDK_BITSTREAM* HANDLE_FDK_BITSTREAM;

/**
 * \brief CreateBitStream Function.
 *
 * Create and initialize bitstream with extern allocated buffer.
 *
 * \param pBuffer  Pointer to BitBuffer array.
 * \param bufSize  Length of BitBuffer array. (awaits size 2^n and <= MAX_BUFSIZE_BYTES)
 * \param config   Initialize BitStream as Reader or Writer.
 */
FDK_INLINE
HANDLE_FDK_BITSTREAM FDKcreateBitStream(UCHAR* pBuffer, UINT bufSize,
                                        FDK_BS_CFG config = BS_READER) {
  HANDLE_FDK_BITSTREAM hBitStream = (HANDLE_FDK_BITSTREAM)FDKcalloc(1, sizeof(FDK_BITSTREAM));
  if (hBitStream == NULL) return NULL;
  FDK_InitBitBuffer(&hBitStream->hBitBuf, pBuffer, bufSize, 0);

  /* init cache */
  hBitStream->CacheWord = hBitStream->BitsInCache = 0;
  hBitStream->ConfigCache = config;

  return hBitStream;
}

/**
 * \brief Initialize BistreamBuffer. BitBuffer can point to filled BitBuffer array .
 *
 * \param hBitStream HANDLE_FDK_BITSTREAM handle
 * \param pBuffer    Pointer to BitBuffer array.
 * \param bufSize    Length of BitBuffer array in bytes. (awaits size 2^n and <= MAX_BUFSIZE_BYTES)
 * \param validBits  Number of valid BitBuffer filled Bits.
 * \param config     Initialize BitStream as Reader or Writer.
 * \return void
 */
FDK_INLINE
void FDKinitBitStream(HANDLE_FDK_BITSTREAM hBitStream, UCHAR* pBuffer, UINT bufSize, UINT validBits,
                      FDK_BS_CFG config = BS_READER) {
  FDK_InitBitBuffer(&hBitStream->hBitBuf, pBuffer, bufSize, validBits);

  /* init cache */
  hBitStream->CacheWord = hBitStream->BitsInCache = 0;
  hBitStream->ConfigCache = config;
}

/**
 * \brief ResetBitbuffer Function. Reset states in BitBuffer and Cache.
 *
 * \param hBitStream HANDLE_FDK_BITSTREAM handle
 * \param config     Initialize BitStream as Reader or Writer.
 * \return void
 */
FDK_INLINE void FDKresetBitbuffer(HANDLE_FDK_BITSTREAM hBitStream, FDK_BS_CFG config = BS_READER) {
  FDK_ResetBitBuffer(&hBitStream->hBitBuf);

  /* init cache */
  hBitStream->CacheWord = hBitStream->BitsInCache = 0;
  hBitStream->ConfigCache = config;
}

/** DeleteBitStream.

    Deletes the in Create Bitstream allocated BitStream and BitBuffer.
*/
FDK_INLINE void FDKdeleteBitStream(HANDLE_FDK_BITSTREAM hBitStream) {
  FDK_DeleteBitBuffer(&hBitStream->hBitBuf);
  FDKfree(hBitStream);
}

/**
 * \brief ReadBits Function (forward). This function returns a number of sequential
 *        bits from the input bitstream.
 *
 * \param hBitStream HANDLE_FDK_BITSTREAM handle
 * \param numberOfBits  The number of bits to be retrieved. ( (0),1 <= numberOfBits <= 32)
 * \return the requested bits, right aligned
 * \return
 */
#if defined(__clang__) && (__clang_major__ >= 12)
__attribute__((no_sanitize("unsigned-shift-base")))
#endif
FDK_INLINE UINT
FDKreadBits(HANDLE_FDK_BITSTREAM hBitStream, const UINT numberOfBits) {
  UINT bits = 0;
  INT missingBits = (INT)numberOfBits - (INT)hBitStream->BitsInCache;

  FDK_ASSERT(numberOfBits <= 32);
  if (missingBits > 0) {
    if (missingBits != 32) bits = hBitStream->CacheWord << missingBits;
    hBitStream->CacheWord = FDK_get32(&hBitStream->hBitBuf);
    hBitStream->BitsInCache += CACHE_BITS;
  }

  hBitStream->BitsInCache -= numberOfBits;

  return (bits | (hBitStream->CacheWord >> hBitStream->BitsInCache)) & BitMask[numberOfBits];
}

FDK_INLINE UINT FDKreadBit(HANDLE_FDK_BITSTREAM hBitStream) {
  if (!hBitStream->BitsInCache) {
    hBitStream->CacheWord = FDK_get32(&hBitStream->hBitBuf);
    hBitStream->BitsInCache = CACHE_BITS - 1;
    return hBitStream->CacheWord >> 31;
  }
  hBitStream->BitsInCache--;

  return (hBitStream->CacheWord >> hBitStream->BitsInCache) & 1;
}

/**
 * \brief Read2Bits Function (forward). This function reads 2 sequential
 *        bits from the input bitstream. It is the optimized version
          of FDKreadBits() for reading 2 bits.
 *
 * \param hBitStream HANDLE_FDK_BITSTREAM handle
 * \return the requested bits, right aligned
 * \return
 */
#if defined(__clang__) && (__clang_major__ >= 12)
__attribute__((no_sanitize("unsigned-shift-base")))
#endif
FDK_INLINE UINT
FDKread2Bits(HANDLE_FDK_BITSTREAM hBitStream) {
  /*
  ** Version corresponds to optimized FDKreadBits implementation
  ** calling FDK_get32, that keeps read pointer aligned.
  */
  UINT bits = 0;
  INT missingBits = (INT)2 - (INT)hBitStream->BitsInCache;
  if (missingBits > 0) {
    bits = hBitStream->CacheWord << missingBits;
    hBitStream->CacheWord = FDK_get32(&hBitStream->hBitBuf);
    hBitStream->BitsInCache += CACHE_BITS;
  }

  hBitStream->BitsInCache -= 2;

  return (bits | (hBitStream->CacheWord >> hBitStream->BitsInCache)) & 0x3;
}

/**
 * \brief ReadBits Function (backward). This function returns a number of sequential bits
 *        from the input bitstream.
 *
 * \param hBitStream HANDLE_FDK_BITSTREAM handle
 * \param numberOfBits  The number of bits to be retrieved.
 * \return the requested bits, right aligned
 */
FDK_INLINE UINT FDKreadBitsBwd(HANDLE_FDK_BITSTREAM hBitStream, const UINT numberOfBits) {
  const UINT validMask = BitMask[numberOfBits];

  if (hBitStream->BitsInCache <= numberOfBits) {
    const INT freeBits = (CACHE_BITS - 1) - hBitStream->BitsInCache;

    hBitStream->CacheWord =
        (hBitStream->CacheWord << freeBits) | FDK_getBwd(&hBitStream->hBitBuf, freeBits);
    hBitStream->BitsInCache += freeBits;
  }

  hBitStream->BitsInCache -= numberOfBits;

  return (hBitStream->CacheWord >> hBitStream->BitsInCache) & validMask;
}

/**
 * \brief read an integer value using a varying number of bits from the bitstream
 *
 *        q.v. ISO/IEC FDIS 23003-3  Table 16
 *
 * \param hBitStream HANDLE_FDK_BITSTREAM handle
 * \param nBits1  number of bits to read for a small integer value or escape value
 * \param nBits2  number of bits to read for a medium sized integer value or escape value
 * \param nBits3  number of bits to read for a large integer value
 * \return integer value read from bitstream
 */
FDK_INLINE UINT escapedValue(HANDLE_FDK_BITSTREAM hBitStream, int nBits1, int nBits2, int nBits3) {
  UINT value = FDKreadBits(hBitStream, nBits1);

  if (value == (UINT)(1 << nBits1) - 1) {
    UINT valueAdd = FDKreadBits(hBitStream, nBits2);
    value += valueAdd;
    if (valueAdd == (UINT)(1 << nBits2) - 1) {
      UINT tempDiff = (UINT)0xFFFFFFFF - value;
      UINT tempValue = FDKreadBits(hBitStream, nBits3);
      if (tempValue > tempDiff) {
        return (UINT)0xFFFFFFFF;
      }
      value += tempValue;
    }
  }

  return value;
}

/**
 * \brief return a number of bits from the bitBuffer.
 *        You have to know what you do! Cache has to be synchronized before using this
 *        function.
 *
 * \param hBitStream HANDLE_FDK_BITSTREAM handle
 * \param numBits The number of bits to be retrieved.
 * \return the requested bits, right aligned
 */
FDK_INLINE UINT FDKgetBits(HANDLE_FDK_BITSTREAM hBitStream, UINT numBits) {
  return FDK_get(&hBitStream->hBitBuf, numBits);
}

/**
 * \brief WriteBits Function. This function writes numberOfBits of value into bitstream.
 *
 * \param hBitStream HANDLE_FDK_BITSTREAM handle
 * \param value         The data to be written
 * \param numberOfBits  The number of bits to be written
 * \return              Number of bits written
 */
FDK_INLINE UCHAR FDKwriteBits(HANDLE_FDK_BITSTREAM hBitStream, UINT value,
                              const UINT numberOfBits) {
  const UINT validMask = BitMask[numberOfBits];

  if (hBitStream == NULL) {
    return numberOfBits;
  }

  if ((hBitStream->BitsInCache + numberOfBits) < CACHE_BITS) {
    hBitStream->BitsInCache += numberOfBits;
    hBitStream->CacheWord = (hBitStream->CacheWord << numberOfBits) | (value & validMask);
  } else {
    /* Put always 32 bits into memory             */
    /* - fill cache's LSBits with MSBits of value */
    /* - store 32 bits in memory using subroutine */
    /* - fill remaining bits into cache's LSBits  */
    /* - upper bits in cache are don't care       */

    /* Compute number of bits to be filled into cache */
    int missing_bits = CACHE_BITS - hBitStream->BitsInCache;
    int remaining_bits = numberOfBits - missing_bits;
    value = value & validMask;
    /* Avoid shift left by 32 positions */
    UINT CacheWord = (missing_bits == 32) ? 0 : (hBitStream->CacheWord << missing_bits);
    CacheWord |= (value >> (remaining_bits));
    FDK_put(&hBitStream->hBitBuf, CacheWord, 32);

    hBitStream->CacheWord = value;
    hBitStream->BitsInCache = remaining_bits;
  }

  return numberOfBits;
}

/**
 * \brief WriteBits Function (backward). This function writes numberOfBits of value into bitstream.
 *
 * \param hBitStream HANDLE_FDK_BITSTREAM handle
 * \param value         Variable holds data to be written.
 * \param numberOfBits  The number of bits to be written.
 * \return number of bits written
 */
FDK_INLINE UCHAR FDKwriteBitsBwd(HANDLE_FDK_BITSTREAM hBitStream, UINT value,
                                 const UINT numberOfBits) {
  const UINT validMask = BitMask[numberOfBits];

  if ((hBitStream->BitsInCache + numberOfBits) <= CACHE_BITS) {
    hBitStream->BitsInCache += numberOfBits;
    hBitStream->CacheWord = (hBitStream->CacheWord << numberOfBits) | (value & validMask);
  } else {
    FDK_putBwd(&hBitStream->hBitBuf, hBitStream->CacheWord, hBitStream->BitsInCache);
    hBitStream->BitsInCache = numberOfBits;
    hBitStream->CacheWord = (value & validMask);
  }

  return numberOfBits;
}

/**
 * \brief write an integer value using a varying number of bits from the bitstream
 *
 *        q.v. ISO/IEC FDIS 23003-3  Table 16
 *
 * \param hBitStream HANDLE_FDK_BITSTREAM handle
 * \param value   the data to be written
 * \param nBits1  number of bits to write for a small integer value or escape value
 * \param nBits2  number of bits to write for a medium sized integer value or escape value
 * \param nBits3  number of bits to write for a large integer value
 * \return number of bits written
 */
FDK_INLINE UCHAR FDKwriteEscapedValue(HANDLE_FDK_BITSTREAM hBitStream, UINT value, UINT nBits1,
                                      UINT nBits2, UINT nBits3) {
  UCHAR nbits = 0;
  UINT tmp = (1 << nBits1) - 1;

  if (value < tmp) {
    nbits += FDKwriteBits(hBitStream, value, nBits1);
  } else {
    nbits += FDKwriteBits(hBitStream, tmp, nBits1);
    value -= tmp;
    tmp = (1 << nBits2) - 1;

    if (value < tmp) {
      nbits += FDKwriteBits(hBitStream, value, nBits2);
    } else {
      nbits += FDKwriteBits(hBitStream, tmp, nBits2);
      value -= tmp;

      nbits += FDKwriteBits(hBitStream, value, nBits3);
    }
  }

  return nbits;
}

/**
 * \brief SyncCache Function. Clear cache after read forward.
 *
 * \param hBitStream HANDLE_FDK_BITSTREAM handle
 * \return void
 */
FDK_INLINE void FDKsyncCache(HANDLE_FDK_BITSTREAM hBitStream) {
  if (hBitStream->ConfigCache == BS_READER)
#ifdef FUNCTION_FDK_pushBackBS_READER
    FDK_pushBackBS_READER(&hBitStream->hBitBuf, hBitStream->BitsInCache);
#else
    FDK_pushBack(&hBitStream->hBitBuf, hBitStream->BitsInCache, hBitStream->ConfigCache);
#endif
  else if (hBitStream->BitsInCache) /* BS_WRITER */
    FDK_put(&hBitStream->hBitBuf, hBitStream->CacheWord, hBitStream->BitsInCache);

  hBitStream->BitsInCache = 0;
  hBitStream->CacheWord = 0;
}

/**
 * \brief SyncCache Function. Clear cache after read backwards.
 *
 * \param  hBitStream HANDLE_FDK_BITSTREAM handle
 * \return void
 */
FDK_INLINE void FDKsyncCacheBwd(HANDLE_FDK_BITSTREAM hBitStream) {
  if (hBitStream->ConfigCache == BS_READER) {
    FDK_pushForward(&hBitStream->hBitBuf, hBitStream->BitsInCache, hBitStream->ConfigCache);
  } else { /* BS_WRITER */
    FDK_putBwd(&hBitStream->hBitBuf, hBitStream->CacheWord, hBitStream->BitsInCache);
  }

  hBitStream->BitsInCache = 0;
  hBitStream->CacheWord = 0;
}

/**
 * \brief Byte Alignment Function with anchor
 *        This function performs the byte_alignment() syntactic function on the input stream,
 *        i.e. some bits will be discarded so that the next bits to be read/written would be aligned
 *        on a byte boundary with respect to the given alignment anchor.
 *
 * \param hBitStream HANDLE_FDK_BITSTREAM handle
 * \param alignmentAnchor bit position to be considered as origin for byte alignment
 * \return void
 */
FDK_INLINE void FDKbyteAlign(HANDLE_FDK_BITSTREAM hBitStream, UINT alignmentAnchor) {
  FDKsyncCache(hBitStream);
  if (hBitStream->ConfigCache == BS_READER) {
    FDK_pushForward(
        &hBitStream->hBitBuf,
        (8 - (((INT)alignmentAnchor - FDK_getValidBits(&hBitStream->hBitBuf)) & 0x07)) & 0x07,
        hBitStream->ConfigCache);
  } else {
    FDK_put(&hBitStream->hBitBuf, 0,
            (8 - ((FDK_getValidBits(&hBitStream->hBitBuf) - (INT)alignmentAnchor) & 0x07)) & 0x07);
  }
}

/**
 * \brief Push Back(Cache) / For / BiDirectional Function.
 *        PushBackCache function ungets a number of bits erroneously read/written by the last Get()
 * call. NB: The number of bits to be stuffed back into the stream may never exceed the number of
 * bits returned by the immediately preceding Get() call.
 *
 *       PushBack function ungets a number of bits (combines cache and bitbuffer indices)
 *       PushFor  function gets a number of bits (combines cache and bitbuffer indices)
 *       PushBiDirectional gets/ungets number of bits as defined in PusBack/For function
 *       NB: The sign of bits is not known, so the function checks direction and calls
 *        appropriate function. (positive sign pushFor, negative sign pushBack )
 *
 * \param hBitStream HANDLE_FDK_BITSTREAM handle
 * \param numberOfBits  The number of bits to be pushed back/for.
 * \return void
 */
FDK_INLINE void FDKpushBackCache(HANDLE_FDK_BITSTREAM hBitStream, const UINT numberOfBits) {
  FDK_ASSERT((hBitStream->BitsInCache + numberOfBits) <= CACHE_BITS);
  hBitStream->BitsInCache += numberOfBits;
}

FDK_INLINE void FDKpushBack(HANDLE_FDK_BITSTREAM hBitStream, const UINT numberOfBits) {
  if ((hBitStream->BitsInCache + numberOfBits) < CACHE_BITS &&
      (hBitStream->ConfigCache == BS_READER)) {
    hBitStream->BitsInCache += numberOfBits;
    FDKsyncCache(hBitStream); /* sync cache to avoid invalid cache */
  } else {
    FDKsyncCache(hBitStream);
    FDK_pushBack(&hBitStream->hBitBuf, numberOfBits, hBitStream->ConfigCache);
  }
}

FDK_INLINE void FDKpushFor(HANDLE_FDK_BITSTREAM hBitStream, const UINT numberOfBits) {
  if ((hBitStream->BitsInCache > numberOfBits) && (hBitStream->ConfigCache == BS_READER)) {
    hBitStream->BitsInCache -= numberOfBits;
  } else {
    FDKsyncCache(hBitStream);
    FDK_pushForward(&hBitStream->hBitBuf, numberOfBits, hBitStream->ConfigCache);
  }
}

FDK_INLINE void FDKpushBiDirectional(HANDLE_FDK_BITSTREAM hBitStream, const INT numberOfBits) {
  if (numberOfBits >= 0)
    FDKpushFor(hBitStream, numberOfBits);
  else
    FDKpushBack(hBitStream, -numberOfBits);
}

/**
 * \brief GetValidBits Function.  Clear cache and return valid Bits from Bitbuffer.
 * \param hBitStream HANDLE_FDK_BITSTREAM handle
 * \return amount of valid bits that still can be read or were already written. Negative value means
 *         that too many bits were read.
 *
 */
FDK_INLINE INT FDKgetValidBits(HANDLE_FDK_BITSTREAM hBitStream) {
  FDKsyncCache(hBitStream);
  return FDK_getValidBits(&hBitStream->hBitBuf);
}

/**
 * \brief return amount of unused Bits from Bitbuffer.
 * \param hBitStream HANDLE_FDK_BITSTREAM handle
 * \return amount of free bits that still can be written into the bitstream
 */
FDK_INLINE INT FDKgetFreeBits(HANDLE_FDK_BITSTREAM hBitStream) {
  return FDK_getFreeBits(&hBitStream->hBitBuf);
}

/**
 * \brief Fill the BitBuffer with a number of input bytes from  external source.
 *        The bytesValid variable returns the number of ramaining valid bytes in extern inputBuffer.
 *
 * \param hBitStream  HANDLE_FDK_BITSTREAM handle
 * \param inputBuffer Pointer to input buffer with bitstream data.
 * \param bufferSize  Total size of inputBuffer array.
 * \param bytesValid  Input: number of valid bytes in inputBuffer. Output: bytes still left unread
 * in inputBuffer.
 * \return void
 */
FDK_INLINE void FDKfeedBuffer(HANDLE_FDK_BITSTREAM hBitStream, const UCHAR inputBuffer[],
                              const UINT bufferSize, UINT* bytesValid) {
  FDKsyncCache(hBitStream);
  FDK_Feed(&hBitStream->hBitBuf, inputBuffer, bufferSize, bytesValid);
}

/**
 * \brief fill destination BitBuffer with a number of bytes from source BitBuffer. The
 *        bytesValid variable returns the number of ramaining valid bytes in source BitBuffer.
 *
 * \param hBSDst            HANDLE_FDK_BITSTREAM handle to write data into
 * \param hBSSrc            HANDLE_FDK_BITSTREAM handle to read data from
 * \param bytesValid        Input: number of valid bytes in inputBuffer. Output: bytes still left
 * unread in inputBuffer.
 * \return void
 */
FDK_INLINE void FDKcopyBuffer(HANDLE_FDK_BITSTREAM hBSDst, HANDLE_FDK_BITSTREAM hBSSrc,
                              UINT* bytesValid) {
  FDKsyncCache(hBSSrc);
  FDK_Copy(&hBSDst->hBitBuf, &hBSSrc->hBitBuf, bytesValid);
}

/**
 * \brief fill the outputBuffer with all valid bytes hold in BitBuffer. The WriteBytes
 *        variable returns the number of written Bytes.
 *
 * \param hBitStream    HANDLE_FDK_BITSTREAM handle
 * \param outputBuffer  Pointer to output buffer.
 * \param writeBytes    Number of bytes write to output buffer.
 * \return void
 */
FDK_INLINE void FDKfetchBuffer(HANDLE_FDK_BITSTREAM hBitStream, UCHAR* outputBuffer,
                               UINT* writeBytes) {
  FDKsyncCache(hBitStream);
  FDK_Fetch(&hBitStream->hBitBuf, outputBuffer, writeBytes);
}

#endif
