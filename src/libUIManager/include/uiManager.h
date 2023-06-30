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

/**************** MPEG-H 3DA User Interaction manager library ******************

   Author(s):   Matthias Neusinger

   Description: MPEG-H UI Manager

*******************************************************************************/

#ifndef UIMANAGER_H
#define UIMANAGER_H

#include "tp_data.h"
#include "common_fix.h"

/* input flags */
#define UI_MANAGER_FORCE_UPDATE 1      /* force output of XML, even if nothing changed */
#define UI_MANAGER_FORCE_RESTART_XML 4 /* force restart of partial XML output */

/* output flags */
#define UI_MANAGER_NO_CHANGE 1      /* nothing changed, no XML written */
#define UI_MANAGER_CONTINUES_XML 2  /* partial XML output, continues output of previous call */
#define UI_MANAGER_INCOMPLETE_XML 4 /* XML output is not complete, further call required */
#define UI_MANAGER_SHORT_OUTPUT 8   /* only minimal XML output was written */

#define UI_MANAGER_USE_DEFAULT_TARGET_LOUDNESS -128
#define UI_MANAGER_DRC_OFF 0xFFFFFFFF                  /* -1 */
#define UI_MANAGER_USE_DEFAULT_DRC_SELECTED 0xFFFFFFFE /* -2 */

/* error codes */
typedef enum {
  UI_MANAGER_OK = 0,
  UI_MANAGER_OUT_OF_MEMORY,
  UI_MANAGER_BUFFER_TOO_SMALL,
  UI_MANAGER_INVALID_PARAM,
  UI_MANAGER_NOT_ALLOWED,
  UI_MANAGER_INVALID_STATE,
  UI_MANAGER_OK_BUT_NO_VALID_DATA
} UI_MANAGER_ERROR;

/* UI state data */
struct USER_INTERACTIVITY_GROUP_STATUS {
  UCHAR groupID;
  UCHAR onOff;
  UCHAR routeToWIRE;
  UCHAR routeToWireID;
  UCHAR changePosition;
  UCHAR azOffset;
  UCHAR elOffset;
  UCHAR distFact;
  UCHAR changeGain;
  UCHAR gain;
};

typedef struct USER_INTERACTIVITY_STATUS {
  UCHAR interactionMode;
  UCHAR numGroups;
  UCHAR groupPresetID;

  USER_INTERACTIVITY_GROUP_STATUS groupData[ASI_MAX_GROUPS];
} USER_INTERACTIVITY_STATUS;

typedef struct {
  SCHAR targetLoudness;
  ULONG drcSelected;
  SCHAR albumMode;
  SHORT boost;
  SHORT compress;
} UI_DRC_LOUDNESS_STATUS;

/* instance handle */
struct UI_MANAGER;
typedef struct UI_MANAGER* HANDLE_UI_MANAGER;

INT UI_Manager_Create(HANDLE_UI_MANAGER* phUiManager);

AUDIO_SCENE_INFO* UI_Manager_GetAsiPointer(HANDLE_UI_MANAGER hUiManager);

UI_MANAGER_ERROR UI_Manager_SetIsActive(HANDLE_UI_MANAGER hUiManager, UCHAR isActive);

UI_MANAGER_ERROR UI_Manager_GetXmlSceneState(HANDLE_UI_MANAGER hUiManager, char* xmlOut,
                                             UINT xmlOutSize, UINT flagsIn, UINT* flagsOut);

UI_MANAGER_ERROR UI_Manager_ApplyXmlAction(HANDLE_UI_MANAGER hUiManager, const char* xmlIn,
                                           UINT xmlInSize, UINT* flagsOut);

UI_MANAGER_ERROR UI_Manager_SetUUID(HANDLE_UI_MANAGER hUiManager, UCHAR uuid[16],
                                    UCHAR applyAsiCrc);

UI_MANAGER_ERROR UI_Manager_GetInteractivityStatus(HANDLE_UI_MANAGER hUiManager,
                                                   USER_INTERACTIVITY_STATUS* pUiStatus,
                                                   UCHAR* pSignalChanged);

UI_MANAGER_ERROR UI_Manager_GetDrcLoudnessStatus(HANDLE_UI_MANAGER hUiManager,
                                                 UI_DRC_LOUDNESS_STATUS* pDrcLoudnessStatus);

UI_MANAGER_ERROR UI_Manager_GetStatusChanged(HANDLE_UI_MANAGER hUiManager,
                                             UCHAR* pInteractivityStatusChanged,
                                             UCHAR* pDrcLoudnessStatusChanged);

UI_MANAGER_ERROR UI_Manager_SetPersistenceMemory(HANDLE_UI_MANAGER hUiManager,
                                                 void* persistenceMemoryBlock,
                                                 USHORT persistenceMemorySize);

UI_MANAGER_ERROR UI_Manager_GetPersistenceMemory(HANDLE_UI_MANAGER hUiManager,
                                                 void** persistenceMemoryBlock,
                                                 USHORT* persistenceMemorySize);

INT UI_Manager_Delete(HANDLE_UI_MANAGER* phUiManager);

#endif
