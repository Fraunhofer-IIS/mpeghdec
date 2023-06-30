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

   Description: joint stereo processing

*******************************************************************************/

#ifndef STEREO_H
#define STEREO_H

#include "machine_type.h"
#include "FDK_bitstream.h"
#include "common_fix.h"

#define SFB_PER_PRED_BAND 2

#define SR_FNA_OUT \
  0 /* Additional scaling of the CJointStereo_filterAndAdd()-output to avoid overflows.    */
    /* The scaling factor can be set to 0 if the coefficients are prescaled appropriately. */
    /* Prescaling via factor SF_FNA_COEFFS is done at compile-time but should only be      */
    /* utilized if the coefficients are stored as FIXP_DBL. (cp. aac_rom.cpp/.h)           */

/* The NO_CPLX_PRED_BUGFIX-switch was introduced to enable decoding of conformance-streams in way
   that they are comparable to buggy reference-streams. This is established by storing the
   prediction direction for computation of the "downmix MDCT of previous frame". This is not
   standard compliant. Once correct reference-streams for complex-stereo-prediction are available
   this switch becomes obsolete.
*/
/*#define NO_CPLX_PRED_BUGFIX*/

enum { JointStereoMaximumGroups = 8, JointStereoMaximumBands = 64 };

typedef struct {
  UCHAR pred_dir;        // 0 = prediction from mid to side channel, 1 = vice versa
  UCHAR igf_pred_dir;    // 0 = prediction from mid to side channel, 1 = vice versa
  UCHAR complex_coef;    // 0 = alpha_q_im[x] is 0 for all prediction bands, 1 = alpha_q_im[x] is
                         // transmitted via bitstream
  UCHAR use_prev_frame;  // 0 = use current frame for MDST estimation, 1 = use current and previous
                         // frame

  SHORT alpha_q_re[JointStereoMaximumGroups][JointStereoMaximumBands];
  SHORT alpha_q_im[JointStereoMaximumGroups][JointStereoMaximumBands];
} CCplxPredictionData;

/* joint stereo scratch memory (valid for this frame) */
typedef struct {
  UCHAR MsMaskPresent;
  UCHAR MsUsed[JointStereoMaximumBands]; /*!< every arry element contains flags for up to 8 groups.
                                            this array is also utilized for complex stereo
                                            prediction. */

  UCHAR cplx_pred_flag; /* stereo complex prediction was signalled for this frame */

  UCHAR IGF_MsMaskPresent;
  UCHAR igf_cplx_pred_flag;

  /* The following array and variable are needed for the case  when INF is active */
  FIXP_DBL store_dmx_re_prev[1024];
  SHORT store_dmx_re_prev_e;

} CJointStereoData;

/* joint stereo persistent memory */
typedef struct {
  UCHAR clearSpectralCoeffs; /* indicates that the spectral coeffs must be cleared because the
                                transform splitting active flag of the left and right channel was
                                different */

  FIXP_DBL* scratchBuffer; /* pointer to scratch buffer */

  FIXP_DBL* spectralCoeffs[2]; /* spectral coefficients of this channel utilized by complex stereo
                                  prediction */
  SHORT* specScale[2];
  FIXP_DBL* scratchBuffer2; /* pointer to another scratch buffer */

  SHORT alpha_q_re_prev[JointStereoMaximumGroups][JointStereoMaximumBands];
  SHORT alpha_q_im_prev[JointStereoMaximumGroups][JointStereoMaximumBands];

  UCHAR winSeqPrev;
  UCHAR winShapePrev;
  UCHAR winGroupsPrev;

} CJointStereoPersistentData;

/*!
  \brief Read joint stereo data from bitstream

  The function reads joint stereo data from bitstream.

  \param bs bit stream handle data source.
  \param pJointStereoData pointer to stereo data structure to receive decoded data.
  \param windowGroups number of window groups.
  \param scaleFactorBandsTransmitted number of transmitted scalefactor bands.
  \param flags decoder flags

  \return  0 on success, -1 on error.
*/
int CJointStereo_Read(HANDLE_FDK_BITSTREAM bs, CJointStereoData* pJointStereoData,
                      const int windowGroups, const int scaleFactorBandsTransmitted,
                      const int max_sfb_ste_clear,
                      CJointStereoPersistentData* pJointStereoPersistentData,
                      CCplxPredictionData* cplxPredictionData, int cplxPredictionActiv,
                      int scaleFactorBandsTotal, int windowSequence, const UINT flags);

int CJointStereo_ReadIGF(HANDLE_FDK_BITSTREAM bs, CJointStereoData* pJointStereoData,
                         const int windowGroups, const int igfStartSfb, const int igfStopSfb,
                         const int igfUseHighRes,
                         CJointStereoPersistentData* pJointStereoPersistentData,
                         CCplxPredictionData* cplxPredictionData, int windowSequence,
                         const UINT flags);

#endif /* #ifndef STEREO_H */
