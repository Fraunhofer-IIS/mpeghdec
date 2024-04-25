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

/******************** MPEG-H 3DA object rendering library **********************

   Author(s):   Thomas Blender, Arthur Tritthart

   Description: Vector Base Amplitude Panning (VBAP)

*******************************************************************************/

#include "gVBAPRenderer.h"

/* FDK VBAP library info */
#define FDK_VBAP_LIB_VL0 1
#define FDK_VBAP_LIB_VL1 0
#define FDK_VBAP_LIB_VL2 0
#define FDK_VBAP_LIB_TITLE "FDK VBAP"
#ifdef __ANDROID__
#define FDK_VBAP_LIB_BUILD_DATE ""
#define FDK_VBAP_LIB_BUILD_TIME ""
#else
#define FDK_VBAP_LIB_BUILD_DATE __DATE__
#define FDK_VBAP_LIB_BUILD_TIME __TIME__
#endif

/* #include "coresup.h" --- needed for FDKclock in DEBUG_gVBAPRenderer_RenderFrame_Time_func1,2 */

#if defined(__arm__)
#include "arm/gVBAPRenderer_arm.cpp"
#endif

#define OBEJCT 0

#if (GVBAPRENDERER_MAX_FRAMELENGTH > 1024)
#define VBAP_SCALE_GAINS (4)
#else
#define VBAP_SCALE_GAINS (3)
#endif
#define VBAP_MAX_SCALE_OBJECTS (5)
#define VBAP_MAX_SCALE_SAMPLES (VBAP_SCALE_GAINS + VBAP_MAX_SCALE_OBJECTS)

/* spread_depth_table[s] = (2^(s/3)/2) - 0.5; with an exponent of 4  */
static const FIXP_DBL spread_depth_table[16] = {
    FL2FXCONST_DBL(0.000000000000000 / 16.), FL2FXCONST_DBL(0.129960524947437 / 16.),
    FL2FXCONST_DBL(0.293700525984100 / 16.), FL2FXCONST_DBL(0.500000000000000 / 16.),
    FL2FXCONST_DBL(0.759921049894873 / 16.), FL2FXCONST_DBL(1.087401051968200 / 16.),
    FL2FXCONST_DBL(1.500000000000000 / 16.), FL2FXCONST_DBL(2.019842099789750 / 16.),
    FL2FXCONST_DBL(2.674802103936400 / 16.), FL2FXCONST_DBL(3.500000000000000 / 16.),
    FL2FXCONST_DBL(4.539684199579490 / 16.), FL2FXCONST_DBL(5.849604207872800 / 16.),
    FL2FXCONST_DBL(7.500000000000000 / 16.), FL2FXCONST_DBL(9.579368399158980 / 16.),
    FL2FXCONST_DBL(12.19920841574560 / 16.), FL2FXCONST_DBL(15.50000000000000 / 16.)};

/* gain decoding table with an exponent of 3:
   gain[o] = pow(10.0, (gain[o] - 32.0) / 40.0);
   MIN(MAX(gain[o], 0.004), 5.957);
*/
#define GAIN_TAB FL2FXCONST_DBL
static const FIXP_DBL gain_dec_tab[129] = {
    GAIN_TAB(0.0005),      GAIN_TAB(0.000527121), GAIN_TAB(0.000558354), GAIN_TAB(0.000591439),
    GAIN_TAB(0.000626484), GAIN_TAB(0.000663606), GAIN_TAB(0.000702927), GAIN_TAB(0.000744578),
    GAIN_TAB(0.000788697), GAIN_TAB(0.00083543),  GAIN_TAB(0.000884932), GAIN_TAB(0.000937368),
    GAIN_TAB(0.00099291),  GAIN_TAB(0.00105174),  GAIN_TAB(0.00111406),  GAIN_TAB(0.00118008),
    GAIN_TAB(0.00125),     GAIN_TAB(0.00132407),  GAIN_TAB(0.00140252),  GAIN_TAB(0.00148563),
    GAIN_TAB(0.00157366),  GAIN_TAB(0.0016669),   GAIN_TAB(0.00176567),  GAIN_TAB(0.00187029),
    GAIN_TAB(0.00198112),  GAIN_TAB(0.00209851),  GAIN_TAB(0.00222285),  GAIN_TAB(0.00235456),
    GAIN_TAB(0.00249408),  GAIN_TAB(0.00264186),  GAIN_TAB(0.0027984),   GAIN_TAB(0.00296422),
    GAIN_TAB(0.00313986),  GAIN_TAB(0.00332591),  GAIN_TAB(0.00352298),  GAIN_TAB(0.00373173),
    GAIN_TAB(0.00395285),  GAIN_TAB(0.00418707),  GAIN_TAB(0.00443517),  GAIN_TAB(0.00469797),
    GAIN_TAB(0.00497634),  GAIN_TAB(0.00527121),  GAIN_TAB(0.00558354),  GAIN_TAB(0.00591439),
    GAIN_TAB(0.00626484),  GAIN_TAB(0.00663606),  GAIN_TAB(0.00702927),  GAIN_TAB(0.00744578),
    GAIN_TAB(0.00788697),  GAIN_TAB(0.0083543),   GAIN_TAB(0.00884932),  GAIN_TAB(0.00937368),
    GAIN_TAB(0.0099291),   GAIN_TAB(0.0105174),   GAIN_TAB(0.0111406),   GAIN_TAB(0.0118008),
    GAIN_TAB(0.0125),      GAIN_TAB(0.0132407),   GAIN_TAB(0.0140252),   GAIN_TAB(0.0148563),
    GAIN_TAB(0.0157366),   GAIN_TAB(0.016669),    GAIN_TAB(0.0176567),   GAIN_TAB(0.0187029),
    GAIN_TAB(0.0198112),   GAIN_TAB(0.0209851),   GAIN_TAB(0.0222285),   GAIN_TAB(0.0235456),
    GAIN_TAB(0.0249408),   GAIN_TAB(0.0264186),   GAIN_TAB(0.027984),    GAIN_TAB(0.0296422),
    GAIN_TAB(0.0313986),   GAIN_TAB(0.0332591),   GAIN_TAB(0.0352298),   GAIN_TAB(0.0373173),
    GAIN_TAB(0.0395285),   GAIN_TAB(0.0418707),   GAIN_TAB(0.0443517),   GAIN_TAB(0.0469797),
    GAIN_TAB(0.0497634),   GAIN_TAB(0.0527121),   GAIN_TAB(0.0558354),   GAIN_TAB(0.0591439),
    GAIN_TAB(0.0626484),   GAIN_TAB(0.0663606),   GAIN_TAB(0.0702927),   GAIN_TAB(0.0744578),
    GAIN_TAB(0.0788697),   GAIN_TAB(0.083543),    GAIN_TAB(0.0884932),   GAIN_TAB(0.0937368),
    GAIN_TAB(0.099291),    GAIN_TAB(0.105174),    GAIN_TAB(0.111406),    GAIN_TAB(0.118008),
    GAIN_TAB(0.125),       GAIN_TAB(0.132407),    GAIN_TAB(0.140252),    GAIN_TAB(0.148563),
    GAIN_TAB(0.157366),    GAIN_TAB(0.16669),     GAIN_TAB(0.176567),    GAIN_TAB(0.187029),
    GAIN_TAB(0.198112),    GAIN_TAB(0.209851),    GAIN_TAB(0.222285),    GAIN_TAB(0.235456),
    GAIN_TAB(0.249408),    GAIN_TAB(0.264186),    GAIN_TAB(0.27984),     GAIN_TAB(0.296422),
    GAIN_TAB(0.313986),    GAIN_TAB(0.332591),    GAIN_TAB(0.352298),    GAIN_TAB(0.373173),
    GAIN_TAB(0.395285),    GAIN_TAB(0.418707),    GAIN_TAB(0.443517),    GAIN_TAB(0.469797),
    GAIN_TAB(0.497634),    GAIN_TAB(0.527121),    GAIN_TAB(0.558354),    GAIN_TAB(0.591439),
    GAIN_TAB(0.626484),    GAIN_TAB(0.663606),    GAIN_TAB(0.702927),    GAIN_TAB(0.744578),
    GAIN_TAB(0.744625)};

int generateInverseMatrices(HANDLE_GVBAPRENDERER hgVBAPRenderer);

/**
 * @brief Parse a COAM I-Frame
 * @param  bs           bitstream structure
 * @param  pos          output values (azimuth, elevation, radius, gain, spread)
 * @param  fixed_val    flag (azimuth, elevation, radius, gain, spread)
 * @param  num_objects  number of objects
 * @param  hasDynamicObjectPriority
 * @param  hasUniformSpread
 */
static void get_iframe(HANDLE_FDK_BITSTREAM bs, INT* pos, UCHAR* fixed_val, UINT num_objects,
                       UCHAR hasDynamicObjectPriority, UCHAR hasUniformSpread) {
  int wordsize[6] = {OAM_BITS_AZI,  OAM_BITS_ELE,    OAM_BITS_RAD,
                     OAM_BITS_GAIN, OAM_BITS_SPREAD, OAM_BITS_OBJ_PRIORITY};
  int sign[6] = {1, 1, 0, 1, 0, 0}; /* 1 signed; 0 = unsigned */
  int common_val;
  UCHAR n, n0;

  /* Table 76 */
  /* intracoded_object_metadata_low_delay() */
  if (num_objects > 1) /* Number of objects in one single OAM frame */
  {
    for (n = 0; n < (OAM_NUMBER_COMPONENTS + hasDynamicObjectPriority); n++) {
      fixed_val[n] = FDKreadBit(
          bs); /* fixed {azimuth; elevation; radius; gain; spread; dynamic_object_priority} */
      if (!hasUniformSpread && (n == SPREAD)) {
        fixed_val[SPREAD_HEIGHT] = fixed_val[n];
        fixed_val[SPREAD_DEPTH] = fixed_val[n];
      }
      if (fixed_val[n]) {
        pos[n] = FDKreadBits(bs, wordsize[n]); /* default {azimuth; elevation; radius; gain; spread;
                                                  dynamic_object_priority} */
        /* shift signed bit to right position */
        if (sign[n]) {
          pos[n] = pos[n] << (32 - wordsize[n]);
          pos[n] = pos[n] >> (32 - wordsize[n]);
        }
        for (n0 = OAM_NUMBER_MAX_COMPONENTS; n0 < OAM_NUMBER_MAX_COMPONENTS * num_objects;
             n0 += OAM_NUMBER_MAX_COMPONENTS) {
          pos[n + n0] = pos[n]; /* copy fixed value for all objects */
        }
        if (!hasUniformSpread && (n == SPREAD)) {
          pos[SPREAD_HEIGHT] =
              (UCHAR)FDKreadBits(bs, OAM_BITS_SPREAD_HEIGHT); /* position spread_height */
          pos[SPREAD_DEPTH] =
              (UCHAR)FDKreadBits(bs, OAM_BITS_SPREAD_DEPTH); /* position spread_depth */
          for (n0 = OAM_NUMBER_MAX_COMPONENTS; n0 < OAM_NUMBER_MAX_COMPONENTS * num_objects;
               n0 += OAM_NUMBER_MAX_COMPONENTS) {
            pos[SPREAD_HEIGHT + n0] = pos[SPREAD_HEIGHT]; /* copy fixed value for all objects */
            pos[SPREAD_DEPTH + n0] = pos[SPREAD_DEPTH];   /* copy fixed value for all objects */
          }
        }
      } else {
        common_val = FDKreadBit(
            bs); /* common {azimuth; elevation; radius; gain; spread; dynamic_object_priority} */

        if (common_val) {
          pos[n] = FDKreadBits(bs, wordsize[n]); /* default {azimuth; elevation; radius; gain;
                                                    spread; dynamic_object_priority} */
          /* shift signed bit to right position */
          if (sign[n]) {
            pos[n] = pos[n] << (32 - wordsize[n]);
            pos[n] = pos[n] >> (32 - wordsize[n]);
          }
          for (n0 = OAM_NUMBER_MAX_COMPONENTS; n0 < OAM_NUMBER_MAX_COMPONENTS * num_objects;
               n0 += OAM_NUMBER_MAX_COMPONENTS) {
            pos[n + n0] = pos[n]; /* copy common value for all objects */
          }
          if (!hasUniformSpread && (n == SPREAD)) {
            pos[SPREAD_HEIGHT] =
                (UCHAR)FDKreadBits(bs, OAM_BITS_SPREAD_HEIGHT); /* position spread_height */
            pos[SPREAD_DEPTH] =
                (UCHAR)FDKreadBits(bs, OAM_BITS_SPREAD_DEPTH); /* position spread_depth */
            for (n0 = OAM_NUMBER_MAX_COMPONENTS; n0 < OAM_NUMBER_MAX_COMPONENTS * num_objects;
                 n0 += OAM_NUMBER_MAX_COMPONENTS) {
              pos[SPREAD_HEIGHT + n0] = pos[SPREAD_HEIGHT]; /* copy fixed value for all objects */
              pos[SPREAD_DEPTH + n0] = pos[SPREAD_DEPTH];   /* copy fixed value for all objects */
            }
          }
        } else {
          for (n0 = 0; n0 < OAM_NUMBER_MAX_COMPONENTS * num_objects;
               n0 += OAM_NUMBER_MAX_COMPONENTS) {
            pos[n0 + n] = FDKreadBits(bs, wordsize[n]); /* position {azimuth; elevation; radius;
                                                           gain; spread; dynamic_object_priority} */
            /* shift signed bit to right position */
            if (sign[n]) {
              pos[n0 + n] = pos[n0 + n] << (32 - wordsize[n]);
              pos[n0 + n] = pos[n0 + n] >> (32 - wordsize[n]);
            }
            if (!hasUniformSpread && (n == SPREAD)) {
              pos[SPREAD_HEIGHT + n0] =
                  (UCHAR)FDKreadBits(bs, OAM_BITS_SPREAD_HEIGHT); /* position spread_height */
              pos[SPREAD_DEPTH + n0] =
                  (UCHAR)FDKreadBits(bs, OAM_BITS_SPREAD_DEPTH); /* position spread_depth */
            }
          }
        } /* else of if (common_val) */
      }   /* else of if (fixed_val[n]) */
    }     /* for loop over n */
  }       /* if (num_objects > 1) */
  else {
    for (n = 0; n < (OAM_NUMBER_COMPONENTS + hasDynamicObjectPriority); n++) {
      pos[n] = (UCHAR)FDKreadBits(bs, wordsize[n]); /* position {azimuth; elevation; radius; gain;
                                                       spread; dynamic_object_priority} */
      /* shift signed bit to right position */
      if (sign[n]) {
        pos[n] = pos[n] << (32 - wordsize[n]);
        pos[n] = pos[n] >> (32 - wordsize[n]);
      }
      if (!hasUniformSpread && (n == SPREAD)) {
        pos[SPREAD_HEIGHT] =
            (UCHAR)FDKreadBits(bs, OAM_BITS_SPREAD_HEIGHT); /* position spread_height */
        pos[SPREAD_DEPTH] =
            (UCHAR)FDKreadBits(bs, OAM_BITS_SPREAD_DEPTH); /* position spread_depth */
      }
    }
  }
}

