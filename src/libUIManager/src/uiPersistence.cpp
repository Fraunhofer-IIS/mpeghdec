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

   Description: MPEG-H UI persistence handling

*******************************************************************************/

#include "uiPersistence.h"

#include "uiManagerInternal.h"
#include "FDK_crc.h"

#define AVG_CMDS_PER_KEY 3 /* average number of commands per key */

#define INVALID_COMMAND 0xFF
#define INVALID_INDEX 0xFFFF

#define VERSION_ID 0x0101

#define FLAG_BEFORE_LANGUAGE_CHANGE 0x01
#define FLAG_BEFORE_ACCESSIBILITY_CHANGE 0x02

/* key (content ID data) */
typedef struct {
  UCHAR uuid[16];
  USHORT firstCmdIdx;
  USHORT nextKeyIdx;
} UI_PERSISTENCE_KEY;

/* UI command */
typedef struct {
  UCHAR type;
  UCHAR id;
  SHORT param;
  USHORT nextCmdIdx;
  USHORT flags;
} UI_PERSISTENCE_COMMAND;

/* persistence manager */
struct UI_PERSISTENCE_MANAGER {
  void* memory;
  USHORT memSize;

  USHORT maxKeys;
  UI_PERSISTENCE_KEY* keys;

  USHORT maxCommands;
  UI_PERSISTENCE_COMMAND* commands;

  USHORT firstKeyIdx;
  USHORT nextGetCmdIdx;
  UCHAR nextGetCmdFlags;

  UCHAR accessibilitySetting;
  char preferredLanguage[3];
};

/* delete commands */
static void deleteCommands(HANDLE_UI_PERSISTENCE_MANAGER hPersistence, USHORT firstCmdIdx) {
  USHORT cmdIdx = firstCmdIdx;

  while (cmdIdx != INVALID_INDEX) {
    hPersistence->commands[cmdIdx].type = INVALID_COMMAND;
    cmdIdx = hPersistence->commands[cmdIdx].nextCmdIdx;
  }
}

/* add flag for all existing commands */
static void addCommandFlag(HANDLE_UI_PERSISTENCE_MANAGER hPersistence, UCHAR flag) {
  USHORT i;

  for (i = 0; i < hPersistence->maxCommands; i++) {
    hPersistence->commands[i].flags |= flag;
  }
}

/* get key (for reading find existing, for writing find empty or delete oldest) */
static USHORT getKey(HANDLE_UI_PERSISTENCE_MANAGER hPersistence, const UCHAR* uuid,
                     UCHAR forWriting) {
  UI_PERSISTENCE_KEY* keys = hPersistence->keys;
  USHORT keyIdx = hPersistence->firstKeyIdx, prevKeyIdx = INVALID_INDEX;

  /* loop over key entries */
  while (keyIdx != INVALID_INDEX) {
    /* stop on empty key */
    if (keys[keyIdx].firstCmdIdx == INVALID_INDEX) {
      if (forWriting) {
        break;
      } else {
        return INVALID_INDEX;
      }
    }

    /* stop if existing key found */
    if (FDKmemcmp(keys[keyIdx].uuid, uuid, 16) == 0) break;

    if (forWriting) {
      /* if this is the last key, delete it */
      if (keys[keyIdx].nextKeyIdx == INVALID_INDEX) {
        /* delete commands */
        deleteCommands(hPersistence, keys[keyIdx].firstCmdIdx);
        keys[keyIdx].firstCmdIdx = INVALID_INDEX;

        break;
      }
    }

    /* go to next key */
    prevKeyIdx = keyIdx;
    keyIdx = keys[keyIdx].nextKeyIdx;
  }

  /* move key to front */
  if ((keyIdx != INVALID_INDEX) && (prevKeyIdx != INVALID_INDEX)) {
    keys[prevKeyIdx].nextKeyIdx = keys[keyIdx].nextKeyIdx;
    keys[keyIdx].nextKeyIdx = hPersistence->firstKeyIdx;
    hPersistence->firstKeyIdx = keyIdx;
  }

  return keyIdx;
}

