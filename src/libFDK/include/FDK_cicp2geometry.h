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

   Author(s):

   Description:

*******************************************************************************/

/***********************************************************************************

 This software module was originally developed by

 Fraunhofer IIS

 in the course of development of the ISO/IEC 23008-3 for reference purposes and its
 performance may not have been optimized. This software module is an implementation
 of one or more tools as specified by the ISO/IEC 23008-3 standard. ISO/IEC gives
 you a royalty-free, worldwide, non-exclusive, copyright license to copy, distribute,
 and make derivative works of this software module or modifications  thereof for use
 in implementations or products claiming conformance to the ISO/IEC 23008-3 standard
 and which satisfy any specified conformance criteria. Those intending to use this
 software module in products are advised that its use may infringe existing patents.
 ISO/IEC have no liability for use of this software module or modifications thereof.
 Copyright is not released for products that do not conform to the ISO/IEC 23008-3
 standard.

 Fraunhofer IIS retains full right to modify and use the code for its own purpose,
 assign or donate the code to a third party and to inhibit third parties from using
 the code for products that do not conform to MPEG-related ITU Recommendations and/or
 ISO/IEC International Standards.

 This copyright notice must be included in all copies or derivative works.

 Copyright (c) ISO/IEC 2013.

 ***********************************************************************************/

#ifndef FDK_CICP2GEOMETRY_H
#define FDK_CICP2GEOMETRY_H

#include "machine_type.h"

/** @mainpage cicp2geometryLib
 *
 * Converts CICP information to geometric description of the
 * reproduction setup or (tries) vice versa.
 * Also allows to read and write geometry description from/to
 * files.
 *
 * @author Michael Fischer
 **/

/** \file cicp2geometry.h */

#define CICP2GEOMETRY_MAX_LOUDSPEAKERS (32)

/**********************************************************************/ /**

 **************************************************************************/

typedef enum {
  CICP2GEOMETRY_OK = 0,                   /**< no error */
  CICP2GEOMETRY_INIT_ERROR = -999,        /**< initialization failed */
  CICP2GEOMETRY_CONFIG_ERROR = -998,      /**< bad configuration of the channel CICP2GEOMETRY */
  CICP2GEOMETRY_BAD_POINTER = -997,       /**< bad pointer or corrupted memory */
  CICP2GEOMETRY_ALLOC_ERROR = -996,       /**< memory couldn't be allocated */
  CICP2GEOMETRY_INVALID_AZIMITH = -995,   /**< invalid azimuth angle */
  CICP2GEOMETRY_INVALID_ELEVATION = -994, /**< invalid elevation angle */
  CICP2GEOMETRY_INVALID_LFE_FLAG = -993,  /**< invalid LFE flag */
  CICP2GEOMETRY_INVALID_SCREEN_RELATIVE_FLAG = -992, /**< invalid screen relative flag */
  CICP2GEOMETRY_UNKNOWN_SPEAKER_INDEX = -991,        /**< unknown cicp speaker index */
  CICP2GEOMETRY_INVALID_CICP_INDEX = -990,           /**< invalid cicp index */
  CICP2GEOMETRY_UNSUPPORTED_CHANNEL_COUNT = -989,    /**< unsupported number of channels */
  CICP2GEOMETRY_INVALID_GEO_FILE_SYNTAX = -988, /**< invalid syntax within geometric description */
  CICP2GEOMETRY_INVALID_FILEHANDLE = -987       /**< invalid file handle */
} CICP2GEOMETRY_ERROR;

/**********************************************************************/ /**

 **************************************************************************/

#define CICP2GEOMETRY_CICP_INVALID (-1000)
#define CICP2GEOMETRY_CICP_VALID 0

/**********************************************************************/ /**

 **************************************************************************/

typedef enum {
  CICP2GEOMETRY_LOUDSPEAKER_KNOWN = 1,   /**< Known CICP loudspeaker */
  CICP2GEOMETRY_LOUDSPEAKER_UNKNOWN = 0, /**< Unknown CICP loudspeaker */
  CICP2GEOMETRY_LOUDSPEAKER_INVALID = -1 /**< Invalid index for a CICP loudspeaker */
} CICP2GEOMETRY_LOUDSPEAKER_TYPE;

/**********************************************************************/ /**

 **************************************************************************/
typedef struct cicp2geometry_channel_geometry {
  int cicpLoudspeakerIndex; /**< index specifying cicp loudspeaker. -1 if only geometry is given. */
  int Az;                   /**< azimuth angle of the channel    */
  int El;                   /**< elevation angle of the channel  */
  int LFE;                  /**< flag whether channel is an LFE  */
  int screenRelative;       /**< flag whether loudspeaker position is screen relative */
  int hasDistance;          /**< distance info available for loudspeaker */
  unsigned int distance;    /**< distance value in cm */
  int hasLoudspeakerCalibrationGain;              /**< calibration info available for loudspeaker */
  int loudspeakerCalibrationGain;                 /**< linear calibration gain */
  CICP2GEOMETRY_LOUDSPEAKER_TYPE loudspeakerType; /**<  enum describing the loudspeaker type */
} CICP2GEOMETRY_CHANNEL_GEOMETRY;

/**********************************************************************/ /**
 get_geometry_from_cicp() expects a cicpIndex and will define  the
 CICP2GEOMETRY_CHANNEL_GEOMETRY for each channel. Additionally the number
 of channels and LFEs is returned.

 \return returns 0 for no error, 1 in case of an error.
 **************************************************************************/
