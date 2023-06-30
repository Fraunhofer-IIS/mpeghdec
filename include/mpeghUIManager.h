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

/**************************************  ***************************************
   Author(s): Matthias Neusinger
   Description: MPEG-H UI Manager

   This software and/or program is protected by copyright law and international
   treaties. Any reproduction or distribution of this software and/or program,
   or any portion of it, may result in severe civil and criminal penalties, and
   will be prosecuted to the maximum extent possible under law.

*******************************************************************************/

#ifndef MPEGHUIMANAGER_H
#define MPEGHUIMANAGER_H

/**
 * \file   mpeghUIManager.h
 * \brief  MPEG-H UI Manager library interface header file.
 */

#include "machine_type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MPEGH_UI_MANAGER* HANDLE_MPEGH_UI_MANAGER;

#define MAX_NUMBER_SECONDARY_STREAMS 16

typedef enum {
  MPEGH_UI_OK = 0,
  MPEGH_UI_OUT_OF_MEMORY,
  MPEGH_UI_BUFFER_TOO_SMALL, /* This error is not recoverable until a new RAP frame is processed,
                                   i.e. UI events will be delayed. */
  MPEGH_UI_INVALID_PARAM,
  MPEGH_UI_NOT_ALLOWED,
  MPEGH_UI_INVALID_STATE,
  MPEGH_UI_PARSE_ERROR,
  MPEGH_UI_OK_BUT_NO_VALID_DATA
} MPEGH_UI_ERROR;

/*!
  \brief
  Input flag for mpegh_UI_GetXmlSceneState(): Force output of XML
  description, even if nothing has changed since the last call.
*/
#define MPEGH_UI_FORCE_UPDATE 1
/*!
  \brief
  Input flag for mpegh_UI_GetXmlSceneState(): Force restart of
  XML output if incomplete output was returned by previous call.
*/
#define MPEGH_UI_FORCE_RESTART_XML 4

/*!
  \brief
  Flag returned by mpegh_UI_GetXmlSceneState(): Nothing has
  changed since the last call, no XML output was generated.
*/
#define MPEGH_UI_NO_CHANGE 1
/*!
  \brief
  Flag returned by mpegh_UI_GetXmlSceneState(): XML output
  is a continuation of incomplete output of the previous call.
*/
#define MPEGH_UI_CONTINUES_XML 2
/*!
  \brief
  Flag returned by mpegh_UI_GetXmlSceneState(): XML output
  is not complete, at least one further call of the function is
  required to get the complete XML output.
*/
#define MPEGH_UI_INCOMPLETE_XML 4
/*!
  \brief
  Flag returned by mpegh_UI_GetXmlSceneState(): only minimal
  XML output was generated, a further call of the function will
  return the full XML scene description.
*/
#define MPEGH_UI_SHORT_OUTPUT 8

/**
 * @brief  Open a mpegh UI manager instance
 *
 * @return  UI manager handle
 */
LINKSPEC_H HANDLE_MPEGH_UI_MANAGER mpegh_UI_Manager_Open();

/**
 * @brief  De-allocate all resources of a mpegh UI manager instance.
 *
 * @param[in,out] self  UI manager handle.
 * @return              void
 */
LINKSPEC_H void mpegh_UI_Manager_Close(HANDLE_MPEGH_UI_MANAGER self);

/**
 * @brief  Get XML description of audio scene and available user interactivity parameters
 *
 * @param[in]  self        UI manager handle.
 * @param[out] xmlOut      Pointer to external output buffer where the XML string will be stored.
 * @param[in]  xmlOutSize  Size of the output buffer (number of chars). Has to be at least 1024 to
 *                         ensure success of the function.
 * @param[in]  flagsIn     Bit field with flags for the UI manager: \n
 *                         (flags & ::MPEGH_UI_FORCE_UPDATE) != 0: Force output of XML
 *                         description even if nothing has changed \n
 *                         (flags & ::MPEGH_UI_FORCE_RESTART_XML) != 0: Restart XML output from
 *                         the beginning (if incomplete output was returned by previous call).
 * @param[out]  flagsOut   Pointer to bit field with flags returned by the UI manager: \n
 *                         (flags & ::MPEGH_UI_NO_CHANGE) != 0: Nothing has changed since the
 *                         previous call of this function, no XML output was written. \n
 *                         (flags & ::MPEGH_UI_CONTINUES_XML) != 0: The XML output is a
 *                         continuation of incomplete output of the previous call and has to be
 *                         appended to the output of the previous call. \n
 *                         (flags & ::MPEGH_UI_INCOMPLETE_XML) != 0: The XML output is not yet
 *                         complete, at least one further call of the function is required to get
 *                         the remaining part.
 * @return                 Error code.
 */
