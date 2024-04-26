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

#ifndef VBAP_CORE_H
#define VBAP_CORE_H

#include "cartesianMath.h"

#define VBAP_PCM FIXP_DBL

#define GVBAPRENDERER_MAX_OBJECTS 32
#define GVBAPRENDERER_MAX_CHANNEL_OUT 24
#define GVBAPRENDERER_MAX_GHOST_SPEAKER 6
#define GVBAPRENDERER_MAX_OAM_FRAMES_PER_CORE_FRAME 16
#define GVBAPRENDERER_MAX_FRAMELENGTH 1024

#define GVBAP_LEGACY 0
#define GVBAP_ENHANCED 1

#define GVBAP_SPREAD_NUM_VSO_AZI (12) /* 360 / 30 */
#define GVBAP_SPREAD_NUM_VSO_ELE (7)  /* 180 / 30 + 1 */

#define GVBAP_SPREAD_NUM_VSO (GVBAP_SPREAD_NUM_VSO_AZI * (GVBAP_SPREAD_NUM_VSO_ELE - 2) + 2)

/* OAM definitions */
#define OAM_NUMBER_COMPONENTS 5 /* number of OAM components */

#define OAM_BITS_AZI 8           /* number of quantization bits for the azimuth angle */
#define OAM_BITS_ELE 6           /* number of quantization bits for the elevation angle */
#define OAM_BITS_RAD 4           /* number of quantization bits for the radius (logarithmic) */
#define OAM_BITS_GAIN 7          /* number of quantization bits for the object gain */
#define OAM_BITS_SPREAD 7        /* number of quantization bits for the spread angle */
#define OAM_BITS_SPREAD_HEIGHT 5 /* number of quantization bits for the spread angle */
#define OAM_BITS_SPREAD_DEPTH 4  /* number of quantization bits for the spread angle */
#define OAM_BITS_OBJ_PRIORITY 3  /* number of quantization bits for the object priority */

typedef enum {
  AZIMUTH = 0,
  ELEVATION = 1,
  RADIUS = 2,
  GAIN = 3,
  SPREAD = 4,
  DYNAMIC_OBJECT_PRIORITY = 5,
  SPREAD_HEIGHT = 6,
  SPREAD_DEPTH = 7,
  OAM_NUMBER_MAX_COMPONENTS = 8 /* maximal number of OAM components */
} OAM_COMPONENT;

#define MIN_GAIN -42949672 /* -0.01f */

typedef struct _GVBAPRENDERER* HANDLE_GVBAPRENDERER;

/* This structure can hold a single OAM sample. */
typedef struct {
  PointSpherical sph;
  PointCartesian cart;
  FIXP_DBL gain; /**< linear gain value.                            Range 0.004 ... 5.957  Exponent
                    = 3     */
  FIXP_DBL spreadAngle;  /**< spread angle or spread width in radian.       Range  0 ... 180  mapped
                            to 0 ... 1.0 */
  FIXP_DBL spreadHeight; /**< spread Height in radian.                      Range  0 ...  90 mapped
                            to 0 ... 0.5 */
  FIXP_DBL spreadDepth;  /**< spread Depth.                                 Range  0 ... 15.5
                            Exponent = 4     */
  USHORT goa_bsObjectDistance;
} OAM_SAMPLE;

/*
 * @brief Contains all relevant information for one speaker
 * @param sph       Spherical speaker coordinates
 * @param isLFE     true if this speaker is a LFE, false if not
 * @param isGhost   true if this speaker is a virtual ghost speaker, false if not
 */
typedef struct _SPEAKER {
  PointSpherical sph;
  bool isLFE;
  bool isGhost;

} SPEAKER;

/*
 * @brief holds the relevant information for each speaker triangle
 * @param triangle        the indices for the three speakers which build a speaker triangle
 * @param matrix          the matrix which holds the Cartesian speaker coordinates (x,y,z) for the
 * three triangle speakers in each row
 * @param inverseMatrix   the inverse matrix of the matrix above
 */
typedef struct _SPEAKERTRIPLET {
  INT triangle[3];
  PointCartesian matrix[3];
  PointCartesian inverseMatrix[3];
  INT exponentInverseMatrix;

} SPEAKERTRIPLET;

/*
 * @brief all required information about the speaker setup
 * @param speakerList     a list of all real and LFE speaker with all necessary information
 * @param speakerTriplet  a list of all speaker triangles with matrices for rendering
 * @param coversUpperHalfSphere   true if the loudspeaker setup covers the sphere above 5 degrees
 * elevation
 * @param coversLowerHalfSphere   true if the loudspeaker setup covers the sphere below -5 degrees
 * elevation
 * @param coversLowerHalfSphere   true if the loudspeaker setup has speakers above 21 degrees
 * elevation
 */
typedef struct _SPEAKERSETUP {
  SPEAKER* speakerList;
  SPEAKERTRIPLET* speakerTriplet;

  int greatestInverseMatrixExponent;

  int speakerListSize;
  int speakerTripletSize;

  int* mapping;

  bool coversUpperHalfSphere;
  bool coversLowerHalfSphere;
  bool hasHeightSpeakers;

} SPEAKERSETUP;

/*
 * @brief gVBAP Handler. Holds all information about the speaker setup, the calculated start and end
 * gains, information about the output channels and number of objects. Additional it holds some
 * needed cache
 * @param speakerSetup      all information about the speakersetup
 * @param startGains        the calculated start gains
 * @param endGains          the calculated end gains
 * @param outputCache       cache on which all objects for one output channel is rendered. For copy
 * to output buffer a left shift with saturation will be done on this framelength * 32 bit cache.
 * @param baseVectors       holds the 19 basevectors which are calculated in spreading
 * @param numChannels       number of real speaker including LFE
 * @param numLFE            number of LFE
 * @param numGhosts         number of virtual speaker
 * @param numObjects        number of objects
 * @param frameLength       length of the current frame
 * @param gainCacheLength   length of the gainCache (it should be numChannels + numGhosts)
 * @param startGainsFilled  indicates whether the startGains are filled from last endGains or not
 * (important for first run)
 */