/* delete key */
static void deleteKey(HANDLE_UI_PERSISTENCE_MANAGER hPersistence, const UCHAR* uuid) {
  UI_PERSISTENCE_KEY* keys = hPersistence->keys;
  USHORT keyIdx = hPersistence->firstKeyIdx, prevKeyIdx = INVALID_INDEX;

  /* find key */
  while (keyIdx != INVALID_INDEX) {
    /* stop on empty key */
    if (keys[keyIdx].firstCmdIdx == INVALID_INDEX) return;

    /* stop if key found */
    if (FDKmemcmp(keys[keyIdx].uuid, uuid, 16) == 0) break;

    /* go to next key */
    prevKeyIdx = keyIdx;
    keyIdx = keys[keyIdx].nextKeyIdx;
  }

  /* delete key */
  if (keyIdx != INVALID_INDEX) {
    deleteCommands(hPersistence, keys[keyIdx].firstCmdIdx);
    keys[keyIdx].firstCmdIdx = INVALID_INDEX;

    /* move key to end */
    if (keys[keyIdx].nextKeyIdx != INVALID_INDEX) {
      if (prevKeyIdx == INVALID_INDEX) {
        hPersistence->firstKeyIdx = keys[keyIdx].nextKeyIdx;
      } else {
        keys[prevKeyIdx].nextKeyIdx = keys[keyIdx].nextKeyIdx;
      }

      prevKeyIdx = keys[keyIdx].nextKeyIdx;
      while (keys[prevKeyIdx].nextKeyIdx != INVALID_INDEX) prevKeyIdx = keys[prevKeyIdx].nextKeyIdx;
      keys[prevKeyIdx].nextKeyIdx = keyIdx;

      keys[keyIdx].nextKeyIdx = INVALID_INDEX;
    }
  }
}

/* delete oldest key */
static USHORT deleteOldestKey(HANDLE_UI_PERSISTENCE_MANAGER hPersistence) {
  UI_PERSISTENCE_KEY* keys = hPersistence->keys;
  USHORT keyIdx = hPersistence->firstKeyIdx;

  /* find last key */
  while (keyIdx != INVALID_INDEX) {
    /* stop on last valid key */
    if (keys[keyIdx].nextKeyIdx == INVALID_INDEX) break;
    if (keys[keys[keyIdx].nextKeyIdx].firstCmdIdx == INVALID_INDEX) break;

    /* go to next key */
    keyIdx = keys[keyIdx].nextKeyIdx;
  }

  /* delete commands */
  deleteCommands(hPersistence, keys[keyIdx].firstCmdIdx);
  keys[keyIdx].firstCmdIdx = INVALID_INDEX;

  return keyIdx;
}

/* get free command entry */
static USHORT getFreeCmd(HANDLE_UI_PERSISTENCE_MANAGER hPersistence) {
  /* find free command entry */
  for (int i = 0; i < hPersistence->maxCommands; i++) {
    if (hPersistence->commands[i].type == INVALID_COMMAND) {
      return i;
    }
  }

  /* if no free entry found, delete old key and try again */
  deleteOldestKey(hPersistence);
  return getFreeCmd(hPersistence);
}

/* calc memory block CRC */
static USHORT calcCRC(void* mem, USHORT size) {
  FDK_CRCINFO crcInfo;
  FDK_BITSTREAM bs;
  INT reg;

  FDKinitBitStream(&bs, (UCHAR*)mem, MAX_BUFSIZE_BYTES, (UINT)size << 3);
  FDKcrcInit(&crcInfo, 0x8021, 0xFFFF, 16);
  reg = FDKcrcStartReg(&crcInfo, &bs, 0);
  FDKpushFor(&bs, (UINT)size << 3);
  FDKcrcEndReg(&crcInfo, &bs, reg);

  return FDKcrcGetCRC(&crcInfo);
}

