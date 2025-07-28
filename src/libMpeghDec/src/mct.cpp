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

   Author(s):   Matthias Neusinger

   Description: multi-channel tool

*******************************************************************************/

#include "aacdecoder.h"

#include "mct.h"
#include "FDK_igfDec.h"

#define CODE_BOOK_ALPHA_LAV 121
#define CODE_BOOK_BETA_LAV 65
#define DEFAULT_BETA (48) /*equals 45 degrees */
#define DEFAULT_ALPHA (0)
#define READ_MASK(a, b) ((a) & (UINT64)1 << (63 - (b)))

/* huffman tables */

static const int huff_ctabAngle[CODE_BOOK_BETA_LAV] = {
    0x00000000, 0x0000000B, 0x00000012, 0x0000001B, 0x0000001F, 0x00000031, 0x0000003A, 0x00000043,
    0x00000065, 0x00000073, 0x00000082, 0x0000009A, 0x000000CE, 0x000000EE, 0x00000106, 0x0000013A,
    0x000001D9, 0x000001DE, 0x00000202, 0x00000261, 0x0000020F, 0x0000020E, 0x00000263, 0x00000266,
    0x00000272, 0x00000271, 0x00000277, 0x00000276, 0x00000334, 0x00000325, 0x00000326, 0x00000327,
    0x00000324, 0x00000323, 0x00000335, 0x00000322, 0x00000320, 0x00000321, 0x00000273, 0x00000270,
    0x00000267, 0x00000260, 0x000004C4, 0x000004C5, 0x00000203, 0x000001DF, 0x000001DA, 0x000001D8,
    0x0000019B, 0x000001DB, 0x00000132, 0x00000100, 0x000000CF, 0x000000CC, 0x0000009B, 0x00000081,
    0x00000072, 0x0000004F, 0x00000042, 0x00000038, 0x00000030, 0x0000001E, 0x0000001A, 0x00000011,
    0x0000000A};

static const int huff_ltabAngle[CODE_BOOK_BETA_LAV] = {
    0x00000001, 0x00000004, 0x00000005, 0x00000005, 0x00000005, 0x00000006, 0x00000006, 0x00000007,
    0x00000007, 0x00000007, 0x00000008, 0x00000008, 0x00000008, 0x00000008, 0x00000009, 0x00000009,
    0x00000009, 0x00000009, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A,
    0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A,
    0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A,
    0x0000000A, 0x0000000A, 0x0000000B, 0x0000000B, 0x0000000A, 0x00000009, 0x00000009, 0x00000009,
    0x00000009, 0x00000009, 0x00000009, 0x00000009, 0x00000008, 0x00000008, 0x00000008, 0x00000008,
    0x00000007, 0x00000007, 0x00000007, 0x00000006, 0x00000006, 0x00000005, 0x00000005, 0x00000005,
    0x00000004};
#define HUFF_LTABANGLE_MAX_VALUE 0xB

static const int huff_ctabscf[CODE_BOOK_ALPHA_LAV] = {
    0x0003ffe8, 0x0003ffe6, 0x0003ffe7, 0x0003ffe5, 0x0007fff5, 0x0007fff1, 0x0007ffed, 0x0007fff6,
    0x0007ffee, 0x0007ffef, 0x0007fff0, 0x0007fffc, 0x0007fffd, 0x0007ffff, 0x0007fffe, 0x0007fff7,
    0x0007fff8, 0x0007fffb, 0x0007fff9, 0x0003ffe4, 0x0007fffa, 0x0003ffe3, 0x0001ffef, 0x0001fff0,
    0x0000fff5, 0x0001ffee, 0x0000fff2, 0x0000fff3, 0x0000fff4, 0x0000fff1, 0x00007ff6, 0x00007ff7,
    0x00003ff9, 0x00003ff5, 0x00003ff7, 0x00003ff3, 0x00003ff6, 0x00003ff2, 0x00001ff7, 0x00001ff5,
    0x00000ff9, 0x00000ff7, 0x00000ff6, 0x000007f9, 0x00000ff4, 0x000007f8, 0x000003f9, 0x000003f7,
    0x000003f5, 0x000001f8, 0x000001f7, 0x000000fa, 0x000000f8, 0x000000f6, 0x00000079, 0x0000003a,
    0x00000038, 0x0000001a, 0x0000000b, 0x00000004, 0x00000000, 0x0000000a, 0x0000000c, 0x0000001b,
    0x00000039, 0x0000003b, 0x00000078, 0x0000007a, 0x000000f7, 0x000000f9, 0x000001f6, 0x000001f9,
    0x000003f4, 0x000003f6, 0x000003f8, 0x000007f5, 0x000007f4, 0x000007f6, 0x000007f7, 0x00000ff5,
    0x00000ff8, 0x00001ff4, 0x00001ff6, 0x00001ff8, 0x00003ff8, 0x00003ff4, 0x0000fff0, 0x00007ff4,
    0x0000fff6, 0x00007ff5, 0x0003ffe2, 0x0007ffd9, 0x0007ffda, 0x0007ffdb, 0x0007ffdc, 0x0007ffdd,
    0x0007ffde, 0x0007ffd8, 0x0007ffd2, 0x0007ffd3, 0x0007ffd4, 0x0007ffd5, 0x0007ffd6, 0x0007fff2,
    0x0007ffdf, 0x0007ffe7, 0x0007ffe8, 0x0007ffe9, 0x0007ffea, 0x0007ffeb, 0x0007ffe6, 0x0007ffe0,
    0x0007ffe1, 0x0007ffe2, 0x0007ffe3, 0x0007ffe4, 0x0007ffe5, 0x0007ffd7, 0x0007ffec, 0x0007fff4,
    0x0007fff3};

static const int huff_ltabscf[CODE_BOOK_ALPHA_LAV] = {
    0x00000012, 0x00000012, 0x00000012, 0x00000012, 0x00000013, 0x00000013, 0x00000013, 0x00000013,
    0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013,
    0x00000013, 0x00000013, 0x00000013, 0x00000012, 0x00000013, 0x00000012, 0x00000011, 0x00000011,
    0x00000010, 0x00000011, 0x00000010, 0x00000010, 0x00000010, 0x00000010, 0x0000000f, 0x0000000f,
    0x0000000e, 0x0000000e, 0x0000000e, 0x0000000e, 0x0000000e, 0x0000000e, 0x0000000d, 0x0000000d,
    0x0000000c, 0x0000000c, 0x0000000c, 0x0000000b, 0x0000000c, 0x0000000b, 0x0000000a, 0x0000000a,
    0x0000000a, 0x00000009, 0x00000009, 0x00000008, 0x00000008, 0x00000008, 0x00000007, 0x00000006,
    0x00000006, 0x00000005, 0x00000004, 0x00000003, 0x00000001, 0x00000004, 0x00000004, 0x00000005,
    0x00000006, 0x00000006, 0x00000007, 0x00000007, 0x00000008, 0x00000008, 0x00000009, 0x00000009,
    0x0000000a, 0x0000000a, 0x0000000a, 0x0000000b, 0x0000000b, 0x0000000b, 0x0000000b, 0x0000000c,
    0x0000000c, 0x0000000d, 0x0000000d, 0x0000000d, 0x0000000e, 0x0000000e, 0x00000010, 0x0000000f,
    0x00000010, 0x0000000f, 0x00000012, 0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013,
    0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013,
    0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013,
    0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013,
    0x00000013};
#define HUFF_LTABSCF_MAX_VALUE 0x13

/* quantized sin/cos tables for index */
static const FIXP_SPK tabIndexToCosSinAlpha[CODE_BOOK_BETA_LAV] = {
    {{(SHORT)0x0000, (SHORT)0x8000}}, {{(SHORT)0x0648, (SHORT)0x8027}},
    {{(SHORT)0x0C8C, (SHORT)0x809E}}, {{(SHORT)0x12C8, (SHORT)0x8163}},
    {{(SHORT)0x18F9, (SHORT)0x8276}}, {{(SHORT)0x1F1A, (SHORT)0x83D6}},
    {{(SHORT)0x2528, (SHORT)0x8583}}, {{(SHORT)0x2B1F, (SHORT)0x877B}},
    {{(SHORT)0x30FC, (SHORT)0x89BE}}, {{(SHORT)0x36BA, (SHORT)0x8C4A}},
    {{(SHORT)0x3C57, (SHORT)0x8F1D}}, {{(SHORT)0x41CE, (SHORT)0x9236}},
    {{(SHORT)0x471D, (SHORT)0x9592}}, {{(SHORT)0x4C40, (SHORT)0x9930}},
    {{(SHORT)0x5134, (SHORT)0x9D0E}}, {{(SHORT)0x55F6, (SHORT)0xA129}},
    {{(SHORT)0x5A82, (SHORT)0xA57E}}, {{(SHORT)0x5ED7, (SHORT)0xAA0A}},
    {{(SHORT)0x62F2, (SHORT)0xAECC}}, {{(SHORT)0x66D0, (SHORT)0xB3C0}},
    {{(SHORT)0x6A6E, (SHORT)0xB8E3}}, {{(SHORT)0x6DCA, (SHORT)0xBE32}},
    {{(SHORT)0x70E3, (SHORT)0xC3A9}}, {{(SHORT)0x73B6, (SHORT)0xC946}},
    {{(SHORT)0x7642, (SHORT)0xCF04}}, {{(SHORT)0x7885, (SHORT)0xD4E1}},
    {{(SHORT)0x7A7D, (SHORT)0xDAD8}}, {{(SHORT)0x7C2A, (SHORT)0xE0E6}},
    {{(SHORT)0x7D8A, (SHORT)0xE707}}, {{(SHORT)0x7E9D, (SHORT)0xED38}},
    {{(SHORT)0x7F62, (SHORT)0xF374}}, {{(SHORT)0x7FD9, (SHORT)0xF9B8}},
    {{(SHORT)0x7FFF, (SHORT)0x0000}}, {{(SHORT)0x7FD9, (SHORT)0x0648}},
    {{(SHORT)0x7F62, (SHORT)0x0C8C}}, {{(SHORT)0x7E9D, (SHORT)0x12C8}},
    {{(SHORT)0x7D8A, (SHORT)0x18F9}}, {{(SHORT)0x7C2A, (SHORT)0x1F1A}},
    {{(SHORT)0x7A7D, (SHORT)0x2528}}, {{(SHORT)0x7885, (SHORT)0x2B1F}},
    {{(SHORT)0x7642, (SHORT)0x30FC}}, {{(SHORT)0x73B6, (SHORT)0x36BA}},
    {{(SHORT)0x70E3, (SHORT)0x3C57}}, {{(SHORT)0x6DCA, (SHORT)0x41CE}},
    {{(SHORT)0x6A6E, (SHORT)0x471D}}, {{(SHORT)0x66D0, (SHORT)0x4C40}},
    {{(SHORT)0x62F2, (SHORT)0x5134}}, {{(SHORT)0x5ED7, (SHORT)0x55F6}},
    {{(SHORT)0x5A82, (SHORT)0x5A82}}, {{(SHORT)0x55F6, (SHORT)0x5ED7}},
    {{(SHORT)0x5134, (SHORT)0x62F2}}, {{(SHORT)0x4C40, (SHORT)0x66D0}},
    {{(SHORT)0x471D, (SHORT)0x6A6E}}, {{(SHORT)0x41CE, (SHORT)0x6DCA}},
    {{(SHORT)0x3C57, (SHORT)0x70E3}}, {{(SHORT)0x36BA, (SHORT)0x73B6}},
    {{(SHORT)0x30FC, (SHORT)0x7642}}, {{(SHORT)0x2B1F, (SHORT)0x7885}},
    {{(SHORT)0x2528, (SHORT)0x7A7D}}, {{(SHORT)0x1F1A, (SHORT)0x7C2A}},
    {{(SHORT)0x18F9, (SHORT)0x7D8A}}, {{(SHORT)0x12C8, (SHORT)0x7E9D}},
    {{(SHORT)0x0C8C, (SHORT)0x7F62}}, {{(SHORT)0x0648, (SHORT)0x7FD9}},
    {{(SHORT)0x0000, (SHORT)0x7FFF}}};