/**
 * @brief Parse a COAM D-Frame
 * @param  bs           bitstream structure
 * @param  pos          output values (azimuth, elevation, radius, gain, spread)
 * @param  fixed_val    flag (azimuth, elevation, radius, gain, spread)
 * @param  num_objects  number of objects
 * @param  hasDynamicObjectPriority
 * @param  hasUniformSpread
 */
static void get_dframe(HANDLE_FDK_BITSTREAM bs, INT* pos, const UCHAR* fixed_val, UINT num_objects,
                       UCHAR hasDynamicObjectPriority, UCHAR hasUniformSpread) {
  UCHAR flag_absolute, has_object_data;
  UINT obj;
  UCHAR n;
  UCHAR n0 = 0;

  UCHAR num_bits;
  UCHAR flag_val;

  UCHAR bits;

  INT relative_part; /* 32 bit signed integer */

  int wordsize[6] = {OAM_BITS_AZI,  OAM_BITS_ELE,    OAM_BITS_RAD,
                     OAM_BITS_GAIN, OAM_BITS_SPREAD, OAM_BITS_OBJ_PRIORITY};
  int sign[6] = {1, 1, 0, 1, 0, 0}; /* 1 signed; 0 = unsigned */

  /* Table 77 */
  /* dynamic_object_metadata() */
  flag_absolute = FDKreadBit(bs);
  for (obj = 0; obj < num_objects; obj++) {
    has_object_data = FDKreadBit(bs);
    if (has_object_data) {
      /* Table 78 */
      /* single_dynamic_object_metadata() */
      if (flag_absolute) {
        for (n = 0; n < (OAM_NUMBER_COMPONENTS + hasDynamicObjectPriority); n++) {
          pos[n0 + n] = 0;
          if (fixed_val[n] == 0) {
            pos[n0 + n] = FDKreadBits(bs, wordsize[n]);
            if (sign[n]) { /* recover sign bit */
              pos[n0 + n] = pos[n0 + n] << (32 - wordsize[n]);
              pos[n0 + n] = pos[n0 + n] >> (32 - wordsize[n]);
            }
            if (!hasUniformSpread && (n == SPREAD)) {
              pos[SPREAD_HEIGHT + n0] =
                  (UCHAR)FDKreadBits(bs, OAM_BITS_SPREAD_HEIGHT); /* position spread_height */
              pos[SPREAD_DEPTH + n0] =
                  (UCHAR)FDKreadBits(bs, OAM_BITS_SPREAD_DEPTH); /* position spread_depth */
            }
          } else
            pos[n0 + n] = 0;
        }
      } else /* DPCM */
      {
        num_bits = FDKreadBits(bs, 3);
        num_bits += 2;
        for (n = 0; n < (OAM_NUMBER_COMPONENTS + hasDynamicObjectPriority); n++) {
          relative_part = 0;
          if (fixed_val[n] == 0) {
            flag_val = FDKreadBit(bs);
            if (flag_val) {
              if (wordsize[n] == OAM_BITS_AZI) {
                bits = num_bits;
              } else {
                bits = (UCHAR)fMin((int)num_bits, wordsize[n] + 1);
              }
              relative_part = FDKreadBits(bs, bits);
              /* every value is signed */
              relative_part = relative_part << (32 - bits);
              relative_part = relative_part >> (32 - bits);

              pos[n0 + n] += relative_part;
            }
            if (!hasUniformSpread && (n == SPREAD)) {
              flag_val = FDKreadBit(bs);
              if (flag_val) {
                bits = (UCHAR)fMin((int)num_bits, OAM_BITS_SPREAD_HEIGHT + 1);
                relative_part = FDKreadBits(bs, bits);
                relative_part = relative_part << (32 - bits);
                relative_part = relative_part >> (32 - bits);
                pos[SPREAD_HEIGHT + n0] += relative_part;
              }

              flag_val = FDKreadBit(bs);
              if (flag_val) {
                bits = (UCHAR)fMin((int)num_bits, OAM_BITS_SPREAD_DEPTH + 1);
                relative_part = FDKreadBits(bs, bits);
                relative_part = relative_part << (32 - bits);
                relative_part = relative_part >> (32 - bits);
                pos[SPREAD_DEPTH + n0] += relative_part;
              }
            }
          }
        } /* for (n = 0; n < (OAM_NUMBER_COMPONENTS + hasDynamicObjectPriority); n++) */
      }   /* if (flag_absolute) */
    }     /* if (has_object_data) */
    n0 += OAM_NUMBER_MAX_COMPONENTS;
  } /* for (obj = 0; obj < num_objects; obj++) */
}

static FIXP_DBL range(FIXP_DBL value, FIXP_DBL minval, FIXP_DBL maxval) {
  FIXP_DBL ret;
  ret = fMin(maxval, fMax(value, minval));
  return ret;
}

static void oam_concealment(HANDLE_GVBAPRENDERER hgVBAPRenderer) {
  int k, i;

  /* Fill current samples with last valid samples */
  /* If there were no valid samples yet then everything will be rendered to center with gain 1.0 */
  for (k = 0; k < hgVBAPRenderer->numOamFrames; k++) {
    hgVBAPRenderer->metadataPresent[k] = hgVBAPRenderer->metadataPresentValid[k];

    if (hgVBAPRenderer->metadataPresent[k]) {
      for (i = 0; i < hgVBAPRenderer->numObjects; i++) {
        hgVBAPRenderer->oamSamples[k][i].gain = hgVBAPRenderer->oamSamplesValid[k][i].gain;
        hgVBAPRenderer->oamSamples[k][i].sph.azi = hgVBAPRenderer->oamSamplesValid[k][i].sph.azi;
        hgVBAPRenderer->oamSamples[k][i].sph.ele = hgVBAPRenderer->oamSamplesValid[k][i].sph.ele;
        hgVBAPRenderer->oamSamples[k][i].sph.rad = hgVBAPRenderer->oamSamplesValid[k][i].sph.rad;
        hgVBAPRenderer->oamSamples[k][i].goa_bsObjectDistance =
            hgVBAPRenderer->oamSamplesValid[k][i].goa_bsObjectDistance;
        hgVBAPRenderer->oamSamples[k][i].spreadAngle =
            hgVBAPRenderer->oamSamplesValid[k][i].spreadAngle;
        hgVBAPRenderer->oamSamples[k][i].spreadDepth =
            hgVBAPRenderer->oamSamplesValid[k][i].spreadDepth;
        hgVBAPRenderer->oamSamples[k][i].spreadHeight =
            hgVBAPRenderer->oamSamplesValid[k][i].spreadHeight;
      }
    }
  }
}

static int object_metadata(HANDLE_FDK_BITSTREAM bs, UCHAR lowDelayMetadataCoding, UINT num_objects,
                           UCHAR hasDynamicObjectPriority, UCHAR hasUniformSpread, INT* pos,
                           UCHAR* fixed_val, OAM_SAMPLE* oamSample, INT renderMode) {
  UINT has_intracoded_object_metadata;
  UINT i;

  /* Table 70 */
  if (lowDelayMetadataCoding == 0) /* object_metadata_efficient */
  {
    if (FDKreadBit(bs)) /* has_differential_metadata */
    {
      return 1;
    }
  } else /* object_metadata_low_delay */
  {
    /* Table 75 */
    has_intracoded_object_metadata = FDKreadBit(bs);
    if (has_intracoded_object_metadata) /* has_intracoded_object_metadata */
    {
      get_iframe(bs, &pos[0], &fixed_val[0], num_objects, hasDynamicObjectPriority,
                 hasUniformSpread); /* intracoded_object_metadata_low_delay() */
    } else {
      get_dframe(bs, &pos[0], &fixed_val[0], num_objects, hasDynamicObjectPriority,
                 hasUniformSpread); /* dynamic_object_metadata */
    }
  }

  /**************************************/
  /* Post Processing of Object Metadata */
  /**************************************/
  for (i = 0; i < num_objects; i++) {
    int pi = i * OAM_NUMBER_MAX_COMPONENTS;

    /* Azimuth limit Range -180 ... 180 Degree */
    INT tempAZI = (INT)range((FIXP_DBL)pos[pi + AZIMUTH], (FIXP_DBL)-120,
                             (FIXP_DBL)120); /* 180 = 1.5 * 120 */
    /* revert encoders scaling */
    oamSample[i].sph.azi = FIXP_DBL(tempAZI * (INT)17895697); /* (2^31 - 1) / 120 */

    /* Elevation limit Range -90 ... 90 Degree */
    INT tempELE = (INT)range(pos[pi + ELEVATION], (FIXP_DBL)-30, (FIXP_DBL)30); /* 90 = 3.0 * 30 */
    /* revert encoders scaling */
    oamSample[i].sph.ele = FIXP_DBL(tempELE * (INT)35791394); /* (2^31 - 1) / 60 */

    if (renderMode == GVBAP_LEGACY) {
      oamSample[i].sph.rad = FL2FXCONST_DBL(1.0 / 16); /* set radius to 1.0 */
    }

    /* Gain revert encoders scaling */
    const int tab_index = fMin(64, fMax(-64, pos[pi + GAIN])) + 64;
    oamSample[i].gain = gain_dec_tab[tab_index];

    /* Spread limit Range 0 ... 180 Degree */
    INT tempSP =
        (INT)range((FIXP_DBL)pos[pi + SPREAD], (FIXP_DBL)0, (FIXP_DBL)120); /* 180 = 1.5 * 120 */
    /* revert encoders scaling */
    oamSample[i].spreadAngle = FIXP_DBL(tempSP * (INT)17895697); /* (2^31 - 1) / 120 */

    if (hasUniformSpread == 0) {
      FIXP_DBL spreadDepth;
      /* SpreadHeigh limit Range 0 ... 90 Degree */
      INT tempSH = (INT)range((FIXP_DBL)pos[pi + SPREAD_HEIGHT], (FIXP_DBL)0,
                              (FIXP_DBL)30); /* 90 = 3.0 * 30 */
      /* SpreadDepth limit Range 0 ... 15.5 */
      INT tempSD =
          (INT)range((FIXP_DBL)pos[pi + SPREAD_DEPTH], (FIXP_DBL)0, (FIXP_DBL)15); /* 0 ... 15.5 */
      spreadDepth = spread_depth_table[tempSD];
      spreadDepth = fMult(spreadDepth, FL2FXCONST_DBL(1.0 / 15.5));
      /* revert encoders scaling */
      oamSample[i].spreadHeight = FIXP_DBL(tempSH * (INT)35791394); /* (2^31 - 1) / (2*30) */
      oamSample[i].spreadDepth = spreadDepth;                       /* exponent = 4 */
    } else /* prevent uninitialized values */
    {
      oamSample[i].spreadHeight = (FIXP_DBL)0;
      oamSample[i].spreadDepth = (FIXP_DBL)0;
    }
  }

  return 0;
}

