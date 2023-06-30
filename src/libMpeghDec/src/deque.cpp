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

   Author(s):

   Description:

*******************************************************************************/

#include "deque.h"
#include "genericStds.h"

int deque_alloc(deque* q, unsigned int length, unsigned int block_size) {
  q->data = (void*)FDKcalloc(length, block_size);
  if (q->data == NULL) {
    return -1;
  }

  q->block_size = block_size;
  q->first = 0;
  q->last = 0;
  q->size = 0;
  q->max = length;
  q->full = 0;
  return 0;
}

void deque_free(deque* q) {
  if (q->data != NULL) {
    FDKfree(q->data);
    q->data = NULL;
  }
  deque_clear(q);
}

int deque_push_back(deque* q, void* data) {
  if (q->full) {
    return -1;
  }

  FDKmemcpy((unsigned char*)q->data + q->last * q->block_size, data, q->block_size);
  q->last = (q->last + 1) % q->max;
  q->size++;
  if (q->size == q->max) {
    q->full = true;
  }
  return 0;
}

int deque_bulk_push_back(deque* q, void* data, unsigned int numData) {
  if (q->full || q->size + numData > q->max) {
    return -1;
  }

  unsigned int space = q->max - q->last;
  unsigned int left = 0;
  unsigned int toCopy = 0;
  if (space > 0) {
    if (numData >= space) {
      toCopy = space;
      left = numData - space;
    } else {
      toCopy = numData;
      left = 0;
    }
    void* writePos = (unsigned char*)q->data + q->last * q->block_size;
    FDKmemcpy(writePos, data, toCopy * q->block_size);
  } else {
    left = numData;
  }

  if (left > 0) {
    unsigned char* tmpData = (unsigned char*)data + (numData - left) * q->block_size;
    ;
    FDKmemcpy((unsigned char*)q->data, tmpData, left * q->block_size);
  }
  q->last = (q->last + numData) % q->max;
  q->size += numData;
  if (q->size == q->max) {
    q->full = true;
  }
  return 0;
}

int deque_bulk_push_back_zeros(deque* q, unsigned int numZeros) {
  if (q->full || q->size + numZeros > q->max) {
    return -1;
  }

  unsigned int space = q->max - q->last;
  unsigned int left = 0;
  unsigned int toCopy = 0;
  if (space > 0) {
    if (numZeros >= space) {
      toCopy = space;
      left = numZeros - space;
    } else {
      toCopy = numZeros;
      left = 0;
    }
    void* writePos = (unsigned char*)q->data + q->last * q->block_size;
    FDKmemset(writePos, 0, toCopy * q->block_size);
  } else {
    left = numZeros;
  }

  if (left > 0) {
    FDKmemset((unsigned char*)q->data, 0, left * q->block_size);
  }
  q->last = (q->last + numZeros) % q->max;
  q->size += numZeros;
  if (q->size == q->max) {
    q->full = true;
  }
  return 0;
}

void* deque_pop_back(deque* q) {
  if (q->size == 0) {
    return NULL;
  }

  unsigned int tmp = q->last;
  if (tmp == 0) {
    tmp = q->size;
  }
  const unsigned int pos = (tmp - 1) % q->max;
  void* data = (unsigned char*)q->data + pos * q->block_size;
  q->last = pos;
  q->size--;
  q->full = false;
  return data;
}

void* deque_pop_front(deque* q) {
  if (q->size == 0) {
    return NULL;
  }

  void* data = (unsigned char*)q->data + q->first * q->block_size;
  q->first = (q->first + 1) % q->max;
  q->size--;
  q->full = false;
  return data;
}

int deque_bulk_pop_front_copy(deque* q, void* destination, unsigned int numData) {
  if (q->size < numData) {
    return -1;
  }

  unsigned int left = numData;
  void* data = (unsigned char*)q->data + q->first * q->block_size;
  void* dest = (unsigned char*)destination;
  if (q->first + numData >= q->max) {
    unsigned int toCopy = q->max - q->first;
    FDKmemcpy(dest, data, toCopy * q->block_size);
    left -= toCopy;
    data = (unsigned char*)q->data;
    dest = (unsigned char*)destination + toCopy * q->block_size;
  }
  FDKmemcpy(dest, data, left * q->block_size);
  q->first = (q->first + numData) % q->max;
  q->size -= numData;
  q->full = false;
  return 0;
}

int deque_bulk_pop_front(deque* q, unsigned int numData) {
  if (q->size < numData) {
    return -1;
  }

  q->first = (q->first + numData) % q->max;
  q->size -= numData;
  q->full = false;
  return 0;
}

bool deque_full(const deque* q) {
  return (q->full) ? true : false;
}

bool deque_empty(const deque* q) {
  return (q->size) ? false : true;
}

unsigned int deque_size(const deque* q) {
  return q->size;
}

unsigned int deque_space(const deque* q) {
  return q->max - q->size;
}

void deque_clear(deque* q) {
  q->first = 0;
  q->last = 0;
  q->size = 0;
  q->full = false;
}

void* deque_at(const deque* q, unsigned int index) {
  if (index >= q->size) {
    return NULL;
  }

  const unsigned int pos = (q->first + index) % q->max;
  void* data = (unsigned char*)q->data + pos * q->block_size;
  return data;
}

void* deque_front(const deque* q) {
  return deque_at(q, 0);
}

void* deque_back(const deque* q) {
  if (q->size == 0) {
    return NULL;
  }
  return deque_at(q, q->size - 1);
}