/* Include platform specific implementations */
#if defined(__arm__)
#include "arm/mct_arm.cpp"
#endif

static inline WHITENING_LEVEL GetTileWhiteningLevel(IGF_PRIVATE_DATA_HANDLE hPrivateData,
                                                    INT TileNum) {
  return hPrivateData->bitstreamData[0].igfWhiteningLevel[TileNum];
}

int CMct_Initialize(CMctPtr* pCMctPtr, const ULONG mctChanMask, int firstSigIdx,
                    int signalsInGroup) {
  int j;
  CMctPtr mct;

  if (pCMctPtr == NULL) return -1;
  if (*pCMctPtr != NULL) return -1;

  mct = (CMct*)FDKcalloc(sizeof(CMct), 1);
  if (!mct) return AAC_DEC_OUT_OF_MEMORY;

  mct->mctWork = (CMctWorkPtr)FDKcalloc(sizeof(CMctWork), 1);
  if (!mct->mctWork) {
    goto bail;
  }

  /* get channelmap/mask for which channels the tool is active */
  mct->numMctChannels = 0;
  for (j = 0; j < signalsInGroup; j++) {
    const int channelMask_j = mctChanMask & ((ULONG)1 << (31 - j));
    if (channelMask_j) {
      mct->channelMap[mct->numMctChannels] = j + firstSigIdx;
      mct->numMctChannels++;
    }
  }

  if (mct->numMctChannels) {
    mct->prevOutSpec = (FIXP_DBL*)FDKmalloc(mct->numMctChannels * 1024 * sizeof(FIXP_DBL));
    if (!mct->prevOutSpec) goto bail;
    mct->prevOutSpec_exp = (SHORT*)FDKmalloc(mct->numMctChannels * 8 * 16 * sizeof(SHORT));
    if (!mct->prevOutSpec_exp) goto bail;
  } else {
    mct->prevOutSpec = NULL;
    mct->prevOutSpec_exp = NULL;
  }

  (*pCMctPtr) = mct;

  return AAC_DEC_OK;

bail:
  if (mct->mctWork != NULL) FDKfree(mct->mctWork);
  if (mct->prevOutSpec != NULL) FDKfree(mct->prevOutSpec);
  /* if (mct->prevOutSpec_exp != NULL) FDKfree(mct->prevOutSpec_exp); */
  FDKfree(mct);

  return AAC_DEC_OUT_OF_MEMORY;
}

void CMct_Destroy(CMctPtr self) {
  if (self != NULL) {
    if (self->mctWork != NULL) {
      FDKfree(self->mctWork);
    }
    if (self->prevOutSpec != NULL) {
      FDKfree(self->prevOutSpec);
    }
    if (self->prevOutSpec_exp != NULL) {
      FDKfree(self->prevOutSpec_exp);
    }
    FDKfree(self);
  }
}

static int CMct_inverseMctParseChannelPair(HANDLE_FDK_BITSTREAM hbitBuffer, UCHAR* pCodePairs,
                                           unsigned int nChannels) {
  int err = 1;
  unsigned int chan0 = 0, chan1;

  int pairIdx = 0;
  int pairCounter = 0;
  int maxNumPairIdx = (int)nChannels * ((int)nChannels - 1) / 2 - 1;
  int numBits = 1;

  if (hbitBuffer == NULL) return 1;
  if (pCodePairs == NULL) return 1;
  if (maxNumPairIdx < 0) return 1;

  while (maxNumPairIdx >>= 1) {
    ++numBits;
  }

  pairIdx = FDKreadBits(hbitBuffer, numBits);

  for (chan1 = 1; chan1 < nChannels; chan1++) {
    for (chan0 = 0; chan0 < chan1; chan0++) {
      if (pairCounter == pairIdx) {
        pCodePairs[0] = chan0;
        pCodePairs[1] = chan1;
        err = 0; /* return from function */
        break;
      } else
        pairCounter++;
    }
    if (err == 0) { /* pair found */
      break;
    }
  }

  return err;
}

static int CMct_decodeHuffman(HANDLE_FDK_BITSTREAM hbitBuffer, const int* const ctab,
                              const int* const ltab, int maxl, int limit) {
  int tabIdx, shift;

  int bsVal = FDKreadBits(hbitBuffer, maxl);

  for (tabIdx = 0; tabIdx < limit; tabIdx++) {
    shift = maxl - ltab[tabIdx]; /* shift in [0.. maxl] */
    if ((bsVal >> shift) == ctab[tabIdx]) {
      /* found */
      break;
    }
  }
  if (tabIdx >= limit) {
    /* End of angle code book reached */
    tabIdx = -1;
    shift = maxl;
  }
  FDKpushBack(hbitBuffer, shift);

  return tabIdx;
}

static int Read_MultichannelCodingBox(const int MCTSignalingType, const int usacIndepFlag,
                                      HANDLE_FDK_BITSTREAM hbitBuffer, const CMctPtr self,
                                      CMctWorkPtr work, const int i, const int* huff_ctab,
                                      const int* huff_ltab, const int huff_ltab_max_val,
                                      const int code_book_lav) {
  int retVal = 0;

  /* read channel pair */
  if (work->keepTree == 0) {
    retVal = CMct_inverseMctParseChannelPair(hbitBuffer, work->codePairs[i], self->numMctChannels);
    if (retVal > 0) {
      return 1;
    }
  }

  /* read flags */
  work->bHasMctMask[i] = FDKreadBit(hbitBuffer);
  work->bHasBandwiseCoeffs[i] = FDKreadBit(hbitBuffer);

  /* read number of bands for non-fullband boxes */
  work->numMctMaskBands[i] = MAX_NUM_MCT_BANDS;
  if (work->bHasMctMask[i] || work->bHasBandwiseCoeffs[i]) {
    int pairIsShort;
    pairIsShort = FDKreadBit(hbitBuffer);
    work->numMctMaskBands[i] = FDKreadBits(hbitBuffer, 5);
    if (pairIsShort) {
      work->numMctMaskBands[i] *= 8;
    }
    if (work->numMctMaskBands[i] > MAX_NUM_MCT_BANDS) {
      return 1;
    }
  }

  /* evaluate mctMask flag */
  if (work->bHasMctMask[i] && work->numMctMaskBands[i]) {
    /* parse mct mask: read work->numMctMaskBands[i] (= 1...64) bits and left align them in UINT64
     */
    const UINT64 bits_hi =
        (UINT64)FDKreadBits(hbitBuffer, fMin((int)32, (int)work->numMctMaskBands[i]))
        << fMax(32, (64 - work->numMctMaskBands[i]));
    const UINT64 bits_lo =
        (UINT64)FDKreadBits(hbitBuffer, fMax((int)0, (int)work->numMctMaskBands[i] - 32))
        << (64 - work->numMctMaskBands[i]);
    work->mctMask[i] = bits_hi | bits_lo;
  } else {
    /* all bands active */
    work->mctMask[i] = 0xffffffffffffffffULL;
  }

  work->predDir[i] = (MCTSignalingType == 0) ? FDKreadBit(hbitBuffer) : 0;

  if (usacIndepFlag > 0) {
    work->bDeltaTime[i] = 0;
  } else {
    work->bDeltaTime[i] = FDKreadBit(hbitBuffer);
  }

  /*Sanity check*/
  if ((work->bDeltaTime[i]) && (self->MCCSignalingType != self->MCTSignalingTypePrev)) {
    return 1;
  }

  if (work->bHasBandwiseCoeffs[i] == 0) {
    /* use fullband angle */
    int val =
        CMct_decodeHuffman(hbitBuffer, huff_ctab, huff_ltab, huff_ltab_max_val, code_book_lav);
    FDK_ASSERT(val >= 0 && val < 256);
    work->pairCoeffDeltaFb[i] = val;
  } else {
    /* use bandwise angles */
    for (int j = 0; j < work->numMctMaskBands[i]; j++) {
      if (READ_MASK(work->mctMask[i], j) > 0) {
        int val =
            CMct_decodeHuffman(hbitBuffer, huff_ctab, huff_ltab, huff_ltab_max_val, code_book_lav);
        FDK_ASSERT(val >= 0 && val < 256);
        work->pairCoeffDeltaSfb[i][j] = val;
      }
    }
  }

  return 0;
}

