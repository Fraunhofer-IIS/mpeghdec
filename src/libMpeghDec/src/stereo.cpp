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

#include "stereo.h"

#include "aac_rom.h"
#include "FDK_bitstream.h"
#include "channelinfo.h"
#include "FDK_audio.h"

#if defined(__arm__)
#include "arm/stereo_arm.cpp"
#endif

enum { L = 0, R = 1 };

#include "block.h"

int CJointStereo_Read(HANDLE_FDK_BITSTREAM bs, CJointStereoData* pJointStereoData,
                      const int windowGroups, const int scaleFactorBandsTransmitted,
                      const int max_sfb_ste_clear,
                      CJointStereoPersistentData* pJointStereoPersistentData,
                      CCplxPredictionData* cplxPredictionData, int cplxPredictionActiv,
                      int scaleFactorBandsTotal, int windowSequence, const UINT flags) {
  int group, band;

  pJointStereoData->MsMaskPresent = (UCHAR)FDKread2Bits(bs);

  FDKmemclear(pJointStereoData->MsUsed, scaleFactorBandsTransmitted * sizeof(UCHAR));

  pJointStereoData->cplx_pred_flag = 0;
  if (cplxPredictionActiv) {
    cplxPredictionData->pred_dir = 0;
    cplxPredictionData->complex_coef = 0;
    cplxPredictionData->use_prev_frame = 0;
    cplxPredictionData->igf_pred_dir = 0;
  }

  switch (pJointStereoData->MsMaskPresent) {
    case 0: /* no M/S */
      /* all flags are already cleared */
      break;

    case 1: /* read ms_used */
      for (group = 0; group < windowGroups; group++) {
        for (band = 0; band < scaleFactorBandsTransmitted; band++) {
          pJointStereoData->MsUsed[band] |= (FDKreadBit(bs) << group);
        }
      }
      break;

    case 2: /* full spectrum M/S */
      for (band = 0; band < scaleFactorBandsTransmitted; band++) {
        pJointStereoData->MsUsed[band] = 255; /* set all flags to 1 */
      }
      break;

    case 3:
      /* M/S coding is disabled, complex stereo prediction is enabled */
      if (flags & (AC_USAC | AC_RSVD50 | AC_MPEGH3DA)) {
        if (cplxPredictionActiv) { /* 'if (stereoConfigIndex == 0)' */

          pJointStereoData->cplx_pred_flag = 1;

          /* cplx_pred_data()  cp. ISO/IEC FDIS 23003-3:2011(E)  Table 26 */
          int cplx_pred_all = 0; /* local use only */
          cplx_pred_all = FDKreadBit(bs);

          if (cplx_pred_all) {
            for (group = 0; group < windowGroups; group++) {
              UCHAR groupmask = ((UCHAR)1 << group);
              for (band = 0; band < scaleFactorBandsTransmitted; band++) {
                pJointStereoData->MsUsed[band] |= groupmask;
              }
            }
          } else {
            for (group = 0; group < windowGroups; group++) {
              for (band = 0; band < scaleFactorBandsTransmitted; band += SFB_PER_PRED_BAND) {
                pJointStereoData->MsUsed[band] |= (FDKreadBit(bs) << group);
                if ((band + 1) < scaleFactorBandsTotal) {
                  pJointStereoData->MsUsed[band + 1] |=
                      (pJointStereoData->MsUsed[band] & ((UCHAR)1 << group));
                }
              }
            }
          }
        } else {
          return -1;
        }
      }
      break;
  }

  if (cplxPredictionActiv) {
    /* If all sfb are MS-ed then no complex prediction */
    if (pJointStereoData->MsMaskPresent == 3) {
      if (pJointStereoData->cplx_pred_flag) {
        int delta_code_time = 0;

        /* set pointer to Huffman codebooks */
        const CodeBookDescription* hcb = &AACcodeBookDescriptionSCL;
        /* set predictors to zero in case of a transition from long to short window sequences and
         * vice versa */
        if (((windowSequence == BLOCK_SHORT) &&
             (pJointStereoPersistentData->winSeqPrev != BLOCK_SHORT)) ||
            ((pJointStereoPersistentData->winSeqPrev == BLOCK_SHORT) &&
             (windowSequence != BLOCK_SHORT))) {
          FDKmemclear(pJointStereoPersistentData->alpha_q_re_prev,
                      JointStereoMaximumGroups * JointStereoMaximumBands * sizeof(SHORT));
          FDKmemclear(pJointStereoPersistentData->alpha_q_im_prev,
                      JointStereoMaximumGroups * JointStereoMaximumBands * sizeof(SHORT));
        }
        if (flags & AC_MPEGH3DA) {
          /* Clean the array up to the maximum SFB transmitted. The rest is not cleaned, because it
           * may be used by IGF complex prediction */
          for (group = 0; group < JointStereoMaximumGroups; group++) {
            for (band = 0; band < scaleFactorBandsTransmitted; band++) {
              cplxPredictionData->alpha_q_re[group][band] = 0;
              cplxPredictionData->alpha_q_im[group][band] = 0;
            }
          }
        } else {
          FDKmemclear(cplxPredictionData->alpha_q_re,
                      JointStereoMaximumGroups * JointStereoMaximumBands * sizeof(SHORT));
          FDKmemclear(cplxPredictionData->alpha_q_im,
                      JointStereoMaximumGroups * JointStereoMaximumBands * sizeof(SHORT));
        }

        /* 0 = mid->side prediction, 1 = side->mid prediction */
        cplxPredictionData->pred_dir = FDKreadBit(bs);
        cplxPredictionData->complex_coef = FDKreadBit(bs);

        if (cplxPredictionData->complex_coef) {
          if (flags & AC_INDEP) {
            cplxPredictionData->use_prev_frame = 0;
          } else {
            cplxPredictionData->use_prev_frame = FDKreadBit(bs);
          }
        }

        if (flags & AC_INDEP) {
          delta_code_time = 0;
        } else {
          delta_code_time = FDKreadBit(bs);
        }

        {
          int last_alpha_q_re = 0, last_alpha_q_im = 0;

          for (group = 0; group < windowGroups; group++) {
            for (band = 0; band < scaleFactorBandsTransmitted; band += SFB_PER_PRED_BAND) {
              if (delta_code_time == 1) {
                if (group > 0) {
                  last_alpha_q_re = cplxPredictionData->alpha_q_re[group - 1][band];
                  last_alpha_q_im = cplxPredictionData->alpha_q_im[group - 1][band];
                } else if ((windowSequence == BLOCK_SHORT) &&
                           (pJointStereoPersistentData->winSeqPrev == BLOCK_SHORT)) {
                  FDK_ASSERT(pJointStereoPersistentData->winGroupsPrev > 0);
                  last_alpha_q_re =
                      pJointStereoPersistentData
                          ->alpha_q_re_prev[pJointStereoPersistentData->winGroupsPrev - 1][band];
                  last_alpha_q_im =
                      pJointStereoPersistentData
                          ->alpha_q_im_prev[pJointStereoPersistentData->winGroupsPrev - 1][band];
                } else {
                  last_alpha_q_re = pJointStereoPersistentData->alpha_q_re_prev[group][band];
                  last_alpha_q_im = pJointStereoPersistentData->alpha_q_im_prev[group][band];
                }

              } else {
                if (band > 0) {
                  last_alpha_q_re = cplxPredictionData->alpha_q_re[group][band - 1];
                  last_alpha_q_im = cplxPredictionData->alpha_q_im[group][band - 1];
                } else {
                  last_alpha_q_re = 0;
                  last_alpha_q_im = 0;
                }

              } /* if (delta_code_time == 1) */

              if (pJointStereoData->MsUsed[band] & ((UCHAR)1 << group)) {
                int dpcm_alpha_re, dpcm_alpha_im;

                dpcm_alpha_re = CBlock_DecodeHuffmanWord(bs, hcb);
                dpcm_alpha_re -= 60;
                dpcm_alpha_re *= -1;

                cplxPredictionData->alpha_q_re[group][band] = dpcm_alpha_re + last_alpha_q_re;

                if (cplxPredictionData->complex_coef) {
                  dpcm_alpha_im = CBlock_DecodeHuffmanWord(bs, hcb);
                  dpcm_alpha_im -= 60;
                  dpcm_alpha_im *= -1;

                  cplxPredictionData->alpha_q_im[group][band] = dpcm_alpha_im + last_alpha_q_im;
                } else {
                  cplxPredictionData->alpha_q_im[group][band] = 0;
                }

              } else {
                cplxPredictionData->alpha_q_re[group][band] = 0;
                cplxPredictionData->alpha_q_im[group][band] = 0;
              } /* if (pJointStereoData->MsUsed[band] & ((UCHAR)1 << group)) */

              if ((band + 1) < scaleFactorBandsTransmitted) { /* <= this should be the correct way
                                                                 (cp. ISO_IEC_FDIS_23003-0(E) */
                /*    7.7.2.3.2 Decoding of prediction coefficients) */
                cplxPredictionData->alpha_q_re[group][band + 1] =
                    cplxPredictionData->alpha_q_re[group][band];
                cplxPredictionData->alpha_q_im[group][band + 1] =
                    cplxPredictionData->alpha_q_im[group][band];
              } /* if ((band+1)<scaleFactorBandsTotal) */

              pJointStereoPersistentData->alpha_q_re_prev[group][band] =
                  cplxPredictionData->alpha_q_re[group][band];
              pJointStereoPersistentData->alpha_q_im_prev[group][band] =
                  cplxPredictionData->alpha_q_im[group][band];
            }

            for (band = scaleFactorBandsTransmitted; band < max_sfb_ste_clear; band++) {
              cplxPredictionData->alpha_q_re[group][band] = 0;
              cplxPredictionData->alpha_q_im[group][band] = 0;
              pJointStereoPersistentData->alpha_q_re_prev[group][band] = 0;
              pJointStereoPersistentData->alpha_q_im_prev[group][band] = 0;
            }
          }
        }
      }
    } else {
      for (group = 0; group < windowGroups; group++) {
        for (band = 0; band < max_sfb_ste_clear; band++) {
          pJointStereoPersistentData->alpha_q_re_prev[group][band] = 0;
          pJointStereoPersistentData->alpha_q_im_prev[group][band] = 0;
        }
      }
    }

    pJointStereoPersistentData->winGroupsPrev = windowGroups;
  }

  return 0;
}