/* update memory */
static void updateMemory(HANDLE_UI_PERSISTENCE_MANAGER hPersistence) {
  USHORT *pVersionID, *pFirstKeyIdx, *pCRC;
  UCHAR* pPreferences;
  USHORT* p = (USHORT*)hPersistence->memory;

  if (!hPersistence->memory) return;

  /* 16 bits at end for CRC */
  pCRC = p + hPersistence->memSize / sizeof(USHORT) - 1;

  /* 16 bits for version ID */
  pVersionID = p;
  p++;

  /* 16 bits for first key index */
  pFirstKeyIdx = p;
  p++;

  /* preferred settings */
  pPreferences = (UCHAR*)p;
  p += 4 / sizeof(USHORT);

  /* update */
  *pVersionID = VERSION_ID;
  *pFirstKeyIdx = hPersistence->firstKeyIdx;
  pPreferences[0] = hPersistence->accessibilitySetting;
  pPreferences[1] = (UCHAR)hPersistence->preferredLanguage[0];
  pPreferences[2] = (UCHAR)hPersistence->preferredLanguage[1];
  pPreferences[3] = (UCHAR)hPersistence->preferredLanguage[2];

  *pCRC = calcCRC(hPersistence->memory, hPersistence->memSize - sizeof(USHORT));
}

/* create instance */
HANDLE_UI_PERSISTENCE_MANAGER persistenceManagerCreate() {
  /* create persistence manager instance */
  HANDLE_UI_PERSISTENCE_MANAGER hPersistence =
      (HANDLE_UI_PERSISTENCE_MANAGER)FDKcalloc(1, sizeof(UI_PERSISTENCE_MANAGER));

  return hPersistence;
}

/* delete instance */
void persistenceManagerDelete(HANDLE_UI_PERSISTENCE_MANAGER hPersistence) {
  FDKfree(hPersistence);
}

/* set memory */
INT persistenceManagerSetMemory(HANDLE_UI_PERSISTENCE_MANAGER hPersistence, void* persistenceMemory,
                                USHORT persistenceMemorySize) {
  USHORT* p = (USHORT*)persistenceMemory;
  USHORT left;
  USHORT *pVersionID, *pFirstKeyIdx, *pCRC;
  UCHAR* pPreferences;
  INT ret = 0;

  /* finalize current persistence memory block */
  updateMemory(hPersistence);

  /* check required 16 bit alignment */
  if ((size_t)persistenceMemory & 1) return -2;

  if (persistenceMemorySize & 1) persistenceMemorySize--;

  left = persistenceMemorySize;

  /* if no memory passed, disable persistency */
  if (!persistenceMemory || !persistenceMemorySize) {
    hPersistence->memory = NULL;
    hPersistence->memSize = 0;

    return 0;
  }

  /* if same memory block passed again, do nothing */
  if ((persistenceMemory == hPersistence->memory) &&
      (persistenceMemorySize == hPersistence->memSize))
    return 1;

  /* 16 bits at end for CRC */
  pCRC = p + left / sizeof(USHORT) - 1;
  left -= sizeof(USHORT);

  /* 16 bits for version ID */
  pVersionID = p;
  p++;
  left -= sizeof(USHORT);

  /* 16 bits for first key index */
  pFirstKeyIdx = p;
  p++;
  left -= sizeof(USHORT);

  /* preferred settings */
  pPreferences = (UCHAR*)p;
  p += 4 / sizeof(USHORT);
  left -= 4;

  /* assign memory to keys */
  hPersistence->maxKeys =
      left / (sizeof(UI_PERSISTENCE_KEY) + AVG_CMDS_PER_KEY * sizeof(UI_PERSISTENCE_COMMAND));
  hPersistence->keys = (UI_PERSISTENCE_KEY*)p;
  p += hPersistence->maxKeys * sizeof(UI_PERSISTENCE_KEY) / sizeof(USHORT);
  left -= hPersistence->maxKeys * sizeof(UI_PERSISTENCE_KEY);

  /* assign remaining memory to commands */
  hPersistence->maxCommands = left / sizeof(UI_PERSISTENCE_COMMAND);
  hPersistence->commands = (UI_PERSISTENCE_COMMAND*)p;

  /* check memory is not too small */
  if ((hPersistence->maxKeys < 10) || (hPersistence->maxCommands < 30)) {
    hPersistence->memory = NULL;
    return -1;
  }

  /* init */
  if ((*pVersionID != VERSION_ID) ||
      (*pCRC != calcCRC(persistenceMemory, persistenceMemorySize - sizeof(USHORT)))) {
    /* clear memory */
    FDKmemset(persistenceMemory, 0xFF, persistenceMemorySize);

    /* init keys */
    hPersistence->firstKeyIdx = 0;

    for (USHORT i = 0; i < hPersistence->maxKeys; i++) {
      hPersistence->keys[i].nextKeyIdx = i + 1;
    }
    hPersistence->keys[hPersistence->maxKeys - 1].nextKeyIdx = INVALID_INDEX;

    ret = 0;
  } else {
    /* restore first key index */
    hPersistence->firstKeyIdx = *pFirstKeyIdx;

    /* restore preferred settings */
    hPersistence->accessibilitySetting = pPreferences[0];
    hPersistence->preferredLanguage[0] = (char)pPreferences[1];
    hPersistence->preferredLanguage[1] = (char)pPreferences[2];
    hPersistence->preferredLanguage[2] = (char)pPreferences[3];

    ret = 1;
  }

  hPersistence->memory = persistenceMemory;
  hPersistence->memSize = persistenceMemorySize;
  hPersistence->nextGetCmdIdx = INVALID_INDEX;

  return ret;
}