int CMct_inverseMctParseBS(CMctPtr self, HANDLE_FDK_BITSTREAM hbitBuffer, int usacIndepFlag,
                           int nChannels) {
  int i = 0, j = 0, default_value = 0;
  int retVal = 0;
  CMctWorkPtr work = NULL;

  if (self == NULL) return -1;
  if (hbitBuffer == NULL) return -1;

  work = self->mctWork;

  UINT MCTStereoFilling = FDKreadBit(hbitBuffer);
  self->MCCSignalingType = FDKreadBit(hbitBuffer);
  work->keepTree = 0;
  if (!usacIndepFlag) {
    work->keepTree = FDKreadBit(hbitBuffer);
  }

  if (self->MCCSignalingType == 0) {
    default_value = DEFAULT_ALPHA;
  } else if (self->MCCSignalingType == 1) {
    default_value = DEFAULT_BETA;
  }

  if (usacIndepFlag > 0) {
    FDK_ASSERT(work->keepTree == 0);

    /* reset coefficient memory on indepFlag */
    for (i = 0; i < MAX_NUM_MCT_BOXES; i++) {
      self->pairCoeffQFbPrev[i] = default_value;
      for (j = 0; j < MAX_NUM_MCT_BANDS; j++) {
        self->pairCoeffQSfbPrev[i][j] = default_value;
      }
    }
  }

  /* reset coefficient memory above the number of pairs */
  for (i = work->numPairs; i < MAX_NUM_MCT_BOXES; i++) {
    self->pairCoeffQFbPrev[i] = default_value;
    for (j = 0; j < MAX_NUM_MCT_BANDS; j++) {
      self->pairCoeffQSfbPrev[i][j] = default_value;
    }
  }

  if (work->keepTree == 0) {
    UINT tmp = escapedValue(hbitBuffer, 5, 8, 16);
    if (tmp > MAX_NUM_MCT_BOXES) {
      return 1;
    }
    work->numPairs = (UCHAR)tmp;
  }

  for (i = 0; i < work->numPairs; i++) {
    if (MCTStereoFilling) {
      work->hasStereoFilling[i] = FDKreadBit(hbitBuffer);
    } else {
      work->hasStereoFilling[i] = 0;
    }

    if (self->MCCSignalingType == 0) {
      retVal = Read_MultichannelCodingBox(self->MCCSignalingType, usacIndepFlag, hbitBuffer, self,
                                          work, i, huff_ctabscf, huff_ltabscf,
                                          HUFF_LTABSCF_MAX_VALUE, CODE_BOOK_ALPHA_LAV);
      if (retVal) {
        return retVal;
      }
    } else if (self->MCCSignalingType == 1) {
      Read_MultichannelCodingBox(self->MCCSignalingType, usacIndepFlag, hbitBuffer, self, work, i,
                                 huff_ctabAngle, huff_ltabAngle, HUFF_LTABANGLE_MAX_VALUE,
                                 CODE_BOOK_BETA_LAV);
      if (retVal) {
        return retVal;
      }
    }
  }

  self->MCTSignalingTypePrev = self->MCCSignalingType;

  return 0;
}

static void clean_sfb_band_MCT_STEFI_DMX(FIXP_DBL* outCoefficient, INT outCoefficient_exp,
                                         INT length) {
  /*Thresholds for testing. Thresholds created with maximum possible precision */

  //"1e-8"
  const FIXP_DBL value0_001 = 0x55e63b80;
  const INT value0_001_exp = -26;

  ////"1e-7"
  // const FIXP_DBL value0_001=0x6b5fca80;
  // const INT value0_001_exp=-23;

  ////"1e-6"
  // const FIXP_DBL value0_001=0x431bde80;
  // const INT value0_001_exp=-19;

  ////"1e-5"
  // const FIXP_DBL value0_001=0x53e2d600;
  // const INT value0_001_exp=-16;

  ////"1e-4"
  // const FIXP_DBL value0_001=0x68db8b80;
  // const INT value0_001_exp=-13;

  ////"1e-3"
  // const FIXP_DBL value0_001=0x41893780;
  // const INT value0_001_exp=-9;

  ////"1e-2"
  // const FIXP_DBL value0_001=0x51eb8500;
  // const INT value0_001_exp=-6;

  INT delta = outCoefficient_exp - value0_001_exp;

  if (delta >= 0) {
    delta = fMin(31, delta);
    FIXP_DBL CleanThreshold = value0_001 >> delta;
    if (CleanThreshold != (FIXP_DBL)0) {
      for (int i = 0; i < length; i++) {
        if (fAbs(outCoefficient[i]) < CleanThreshold) outCoefficient[i] = (FIXP_DBL)0;
      }
    }

  } else {
    FDKmemclear(outCoefficient, sizeof(FIXP_DBL) * length);
  }
}

static int inverseDpcmAngleCoding(CMctPtr self, SHORT pairCoeffQSfb[], SHORT* pairCoeffQFb,
                                  int pair, int bIsShortBlock, int windowsPerFrame) {
  int lastVal;
  int coeffQ;
  int band = 0;
  int band_mod_mctBandsPerWindow;
  int mctBandsPerWindow = 0;
  CMctWorkPtr work = self->mctWork;

  /* split transmitted mct bands into short blocks */
  if (!bIsShortBlock) {
    FDK_ASSERT(windowsPerFrame == 1);
  }
  mctBandsPerWindow = work->numMctMaskBands[pair] >>
                      ((windowsPerFrame == 1) ? 0 : 3); /* Short: divide by 8, Long: divide by 1 */

  if (self->MCCSignalingType == 0) {
    if (bIsShortBlock != self->windowSequenceIsShortPrev[pair]) {
      /* reset previous angles */
      self->pairCoeffQFbPrev[pair] = DEFAULT_ALPHA;
#if (DEFAULT_ALPHA != 0)
      for (band = 0; band < MAX_NUM_MCT_BANDS; band++) {
        self->pairCoeffQSfbPrev[pair][band] = DEFAULT_ALPHA;
      }
#else
      FDKmemclear(self->pairCoeffQSfbPrev[pair],
                  MAX_NUM_MCT_BANDS * sizeof(self->pairCoeffQSfbPrev[0][0]));
#endif
    }
    self->windowSequenceIsShortPrev[pair] = bIsShortBlock;

    if (work->bHasBandwiseCoeffs[pair] == 0) {
      /* use fullband coeff */
      if (work->bDeltaTime[pair] > 0) {
        lastVal = self->pairCoeffQFbPrev[pair];
      } else {
        lastVal = DEFAULT_ALPHA;
      }

      /* inverse dpcm */
      coeffQ = lastVal - work->pairCoeffDeltaFb[pair] + 60;

      self->pairCoeffQFbPrev[pair] = coeffQ;

      band_mod_mctBandsPerWindow = 0;
      for (band = 0; band < work->numMctMaskBands[pair]; band++) {
        /* set all coeffs to fullband coeff */
        pairCoeffQSfb[band] = coeffQ;

        /* set previous coeffs according to mctMask */
        if (READ_MASK(work->mctMask[pair], band) > 0) {
          self->pairCoeffQSfbPrev[pair][band_mod_mctBandsPerWindow] = coeffQ;
        } else {
          self->pairCoeffQSfbPrev[pair][band_mod_mctBandsPerWindow] = DEFAULT_ALPHA;
        }
        band_mod_mctBandsPerWindow++;
        if (band_mod_mctBandsPerWindow == mctBandsPerWindow) band_mod_mctBandsPerWindow = 0;
      }

    } else {
      /* use bandwise coeffs */
      band_mod_mctBandsPerWindow = 0;
      for (band = 0; band < work->numMctMaskBands[pair]; band++) {
        if (work->bDeltaTime[pair] > 0) {
          /* modulo operation to use previous short block of same frame, no effect for long blocks
           */
          lastVal = self->pairCoeffQSfbPrev[pair][band_mod_mctBandsPerWindow];
        } else {
          /* reset freqency differential per shortblock to avoid predicting from highest to lowest
           * freq */
          if (band_mod_mctBandsPerWindow == 0) {
            lastVal = DEFAULT_ALPHA;
          }
        }

        if (READ_MASK(work->mctMask[pair], band) > 0) {
          /* inverse dpcm */
          coeffQ = lastVal - work->pairCoeffDeltaSfb[pair][band] + 60;

          pairCoeffQSfb[band] = coeffQ;
          lastVal = coeffQ;
          /* shortblock modulo operation only for previous values */
          self->pairCoeffQSfbPrev[pair][band_mod_mctBandsPerWindow] = coeffQ;
        } else {
          /* reset coeffs for inactive bands */
          self->pairCoeffQSfbPrev[pair][band_mod_mctBandsPerWindow] = DEFAULT_ALPHA;
        }
        band_mod_mctBandsPerWindow++;
        if (band_mod_mctBandsPerWindow == mctBandsPerWindow) band_mod_mctBandsPerWindow = 0;
      }

      /* reset fullband coeff */
      self->pairCoeffQFbPrev[pair] = DEFAULT_ALPHA;
    }

    /* reset previous coeffs for bands above mct mask */
    for (band = mctBandsPerWindow; band < MAX_NUM_MCT_BANDS; band++) {
      self->pairCoeffQSfbPrev[pair][band] = DEFAULT_ALPHA;
    }
  } else if (self->MCCSignalingType == 1) {
    if (bIsShortBlock != self->windowSequenceIsShortPrev[pair]) {
      /* reset previous coeffs */
      self->pairCoeffQFbPrev[pair] = DEFAULT_BETA;
      for (band = 0; band < MAX_NUM_MCT_BANDS; band++) {
        self->pairCoeffQSfbPrev[pair][band] = DEFAULT_BETA;
      }
    }
    self->windowSequenceIsShortPrev[pair] = bIsShortBlock;

    if (work->bHasBandwiseCoeffs[pair] == 0) {
      /* use fullband coeff */
      if (work->bDeltaTime[pair] > 0) {
        lastVal = self->pairCoeffQFbPrev[pair];
      } else {
        lastVal = DEFAULT_BETA;
      }

      /* inverse dpcm */
      coeffQ = lastVal + work->pairCoeffDeltaFb[pair];

      /* wrap around */
      if (coeffQ >= CODE_BOOK_BETA_LAV) {
        coeffQ -= CODE_BOOK_BETA_LAV;
      }

      if ((coeffQ < 0) || (coeffQ >= CODE_BOOK_BETA_LAV)) {
        return -1;
      }

      *pairCoeffQFb = coeffQ;

      self->pairCoeffQFbPrev[pair] = coeffQ;

      band_mod_mctBandsPerWindow = 0;
      for (band = 0; band < work->numMctMaskBands[pair]; band++) {
        /* set all coeffs to fullband coeff */
        pairCoeffQSfb[band] = coeffQ;

        /* set previous coeffs according to mctMask */
        if (READ_MASK(work->mctMask[pair], band) > 0) {
          self->pairCoeffQSfbPrev[pair][band_mod_mctBandsPerWindow] = coeffQ;
        } else {
          self->pairCoeffQSfbPrev[pair][band_mod_mctBandsPerWindow] = DEFAULT_BETA;
        }
        band_mod_mctBandsPerWindow++;
        if (band_mod_mctBandsPerWindow == mctBandsPerWindow) band_mod_mctBandsPerWindow = 0;
      }

    } else {
      /* use bandwise coeffs */
      lastVal = DEFAULT_BETA;

      band_mod_mctBandsPerWindow = 0;
      for (band = 0; band < work->numMctMaskBands[pair]; band++) {
        if (work->bDeltaTime[pair] > 0) {
          /* modulo operation to use previous short block of same frame, no effect for long blocks
           */
          lastVal = self->pairCoeffQSfbPrev[pair][band_mod_mctBandsPerWindow];
        } else {
          /* reset freqency differential per shortblock to avoid predicting from highest to lowest
           * freq */
          if (band_mod_mctBandsPerWindow == 0) {
            lastVal = DEFAULT_BETA;
          }
        }

        if (READ_MASK(work->mctMask[pair], band) > 0) {
          /* inverse dpcm */
          coeffQ = lastVal + work->pairCoeffDeltaSfb[pair][band];

          /* wrap around */
          if (coeffQ >= CODE_BOOK_BETA_LAV) {
            coeffQ -= CODE_BOOK_BETA_LAV;
          }

          if ((coeffQ < 0) || (coeffQ >= CODE_BOOK_BETA_LAV)) {
            return -1;
          }

          pairCoeffQSfb[band] = coeffQ;
          lastVal = coeffQ;
          /* shortblock modulo operation only for previous values */
          self->pairCoeffQSfbPrev[pair][band_mod_mctBandsPerWindow] = coeffQ;
        } else {
          /* reset coeffs for inactive bands */
          self->pairCoeffQSfbPrev[pair][band_mod_mctBandsPerWindow] = DEFAULT_BETA;
        }
        band_mod_mctBandsPerWindow++;
        if (band_mod_mctBandsPerWindow == mctBandsPerWindow) band_mod_mctBandsPerWindow = 0;
      }

      /* reset fullband coeff */
      self->pairCoeffQFbPrev[pair] = DEFAULT_BETA;
    }

    /* reset previous coeffs for bands above mct mask */
    for (band = mctBandsPerWindow; band < MAX_NUM_MCT_BANDS; band++) {
      self->pairCoeffQSfbPrev[pair][band] = DEFAULT_BETA;
    }
  }

  return mctBandsPerWindow;
}