int CJointStereo_ReadIGF(HANDLE_FDK_BITSTREAM bs, CJointStereoData* pJointStereoData,
                         const int windowGroups, const int igfStartSfb, const int igfStopSfb,
                         const int igfUseHighRes,
                         CJointStereoPersistentData* pJointStereoPersistentData,
                         CCplxPredictionData* cplxPredictionData, int windowSequence,
                         const UINT flags) {
  int group, sfb;

  pJointStereoData->IGF_MsMaskPresent = (UCHAR)FDKread2Bits(bs);

  FDKmemclear(&(pJointStereoData->MsUsed[igfStartSfb]),
              (JointStereoMaximumBands - igfStartSfb) * sizeof(UCHAR));

  pJointStereoData->igf_cplx_pred_flag = 0;

  switch (pJointStereoData->IGF_MsMaskPresent) {
    case 0: /* no IGF M/S */
      /* all flags are already cleared */
      for (sfb = igfStartSfb; sfb < igfStopSfb; sfb++) {
        pJointStereoData->MsUsed[sfb] = 0; /* set all flags to 0 */
      }
      break;

    case 1: /* read IGF ms_used */
      for (group = 0; group < windowGroups; group++) {
        for (sfb = igfStartSfb; sfb < igfStopSfb; sfb += (2 - igfUseHighRes)) {
          pJointStereoData->MsUsed[sfb] |= (FDKreadBit(bs) << group);
          if (!igfUseHighRes && (sfb + 1) < igfStopSfb) {
            pJointStereoData->MsUsed[sfb + 1] = pJointStereoData->MsUsed[sfb];
          }
        }
      }
      break;

    case 2: /* IGF full spectrum M/S */
      for (sfb = igfStartSfb; sfb < igfStopSfb; sfb++) {
        pJointStereoData->MsUsed[sfb] = 255; /* set all flags to 1 */
      }
      break;

    case 3:
      pJointStereoData->igf_cplx_pred_flag = 1;

      int igf_stereo_pred_all = 0;
      igf_stereo_pred_all = FDKreadBit(bs);

      if (igf_stereo_pred_all) {
        for (group = 0; group < windowGroups; group++) {
          UCHAR groupmask = ((UCHAR)1 << group);
          for (sfb = igfStartSfb; sfb < igfStopSfb; sfb++) {
            pJointStereoData->MsUsed[sfb] |= groupmask;
          }
        }
      } else {
        for (group = 0; group < windowGroups; group++) {
          for (sfb = igfStartSfb; sfb < igfStopSfb; sfb += SFB_PER_PRED_BAND) {
            pJointStereoData->MsUsed[sfb] |= (FDKreadBit(bs) << group);
            if ((sfb + 1) < igfStopSfb) {
              pJointStereoData->MsUsed[sfb + 1] = pJointStereoData->MsUsed[sfb];
            }
          }
        }
      }
      break;
  } /* end switch */

  if (pJointStereoData->IGF_MsMaskPresent != 3) return 0;

  if (pJointStereoData->igf_cplx_pred_flag) {
    int igf_delta_code_time = 0;

    /* set pointer to Huffman codebooks */
    const CodeBookDescription* hcb = &AACcodeBookDescriptionSCL;
    /* set predictors to zero in case of a transition from long to short window sequences and vice
     * versa */
    if (((windowSequence == BLOCK_SHORT) &&
         (pJointStereoPersistentData->winSeqPrev != BLOCK_SHORT)) ||
        ((pJointStereoPersistentData->winSeqPrev == BLOCK_SHORT) &&
         (windowSequence != BLOCK_SHORT))) {
      for (group = 0; group < windowGroups; group++) {
        for (sfb = igfStartSfb; sfb < igfStopSfb; sfb++) {
          pJointStereoPersistentData->alpha_q_re_prev[group][sfb] = 0;
        }
      }
    }

    /* make sure prediction coefficients are reset */
    for (group = 0; group < windowGroups; group++) {
      /* Clean the IGF used part of the array. The lower part of the array is used by standard
       * MS/Complex stereo prediction */
      for (sfb = igfStartSfb; sfb < igfStopSfb; sfb++) {
        cplxPredictionData->alpha_q_re[group][sfb] = 0;
      }
    }

    /* 0 = mid->side prediction, 1 = side->mid prediction */
    cplxPredictionData->igf_pred_dir = FDKreadBit(bs);

    if (flags & AC_INDEP) {
      igf_delta_code_time = 0;
    } else {
      igf_delta_code_time = FDKreadBit(bs);
    }

    int last_alpha_q_re = 0;

    for (group = 0; group < windowGroups; group++) {
      for (sfb = igfStartSfb; sfb < igfStopSfb; sfb += SFB_PER_PRED_BAND) {
        if (igf_delta_code_time == 1) {
          if (group > 0) {
            last_alpha_q_re = cplxPredictionData->alpha_q_re[group - 1][sfb];
          } else {
            FDK_ASSERT(pJointStereoPersistentData->winGroupsPrev > 0);
            last_alpha_q_re =
                pJointStereoPersistentData
                    ->alpha_q_re_prev[pJointStereoPersistentData->winGroupsPrev - 1][sfb];
          }
        } else {
          if (sfb > igfStartSfb) {
            last_alpha_q_re = cplxPredictionData->alpha_q_re[group][sfb - 1];
          } else {
            last_alpha_q_re = 0;
          }

        } /* if (delta_code_time == 1) */

        if (pJointStereoData->MsUsed[sfb] & ((UCHAR)1 << group)) {
          int dpcm_alpha_re;

          dpcm_alpha_re = CBlock_DecodeHuffmanWord(bs, hcb);
          dpcm_alpha_re -= 60;
          dpcm_alpha_re *= -1;

          cplxPredictionData->alpha_q_re[group][sfb] = dpcm_alpha_re + last_alpha_q_re;

        } else {
          cplxPredictionData->alpha_q_re[group][sfb] = 0;
        } /* if (pJointStereoData->MsUsed[band] & ((UCHAR)1 << group)) */

        if ((sfb + 1) <
            igfStopSfb) { /* <= this should be the correct way (cp. ISO_IEC_FDIS_23003-0(E) */
          cplxPredictionData->alpha_q_re[group][sfb + 1] =
              cplxPredictionData->alpha_q_re[group][sfb];
        } /* (sfb+1)< igfStopSfb  */

        pJointStereoPersistentData->alpha_q_re_prev[group][sfb] =
            cplxPredictionData->alpha_q_re[group][sfb];
        pJointStereoPersistentData->alpha_q_im_prev[group][sfb] =
            cplxPredictionData->alpha_q_im[group][sfb];

      } /* for (sfb=igfStartSfb; sfb<igfStopSfb;sfb+=SFB_PER_PRED_BAND) */

      for (sfb = igfStopSfb; sfb < JointStereoMaximumBands; sfb++) {
        cplxPredictionData->alpha_q_re[group][sfb] = 0;
        cplxPredictionData->alpha_q_im[group][sfb] = 0;
        pJointStereoPersistentData->alpha_q_re_prev[group][sfb] = 0;
        pJointStereoPersistentData->alpha_q_im_prev[group][sfb] = 0;
      }

    } /* for (group = 0; group<windowGroups; group++) */
  }

  pJointStereoPersistentData->winGroupsPrev = windowGroups;

  return 0;
}

static const int indices_1[] = {2, 1, 0, 1, 2, 3};
static const int indices_2[] = {1, 0, 0, 2, 3, 4};
static const int indices_3[] = {0, 0, 1, 3, 4, 5};

static const int subtr_1[] = {6, 5, 4, 2, 1, 1};
static const int subtr_2[] = {5, 4, 3, 1, 1, 2};
static const int subtr_3[] = {4, 3, 2, 1, 2, 3};

static void CJointStereo_filterAndAdd(FIXP_DBL* in, int len, int windowLen, const FIXP_FILT* coeff,
                                      FIXP_DBL* out,
                                      UCHAR isCurrent /* output values with even index get a positve
                                                         addon (=1) or a negative addon (=0) */
) {
  int i, j;

  if (isCurrent == 1) {
    /* exploit the symmetry of the table: coeff[6] = - coeff[0],
                                          coeff[5] = - coeff[1],
                                          coeff[4] = - coeff[2],
                                          coeff[3] = 0
    */

    for (i = 0; i < 3; i++) {
      out[0] -= (FIXP_DBL)fMultDiv2(coeff[i], in[indices_1[i]]) >> SR_FNA_OUT;
      out[0] += (FIXP_DBL)fMultDiv2(coeff[i], in[indices_1[5 - i]]) >> SR_FNA_OUT;
    }

    for (i = 0; i < 3; i++) {
      out[1] -= (FIXP_DBL)fMultDiv2(coeff[i], in[indices_2[i]]) >> SR_FNA_OUT;
      out[1] += (FIXP_DBL)fMultDiv2(coeff[i], in[indices_2[5 - i]]) >> SR_FNA_OUT;
    }

    for (i = 0; i < 3; i++) {
      out[2] -= (FIXP_DBL)fMultDiv2(coeff[i], in[indices_3[i]]) >> SR_FNA_OUT;
      out[2] += (FIXP_DBL)fMultDiv2(coeff[i], in[indices_3[5 - i]]) >> SR_FNA_OUT;
    }

    for (j = 3; j < (len - 3); j++) {
      for (i = 0; i < 3; i++) {
        out[j] -= (FIXP_DBL)fMultDiv2(coeff[i], in[j - 3 + i]) >> SR_FNA_OUT;
        out[j] += (FIXP_DBL)fMultDiv2(coeff[i], in[j + 3 - i]) >> SR_FNA_OUT;
      }
    }

    for (i = 0; i < 3; i++) {
      out[len - 3] -= (FIXP_DBL)fMultDiv2(coeff[i], in[len - subtr_1[i]]) >> SR_FNA_OUT;
      out[len - 3] += (FIXP_DBL)fMultDiv2(coeff[i], in[len - subtr_1[5 - i]]) >> SR_FNA_OUT;
    }

    for (i = 0; i < 3; i++) {
      out[len - 2] -= (FIXP_DBL)fMultDiv2(coeff[i], in[len - subtr_2[i]]) >> SR_FNA_OUT;
      out[len - 2] += (FIXP_DBL)fMultDiv2(coeff[i], in[len - subtr_2[5 - i]]) >> SR_FNA_OUT;
    }

    for (i = 0; i < 3; i++) {
      out[len - 1] -= (FIXP_DBL)fMultDiv2(coeff[i], in[len - subtr_3[i]]) >> SR_FNA_OUT;
      out[len - 1] += (FIXP_DBL)fMultDiv2(coeff[i], in[len - subtr_3[5 - i]]) >> SR_FNA_OUT;
    }

  } else {
    /* exploit the symmetry of the table: coeff[6] = coeff[0],
                                          coeff[5] = coeff[1],
                                          coeff[4] = coeff[2]
    */

    for (i = 0; i < 3; i++) {
      out[0] -= (FIXP_DBL)fMultDiv2(coeff[i], in[indices_1[i]] >> SR_FNA_OUT);
      out[0] -= (FIXP_DBL)fMultDiv2(coeff[i], in[indices_1[5 - i]] >> SR_FNA_OUT);
    }
    out[0] -= (FIXP_DBL)fMultDiv2(coeff[3], in[0] >> SR_FNA_OUT);

    for (i = 0; i < 3; i++) {
      out[1] += (FIXP_DBL)fMultDiv2(coeff[i], in[indices_2[i]] >> SR_FNA_OUT);
      out[1] += (FIXP_DBL)fMultDiv2(coeff[i], in[indices_2[5 - i]] >> SR_FNA_OUT);
    }
    out[1] += (FIXP_DBL)fMultDiv2(coeff[3], in[1] >> SR_FNA_OUT);

    for (i = 0; i < 3; i++) {
      out[2] -= (FIXP_DBL)fMultDiv2(coeff[i], in[indices_3[i]] >> SR_FNA_OUT);
      out[2] -= (FIXP_DBL)fMultDiv2(coeff[i], in[indices_3[5 - i]] >> SR_FNA_OUT);
    }
    out[2] -= (FIXP_DBL)fMultDiv2(coeff[3], in[2] >> SR_FNA_OUT);

    for (j = 3; j < (len - 4); j++) {
      for (i = 0; i < 3; i++) {
        out[j] += (FIXP_DBL)fMultDiv2(coeff[i], in[j - 3 + i] >> SR_FNA_OUT);
        out[j] += (FIXP_DBL)fMultDiv2(coeff[i], in[j + 3 - i] >> SR_FNA_OUT);
      }
      out[j] += (FIXP_DBL)fMultDiv2(coeff[3], in[j] >> SR_FNA_OUT);

      j++;

      for (i = 0; i < 3; i++) {
        out[j] -= (FIXP_DBL)fMultDiv2(coeff[i], in[j - 3 + i] >> SR_FNA_OUT);
        out[j] -= (FIXP_DBL)fMultDiv2(coeff[i], in[j + 3 - i] >> SR_FNA_OUT);
      }
      out[j] -= (FIXP_DBL)fMultDiv2(coeff[3], in[j] >> SR_FNA_OUT);
    }

    for (i = 0; i < 3; i++) {
      out[len - 3] += (FIXP_DBL)fMultDiv2(coeff[i], in[len - subtr_1[i]] >> SR_FNA_OUT);
      out[len - 3] += (FIXP_DBL)fMultDiv2(coeff[i], in[len - subtr_1[5 - i]] >> SR_FNA_OUT);
    }
    out[len - 3] += (FIXP_DBL)fMultDiv2(coeff[3], in[len - 3] >> SR_FNA_OUT);

    for (i = 0; i < 3; i++) {
      out[len - 2] -= (FIXP_DBL)fMultDiv2(coeff[i], in[len - subtr_2[i]] >> SR_FNA_OUT);
      out[len - 2] -= (FIXP_DBL)fMultDiv2(coeff[i], in[len - subtr_2[5 - i]] >> SR_FNA_OUT);
    }
    out[len - 2] -= (FIXP_DBL)fMultDiv2(coeff[3], in[len - 2] >> SR_FNA_OUT);

    for (i = 0; i < 3; i++) {
      out[len - 1] += (FIXP_DBL)fMultDiv2(coeff[i], in[len - subtr_3[i]] >> SR_FNA_OUT);
      out[len - 1] += (FIXP_DBL)fMultDiv2(coeff[i], in[len - subtr_3[5 - i]] >> SR_FNA_OUT);
    }
    out[len - 1] += (FIXP_DBL)fMultDiv2(coeff[3], in[len - 1] >> SR_FNA_OUT);
  }
}