int objectMetadataFrame(HANDLE_GVBAPRENDERER hgVBAPRenderer, HANDLE_FDK_BITSTREAM bs,
                        int usacExtElementPayloadLength, UCHAR lowDelayMetadataCoding,
                        UINT num_objects, UCHAR hasDynamicObjectPriority, UCHAR hasUniformSpread) {
  INT cnt_read_bits;
  int left_bits, err = 0;
  UINT i;
  UINT numOamFrames;

  hgVBAPRenderer->oamDataValid = 0;

  numOamFrames = hgVBAPRenderer->numOamFrames;

  hgVBAPRenderer->flagOamFrameOk = 0;

  hgVBAPRenderer->hasUniformSpread = hasUniformSpread; /* store uniformSpread information */

  INT* pos = &(hgVBAPRenderer->OAM_parsed_data[0]);

  cnt_read_bits = FDKgetValidBits(bs);

  if (hgVBAPRenderer->oamFrameLength < hgVBAPRenderer->frameLength) {
    int presentFrames = 0;

    for (i = 0; i < numOamFrames; i++) {
      hgVBAPRenderer->metadataPresent[i] = FDKreadBit(bs);

      if (hgVBAPRenderer->metadataPresent[i]) {
        err = object_metadata(bs, lowDelayMetadataCoding, num_objects, hasDynamicObjectPriority,
                              hasUniformSpread, pos, hgVBAPRenderer->fixed_val,
                              hgVBAPRenderer->oamSamples[i], hgVBAPRenderer->renderMode);
        if (err) {
          return err;
        }
        presentFrames++;
      }
    }
    if (presentFrames == 0) {
      return 1;
    }
  } else {
    hgVBAPRenderer->metadataPresent[0] = 1;

    err = object_metadata(bs, lowDelayMetadataCoding, num_objects, hasDynamicObjectPriority,
                          hasUniformSpread, pos, hgVBAPRenderer->fixed_val,
                          hgVBAPRenderer->oamSamples[0], hgVBAPRenderer->renderMode);
    if (err) {
      return err;
    }
  }

  /* point bitstream buffer to right position */
  left_bits = FDKgetValidBits(bs);
  if (left_bits < 0) {
    return -1;
  }
  cnt_read_bits = cnt_read_bits - left_bits;
  left_bits = (usacExtElementPayloadLength << 3) - cnt_read_bits;
  if (left_bits < 0) {
    return -1;
  }
  FDKpushBiDirectional(bs, left_bits); /* set bitstream to right position */

  /* Valid object meta data available => save it for concealment */
  {
    int k, l;

    hgVBAPRenderer->oamDataValid = 1;

    /* Copy new valid object meta data to oamSamplesValid for concealment */
    for (k = 0; k < hgVBAPRenderer->numOamFrames; k++) {
      hgVBAPRenderer->metadataPresentValid[k] = hgVBAPRenderer->metadataPresent[k];

      if (hgVBAPRenderer->metadataPresentValid[k]) {
        for (l = 0; l < hgVBAPRenderer->numObjects; l++) {
          hgVBAPRenderer->oamSamplesValid[k][l].gain = hgVBAPRenderer->oamSamples[k][l].gain;
          hgVBAPRenderer->oamSamplesValid[k][l].sph.azi = hgVBAPRenderer->oamSamples[k][l].sph.azi;
          hgVBAPRenderer->oamSamplesValid[k][l].sph.ele = hgVBAPRenderer->oamSamples[k][l].sph.ele;
          hgVBAPRenderer->oamSamplesValid[k][l].sph.rad = hgVBAPRenderer->oamSamples[k][l].sph.rad;
          hgVBAPRenderer->oamSamplesValid[k][l].goa_bsObjectDistance =
              hgVBAPRenderer->oamSamples[k][l].goa_bsObjectDistance;
          hgVBAPRenderer->oamSamplesValid[k][l].spreadAngle =
              hgVBAPRenderer->oamSamples[k][l].spreadAngle;
          hgVBAPRenderer->oamSamplesValid[k][l].spreadDepth =
              hgVBAPRenderer->oamSamples[k][l].spreadDepth;
          hgVBAPRenderer->oamSamplesValid[k][l].spreadHeight =
              hgVBAPRenderer->oamSamples[k][l].spreadHeight;
        }
      }
    }
  }

  return err;
}

static FIXP_DBL decodeDistance(INT positionDistance, INT bsReferenceDistance) {
  FIXP_DBL distance = 0;

  if (positionDistance > 0) {
    INT e;
    distance = fMult(FL2FXCONST_DBL(0.0472188798661443 * 16),
                     (FIXP_DBL)(LONG)(positionDistance - bsReferenceDistance - 120)
                         << (31 - 9)); /* exp 5 */
    distance = f2Pow(distance, 5, &e);

    if ((e > 4) && (distance > ((FIXP_DBL)MAXVAL_DBL >> (e - 4)))) {
      distance = (FIXP_DBL)MAXVAL_DBL;
    } else {
      distance = scaleValue(distance, e - 4);
    }
  }

  return distance;
}

int prodMetadataFrameGroup(HANDLE_GVBAPRENDERER hgVBAPRenderer, HANDLE_FDK_BITSTREAM bs,
                           UINT numObjects, INT bsReferenceDistance, INT hasObjectDistance) {
  UINT o;
  int i;

  if (hasObjectDistance) {
    int hasIntraCodedData = FDKreadBit(bs);

    if (hasIntraCodedData) {
      if (numObjects > 1) {
        int fixedDistance = FDKreadBit(bs);

        if (fixedDistance) {
          INT defaultDistance = FDKreadBits(bs, 9);
          if (hgVBAPRenderer) {
            FIXP_DBL distance = decodeDistance(defaultDistance, bsReferenceDistance);
            for (i = 0; i < hgVBAPRenderer->numOamFrames; i++) {
              for (o = 0; o < numObjects; o++) {
                hgVBAPRenderer->oamSamples[i][o].sph.rad = distance;
                hgVBAPRenderer->oamSamples[i][o].goa_bsObjectDistance = defaultDistance;
              }
            }
          }
        } else {
          int commonDistance = FDKreadBit(bs);

          if (commonDistance) {
            INT defaultDistance = FDKreadBits(bs, 9);
            if (hgVBAPRenderer) {
              FIXP_DBL distance = decodeDistance(defaultDistance, bsReferenceDistance);
              for (i = 0; i < hgVBAPRenderer->numOamFrames; i++) {
                for (o = 0; o < numObjects; o++) {
                  hgVBAPRenderer->oamSamples[i][o].sph.rad = distance;
                  hgVBAPRenderer->oamSamples[i][o].goa_bsObjectDistance = defaultDistance;
                }
              }
            }
          } else {
            for (o = 0; o < numObjects; o++) {
              INT positionDistance = FDKreadBits(bs, 9);
              if (hgVBAPRenderer) {
                FIXP_DBL distance = decodeDistance(positionDistance, bsReferenceDistance);
                for (i = 0; i < hgVBAPRenderer->numOamFrames; i++) {
                  hgVBAPRenderer->oamSamples[i][o].sph.rad = distance;
                  hgVBAPRenderer->oamSamples[i][o].goa_bsObjectDistance = positionDistance;
                }
              }
            }
          }
        }
      } else {
        INT positionDistance = FDKreadBits(bs, 9);
        if (hgVBAPRenderer) {
          FIXP_DBL distance = decodeDistance(positionDistance, bsReferenceDistance);
          for (i = 0; i < hgVBAPRenderer->numOamFrames; i++) {
            hgVBAPRenderer->oamSamples[i][0].sph.rad = distance;
            hgVBAPRenderer->oamSamples[i][0].goa_bsObjectDistance = positionDistance;
          }
        }
      }
    } else {
      return 1;
    }
  } else {
    if (hgVBAPRenderer) {
      for (i = 0; i < hgVBAPRenderer->numOamFrames; i++) {
        for (o = 0; o < numObjects; o++)
          hgVBAPRenderer->oamSamples[i][o].sph.rad = FL2FXCONST_DBL(1.0 / 16);
      }
    }
  }

  return 0;
}

/* ------------------ SPREAD -------------------- */

#define GVBAP_SPREAD_OPEN_ANGLE_VSO (FL2FXCONST_DBL(30.0 / 180.0)) /* 30 degrees */
#define GVBAP_SPREAD_PARABLE_EXP 6

#define GVBAP_SPREAD_80DB (0.0001f)
#define GVBAP_SPREAD_PARABLE_SCALING_FACTOR \
  (-1.1682f) /* -4*(1-db2mag(-3)), comes from -3dB = -scalFact*(spd/2)^2+1 */

static const int ringIndices30_1[] = {10, 11, 0, 1, 2, 9, 3};
static const int ringIndices30_2[] = {8, 7, 6, 5, 4, 9, 3};

static FIXP_DBL gVBAPRenderer_Spread_internal_spread_mappingToH(FIXP_DBL spreadAngle) {
  FIXP_DBL spreadAngleMapped = fMultDiv2(FL2FXCONST_DBL(1.25 / 2.0), spreadAngle);
  spreadAngleMapped += fMultDiv2(FL2FXCONST_DBL(0.0069 / 2 * 180), fMult(spreadAngle, spreadAngle));
  spreadAngleMapped += GVBAP_SPREAD_OPEN_ANGLE_VSO >> 1;
  return spreadAngleMapped; /* exp 1 */
}

static FIXP_DBL gVBAPRenderer_Spread_internal_spread_Parable(
    FIXP_DBL spreadAngle /* exp 1 */) /* output scaled by 180^2 / 2^GVBAP_SPREAD_PARABLE_EXP
                                         compared to float implementation */
{
  FIXP_DBL tmp = fMult(spreadAngle, spreadAngle); /* exp 2 */
  INT e = 0;

  tmp = -fDivNormHighPrec(-FL2FXCONST_DBL(GVBAP_SPREAD_PARABLE_SCALING_FACTOR / 2), tmp,
                          &e); /* exp e-1 */
  return scaleValue(tmp, e - 1 - GVBAP_SPREAD_PARABLE_EXP);
}

static int gVBAPRenderer_Spread_internal_indexRingLimiter(FIXP_DBL spreadAngle /* exp 1 */) {
  return fMin(GVBAP_SPREAD_NUM_VSO_AZI >> 1, -(int)((-spreadAngle + (spreadAngle >> 2)) >> 27));
}

static void gVBAPRenderer_Spread_internal_calculateLayerGains(HANDLE_GVBAPRENDERER phgVBAPRenderer,
                                                              FIXP_DBL angleObject, int loopLimit,
                                                              FIXP_DBL parable, FIXP_DBL* gains) {
  /* internal parameters */
  int plumin, multiple, n, idx1, idx2, idx1Arg, idx2Arg;
  FIXP_DBL diffCLKDir, diffAntiCLKDir, tmp;

  /* sign of object angle */
  plumin = (angleObject < (FIXP_DBL)0) ? -1 : 1;

  /* intermediate spread parameters for indexRing */
  tmp = fAbs(angleObject);
  multiple = (int)((tmp + (-tmp >> 2)) >> 28);

  diffCLKDir = fMin(GVBAP_SPREAD_OPEN_ANGLE_VSO,
                    tmp - (FIXP_DBL)((int)GVBAP_SPREAD_OPEN_ANGLE_VSO * multiple));
  diffAntiCLKDir = GVBAP_SPREAD_OPEN_ANGLE_VSO - diffCLKDir;

  if (angleObject < (FIXP_DBL)0) multiple = -multiple;

  /* calculate gains for the given layer */
  for (n = 0; n < loopLimit; n++) {
    idx1Arg = multiple + GVBAP_SPREAD_NUM_VSO_AZI - plumin * (n - 0);
    idx2Arg = multiple + GVBAP_SPREAD_NUM_VSO_AZI + plumin * (n + 1);
    idx1 = phgVBAPRenderer->spread_indexRing[idx1Arg];
    idx2 = phgVBAPRenderer->spread_indexRing[idx2Arg];

    tmp = (diffCLKDir + (FIXP_DBL)((int)GVBAP_SPREAD_OPEN_ANGLE_VSO * n));
    tmp = fMult(tmp, tmp);
    if (tmp < (FIXP_DBL)0) tmp = (FIXP_DBL)MAXVAL_DBL;
    tmp = fMult(parable, tmp);
    tmp = fMax(FL2FXCONST_DBL(0), tmp + ((FIXP_DBL)MAXVAL_DBL >> GVBAP_SPREAD_PARABLE_EXP))
          << GVBAP_SPREAD_PARABLE_EXP;
    gains[idx1] = tmp;

    tmp = (diffAntiCLKDir + (FIXP_DBL)((int)GVBAP_SPREAD_OPEN_ANGLE_VSO * n));
    tmp = fMult(tmp, tmp);
    if (tmp < (FIXP_DBL)0) tmp = (FIXP_DBL)MAXVAL_DBL;
    tmp = fMult(parable, tmp);
    tmp = fMax(FL2FXCONST_DBL(0), tmp + ((FIXP_DBL)MAXVAL_DBL >> GVBAP_SPREAD_PARABLE_EXP))
          << GVBAP_SPREAD_PARABLE_EXP;
    gains[idx2] = tmp;
  }
}