static inline void applyMctInverseRotationFrame(FIXP_DBL* left, int lScale, FIXP_DBL* right,
                                                int rScale, FIXP_DBL* prev_dmx, SHORT alphaQ,
                                                INT nSamples) {
  int n;
  FIXP_SGL SinAlpha, CosAlpha;

  CosAlpha = tabIndexToCosSinAlpha[alphaQ].v.re;
  SinAlpha = tabIndexToCosSinAlpha[alphaQ].v.im;

  for (n = 0; n < nSamples; n++) {
    *prev_dmx++ =
        (fMultDiv2((*left++), CosAlpha) >> lScale) + (fMultDiv2((*right++), SinAlpha) >> rScale);
  }
}

#ifndef FUNCTION_applyMctRotationIdx
static void applyMctRotationIdx(FIXP_DBL* dmx, SHORT* dmxExp, FIXP_DBL* res, SHORT* resExp,
                                SHORT alphaQ, INT nSamples) {
  int lScale, rScale;
  int temp = *dmxExp - *resExp;
  if (temp >= 0) {
    lScale = 0;
    rScale = temp;
  } else {
    rScale = 0;
    lScale = -temp;
  }

  int OutExp = *dmxExp + lScale + 1;

  lScale = fMin(31, lScale);
  rScale = fMin(31, rScale);

  FIXP_SGL SinAlpha, CosAlpha;

  CosAlpha = tabIndexToCosSinAlpha[alphaQ].v.re;
  SinAlpha = tabIndexToCosSinAlpha[alphaQ].v.im;

  if ((CosAlpha == (FIXP_SGL)0) && (SinAlpha > (FIXP_SGL)0)) {
    for (int n = 0; n < nSamples; n++) {
      FIXP_DBL temp_dmx = dmx[n];
      dmx[n] = -res[n];
      res[n] = temp_dmx;
    }

    SHORT temp_exp = *dmxExp;
    *dmxExp = *resExp;
    *resExp = temp_exp;
  } else if ((CosAlpha == (FIXP_SGL)0) && (SinAlpha < (FIXP_SGL)0)) {
    for (int n = 0; n < nSamples; n++) {
      FIXP_DBL temp_dmx = dmx[n];
      dmx[n] = res[n];
      res[n] = -temp_dmx;
    }

    SHORT temp_exp = *dmxExp;
    *dmxExp = *resExp;
    *resExp = temp_exp;
  } else if ((SinAlpha == (FIXP_SGL)0) && (CosAlpha > (FIXP_SGL)0)) {
    /*Do nothing*/
  } else {
#ifdef FUNCTION_applyMctRotationIdx_func1
    applyMctRotationIdx_func1(dmx, dmxExp, res, resExp, (INT)OutExp, nSamples, SinAlpha, CosAlpha,
                              (INT)lScale, (INT)rScale);
#else

    for (int n = 0; n < nSamples; n++) {
      FIXP_DBL temp_dmx = dmx[n];
      FIXP_DBL temp_res = res[n];
      dmx[n] =
          (fMultDiv2(temp_dmx, CosAlpha) >> lScale) - (fMultDiv2(temp_res, SinAlpha) >> rScale);
      res[n] =
          (fMultDiv2(temp_dmx, SinAlpha) >> lScale) + (fMultDiv2(temp_res, CosAlpha) >> rScale);
    }

    int headroom = getScalefactor(dmx, nSamples) - 1;
    scaleValues(dmx, nSamples, headroom);
    *dmxExp = OutExp - headroom;

    headroom = getScalefactor(res, nSamples) - 1;
    scaleValues(res, nSamples, headroom);
    *resExp = OutExp - headroom;
#endif
  }
}
#endif /* #ifndef FUNCTION_applyMctRotationIdx */

#ifndef FUNCTION_applyMctPrediction
static void applyMctPrediction(FIXP_DBL* dmx, SHORT* dmxExp, FIXP_DBL* res, SHORT* resExp,
                               SHORT alphaQ, int nSamples, int predDir) {
  int n;
  int lScale, rScale;

  /* 0.1 in Q-3.34 */
  const FIXP_DBL pointOne = 0x66666666;

  FIXP_DBL alpha_re_tmp = (FIXP_DBL)0;
  int alphaExp = 0;

  if (alphaQ != 0) {
    /* Find the minimum common headroom for alpha_re and alpha_im */
    int alpha_re_headroom = CountLeadingBits(((INT)alphaQ) << 16);

    /* Multiply alpha by 0.1 with maximum precision */
    alpha_re_tmp = fMult((FIXP_SGL)((FIXP_SGL)alphaQ << alpha_re_headroom), pointOne);

    /* Calculate shift exponent */
    /* (Q-3.34 *( Q15.0 - headroom)*/
    alphaExp = 15 - alpha_re_headroom + (-3);
  }

  /* Upmix process */
  INT igf_pred_dir = predDir ? -1 : 1;

  int temp = *dmxExp - *resExp;
  if (temp >= 0) {
    lScale = 2;
    rScale = 2 + temp;
  } else {
    rScale = 2;
    lScale = (-temp + 2);
  }

  if (alphaExp > 1) {
    lScale += (alphaExp - 1);
    rScale += (alphaExp - 1);
  }

  int OutExp = *dmxExp + lScale;

  int diff_scaling = lScale - alphaExp;

  diff_scaling = fMin(31, diff_scaling);
  lScale = fMin(31, lScale);
  rScale = fMin(31, rScale);

  FIXP_DBL* p2CoeffL = dmx;
  FIXP_DBL* p2CoeffR = res;

  for (n = 0; n < nSamples; n++) {
    FIXP_DBL tempR = *p2CoeffR;
    FIXP_DBL tempL = *p2CoeffL;

    /* alpha_re[i] * specL[i] */
    FIXP_DBL help1 = fMult(alpha_re_tmp, tempL) >> diff_scaling;

    tempL >>= lScale;
    tempR >>= rScale;

    /* side = specR[i] + alpha_re[i] * specL[i]*/
    FIXP_DBL help2 = tempR + help1;

    /* specR[i] = -/+ (specL[i] + side; */
    *p2CoeffR++ = (FIXP_DBL)((LONG)(tempL - help2) * (LONG)igf_pred_dir);

    /* specL[i] = specL[i] + side; */
    *p2CoeffL++ = tempL + help2;
  }

  int headroom = getScalefactor(dmx, nSamples);
  scaleValues(dmx, nSamples, headroom);
  *dmxExp = OutExp - headroom;

  headroom = getScalefactor(res, nSamples);
  scaleValues(res, nSamples, headroom);
  *resExp = OutExp - headroom;
}
#endif /* #ifndef applyMctPrediction */

static void applyMctRotationWrapper(const CMctPtr self, FIXP_DBL* RESTRICT dmx, SHORT* dmxSfbExp,
                                    FIXP_DBL* RESTRICT res, SHORT* resSfbExp, SHORT* alphaQSfb,
                                    const UINT64 mctMask, const int numMctBands, const SHORT alphaQ,
                                    const int totalSfb, const int pair, const SHORT* BandOffsets) {
  CMctWorkPtr work = self->mctWork;

  if (self->MCCSignalingType == 0) {
    /* apply fullband box */
    if (!work->bHasBandwiseCoeffs[pair] && !work->bHasMctMask[pair]) {
      SHORT alpha = alphaQSfb[0];
      INT predDir = (INT)work->predDir[pair];
#if defined(FUNCTION_applyMctRotationWrapper_func1)
      applyMctRotationWrapper_func1(dmx, dmxSfbExp, dmxSfbHR, res, resSfbExp, resSfbHR, BandOffsets,
                                    (INT)totalSfb, alpha, predDir);
#else
      for (int sfb = 0; sfb < totalSfb; sfb++) {
        INT startLine = BandOffsets[sfb];
        INT stopLine = BandOffsets[sfb + 1];
        INT nSamples = stopLine - startLine;
        applyMctPrediction(&dmx[startLine], &dmxSfbExp[sfb], &res[startLine], &resSfbExp[sfb],
                           alpha, nSamples, predDir);
      }
#endif
    } else {
      /* apply bandwise processing */
      int sfb_max = fMin(totalSfb, numMctBands * 2);
      int predDir = work->predDir[pair];
      for (int sfb = 0; sfb < sfb_max; sfb++) {
        int i = sfb / 2;
        if (READ_MASK(mctMask, i) == 0) continue;
        INT startLine = BandOffsets[sfb];
        INT stopLine = BandOffsets[sfb + 1];
        INT nSamples = stopLine - startLine;
        applyMctPrediction(&dmx[startLine], &dmxSfbExp[sfb], &res[startLine], &resSfbExp[sfb],
                           alphaQSfb[i], nSamples, predDir);
      }
    }
  } else if (self->MCCSignalingType == 1) {
    /* apply fullband box */
    if (!work->bHasBandwiseCoeffs[pair] && !work->bHasMctMask[pair]) {
#ifdef FUNCTION_applyMctRotationIdx_fullband_box
      applyMctRotationIdx_fullband_box(dmx, dmxSfbExp, res, resSfbExp, BandOffsets, totalSfb,
                                       alphaQSfb[0]);
#else
      SHORT alpha = alphaQSfb[0];
      for (int sfb = 0; sfb < totalSfb; sfb++) {
        INT startLine = BandOffsets[sfb];
        INT stopLine = BandOffsets[sfb + 1];
        INT nSamples = stopLine - startLine;
        applyMctRotationIdx(&dmx[startLine], &dmxSfbExp[sfb], &res[startLine], &resSfbExp[sfb],
                            alpha, nSamples);
      }
#endif
    } else {
      /* apply bandwise processing */
      int sfb_max = fMin(totalSfb, numMctBands * 2);
      for (int sfb = 0; sfb < sfb_max; sfb++) {
        int i = sfb / 2;
        if (READ_MASK(mctMask, i) == 0) continue;
        INT startLine = BandOffsets[sfb];
        INT stopLine = BandOffsets[sfb + 1];
        INT nSamples = stopLine - startLine;
        applyMctRotationIdx(&dmx[startLine], &dmxSfbExp[sfb], &res[startLine], &resSfbExp[sfb],
                            alphaQSfb[i], nSamples);
      }
    }
  }
}