static void CJointStereo_filterAndAdd_Int(
    FIXP_DBL* in, int len, int windowLen, int halfLowpassLine, const FIXP_FILT* coeff,
    FIXP_DBL* out, UCHAR isCurrent /* output values with even index get a positve addon (=1) or a
                                      negative addon (=0) */
) {
  int i, j;

  if (isCurrent == 1) {
    for (i = 0; i < 3; i++) {
      if (indices_1[i] < halfLowpassLine)
        out[0] -= (FIXP_DBL)fMultDiv2(coeff[i], in[(indices_1[i]) << 1]) >> SR_FNA_OUT;
      if (indices_1[5 - i] < halfLowpassLine)
        out[0] += (FIXP_DBL)fMultDiv2(coeff[i], in[(indices_1[5 - i]) << 1]) >> SR_FNA_OUT;
    }

    for (i = 0; i < 3; i++) {
      if (indices_2[i] < halfLowpassLine)
        out[2] -= (FIXP_DBL)fMultDiv2(coeff[i], in[(indices_2[i]) << 1]) >> SR_FNA_OUT;
      if (indices_2[5 - i] < halfLowpassLine)
        out[2] += (FIXP_DBL)fMultDiv2(coeff[i], in[(indices_2[5 - i]) << 1]) >> SR_FNA_OUT;
    }

    for (i = 0; i < 3; i++) {
      if (indices_3[i] < halfLowpassLine)
        out[4] -= (FIXP_DBL)fMultDiv2(coeff[i], in[(indices_3[i]) << 1]) >> SR_FNA_OUT;
      if (indices_3[5 - i] < halfLowpassLine)
        out[4] += (FIXP_DBL)fMultDiv2(coeff[i], in[(indices_3[5 - i]) << 1]) >> SR_FNA_OUT;
    }

    for (j = 3; j < (len - 3); j++) {
      for (i = 0; i < 3; i++) {
        if ((j - 3 + i) < halfLowpassLine)
          out[j << 1] -= (FIXP_DBL)fMultDiv2(coeff[i], in[(j - 3 + i) << 1]) >> SR_FNA_OUT;
        if ((j + 3 - i) < halfLowpassLine)
          out[j << 1] += (FIXP_DBL)fMultDiv2(coeff[i], in[(j + 3 - i) << 1]) >> SR_FNA_OUT;
      }
    }

    for (i = 0; i < 3; i++) {
      if ((len - subtr_1[i]) < halfLowpassLine)
        out[(len - 3) << 1] -=
            (FIXP_DBL)fMultDiv2(coeff[i], in[(len - subtr_1[i]) << 1]) >> SR_FNA_OUT;
      if ((len - subtr_1[5 - i]) < halfLowpassLine)
        out[(len - 3) << 1] +=
            (FIXP_DBL)fMultDiv2(coeff[i], in[(len - subtr_1[5 - i]) << 1]) >> SR_FNA_OUT;
    }

    for (i = 0; i < 3; i++) {
      if ((len - subtr_2[i]) < halfLowpassLine)
        out[(len - 2) << 1] -=
            (FIXP_DBL)fMultDiv2(coeff[i], in[(len - subtr_2[i]) << 1]) >> SR_FNA_OUT;
      if ((len - subtr_2[5 - i]) < halfLowpassLine)
        out[(len - 2) << 1] +=
            (FIXP_DBL)fMultDiv2(coeff[i], in[(len - subtr_2[5 - i]) << 1]) >> SR_FNA_OUT;
    }

    for (i = 0; i < 3; i++) {
      if ((len - subtr_3[i]) < halfLowpassLine)
        out[(len - 1) << 1] -=
            (FIXP_DBL)fMultDiv2(coeff[i], in[(len - subtr_3[i]) << 1]) >> SR_FNA_OUT;
      if ((len - subtr_3[5 - i]) < halfLowpassLine)
        out[(len - 1) << 1] +=
            (FIXP_DBL)fMultDiv2(coeff[i], in[(len - subtr_3[5 - i]) << 1]) >> SR_FNA_OUT;
    }

  } else {
    for (i = 0; i < 3; i++) {
      if (indices_1[i] < halfLowpassLine)
        out[0] -= (FIXP_DBL)fMultDiv2(coeff[i], in[(indices_1[i]) << 1] >> SR_FNA_OUT);
      if (indices_1[5 - i] < halfLowpassLine)
        out[0] -= (FIXP_DBL)fMultDiv2(coeff[i], in[(indices_1[5 - i]) << 1] >> SR_FNA_OUT);
    }
    if (0 < halfLowpassLine) out[0] -= (FIXP_DBL)fMultDiv2(coeff[3], in[0 << 1] >> SR_FNA_OUT);

    for (i = 0; i < 3; i++) {
      if (indices_2[i] < halfLowpassLine)
        out[2] += (FIXP_DBL)fMultDiv2(coeff[i], in[(indices_2[i]) << 1] >> SR_FNA_OUT);
      if (indices_2[5 - i] < halfLowpassLine)
        out[2] += (FIXP_DBL)fMultDiv2(coeff[i], in[(indices_2[5 - i]) << 1] >> SR_FNA_OUT);
    }
    if (1 < halfLowpassLine) out[1 << 1] += (FIXP_DBL)fMultDiv2(coeff[3], in[1 << 1] >> SR_FNA_OUT);

    for (i = 0; i < 3; i++) {
      if (indices_3[i] < halfLowpassLine)
        out[4] -= (FIXP_DBL)fMultDiv2(coeff[i], in[(indices_3[i]) << 1] >> SR_FNA_OUT);
      if (indices_3[5 - i] < halfLowpassLine)
        out[4] -= (FIXP_DBL)fMultDiv2(coeff[i], in[(indices_3[5 - i]) << 1] >> SR_FNA_OUT);
    }
    if (2 < halfLowpassLine) out[2 << 1] -= (FIXP_DBL)fMultDiv2(coeff[3], in[2 << 1] >> SR_FNA_OUT);

    for (j = 3; j < (len - 4); j++) {
      for (i = 0; i < 3; i++) {
        if ((j - 3 + i) < halfLowpassLine)
          out[j << 1] += (FIXP_DBL)fMultDiv2(coeff[i], in[(j - 3 + i) << 1] >> SR_FNA_OUT);
        if ((j + 3 - i) < halfLowpassLine)
          out[j << 1] += (FIXP_DBL)fMultDiv2(coeff[i], in[(j + 3 - i) << 1] >> SR_FNA_OUT);
      }
      if (j < halfLowpassLine)
        out[j << 1] += (FIXP_DBL)fMultDiv2(coeff[3], in[j << 1] >> SR_FNA_OUT);

      j++;

      for (i = 0; i < 3; i++) {
        if ((j - 3 + i) < halfLowpassLine)
          out[j << 1] -= (FIXP_DBL)fMultDiv2(coeff[i], in[(j - 3 + i) << 1] >> SR_FNA_OUT);
        if ((j + 3 - i) < halfLowpassLine)
          out[j << 1] -= (FIXP_DBL)fMultDiv2(coeff[i], in[(j + 3 - i) << 1] >> SR_FNA_OUT);
      }
      if (j < halfLowpassLine)
        out[j << 1] -= (FIXP_DBL)fMultDiv2(coeff[3], in[j << 1] >> SR_FNA_OUT);
    }

    for (i = 0; i < 3; i++) {
      if ((len - subtr_1[i]) < halfLowpassLine)
        out[(len - 3) << 1] +=
            (FIXP_DBL)fMultDiv2(coeff[i], in[(len - subtr_1[i]) << 1] >> SR_FNA_OUT);
      if ((len - subtr_1[5 - i]) < halfLowpassLine)
        out[(len - 3) << 1] +=
            (FIXP_DBL)fMultDiv2(coeff[i], in[(len - subtr_1[5 - i]) << 1] >> SR_FNA_OUT);
    }
    if ((len - 3) < halfLowpassLine)
      out[(len - 3) << 1] += (FIXP_DBL)fMultDiv2(coeff[3], in[(len - 3) << 1] >> SR_FNA_OUT);

    for (i = 0; i < 3; i++) {
      if ((len - subtr_2[i]) < halfLowpassLine)
        out[(len - 2) << 1] -=
            (FIXP_DBL)fMultDiv2(coeff[i], in[(len - subtr_2[i]) << 1] >> SR_FNA_OUT);
      if ((len - subtr_2[5 - i]) < halfLowpassLine)
        out[(len - 2) << 1] -=
            (FIXP_DBL)fMultDiv2(coeff[i], in[(len - subtr_2[5 - i]) << 1] >> SR_FNA_OUT);
    }
    if ((len - 2) < halfLowpassLine)
      out[(len - 2) << 1] -= (FIXP_DBL)fMultDiv2(coeff[3], in[(len - 2) << 1] >> SR_FNA_OUT);

    for (i = 0; i < 3; i++) {
      if ((len - subtr_3[i]) < halfLowpassLine)
        out[(len - 1) << 1] +=
            (FIXP_DBL)fMultDiv2(coeff[i], in[(len - subtr_3[i]) << 1] >> SR_FNA_OUT);
      if ((len - subtr_3[5 - i]) < halfLowpassLine)
        out[(len - 1) << 1] +=
            (FIXP_DBL)fMultDiv2(coeff[i], in[(len - subtr_3[5 - i]) << 1] >> SR_FNA_OUT);
    }
    if ((len - 1) < halfLowpassLine)
      out[(len - 1) << 1] += (FIXP_DBL)fMultDiv2(coeff[3], in[(len - 1) << 1] >> SR_FNA_OUT);
  }
}