CICP2GEOMETRY_ERROR cicp2geometry_get_geometry_from_cicp(
    int cicpIndex, /**<  in: cicpIndex indicating the reproduction setup  */
    CICP2GEOMETRY_CHANNEL_GEOMETRY
        AzElLfe[CICP2GEOMETRY_MAX_LOUDSPEAKERS], /**< out: pointer to an array with
                                                    CICP2GEOMETRY_CHANNEL_GEOMETRYs */
    int* numChannels, /**< out: number of effective output channels for chosen reproduction setup
                         (i.e. w/o LFEs) */
    int* numLFEs      /**< out: number of LFEs in chosen reproduction setup  */
);

/**********************************************************************/ /**
 get_numChannels_from_cicp() expects a cicpIndex and returns the number
 of channels.

 \return returns 0 for no error, 1 in case of an error.
 **************************************************************************/
CICP2GEOMETRY_ERROR cicp2geometry_get_numChannels_from_cicp(int cicpIndex, int* numTotalChannels);

inline int cicp2geometry_get_numChannels_from_cicp(int cicpIndex) {
  int n = 0;
  cicp2geometry_get_numChannels_from_cicp(cicpIndex, &n);
  return n;
}

/**********************************************************************/ /**
 cicp2geometry_get_cicpIndex_from_geometry() expects an array of
 CICP2GEOMETRY_CHANNEL_GEOMETRYs and will match these to a cicp loudspeaker
 layout index- if there is a match.

 \return returns 0 for no error, 1 in case of an error.
 **************************************************************************/
CICP2GEOMETRY_ERROR cicp2geometry_get_cicpIndex_from_geometry(
    CICP2GEOMETRY_CHANNEL_GEOMETRY*
        AzElLfe,     /**< in: pointer to an array with CICP2GEOMETRY_CHANNEL_GEOMETRYs */
    int numChannels, /**< in: number of effective output channels in this reproduction setup (i.e.
                        w/o LFEs) */
    int numLFEs,     /**< in: number of LFEs in this reproduction setup  */
    int* cicpIndex   /**< out: cicpIndex of this reproduction setup  */
);

/**********************************************************************/ /**
 cicp2geometry_get_geometry_from_cicp_loudspeaker_index() expects a CICP
 loudspeaker index and writes the according geometric data into the struct
 of the given pointer.

 \return returns 0 for no error, 1 in case of an error.
 **************************************************************************/
CICP2GEOMETRY_ERROR cicp2geometry_get_geometry_from_cicp_loudspeaker_index(
    int cicpLoudspeakerIndex, /**<  in: cicp speaker index specifying the geometric position */
    CICP2GEOMETRY_CHANNEL_GEOMETRY*
        AzElLfe /**< out:pointer to an array with CICP2GEOMETRY_CHANNEL_GEOMETRYs */
);

#ifdef FDK_FC_MISPLACED_SPEAKER_ENABLE
/**********************************************************************/ /**
 cicp2geometry_compare_geometry() compares two CICP2GEOMETRY_CHANNEL_GEOMETRY
 arrays.

 \return returns 0 if both geometries are equal.
 **************************************************************************/
int cicp2geometry_compare_geometry(
    CICP2GEOMETRY_CHANNEL_GEOMETRY geoOne[CICP2GEOMETRY_MAX_LOUDSPEAKERS],
    unsigned int numChannelsOne,
    CICP2GEOMETRY_CHANNEL_GEOMETRY geoTwo[CICP2GEOMETRY_MAX_LOUDSPEAKERS],
    unsigned int numChannelsTwo, unsigned int tolerance);
#endif

/*********************************************************************/ /**
                                                                         *************************************************************************/
CICP2GEOMETRY_ERROR cicp2geometry_get_number_of_lfes(CICP2GEOMETRY_CHANNEL_GEOMETRY* AzElLfe,
                                                     UINT numChannels, UINT* numLfes);

/*********************************************************************/ /**
                                                                         *************************************************************************/
CICP2GEOMETRY_ERROR cicp2geometry_get_deviation_angles(
    CICP2GEOMETRY_CHANNEL_GEOMETRY* AzElLfe, UINT numSpeaker, INT* azDev, INT* elDev,
    INT* isDeviated, CICP2GEOMETRY_CHANNEL_GEOMETRY* normalizedGeo);

/**
 * \brief Obtain indices and gains to a given target layout speaker geometry
 *        which would be suitable to mix in a given amount of speakers signals.
 * \param targetLayout The target CICP speaker layout index
 * \param numSpeakers The amount of speaker signals to be matched
 * \param numSignalsMix Pointer to store the amount of speaker signals to mix to.
 * \param speakerPosIndices Pointer to an array were the indices of the target layout are stored to
 * map the signals.
 * \param speakerGainsMatrix Pointer to an array were the gain matrix from numSpeakers to
 * numSignalsMix is stored.
 * \return CICP2GEOMETRY_ERROR
 */
CICP2GEOMETRY_ERROR cicp2geometry_get_front_speakers(const INT targetLayout, const INT numSpeakers,
                                                     INT* numSignalsMix, INT* speakerPosIndices,
                                                     INT* speakerGains);

#endif /* __CICP2GEOMETRY_H__ */
