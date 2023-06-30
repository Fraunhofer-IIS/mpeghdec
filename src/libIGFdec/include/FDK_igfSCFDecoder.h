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

/********************** Intelligent gap filling library ************************

   Author(s): Youliy Ninov, Florin Ghido, Andreas Niedermeier

   Description:

*******************************************************************************/

#ifndef FDK_IGFSCFDECODER_H
#define FDK_IGFSCFDECODER_H

#include "FDK_bitstream.h"
#include "genericStds.h"

#include "ac_arith_coder.h"

typedef enum igf_scf_win_mode {
  IGF_SCF_LONG,
  IGF_SCF_SHORT,
  IGF_SCF_TCX_MEDIUM,
  IGF_SCF_TCX_LONG,
  IGF_SCF_CODEC_TRANSITION
} IGF_SCF_WIN_MODE;

typedef struct igfscfdec_private_data_struct {
  HANDLE_FDK_BITSTREAM hBs;
  Tastat st;
  SCHAR prevBuffer[112]; /* 64 (LB) + 3*16(SB and 2 TCX modes) */
  SCHAR* prevSB;
  SCHAR* prevLB;
  SCHAR* prevTCXmedium;
  SCHAR* prevTCXlong;
  SCHAR prevDLB;
  SCHAR prevDSB;
  SCHAR prevDTCXmedium;
  SCHAR prevDTCXlong;
  INT scfCountLongBlock;
  INT scfCountShortBlock;
  INT scfCountTCXBlock;
  SCHAR* sfe;
  INT t;
  INT lastFrameBlockType;

  INT arith_decode_first_symbol;
  INT high;
  INT low;
  INT bits_to_follow;
  INT value;
} IGFSCFDEC_PRIVATE_DATA, *IGFSCFDEC_PRIVATE_DATA_HANDLE;

/**********************************************************************/ /**
 definiton of public data for this module
 **************************************************************************/
typedef struct igfscfdec_public_data_struct {
  INT bitsReaded; /* after a call bitsReaded contains the number of bits consumed by the decoder */
} IGFSCFDEC_INSTANCE, *IGFSCFDEC_INSTANCE_HANDLE;

void iisIGFSCFDecLibInit(IGFSCFDEC_PRIVATE_DATA* hPrivateData, INT scfCountLongBlock,
                         INT scfCountShortBlock, INT scfCountTCXBlock);

int iisIGFSCFDecoderGetLastFrameWasShortBlock(
    IGFSCFDEC_PRIVATE_DATA* hPrivateData /**< inout: handle to public data or NULL in case there was
                                            no instance created */
);

void iisIGFSCFDecoderSetLastFrameWasShortBlock(
    IGFSCFDEC_PRIVATE_DATA* hPrivateData, /**< inout: handle to public data or NULL in case there
                                             was no instance created */
    int isShortBlock);

/**********************************************************************/ /**
 resets the internal decoder memory (context memory
 **************************************************************************/
void iisIGFSCFDecoderReset(IGFSCFDEC_PRIVATE_DATA* hPublicData);

/**********************************************************************/ /**
 main decoder function
 **************************************************************************/
void iisIGFSCFDecoderDecode(
    IGFSCFDEC_PRIVATE_DATA* hPrivateData, HANDLE_FDK_BITSTREAM hBStr,
    SCHAR* sfe, /**< out: ptr to an array which will contain the decoded quantized coefficients */
    INT indepFlag,    /**< in: if  1 on input the encoder will be forced to reset,
                         if  0 on input the encodder will be forced to encode without a reset */
    INT isShortBlock, /**< in: if  1 on input a short block data will be decompressed */
    INT noShortBlock, INT sfeReducePrecisionEnabled);

#endif