#if !defined(FUNCTION_CJointStereo_GenerateMSOutput)
#define FUNCTION_CJointStereo_GenerateMSOutput
static inline void CJointStereo_GenerateMSOutput(FIXP_DBL* pSpecLCurrBand, FIXP_DBL* pSpecRCurrBand,
                                                 UINT leftScale, UINT rightScale, UINT nSfbBands) {
  unsigned int i;

  FIXP_DBL leftCoefficient0;
  FIXP_DBL leftCoefficient1;
  FIXP_DBL leftCoefficient2;
  FIXP_DBL leftCoefficient3;

  FIXP_DBL rightCoefficient0;
  FIXP_DBL rightCoefficient1;
  FIXP_DBL rightCoefficient2;
  FIXP_DBL rightCoefficient3;

  for (i = nSfbBands; i > 0; i -= 4) {
    leftCoefficient0 = pSpecLCurrBand[i - 4];
    leftCoefficient1 = pSpecLCurrBand[i - 3];
    leftCoefficient2 = pSpecLCurrBand[i - 2];
    leftCoefficient3 = pSpecLCurrBand[i - 1];

    rightCoefficient0 = pSpecRCurrBand[i - 4];
    rightCoefficient1 = pSpecRCurrBand[i - 3];
    rightCoefficient2 = pSpecRCurrBand[i - 2];
    rightCoefficient3 = pSpecRCurrBand[i - 1];

    /* MS output generation */
    leftCoefficient0 >>= leftScale;
    leftCoefficient1 >>= leftScale;
    leftCoefficient2 >>= leftScale;
    leftCoefficient3 >>= leftScale;

    rightCoefficient0 >>= rightScale;
    rightCoefficient1 >>= rightScale;
    rightCoefficient2 >>= rightScale;
    rightCoefficient3 >>= rightScale;

    pSpecLCurrBand[i - 4] = leftCoefficient0 + rightCoefficient0;
    pSpecLCurrBand[i - 3] = leftCoefficient1 + rightCoefficient1;
    pSpecLCurrBand[i - 2] = leftCoefficient2 + rightCoefficient2;
    pSpecLCurrBand[i - 1] = leftCoefficient3 + rightCoefficient3;

    pSpecRCurrBand[i - 4] = leftCoefficient0 - rightCoefficient0;
    pSpecRCurrBand[i - 3] = leftCoefficient1 - rightCoefficient1;
    pSpecRCurrBand[i - 2] = leftCoefficient2 - rightCoefficient2;
    pSpecRCurrBand[i - 1] = leftCoefficient3 - rightCoefficient3;
  }
}
#endif /* !defined(FUNCTION_CJointStereo_GenerateMSOutput) */

