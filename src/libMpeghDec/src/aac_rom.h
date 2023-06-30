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

   Author(s):   Josef Hoepfl

   Description: Definition of constant tables

*******************************************************************************/

#ifndef AAC_ROM_H
#define AAC_ROM_H

#include "common_fix.h"
#include "FDK_audio.h"

#define PCM_AAC LONG
#define PCM_DEC FIXP_DBL
#define MAXVAL_PCM_DEC MAXVAL_DBL
#define MINVAL_PCM_DEC MINVAL_DBL
#define FIXP_DBL2PCM_DEC(x) (x)
#define PCM_DEC2FIXP_DBL(x) (x)
#define PCM_DEC_BITS DFRACT_BITS
#define PCM_DEC2FX_PCM(x) FX_DBL2FX_PCM(x)
#define FX_PCM2PCM_DEC(x) FX_PCM2FX_DBL(x)

#define AAC_NF_NO_RANDOM_VAL 512 /*!< Size of random number array for noise floor */

#define INV_QUANT_TABLESIZE (1024)

extern const FIXP_DBL InverseQuantTableMant[INV_QUANT_TABLESIZE + 1];
extern const UCHAR InverseQuantTableExp[INV_QUANT_TABLESIZE + 1];
extern const FIXP_DBL InverseQuantInterpolationTable[8];
extern const FIXP_DBL a2ToPow0p25Mant[4];
extern const INT a2ToPow0p25Exp;
extern const INT InverseQuantInterpolationExp;
extern const FIXP_DBL MantissaTable[4][14];

typedef struct {
  const SHORT* sfbOffsetLong;
  const SHORT* sfbOffsetShort;
  UCHAR numberOfSfbLong;
  UCHAR numberOfSfbShort;
} SFB_INFO;

extern const SFB_INFO sfbOffsetTables[5][16];

/* Huffman tables */
enum { HuffmanBits = 2, HuffmanEntries = (1 << HuffmanBits) };

typedef struct {
  const USHORT (*CodeBook)[HuffmanEntries];
  UCHAR Dimension;
  UCHAR numBits;
  UCHAR Offset;
} CodeBookDescription;

extern const CodeBookDescription AACcodeBookDescriptionSCL;

extern const UCHAR tns_max_bands_tbl[13][2];

#define FIXP_TCC FIXP_DBL

extern const FIXP_TCC FDKaacDec_tnsCoeff3[8];
extern const FIXP_TCC FDKaacDec_tnsCoeff4[16];

extern const USHORT AacDec_randomSign[AAC_NF_NO_RANDOM_VAL / 16];

/* Channel mapping indices for time domain I/O.
   The first dimension is the channel configuration index. */
extern const UCHAR channelMappingTablePassthrough[15][24];
extern const UCHAR channelMappingTableWAV[15][24];

#define SF_FNA_COEFFS 1 /* Compile-time prescaler for MDST-filter coefficients. */
/* SF_FNA_COEFFS > 0 should only be considered for FIXP_DBL-coefficients  */
/* (i.e. if CPLX_PRED_FILTER_16BIT is not defined).                       */
/* With FIXP_DBL loss of precision is possible for SF_FNA_COEFFS > 11.    */

#ifdef CPLX_PRED_FILTER_16BIT
#define FIXP_FILT FIXP_SGL
#define FILT(a) ((FL2FXCONST_SGL(a)) >> SF_FNA_COEFFS)
#else
#define FIXP_FILT FIXP_DBL
#define FILT(a) ((FL2FXCONST_DBL(a)) >> SF_FNA_COEFFS)
#endif

extern const FIXP_FILT
    mdst_filt_coef_curr[20][3]; /* MDST-filter coefficient tables used for current window  */
extern const FIXP_FILT
    mdst_filt_coef_prev[6][4]; /* MDST-filter coefficient tables used for previous window */

#endif /* #ifndef AAC_ROM_H */