static void gVBAPRenderer_Spread_internal_calculateVSOGains(
    HANDLE_GVBAPRENDERER phgVBAPRenderer, FIXP_DBL objAngleEle, FIXP_DBL* tmpGainAzi1,
    FIXP_DBL* tmpGainAzi2, FIXP_DBL* tmpGainEle1, FIXP_DBL* tmpGainEle2, FIXP_DBL parableAzi,
    FIXP_DBL parableEle) {
  /* internal parameters */
  int objNo = 0, na, ne, idx;
  FIXP_DBL eleNorm, diffEleNorm, normTerm, tmp, tmp2;
  INT e;

  /* calculate VSO spread gains */
  for (ne = 0; ne < GVBAP_SPREAD_NUM_VSO_ELE - 2; ne++) {
    for (na = 0; na < GVBAP_SPREAD_NUM_VSO_AZI; na++) {
      phgVBAPRenderer->spread_gainsSpread[objNo] =
          (fMult(tmpGainAzi1[na], tmpGainEle1[ne]) >> 1) +
          (fMult(tmpGainAzi2[na], tmpGainEle2[ne]) >> 1); /* exp 1 */
      objNo++;
    }
  }

  /* spread gains for VSO with elevation +/-90 degrees */
  /* Separate calculation as azi angle for VSO with elevation +/-90 degrees is the object azimuth
   * angle itself, but defined once -> diffazi = 0 or 180 */
  tmp = fMax(FL2FXCONST_DBL(0), parableAzi + ((FIXP_DBL)MAXVAL_DBL >> GVBAP_SPREAD_PARABLE_EXP))
        << GVBAP_SPREAD_PARABLE_EXP;

  idx = GVBAP_SPREAD_NUM_VSO_ELE - 2;
  phgVBAPRenderer->spread_gainsSpread[objNo] =
      (tmpGainEle1[idx] >> 1) + (fMult(tmp, tmpGainEle2[idx]) >> 1); /* exp 1 */
  objNo++;
  idx = GVBAP_SPREAD_NUM_VSO_ELE - 1;
  phgVBAPRenderer->spread_gainsSpread[objNo] =
      (tmpGainEle1[idx] >> 1) + (fMult(tmp, tmpGainEle2[idx]) >> 1); /* exp 1 */

  /* normalization due to extension of elevation from +/-90 degrees to +/-180 degrees */
  if (objAngleEle >= (FIXP_DBL)0) {
    eleNorm = (FIXP_DBL)MAXVAL_DBL - objAngleEle;
    diffEleNorm = eleNorm - objAngleEle;
  } else {
    eleNorm = (FIXP_DBL)MINVAL_DBL - objAngleEle;
    diffEleNorm = eleNorm - objAngleEle;
  }

  tmp2 = fMult(parableEle, fMult(diffEleNorm, diffEleNorm));
  tmp2 = fMax(FL2FXCONST_DBL(0), tmp2 + ((FIXP_DBL)MAXVAL_DBL >> GVBAP_SPREAD_PARABLE_EXP))
         << GVBAP_SPREAD_PARABLE_EXP;
  normTerm = ((FIXP_DBL)MAXVAL_DBL >> 1) + (fMult(tmp, tmp2) >> 1); /* exp 1 */
  e = 1;
  normTerm = invFixp(normTerm, &e);
  normTerm = scaleValue(normTerm, e - 1); /* exp 1 */
  for (objNo = 0; objNo < GVBAP_SPREAD_NUM_VSO; objNo++) {
    tmp = fMult(normTerm, phgVBAPRenderer->spread_gainsSpread[objNo]) << 1; /* exp 1 */
    if (tmp < (FIXP_DBL)0) tmp = (FIXP_DBL)MAXVAL_DBL;
    phgVBAPRenderer->spread_gainsSpread[objNo] = tmp; /* exp 1 */
  }
}

static void gVBAPRenderer_Spread_internal_normalizeVector(FIXP_DBL* vector, int vectorLength) {
  INT n, e;
  FIXP_DBL denom, nrg = 0, tmp;

  int shift = 32 - fNormz((FIXP_DBL)(vectorLength - 1));
  int headroom = getScalefactor(vector, vectorLength);

  for (n = 0; n < vectorLength; n++) {
    tmp = vector[n] << headroom;
    nrg += fMult(tmp, tmp) >> shift;
  }

  if (shift & 1) {
    nrg >>= 1;
    shift++;
  }

  e = shift - (headroom << 1);

  tmp = fMax((FIXP_DBL)(LONG)1,
             scaleValue(FL2FXCONST_DBL(GVBAP_SPREAD_80DB * GVBAP_SPREAD_80DB * (1 << 26)),
                        fMax(-31, -e - 26)));
  nrg = fMax(nrg, tmp);

  denom = invSqrtNorm2(nrg, &e);
  e -= (shift >> 1) - headroom;
  e -= 3;

  if (e >= 0) {
    for (n = 0; n < vectorLength; n++) {
      vector[n] = fMult(vector[n], denom) << e;
    }
  } else {
    e = -e;
    for (n = 0; n < vectorLength; n++) {
      vector[n] = fMult(vector[n], denom) >> e;
    }
  }
}

static void gVBAPRenderer_Spread_internal_calculateGains(HANDLE_GVBAPRENDERER phgVBAPRenderer,
                                                         FIXP_DBL objAngleAzi, FIXP_DBL objAngleEle,
                                                         FIXP_DBL* spread, FIXP_DBL* gainArray) {
  /* internal parameters */
  int objNo, idx, n, m, gNo, loopLimitAzi, loopLimitEle;
  FIXP_DBL spreadAngleAziMapped, spreadAngleEleMapped, parableAzi, parableEle, smoothGain;
  FIXP_DBL tmpGainAzi1[GVBAP_SPREAD_NUM_VSO_AZI] = {0}, tmpGainAzi2[GVBAP_SPREAD_NUM_VSO_AZI] = {0},
           tmpGainEle1[GVBAP_SPREAD_NUM_VSO_ELE] = {0}, tmpGainEle2[GVBAP_SPREAD_NUM_VSO_ELE] = {0},
           tmpGainEle[GVBAP_SPREAD_NUM_VSO_AZI] = {0};
  FIXP_DBL spreadAngleAzi = 0, spreadAngleEle = 0;
  int muteVoGH = 0;

  if (phgVBAPRenderer->hasUniformSpread) {
    spreadAngleAzi = spread[0];
    spreadAngleEle = spread[0] >> 1;
  } else {
    spreadAngleAzi = spread[0];
    spreadAngleEle = spread[1];
    if (spreadAngleEle < (FIXP_DBL)0) spreadAngleEle = (FIXP_DBL)MAXVAL_DBL;
  }

  /* smoothing gain for transition between spread=0 and spread>0 w/o jumps */
  FIXP_DBL smoothGain_temp = fMult(
      FL2FXCONST_DBL(0.89), fMin((FIXP_DBL)(MAXVAL_DBL / 6), fMax(spreadAngleAzi, spreadAngleEle)));
  smoothGain = ((smoothGain_temp << 2) + (smoothGain_temp << 1)); /* "*6" */
  smoothGain_temp = fMult(FL2FXCONST_DBL(0.11),
                          fMin((FIXP_DBL)(MAXVAL_DBL / 6), fMin(spreadAngleAzi, spreadAngleEle)));
  smoothGain += ((smoothGain_temp << 2) + (smoothGain_temp << 1)); /* "*6" */

  /* mapping to MPEGH effect strength behavior - perceptually tuned */
  spreadAngleAziMapped =
      gVBAPRenderer_Spread_internal_spread_mappingToH(spreadAngleAzi); /* exp 1 */
  spreadAngleEleMapped =
      gVBAPRenderer_Spread_internal_spread_mappingToH(spreadAngleEle); /* exp 1 */

  /* preprocessing steps for non-uniform spread */
  if (spreadAngleAziMapped > spreadAngleEleMapped) {
    /* ensure minimum vertical spread */
    spreadAngleEleMapped = fMax(spreadAngleEleMapped, GVBAP_SPREAD_OPEN_ANGLE_VSO >> 1); /* exp 1 */
  } else if (spreadAngleAziMapped < spreadAngleEleMapped) {
    /* muting spread on ghost loudspeakers at +/-90 degrees elevation in case of vertical spread */
    muteVoGH = 1;

    /* ensure minimum horizontal spread */
    spreadAngleAziMapped = fMax(spreadAngleAziMapped, GVBAP_SPREAD_OPEN_ANGLE_VSO >> 1); /* exp 1 */
  }

  /* calculate parable values for spread gain */
  parableAzi = gVBAPRenderer_Spread_internal_spread_Parable(spreadAngleAziMapped);
  parableEle = gVBAPRenderer_Spread_internal_spread_Parable(spreadAngleEleMapped);

  /* calculate minimum number of loop itarations required for calculating the largest angle
   * difference that does not lead to zero */
  loopLimitAzi = gVBAPRenderer_Spread_internal_indexRingLimiter(spreadAngleAziMapped);
  loopLimitEle = gVBAPRenderer_Spread_internal_indexRingLimiter(
      spreadAngleEleMapped); /* also here spread_numVSO_azi is required */

  /* calculate azimuth layer gains */
  gVBAPRenderer_Spread_internal_calculateLayerGains(phgVBAPRenderer, objAngleAzi, loopLimitAzi,
                                                    parableAzi, tmpGainAzi1);
  for (n = 0; n < GVBAP_SPREAD_NUM_VSO_AZI / 2; n++) {
    idx = GVBAP_SPREAD_NUM_VSO_AZI / 2 + n;
    tmpGainAzi2[n] = tmpGainAzi1[idx];
    tmpGainAzi2[idx] = tmpGainAzi1[n];
  }

  /* calculate elevation layer gains */
  gVBAPRenderer_Spread_internal_calculateLayerGains(phgVBAPRenderer, objAngleEle, loopLimitEle,
                                                    parableEle, tmpGainEle);
  for (m = 0; m < GVBAP_SPREAD_NUM_VSO_ELE; m++) {
    tmpGainEle1[m] = tmpGainEle[ringIndices30_1[m]];
    tmpGainEle2[m] = tmpGainEle[ringIndices30_2[m]];
  }

  /* calculate VSO gains */
  gVBAPRenderer_Spread_internal_calculateVSOGains(phgVBAPRenderer, objAngleEle, tmpGainAzi1,
                                                  tmpGainAzi2, tmpGainEle1, tmpGainEle2, parableAzi,
                                                  parableEle);

  /* applying VSO spread gains to VSO loudspeaker gains (panning gains of VSO) */
  for (gNo = 0; gNo < phgVBAPRenderer->spread_numInvolvedLS; gNo++) {
    FIXP_DBL tmp = gainArray[gNo] >> 2; /* exp 5 */

    if (!muteVoGH || ((gNo != phgVBAPRenderer->ghostVoiceOfGodSpeakerIndex) &&
                      (gNo != phgVBAPRenderer->ghostVoiceOfHellSpeakerIndex))) {
      for (objNo = 0; objNo < GVBAP_SPREAD_NUM_VSO; objNo++) {
        tmp += fMult(fMult(phgVBAPRenderer->spread_gainsVSO[objNo][gNo],
                           phgVBAPRenderer->spread_gainsSpread[objNo]),
                     smoothGain) >>
               4; /* exp 5 */
      }
    }

    gainArray[gNo] = tmp; /* exp 5 */
  }

  /* gain normalization */
  gVBAPRenderer_Spread_internal_normalizeVector(gainArray, phgVBAPRenderer->spread_numInvolvedLS);
}

static void gVBAPRenderer_Spread_renderSpread(HANDLE_GVBAPRENDERER phgVBAPRenderer,
                                              FIXP_DBL* outputGainArray, OAM_SAMPLE oamSample) {
  FIXP_DBL spread[3] = {oamSample.spreadAngle, oamSample.spreadHeight, oamSample.spreadDepth};
  FIXP_DBL gain = oamSample.gain;

  oamSample.spreadAngle = oamSample.spreadHeight = oamSample.spreadDepth = (FIXP_DBL)0;
  oamSample.gain = FL2FXCONST_DBL(1.0 / 8);
  FDKmemset(outputGainArray, 0, phgVBAPRenderer->numChannels * sizeof(FIXP_DBL));

  /* calculate VBAP gains (assuming spread = 0 for implemented MDAP spread, no downmixing of ghost
   * speakers, gain factor set to 1.0 because gain factor will be applied after spread gains have
   * also been factored in) */
  calculateVbap(phgVBAPRenderer, oamSample, phgVBAPRenderer->spread_gainArray, 1);

  /* calculate spread gains */
  gVBAPRenderer_Spread_internal_calculateGains(phgVBAPRenderer, oamSample.sph.azi,
                                               oamSample.sph.ele, spread,
                                               phgVBAPRenderer->spread_gainArray);

  /* downmix ghost loudspeakers to real loudspeaker array */
  for (int row = 0; row < phgVBAPRenderer->downmixMatrixNumRows; row++) {
    for (int col = 0; col < phgVBAPRenderer->downmixMatrixNumCols; col++) {
      outputGainArray[row] +=
          fMult(phgVBAPRenderer->spread_gainArray[col], phgVBAPRenderer->downmixMatrix[row][col]) >>
          5;
    }
    FDK_ASSERT(outputGainArray[row] >= (FIXP_DBL)0);
  }

  /* gain normalization */
  gVBAPRenderer_Spread_internal_normalizeVector(outputGainArray, phgVBAPRenderer->numChannels);

  /* Apply gain factor */
  for (int i = 0; i < phgVBAPRenderer->numChannels; i++) {
    outputGainArray[i] = fMult(outputGainArray[i], gain) << 3;
  }
}