static void CMct_StereoFilling_GetPreviousDmx(
    CMctPtr self, const SHORT pairCoeffQSfb[], const SHORT* pairCoeffQFb,
    CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo,
    CAacDecoderChannelInfo* pAacDecoderChannelInfo, FIXP_DBL* prevSpec1, SHORT* prevSpec1_exp,
    FIXP_DBL* prevSpec2, SHORT* prevSpec2_exp, FIXP_DBL* dmx_prev, SHORT* dmx_prev_win_exp,
    INT pair, const SHORT* pScaleFactorBandOffsets, const UCHAR* pWindowGroupLength,
    const int windowGroups, const INT max_noise_sfb, UCHAR* band_is_noise,
    const int applyPrediction) {
  int window, group;
  int noiseFillingStartOffset, sfb, nfStartOffset_sfb;
  int lScale, rScale, temp;
  FIXP_DBL *leftSpectrum, *rightSpectrum, *outSpectrum;
  CMctWorkPtr work = self->mctWork;

  noiseFillingStartOffset =
      (GetWindowSequence(&pAacDecoderChannelInfo->icsInfo) == BLOCK_SHORT) ? 20 : 160;

  /* determine sfb from where on noise filling is applied */
  for (sfb = 0; pScaleFactorBandOffsets[sfb] < noiseFillingStartOffset; sfb++)
    ;
  nfStartOffset_sfb = sfb;

  if (self->MCCSignalingType == 0) {
    if (applyPrediction) {
      for (window = 0, group = 0; group < windowGroups; group++) {
        for (int groupwin = 0; groupwin < pWindowGroupLength[group]; groupwin++, window++) {
          leftSpectrum = prevSpec1 + window * pAacDecoderChannelInfo->granuleLength;
          rightSpectrum = prevSpec2 + window * pAacDecoderChannelInfo->granuleLength;
          outSpectrum = dmx_prev + window * pAacDecoderChannelInfo->granuleLength;

          for (sfb = nfStartOffset_sfb; sfb < max_noise_sfb; sfb++) {
            /* Run algorithm only if the particular band has been fully noise filled, i.e. that it
              has been empty before the noise-filling process */
            if (!band_is_noise[group * 16 + sfb]) {
              continue;
            }

            /* Find if a difference between the scalings exists.*/
            /* Add 1 so that we multiply by 0.5 */
            temp = prevSpec1_exp[window * 16 + sfb] - prevSpec2_exp[window * 16 + sfb];
            if (temp >= 0) {
              lScale = 1;
              rScale = temp + 1;
              rScale = fMin((INT)rScale, 31);
              dmx_prev_win_exp[window * 16 + sfb] = prevSpec1_exp[window * 16 + sfb];
            } else {
              lScale = -temp + 1;
              lScale = fMin((INT)lScale, 31);
              rScale = 1;
              dmx_prev_win_exp[window * 16 + sfb] = prevSpec2_exp[window * 16 + sfb];
            }

            FIXP_DBL* leftCoefficient = &leftSpectrum[pScaleFactorBandOffsets[sfb]];
            FIXP_DBL* rightCoefficient = &rightSpectrum[pScaleFactorBandOffsets[sfb]];
            FIXP_DBL* outCoefficient = &outSpectrum[pScaleFactorBandOffsets[sfb]];

            for (int index = pScaleFactorBandOffsets[sfb]; index < pScaleFactorBandOffsets[sfb + 1];
                 index++) {
              FIXP_DBL temp_left, temp_right;

              /* MS output generation */
              temp_left = (*leftCoefficient++ >> lScale);
              temp_right = (*rightCoefficient++ >> rScale);

              if (work->predDir[pair] == 0) {
                *outCoefficient++ = temp_left + temp_right;
              } else {
                *outCoefficient++ = temp_left - temp_right;
              }
            }

          } /*for (band=0; band<scaleFactorBandsTransmitted; band++)*/
        }   /*  for (int groupwin=0; groupwin<pWindowGroupLength[group]; groupwin++, window++)*/
      }     /* window */
    }
  } /*  if(self->MCCSignalingType == 0) */

  else if (!work->bHasBandwiseCoeffs[pair] && !work->bHasMctMask[pair]) {
    SHORT alphaQ = pairCoeffQSfb[0];

    for (window = 0, group = 0; group < windowGroups; group++) {
      for (int groupwin = 0; groupwin < pWindowGroupLength[group]; groupwin++, window++) {
        leftSpectrum = prevSpec1 + window * pAacDecoderChannelInfo->granuleLength;
        rightSpectrum = prevSpec2 + window * pAacDecoderChannelInfo->granuleLength;
        outSpectrum = dmx_prev + window * pAacDecoderChannelInfo->granuleLength;

        for (sfb = nfStartOffset_sfb; sfb < max_noise_sfb; sfb++) {
          /* Run algorithm only if the particular band has been fully noise filled, i.e. that it has
          been empty before the noise-filling process */
          if (!band_is_noise[group * 16 + sfb]) {
            continue;
          }

          /* Find if a difference between the scalings exists. */
          FIXP_DBL* leftCoefficient = &leftSpectrum[pScaleFactorBandOffsets[sfb]];
          FIXP_DBL* rightCoefficient = &rightSpectrum[pScaleFactorBandOffsets[sfb]];
          FIXP_DBL* outCoefficient = &outSpectrum[pScaleFactorBandOffsets[sfb]];

          temp = prevSpec1_exp[window * 16 + sfb] - prevSpec2_exp[window * 16 + sfb];
          if (temp > 0) {
            lScale = 0;
            rScale = fMin(temp, 31);
            dmx_prev_win_exp[window * 16 + sfb] = prevSpec1_exp[window * 16 + sfb] + 1;
          } else {
            lScale = fMin(-temp, 31);
            rScale = 0;
            dmx_prev_win_exp[window * 16 + sfb] = prevSpec2_exp[window * 16 + sfb] + 1;
          }

          applyMctInverseRotationFrame(
              leftCoefficient, lScale, rightCoefficient, rScale, outCoefficient, alphaQ,
              (pScaleFactorBandOffsets[sfb + 1] - pScaleFactorBandOffsets[sfb]));

          clean_sfb_band_MCT_STEFI_DMX(
              outCoefficient, dmx_prev_win_exp[window * 16 + sfb],
              (pScaleFactorBandOffsets[sfb + 1] - pScaleFactorBandOffsets[sfb]));

        } /*for (band=0; band<scaleFactorBandsTransmitted; band++)*/
      }   /*  for (int groupwin=0; groupwin<pWindowGroupLength[group]; groupwin++, window++)*/
    }     /* window */

  } else {
    for (window = 0, group = 0; group < windowGroups; group++) {
      for (int groupwin = 0; groupwin < pWindowGroupLength[group]; groupwin++, window++) {
        leftSpectrum = prevSpec1 + window * pAacDecoderChannelInfo->granuleLength;
        rightSpectrum = prevSpec2 + window * pAacDecoderChannelInfo->granuleLength;
        outSpectrum = dmx_prev + window * pAacDecoderChannelInfo->granuleLength;

        for (sfb = nfStartOffset_sfb; sfb < max_noise_sfb; sfb++) {
          /* Run algorithm only if the particular band has been fully noise filled, i.e. that it has
          been empty before the noise-filling process */
          if (!band_is_noise[group * 16 + sfb]) {
            continue;
          }

          FIXP_DBL* leftCoefficient = &leftSpectrum[pScaleFactorBandOffsets[sfb]];
          FIXP_DBL* rightCoefficient = &rightSpectrum[pScaleFactorBandOffsets[sfb]];
          FIXP_DBL* outCoefficient = &outSpectrum[pScaleFactorBandOffsets[sfb]];

          if (READ_MASK(work->mctMask[pair], sfb >> 1)) {
            /* Find if a difference between the scalings exists. */
            temp = prevSpec1_exp[window * 16 + sfb] - prevSpec2_exp[window * 16 + sfb];
            if (temp > 0) {
              lScale = 0;
              rScale = fMin(temp, 31);
              dmx_prev_win_exp[window * 16 + sfb] = prevSpec1_exp[window * 16 + sfb] + 1;
            } else {
              lScale = fMin(-temp, 31);
              rScale = 0;
              dmx_prev_win_exp[window * 16 + sfb] = prevSpec2_exp[window * 16 + sfb] + 1;
            }

            SHORT alphaQ = pairCoeffQSfb[sfb >> 1];

            applyMctInverseRotationFrame(
                leftCoefficient, lScale, rightCoefficient, rScale, outCoefficient, alphaQ,
                (pScaleFactorBandOffsets[sfb + 1] - pScaleFactorBandOffsets[sfb]));

            clean_sfb_band_MCT_STEFI_DMX(
                outCoefficient, dmx_prev_win_exp[window * 16 + sfb],
                (pScaleFactorBandOffsets[sfb + 1] - pScaleFactorBandOffsets[sfb]));

          } else {
            int width = pScaleFactorBandOffsets[sfb + 1] - pScaleFactorBandOffsets[sfb];

            FDKmemcpy(outCoefficient, leftCoefficient, sizeof(FIXP_DBL) * width);

            band_is_noise[group * 16 + sfb] = 0;

            dmx_prev_win_exp[window * 16 + sfb] = prevSpec1_exp[window * 16 + sfb];
          }

        } /*for (band=0; band<scaleFactorBandsTransmitted; band++)*/
      }   /*  for (int groupwin=0; groupwin<pWindowGroupLength[group]; groupwin++, window++)*/
    }     /* window */
  }

  /* MS stereo */
}