void CJointStereo_ApplyMS(CAacDecoderChannelInfo* pAacDecoderChannelInfo[2],
                          CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo[2],
                          FIXP_DBL* spectrumL, FIXP_DBL* spectrumR, SHORT* SFBleftScale,
                          SHORT* SFBrightScale, SHORT* specScaleL, SHORT* specScaleR,
                          const SHORT* pScaleFactorBandOffsets, const UCHAR* pWindowGroupLength,
                          const int windowGroups, const int max_sfb_ste_outside,
                          const int scaleFactorBandsTransmittedL,
                          const int scaleFactorBandsTransmittedR, FIXP_DBL* store_dmx_re_prev,
                          SHORT* store_dmx_re_prev_e, const int mainband_flag) {
  int window, group, band;
  UCHAR groupMask;
  CJointStereoData* pJointStereoData = pAacDecoderChannelInfo[L]->pComData->pJointStereoData;
  CCplxPredictionData* cplxPredictionData =
      pAacDecoderChannelInfo[L]->pComStaticData->cplxPredictionData;

  int max_sfb_ste = fMax(scaleFactorBandsTransmittedL, scaleFactorBandsTransmittedR);
  int min_sfb_ste = fMin(scaleFactorBandsTransmittedL, scaleFactorBandsTransmittedR);
  int scaleFactorBandsTransmitted = min_sfb_ste;

  if (pJointStereoData->cplx_pred_flag) {
    int windowLen, groupwin, frameMaxScale;
    CJointStereoPersistentData* pJointStereoPersistentData =
        &pAacDecoderStaticChannelInfo[L]->pCpeStaticData->jointStereoPersistentData;
    FIXP_DBL* const staticSpectralCoeffsL =
        pAacDecoderStaticChannelInfo[L]
            ->pCpeStaticData->jointStereoPersistentData.spectralCoeffs[L];
    FIXP_DBL* const staticSpectralCoeffsR =
        pAacDecoderStaticChannelInfo[L]
            ->pCpeStaticData->jointStereoPersistentData.spectralCoeffs[R];
    SHORT* const staticSpecScaleL =
        pAacDecoderStaticChannelInfo[L]->pCpeStaticData->jointStereoPersistentData.specScale[L];
    SHORT* const staticSpecScaleR =
        pAacDecoderStaticChannelInfo[L]->pCpeStaticData->jointStereoPersistentData.specScale[R];

    FIXP_DBL* dmx_re =
        pAacDecoderStaticChannelInfo[L]->pCpeStaticData->jointStereoPersistentData.scratchBuffer;
    FIXP_DBL* dmx_re_prev =
        pAacDecoderStaticChannelInfo[L]->pCpeStaticData->jointStereoPersistentData.scratchBuffer +
        1024;

    SHORT dmx_re_prev_e = 0;
    /* When MS is applied over the main band this value gets computed. Otherwise (for the tiles) it
     * uses the assigned value */
    dmx_re_prev_e = *store_dmx_re_prev_e;

    const FIXP_FILT* pCoeff;
    const FIXP_FILT* pCoeffPrev;
    int coeffPointerOffset;

    int previousShape = (int)pJointStereoPersistentData->winShapePrev;
    int currentShape = (int)pAacDecoderChannelInfo[L]->icsInfo.WindowShape;

    /* complex stereo prediction */

    /* 0. preparations */

    /* 0.0. get scratch buffer for downmix MDST */
    FIXP_DBL* dmx_im =
        pAacDecoderStaticChannelInfo[L]->pCpeStaticData->jointStereoPersistentData.scratchBuffer2;

    /* 0.1. window lengths */

    /* get length of short window for current configuration */
    windowLen = pAacDecoderChannelInfo[L]
                    ->granuleLength; /* framelength 768 => 96, framelength 1024 => 128 */

    /* if this is no short-block set length for long-block */
    if (pAacDecoderChannelInfo[L]->icsInfo.WindowSequence != BLOCK_SHORT) {
      windowLen *= 8;
    }

    /* 0.2. set pointer to filter-coefficients for MDST excitation including previous frame portions
     */
    /*      cp. ISO/IEC FDIS 23003-3:2011(E) table 125 */

    /* set pointer to default-position */
    pCoeffPrev = mdst_filt_coef_prev[previousShape];

    if (cplxPredictionData->complex_coef == 1) {
      switch (pAacDecoderChannelInfo[L]->icsInfo.WindowSequence) { /* current window sequence */
        case BLOCK_SHORT:
        case BLOCK_LONG:
          pCoeffPrev = mdst_filt_coef_prev[previousShape];
          break;

        case BLOCK_START:
          if ((pJointStereoPersistentData->winSeqPrev == BLOCK_SHORT) ||
              (pJointStereoPersistentData->winSeqPrev == BLOCK_START)) {
            /* a stop-start-sequence can only follow on an eight-short-sequence or a start-sequence
             */
            pCoeffPrev = mdst_filt_coef_prev[2 + previousShape];
          } else {
            pCoeffPrev = mdst_filt_coef_prev[previousShape];
          }
          break;

        case BLOCK_STOP:
          pCoeffPrev = mdst_filt_coef_prev[2 + previousShape];
          break;

        default:
          pCoeffPrev = mdst_filt_coef_prev[previousShape];
          break;
      }
    }

    /* 0.3. set pointer to filter-coefficients for MDST excitation */

    /* define offset of pointer to filter-coefficients for MDST exitation employing only the current
     * frame */
    if ((previousShape == SHAPE_SINE) && (currentShape == SHAPE_SINE)) {
      coeffPointerOffset = 0;
    } else if ((previousShape == SHAPE_SINE) && (currentShape == SHAPE_KBD)) {
      coeffPointerOffset = 2;
    } else if ((previousShape == SHAPE_KBD) && (currentShape == SHAPE_KBD)) {
      coeffPointerOffset = 1;
    } else /* if ( (previousShape == SHAPE_KBD) && (currentShape == SHAPE_SINE) ) */
    {
      coeffPointerOffset = 3;
    }

    /* set pointer to filter-coefficient table cp. ISO/IEC FDIS 23003-3:2011(E) table 124 */
    switch (pAacDecoderChannelInfo[L]->icsInfo.WindowSequence) { /* current window sequence */
      case BLOCK_SHORT:
      case BLOCK_LONG:
        pCoeff = mdst_filt_coef_curr[coeffPointerOffset];
        break;

      case BLOCK_START:
        if ((pJointStereoPersistentData->winSeqPrev == BLOCK_SHORT) ||
            (pJointStereoPersistentData->winSeqPrev == BLOCK_START)) {
          /* a stop-start-sequence can only follow on an eight-short-sequence or a start-sequence */
          pCoeff = mdst_filt_coef_curr[12 + coeffPointerOffset];
        } else {
          pCoeff = mdst_filt_coef_curr[4 + coeffPointerOffset];
        }
        break;

      case BLOCK_STOP:
        pCoeff = mdst_filt_coef_curr[8 + coeffPointerOffset];
        break;

      default:
        pCoeff = mdst_filt_coef_curr[coeffPointerOffset];
    }

    /* 0.4. find maximum common (l/r) band-scaling-factor for whole sequence (all windows) */
    frameMaxScale = 0;
    for (window = 0, group = 0; group < windowGroups; group++) {
      for (groupwin = 0; groupwin < pWindowGroupLength[group]; groupwin++, window++) {
        SHORT* leftScale = &SFBleftScale[window * 16];
        SHORT* rightScale = &SFBrightScale[window * 16];
        int windowMaxScale = 0;

        /* find maximum scaling factor of all bands in this window */
        for (band = 0; band < min_sfb_ste; band++) {
          int lScale = leftScale[band];
          int rScale = rightScale[band];
          int commonScale = ((lScale > rScale) ? lScale : rScale);
          windowMaxScale = (windowMaxScale < commonScale) ? commonScale : windowMaxScale;
        }
        if (scaleFactorBandsTransmittedL >
            min_sfb_ste) { /* i.e. scaleFactorBandsTransmittedL == max_sfb_ste */
          for (; band < max_sfb_ste; band++) {
            int lScale = leftScale[band];
            windowMaxScale = (windowMaxScale < lScale) ? lScale : windowMaxScale;
          }
        } else {
          if (scaleFactorBandsTransmittedR >
              min_sfb_ste) { /* i.e. scaleFactorBandsTransmittedR == max_sfb_ste */
            for (; band < max_sfb_ste; band++) {
              int rScale = rightScale[band];
              windowMaxScale = (windowMaxScale < rScale) ? rScale : windowMaxScale;
            }
          }
        }

        /* find maximum common SF of all windows */
        frameMaxScale = (frameMaxScale < windowMaxScale) ? windowMaxScale : frameMaxScale;
      }
    }

    /* add some headroom for overflow protection during filter and add operation */
    frameMaxScale += 2;

    /* process on window-basis (i.e. iterate over all groups and corresponding windows) */
    for (window = 0, group = 0; group < windowGroups; group++) {
      groupMask = 1 << group;

      for (groupwin = 0; groupwin < pWindowGroupLength[group]; groupwin++, window++) {
        /* initialize the MDST with zeros */
        FDKmemclear(&dmx_im[windowLen * window], windowLen * sizeof(FIXP_DBL));

        /* 1. calculate the previous downmix MDCT. We do this once just for the Main band. */
        if (cplxPredictionData->complex_coef == 1) {
          if (cplxPredictionData->use_prev_frame == 1) {
            if (mainband_flag) {
              /* if this is a long-block or the first window of a short-block
                 calculate the downmix MDCT of the previous frame.
                 use_prev_frame is assumed not to change during a frame!
              */

              /* first determine shiftfactors to scale left and right channel */
              if ((pAacDecoderChannelInfo[L]->icsInfo.WindowSequence != BLOCK_SHORT) ||
                  (window == 0)) {
                int index_offset = 0;
                int srLeftChan = 0;
                int srRightChan = 0;
                if (pAacDecoderChannelInfo[L]->icsInfo.WindowSequence == BLOCK_SHORT) {
                  /* use the last window of the previous frame for MDCT calculation if this is a
                   * short-block. */
                  index_offset = windowLen * 7;
                  if (staticSpecScaleL[7] > staticSpecScaleR[7]) {
                    srRightChan = staticSpecScaleL[7] - staticSpecScaleR[7];
                    dmx_re_prev_e = staticSpecScaleL[7];
                  } else {
                    srLeftChan = staticSpecScaleR[7] - staticSpecScaleL[7];
                    dmx_re_prev_e = staticSpecScaleR[7];
                  }
                } else {
                  if (staticSpecScaleL[0] > staticSpecScaleR[0]) {
                    srRightChan = staticSpecScaleL[0] - staticSpecScaleR[0];
                    dmx_re_prev_e = staticSpecScaleL[0];
                  } else {
                    srLeftChan = staticSpecScaleR[0] - staticSpecScaleL[0];
                    dmx_re_prev_e = staticSpecScaleR[0];
                  }
                }

                /* now scale channels and determine downmix MDCT of previous frame */
                if (pAacDecoderStaticChannelInfo[L]
                        ->pCpeStaticData->jointStereoPersistentData.clearSpectralCoeffs == 1) {
                  FDKmemclear(dmx_re_prev, windowLen * sizeof(FIXP_DBL));
                  dmx_re_prev_e = 0;
                } else {
                  if (cplxPredictionData->pred_dir == 0) {
                    for (int i = 0; i < windowLen; i++) {
                      dmx_re_prev[i] = ((staticSpectralCoeffsL[index_offset + i] >>
                                         fMin(DFRACT_BITS - 1, srLeftChan + 1)) +
                                        (staticSpectralCoeffsR[index_offset + i] >>
                                         fMin(DFRACT_BITS - 1, srRightChan + 1)));
                    }
                  } else {
                    for (int i = 0; i < windowLen; i++) {
                      dmx_re_prev[i] = ((staticSpectralCoeffsL[index_offset + i] >>
                                         fMin(DFRACT_BITS - 1, srLeftChan + 1)) -
                                        (staticSpectralCoeffsR[index_offset + i] >>
                                         fMin(DFRACT_BITS - 1, srRightChan + 1)));
                    }
                  }
                }

                /* In case that we use INF we have to preserve the state of the "dmx_re_prev"
                (original or computed). This is necessary because we have to apply MS over the
                separate IGF tiles. */
                FDKmemcpy(store_dmx_re_prev, &dmx_re_prev[0], windowLen * sizeof(FIXP_DBL));

                /* Particular exponent of the computed/original "dmx_re_prev" must be kept for the
                 * tile MS calculations if necessary.*/
                *store_dmx_re_prev_e = dmx_re_prev_e;

              } /* if ( (pAacDecoderChannelInfo[L]->icsInfo.WindowSequence != BLOCK_SHORT) ||
                   (window == 0) ) */
            }

          } /* if ( pJointStereoData->use_prev_frame == 1 ) */

        } /* if ( pJointStereoData->complex_coef == 1 ) */

        /* 2. calculate downmix MDCT of current frame */

        /* set pointer to scale-factor-bands of current window */
        SHORT* leftScale = &SFBleftScale[window * 16];
        SHORT* rightScale = &SFBrightScale[window * 16];

        specScaleL[window] = specScaleR[window] = frameMaxScale;

        /* adapt scaling-factors to previous frame */
        if (cplxPredictionData->use_prev_frame == 1) {
          if (window == 0) {
            if (dmx_re_prev_e < frameMaxScale) {
              if (mainband_flag == 0) {
                scaleValues(dmx_re_prev, store_dmx_re_prev, windowLen,
                            -fMin(DFRACT_BITS - 1, (frameMaxScale - dmx_re_prev_e)));
              } else {
                scaleValues(dmx_re_prev, windowLen,
                            -fMin(DFRACT_BITS - 1, (frameMaxScale - dmx_re_prev_e)));
              }
            } else {
              if (mainband_flag == 0) {
                FDKmemcpy(dmx_re_prev, store_dmx_re_prev, windowLen * sizeof(FIXP_DBL));
              }
              specScaleL[0] = dmx_re_prev_e;
              specScaleR[0] = dmx_re_prev_e;
            }
          } else { /* window != 0 */
            FDK_ASSERT(pAacDecoderChannelInfo[L]->icsInfo.WindowSequence == BLOCK_SHORT);
            if (specScaleL[window - 1] < frameMaxScale) {
              scaleValues(&dmx_re[windowLen * (window - 1)], windowLen,
                          -fMin(DFRACT_BITS - 1, (frameMaxScale - specScaleL[window - 1])));
            } else {
              specScaleL[window] = specScaleL[window - 1];
              specScaleR[window] = specScaleR[window - 1];
            }
          }
        } /* if ( pJointStereoData->use_prev_frame == 1 ) */

        /* scaling factors of both channels ought to be equal now */
        FDK_ASSERT(specScaleL[window] == specScaleR[window]);

#ifdef FUNCTION_CJointStereo_ApplyMS_func2
        CJointStereo_ApplyMS_func2(
            (FIXP_DBL * RESTRICT) & spectrumL[windowLen * window],
            (FIXP_DBL * RESTRICT) & spectrumR[windowLen * window],
            (FIXP_DBL * RESTRICT) & dmx_re[windowLen * window], (SHORT * RESTRICT) leftScale,
            (SHORT * RESTRICT) rightScale, (SHORT * RESTRICT) pScaleFactorBandOffsets,
            specScaleL[window], (INT)max_sfb_ste, (UCHAR * RESTRICT) pJointStereoData->MsUsed,
            (UCHAR)groupMask, cplxPredictionData->pred_dir ? (SHORT)-1 : (SHORT)1);
#else
        /* rescale signal and calculate downmix MDCT */
        for (band = 0; band < max_sfb_ste; band++) {
          /* first adapt scaling of current band to scaling of current window => shift signal right
           */
          int lScale = leftScale[band];
          int rScale = rightScale[band];

          lScale = fMin(DFRACT_BITS - 1, specScaleL[window] - lScale);
          rScale = fMin(DFRACT_BITS - 1,
                        specScaleL[window] -
                            rScale); /* L or R doesn't matter, specScales are equal at this point */

          /* Write back to sfb scale to cover the case when max_sfb_ste < max_sfb */
          leftScale[band] = rightScale[band] = specScaleL[window];

          FDK_PRAGMA_MUST_ITERATE(pScaleFactorBandOffsets[band + 1] - pScaleFactorBandOffsets[band],
                                  4, 96, 4)
          for (int i = pScaleFactorBandOffsets[band]; i < pScaleFactorBandOffsets[band + 1]; i++) {
            spectrumL[windowLen * window + i] >>= lScale;
            spectrumR[windowLen * window + i] >>= rScale;
          }

          /* now calculate downmix MDCT */
          if (pJointStereoData->MsUsed[band] & groupMask) {
            FDK_PRAGMA_MUST_ITERATE(
                pScaleFactorBandOffsets[band + 1] - pScaleFactorBandOffsets[band], 4, 96, 4)
            for (int i = pScaleFactorBandOffsets[band]; i < pScaleFactorBandOffsets[band + 1];
                 i++) {
              dmx_re[windowLen * window + i] = spectrumL[windowLen * window + i];
            }
          } else {
            if (cplxPredictionData->pred_dir == 0) {
              FDK_PRAGMA_MUST_ITERATE(
                  pScaleFactorBandOffsets[band + 1] - pScaleFactorBandOffsets[band], 4, 96, 4)
              for (int i = pScaleFactorBandOffsets[band]; i < pScaleFactorBandOffsets[band + 1];
                   i++) {
                dmx_re[windowLen * window + i] =
                    (spectrumL[windowLen * window + i] + spectrumR[windowLen * window + i]) >> 1;
              }
            } else {
              FDK_PRAGMA_MUST_ITERATE(
                  pScaleFactorBandOffsets[band + 1] - pScaleFactorBandOffsets[band], 4, 96, 4)
              for (int i = pScaleFactorBandOffsets[band]; i < pScaleFactorBandOffsets[band + 1];
                   i++) {
                dmx_re[windowLen * window + i] =
                    (spectrumL[windowLen * window + i] - spectrumR[windowLen * window + i]) >> 1;
              }
            }
          }

        } /* for ( band=0; band<max_sfb_ste; band++ ) */
#endif
        /* Clean until the end */
        for (int i = pScaleFactorBandOffsets[max_sfb_ste_outside]; i < windowLen; i++) {
          dmx_re[windowLen * window + i] = (FIXP_DBL)0;
        }

        /* 3. calculate MDST-portion corresponding to the current frame. */
        if (cplxPredictionData->complex_coef == 1) {
          /* If transform splitting is active */
          if (pAacDecoderChannelInfo[L]->transform_splitting_active) {
            /* Deinterleave */
            const int halfTransLength = 512;
            const int halfLowpassLine = pScaleFactorBandOffsets[max_sfb_ste_outside] >> 1;

            /* The length of the filter processing must be extended because of filter boundary
             * problems */
            int extended_band = fMin(halfLowpassLine + 7, halfTransLength);

            if ((previousShape == SHAPE_SINE) && (currentShape == SHAPE_SINE)) {
              coeffPointerOffset = 0;
            } else if ((previousShape == SHAPE_SINE) && (currentShape == SHAPE_KBD)) {
              coeffPointerOffset = 2;
            } else if ((previousShape == SHAPE_KBD) && (currentShape == SHAPE_KBD)) {
              coeffPointerOffset = 1;
            } else /* if ( (previousShape == SHAPE_KBD) && (currentShape == SHAPE_SINE) ) */
            {
              coeffPointerOffset = 3;
            }

            pCoeff = mdst_filt_coef_curr[16 + coeffPointerOffset];
            pCoeffPrev = mdst_filt_coef_prev[4 + previousShape];

            CJointStereo_filterAndAdd_Int(dmx_re, extended_band, halfTransLength, halfLowpassLine,
                                          pCoeff, dmx_im, 1);

            if (cplxPredictionData->use_prev_frame == 1) {
              CJointStereo_filterAndAdd_Int(&dmx_re_prev[1], extended_band, halfTransLength,
                                            extended_band, pCoeffPrev, dmx_im, 0);
            }

            if (currentShape == SHAPE_SINE) {
              coeffPointerOffset = 0;
            } else {
              coeffPointerOffset = 1;
            }

            pCoeff = mdst_filt_coef_curr[16 + coeffPointerOffset];
            pCoeffPrev = mdst_filt_coef_prev[4 + currentShape];

            CJointStereo_filterAndAdd_Int(dmx_re + 1, extended_band, halfTransLength,
                                          halfLowpassLine, pCoeff, &dmx_im[1], 1);

            CJointStereo_filterAndAdd_Int(dmx_re, extended_band, halfTransLength, halfLowpassLine,
                                          pCoeffPrev, &dmx_im[1], 0);

          } else {
            /* 3.1 move pointer in filter-coefficient table in case of short window sequence */
            /*     (other coefficients are utilized for the last 7 short windows)            */
            if ((pAacDecoderChannelInfo[L]->icsInfo.WindowSequence == BLOCK_SHORT) &&
                (window != 0)) {
              pCoeff = mdst_filt_coef_curr[currentShape];
              pCoeffPrev = mdst_filt_coef_prev[currentShape];
            }

            /* The length of the filter processing must be extended because of filter boundary
             * problems */
            int extended_band = fMin(pScaleFactorBandOffsets[max_sfb_ste_outside] + 7, windowLen);

            /* 3.2. estimate downmix MDST from current frame downmix MDCT */
            if ((pAacDecoderChannelInfo[L]->icsInfo.WindowSequence == BLOCK_SHORT) &&
                (window != 0)) {
              CJointStereo_filterAndAdd(&dmx_re[windowLen * window], extended_band, windowLen,
                                        pCoeff, &dmx_im[windowLen * window], 1);

              CJointStereo_filterAndAdd(&dmx_re[windowLen * (window - 1)], extended_band, windowLen,
                                        pCoeffPrev, &dmx_im[windowLen * window], 0);
            } else {
              CJointStereo_filterAndAdd(dmx_re, extended_band, windowLen, pCoeff, dmx_im, 1);

              if (cplxPredictionData->use_prev_frame == 1) {
                CJointStereo_filterAndAdd(dmx_re_prev, extended_band, windowLen, pCoeffPrev,
                                          &dmx_im[windowLen * window], 0);
              }
            }

          } /* if(pAacDecoderChannelInfo[L]->transform_splitting_active) */
        }   /* if ( pJointStereoData->complex_coef == 1 ) */

        /* 4. upmix process */
#ifdef FUNCTION_CJointStereo_ApplyMS_func3
        CJointStereo_ApplyMS_func3((FIXP_DBL * RESTRICT) & spectrumL[windowLen * window],
                                   (FIXP_DBL * RESTRICT) & spectrumR[windowLen * window],
                                   (FIXP_DBL * RESTRICT) & dmx_re[windowLen * window],
                                   (FIXP_DBL * RESTRICT) & dmx_im[windowLen * window],
                                   (FIXP_SGL * RESTRICT) cplxPredictionData->alpha_q_re[group],
                                   (FIXP_SGL * RESTRICT) cplxPredictionData->alpha_q_im[group],
                                   (SHORT * RESTRICT) pScaleFactorBandOffsets,
                                   (INT)max_sfb_ste_outside,
                                   (UCHAR * RESTRICT) pJointStereoData->MsUsed, (UCHAR)groupMask,
                                   cplxPredictionData->pred_dir ? (SHORT)-1 : (SHORT)1);
#else
        LONG pred_dir = cplxPredictionData->pred_dir ? -1 : 1;
        /* 0.1 in Q-3.34 */
        const FIXP_DBL pointOne = 0x66666666; /* 0.8 */
        /* Shift value for the downmix */
        const INT shift_dmx = SF_FNA_COEFFS + 1;

        for (band = 0; band < max_sfb_ste_outside; band++) {
          if (pJointStereoData->MsUsed[band] & groupMask) {
            FIXP_SGL tempRe = (FIXP_SGL)cplxPredictionData->alpha_q_re[group][band];
            FIXP_SGL tempIm = (FIXP_SGL)cplxPredictionData->alpha_q_im[group][band];

            /* Find the minimum common headroom for alpha_re and alpha_im */
            int alpha_re_headroom = CountLeadingBits((INT)tempRe) - 16;
            if (tempRe == (FIXP_SGL)0) alpha_re_headroom = 15;
            int alpha_im_headroom = CountLeadingBits((INT)tempIm) - 16;
            if (tempIm == (FIXP_SGL)0) alpha_im_headroom = 15;
            int val = fMin(alpha_re_headroom, alpha_im_headroom);

            /* Multiply alpha by 0.1 with maximum precision */
            FDK_ASSERT(val >= 0);
            FIXP_DBL alpha_re_tmp = fMult((FIXP_SGL)(tempRe << val), pointOne);
            FIXP_DBL alpha_im_tmp = fMult((FIXP_SGL)(tempIm << val), pointOne);

            /* Calculate alpha exponent */
            /* (Q-3.34 * Q15.0) shifted left by "val" */
            int alpha_re_exp = -3 + 15 - val;

            int help3_shift = alpha_re_exp + 1;

            FIXP_DBL* p2CoeffL = &(spectrumL[windowLen * window + pScaleFactorBandOffsets[band]]);
            FIXP_DBL* p2CoeffR = &(spectrumR[windowLen * window + pScaleFactorBandOffsets[band]]);
            FIXP_DBL* p2dmxIm = &(dmx_im[windowLen * window + pScaleFactorBandOffsets[band]]);
            FIXP_DBL* p2dmxRe = &(dmx_re[windowLen * window + pScaleFactorBandOffsets[band]]);

            for (int i = pScaleFactorBandOffsets[band]; i < pScaleFactorBandOffsets[band + 1];
                 i++) {
              /* Calculating helper term:
                    side = specR[i] - alpha_re[i] * dmx_re[i] - alpha_im[i] * dmx_im[i];

                Here "dmx_re" may be the same as "specL" or alternatively keep the downmix.
                "dmx_re" and "specL" are two different pointers pointing to separate arrays, which
                may or may not contain the same data (with different scaling).

                specL[i] =   + (specL[i] + side);
                specR[i] = -/+ (specL[i] - side);
              */
              FIXP_DBL side, left, right;

              side = fMultAddDiv2(fMultDiv2(alpha_re_tmp, *p2dmxRe++), alpha_im_tmp,
                                  (*p2dmxIm++) << shift_dmx);
              side = ((*p2CoeffR) >> 2) -
                     (FIXP_DBL)SATURATE_SHIFT(side, -(help3_shift - 2), DFRACT_BITS - 2);

              left = ((*p2CoeffL) >> 2) + side;
              right = ((*p2CoeffL) >> 2) - side;
              right = (FIXP_DBL)((LONG)right * pred_dir);

              *p2CoeffL++ = SATURATE_LEFT_SHIFT_ALT(left, 2, DFRACT_BITS);
              *p2CoeffR++ = SATURATE_LEFT_SHIFT_ALT(right, 2, DFRACT_BITS);
            }
          }

        } /* for ( band=0; band < max_sfb_ste; band++ ) */
#endif  /* #ifdef FUNCTION_CJointStereo_ApplyMS_func3 */
      } /* for ( groupwin=0; groupwin<pWindowGroupLength[group]; groupwin++, window++ ) */

    } /* for ( window = 0, group = 0; group < windowGroups; group++ ) */
  } else {
    /* MS stereo */

    for (window = 0, group = 0; group < windowGroups; group++) {
      groupMask = 1 << group;

      for (int groupwin = 0; groupwin < pWindowGroupLength[group]; groupwin++, window++) {
        FIXP_DBL *leftSpectrum, *rightSpectrum;
        SHORT* leftScale = &SFBleftScale[window * 16];
        SHORT* rightScale = &SFBrightScale[window * 16];

        leftSpectrum = SPEC(spectrumL, window, pAacDecoderChannelInfo[L]->granuleLength);
        rightSpectrum = SPEC(spectrumR, window, pAacDecoderChannelInfo[R]->granuleLength);

        for (band = 0; band < max_sfb_ste_outside; band++) {
          if (pJointStereoData->MsUsed[band] & groupMask) {
            int lScale = leftScale[band];
            int rScale = rightScale[band];
            int commonScale = lScale > rScale ? lScale : rScale;
            unsigned int offsetCurrBand, offsetNextBand;

            /* ISO/IEC 14496-3 Chapter 4.6.8.1.1 :
               M/S joint channel coding can only be used if common_window is 1. */
            FDK_ASSERT(GetWindowSequence(&pAacDecoderChannelInfo[L]->icsInfo) ==
                       GetWindowSequence(&pAacDecoderChannelInfo[R]->icsInfo));
            FDK_ASSERT(GetWindowShape(&pAacDecoderChannelInfo[L]->icsInfo) ==
                       GetWindowShape(&pAacDecoderChannelInfo[R]->icsInfo));

            commonScale++;
            leftScale[band] = commonScale;
            rightScale[band] = commonScale;

            lScale = fMin(DFRACT_BITS - 1, commonScale - lScale);
            rScale = fMin(DFRACT_BITS - 1, commonScale - rScale);

            FDK_ASSERT(lScale >= 0 && rScale >= 0);

            offsetCurrBand = pScaleFactorBandOffsets[band];
            offsetNextBand = pScaleFactorBandOffsets[band + 1];

#if defined(FUNCTION_CJointStereo_GenerateMSOutput)
            CJointStereo_GenerateMSOutput(&(leftSpectrum[offsetCurrBand]),
                                          &(rightSpectrum[offsetCurrBand]), lScale, rScale,
                                          offsetNextBand - offsetCurrBand);
#else
            for (unsigned int index = offsetCurrBand; index < offsetNextBand; index++) {
              FIXP_DBL leftCoefficient = leftSpectrum[index];
              FIXP_DBL rightCoefficient = rightSpectrum[index];

              /* MS output generation */
              leftCoefficient >>= lScale;
              rightCoefficient >>= rScale;

              leftSpectrum[index] = leftCoefficient + rightCoefficient;
              rightSpectrum[index] = leftCoefficient - rightCoefficient;
            }
#endif
          }
        }
        if (scaleFactorBandsTransmittedL > scaleFactorBandsTransmitted) {
          for (; band < scaleFactorBandsTransmittedL; band++) {
            if (pJointStereoData->MsUsed[band] & groupMask) {
              rightScale[band] = leftScale[band];

              FDK_PRAGMA_MUST_ITERATE(
                  pScaleFactorBandOffsets[band + 1] - pScaleFactorBandOffsets[band], 4, 96, 4)
              for (int index = pScaleFactorBandOffsets[band];
                   index < pScaleFactorBandOffsets[band + 1]; index++) {
                FIXP_DBL leftCoefficient = leftSpectrum[index];
                /* FIXP_DBL rightCoefficient = (FIXP_DBL)0; */
                rightSpectrum[index] = leftCoefficient;
              }
            }
          }
        } else if (scaleFactorBandsTransmittedR > scaleFactorBandsTransmitted) {
          for (; band < scaleFactorBandsTransmittedR; band++) {
            if (pJointStereoData->MsUsed[band] & groupMask) {
              leftScale[band] = rightScale[band];

              FDK_PRAGMA_MUST_ITERATE(
                  pScaleFactorBandOffsets[band + 1] - pScaleFactorBandOffsets[band], 4, 96, 4)
              for (int index = pScaleFactorBandOffsets[band];
                   index < pScaleFactorBandOffsets[band + 1]; index++) {
                /* FIXP_DBL leftCoefficient  = (FIXP_DBL)0; */
                FIXP_DBL rightCoefficient = rightSpectrum[index];

                leftSpectrum[index] = rightCoefficient;
                rightSpectrum[index] = fNegateSaturate(rightCoefficient);
              }
            }
          }
        }
      }
    }

    /* Reset MsUsed flags if no explicit signalling was transmitted. Necessary for intensity coding.
       PNS correlation signalling was mapped before calling CJointStereo_ApplyMS(). */
    if (pJointStereoData->MsMaskPresent == 2) {
    }
  }
}

