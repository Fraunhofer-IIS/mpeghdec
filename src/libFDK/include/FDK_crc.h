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

   Description: CRC calculation

*******************************************************************************/

#ifndef FDK_CRC_H
#define FDK_CRC_H

#include "FDK_bitstream.h"

#define MAX_CRC_REGS                                                                   \
  3 /*!< Maximal number of overlapping crc region in ADTS channel pair element is two. \
         Select three independent regions preventively. */

/**
 *  This structure describes single crc region used for crc calculation.
 */
typedef struct {
  UCHAR isActive;
  INT maxBits;
  INT bitBufCntBits;
  INT validBits;

} CCrcRegData;

/**
 *  CRC info structure.
 */
typedef struct {
  CCrcRegData crcRegData[MAX_CRC_REGS]; /*!< Multiple crc region description. */
  const USHORT* pCrcLookup;             /*!< Pointer to lookup table filled in FDK_crcInit(). */

  USHORT crcPoly;    /*!< CRC generator polynom. */
  USHORT crcMask;    /*!< CRC mask. */
  USHORT startValue; /*!< CRC start value. */
  UCHAR crcLen;      /*!< CRC length. */

  UINT regStart; /*!< Start region marker for synchronization. */
  UINT regStop;  /*!< Stop region marker for synchronization. */

  USHORT crcValue; /*!< Crc value to be calculated. */

} FDK_CRCINFO;

/**
 *  CRC info handle.
 */
typedef FDK_CRCINFO* HANDLE_FDK_CRCINFO;

/**
 * \brief  Initialize CRC structure.
 *
 * The function initializes existing crc info structure with denoted configuration.
 *
 * \param hCrcInfo              Pointer to an outlying allocated crc info structure.
 * \param crcPoly               Configure crc polynom.
 * \param crcStartValue         Configure crc start value.
 * \param crcLen                Configure crc length.
 *
 * \return  none
 */
void FDKcrcInit(HANDLE_FDK_CRCINFO hCrcInfo, const UINT crcPoly, const UINT crcStartValue,
                const UINT crcLen);

/**
 * \brief  Reset CRC info structure.
 *
 * This function clears all intern states of the crc structure.
 *
 * \param hCrcInfo              Pointer to crc info stucture.
 *
 * \return  none
 */
void FDKcrcReset(HANDLE_FDK_CRCINFO hCrcInfo);

/**
 * \brief  Start CRC region with maximum number of bits.
 *
 * This function marks position in bitstream to be used as start point for crc calculation.
 * Bitstream range for crc calculation can be limited or kept dynamic depending on mBits parameter.
 * The crc region has to be terminated with FDKcrcEndReg() in each case.
 *
 * \param hCrcInfo              Pointer to crc info stucture.
 * \param hBs                   Pointer to current bit buffer structure.
 * \param mBits                 Number of bits in crc region to be calculated.
 *                              - mBits > 0: Zero padding will be used for CRC calculation, if there
 *                                           are less than mBits bits available.
 *                              - mBits < 0: No zero padding is done.
 *                              - mBits = 0: The number of bits used in crc calculation is
 * dynamically, depending on bitstream position between FDKcrcStartReg() and FDKcrcEndReg() call.
 *
 * \return  ID for the created region, -1 in case of an error
 */
INT FDKcrcStartReg(HANDLE_FDK_CRCINFO hCrcInfo, const HANDLE_FDK_BITSTREAM hBs, const INT mBits);

/**
 * \brief  Ends CRC region.
 *
 * This function terminates crc region specified with FDKcrcStartReg(). The number of bits in crc
 * region depends on mBits parameter of FDKcrcStartReg(). This function calculates and updates crc
 * in info structure.
 *
 * \param hCrcInfo              Pointer to crc info stucture.
 * \param hBs                   Pointer to current bit buffer structure.
 * \param reg                   Crc region ID created in FDKcrcStartReg().
 *
 * \return  0 on success
 */
INT FDKcrcEndReg(HANDLE_FDK_CRCINFO hCrcInfo, const HANDLE_FDK_BITSTREAM hBs, const INT reg);

/**
 * \brief  This function returns crc value from info struct.
 *
 * \param hCrcInfo              Pointer to crc info stucture.
 *
 * \return  CRC value masked with crc length.
 */
USHORT FDKcrcGetCRC(const HANDLE_FDK_CRCINFO hCrcInfo);

#endif /* FDK_CRC_H */