static void CMct_StereoFilling_Calc_Energy_SFBwise(CAacDecoderChannelInfo* chInfo,
                                                   CIcsInfo* icsInfo,
                                                   SamplingRateInfo* samplingRateInfo,
                                                   FIXP_DBL* Energy, INT* Energy_exp) {
  FIXP_DBL dmx_energy = 0;

  int noiseFillingStartOffset, sfb, nfStartOffset_sfb;
  int EndOffset, EndOffset_sfb;

  noiseFillingStartOffset = 160;

  /* determine sfb from where on noise filling is applied */
  const SHORT* pScaleFactorBandOffsets = GetScaleFactorBandOffsets(icsInfo, samplingRateInfo);
  for (sfb = 0; pScaleFactorBandOffsets[sfb] < noiseFillingStartOffset; sfb++)
    ;
  nfStartOffset_sfb = sfb;

  EndOffset = 1024 >> 1;  //!!
  for (sfb = 0; pScaleFactorBandOffsets[sfb] < EndOffset; sfb++)
    ;
  EndOffset_sfb = sfb;

  /* Get the sfb scale data */
  SHORT* p2_exponent_dmx = &(chInfo->pDynData->aSfbScale[nfStartOffset_sfb]);

  /*Find a suitable headroom for subsequent additions */
  INT sfb_width =
      pScaleFactorBandOffsets[nfStartOffset_sfb + 1] - pScaleFactorBandOffsets[nfStartOffset_sfb];
  int sfb_shift = 0;
  while (1) {
    INT temp = (INT)1 << sfb_shift;
    if (temp >= sfb_width)
      break;
    else {
      sfb_shift++;
    }
  }

  int max_scale_factor_dmx = (*p2_exponent_dmx << 1) + sfb_shift + 1;

  for (sfb = nfStartOffset_sfb; sfb < EndOffset_sfb; sfb++) {
    INT bin_start = pScaleFactorBandOffsets[sfb];
    INT bin_stop = pScaleFactorBandOffsets[sfb + 1];
    INT width = bin_stop - bin_start;

    /*Find a suitable headroom for subsequent additions */
    int shift = 0;
    while (1) {
      INT temp = (INT)1 << shift;
      if (temp >= width)
        break;
      else {
        shift++;
      }
    }

    FIXP_DBL* p2dmx = &(chInfo->pSpectralCoefficient[bin_start]);
    FIXP_DBL temp_dmx_energy = 0;
    for (int bin = 0; bin < width; bin++) {
      temp_dmx_energy += (fPow2Div2(*p2dmx++) >> shift);
    }

    if (temp_dmx_energy != (FIXP_DBL)0) {
      int energy_adjust_shift = (*p2_exponent_dmx << 1) + shift + 1;
      int diff = 0;

      if (energy_adjust_shift > max_scale_factor_dmx) {
        diff = fMin((energy_adjust_shift - max_scale_factor_dmx), 31);
        dmx_energy >>= diff;
        dmx_energy += temp_dmx_energy;
        max_scale_factor_dmx = energy_adjust_shift;
      } else {
        diff = fMin((max_scale_factor_dmx - energy_adjust_shift), 31);
        temp_dmx_energy >>= diff;
        dmx_energy += temp_dmx_energy;
      }

      /* Adjust not to overflow */
      if (CntLeadingZeros(dmx_energy) == 1) {
        dmx_energy >>= 1;
        max_scale_factor_dmx += 1;
      }

    } /* if(temp_dmx_energy!=(FIXP_DBL)0) */

    p2_exponent_dmx++;

  } /* for (sfb= nfStartOffset_sfb; sfb < EndOffset_sfb; sfb++) */

  *Energy = dmx_energy;
  *Energy_exp = max_scale_factor_dmx;
}

static void CMct_StereoFilling_SyncRNGs(CAacDecoderStaticChannelInfo* stChInfo1,
                                        CAacDecoderStaticChannelInfo* stChInfo2,
                                        CAacDecoderChannelInfo* chInfo1,
                                        CAacDecoderChannelInfo* chInfo2, CIcsInfo* icsInfo1,
                                        CIcsInfo* icsInfo2, SamplingRateInfo* samplingRateInfo) {
  /* Whitening is applied only to long frames */
  if (IsLongBlock(icsInfo1) && IsLongBlock(icsInfo2)) {
    /* Check if Strong whitening is used in some of the tiles */
    INT NumOfTiles = iisIGFDecLibGetNumberOfTiles(&stChInfo2->IGF_StaticData, 0);
    INT SyncRNGflag = 0;
    for (int i = 0; i < NumOfTiles; i++) {
      WHITENING_LEVEL Level = GetTileWhiteningLevel(&chInfo2->IGFdata, i);
      if (Level == WHITENING_STRONG) {
        SyncRNGflag = 1;
        break;
      }
    }

    /* When Strong whitening is used syncronize the RNGs */
    if (SyncRNGflag == (INT)1) {
      FIXP_DBL Energy_dmx, Energy_res;
      INT Energy_dmx_exp, Energy_res_exp;

      CMct_StereoFilling_Calc_Energy_SFBwise(chInfo1, icsInfo1, samplingRateInfo, &Energy_dmx,
                                             &Energy_dmx_exp);
      CMct_StereoFilling_Calc_Energy_SFBwise(chInfo2, icsInfo2, samplingRateInfo, &Energy_res,
                                             &Energy_res_exp);

      /* Allign exponents of dmx and res */
      int diff = Energy_dmx_exp - Energy_res_exp;
      if (diff >= 0) {
        diff = fMin(diff, 31);
        Energy_res >>= diff;
      } else {
        diff = fMin(-diff, 31);
        Energy_dmx >>= diff;
      }

      /* if (enDmx * 0.125f > enRes) */
      Energy_dmx >>= 3;
      if (Energy_dmx > Energy_res) {
        stChInfo2->nfRandomSeed = stChInfo1->nfRandomSeed;
      }

    } /* if(SyncRNGflag ==(INT)1) */

  } /* if(IsLongBlock(icsInfo1) && IsLongBlock(icsInfo2)) */
}