/* get memory */
void persistenceManagerGetMemory(HANDLE_UI_PERSISTENCE_MANAGER hPersistence,
                                 void** persistenceMemory, USHORT* persistenceMemorySize) {
  updateMemory(hPersistence);

  if (persistenceMemory) *persistenceMemory = hPersistence->memory;
  if (persistenceMemorySize) *persistenceMemorySize = hPersistence->memSize;
}

/* save command */
void persistenceManagerSaveCommand(HANDLE_UI_PERSISTENCE_MANAGER hPersistence,
                                   const UI_MANAGER_ACTION* uiAction) {
  UI_PERSISTENCE_COMMAND command;
  USHORT keyIdx, cmdIdx, lastCmdIdx;

  /* check if active */
  if (!hPersistence->memory) return;

  /* init command data */
  command.type = uiAction->actionType;
  command.id = 0;
  command.param = 0;
  command.nextCmdIdx = INVALID_INDEX;
  command.flags = 0;

  /* handle UI actions */
  switch (uiAction->actionType) {
    case UI_MANAGER_COMMAND_RESET:
      deleteKey(hPersistence, uiAction->uuid);
      return;

    case UI_MANAGER_COMMAND_PRESET_SELECTED:
      deleteKey(hPersistence, uiAction->uuid);
      command.id = (UCHAR)uiAction->paramInt;
      break;

    case UI_MANAGER_COMMAND_AUDIO_ELEMENT_MUTING_CHANGED:
    case UI_MANAGER_COMMAND_AUDIO_ELEMENT_SWITCH_MUTING_CHANGED:
      command.id = (UCHAR)uiAction->paramInt;
      command.param = uiAction->paramBool;
      break;

    case UI_MANAGER_COMMAND_AUDIO_ELEMENT_BALANCE_CHANGED:
    case UI_MANAGER_COMMAND_AUDIO_ELEMENT_AZIMUTH_CHANGED:
    case UI_MANAGER_COMMAND_AUDIO_ELEMENT_ELEVATION_CHANGED:
    case UI_MANAGER_COMMAND_AUDIO_ELEMENT_SWITCH_BALANCE_CHANGED:
    case UI_MANAGER_COMMAND_AUDIO_ELEMENT_SWITCH_AZIMUTH_CHANGED:
    case UI_MANAGER_COMMAND_AUDIO_ELEMENT_SWITCH_ELEVATION_CHANGED:
      command.id = (UCHAR)uiAction->paramInt;
      command.param = (SHORT)(LONG)(uiAction->paramFloat >> 15);
      break;

    case UI_MANAGER_COMMAND_AUDIO_ELEMENT_SWITCH_SELECTED:
      command.id = (UCHAR)uiAction->paramInt;
      command.param = (SHORT)(LONG)(uiAction->paramFloat >> 16);
      break;

    case UI_MANAGER_COMMAND_ACCESSIBILITY_PREFERENCE:
      if (uiAction->paramInt != hPersistence->accessibilitySetting) {
        hPersistence->accessibilitySetting = (UCHAR)uiAction->paramInt;
        addCommandFlag(hPersistence, FLAG_BEFORE_ACCESSIBILITY_CHANGE);
      }
      return;

    case UI_MANAGER_COMMAND_AUDIO_LANGUAGE_SELECTED:
      if ((uiAction->paramText[0] != hPersistence->preferredLanguage[0]) ||
          (uiAction->paramText[1] != hPersistence->preferredLanguage[1]) ||
          (uiAction->paramText[2] != hPersistence->preferredLanguage[2])) {
        hPersistence->preferredLanguage[0] = uiAction->paramText[0];
        hPersistence->preferredLanguage[1] = uiAction->paramText[1];
        hPersistence->preferredLanguage[2] = uiAction->paramText[2];
        addCommandFlag(hPersistence, FLAG_BEFORE_LANGUAGE_CHANGE);
      }
      return;

    default:
      return;
  }

  /* get key */
  keyIdx = getKey(hPersistence, uiAction->uuid, 1);

  FDKmemcpy(hPersistence->keys[keyIdx].uuid, uiAction->uuid, 16);

  /* get command index */
  cmdIdx = hPersistence->keys[keyIdx].firstCmdIdx;
  lastCmdIdx = cmdIdx;

  /* check for existing command to replace */
  while (cmdIdx != INVALID_INDEX) {
    UI_PERSISTENCE_COMMAND* cmd2 = &(hPersistence->commands[cmdIdx]);

    /* if command of same type with same ID exists, replace it */
    if ((cmd2->type == command.type) && (cmd2->id == command.id)) {
      command.nextCmdIdx = cmd2->nextCmdIdx;
      break;
    }

    lastCmdIdx = cmdIdx;
    cmdIdx = cmd2->nextCmdIdx;
  }

  /* if no existing command to replace, add new command*/
  if (cmdIdx == INVALID_INDEX) {
    cmdIdx = getFreeCmd(hPersistence);

    if ((lastCmdIdx == INVALID_INDEX) ||
        (hPersistence->keys[keyIdx].firstCmdIdx == INVALID_INDEX)) {
      /* this is the first command for this key */
      hPersistence->keys[keyIdx].firstCmdIdx = cmdIdx;
    } else {
      /* set next index for last command of this key */
      hPersistence->commands[lastCmdIdx].nextCmdIdx = cmdIdx;
    }
  }

  /* write command */
  hPersistence->commands[cmdIdx] = command;
}