LINKSPEC_H MPEGH_UI_ERROR mpegh_UI_GetXmlSceneState(HANDLE_MPEGH_UI_MANAGER self, char* xmlOut,
                                                    UINT xmlOutSize, UINT flagsIn, UINT* flagsOut);

/**
 * @brief  Send XML description of UI command to the UI manager
 *
 * @param[in]  self       UI manager handle.
 * @param[in]  xmlIn      Pointer to a buffer containing the XML command string.
 * @param[in]  xmlInSize  Size of the XML string (number of chars).
 * @param[out] flagsOut   Pointer to bit field with flags returned by the UI manager, currently not
 *                        used.
 * @return                Error code.
 */
LINKSPEC_H MPEGH_UI_ERROR mpegh_UI_ApplyXmlAction(HANDLE_MPEGH_UI_MANAGER self, const char* xmlIn,
                                                  UINT xmlInSize, UINT* flagsOut);

/**
 * @brief  Feed MHAS input into UI manager
 *
 * @param[in] self        UI manager handle.
 * @param[in] mhasBuffer  Buffer containing all MHAS packets for exactly one audio frame.
 * @param[in] mhasLength  Size of MHAS packets in bytes.
 * @return                Error code.
 */
LINKSPEC_H MPEGH_UI_ERROR mpegh_UI_FeedMHAS(HANDLE_MPEGH_UI_MANAGER self, UCHAR* mhasBuffer,
                                            UINT mhasLength);

/**
 * @brief  Get updated MHAS output from UI manager
 *
 * @param[in]     self              UI manager handle.
 * @param[in,out] mhasBuffer        Buffer receiving updated MHAS packets (including UI and DRC
 *                                  packets).
 * @param[in]     mhasBufferLength  Size of buffer in bytes. This buffer must be large enough to
 *                                  also include UI MHAS packets. If the buffer is not large
 *                                  enough, pending UI packets will be delayed until the next RAP
 *                                  frame.
 * @param[out]    mhasLength        Pointer to variable receiving size of updated MHAS packets in
 *                                  bytes.
 * @return                          Error code.
 */
LINKSPEC_H MPEGH_UI_ERROR mpegh_UI_UpdateMHAS(HANDLE_MPEGH_UI_MANAGER self, UCHAR* mhasBuffer,
                                              UINT mhasBufferLength, UINT* mhasLength);

/**
 * @brief  Set memory block for storing UI persistency data (activates internal persistence
 *         handling)
 *
 * @param[in] self                    UI manager handle.
 * @param[in] persistenceMemoryBlock  Pointer to memory block used by UI manager for saving
 *                                    persistency data. The caller is responsible for allocating
 *                                    the memory. The start address of the memory block has to be
 *                                    16 bit aligned (i.e. a multiple of 2), otherwise the function
 *                                    fails and returns an error. The memory block must not be
 *                                    modified or deallocated before the UI manager is closed or a
 *                                    different memory block is set by a further call of this
 *                                    function. The memory block can be empty or filled with saved
 *                                    content of a previous session. If the content of the memory
 *                                    block is saved for later use, a call of
 *                                    mpegh_UI_GetPersistenceMemory() is required directly
 *                                    before saving the data. This parameter can be NULL for
 *                                    deactivating internal persistency handling.
 * @param[in] persistenceMemorySize   Size of the memory block in bytes. The minimum recommended
 *                                    size is 512 bytes. If the size is too small, the function
 *                                    fails and returns an error.
 * @return                            Error code.
 */
LINKSPEC_H MPEGH_UI_ERROR mpegh_UI_SetPersistenceMemory(HANDLE_MPEGH_UI_MANAGER self,
                                                        void* persistenceMemoryBlock,
                                                        USHORT persistenceMemorySize);

/**
 * @brief  Get current UI persistency memory block (updates content, call is required before
 *         storing persistence memory for later reuse!)
 *
 * @param[in]     self                    UI manager handle.
 * @param[in,out] persistenceMemoryBlock  Pointer to a pointer receiving the address of the
 *                                        currently active memory block used by the UI manager for
 *                                        saving persistency data. After the call the content of
 *                                        the memory block is ready for storing it for late reuse.
 *                                        A NULL pointer is returned if internal persistency
 *                                        handling has not been activated.
 * @param[out]    persistenceMemorySize   Pointer to a variable receiving the size of the current
 *                                        memory block in bytes.
 * @return                                Error code.
 */
LINKSPEC_H MPEGH_UI_ERROR mpegh_UI_GetPersistenceMemory(HANDLE_MPEGH_UI_MANAGER self,
                                                        void** persistenceMemoryBlock,
                                                        USHORT* persistenceMemorySize);

#ifdef __cplusplus
}
#endif

#endif