void CJointStereo_ApplyMSIGFcore(CAacDecoderChannelInfo* pAacDecoderChannelInfo[2],
                                 CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo[2],
                                 const SHORT* pScaleFactorBandOffsets,
                                 const UCHAR* pWindowGroupLength, const int windowGroups,
                                 const int max_sfb_ste, const int scaleFactorBandsTransmittedL,
                                 const int scaleFactorBandsTransmittedR) {
  INT NumberOfTiles = iisIGFDecLibGetNumberOfTiles(
      &pAacDecoderStaticChannelInfo[L]->IGF_StaticData,
      GetWindowSequence(&pAacDecoderChannelInfo[L]->icsInfo) == BLOCK_SHORT);

  /* Set pointers to the already computed previous DMX and its exaponent */
  FIXP_DBL* PreviousDMX = pAacDecoderChannelInfo[L]->pComData->pJointStereoData->store_dmx_re_prev;
  SHORT* PreviousDMXexp =
      &(pAacDecoderChannelInfo[L]->pComData->pJointStereoData->store_dmx_re_prev_e);

  /* Apply MS over tiles */
  for (int tileIdx = 0; tileIdx < NumberOfTiles; tileIdx++) {
    /* Get tile spectrum */
    FIXP_DBL* tile_spectrumL = iisIGFDecLibAccessSourceSpectrum(
        &pAacDecoderStaticChannelInfo[L]->IGF_StaticData, &pAacDecoderChannelInfo[L]->IGFdata,
        tileIdx, GetWindowSequence(&pAacDecoderChannelInfo[L]->icsInfo) == BLOCK_SHORT, 0);
    FIXP_DBL* tile_spectrumR = iisIGFDecLibAccessSourceSpectrum(
        &pAacDecoderStaticChannelInfo[R]->IGF_StaticData, &pAacDecoderChannelInfo[R]->IGFdata,
        tileIdx, GetWindowSequence(&pAacDecoderChannelInfo[R]->icsInfo) == BLOCK_SHORT, 0);

    /* Get tile SFB scaling data */
    SHORT* leftScale = iisIGFDecLibAccessSourceSpectrum_exponent(
        &pAacDecoderStaticChannelInfo[L]->IGF_StaticData, tileIdx,
        GetWindowSequence(&pAacDecoderChannelInfo[L]->icsInfo) == BLOCK_SHORT);
    SHORT* rightScale = iisIGFDecLibAccessSourceSpectrum_exponent(
        &pAacDecoderStaticChannelInfo[R]->IGF_StaticData, tileIdx,
        GetWindowSequence(&pAacDecoderChannelInfo[R]->icsInfo) == BLOCK_SHORT);

    /* Set a pointer to the spectrum scale values to be computed */
    SHORT* specScaleL = iisIGFDecLibAccessSourceSpectrum_Scale(
        &pAacDecoderStaticChannelInfo[L]->IGF_StaticData, &pAacDecoderChannelInfo[L]->IGFdata,
        tileIdx, GetWindowSequence(&pAacDecoderChannelInfo[L]->icsInfo) == BLOCK_SHORT);
    SHORT* specScaleR = iisIGFDecLibAccessSourceSpectrum_Scale(
        &pAacDecoderStaticChannelInfo[R]->IGF_StaticData, &pAacDecoderChannelInfo[R]->IGFdata,
        tileIdx, GetWindowSequence(&pAacDecoderChannelInfo[R]->icsInfo) == BLOCK_SHORT);

    /* Apply MS over the tiles */
    CJointStereo_ApplyMS(pAacDecoderChannelInfo, pAacDecoderStaticChannelInfo, tile_spectrumL,
                         tile_spectrumR, leftScale, rightScale, specScaleL, specScaleR,
                         pScaleFactorBandOffsets, pWindowGroupLength, windowGroups, max_sfb_ste,
                         scaleFactorBandsTransmittedL, scaleFactorBandsTransmittedR, PreviousDMX,
                         PreviousDMXexp, 0);

  } /* for(int tileIdx=0;tileIdx<NumberOfTiles;tileIdx++) */
}