static void MCT_StereoFilling(FIXP_DBL* pSpec, SHORT* pSpec_exp,
                              CAacDecoderChannelInfo* pAacDecoderChannelInfo,
                              SamplingRateInfo* pSamplingRateInfo, const INT max_noise_sfb,
                              FIXP_DBL* dmx_prev_modified, SHORT* dmx_prev_modified_exp,
                              UCHAR* band_is_noise) {
  /* Comment:
      This algorithm does not work the same way as the Reference software. This is due to the
     datatypes being available as input. The RefSoft takes the scaled input, unscales it, works with
     un-scaled data and finally scales the data back with the orginal scaling factor. The present
     implemenation avoids this scaling/rescaling by working with scaled input data altogether. This
     leads to different computational values throughout the algorithm. In fact direct one to one
     intermediate value matching between RefSoft and the present one will be impossible. Just the
     output values will coincide. */

  const SHORT* swb_offset =
      GetScaleFactorBandOffsets(&pAacDecoderChannelInfo->icsInfo, pSamplingRateInfo);
  int sfb, noiseFillingStartOffset, nfStartOffset_sfb;

  /* Only for long frames */
  noiseFillingStartOffset = 160;

  /* determine sfb from where on noise filling is applied */
  for (sfb = 0; swb_offset[sfb] < noiseFillingStartOffset; sfb++)
    ;
  nfStartOffset_sfb = sfb;

  for (sfb = nfStartOffset_sfb; sfb < max_noise_sfb; sfb++) {
    /* Run algorithm only if the particular band has been fully noise filled, i.e. that it has been
     empty before the noise-filling process */
    if (!band_is_noise[sfb]) {
      continue;
    }

    short bin_start = swb_offset[sfb];
    short bin_stop = swb_offset[sfb + 1];

    /* Calculate the widht of the particular sfb */
    short sfb_width = bin_stop - bin_start;

    /* Find a suitable shift for dmx addition */
    const int shift = DFRACT_BITS - fNormz((FIXP_DBL)sfb_width);

    INT head_shift_sfb = getScalefactor(&pSpec[bin_start], sfb_width);

    /* Calculate the energy in the particular SFB */
    FIXP_DBL enRes = (FIXP_DBL)0;
    INT enRes_e = 0;
    if (head_shift_sfb != 31) {
      for (int bin = bin_start; bin < bin_stop; bin++) {
        enRes += (fPow2Div2(pSpec[bin] << head_shift_sfb) >> shift);
      } /* for (bin...) */

      enRes_e = (((INT)pSpec_exp[sfb] - head_shift_sfb) << 1) + 1 + shift + 4;
    }

    /* Generate const "gain * 1" */
    int lsb = pAacDecoderChannelInfo->pDynData->aScaleFactor[sfb] & 3;
    int msb = pAacDecoderChannelInfo->pDynData->aScaleFactor[sfb] >> 2;
    FIXP_DBL Scaled_One = MantissaTable[lsb][0];
    SHORT Scaled_One_exp = msb + 1;

    /* Calculate the energy of 1 */
    FIXP_DBL Energy_of_One = fPow2(Scaled_One);

    /* Multiply spectrum by 4 implicitly */
    pSpec_exp[sfb] += 2;

    /* Calculate enTarget. It is the lenght of the sfb mutiplied by the scaled energy */
    INT length_bits = CountLeadingBits(((LONG)sfb_width << 16));
    INT length_e = 15 - length_bits;
    FIXP_DBL enTarget = fMult(Energy_of_One, (FIXP_SGL)((FIXP_SGL)sfb_width << length_bits));
    INT enTarget_e = (Scaled_One_exp << 1) + length_e;

    /* Alling the exponents of the two energy values*/
    INT delta = enTarget_e - enRes_e;

    INT Energy_Difference_e;
    if (delta > 0) {
      delta = fMin(31, delta);
      enRes >>= delta;
      Energy_Difference_e = enTarget_e;
      enRes_e = enTarget_e;
    } else {
      delta = -delta;
      delta = fMin(31, delta);
      enTarget >>= delta;
      Energy_Difference_e = enRes_e;
      enTarget_e = enRes_e;
    }

    /* Subtract the two energy values */
    FIXP_DBL Energy_Difference = enTarget - enRes;

    if (Energy_Difference > (FIXP_DBL)0) {
      /* Get the previous downmix of the particular window */
      FIXP_DBL* dmx_prev = dmx_prev_modified;

      FIXP_DBL energy_dmx = (FIXP_DBL)0;
      INT energy_dmx_e = 0;

      /* Find the available headroom of the downmix signal and a suitable shift value*/
      INT head_shift_dmx = getScalefactor(&dmx_prev[bin_start], sfb_width);

      /* Calculate the energy of the previous downmix. If it is non-zero, then the
      downmix will be scaled and added to the signal.*/
      if (head_shift_dmx != 31) {
        /* "1e-8" */
        const FIXP_DBL value0_001 = 0x55e63b80;
        const INT value0_001_exp = -26;

        /* Calculate "energy_dmx" exponent of the particular window */
        INT dmx_exp = (((INT)dmx_prev_modified_exp[sfb] - head_shift_dmx) << 1) + 1 + shift;

        delta = dmx_exp - value0_001_exp;

        /*If the energy is comparable to the threshold, then calculate it.
        Otherwise, simply neglect it (set it to zero)*/
        if (delta > 0) {
          for (int bin = bin_start; bin < bin_stop; bin++) {
            energy_dmx += (fPow2Div2(dmx_prev[bin] << head_shift_dmx) >> shift);
          } /* for (bin...) */

          delta = fMin(31, delta);
          energy_dmx += (value0_001 >> delta);
          energy_dmx_e = dmx_exp;
        }
      }

      if (energy_dmx > (FIXP_DBL)0) {
        int temp_int;

        /* Divide by energy_sfb. Calculate (enTarget - enRes) / energy_dmx */
        FIXP_DBL temp_FIXP_DBL = fDivNorm(Energy_Difference, energy_dmx, &temp_int);

        /* Correct the output exponent */
        temp_int += Energy_Difference_e - energy_dmx_e;

        /* Calculate sqrt((enTarget - enRes) / energy_dmx) */
        FIXP_DBL tmp = sqrtFixp_lookup(temp_FIXP_DBL, &temp_int);

        /* We have to limit the gain we compute according to the RefSoft to 10. In RefSoft
        however they use unscaled data  while we use scaled data. Therefore we have to
        compare to a scaled threshold, i.e. 10* "Scaled_One" */
        const FIXP_SGL Ten = 0x5000;
        const INT Ten_exp = 4;
        FIXP_DBL Ten_Scaled = fMult(Ten, Scaled_One);
        INT Ten_Scaled_exp = Ten_exp + Scaled_One_exp;

        /* Temporary values for comparison purposes */
        FIXP_DBL temp_Ten_Scaled = Ten_Scaled;
        FIXP_DBL temp_tmp = tmp;

        /* We align the values to the same exponent */
        delta = temp_int - Ten_Scaled_exp;
        if (delta >= 0) {
          delta = fMin(31, delta);
          temp_Ten_Scaled >>= delta;
        } else {
          delta = -delta;
          delta = fMin(31, delta);
          temp_tmp >>= delta;
        }
        /* Compare and limit if necessary */
        if (temp_tmp > temp_Ten_Scaled) {
          tmp = Ten_Scaled;
          temp_int = Ten_Scaled_exp;
        }

        /* Get the previous downmix of the particular window */
        dmx_prev = dmx_prev_modified;

        /* Modify the downmix so that it can be added to the spectrum later: "dmx_prev*tmp" */
        for (int bin = bin_start; bin < bin_stop; bin++) {
          dmx_prev[bin] = fMultDiv2(dmx_prev[bin] << head_shift_dmx, tmp);
        } /* for (bin...) */

        /* "dmx_prev*tmp" exponent for every sfb and window */
        dmx_prev_modified_exp[sfb] = (dmx_prev_modified_exp[sfb] - head_shift_dmx + 1) + temp_int;

        /*Adjust exponents. Add and compute new energy*/
        int diff_1 = dmx_prev_modified_exp[sfb] - pSpec_exp[sfb];
        /* Adjust binary point and add. Calculate energy after addition of the dmx */
        enRes = (FIXP_DBL)0;
        if (diff_1 > 0) {
          diff_1 = fMin(31, diff_1);
          for (int bin = bin_start; bin < bin_stop; bin++) {
            FIXP_DBL temp = dmx_prev[bin] + (pSpec[bin] >> diff_1);
            pSpec[bin] = temp;
            enRes += fPow2(temp) >> shift;
          }
          pSpec_exp[sfb] = dmx_prev_modified_exp[sfb];
        } else {
          /* Shift right by 1 bit more to guarantee headroom for addition */
          diff_1 = -diff_1 + 1;
          diff_1 = fMin(31, diff_1);
          for (int bin = bin_start; bin < bin_stop; bin++) {
            FIXP_DBL temp = (dmx_prev[bin] >> diff_1) + (pSpec[bin] >> 1);
            pSpec[bin] = temp;
            enRes += fPow2(temp) >> shift;
          }
          pSpec_exp[sfb] += 1;
        }

        enRes_e = (pSpec_exp[sfb] << 1) + shift;

      } /* if(energy_dmx != (FIXP_DBL)0) */

      if (enRes > (FIXP_DBL)0) {
        /*Calculate (enTarget / enRes) */
        int temp_int;
        FIXP_DBL temp_FIXP_DBL = fDivNorm(enTarget, enRes, &temp_int);
        temp_int += (enTarget_e - enRes_e);

        /*Calculate sqrt(enTarget / enRes) */
        FIXP_DBL tmp = sqrtFixp_lookup(temp_FIXP_DBL, &temp_int);

        /* We have to limit the gain we compute according to the RefSoft to 10. In RefSoft
        however they use unscaled data  while we use scaled data. Therefore we have to
        compare to a scaled threshold, i.e. 10* "Scaled_One" */
        const FIXP_SGL Ten = 0x5000;
        const INT Ten_exp = 4;
        FIXP_DBL Ten_Scaled = fMult(Ten, Scaled_One);
        INT Ten_Scaled_exp = Ten_exp + Scaled_One_exp;

        /* Temporary values for comparison purposes */
        FIXP_DBL temp_Ten_Scaled = Ten_Scaled;
        FIXP_DBL temp_tmp = tmp;

        /* We align the values to the same exponent */
        delta = temp_int - Ten_Scaled_exp;
        if (delta >= 0) {
          delta = fMin(31, delta);
          temp_Ten_Scaled >>= delta;
        } else {
          delta = -delta;
          delta = fMin(31, delta);
          temp_tmp >>= delta;
        }
        /* Compare and limit if necessary */
        if (temp_tmp > temp_Ten_Scaled) {
          tmp = Ten_Scaled;
          temp_int = Ten_Scaled_exp;
        }

        /* Stereo filling changes the dynamic range of the signal,despite keeping the energy of
        the signal constant. */
        INT head_shift = getScalefactor(&pSpec[bin_start], sfb_width);

        /* Modify spectrum */
        for (int bin = bin_start; bin < bin_stop; bin++) {
          pSpec[bin] = fMultDiv2(pSpec[bin] << head_shift, tmp);
        } /* for (bin...) */

        /* Calculate output exponent */
        pSpec_exp[sfb] = (pSpec_exp[sfb] - head_shift) + temp_int + 1;

      } /* if(enRes > 0) */

    } /* if(Energy_Difference > (FIXP_DBL)0) */

  } /* for (sfb...) */
}