typedef struct _GVBAPRENDERER {
  PointCartesian baseVectors[19 + 1]; /* +1 for xtensa to have the possibility to do two operation
                                         at the same time */

  SPEAKERSETUP speakerSetup;

  OAM_SAMPLE** oamSamples; /* Dim: [OAM frames][objects] */

  UCHAR metadataPresent[GVBAPRENDERER_MAX_OAM_FRAMES_PER_CORE_FRAME]; /* Dim: [OAM frames] */

  UCHAR hasUniformSpread;

  UCHAR oamDataValid;
  UCHAR metadataPresentValid[GVBAPRENDERER_MAX_OAM_FRAMES_PER_CORE_FRAME]; /* Dim: [OAM frames] Used
                                                                              for concealment */
  OAM_SAMPLE** oamSamplesValid; /* Dim: [OAM frames][objects] Used for concealment */

  UCHAR fixed_val[OAM_NUMBER_MAX_COMPONENTS];
  INT* OAM_parsed_data; /* 32 bit signed integer */

  FIXP_DBL* startGainsMax;
  FIXP_DBL* prevGainsMax;

  FIXP_DBL** startGains; /* Dim: [objects][real channels without LFE] */
  FIXP_DBL** endGains;   /* Dim: [objects][real channels without LFE] */

  FIXP_DBL** stepState;  /* Dim: [objects][real channels without LFE] */
  FIXP_DBL** scaleState; /* Dim: [objects][real channels without LFE] */

  FIXP_DBL* gainCache; /* Dim: [real channels without LFE but with ghost speakers] */

  FIXP_DBL** downmixMatrix; /* Dim: [real speakers without LFE][real speaker without LFE + ghost
                               speaker] */
  int downmixMatrixNumRows, downmixMatrixNumCols;

  SCHAR numChannels;
  SCHAR numLFE;
  SCHAR numGhosts;
  SCHAR numObjects;
  SHORT frameLength;
  SHORT oamFrameLength;
  SCHAR numOamFrames;
  SCHAR gainCacheLength;

  UCHAR startGainsFilled;

  FIXP_DBL*
      spread_gainsVSO[GVBAP_SPREAD_NUM_VSO]; /**< vbap-panned gains of virtual spread objects */
  FIXP_DBL spread_gainsSpread[GVBAP_SPREAD_NUM_VSO]; /**< spread gains for virtual spread objects */
  FIXP_DBL* spread_gainArray;                        /**< spread gain array */
  SCHAR ghostVoiceOfGodSpeakerIndex;                 /**< index of ghost voice of god speaker */
  SCHAR ghostVoiceOfHellSpeakerIndex;                /**< index of ghost voice of hell speaker */
  UCHAR renderMode; /**< 0: original rendering (MPEG standardized), 1: alternative (proprietary)
                       rendering (new spread rendering, new imaginary speaker initialization) */
} GVBAPRENDERER;

/*
 * @brief use Cramer rule to calculate the inverse of the Matrix M
 *        Store the results in transposed order in the gVBAPRenderer Handle.
 * @param hgVBAPRenderer     gVBAPRenderer Handle
 */
int generateInverseMatrices(HANDLE_GVBAPRENDERER hgVBAPRenderer);

/*
 * @brief Some kind of wrapper for calculateVbapGain
 *        It checks whether a downmix matrix exists and if yes it maps the ghosts speakers to real
 * ones
 * @param hgVBAPRenderer  gVBAPRenderer Handle
 * @param source          Cartesian coordinates of object position
 * @param final_gains     Array of gains where the result will be stored. The length of this array
 * is the real number of speaker including LFE
 * @param gainFactor      Is a gain factor from the OAM sample
 * @param spreadAngle     Is a spread angle from the OAM sample
 */
void calculateVbap(HANDLE_GVBAPRENDERER hgVBAPRenderer, OAM_SAMPLE source, FIXP_DBL* final_gains,
                   int noDownmix);

/*
 * @brief Calculates gain factors for given object position.
 *         Includes OAM gain factor and OAM spread angle in calculation
 * @param hgVBAPRenderer  gVBAPRenderer Handle
 * @param source          Cartesian coordinates of object position
 * @param final_gains     Array of gains where the result will be stored. The length of this array
 * is the real number of speaker including LFE
 * @param gainFactor      Is a gain factor from the OAM sample
 * @param spreadAngle     Is a spread angle from the OAM sample
 */
void calculateVbapGain(HANDLE_GVBAPRENDERER hgVBAPRenderer, OAM_SAMPLE source);

/*
 * @brief Calculates spread vectors
 * @param hgVBAPRenderer  gVBAPRenderer Handle
 * @param source          Cartesian coordinates of object position
 * @param spreadAngle     Is a spread angle from the OAM sample.        -180 ... 180 degrees is
 * mapped to -1 ... 1
 */
void calcSpreadVectors(HANDLE_GVBAPRENDERER hgVBAPRenderer, OAM_SAMPLE source);

/*
 * @brief Calculates the triangle into which the base vectors points.
 *        At same time it calculates the gains for the three loudspeakers.
 * @param hgVBAPRenderer  gVBAPRenderer Handle
 */
void calculateOneSourcePosition(HANDLE_GVBAPRENDERER hgVBAPRenderer, PointCartesian* baseVector);

void calcSpreadGains(HANDLE_GVBAPRENDERER hgVBAPRenderer, FIXP_DBL spreadAngle);

#endif
