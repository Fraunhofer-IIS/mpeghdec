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

   Description: common bitbuffer read/write routines

*******************************************************************************/

#ifndef FDK_BITBUFFER_H
#define FDK_BITBUFFER_H

#include "FDK_archdef.h"
#include "machine_type.h"

/* leave 3 bits headroom so MAX_BUFSIZE can be represented in bits as well. */
#define MAX_BUFSIZE_BYTES (0x08000000)

typedef struct {
  INT ValidBits;
  UINT ReadOffset;
  UINT WriteOffset;
  UINT BitNdx;

  UCHAR* Buffer; /* struct member offset:  5 */
  UINT bufSize;  /* struct member offset:  6 */
  UINT bufBits;  /* struct member offset:  7 */
} FDK_BITBUF;

typedef FDK_BITBUF* HANDLE_FDK_BITBUF;

#ifdef __cplusplus
extern "C" {
#endif

extern const UINT BitMask[32 + 1];

/**  The BitBuffer Functions are called straight from FDK_bitstream Interface.
     For Functions functional survey look there.
*/

void FDK_CreateBitBuffer(HANDLE_FDK_BITBUF* hBitBuffer, UCHAR* pBuffer, UINT bufSize);

void FDK_InitBitBuffer(HANDLE_FDK_BITBUF hBitBuffer, UCHAR* pBuffer, UINT bufSize, UINT validBits);

void FDK_ResetBitBuffer(HANDLE_FDK_BITBUF hBitBuffer);

void FDK_DeleteBitBuffer(HANDLE_FDK_BITBUF hBitBuffer);

#ifdef INLINE_FDK_get

#define FUNCTION_FDK_get

FDK_INLINE INT FDK_get(HANDLE_FDK_BITBUF hBitBuf, const UINT numberOfBits) {
  UINT byteOffset = hBitBuf->BitNdx >> 3;
  UINT bitOffset = hBitBuf->BitNdx & 0x07;

  hBitBuf->BitNdx = (hBitBuf->BitNdx + numberOfBits) & (hBitBuf->bufBits - 1);
  hBitBuf->ValidBits -= numberOfBits;

  UINT byteMask = hBitBuf->bufSize - 1;

  UINT tx = (hBitBuf->Buffer[byteOffset & byteMask] << 24) |
            (hBitBuf->Buffer[(byteOffset + 1) & byteMask] << 16) |
            (hBitBuf->Buffer[(byteOffset + 2) & byteMask] << 8) |
            hBitBuf->Buffer[(byteOffset + 3) & byteMask];

  tx <<= bitOffset;
  tx |= hBitBuf->Buffer[(byteOffset + 4) & byteMask] >> (8 - bitOffset);
  return (tx >> (32 - numberOfBits));
}
#else
INT FDK_get(HANDLE_FDK_BITBUF hBitBuffer, const UINT numberOfBits);
#endif /* #ifdef INLINE_FDK_get */

#ifdef INLINE_FDK_get32

#define FUNCTION_FDK_get32

FDK_INLINE INT FDK_get32(HANDLE_FDK_BITBUF hBitBuf) {
  UINT BitNdx = hBitBuf->BitNdx + 32;
  hBitBuf->BitNdx = BitNdx & (hBitBuf->bufBits - 1);
  hBitBuf->ValidBits -= 32;

  UINT byteOffset = (BitNdx - 1) >> 3;
  if (BitNdx <= hBitBuf->bufBits) {
    UINT cache = (hBitBuf->Buffer[(byteOffset - 3)] << 24) |
                 (hBitBuf->Buffer[(byteOffset - 2)] << 16) |
                 (hBitBuf->Buffer[(byteOffset - 1)] << 8) | hBitBuf->Buffer[(byteOffset - 0)];

    if ((BitNdx = (BitNdx & 7)) != 0) {
      cache = (cache >> (8 - BitNdx)) | ((UINT)hBitBuf->Buffer[byteOffset - 4] << (24 + BitNdx));
    }
    return (cache);
  } else {
    UINT byte_mask = hBitBuf->bufSize - 1;
    UINT cache = (hBitBuf->Buffer[(byteOffset - 3) & byte_mask] << 24) |
                 (hBitBuf->Buffer[(byteOffset - 2) & byte_mask] << 16) |
                 (hBitBuf->Buffer[(byteOffset - 1) & byte_mask] << 8) |
                 hBitBuf->Buffer[(byteOffset - 0) & byte_mask];

    if ((BitNdx = (BitNdx & 7)) != 0) {
      cache = (cache >> (8 - BitNdx)) |
              ((UINT)hBitBuf->Buffer[(byteOffset - 4) & byte_mask] << (24 + BitNdx));
    }
    return (cache);
  }
}
#else
INT FDK_get32(HANDLE_FDK_BITBUF hBitBuf);
#endif

void FDK_put(HANDLE_FDK_BITBUF hBitBuffer, UINT value, const UINT numberOfBits);

INT FDK_getBwd(HANDLE_FDK_BITBUF hBitBuffer, const UINT numberOfBits);
void FDK_putBwd(HANDLE_FDK_BITBUF hBitBuffer, UINT value, const UINT numberOfBits);

#ifdef INLINE_FDK_pushBack

#define FUNCTION_FDK_pushBack

FDK_INLINE void FDK_pushBack(HANDLE_FDK_BITBUF hBitBuf, const UINT numberOfBits, UCHAR config) {
  hBitBuf->ValidBits += (config == 0) ? numberOfBits : (-(INT)numberOfBits);
  hBitBuf->BitNdx = (hBitBuf->BitNdx - numberOfBits) & (hBitBuf->bufBits - 1);
}
#else
void FDK_pushBack(HANDLE_FDK_BITBUF hBitBuffer, const UINT numberOfBits, UCHAR config);
#endif

#ifdef INLINE_FDK_pushBackBS_READER

#define FUNCTION_FDK_pushBackBS_READER

static inline void FDK_pushBackBS_READER(HANDLE_FDK_BITBUF hBitBuf, const UINT numberOfBits) {
  hBitBuf->ValidBits += numberOfBits;
  hBitBuf->BitNdx = (hBitBuf->BitNdx - numberOfBits) & (hBitBuf->bufBits - 1);
}
#else
void FDK_pushBackBS_READER(HANDLE_FDK_BITBUF hBitBuf, const UINT numberOfBits);
#endif /* #ifdef INLINE_FDK_pushBackBS_READER */

void FDK_pushForward(HANDLE_FDK_BITBUF hBitBuffer, const UINT numberOfBits, UCHAR config);

#ifdef INLINE_FDK_getValidBits

#define FUNCTION_FDK_getValidBits

static inline INT FDK_getValidBits(HANDLE_FDK_BITBUF hBitBuffer) {
  return hBitBuffer->ValidBits;
}
#else
INT FDK_getValidBits(HANDLE_FDK_BITBUF hBitBuffer);
#endif /* #ifdef INLINE_FDK_getValidBits */

INT FDK_getFreeBits(HANDLE_FDK_BITBUF hBitBuffer);

void FDK_Feed(HANDLE_FDK_BITBUF hBitBuffer, const UCHAR inputBuffer[], const UINT bufferSize,
              UINT* bytesValid);

void FDK_Copy(HANDLE_FDK_BITBUF hBitBufDst, HANDLE_FDK_BITBUF hBitBufSrc, UINT* bytesValid);

void FDK_Fetch(HANDLE_FDK_BITBUF hBitBuffer, UCHAR outBuf[], UINT* writeBytes);

#ifdef __cplusplus
}
#endif

#endif