static FIXP_DBL gVBAPRenderer_Spread_distanceToSpreadIndividual(FIXP_DBL d, FIXP_DBL sIn) {
  /* sNorm = (((float)atan(0.1f / d)) * 0.31830988f * 2.0f - 0.0636f) * 1.0679f; */
  /* sTmp = 0.5f * ((1.0f - d) + sNorm); */
  static const FIXP_SGL table[257] = {
      FL2FXCONST_SGL(0.999991), FL2FXCONST_SGL(0.984766), FL2FXCONST_SGL(0.969582),
      FL2FXCONST_SGL(0.954478), FL2FXCONST_SGL(0.939491), FL2FXCONST_SGL(0.924659),
      FL2FXCONST_SGL(0.910015), FL2FXCONST_SGL(0.895589), FL2FXCONST_SGL(0.881408),
      FL2FXCONST_SGL(0.867496), FL2FXCONST_SGL(0.853873), FL2FXCONST_SGL(0.840554),
      FL2FXCONST_SGL(0.827553), FL2FXCONST_SGL(0.814878), FL2FXCONST_SGL(0.802536),
      FL2FXCONST_SGL(0.790529), FL2FXCONST_SGL(0.778860), FL2FXCONST_SGL(0.767526),
      FL2FXCONST_SGL(0.756523), FL2FXCONST_SGL(0.745846), FL2FXCONST_SGL(0.735490),
      FL2FXCONST_SGL(0.725446), FL2FXCONST_SGL(0.715707), FL2FXCONST_SGL(0.706262),
      FL2FXCONST_SGL(0.697102), FL2FXCONST_SGL(0.688218), FL2FXCONST_SGL(0.679600),
      FL2FXCONST_SGL(0.671236), FL2FXCONST_SGL(0.663118), FL2FXCONST_SGL(0.655235),
      FL2FXCONST_SGL(0.647578), FL2FXCONST_SGL(0.640136), FL2FXCONST_SGL(0.632901),
      FL2FXCONST_SGL(0.625863), FL2FXCONST_SGL(0.619014), FL2FXCONST_SGL(0.612346),
      FL2FXCONST_SGL(0.605850), FL2FXCONST_SGL(0.599518), FL2FXCONST_SGL(0.593344),
      FL2FXCONST_SGL(0.587319), FL2FXCONST_SGL(0.581438), FL2FXCONST_SGL(0.575694),
      FL2FXCONST_SGL(0.570081), FL2FXCONST_SGL(0.564593), FL2FXCONST_SGL(0.559223),
      FL2FXCONST_SGL(0.553968), FL2FXCONST_SGL(0.548822), FL2FXCONST_SGL(0.543781),
      FL2FXCONST_SGL(0.538839), FL2FXCONST_SGL(0.533992), FL2FXCONST_SGL(0.529237),
      FL2FXCONST_SGL(0.524569), FL2FXCONST_SGL(0.519985), FL2FXCONST_SGL(0.515481),
      FL2FXCONST_SGL(0.511053), FL2FXCONST_SGL(0.506700), FL2FXCONST_SGL(0.502417),
      FL2FXCONST_SGL(0.498202), FL2FXCONST_SGL(0.494053), FL2FXCONST_SGL(0.489965),
      FL2FXCONST_SGL(0.485938), FL2FXCONST_SGL(0.481968), FL2FXCONST_SGL(0.478054),
      FL2FXCONST_SGL(0.474193), FL2FXCONST_SGL(0.470384), FL2FXCONST_SGL(0.466623),
      FL2FXCONST_SGL(0.462911), FL2FXCONST_SGL(0.459244), FL2FXCONST_SGL(0.455621),
      FL2FXCONST_SGL(0.452040), FL2FXCONST_SGL(0.448501), FL2FXCONST_SGL(0.445001),
      FL2FXCONST_SGL(0.441539), FL2FXCONST_SGL(0.438114), FL2FXCONST_SGL(0.434724),
      FL2FXCONST_SGL(0.431368), FL2FXCONST_SGL(0.428046), FL2FXCONST_SGL(0.424756),
      FL2FXCONST_SGL(0.421496), FL2FXCONST_SGL(0.418267), FL2FXCONST_SGL(0.415066),
      FL2FXCONST_SGL(0.411893), FL2FXCONST_SGL(0.408748), FL2FXCONST_SGL(0.405628),
      FL2FXCONST_SGL(0.402534), FL2FXCONST_SGL(0.399465), FL2FXCONST_SGL(0.396419),
      FL2FXCONST_SGL(0.393397), FL2FXCONST_SGL(0.390397), FL2FXCONST_SGL(0.387418),
      FL2FXCONST_SGL(0.384461), FL2FXCONST_SGL(0.381524), FL2FXCONST_SGL(0.378607),
      FL2FXCONST_SGL(0.375709), FL2FXCONST_SGL(0.372830), FL2FXCONST_SGL(0.369969),
      FL2FXCONST_SGL(0.367126), FL2FXCONST_SGL(0.364299), FL2FXCONST_SGL(0.361490),
      FL2FXCONST_SGL(0.358697), FL2FXCONST_SGL(0.355919), FL2FXCONST_SGL(0.353157),
      FL2FXCONST_SGL(0.350410), FL2FXCONST_SGL(0.347677), FL2FXCONST_SGL(0.344958),
      FL2FXCONST_SGL(0.342253), FL2FXCONST_SGL(0.339562), FL2FXCONST_SGL(0.336883),
      FL2FXCONST_SGL(0.334218), FL2FXCONST_SGL(0.331564), FL2FXCONST_SGL(0.328923),
      FL2FXCONST_SGL(0.326293), FL2FXCONST_SGL(0.323675), FL2FXCONST_SGL(0.321068),
      FL2FXCONST_SGL(0.318473), FL2FXCONST_SGL(0.315887), FL2FXCONST_SGL(0.313312),
      FL2FXCONST_SGL(0.310748), FL2FXCONST_SGL(0.308193), FL2FXCONST_SGL(0.305647),
      FL2FXCONST_SGL(0.303112), FL2FXCONST_SGL(0.300585), FL2FXCONST_SGL(0.298068),
      FL2FXCONST_SGL(0.295559), FL2FXCONST_SGL(0.293059), FL2FXCONST_SGL(0.290567),
      FL2FXCONST_SGL(0.288083), FL2FXCONST_SGL(0.285608), FL2FXCONST_SGL(0.283140),
      FL2FXCONST_SGL(0.280680), FL2FXCONST_SGL(0.278228), FL2FXCONST_SGL(0.275782),
      FL2FXCONST_SGL(0.273344), FL2FXCONST_SGL(0.270913), FL2FXCONST_SGL(0.268489),
      FL2FXCONST_SGL(0.266072), FL2FXCONST_SGL(0.263661), FL2FXCONST_SGL(0.261257),
      FL2FXCONST_SGL(0.258859), FL2FXCONST_SGL(0.256467), FL2FXCONST_SGL(0.254081),
      FL2FXCONST_SGL(0.251702), FL2FXCONST_SGL(0.249328), FL2FXCONST_SGL(0.246959),
      FL2FXCONST_SGL(0.244597), FL2FXCONST_SGL(0.242240), FL2FXCONST_SGL(0.239888),
      FL2FXCONST_SGL(0.237541), FL2FXCONST_SGL(0.235200), FL2FXCONST_SGL(0.232863),
      FL2FXCONST_SGL(0.230532), FL2FXCONST_SGL(0.228205), FL2FXCONST_SGL(0.225884),
      FL2FXCONST_SGL(0.223567), FL2FXCONST_SGL(0.221254), FL2FXCONST_SGL(0.218946),
      FL2FXCONST_SGL(0.216643), FL2FXCONST_SGL(0.214344), FL2FXCONST_SGL(0.212049),
      FL2FXCONST_SGL(0.209758), FL2FXCONST_SGL(0.207471), FL2FXCONST_SGL(0.205189),
      FL2FXCONST_SGL(0.202910), FL2FXCONST_SGL(0.200636), FL2FXCONST_SGL(0.198365),
      FL2FXCONST_SGL(0.196098), FL2FXCONST_SGL(0.193834), FL2FXCONST_SGL(0.191574),
      FL2FXCONST_SGL(0.189318), FL2FXCONST_SGL(0.187065), FL2FXCONST_SGL(0.184816),
      FL2FXCONST_SGL(0.182570), FL2FXCONST_SGL(0.180328), FL2FXCONST_SGL(0.178089),
      FL2FXCONST_SGL(0.175852), FL2FXCONST_SGL(0.173620), FL2FXCONST_SGL(0.171390),
      FL2FXCONST_SGL(0.169163), FL2FXCONST_SGL(0.166939), FL2FXCONST_SGL(0.164719),
      FL2FXCONST_SGL(0.162501), FL2FXCONST_SGL(0.160286), FL2FXCONST_SGL(0.158074),
      FL2FXCONST_SGL(0.155864), FL2FXCONST_SGL(0.153658), FL2FXCONST_SGL(0.151454),
      FL2FXCONST_SGL(0.149253), FL2FXCONST_SGL(0.147054), FL2FXCONST_SGL(0.144858),
      FL2FXCONST_SGL(0.142664), FL2FXCONST_SGL(0.140473), FL2FXCONST_SGL(0.138284),
      FL2FXCONST_SGL(0.136098), FL2FXCONST_SGL(0.133914), FL2FXCONST_SGL(0.131733),
      FL2FXCONST_SGL(0.129553), FL2FXCONST_SGL(0.127376), FL2FXCONST_SGL(0.125202),
      FL2FXCONST_SGL(0.123029), FL2FXCONST_SGL(0.120859), FL2FXCONST_SGL(0.118691),
      FL2FXCONST_SGL(0.116525), FL2FXCONST_SGL(0.114360), FL2FXCONST_SGL(0.112198),
      FL2FXCONST_SGL(0.110038), FL2FXCONST_SGL(0.107880), FL2FXCONST_SGL(0.105724),
      FL2FXCONST_SGL(0.103570), FL2FXCONST_SGL(0.101418), FL2FXCONST_SGL(0.099268),
      FL2FXCONST_SGL(0.097119), FL2FXCONST_SGL(0.094973), FL2FXCONST_SGL(0.092828),
      FL2FXCONST_SGL(0.090685), FL2FXCONST_SGL(0.088543), FL2FXCONST_SGL(0.086404),
      FL2FXCONST_SGL(0.084266), FL2FXCONST_SGL(0.082130), FL2FXCONST_SGL(0.079995),
      FL2FXCONST_SGL(0.077862), FL2FXCONST_SGL(0.075731), FL2FXCONST_SGL(0.073601),
      FL2FXCONST_SGL(0.071473), FL2FXCONST_SGL(0.069346), FL2FXCONST_SGL(0.067221),
      FL2FXCONST_SGL(0.065098), FL2FXCONST_SGL(0.062976), FL2FXCONST_SGL(0.060855),
      FL2FXCONST_SGL(0.058736), FL2FXCONST_SGL(0.056618), FL2FXCONST_SGL(0.054502),
      FL2FXCONST_SGL(0.052387), FL2FXCONST_SGL(0.050273), FL2FXCONST_SGL(0.048161),
      FL2FXCONST_SGL(0.046050), FL2FXCONST_SGL(0.043941), FL2FXCONST_SGL(0.041833),
      FL2FXCONST_SGL(0.039726), FL2FXCONST_SGL(0.037620), FL2FXCONST_SGL(0.035516),
      FL2FXCONST_SGL(0.033413), FL2FXCONST_SGL(0.031311), FL2FXCONST_SGL(0.029210),
      FL2FXCONST_SGL(0.027111), FL2FXCONST_SGL(0.025012), FL2FXCONST_SGL(0.022915),
      FL2FXCONST_SGL(0.020819), FL2FXCONST_SGL(0.018724), FL2FXCONST_SGL(0.016631),
      FL2FXCONST_SGL(0.014538), FL2FXCONST_SGL(0.012447), FL2FXCONST_SGL(0.010356),
      FL2FXCONST_SGL(0.008267), FL2FXCONST_SGL(0.006179), FL2FXCONST_SGL(0.004092),
      FL2FXCONST_SGL(0.002006), FL2FXCONST_SGL(-0.000080)};

  FIXP_DBL sTmp, f;
  int i;

  /* limit(d, 0.0001f, 1.0f); */
  d = fMax(d, FL2FXCONST_DBL(0.0001));

  /* table lookup with linear interpolation */
  i = (LONG)d >> (DFRACT_BITS - 1 - 8);
  f = (d & (MAXVAL_DBL >> 8)) << 8;

  sTmp = FX_SGL2FX_DBL(table[i]) + fMult(f, (FIXP_SGL)(table[i + 1] - table[i]));

  /* limit(sTmp, 0.0f, 1.0f); */
  if (sTmp < FL2FXCONST_DBL(0.01f)) {
    sTmp = (FIXP_DBL)0;
  }

  /* return (1.0f - sIn) * sTmp + sIn; */
  return fMult((FIXP_DBL)MAXVAL_DBL - sIn, sTmp) + sIn;
}

static void gVBAPRenderer_Spread_distanceToSpread(OAM_SAMPLE* oamSample, int hasUniformSpread) {
  FIXP_DBL length = MAXVAL_DBL;
  if (oamSample->sph.rad < FL2FXCONST_DBL(1.0 / 16)) length = oamSample->sph.rad << 4;

  if (length < FL2FXCONST_DBL(0.999)) {
    if (hasUniformSpread) {
      oamSample->spreadAngle =
          gVBAPRenderer_Spread_distanceToSpreadIndividual(length, oamSample->spreadAngle);
    } else {
      FIXP_DBL tmp;

      oamSample->spreadAngle =
          gVBAPRenderer_Spread_distanceToSpreadIndividual(length, oamSample->spreadAngle);

      tmp = fMin((FIXP_DBL)MAXVAL_DBL >> 1, oamSample->spreadHeight) << 1;
      oamSample->spreadHeight = gVBAPRenderer_Spread_distanceToSpreadIndividual(length, tmp) >> 1;

      tmp = fMin((FIXP_DBL)MAXVAL_DBL >> 4, oamSample->spreadDepth) << 4;
      oamSample->spreadDepth = gVBAPRenderer_Spread_distanceToSpreadIndividual(length, tmp) >> 4;
    }
  }
}

