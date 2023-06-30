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

   Author(s):   Christian Griebel

   Description: Error concealment structs and types

*******************************************************************************/

#ifndef CONCEAL_TYPES_H
#define CONCEAL_TYPES_H

#include "machine_type.h"
#include "common_fix.h"

#define CONCEAL_MAX_NUM_FADE_FACTORS (32)

#define FIXP_CNCL FIXP_DBL
#define FL2FXCONST_CNCL FL2FXCONST_DBL
#define FX_DBL2FX_CNCL
#define FX_CNCL2FX_DBL
#define CNCL_FRACT_BITS DFRACT_BITS

/* Warning: Do not ever change these values. */
typedef enum {
  ConcealMethodNone = -1,
  ConcealMethodMute = 0,
  ConcealMethodNoise = 1,
  ConcealMethodInter = 2,
  ConcealMethodTonal = 3

} CConcealmentMethod;

typedef enum {
  ConcealState_Ok,
  ConcealState_Single,
  ConcealState_FadeIn,
  ConcealState_Mute,
  ConcealState_FadeOut

} CConcealmentState;

typedef struct {
  FIXP_SGL fadeOutFactor[CONCEAL_MAX_NUM_FADE_FACTORS];
  FIXP_SGL fadeInFactor[CONCEAL_MAX_NUM_FADE_FACTORS];

  CConcealmentMethod method;

  int numFadeOutFrames;
  int numFadeInFrames;
  int numMuteReleaseFrames;
  FIXP_DBL comfortNoiseLevel;

} CConcealParams;

typedef enum {
  FADE_TIMEDOMAIN_TOSPECTRALMUTE = 1,
  FADE_TIMEDOMAIN_FROMSPECTRALMUTE,
  FADE_TIMEDOMAIN
} TDfadingType;

typedef struct {
  CConcealParams* pConcealParams;

  FIXP_CNCL* spectralCoefficient;
  SHORT specScale[8];

  INT iRandomPhase;
  INT prevFrameOk[2];
  INT cntValidFrames;
  INT cntFadeFrames;   /* State for signal fade-in/out */
                       /* States for signal fade-out of frames with more than one window/subframe -
                         [0] used by Update CntFadeFrames mode of CConcealment_ApplyFadeOut, [1] used by FadeOut mode */
  int winGrpOffset[2]; /* State for signal fade-out of frames with more than one window/subframe */
  int attGrpOffset[2]; /* State for faster signal fade-out of frames with transient signal parts */

  SCHAR lastRenderMode;

  UCHAR windowShape;
  BLOCK_TYPE windowSequence;
  UCHAR lastWinGrpLen;

  CConcealmentState concealState;
  CConcealmentState concealState_old;
  FIXP_DBL fade_old;           /* last fading factor */
  TDfadingType lastFadingType; /* last fading type */

  UCHAR lastWindowGroups;
  UCHAR lastWindowGroupLength[8];

  ULONG TDNoiseSeed;
  PCM_DEC TDNoiseStates[3];
  FIXP_SGL TDNoiseCoef[3];
  FIXP_SGL TDNoiseAtt;

} CConcealmentInfo;

#endif /* #ifndef CONCEAL_TYPES_H */