int CMct_MCT_StereoFilling(CMctPtr self, CStreamInfo* streamInfo,
                           CAacDecoderChannelInfo** pAacDecoderChannelInfo,
                           CAacDecoderStaticChannelInfo** pAacDecoderStaticChannelInfo,
                           SamplingRateInfo* samplingRateInfo, const UINT* MCT_elFlags,
                           const UINT usacIndepFrame) {
  int ch1 = 0, ch2 = 0;
  int pair;
  SHORT alphaQ = 0;
  SHORT* alphaQSfb = NULL;

  CMctWorkPtr work = self->mctWork;

  UCHAR* chTag = self->channelMap;

  /* Sanity check */
  if (work->numPairs > MAX_NUM_MCT_BOXES) {
    return -1;
  }

  for (pair = 0; pair < work->numPairs; pair++) {
    int win, group, groupwin;
    int mctBandOffset = 0;
    int mctBandsPerWindow = work->numMctMaskBands[pair];
    int bIsShortBlock = 0;
    int windowsPerFrame;
    CAacDecoderChannelInfo *chInfo1, *chInfo2;
    CAacDecoderStaticChannelInfo *stChInfo1, *stChInfo2;
    CIcsInfo *icsInfo1, *icsInfo2;
    SHORT pairCoeffQSfb[MAX_NUM_MCT_BANDS];
    SHORT pairCoeffQFb;

    ch1 = work->codePairs[pair][0];
    ch2 = work->codePairs[pair][1];
    chInfo1 = pAacDecoderChannelInfo[chTag[ch1]];
    stChInfo1 = pAacDecoderStaticChannelInfo[chTag[ch1]];
    icsInfo1 = &(chInfo1->icsInfo);

    chInfo2 = pAacDecoderChannelInfo[chTag[ch2]];
    stChInfo2 = pAacDecoderStaticChannelInfo[chTag[ch2]];
    icsInfo2 = &(chInfo2->icsInfo);

    windowsPerFrame = GetWindowsPerFrame(icsInfo1);

    /* sanity check */
    if (windowsPerFrame != GetWindowsPerFrame(icsInfo2)) {
      return -1;
    }

    /* short block handling */
    if (windowsPerFrame > 1) {
      bIsShortBlock = 1;
    }

    mctBandsPerWindow = inverseDpcmAngleCoding(self, pairCoeffQSfb, &pairCoeffQFb, pair,
                                               bIsShortBlock, windowsPerFrame);

    if (mctBandsPerWindow < 0) {
      return -1;
    }

    if (work->hasStereoFilling[pair]) {
      /* Make sure the following code runs only for long blocks */
      if (bIsShortBlock == 0) {
        int zeroPrevOutSpec1 = ((icsInfo1->WindowSequence == BLOCK_SHORT) !=
                                (stChInfo1->concealmentInfo.windowSequence == BLOCK_SHORT));
        int zeroPrevOutSpec2 = ((icsInfo2->WindowSequence == BLOCK_SHORT) !=
                                (stChInfo2->concealmentInfo.windowSequence == BLOCK_SHORT));
        int bNoFrameMemory =
            (usacIndepFrame || (stChInfo1->concealmentInfo.lastRenderMode == AACDEC_RENDER_LPD) ||
             (stChInfo2->concealmentInfo.lastRenderMode == AACDEC_RENDER_LPD));

        UCHAR* band_is_noise = chInfo2->pDynData->band_is_noise;

        /* Clear downmix for the case when it is not needed */
        FIXP_DBL prevDmx[1024];
        SHORT prevDmx_exp[(8 * 16)];
        FIXP_DBL emptyBuffer[1024];
        SHORT emptyBuffer_exp[(8 * 16)];

        FDKmemclear(prevDmx, sizeof(FIXP_DBL) * 1024);
        FDKmemclear(prevDmx_exp, sizeof(SHORT) * (8 * 16));

        FIXP_DBL* prevSpec1 = &self->prevOutSpec[work->codePairs[pair][0] * 1024];
        FIXP_DBL* prevSpec2 = &self->prevOutSpec[work->codePairs[pair][1] * 1024];
        SHORT* prevSpec1_exp = &self->prevOutSpec_exp[work->codePairs[pair][0] * (8 * 16)];
        SHORT* prevSpec2_exp = &self->prevOutSpec_exp[work->codePairs[pair][1] * (8 * 16)];

        /* If one can not compute previous dmx from both of the previous elements */
        if ((zeroPrevOutSpec1 && zeroPrevOutSpec2) || bNoFrameMemory) {
          /* No previous DMX can be computed */

        }
        /* If one of the previous elements is available for computing previous dmx */
        else if (zeroPrevOutSpec1 || zeroPrevOutSpec2) {
          FDKmemclear(emptyBuffer, sizeof(FIXP_DBL) * 1024);
          FDKmemclear(emptyBuffer_exp, sizeof(SHORT) * (8 * 16));

          if (zeroPrevOutSpec1) {
            prevSpec1 = emptyBuffer;
            prevSpec1_exp = emptyBuffer_exp;
          } else {
            prevSpec2 = emptyBuffer;
            prevSpec2_exp = emptyBuffer_exp;
          }

          CMct_StereoFilling_GetPreviousDmx(
              self, pairCoeffQSfb, &pairCoeffQFb, stChInfo2, chInfo2, prevSpec1, prevSpec1_exp,
              prevSpec2, prevSpec2_exp, prevDmx, prevDmx_exp, pair,
              GetScaleFactorBandOffsets(icsInfo1, samplingRateInfo),
              GetWindowGroupLengthTable(icsInfo1), GetWindowGroups(icsInfo1), icsInfo2->MaxSfBands,
              band_is_noise, zeroPrevOutSpec1 && zeroPrevOutSpec2);

        }
        /* General case: both of the previous elements are available for computing previous dmx */
        else {
          CMct_StereoFilling_GetPreviousDmx(
              self, pairCoeffQSfb, &pairCoeffQFb, stChInfo2, chInfo2, prevSpec1, prevSpec1_exp,
              prevSpec2, prevSpec2_exp, prevDmx, prevDmx_exp, pair,
              GetScaleFactorBandOffsets(icsInfo1, samplingRateInfo),
              GetWindowGroupLengthTable(icsInfo1), GetWindowGroups(icsInfo1), icsInfo2->MaxSfBands,
              band_is_noise, 1);
        } /* else */

        /* PrevDMX is changed/modified during Stero Filling so it must be preserved for the
         * subsequent tile processing intact */
        FIXP_DBL* prevDmx_temp = emptyBuffer;      /*Reuse available buffer*/
        SHORT* prevDmx_exp_temp = emptyBuffer_exp; /*Reuse available buffer*/
        FDKmemcpy(prevDmx_temp, prevDmx, sizeof(FIXP_DBL) * 1024);
        FDKmemcpy(prevDmx_exp_temp, prevDmx_exp, sizeof(SHORT) * (8 * 16));

        /* Stereo Filling parameter calculation */
        MCT_StereoFilling(chInfo2->pSpectralCoefficient, chInfo2->pDynData->aSfbScale, chInfo2,
                          samplingRateInfo, icsInfo2->MaxSfBands, prevDmx, prevDmx_exp,
                          band_is_noise);

        if (MCT_elFlags[chTag[ch2]] & AC_EL_ENHANCED_NOISE) {
          /* Run only if INF is active */
          if (MCT_elFlags[chTag[ch2]] & AC_EL_IGF_USE_ENF) {
            /* Apply MCT Stereo Filling to tiles */
            int NumTiles = iisIGFDecLibGetNumberOfTiles(&stChInfo2->IGF_StaticData, 0);

            for (int tileIdx = 0; tileIdx < NumTiles; tileIdx++) {
              /* Get the spectrum of the particular tile */
              FIXP_DBL* p2_tile_spectrum = iisIGFDecLibAccessSourceSpectrum(
                  &stChInfo2->IGF_StaticData, &chInfo2->IGFdata, tileIdx, 0, 0);

              /* Get pointer to the exponent of the particular tile */
              SHORT* p2_tile_spectrum_exp =
                  iisIGFDecLibAccessSourceSpectrum_exponent(&stChInfo2->IGF_StaticData, tileIdx, 0);

              /* Use the stored prevDmx for Stereo Filing */
              FDKmemcpy(prevDmx, prevDmx_temp, sizeof(FIXP_DBL) * 1024);
              FDKmemcpy(prevDmx_exp, prevDmx_exp_temp, sizeof(SHORT) * 128);

              /* Apply MCT Steffi over the particular tile */
              MCT_StereoFilling(p2_tile_spectrum, p2_tile_spectrum_exp, chInfo2, samplingRateInfo,
                                icsInfo2->MaxSfBands, prevDmx, prevDmx_exp, band_is_noise);
            }

          } /* if(MCT_elFlags[chTag[ch2]] & AC_EL_IGF_USE_ENF) */

          CMct_StereoFilling_SyncRNGs(stChInfo1, stChInfo2, chInfo1, chInfo2, icsInfo1, icsInfo2,
                                      samplingRateInfo);

        } /* if(MCT_elFlags[chTag[ch2]] & AC_EL_ENHANCED_NOISE) */
        FDKmemclear(band_is_noise, (8 * 16));

      } /* if(bIsShortBlock == 0) */

    } /* if(work->hasStereoFilling[pair]) */

    alphaQ = pairCoeffQFb;
    alphaQSfb = pairCoeffQSfb;

    icsInfo1->MaxSfBands = icsInfo2->MaxSfBands = fMax(icsInfo1->MaxSfBands, icsInfo2->MaxSfBands);

    /* inverse MCT rotation */
    for (win = 0, group = 0; group < icsInfo1->WindowGroups; group++) {
      for (groupwin = 0; groupwin < icsInfo1->WindowGroupLength[group]; groupwin++, win++) {
        int nSamples =
            (windowsPerFrame > 1) ? chInfo1->granuleLength : streamInfo->aacSamplesPerFrame;

        FIXP_DBL* dmx = chInfo1->pSpectralCoefficient + win * nSamples;
        FIXP_DBL* res = chInfo2->pSpectralCoefficient + win * nSamples;
        int totalSfb = icsInfo1->TotalSfBands;
        const SHORT* BandOffsets = GetScaleFactorBandOffsets(icsInfo1, samplingRateInfo);

        SHORT* dmxSfbExp = &(chInfo1->pDynData->aSfbScale[win * 16]);
        SHORT* resSfbExp = &(chInfo2->pDynData->aSfbScale[win * 16]);

        applyMctRotationWrapper(self, dmx, dmxSfbExp, res, resSfbExp, &alphaQSfb[mctBandOffset],
                                work->mctMask[pair] << mctBandOffset, mctBandsPerWindow, alphaQ,
                                totalSfb, pair, BandOffsets);

        if ((MCT_elFlags[chTag[ch1]] & AC_EL_ENHANCED_NOISE) &&
            (MCT_elFlags[chTag[ch2]] & AC_EL_ENHANCED_NOISE)) {
          /* Run only if INF is active */
          if ((MCT_elFlags[chTag[ch1]] & AC_EL_IGF_USE_ENF) &&
              (MCT_elFlags[chTag[ch2]] & AC_EL_IGF_USE_ENF)) {
            /* Apply MCT Stereo Filling to tiles */
            int NumTiles = iisIGFDecLibGetNumberOfTiles(&stChInfo2->IGF_StaticData, 0);

            for (int tileIdx = 0; tileIdx < NumTiles; tileIdx++) {
              dmx = iisIGFDecLibAccessSourceSpectrum(&stChInfo1->IGF_StaticData, &chInfo1->IGFdata,
                                                     tileIdx,
                                                     (icsInfo1->WindowSequence == 2) ? 1 : 0, win);

              dmxSfbExp = iisIGFDecLibAccessSourceSpectrum_exponent(
                  &stChInfo1->IGF_StaticData, tileIdx, (icsInfo1->WindowSequence == 2) ? 1 : 0);

              res = iisIGFDecLibAccessSourceSpectrum(&stChInfo2->IGF_StaticData, &chInfo2->IGFdata,
                                                     tileIdx,
                                                     (icsInfo2->WindowSequence == 2) ? 1 : 0, win);

              resSfbExp = iisIGFDecLibAccessSourceSpectrum_exponent(
                  &stChInfo2->IGF_StaticData, tileIdx, (icsInfo2->WindowSequence == 2) ? 1 : 0);

              if ((NULL == dmx) || (NULL == res)) {
                break; /* exit while loop, continue with next win */
              }

              /* apply mct */
              applyMctRotationWrapper(self, dmx, &dmxSfbExp[win * 16], res, &resSfbExp[win * 16],
                                      &alphaQSfb[mctBandOffset],
                                      work->mctMask[pair] << mctBandOffset, mctBandsPerWindow,
                                      alphaQ, totalSfb, pair, BandOffsets);

            } /* for(int tileIdx=0;tileIdx<NumTiles;tileIdx++) */

          } /* if(MCT_elFlags[chTag[ch2]] & AC_EL_IGF_USE_ENF) */

        } /* if(MCT_elFlags[chTag[ch2]] & AC_EL_ENHANCED_NOISE) */

        mctBandOffset += mctBandsPerWindow;
      } /* for win=... */

    } /* for group=... */
  }

  return 0;
}

void CMct_StereoFilling_save_prev(CMctPtr self, CAacDecoderChannelInfo** pAacDecoderChannelInfo) {
  CAacDecoderChannelInfo* chInfo;

  UCHAR* chTag = self->channelMap;

  for (int i = 0; i < self->numMctChannels; i++) {
    chInfo = pAacDecoderChannelInfo[chTag[i]];
    FDKmemcpy(&self->prevOutSpec[i * 1024], chInfo->pSpectralCoefficient, 1024 * sizeof(FIXP_DBL));
    FDKmemcpy(&self->prevOutSpec_exp[i * (8 * 16)], chInfo->pDynData->aSfbScale,
              (8 * 16) * sizeof(SHORT));
  }
}

void CMct_StereoFilling_clear_prev(CMctPtr self, CAacDecoderChannelInfo** pAacDecoderChannelInfo) {
  FDKmemclear(&self->prevOutSpec[0], self->numMctChannels * 1024 * sizeof(FIXP_DBL));
  FDKmemclear(&self->prevOutSpec_exp[0], self->numMctChannels * (8 * 16) * sizeof(SHORT));
}