static void gVBAPRenderer_Spread_initVSOPositions(FIXP_DBL* aziVSOAngles, FIXP_DBL* eleVSOAngles) {
  /* interal parameters */
  int ne, na;

  /* define azimuth angles */
  aziVSOAngles[0] = (FIXP_DBL)0;
  for (na = 1; na < GVBAP_SPREAD_NUM_VSO_AZI; na++) {
    aziVSOAngles[na] =
        (FIXP_DBL)(LONG)((INT64)aziVSOAngles[na - 1] + (INT64)GVBAP_SPREAD_OPEN_ANGLE_VSO);
  }

  /* define elevation angles */
  eleVSOAngles[0] = (FIXP_DBL)MINVAL_DBL >> 1; /* -90 degrees */
  for (ne = 1; ne < GVBAP_SPREAD_NUM_VSO_ELE; ne++) {
    eleVSOAngles[ne] = eleVSOAngles[ne - 1] + GVBAP_SPREAD_OPEN_ANGLE_VSO;
  }
}

/* ------ END OF SPREAD ------*/

int gVBAPRenderer_Open(HANDLE_GVBAPRENDERER* phgVBAPRenderer, int numObjects, int frameLength,
                       int oamFrameLength, CICP2GEOMETRY_CHANNEL_GEOMETRY* outGeometryInfo,
                       int outChannels, int outCICPIndex, int hasUniformSpread, int renderMode) {
  HANDLE_GVBAPRENDERER tmp;
  int i;
  int cnt;
  int numLFE = 0;
  int numGhosts = 0;
  int numOamFrames;
  vertexList vList;
  vertexList* vL = &vList;
  triangleList tList;
  triangleList* tL = &tList;
  /* Save azimuth an elevation temporarily for vertex, triangle-List and downmixMatrix generation */
  /* In this way the same order as speakerList is guaranteed for the vertex list and the saved
   * mapping will fit to it */
  FIXP_DBL azimuth[GVBAPRENDERER_MAX_CHANNEL_OUT];
  FIXP_DBL elevation[GVBAPRENDERER_MAX_CHANNEL_OUT];

  /* some value range checks */
  if (numObjects > GVBAPRENDERER_MAX_OBJECTS) {
    /* too many objects */
    return -1;
  }
  if (outChannels > GVBAPRENDERER_MAX_CHANNEL_OUT) {
    /* too many speakers */
    return -1;
  }
  if (frameLength > GVBAPRENDERER_MAX_FRAMELENGTH) {
    return -1;
  }

  tmp = (HANDLE_GVBAPRENDERER)FDKcalloc(1, sizeof(GVBAPRENDERER));
  if (!tmp) {
    return -2; /* could not allocate memory */
  }

  numOamFrames = frameLength / oamFrameLength;

  *phgVBAPRenderer = tmp;

  /* allocate memory for speakerlist and fill it with values and save the mapping order */
  (*phgVBAPRenderer)->speakerSetup.speakerList = (SPEAKER*)FDKmalloc(outChannels * sizeof(SPEAKER));
  (*phgVBAPRenderer)->speakerSetup.mapping = (int*)FDKmalloc(outChannels * sizeof(int));

  if ((*phgVBAPRenderer)->speakerSetup.speakerList == NULL ||
      (*phgVBAPRenderer)->speakerSetup.mapping == NULL) {
    return -2; /* could not allocate memory */
  }

  cnt = 0;
  (*phgVBAPRenderer)->speakerSetup.hasHeightSpeakers = false;
  (*phgVBAPRenderer)->speakerSetup.coversUpperHalfSphere = false;
  (*phgVBAPRenderer)->speakerSetup.coversLowerHalfSphere = false;
  for (i = 0; i < outChannels; i++) {
    if (outGeometryInfo[i].LFE <= 0) /* first copy all non LFE channels */
    {
      (*phgVBAPRenderer)->speakerSetup.speakerList[cnt].sph.azi = FIXP_DBL(
          (INT)(outGeometryInfo[i].Az) *
          (INT)11930464); /* real value is 11930464.71 -> round down will prevent overflow */
      (*phgVBAPRenderer)->speakerSetup.speakerList[cnt].sph.ele =
          FIXP_DBL((INT)(outGeometryInfo[i].El) * (INT)11930464); /* max error is 0.3*180*2^-31 */
      (*phgVBAPRenderer)->speakerSetup.speakerList[cnt].sph.rad = FL2FXCONST_DBL(1.0);
      (*phgVBAPRenderer)->speakerSetup.speakerList[cnt].isLFE = false;
      (*phgVBAPRenderer)->speakerSetup.speakerList[cnt].isGhost = false;
      azimuth[cnt] =
          (*phgVBAPRenderer)
              ->speakerSetup.speakerList[cnt]
              .sph.azi; /* Copy non LFE channels for creating triangles and downmix matrix */
      elevation[cnt] = (*phgVBAPRenderer)->speakerSetup.speakerList[cnt].sph.ele;

      /* get some speaker setup covering information */
      if (outGeometryInfo[i].El > 21)
        (*phgVBAPRenderer)->speakerSetup.hasHeightSpeakers = true;
      else if (outGeometryInfo[i].El >= 5)
        (*phgVBAPRenderer)->speakerSetup.coversUpperHalfSphere = true;
      else if (outGeometryInfo[i].El <= -5)
        (*phgVBAPRenderer)->speakerSetup.coversLowerHalfSphere = true;

      (*phgVBAPRenderer)->speakerSetup.mapping[cnt] = i;
      cnt++;
    }
  }
  numLFE = outChannels - cnt;
  for (i = 0; i < outChannels; i++) {
    if (outGeometryInfo[i].LFE > 0) /* then copy all LFE channels */
    {
      (*phgVBAPRenderer)->speakerSetup.speakerList[cnt].sph.azi = FIXP_DBL(
          (INT)(outGeometryInfo[i].Az *
                (INT)11930464)); /* real value is 11930464.71 -> round down will prevent overflow */
      (*phgVBAPRenderer)->speakerSetup.speakerList[cnt].sph.ele =
          FIXP_DBL((INT)(outGeometryInfo[i].El * (INT)11930464)); /* max error is 0.3*180*2^-31 */
      (*phgVBAPRenderer)->speakerSetup.speakerList[cnt].sph.rad = FL2FXCONST_DBL(1.0);
      (*phgVBAPRenderer)->speakerSetup.speakerList[cnt].isLFE = true;
      (*phgVBAPRenderer)->speakerSetup.speakerList[cnt].isGhost = true;
      (*phgVBAPRenderer)->speakerSetup.mapping[cnt] = i;
      cnt++;
      FDK_ASSERT(cnt <= outChannels); /* don't use more than allocated memory! */
    }
  }
  (*phgVBAPRenderer)->speakerSetup.speakerListSize = cnt;

  /* Generate vertexList and add Ghost speakers if necessary */
  // vL = newVertexList(outChannels - numLFE + 6);    /* Add 6 because a maximum of 6 ghost speakers
  // could be added */
  resetVertexList(vL);

  qh_gen_VertexList(outChannels - numLFE, &azimuth[0], &elevation[0], vL);

  // tL = newTriangleList((2*(outChannels - numLFE))+5);  /* The length of triangle List is a
  // estimated value */
  resetTriangleList(tL);

  /* Generate triangle List for speaker triangles and a downmixMatrix if Ghostspeaker where added */
  if (qh_sphere_triangulation((renderMode == GVBAP_ENHANCED) ? outCICPIndex : 0, vL, tL,
                              &(*phgVBAPRenderer)->downmixMatrix,
                              &(*phgVBAPRenderer)->downmixMatrixNumRows,
                              &(*phgVBAPRenderer)->downmixMatrixNumCols) != 0) {
    return -2; /* could not allocate memory */
  }

  if (vL->size ==
      (outChannels -
       numLFE)) /* if no downmixMatrix exists then free memory and set pointer to NULL */
  {
    if ((*phgVBAPRenderer)->downmixMatrix != NULL) {
      fdkFreeMatrix2D((void**)(*phgVBAPRenderer)->downmixMatrix);
      (*phgVBAPRenderer)->downmixMatrix = NULL;
    }
  }

  /* allocate memory for speakerTriplet and fill it with values */
  if (((*phgVBAPRenderer)->speakerSetup.speakerTriplet =
           (SPEAKERTRIPLET*)FDKmalloc(tL->size * sizeof(SPEAKERTRIPLET))) == NULL) {
    return -2; /* could not allocate memory */
  }

  /* copy triangle list and fill matrices */
  for (i = 0; i < tL->size; i++) {
    (*phgVBAPRenderer)->speakerSetup.speakerTriplet[i].triangle[0] = tL->element[i].index[0];
    (*phgVBAPRenderer)->speakerSetup.speakerTriplet[i].triangle[1] = tL->element[i].index[1];
    (*phgVBAPRenderer)->speakerSetup.speakerTriplet[i].triangle[2] = tL->element[i].index[2];

    (*phgVBAPRenderer)->speakerSetup.speakerTriplet[i].matrix[0] =
        vL->element[tL->element[i].index[0]].xyz;
    (*phgVBAPRenderer)->speakerSetup.speakerTriplet[i].matrix[1] =
        vL->element[tL->element[i].index[1]].xyz;
    (*phgVBAPRenderer)->speakerSetup.speakerTriplet[i].matrix[2] =
        vL->element[tL->element[i].index[2]].xyz;
  }
  (*phgVBAPRenderer)->speakerSetup.speakerTripletSize = tL->size;

  numGhosts = vL->size - (outChannels - numLFE); /* Number of Ghostspeakers */

  /* Calculate inverse matrices */
  generateInverseMatrices(*phgVBAPRenderer);

  /* Set parameters */
  (*phgVBAPRenderer)->numLFE = numLFE;           /* Number of LFE speakers */
  (*phgVBAPRenderer)->numChannels = outChannels; /* Number of real speakers including LFE */
  (*phgVBAPRenderer)->numObjects =
      numObjects; /* Number of objects which will be rendered to the given loudspeaker setup */
  (*phgVBAPRenderer)->frameLength = frameLength;       /* Number of samples for one frame */
  (*phgVBAPRenderer)->oamFrameLength = oamFrameLength; /* Number of samples for one OAM frame */
  (*phgVBAPRenderer)->numOamFrames = numOamFrames;     /* number of OAM frames per core frame*/
  (*phgVBAPRenderer)->numGhosts = numGhosts;           /* Number of ghost speakers */
  (*phgVBAPRenderer)->startGainsFilled =
      0; /* indicates that no endGains copied to startGains yet */

  (*phgVBAPRenderer)->gainCacheLength = outChannels + numGhosts - numLFE;

  /* Alocate memory for gain cache. Length is number of real speakers including LFE + number of
   * ghosts */
  (*phgVBAPRenderer)->gainCache =
      (FIXP_DBL*)FDKmalloc((outChannels + numGhosts - numLFE) * sizeof(FIXP_DBL));

  /* Alocate memory for start and end gains. Length is number of real speakers including LFE */
  (*phgVBAPRenderer)->startGains = (FIXP_DBL**)fdkCallocMatrix2D(
      numObjects, (*phgVBAPRenderer)->numChannels,
      sizeof(FIXP_DBL)); /* Dim: objects x (number of output channels) */
  (*phgVBAPRenderer)->endGains = (FIXP_DBL**)fdkCallocMatrix2D(
      numObjects, (*phgVBAPRenderer)->numChannels,
      sizeof(FIXP_DBL)); /* Dim: objects x (number of output channels) */

  (*phgVBAPRenderer)->stepState = (FIXP_DBL**)fdkCallocMatrix2D(
      numObjects, (*phgVBAPRenderer)->numChannels,
      sizeof(FIXP_DBL)); /* Dim: objects x (number of output channels) */
  (*phgVBAPRenderer)->scaleState = (FIXP_DBL**)fdkCallocMatrix2D(
      numObjects, (*phgVBAPRenderer)->numChannels,
      sizeof(FIXP_DBL)); /* Dim: objects x (number of output channels) */

  (*phgVBAPRenderer)->startGainsMax =
      (FIXP_DBL*)FDKmalloc((*phgVBAPRenderer)->numChannels * sizeof(FIXP_DBL));
  (*phgVBAPRenderer)->prevGainsMax =
      (FIXP_DBL*)FDKmalloc((*phgVBAPRenderer)->numChannels * sizeof(FIXP_DBL));
  if (((*phgVBAPRenderer)->gainCache == NULL) || ((*phgVBAPRenderer)->startGains == NULL) ||
      ((*phgVBAPRenderer)->endGains == NULL) || ((*phgVBAPRenderer)->stepState == NULL) ||
      ((*phgVBAPRenderer)->scaleState == NULL) || ((*phgVBAPRenderer)->startGainsMax == NULL) ||
      ((*phgVBAPRenderer)->prevGainsMax == NULL)) {
    return -2; /* could not allocate memory */
  }

  (*phgVBAPRenderer)->renderMode = renderMode;

  if (renderMode == GVBAP_ENHANCED) {
    /* initialization of spread rendering */
    (*phgVBAPRenderer)->spread_numInvolvedLS = outChannels + numGhosts - numLFE;

    (*phgVBAPRenderer)->ghostVoiceOfGodSpeakerIndex = -1;
    (*phgVBAPRenderer)->ghostVoiceOfHellSpeakerIndex = -1;
    for (i = 0; i < (*phgVBAPRenderer)->spread_numInvolvedLS; i++) {
      /* Assumption: A speaker is a ghost speaker if it is not downmixed to exactly one output
       * speaker */
      int nOutputTargets = 0;
      for (int j = 0; j < (*phgVBAPRenderer)->downmixMatrixNumRows; j++) {
        if ((LONG)(*phgVBAPRenderer)->downmixMatrix[j][i] != 0) nOutputTargets++;
      }

      if (nOutputTargets != 1) {
        FIXP_DBL el = vL->element[i].sph.ele;
        if (el > (FIXP_DBL)(85 * 11930464)) (*phgVBAPRenderer)->ghostVoiceOfGodSpeakerIndex = i;
        if (el < (FIXP_DBL)(-85 * 11930464)) (*phgVBAPRenderer)->ghostVoiceOfHellSpeakerIndex = i;
      }
    }

    (*phgVBAPRenderer)->spread_gainArray =
        (FIXP_DBL*)FDKcalloc((*phgVBAPRenderer)->spread_numInvolvedLS, sizeof(FIXP_DBL));
    if ((*phgVBAPRenderer)->spread_gainArray == NULL) {
      return -2;
    }

    for (i = 0; i < GVBAP_SPREAD_NUM_VSO; i++) {
      (*phgVBAPRenderer)->spread_gainsVSO[i] =
          (FIXP_DBL*)FDKcalloc((*phgVBAPRenderer)->spread_numInvolvedLS, sizeof(FIXP_DBL));
      if ((*phgVBAPRenderer)->spread_gainsVSO[i] == NULL) {
        return -2;
      }
    }

    {
      FIXP_DBL aziVSOAngles[GVBAP_SPREAD_NUM_VSO_AZI], eleVSOAngles[GVBAP_SPREAD_NUM_VSO_ELE];
      int na, ne, objNo = 0;
      OAM_SAMPLE oam;

      /* virtual spread object positions */
      gVBAPRenderer_Spread_initVSOPositions(aziVSOAngles, eleVSOAngles);

      oam.gain = FL2FXCONST_DBL(1.0);
      oam.spreadAngle = oam.spreadHeight = oam.spreadDepth = (FIXP_DBL)0;

      /* panning of VSOs with VBAP */
      for (ne = 1; ne < GVBAP_SPREAD_NUM_VSO_ELE - 1; ne++) {
        for (na = 0; na < GVBAP_SPREAD_NUM_VSO_AZI; na++) {
          oam.sph.azi = aziVSOAngles[na];
          oam.sph.ele = eleVSOAngles[ne];
          oam.sph.rad = FL2FXCONST_DBL(1.0);
          oam.goa_bsObjectDistance = 0;
          oam.cart = sphericalToCartesian(oam.sph);

          /* "non-downmix" gain output including ghost speakers */
          calculateVbap(*phgVBAPRenderer, oam, (*phgVBAPRenderer)->spread_gainsVSO[objNo], 1);
          objNo++;
        }
      }

      /* vbap-panning of virtual spread objects at elevation -90 degrees */
      oam.sph.azi = (FIXP_DBL)0;
      oam.sph.ele = eleVSOAngles[0];
      oam.sph.rad = FL2FXCONST_DBL(1.0);
      oam.cart = sphericalToCartesian(oam.sph);
      calculateVbap(*phgVBAPRenderer, oam, (*phgVBAPRenderer)->spread_gainsVSO[objNo], 1);
      objNo++;

      /* vbap-panning of virtual spread objects at elevation +90 degrees */
      oam.sph.azi = (FIXP_DBL)0;
      oam.sph.ele = eleVSOAngles[GVBAP_SPREAD_NUM_VSO_ELE - 1];
      oam.sph.rad = FL2FXCONST_DBL(1.0);
      oam.cart = sphericalToCartesian(oam.sph);
      calculateVbap(*phgVBAPRenderer, oam, (*phgVBAPRenderer)->spread_gainsVSO[objNo], 1);
      objNo++;

      /* further parameter definition */
      for (i = 0; i < GVBAP_SPREAD_NUM_VSO_AZI; i++) {
        (*phgVBAPRenderer)->spread_indexRing[i] = i;
        (*phgVBAPRenderer)->spread_indexRing[GVBAP_SPREAD_NUM_VSO_AZI + i] = i;
        (*phgVBAPRenderer)->spread_indexRing[2 * GVBAP_SPREAD_NUM_VSO_AZI + i] = i;
      }
    }
  }

  /* Alocate memory for OAM samples */
  ((*phgVBAPRenderer)->oamSamples = (OAM_SAMPLE**)FDKcalloc(numOamFrames, sizeof(OAM_SAMPLE*)));

  if (!((*phgVBAPRenderer)->oamSamples)) {
    return -2; /* could not allocate memory */
  }
  if (((*phgVBAPRenderer)->oamSamples[0] =
           (OAM_SAMPLE*)FDKmalloc(numOamFrames * numObjects * sizeof(OAM_SAMPLE))) == NULL) {
    return -2; /* could not allocate memory */
  }
  for (i = 1; i < numOamFrames; i++) {
    (*phgVBAPRenderer)->oamSamples[i] = (*phgVBAPRenderer)->oamSamples[0] + i * numObjects;
  }

  /* set initial values for the case that the first frame is a bad one */
  for (i = 0; i < (*phgVBAPRenderer)->numObjects; i++) {
    (*phgVBAPRenderer)->oamSamples[numOamFrames - 1][i].gain = FL2FX_DBL(1.00f / 8.0f);
    (*phgVBAPRenderer)->oamSamples[numOamFrames - 1][i].goa_bsObjectDistance = 0;
    (*phgVBAPRenderer)->oamSamples[numOamFrames - 1][i].spreadAngle = (FIXP_DBL)0;
    (*phgVBAPRenderer)->oamSamples[numOamFrames - 1][i].sph.azi = (FIXP_DBL)0;
    (*phgVBAPRenderer)->oamSamples[numOamFrames - 1][i].sph.ele = (FIXP_DBL)0;
    (*phgVBAPRenderer)->oamSamples[numOamFrames - 1][i].sph.rad = FL2FXCONST_DBL(1.0 / 16);
    for (int j = 0; j < numOamFrames; j++) {
      (*phgVBAPRenderer)->oamSamples[j][i].sph.rad = FL2FXCONST_DBL(1.0 / 16);
    }
    (*phgVBAPRenderer)->oamSamples[numOamFrames - 1][i].spreadDepth = (FIXP_DBL)0;
    (*phgVBAPRenderer)->oamSamples[numOamFrames - 1][i].spreadHeight = (FIXP_DBL)0;
  }
  (*phgVBAPRenderer)->metadataPresent[numOamFrames - 1] = 1;
  FDKmemclear((*phgVBAPRenderer)->metadataPresent,
              (numOamFrames - 1) * sizeof((*phgVBAPRenderer)->metadataPresent[0]));

  /* CONCEALMENT */
  if (((*phgVBAPRenderer)->oamSamplesValid =
           (OAM_SAMPLE**)FDKcalloc(numOamFrames, sizeof(OAM_SAMPLE*))) == NULL) {
    return -2; /* could not allocate memory */
  }

  if (((*phgVBAPRenderer)->oamSamplesValid[0] =
           (OAM_SAMPLE*)FDKmalloc(numOamFrames * numObjects * sizeof(OAM_SAMPLE))) == NULL) {
    return -2; /* could not allocate memory */
  }
  for (i = 1; i < numOamFrames; i++) {
    (*phgVBAPRenderer)->oamSamplesValid[i] =
        (*phgVBAPRenderer)->oamSamplesValid[0] + i * numObjects;
  }

  /* if no good frame is stored yet, then everything will rendered to center with gain 1.0 */
  for (i = 0; i < (*phgVBAPRenderer)->numObjects; i++) {
    (*phgVBAPRenderer)->oamSamplesValid[numOamFrames - 1][i].gain = FL2FX_DBL(1.00f / 8.0f);
    (*phgVBAPRenderer)->oamSamplesValid[numOamFrames - 1][i].goa_bsObjectDistance = 0;
    (*phgVBAPRenderer)->oamSamplesValid[numOamFrames - 1][i].sph.azi = (FIXP_DBL)0;
    (*phgVBAPRenderer)->oamSamplesValid[numOamFrames - 1][i].sph.ele = (FIXP_DBL)0;
    (*phgVBAPRenderer)->oamSamplesValid[numOamFrames - 1][i].sph.rad = FL2FXCONST_DBL(1.0 / 16);
    (*phgVBAPRenderer)->oamSamplesValid[numOamFrames - 1][i].spreadAngle = (FIXP_DBL)0;
    (*phgVBAPRenderer)->oamSamplesValid[numOamFrames - 1][i].spreadDepth = (FIXP_DBL)0;
    (*phgVBAPRenderer)->oamSamplesValid[numOamFrames - 1][i].spreadHeight = (FIXP_DBL)0;
  }
  (*phgVBAPRenderer)->metadataPresentValid[numOamFrames - 1] = 1;
  FDKmemclear((*phgVBAPRenderer)->metadataPresentValid,
              (numOamFrames - 1) * sizeof((*phgVBAPRenderer)->metadataPresentValid[0]));
  (*phgVBAPRenderer)->flagOamFrameOk = 0;

  /* clear fixed_val flags */
  FDKmemclear((*phgVBAPRenderer)->fixed_val,
              GVBAPRENDERER_MAX_OBJECTS * sizeof((*phgVBAPRenderer)->fixed_val[0]));

  /* clear OAM parsed data */
  FDKmemclear(
      (*phgVBAPRenderer)->OAM_parsed_data,
      GVBAPRENDERER_MAX_OBJECTS * (OAM_NUMBER_COMPONENTS + 1 + 2) *
          sizeof((*phgVBAPRenderer)->OAM_parsed_data[0])); /* + 2 => spread high and depth */

  if (renderMode == GVBAP_ENHANCED) {
    (*phgVBAPRenderer)->hasUniformSpread = hasUniformSpread;
  } else {
    (*phgVBAPRenderer)->hasUniformSpread = 1; /* set uniform Spread as true */
  }

  return 0;
}