void CJointStereo_ApplyMS_IGF(CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo[2],
                              CAacDecoderChannelInfo* pAacDecoderChannelInfo[2], FIXP_DBL* pSpecL,
                              FIXP_DBL* pSpecR, const SHORT* pScaleFactorBandOffsets,
                              const UCHAR* pWindowGroupLength, const int windowGroups) {
  int window, group, band;
  UCHAR groupMask;
  CJointStereoData* pJointStereoData = pAacDecoderChannelInfo[L]->pComData->pJointStereoData;
  CCplxPredictionData* cplxPredictionData =
      pAacDecoderChannelInfo[L]->pComStaticData->cplxPredictionData;

  int igfSfbStart, igfSfbStop;

  /* Exit if the MS mask is all zero. Nothing to do */
  if (pJointStereoData->IGF_MsMaskPresent == 0) return;

  if (IsLongBlock(&pAacDecoderChannelInfo[0]->icsInfo)) {
    igfSfbStart = pAacDecoderStaticChannelInfo[0]->IGF_StaticData.igfStartSfbLB;
    igfSfbStop = pAacDecoderStaticChannelInfo[0]->IGF_StaticData.igfStopSfbLB;

  } else {
    igfSfbStart = pAacDecoderStaticChannelInfo[0]->IGF_StaticData.igfStartSfbSB;
    igfSfbStop = pAacDecoderStaticChannelInfo[0]->IGF_StaticData.igfStopSfbSB;
  }

  if ((pJointStereoData->igf_cplx_pred_flag) && (pJointStereoData->IGF_MsMaskPresent == 3)) {
    int i, windowLen, groupwin;

    /* get length of short window for current configuration */
    windowLen = pAacDecoderChannelInfo[L]
                    ->granuleLength; /* framelength 768 => 96, framelength 1024 => 128 */
    /* if this is no short-block set length for long-block */
    if (pAacDecoderChannelInfo[L]->icsInfo.WindowSequence != BLOCK_SHORT) {
      windowLen *= 8;
    }

    /* process on window-basis (i.e. iterate over all groups and corresponding windows) */
    for (window = 0, group = 0; group < windowGroups; group++) {
      groupMask = 1 << group;

      for (groupwin = 0; groupwin < pWindowGroupLength[group]; groupwin++, window++) {
        int lScale;
        int rScale;
        int temp;

        temp = pAacDecoderChannelInfo[L]->specScale[window] -
               pAacDecoderChannelInfo[R]->specScale[window];
        if (temp >= 0) {
          lScale = 2;
          rScale = fMin(31, temp + 2);
        } else {
          lScale = fMin(31, -temp + 2);
          rScale = 2;
        }

        /* rescale signal if necessary*/
        {
          for (band = igfSfbStart; band < igfSfbStop; band++) {
            if (pJointStereoData->MsUsed[band] & groupMask) {
              FIXP_DBL* p2_pSpectralCoefficientL =
                  &(pSpecL[windowLen * window + pScaleFactorBandOffsets[band]]);
              FIXP_DBL* p2_pSpectralCoefficientR =
                  &(pSpecR[windowLen * window + pScaleFactorBandOffsets[band]]);
              for (i = pScaleFactorBandOffsets[band]; i < pScaleFactorBandOffsets[band + 1]; i++) {
                *p2_pSpectralCoefficientL++ >>= lScale;
                *p2_pSpectralCoefficientR++ >>= rScale;
              }
            }
          } /* for ( band=0; band<max_sfb_ste; band++ ) */
        }

        /* Upmix process */
        INT igf_pred_dir = cplxPredictionData->igf_pred_dir ? -1 : 1;

        /* 0.1 in Q-3.34 */
        const FIXP_DBL pointOne = 0x66666666;

        for (band = igfSfbStart; band < igfSfbStop; band++) {
          if (pJointStereoData->MsUsed[band] & groupMask) {
            FIXP_SGL tempRe = (FIXP_SGL)cplxPredictionData->alpha_q_re[group][band];

            /* Find the minimum common headroom for alpha_re and alpha_im */
            int alpha_re_headroom = CountLeadingBits(((INT)tempRe) << 16);

            /* Multiply alpha by 0.1 with maximum precision */
            FIXP_DBL alpha_re_tmp = fMult((FIXP_SGL)(tempRe << alpha_re_headroom), pointOne);

            /* Calculate shift exponent */
            /* (Q-3.34 * Q15.0) shifted left by "val" and + 1 because of Div2 Mult */
            int help1_shift = 13 - alpha_re_headroom;

            FIXP_DBL* p2CoeffL = &(pSpecL[windowLen * window + pScaleFactorBandOffsets[band]]);
            FIXP_DBL* p2CoeffR = &(pSpecR[windowLen * window + pScaleFactorBandOffsets[band]]);

            for (i = pScaleFactorBandOffsets[band]; i < pScaleFactorBandOffsets[band + 1]; i++) {
              /* Calculating: side = specR[i] - alpha_re[i] * specL[i]; */

              /* alpha_re[i] * specL[i] */
              FIXP_DBL help1 = fMultDiv2(alpha_re_tmp, *p2CoeffL);

              /* side = specR[i] - alpha_re[i] * specL[i] */
              FIXP_DBL help2 =
                  *p2CoeffR - (FIXP_DBL)SATURATE_SHIFT(help1, -help1_shift, DFRACT_BITS - 2);

              /* specR[i] = -/+ (specL[i] - side; */
              *p2CoeffR = SATURATE_LEFT_SHIFT(
                  (FIXP_DBL)((LONG)(*p2CoeffL - help2) * (LONG)igf_pred_dir), rScale, DFRACT_BITS);
              p2CoeffR++;

              /* specL[i] = specL[i] + side; */
              *p2CoeffL = SATURATE_LEFT_SHIFT((*p2CoeffL + help2), lScale, DFRACT_BITS);
              p2CoeffL++;
            }

          } /* if ( pJointStereoData->MsUsed[band] & groupMask ) */

        } /* for ( band=0; band < max_sfb_ste; band++ ) */

      } /* for ( groupwin=0; groupwin<pWindowGroupLength[group]; groupwin++, window++ ) */

    } /* for ( window = 0, group = 0; group < windowGroups; group++ ) */

    /* free scratch buffer */
    /* C_AALLOC_SCRATCH_END(dmx_im, FIXP_DBL, 1024); */

  } else /* Complex prediction end*/

  {
    int lScale;
    int rScale;
    int temp;
    FIXP_DBL *leftSpectrum, *rightSpectrum;

    /* MS stereo */

    for (window = 0, group = 0; group < windowGroups; group++) {
      groupMask = 1 << group;

      for (int groupwin = 0; groupwin < pWindowGroupLength[group]; groupwin++, window++) {
        /* Find if a difference between the scalings exists. Add 1-bit headroom because of the
         * addtion/subtraction */
        temp = pAacDecoderChannelInfo[L]->specScale[window] -
               pAacDecoderChannelInfo[R]->specScale[window];
        if (temp >= 0) {
          lScale = 1;
          rScale = fMin(31, temp + 1);
        } else {
          lScale = fMin(31, -temp + 1);
          rScale = 1;
        }

        leftSpectrum = (FIXP_DBL*)(pSpecL) + window * pAacDecoderChannelInfo[L]->granuleLength;
        rightSpectrum = (FIXP_DBL*)(pSpecR) + window * pAacDecoderChannelInfo[R]->granuleLength;

        for (band = igfSfbStart; band < igfSfbStop; band++) {
          if (pJointStereoData->MsUsed[band] & groupMask) {
            FIXP_DBL* leftCoefficient = &leftSpectrum[pScaleFactorBandOffsets[band]];
            FIXP_DBL* rightCoefficient = &rightSpectrum[pScaleFactorBandOffsets[band]];

            for (int index = pScaleFactorBandOffsets[band];
                 index < pScaleFactorBandOffsets[band + 1]; index++) {
              FIXP_DBL temp_left, temp_right;

              /* MS output generation */
              temp_left = *leftCoefficient >>= lScale;
              temp_right = *rightCoefficient >>= rScale;

              *leftCoefficient++ =
                  SATURATE_LEFT_SHIFT((temp_left + temp_right), lScale, DFRACT_BITS);
              *rightCoefficient++ =
                  SATURATE_LEFT_SHIFT((temp_left - temp_right), rScale, DFRACT_BITS);
            }
          }
        }
      }
    }
  }
}