/* get command (if UUID passed get first command, if NULL passed get next command) */
UCHAR persistenceManagerGetCommand(HANDLE_UI_PERSISTENCE_MANAGER hPersistence, const UCHAR* uuid,
                                   UI_MANAGER_ACTION* uiAction) {
  USHORT keyIdx, cmdIdx;
  UCHAR flags, flagsDiff;
  UI_PERSISTENCE_COMMAND* command;

  /* check if active */
  if (!hPersistence->memory) return 0;

  /* get command index */
  if (uuid) {
    /* get key index */
    keyIdx = getKey(hPersistence, uuid, 0);
    if (keyIdx == INVALID_INDEX) {
      hPersistence->nextGetCmdIdx = INVALID_INDEX;
      return 0;
    }

    /* first command index */
    cmdIdx = hPersistence->keys[keyIdx].firstCmdIdx;
    flags = FLAG_BEFORE_ACCESSIBILITY_CHANGE | FLAG_BEFORE_LANGUAGE_CHANGE;
  } else {
    /* next command index */
    keyIdx = hPersistence->firstKeyIdx;
    cmdIdx = hPersistence->nextGetCmdIdx;
    flags = hPersistence->nextGetCmdFlags;
  }

  /* find next command */
  flagsDiff = flags;
  if (cmdIdx == INVALID_INDEX) {
    if (flags == 0) return 0;

    flags--;
    cmdIdx = hPersistence->keys[keyIdx].firstCmdIdx;
  }
  while (hPersistence->commands[cmdIdx].flags != flags) {
    cmdIdx = hPersistence->commands[cmdIdx].nextCmdIdx;
    if (cmdIdx == INVALID_INDEX) {
      if (flags == 0) break;

      flags--;
      cmdIdx = hPersistence->keys[keyIdx].firstCmdIdx;
    }
  }
  flagsDiff ^= flags;

  /* send commands to apply automatic accessibility/language selection */
  if (!uuid) {
    if (flagsDiff & FLAG_BEFORE_ACCESSIBILITY_CHANGE) {
      uiAction->actionType = UI_MANAGER_COMMAND_ACCESSIBILITY_PREFERENCE;
      uiAction->presentFlags = 0;

      hPersistence->nextGetCmdIdx = cmdIdx;
      hPersistence->nextGetCmdFlags = flags /* | (flagsDiff & FLAG_BEFORE_LANGUAGE_CHANGE)*/;

      return 1;
    }
    if (flagsDiff & FLAG_BEFORE_LANGUAGE_CHANGE) {
      uiAction->actionType = UI_MANAGER_COMMAND_AUDIO_LANGUAGE_SELECTED;
      uiAction->presentFlags = 0;

      hPersistence->nextGetCmdIdx = cmdIdx;
      hPersistence->nextGetCmdFlags = flags;

      return 1;
    }
  }

  /* check if no more commands */
  if (cmdIdx == INVALID_INDEX) {
    hPersistence->nextGetCmdIdx = INVALID_INDEX;
    hPersistence->nextGetCmdFlags = 0;
    return 0;
  }

  /* restore UI action */
  command = &(hPersistence->commands[cmdIdx]);

  FDKmemcpy(uiAction->uuid, hPersistence->keys[keyIdx].uuid, 16);
  uiAction->actionType = command->type;
  uiAction->presentFlags = 0;

  switch (command->type) {
    case UI_MANAGER_COMMAND_PRESET_SELECTED:
      uiAction->paramInt = (LONG)command->id;
      uiAction->presentFlags |= FLAG_XML_PARAM_INT;
      break;

    case UI_MANAGER_COMMAND_AUDIO_ELEMENT_MUTING_CHANGED:
    case UI_MANAGER_COMMAND_AUDIO_ELEMENT_SWITCH_MUTING_CHANGED:
      uiAction->paramInt = (LONG)command->id;
      uiAction->paramBool = (UCHAR)command->param;
      uiAction->presentFlags |= FLAG_XML_PARAM_INT | FLAG_XML_PARAM_BOOL;
      break;

    case UI_MANAGER_COMMAND_AUDIO_ELEMENT_BALANCE_CHANGED:
    case UI_MANAGER_COMMAND_AUDIO_ELEMENT_AZIMUTH_CHANGED:
    case UI_MANAGER_COMMAND_AUDIO_ELEMENT_ELEVATION_CHANGED:
    case UI_MANAGER_COMMAND_AUDIO_ELEMENT_SWITCH_BALANCE_CHANGED:
    case UI_MANAGER_COMMAND_AUDIO_ELEMENT_SWITCH_AZIMUTH_CHANGED:
    case UI_MANAGER_COMMAND_AUDIO_ELEMENT_SWITCH_ELEVATION_CHANGED:
      uiAction->paramInt = (LONG)command->id;
      uiAction->paramFloat = (FIXP_DBL)((LONG)command->param << 15);
      uiAction->presentFlags |= FLAG_XML_PARAM_INT | FLAG_XML_PARAM_FLOAT;
      break;

    case UI_MANAGER_COMMAND_AUDIO_ELEMENT_SWITCH_SELECTED:
      uiAction->paramInt = (LONG)command->id;
      uiAction->paramFloat = (FIXP_DBL)((LONG)command->param << 16);
      uiAction->presentFlags |= FLAG_XML_PARAM_INT | FLAG_XML_PARAM_FLOAT;
      break;
  }

  /* save next command index */
  hPersistence->nextGetCmdIdx = command->nextCmdIdx;
  hPersistence->nextGetCmdFlags = flags;

  return 1;
}