int gVBAPRenderer_RenderFrame_Time(HANDLE_GVBAPRENDERER RESTRICT hgVBAPRenderer,
                                   VBAP_PCM* RESTRICT inputBuffer, VBAP_PCM* RESTRICT outputBuffer,
                                   const INT startSamplePosition,
                                   const int inputBufferChannelOffset) {
  static const FIXP_DBL inv_length_table[] = {
    (FIXP_DBL)0x04000000,
    (FIXP_DBL)0x02000000,
    (FIXP_DBL)0x01555555,
    (FIXP_DBL)0x01000000,
    (FIXP_DBL)0x00CCCCCD,
    (FIXP_DBL)0x00AAAAAB,
    (FIXP_DBL)0x00924925,
    (FIXP_DBL)0x00800000,
    (FIXP_DBL)0x0071C71C,
    (FIXP_DBL)0x00666666,
    (FIXP_DBL)0x005D1746,
    (FIXP_DBL)0x00555555,
    (FIXP_DBL)0x004EC4EC,
    (FIXP_DBL)0x00492492,
    (FIXP_DBL)0x00444444,
    (FIXP_DBL)0x00400000
#if (GVBAPRENDERER_MAX_FRAMELENGTH > 1024)
    ,
    (FIXP_DBL)0x003C3C3C,
    (FIXP_DBL)0x0038E38E,
    (FIXP_DBL)0x0035E50C,
    (FIXP_DBL)0x00333332,
    (FIXP_DBL)0x0030C30C,
    (FIXP_DBL)0x002E8BA2,
    (FIXP_DBL)0x002C8590,
    (FIXP_DBL)0x002AAAAA,
    (FIXP_DBL)0x0028F5C2,
    (FIXP_DBL)0x00276276,
    (FIXP_DBL)0x0025ED08,
    (FIXP_DBL)0x00249248,
    (FIXP_DBL)0x00234F72,
    (FIXP_DBL)0x00222222,
    (FIXP_DBL)0x00210842,
    (FIXP_DBL)0x00200000
#endif
  };

  int object;
  int channel;
  int mappedChannel;
  int length;
  int numOamFrames, oamFrame, startSample, stopSample, oamFrameWithMetaData;
  FIXP_DBL step;
  FIXP_DBL scale;
  FIXP_DBL inv_length;
  FIXP_DBL numObjectsFract;

  /* check if output buffer exists */
  FDK_ASSERT(outputBuffer != NULL);
  /* Check alignment required for optimizations */
  C_ALLOC_ALIGNED_CHECK(inputBuffer);
  C_ALLOC_ALIGNED_CHECK(outputBuffer);

  if (!hgVBAPRenderer->oamDataValid) {
    oam_concealment(hgVBAPRenderer);
  }

  numOamFrames = hgVBAPRenderer->numOamFrames;
  oamFrame = 0;
  startSample = 0;
  oamFrameWithMetaData = -1;

  /* number of objects in fractional representation scaled by VBAP_MAX_SCALE_OBJECTS bits (maximum
   * of 31 Objects) */
  numObjectsFract = ((FIXP_DBL)hgVBAPRenderer->numObjects)
                    << (DFRACT_BITS - 1 - VBAP_MAX_SCALE_OBJECTS);

  FIXP_DBL endGainsMax[GVBAPRENDERER_MAX_CHANNEL_OUT];

  while (oamFrame < numOamFrames) {
    OAM_SAMPLE *oamStopSamples, oamStopSample;

    if (!hgVBAPRenderer->metadataPresent[oamFrame]) {
      if (oamFrame != (numOamFrames - 1)) {
        oamFrame++;
        continue;
      }
    }

    stopSample = (oamFrame + 1) * hgVBAPRenderer->oamFrameLength;
    if (hgVBAPRenderer->metadataPresent[oamFrame]) {
      oamStopSamples = hgVBAPRenderer->oamSamples[oamFrame];
      oamFrameWithMetaData = oamFrame;
    } else {
      FDK_ASSERT(oamFrameWithMetaData > -1);
      oamStopSamples = hgVBAPRenderer->oamSamples[oamFrameWithMetaData];
    }

    length = stopSample - startSample;
    FDK_ASSERT(length <= GVBAPRENDERER_MAX_FRAMELENGTH);
    inv_length = inv_length_table[(length >> 6) - 1];

    FDK_ASSERT((length & 63) == 0);

    FDK_ASSERT(startSamplePosition <= length);

    /* Do rendering for all objects */
    for (channel = 0; channel < hgVBAPRenderer->numChannels; channel++) {
      endGainsMax[channel] = (FIXP_DBL)0;
    }

    /* First calculate gains for all objects */
    for (object = 0; object < hgVBAPRenderer->numObjects; object++) {
      /* create local copy of sample, as radius is modified */
      oamStopSample = oamStopSamples[object];

      if (hgVBAPRenderer->renderMode == GVBAP_LEGACY) {
        oamStopSample.sph.rad = FL2FXCONST_DBL(1.0 / 16);
        oamStopSample.cart = sphericalToCartesian(oamStopSample.sph);
        calculateVbap(hgVBAPRenderer, oamStopSample, hgVBAPRenderer->endGains[object], 0);
      } else {
        /* distance to spread mapping */
        gVBAPRenderer_Spread_distanceToSpread(&oamStopSample, hgVBAPRenderer->hasUniformSpread);

        oamStopSample.sph.rad = FL2FXCONST_DBL(1.0 / 16);
        oamStopSample.cart = sphericalToCartesian(oamStopSample.sph);

        if ((oamStopSample.spreadAngle != (FIXP_DBL)0) ||
            (!hgVBAPRenderer->hasUniformSpread && oamStopSample.spreadHeight != (FIXP_DBL)0)) {
          gVBAPRenderer_Spread_renderSpread(hgVBAPRenderer, hgVBAPRenderer->endGains[object],
                                            oamStopSample);
        } else {
          calculateVbap(hgVBAPRenderer, oamStopSample, hgVBAPRenderer->endGains[object], 0);
        }
      }

      for (channel = 0; channel < hgVBAPRenderer->numChannels; channel++) {
        endGainsMax[channel] =
            fMax(endGainsMax[channel], hgVBAPRenderer->endGains[object][channel]);
      }

      /* is it the first run? If yes copy endGains to startGains */
      if (hgVBAPRenderer->startGainsFilled == 0) {
        for (channel = 0; channel < hgVBAPRenderer->numChannels; channel++) {
          hgVBAPRenderer->startGains[object][channel] = hgVBAPRenderer->endGains[object][channel];
        }
      }
    }
    if (hgVBAPRenderer->startGainsFilled == 0) {
      for (channel = 0; channel < hgVBAPRenderer->numChannels; channel++) {
        hgVBAPRenderer->prevGainsMax[channel] = (FIXP_DBL)0;
        hgVBAPRenderer->startGainsMax[channel] = endGainsMax[channel];
      }
    }
    hgVBAPRenderer->startGainsFilled = 1;

    /* Add object channels multiplied by gain to output buffer */
    for (channel = 0; channel < hgVBAPRenderer->numChannels; channel++) {
      int s, s1, s2;
      FIXP_DBL tmp, maxGain;

      mappedChannel = hgVBAPRenderer->speakerSetup.mapping[channel];
      C_AALLOC_SCRATCH_START(outputCache, FIXP_DBL, GVBAPRENDERER_MAX_FRAMELENGTH)
      FDKmemclear(outputCache, length * sizeof(FIXP_DBL));

      tmp = fMax(hgVBAPRenderer->startGainsMax[channel], endGainsMax[channel]);
      maxGain = fMax(tmp, hgVBAPRenderer->prevGainsMax[channel]);

      hgVBAPRenderer->prevGainsMax[channel] = tmp;
      hgVBAPRenderer->startGainsMax[channel] = endGainsMax[channel];

      tmp = fMult(maxGain, numObjectsFract);

      /* calculate headroom of accumulated gains */
      s = fMax(CntLeadingZeros(tmp) - 1, 0);

      /* scalefactor s can be in the range of -VBAP_SCALE_GAINS to VBAP_MAX_SCALE_OBJECTS */
      s = fMax(-VBAP_SCALE_GAINS, (VBAP_MAX_SCALE_OBJECTS - s));

      /* scalefactor s2 can be in the range of 1 to VBAP_MAX_SCALE_SAMPLES+1 */
      s2 = VBAP_MAX_SCALE_SAMPLES - VBAP_MAX_SCALE_OBJECTS + s + 1;

      /* consider fMultDiv2() */
      s = s - 1;

      s1 = s;
      if (s < 0) {
        s1 = -s;
      }

      for (object = 0; object < hgVBAPRenderer->numObjects; object++) {
        /* load States */
        FIXP_DBL scaleState = hgVBAPRenderer->scaleState[object][channel];
        FIXP_DBL stepState = hgVBAPRenderer->stepState[object][channel];

        /* linear interpolation */
        step = fMultDiv2((hgVBAPRenderer->endGains[object][channel] -
                          hgVBAPRenderer->startGains[object][channel]),
                         inv_length);
        scale = (hgVBAPRenderer->startGains[object][channel]);

        /* Save last endGains to next startGains */
        hgVBAPRenderer->startGains[object][channel] = hgVBAPRenderer->endGains[object][channel];
        /* Skip cases which are not exactly zero due to calculation differences (improves accuracy
         * and workload) */
        const FIXP_DBL minGain = (FIXP_DBL)0xfff;
        if (scale < minGain && step == (FIXP_DBL)0 && scaleState < minGain &&
            stepState == (FIXP_DBL)0) {
          continue;
        }
        VBAP_PCM* pIn = &inputBuffer[(object * inputBufferChannelOffset) + startSample];
        FDK_ASSERT((startSamplePosition & (INT)7) ==
                   0); /* due to arm (divisible by 8) and xtensa (divisible by 4) restriction in
                          implementation */
        FDK_ASSERT((length & (INT)7) == 0); /* due to arm (divisible by 8) and xtensa (divisible by
                                               4) restriction in implementation */

/* #define DEBUG_gVBAPRenderer_RenderFrame_Time_func1 */
#ifdef DEBUG_gVBAPRenderer_RenderFrame_Time_func1
        INT start_clock = FDKclock();
#endif
#if defined(FUNCTION_gVBAPRenderer_RenderFrame_Time_func1)
        /* save states */
        hgVBAPRenderer->scaleState[object][channel] = scale + step * (length - startSamplePosition);
        hgVBAPRenderer->stepState[object][channel] = step;
        gVBAPRenderer_RenderFrame_Time_func1(outputCache, pIn, length, startSamplePosition, scale,
                                             step, scaleState, stepState, s, s1);
#else
        int sample;

        if (s < 0) {
          FDK_ASSERT(scale <= maxGain);
          FDK_ASSERT(scaleState <= maxGain);
          for (sample = 0; sample < startSamplePosition;
               sample++) /* process first startSamplePosition samples */
          {
            scaleState = scaleState + stepState;
            outputCache[sample] += fMult((FIXP_DBL)pIn[sample], scaleState) << (s1 - 1);
          }
          for (; sample < length; sample++) /* process remaining samples */
          {
            scale = scale + step;
            outputCache[sample] += fMult((FIXP_DBL)pIn[sample], scale) << (s1 - 1);
          }
          FDK_ASSERT(scale <= maxGain);
          FDK_ASSERT(scaleState <= maxGain);
        } else {
          FDK_ASSERT(scale <= maxGain);
          FDK_ASSERT(scaleState <= maxGain);
          for (sample = 0; sample < startSamplePosition;
               sample++) /* process first startSamplePosition samples */
          {
            scaleState = scaleState + stepState;
            outputCache[sample] += fMult((FIXP_DBL)pIn[sample], scaleState) >> (s1 + 1);
          }
          for (; sample < length; sample++) /* process remaining samples */
          {
            scale = scale + step;
            outputCache[sample] += fMult((FIXP_DBL)pIn[sample], scale) >> (s1 + 1);
          }
          FDK_ASSERT(scale <= maxGain);
          FDK_ASSERT(scaleState <= maxGain);
        }
        /* save states */
        hgVBAPRenderer->scaleState[object][channel] = scale;
        hgVBAPRenderer->stepState[object][channel] = step;
#endif
#ifdef DEBUG_gVBAPRenderer_RenderFrame_Time_func1
        INT stop_clock = FDKclock();
        FDKprintf("gVBAPRenderer_RenderFrame_Time_func1: %d  length=%d  s=%d  s1=%d\n",
                  stop_clock - start_clock, length, s, s1);
        for (int sample = 0; sample < length; sample += 4) {
          FDKprintf("0x%08X 0x%08X 0x%08X 0x%08X smp=%d 0x%08X 0x%08X 0x%08X 0x%08X\n",
                    outputCache[sample + 0], outputCache[sample + 1], outputCache[sample + 2],
                    outputCache[sample + 3], sample, pIn[sample + 0], pIn[sample + 1],
                    pIn[sample + 2], pIn[sample + 3]);
        }
        FDKprintf("scale: 0x%08X\n", hgVBAPRenderer->scaleState[object][channel]);
        FDKprintf("step:  0x%08X\n", hgVBAPRenderer->stepState[object][channel]);
        ;
#endif
      }

      VBAP_PCM* pOut = &outputBuffer[(mappedChannel * hgVBAPRenderer->frameLength) + startSample];

/* #define DEBUG_gVBAPRenderer_RenderFrame_Time_func2 */
#ifdef DEBUG_gVBAPRenderer_RenderFrame_Time_func2
      INT start_clock = FDKclock();
#endif

#if defined(FUNCTION_gVBAPRenderer_RenderFrame_Time_func2)
      gVBAPRenderer_RenderFrame_Time_func2(pOut, outputCache, length, s2 - 1);
#else

      /* copy left shifted and saturated samples to output buffer */
      if (s2 > 1) {
        for (int sample = 0; sample < length; sample++) {
          pOut[sample] = fAddSaturate(
              pOut[sample], SATURATE_LEFT_SHIFT(outputCache[sample], s2 - 1, DFRACT_BITS));
        }
      } else {
        for (int sample = 0; sample < length; sample++) {
          pOut[sample] = fAddSaturate(pOut[sample], outputCache[sample]);
        }
      }
#endif
#ifdef DEBUG_gVBAPRenderer_RenderFrame_Time_func2
      INT stop_clock = FDKclock();
      FDKprintf("gVBAPRenderer_RenderFrame_Time_func2: %d  s2=%d\n", stop_clock - start_clock, s2);
      for (int sample = 0; sample < length; sample += 4) {
        FDKprintf("0x%08X 0x%08X 0x%08X 0x%08X smp=%d 0x%08X 0x%08X 0x%08X 0x%08X\n",
                  pOut[sample + 0], pOut[sample + 1], pOut[sample + 2], pOut[sample + 3], sample,
                  outputCache[sample + 0], outputCache[sample + 1], outputCache[sample + 2],
                  outputCache[sample + 3]);
      }
#endif
      C_AALLOC_SCRATCH_END(outputCache, FIXP_DBL, GVBAPRENDERER_MAX_FRAMELENGTH)
    }

    oamFrame++;
    startSample = stopSample;
  } /* while loop over all oamSamples (in case of high rate OAM) */

  return 0;
}