void IGF_StereoFilling_GetPreviousDmx(CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo[2],
                                      CAacDecoderChannelInfo* pAacDecoderChannelInfo[2],
                                      FIXP_DBL* dmx_prev, SHORT* dmx_prev_win_exp,
                                      const SHORT* pScaleFactorBandOffsets,
                                      const UCHAR* pWindowGroupLength, const int windowGroups,
                                      const INT max_noise_sfb, const UCHAR* band_is_noise) {
  int window, group;
  int noiseFillingStartOffset, sfb, nfStartOffset_sfb;
  int lScale, rScale, temp;
  FIXP_DBL *leftSpectrum, *rightSpectrum, *outSpectrum;

  FIXP_DBL* const staticSpectralCoeffsL =
      pAacDecoderStaticChannelInfo[L]->pCpeStaticData->jointStereoPersistentData.spectralCoeffs[L];
  FIXP_DBL* const staticSpectralCoeffsR =
      pAacDecoderStaticChannelInfo[L]->pCpeStaticData->jointStereoPersistentData.spectralCoeffs[R];
  SHORT* const staticSpecScaleL =
      pAacDecoderStaticChannelInfo[L]->pCpeStaticData->jointStereoPersistentData.specScale[L];
  SHORT* const staticSpecScaleR =
      pAacDecoderStaticChannelInfo[L]->pCpeStaticData->jointStereoPersistentData.specScale[R];
  CCplxPredictionData* cplxPredictionData =
      pAacDecoderChannelInfo[L]->pComStaticData->cplxPredictionData;

  noiseFillingStartOffset =
      (GetWindowSequence(&pAacDecoderChannelInfo[L]->icsInfo) == BLOCK_SHORT) ? 20 : 160;

  /* determine sfb from where on noise filling is applied */
  for (sfb = 0; pScaleFactorBandOffsets[sfb] < noiseFillingStartOffset; sfb++)
    ;
  nfStartOffset_sfb = sfb;

  /* MS stereo */
  if (pAacDecoderStaticChannelInfo[L]
          ->pCpeStaticData->jointStereoPersistentData.clearSpectralCoeffs == 1) {
    for (window = 0, group = 0; group < windowGroups; group++) {
      for (int groupwin = 0; groupwin < pWindowGroupLength[group]; groupwin++, window++) {
        outSpectrum = dmx_prev + window * pAacDecoderChannelInfo[R]->granuleLength;

        for (sfb = nfStartOffset_sfb; sfb < max_noise_sfb; sfb++) {
          /* Run algorithm only if the particular band has been fully noise filled, i.e. that it has
           * been empty before the noise-filling process */
          if (!band_is_noise[group * 16 + sfb]) {
            continue;
          }

          FIXP_DBL* outCoefficient = &outSpectrum[pScaleFactorBandOffsets[sfb]];

          for (int index = pScaleFactorBandOffsets[sfb]; index < pScaleFactorBandOffsets[sfb + 1];
               index++) {
            *outCoefficient++ = (FIXP_DBL)0;
          }
        } /* for (band=0; band<scaleFactorBandsTransmitted; band++) */
      }   /*  for (int groupwin=0; groupwin<pWindowGroupLength[group]; groupwin++, window++) */
    }
    return;
  }

  for (window = 0, group = 0; group < windowGroups; group++) {
    for (int groupwin = 0; groupwin < pWindowGroupLength[group]; groupwin++, window++) {
      /* Find if a difference between the scalings exists. Add 1-bit headroom because of the
       * addtion/subtraction */
      temp = staticSpecScaleL[window] - staticSpecScaleR[window];
      if (temp >= 0) {
        lScale = 1;
        rScale = fMin(31, temp + 1);
        dmx_prev_win_exp[window] = staticSpecScaleL[window];
      } else {
        lScale = fMin(31, -temp + 1);
        rScale = 1;
        dmx_prev_win_exp[window] = staticSpecScaleR[window];
      }

      leftSpectrum =
          (FIXP_DBL*)staticSpectralCoeffsL + window * pAacDecoderChannelInfo[L]->granuleLength;
      rightSpectrum =
          (FIXP_DBL*)staticSpectralCoeffsR + window * pAacDecoderChannelInfo[R]->granuleLength;
      outSpectrum = dmx_prev + window * pAacDecoderChannelInfo[R]->granuleLength;

      if (cplxPredictionData->pred_dir == 0) {
        for (sfb = nfStartOffset_sfb; sfb < max_noise_sfb; sfb++) {
          /* Run algorithm only if the particular band has been fully noise filled, i.e. that it has
            been empty before the noise-filling process */
          if (!band_is_noise[group * 16 + sfb]) {
            continue;
          }

          FIXP_DBL* leftCoefficient = &leftSpectrum[pScaleFactorBandOffsets[sfb]];
          FIXP_DBL* rightCoefficient = &rightSpectrum[pScaleFactorBandOffsets[sfb]];
          FIXP_DBL* outCoefficient = &outSpectrum[pScaleFactorBandOffsets[sfb]];

          for (int index = pScaleFactorBandOffsets[sfb]; index < pScaleFactorBandOffsets[sfb + 1];
               index++) {
            *outCoefficient++ = (*leftCoefficient++ >> lScale) + (*rightCoefficient++ >> rScale);
          }

        } /*for (sfb= nfStartOffset_sfb; sfb < max_noise_sfb; sfb++)*/
      } else {
        for (sfb = nfStartOffset_sfb; sfb < max_noise_sfb; sfb++) {
          /* Run algorithm only if the particular band has been fully noise filled, i.e. that it has
            been empty before the noise-filling process */
          if (!band_is_noise[group * 16 + sfb]) {
            continue;
          }

          FIXP_DBL* leftCoefficient = &leftSpectrum[pScaleFactorBandOffsets[sfb]];
          FIXP_DBL* rightCoefficient = &rightSpectrum[pScaleFactorBandOffsets[sfb]];
          FIXP_DBL* outCoefficient = &outSpectrum[pScaleFactorBandOffsets[sfb]];

          for (int index = pScaleFactorBandOffsets[sfb]; index < pScaleFactorBandOffsets[sfb + 1];
               index++) {
            *outCoefficient++ = (*leftCoefficient++ >> lScale) - (*rightCoefficient++ >> rScale);
          }

        } /*for (sfb= nfStartOffset_sfb; sfb < max_noise_sfb; sfb++)*/

      } /* if ( cplxPredictionData->pred_dir==0 )*/

    } /*  for (int groupwin=0; groupwin<pWindowGroupLength[group]; groupwin++, window++)*/
  }
}