int gVBAPRenderer_Close(HANDLE_GVBAPRENDERER hgVBAPRenderer) {
  /* free start and end gain memory */
  fdkFreeMatrix2D((void**)hgVBAPRenderer->startGains);
  fdkFreeMatrix2D((void**)hgVBAPRenderer->endGains);

  fdkFreeMatrix2D((void**)hgVBAPRenderer->stepState);
  fdkFreeMatrix2D((void**)hgVBAPRenderer->scaleState);

  /* free maximum start gain memory */
  FDKfree(hgVBAPRenderer->startGainsMax);

  /* free maximum previous gain memory */
  FDKfree(hgVBAPRenderer->prevGainsMax);

  /* free gain cache */
  FDKfree(hgVBAPRenderer->gainCache);

  /* free speaker triplet */
  FDKfree(hgVBAPRenderer->speakerSetup.speakerTriplet);

  /* free speaker list */
  FDKfree(hgVBAPRenderer->speakerSetup.speakerList);

  /* free speaker mapping array */
  FDKfree(hgVBAPRenderer->speakerSetup.mapping);

  /* free downmixMatrix */
  if (hgVBAPRenderer->downmixMatrix != NULL) {
    fdkFreeMatrix2D((void**)hgVBAPRenderer->downmixMatrix);
    hgVBAPRenderer->downmixMatrix = NULL;
  }

  if (hgVBAPRenderer->oamSamples) {
    if (hgVBAPRenderer->oamSamples[0]) {
      FDKfree(hgVBAPRenderer->oamSamples[0]);
    }
    FDKfree(hgVBAPRenderer->oamSamples);
  }
  if (hgVBAPRenderer->oamSamplesValid) {
    if (hgVBAPRenderer->oamSamplesValid[0]) {
      FDKfree(hgVBAPRenderer->oamSamplesValid[0]);
    }
    FDKfree(hgVBAPRenderer->oamSamplesValid);
  }

  for (int i = 0; i < GVBAP_SPREAD_NUM_VSO; i++) {
    if (hgVBAPRenderer->spread_gainsVSO[i] != NULL) {
      FDKfree(hgVBAPRenderer->spread_gainsVSO[i]);
    }
  }
  if (hgVBAPRenderer->spread_gainArray != NULL) {
    FDKfree(hgVBAPRenderer->spread_gainArray);
  }

  /* free VBAP handle */
  FDKfree(hgVBAPRenderer);

  return 0;
}
