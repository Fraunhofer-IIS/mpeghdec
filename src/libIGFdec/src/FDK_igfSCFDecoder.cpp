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

/********************** Intelligent gap filling library ************************

   Author(s):   Youliy Ninov, Florin Ghido, Andreas Niedermeier

   Description: Arithmetic Decoder for IGF scalefactors

*******************************************************************************/

#include "FDK_igfSCFDecoder.h"
#include "FDK_igfDec.h"

#include "FDK_bitstream.h"

#define cbitsnew 16
#define stat_bitsnew 14
#define ari_q4new (((LONG)1 << cbitsnew) - 1) /* 0xFFFF */
#define ari_q1new (ari_q4new / 4 + 1)         /* 0x4000 */
#define ari_q2new (2 * ari_q1new)             /* 0x8000 */
#define ari_q3new (3 * ari_q1new)             /* 0xC000 */

void iisIGFSCFDecLibInit(IGFSCFDEC_PRIVATE_DATA* hPrivateData, INT scfCountLongBlock,
                         INT scfCountShortBlock, INT scfCountTCXBlock) {
  INT size = 0;
  INT scfCountTCX = scfCountTCXBlock;

  size = sizeof(struct igfscfdec_private_data_struct);

  if (scfCountTCX <= 0) {
    scfCountTCX = scfCountShortBlock;
  }

  FDKmemclear(hPrivateData, size);

  /* init ptrs to qBuffers, this avoids allocating each buffer seperate: */
  hPrivateData->prevLB = &hPrivateData->prevBuffer[0];
  hPrivateData->prevSB = hPrivateData->prevLB + 1 * scfCountLongBlock;
  hPrivateData->prevTCXmedium = hPrivateData->prevSB + 1 * scfCountShortBlock;
  hPrivateData->prevTCXlong = hPrivateData->prevTCXmedium + 1 * scfCountTCX;

  hPrivateData->prevDLB = 0;
  hPrivateData->prevDSB = 0;
  hPrivateData->prevDTCXmedium = 0;
  hPrivateData->prevDTCXlong = 0;

  hPrivateData->scfCountLongBlock = scfCountLongBlock;
  hPrivateData->scfCountShortBlock = scfCountShortBlock;
  hPrivateData->scfCountTCXBlock = scfCountTCX;
}

/*************************************************************************************
helper function:
return TRUE if it is the first symbol of the sequence, FALSE otherwise
****************************************************************************************/
static INT arith_first_symbol(IGFSCFDEC_PRIVATE_DATA_HANDLE hPrivateData /**< instance handle */
) {
  if (hPrivateData->arith_decode_first_symbol) {
    hPrivateData->arith_decode_first_symbol = 0;

    return 1;
  } else {
    return 0;
  }
}

/***************** begin generated tables from sfe_tables.c *****************/

static const SHORT cf_se01[27] = {16370, 16360, 16350, 16336, 16326, 16283, 16215, 16065, 15799,
                                  15417, 14875, 13795, 12038, 9704,  6736,  3918,  2054,  1066,
                                  563,   311,   180,   98,    64,    20,    15,    5,     0};

static const SHORT cf_se10[27] = {16218, 16145, 16013, 15754, 15426, 14663, 13563, 11627, 8894,
                                  6220,  4333,  3223,  2680,  2347,  2058,  1887,  1638,  1472,
                                  1306,  1154,  1012,  895,   758,   655,   562,   489,   0};

static const SHORT cf_se02[7][27] = {
    {16332, 16306, 16278, 16242, 16180, 16086, 15936, 15689, 15289, 14657, 13632, 12095, 9926, 6975,
     4213,  2285,  1163,  637,   349,   196,   125,   82,    52,    28,    11,    2,     0},
    {16370, 16367, 16364, 16358, 16350, 16330, 16284, 16170, 16030,
     15647, 14840, 13094, 10364, 6833,  3742,  1639,  643,   282,
     159,   85,    42,    22,    16,    15,    4,     1,     0},
    {16373, 16371, 16367, 16363, 16354, 16336, 16290, 16204, 16047,
     15735, 14940, 13159, 10171, 6377,  3044,  1212,  474,   208,
     115,   60,    27,    14,    7,     6,     5,     1,     0},
    {16382, 16377, 16367, 16357, 16334, 16281, 16213, 16035, 15613, 14694, 12898, 9720, 5747, 2506,
     1030,  469,   251,   124,   58,    48,    35,    17,    12,    7,     6,     5,    0},
    {16383, 16375, 16374, 16366, 16336, 16250, 16107, 15852, 15398, 14251, 12117, 8796, 5016, 2288,
     998,   431,   236,   132,   89,    37,    16,    12,    4,     3,     2,     1,    0},
    {16375, 16357, 16312, 16294, 16276, 16222, 16133, 15999, 15515, 14655, 13123, 10667, 7324, 4098,
     2073,  1141,  630,   370,   209,   93,    48,    39,    12,    11,    10,    9,     0},
    {16343, 16312, 16281, 16179, 16067, 15730, 15464, 15025, 14392, 13258, 11889, 10224, 7824, 5761,
     3902,  2349,  1419,  837,   520,   285,   183,   122,   71,    61,    40,    20,    0}};

static const SHORT cf_se20[7][27] = {
    {16351, 16344, 16317, 16283, 16186, 16061, 15855, 15477, 14832, 13832, 12286, 10056, 7412, 4889,
     2996,  1739,  1071,  716,   496,   383,   296,   212,   149,   109,   82,    59,    0},
    {16368, 16352, 16325, 16291, 16224, 16081, 15788, 15228, 14074, 12059, 9253, 5952, 3161, 1655,
     1006,  668,   479,   357,   254,   199,   154,   115,   88,    67,    51,   45,   0},
    {16372, 16357, 16339, 16314, 16263, 16169, 15984, 15556, 14590, 12635, 9475, 5625, 2812, 1488,
     913,   641,   467,   347,   250,   191,   155,   117,   89,    72,    59,   46,   0},
    {16371, 16362, 16352, 16326, 16290, 16229, 16067, 15675, 14715, 12655, 9007, 5114, 2636, 1436,
     914,   650,   477,   357,   287,   227,   182,   132,   105,   79,    58,   48,   0},
    {16364, 16348, 16318, 16269, 16192, 16033, 15637, 14489, 12105, 8407, 4951, 2736, 1669, 1156,
     827,   615,   465,   348,   269,   199,   162,   125,   99,    73,   51,   37,   0},
    {16326, 16297, 16257, 16136, 15923, 15450, 14248, 11907, 8443, 5432, 3396, 2226, 1561, 1201,
     909,   699,   520,   423,   323,   255,   221,   163,   121,  87,   71,   50,   0},
    {16317, 16280, 16203, 16047, 15838, 15450, 14749, 13539, 11868, 9790, 7789, 5956, 4521, 3400,
     2513,  1926,  1483,  1100,  816,   590,   431,   306,   214,   149,  105,  60,   0}};

static const SHORT cf_se11[7][7][27] = {
    {{16375, 16372, 16367, 16356, 16326, 16249, 16009, 15318, 13710, 10910, 7311, 3989, 1850, 840,
      380,   187,   103,   66,    46,    36,    26,    20,    15,    12,    8,    6,    0},
     {16383, 16382, 16381, 16380, 16379, 16378, 16377, 16374, 16363,
      16323, 16171, 15649, 14281, 11398, 7299,  3581,  1336,  428,
      135,   49,    17,    7,     4,     3,     2,     1,     0},
     {16383, 16382, 16381, 16380, 16379, 16378, 16375, 16367, 16347,
      16267, 15969, 15044, 12765, 9094,  5087,  2234,  787,   251,
      89,    29,    13,    5,     4,     3,     2,     1,     0},
     {16383, 16382, 16379, 16376, 16359, 16313, 16124, 15490, 13752, 10641, 6693, 3409, 1499, 567,
      208,   76,    34,    17,    10,    7,     6,     5,     4,     3,     2,    1,    0},
     {16383, 16382, 16381, 16380, 16375, 16367, 16336, 16220, 15772, 14485, 12105, 8736, 5367, 2833,
      1387,  581,   239,   98,    46,    24,    12,    9,     7,     6,     5,     2,    0},
     {16383, 16382, 16380, 16379, 16377, 16375, 16347, 16269, 16004,
      15265, 13542, 10823, 7903,  5214,  3145,  1692,  847,   365,
      139,   47,    14,    9,     8,     5,     4,     3,     0},
     {16381, 16378, 16375, 16372, 16336, 16274, 16039, 15643, 14737, 13185, 11186, 8836, 6501, 4198,
      2444,  1270,  615,   281,   153,   93,    63,    48,    42,    33,    24,    21,   0}},
    {{16383, 16382, 16381, 16380, 16379, 16377, 16376, 16373, 16369,
      16357, 16316, 16205, 15866, 14910, 12674, 8962,  4857,  1970,
      632,   204,   75,    34,    15,    9,     5,     3,     0},
     {16383, 16382, 16381, 16380, 16379, 16378, 16377, 16376, 16375,
      16374, 16370, 16356, 16298, 16139, 15598, 14050, 10910, 6488,
      2627,  701,   138,   38,    12,    6,     2,     1,     0},
     {16383, 16382, 16381, 16380, 16379, 16378, 16377, 16375, 16358,
      16292, 15999, 15070, 12735, 8772,  4549,  1595,  376,   95,
      26,    10,    6,     5,     4,     3,     2,     1,     0},
     {16383, 16382, 16381, 16380, 16379, 16378, 16377, 16376, 16375,
      16373, 16361, 16309, 16153, 15563, 13983, 10829, 6716,  3004,
      1002,  267,   74,    19,    5,     4,     2,     1,     0},
     {16383, 16382, 16381, 16380, 16379, 16378, 16377, 16353, 16250,
      15897, 14810, 12582, 9100,  5369,  2494,  884,   281,   87,
      31,    12,    6,     5,     4,     3,     2,     1,     0},
     {16383, 16382, 16379, 16378, 16377, 16371, 16348, 16282, 16042,
      15416, 13942, 11431, 8296,  5101,  2586,  1035,  328,   68,
      15,    9,     6,     5,     4,     3,     2,     1,     0},
     {16383, 16380, 16379, 16373, 16340, 16267, 16130, 15773, 14969, 13751, 11722, 9172, 6092, 3329,
      1507,  563,   186,   86,    26,    23,    10,    7,     6,     5,     4,     1,    0}},
    {{16382, 16381, 16380, 16379, 16377, 16370, 16359, 16312, 16141,
      15591, 14168, 11084, 6852,  3124,  1105,  354,   124,   48,
      25,    14,    7,     6,     5,     4,     3,     1,     0},
     {16383, 16382, 16381, 16380, 16379, 16378, 16377, 16374, 16357,
      16301, 16076, 15343, 13341, 9379,  4693,  1476,  324,   67,
      18,    9,     7,     5,     4,     3,     2,     1,     0},
     {16383, 16382, 16381, 16380, 16379, 16378, 16377, 16369, 16349,
      16265, 15937, 14834, 12076, 7587,  3123,  769,   152,   44,
      13,    7,     6,     5,     4,     3,     2,     1,     0},
     {16383, 16382, 16381, 16380, 16379, 16378, 16376, 16367, 16324,
      16160, 15574, 13854, 10306, 5601,  1880,  436,   113,   34,
      18,    9,     6,     5,     4,     3,     2,     1,     0},
     {16383, 16382, 16381, 16380, 16379, 16378, 16372, 16348, 16267,
      15929, 14858, 12426, 8315,  4098,  1412,  384,   112,   40,
      16,    11,    6,     5,     4,     3,     2,     1,     0},
     {16383, 16382, 16381, 16380, 16379, 16376, 16367, 16310, 16123,
      15532, 13965, 11248, 7655,  3910,  1573,  491,   141,   43,
      18,    9,     8,     5,     4,     3,     2,     1,     0},
     {16383, 16381, 16379, 16378, 16377, 16373, 16371, 16367, 16347,
      16280, 16132, 15778, 14963, 13688, 11380, 8072,  4680,  2140,
      774,   193,   63,    33,    15,    7,     5,     4,     0}},
    {{16382, 16381, 16380, 16379, 16378, 16377, 16373, 16360, 16339,
      16250, 15927, 14873, 12393, 8549,  4645,  2000,  748,   271,
      109,   48,    19,    9,     5,     4,     3,     1,     0},
     {16383, 16382, 16381, 16380, 16379, 16378, 16377, 16371, 16351,
      16244, 15876, 14627, 11604, 6836,  2711,  772,   210,   54,
      21,    8,     6,     5,     4,     3,     2,     1,     0},
     {16383, 16382, 16381, 16380, 16379, 16378, 16376, 16372, 16341,
      16209, 15686, 13965, 10150, 5099,  1594,  333,   74,    27,
      12,    8,     6,     5,     4,     3,     2,     1,     0},
     {16383, 16382, 16381, 16380, 16379, 16378, 16376, 16369, 16321,
      16091, 15261, 12834, 8160,  3248,  821,   187,   59,    22,
      11,    7,     6,     5,     4,     3,     2,     1,     0},
     {16383, 16382, 16381, 16380, 16379, 16378, 16372, 16350, 16249,
      15838, 14425, 11097, 6138,  2238,  628,   180,   53,    21,
      13,    7,     6,     5,     4,     3,     2,     1,     0},
     {16383, 16382, 16381, 16380, 16379, 16377, 16365, 16308, 16026, 15269, 13352, 9583, 5246, 2223,
      754,   202,   57,    26,    9,     8,     7,     6,     4,     3,     2,     1,    0},
     {16379, 16378, 16377, 16376, 16375, 16370, 16365, 16338, 16270,
      16120, 15723, 14760, 12783, 9474,  5727,  2713,  977,   296,
      93,    39,    14,    12,    10,    7,     4,     3,     0}},
    {{16383, 16382, 16379, 16378, 16377, 16370, 16364, 16342, 16267,
      16032, 15272, 13475, 10375, 6652,  3685,  1813,  805,   358,
      152,   61,    33,    20,    9,     7,     5,     3,     0},
     {16383, 16382, 16381, 16380, 16379, 16378, 16376, 16361, 16311,
      16096, 15280, 13085, 9315,  5003,  1992,  647,   170,   60,
      25,    17,    7,     5,     4,     3,     2,     1,     0},
     {16383, 16382, 16381, 16380, 16379, 16378, 16377, 16376, 16375,
      16372, 16355, 16288, 15990, 14926, 12076, 7449,  3161,  981,
      302,   78,    24,    7,     4,     3,     2,     1,     0},
     {16383, 16382, 16381, 16380, 16379, 16377, 16373, 16351, 16264,
      15836, 14299, 10534, 5358,  1777,  499,   145,   44,    17,
      11,    8,     6,     5,     4,     3,     2,     1,     0},
     {16383, 16382, 16381, 16380, 16379, 16377, 16366, 16324, 16155, 15416, 13055, 8332, 3423, 1080,
      304,   97,    39,    16,    9,     7,     6,     5,     4,     3,     2,     1,    0},
     {16383, 16382, 16381, 16380, 16377, 16373, 16359, 16258, 15905, 14720, 11631, 6834, 2911, 1022,
      345,   116,   49,    24,    14,    7,     6,     5,     4,     3,     2,     1,    0},
     {16383, 16380, 16379, 16378, 16377, 16376, 16375, 16370, 16365,
      16338, 16236, 15960, 15302, 13685, 10788, 6853,  3314,  1213,
      417,   149,   59,    25,    8,     3,     2,     1,     0}},
    {{16378, 16377, 16376, 16374, 16373, 16368, 16349, 16303, 16149,
      15653, 14445, 12326, 9581,  6707,  4156,  2251,  1062,  460,
      202,   93,    53,    25,    12,    8,     3,     1,     0},
     {16383, 16382, 16381, 16380, 16379, 16378, 16374, 16365, 16317,
      16146, 15685, 14441, 11949, 8459,  4949,  2280,  874,   300,
      86,    29,    20,    10,    7,     3,     2,     1,     0},
     {16383, 16382, 16381, 16380, 16379, 16377, 16358, 16306, 16114,
      15474, 13793, 10641, 6491,  3116,  1219,  382,   135,   62,
      26,    17,    11,    6,     5,     3,     2,     1,     0},
     {16383, 16382, 16381, 16380, 16379, 16376, 16361, 16305, 16051, 15112, 12593, 8234, 4130, 1583,
      552,   182,   59,    25,    10,    9,     6,     5,     4,     3,     2,     1,    0},
     {16383, 16382, 16381, 16380, 16379, 16376, 16346, 16245, 15837, 14409, 10881, 5964, 2333, 798,
      279,   100,   41,    14,    9,     7,     6,     5,     4,     3,     2,     1,    0},
     {16383, 16382, 16380, 16379, 16377, 16361, 16331, 16156, 15454, 13155, 8820, 4256, 1671, 610,
      218,   84,    42,    14,    10,    9,     8,     6,     5,     4,     3,    2,    0},
     {16382, 16380, 16378, 16377, 16367, 16352, 16241, 16077, 15536, 14352, 11787, 7926, 4119, 1726,
      638,   233,   91,    28,    16,    9,     8,     6,     5,     4,     3,     1,    0}},
    {{16369, 16361, 16352, 16340, 16315, 16284, 16223, 16091, 15848,
      15385, 14573, 13396, 11681, 9316,  6613,  4037,  2144,  1033,
      491,   213,   100,   55,    34,    18,    12,    6,     0},
     {16382, 16381, 16379, 16376, 16371, 16359, 16345, 16306, 16198,
      16002, 15534, 14580, 12881, 10271, 6793,  3572,  1467,  504,
      152,   60,    23,    14,    5,     4,     2,     1,     0},
     {16383, 16382, 16380, 16379, 16378, 16376, 16367, 16360, 16344,
      16292, 16183, 15902, 15224, 13793, 11340, 7866,  4409,  1916,
      689,   225,   80,    34,    16,    6,     3,     1,     0},
     {16381, 16380, 16379, 16377, 16376, 16372, 16366, 16353, 16325,
      16266, 16097, 15632, 14551, 12346, 9014,  5262,  2439,  920,
      324,   126,   50,    20,    9,     6,     4,     1,     0},
     {16383, 16380, 16379, 16377, 16375, 16373, 16369, 16360, 16338,
      16283, 16183, 15892, 15109, 13313, 10173, 6308,  3103,  1264,
      457,   169,   75,    30,    15,    5,     2,     1,     0},
     {16379, 16377, 16372, 16370, 16365, 16347, 16296, 16186, 15988,
      15448, 14083, 11465, 7678,  4215,  1961,  900,   431,   193,
      87,    37,    21,    13,    8,     5,     2,     1,     0},
     {16373, 16368, 16360, 16342, 16320, 16294, 16230, 16123, 15884,
      15548, 14801, 13380, 11064, 7909,  4654,  2378,  1114,  490,
      235,   135,   74,    40,    21,    11,    6,     1,     0}}};

// static short cf_off_se00 = -55;

static const short cf_off_se01 = +2;

static const short cf_off_se10 = -4;

static const short cf_off_se02[7] = {+1, +1, +1, +0, +0, +1, +2};

static const short cf_off_se20[7] = {+0, -2, -2, -2, -3, -4, -3};

static const short cf_off_se11[7][7] = {{-5, +0, +0, -3, -1, +0, -1}, {+1, +3, +0, +3, +0, +0, -1},
                                        {-2, +0, +0, +0, +0, +0, +3}, {+0, +0, +0, +0, +0, +0, +2},
                                        {+0, +0, +3, +0, +0, +0, +4}, {+0, +1, +0, +0, +0, +0, +1},
                                        {+0, +1, +3, +3, +4, +2, +4}};

/*****************   end generated tables from sfe_tables.c *****************/

#define SFE_GROUP_SIZE 2 /* number of scale factor bands to group for Scale Factor Energies */
#define SFE_QUANT_EXTRA                                                    \
  2 /* factor to decrease quantization precision for Scale Factor Energies \
     */

#define MIN_ENC_SEPARATE -12
#define MAX_ENC_SEPARATE +12
#define SYMBOLS_IN_TABLE (1 + (MAX_ENC_SEPARATE - MIN_ENC_SEPARATE + 1) + 1)
/* two extra symbols are used for escape coding of large positive and negative values */

#define CTX_OFFSET 3
static INT quant_ctx(INT ctx) {
  /*
    ctx ... -5 -4 -3 -2 -1 0 1 2 3 4 5 ...
  Q(ctx)... -3 -3 -3 -2 -1 0 1 2 3 3 3 ...
  */
  if (fAbs(ctx) <= 3) {
    return ctx;
  } else if (ctx > 3) {
    return 3;
  } else { /* ctx < -3 */
    return -3;
  }
}

static INT arith_decode_bits(IGFSCFDEC_PRIVATE_DATA_HANDLE hPrivateData, /**< instance handle */
                             INT nBits /**< number of bits to decode */
) {
  INT i;
  static SHORT cf_for_bit[2] = {8192, 0};
  INT x = 0;

  /* ari_start_decoding_14bits */
  if (arith_first_symbol(hPrivateData)) {
    hPrivateData->st.low = 0;
    hPrivateData->st.high = ari_q4new;
    hPrivateData->st.vobf = FDKreadBits(hPrivateData->hBs, cbitsnew);
  }

  for (i = nBits - 1; i >= 0; --i) {
    // int bit = arith_decode(cf_for_bit, 2, hPrivateData);
    INT bit = ari_decode_14bits(hPrivateData->hBs, &(hPrivateData->st), cf_for_bit, 2);
    x = x + (bit << i);
  }

  return x;
}

static INT arith_decode_residual(
    IGFSCFDEC_PRIVATE_DATA_HANDLE hPrivateData, /**< instance handle */
    const SHORT* cumulativeFrequencyTable,      /**< cumulative frequency table to be used */
    INT tableOffset                             /**< offset used to align the table */
) {
  INT val;
  INT x;

  // TEMP
  /* ari_start_decoding_14bits */
  if (arith_first_symbol(hPrivateData)) {
    hPrivateData->st.low = 0;
    hPrivateData->st.high = ari_q4new;
    hPrivateData->st.vobf = FDKreadBits(hPrivateData->hBs, cbitsnew);
  }

  // val = arith_decode(cumulativeFrequencyTable, SYMBOLS_IN_TABLE, hPrivateData);
  val = ari_decode_14bits(hPrivateData->hBs, &(hPrivateData->st), cumulativeFrequencyTable,
                          SYMBOLS_IN_TABLE);
  /* meaning of the values of val: */
  /* esc_{0} MIN_ENC_SEPARATE (MIN_ENC_SEPARATE + 1) ... (MAX_ENC_SEPARATE - 1) MAX_ENC_SEPARATE
   * esc_{SYMBOLS_IN_TABLE - 1} */

  if ((val != 0) && (val != SYMBOLS_IN_TABLE - 1)) {
    INT tmp = (val - 1) + MIN_ENC_SEPARATE;

    tmp -= tableOffset;
    return tmp;
  } else if (val == 0) { /* escape code 0 to indicate x <= MIN_ENC_SEPARATE - 1 */
    /* decode the tail of the distribution */

    /* decode extra with 4 bits */
    INT extra = arith_decode_bits(hPrivateData, 4);
    if (extra == 15) { /* escape code 15 to indicate extra >= 15 */
      /* decode addtional extra with 7 bits */
      extra = 15 + arith_decode_bits(hPrivateData, 7);
    }

    x = (MIN_ENC_SEPARATE - 1) - extra;
  } else { /* escape code (SYMBOLS_IN_TABLE - 1) to indicate x >= MAX_ENC_SEPARATE + 1 */
    /* decode the tail of the distribution */

    /* decode extra with 4 bits */
    INT extra = arith_decode_bits(hPrivateData, 4);
    if (extra == 15) { /* escape code 15 to indicate extra >= 15 */
      /* decode addtional extra with 7 bits */
      extra = 15 + arith_decode_bits(hPrivateData, 7);
    }

    x = (MAX_ENC_SEPARATE + 1) + extra;
  }

  x -= tableOffset;
  return x;
}

static void arith_decode_flush(IGFSCFDEC_PRIVATE_DATA_HANDLE hPrivateData /**< instance handle */
) {
  FDKpushBack(hPrivateData->hBs, 14);
  return;
}

static void arith_decode_flush_no_Data(
    IGFSCFDEC_PRIVATE_DATA_HANDLE hPrivateData /**< instance handle */
) {
  FDKpushFor(hPrivateData->hBs, 2);
  return;
}

static INT decode_sfe_vector(IGFSCFDEC_PRIVATE_DATA_HANDLE hPrivateData, /**< instance handle */
                             INT t, /**< frame counter reset to 0 at each independent frame */
                             SCHAR prev_prev_x0, /**< value d = prev_prev_x[0], when t >= 2 */
                             SCHAR* prev_x,      /**< previous vector */
                             SCHAR* x,           /**< current vector to decode */
                             INT length,         /**< number of elements to decode */
                             INT igfWinMode,     /**< whether a short block is processed */
                             INT sfeReducePrecisionEnabled /**< 0 = no pairing, 1 = pairing */
) {
  /*
     f
     ^
     |  d a x
     |    c b
     |      e  --> t
  */
  INT f;

  INT sfe_group_enabled = (sfeReducePrecisionEnabled && (0 == igfWinMode));
  INT increment = 1; /* size of each group */

  if (sfe_group_enabled) increment = SFE_GROUP_SIZE;

  for (f = 0; f < length; f += increment) {
    INT z;

    if (t == 0) {
      if (f == 0) {
        x[f] = arith_decode_bits(hPrivateData, 7); /* 7 bit raw */
      } else if (f == increment) {
        INT pred = x[f - increment]; /* pred = b */

        x[f] = pred + arith_decode_residual(hPrivateData, cf_se01, cf_off_se01);
      } else {                                                        /* f >= 2 * increment */
        INT pred = x[f - increment];                                  /* pred = b */
        INT ctx = quant_ctx(x[f - increment] - x[f - 2 * increment]); /* Q(b - e) */

        x[f] = pred + arith_decode_residual(hPrivateData, cf_se02[CTX_OFFSET + ctx],
                                            cf_off_se02[CTX_OFFSET + ctx]);
      }
    } else if (f == 0) {
      if (t == 1) {
        INT pred = prev_x[f]; /* pred = a */

        x[f] = pred + arith_decode_residual(hPrivateData, cf_se10, cf_off_se10);
      } else {                                         /* t >= 2 */
        INT pred = prev_x[f];                          /* pred = a */
        INT ctx = quant_ctx(prev_x[f] - prev_prev_x0); /* Q(a - d) */

        x[f] = pred + arith_decode_residual(hPrivateData, cf_se20[CTX_OFFSET + ctx],
                                            cf_off_se20[CTX_OFFSET + ctx]);
      }
    } else { /* (t >= 1) && (f >= increment) */
      INT pred = prev_x[f] + x[f - increment] - prev_x[f - increment]; /* pred = a + b - c */
      INT ctx_f = quant_ctx(prev_x[f] - prev_x[f - increment]);        /* Q(a - c) */
      INT ctx_t = quant_ctx(x[f - increment] - prev_x[f - increment]); /* Q(b - c) */

      x[f] = pred + arith_decode_residual(hPrivateData,
                                          cf_se11[CTX_OFFSET + ctx_t][CTX_OFFSET + ctx_f],
                                          cf_off_se11[CTX_OFFSET + ctx_t][CTX_OFFSET + ctx_f]);
    }

    /* all the values in the group must be identical with the first one */
    for (z = 1; z < increment; ++z) {
      if (f + z < length) {
        x[f + z] = x[f];
      }
    }
  }

  return 0; /* the size is obtained in the calling function, only after calling arith_encode_flush
             */
}
/**********************************************************************/
int iisIGFSCFDecoderGetLastFrameWasShortBlock(
    IGFSCFDEC_PRIVATE_DATA* hPrivateData /**< inout: handle to public data or NULL in case there was
                                            no instance created */
) {
  return hPrivateData->lastFrameBlockType;
}

/**********************************************************************/ /**
 setter func for internal var
 **************************************************************************/
void iisIGFSCFDecoderSetLastFrameWasShortBlock(
    IGFSCFDEC_PRIVATE_DATA* hPrivateData, /**< inout: handle to public data or NULL in case there
                                             was no instance created */
    int igfBlockType) {
  hPrivateData->lastFrameBlockType = igfBlockType;
}

/**********************************************************************
resets the internal decoder memory (context memory
**************************************************************************/
void iisIGFSCFDecoderReset(
    IGFSCFDEC_PRIVATE_DATA* hPrivateData /**< inout: handle to public data or NULL in case there was
                                            no instance created */
) {
  /* reset of coder */
  FDKmemset(hPrivateData->prevLB, 0, sizeof(SCHAR) * hPrivateData->scfCountLongBlock);
  FDKmemset(hPrivateData->prevSB, 0, sizeof(SCHAR) * hPrivateData->scfCountShortBlock);
  FDKmemset(hPrivateData->prevTCXmedium, 0, sizeof(SCHAR) * hPrivateData->scfCountTCXBlock);
  FDKmemset(hPrivateData->prevTCXlong, 0, sizeof(SCHAR) * hPrivateData->scfCountTCXBlock);
  hPrivateData->t = 0;
  hPrivateData->prevDLB = 0;
  hPrivateData->prevDSB = 0;
  hPrivateData->prevDTCXmedium = 0;
  hPrivateData->prevDTCXlong = 0;
}

/**********************************************************************/ /**
 main decoder function:
 **************************************************************************/
void iisIGFSCFDecoderDecode(
    IGFSCFDEC_PRIVATE_DATA* hPrivateData, HANDLE_FDK_BITSTREAM hBStr,
    SCHAR* sfe, /**< out: ptr to an array which will contain the decoded quantized coefficients */
    INT indepFlag,  /**< in: if  1 on input the encoder will be forced to reset,
                      if  0 on input the encodder will be forced to encode without a reset */
    INT igfWinMode, /**< in: */
    INT noShortBlock, INT sfeReducePrecisionEnabled) {
  INT isWindowTransition; /* is set to !0 if there was a change from short window to long window or
                             vice versa */

  /* insert data: */

  hPrivateData->hBs = hBStr;

  hPrivateData->arith_decode_first_symbol = 1;
  hPrivateData->sfe = sfe;

  /* check if coder needs a reset and do it if neccessary */
  isWindowTransition = (hPrivateData->lastFrameBlockType != igfWinMode);
  if (indepFlag || isWindowTransition) {
    /* reset of coder */
    iisIGFSCFDecoderReset(hPrivateData);
  }

  switch (igfWinMode) {
    case IGF_SCF_LONG:
      decode_sfe_vector(hPrivateData, hPrivateData->t, hPrivateData->prevDLB, hPrivateData->prevLB,
                        sfe, hPrivateData->scfCountLongBlock, igfWinMode,
                        sfeReducePrecisionEnabled);
      /* advancing history: */
      hPrivateData->prevDLB = hPrivateData->prevLB[0];
      FDKmemmove(hPrivateData->prevLB, sfe, sizeof(SCHAR) * hPrivateData->scfCountLongBlock);

      if (sfeReducePrecisionEnabled) {
        INT tt;
        /* rescale the sfe output values to align to the default 1.5 dB quantization step */
        /* according to the formula in the calculateScfEnergiesForIGF function: gain = (int)(0.5 + 4
         * * log_2(energy)) */
        /* note that the entire decoding process and history buffer management uses the original
         * unscaled values */
        for (tt = 0; tt < hPrivateData->scfCountLongBlock; ++tt) {
          sfe[tt] *= SFE_QUANT_EXTRA;
        }
      }

      hPrivateData->t++;
      hPrivateData->lastFrameBlockType = igfWinMode;

      if (hPrivateData->scfCountLongBlock) {
        arith_decode_flush(hPrivateData); /* finish decoding */
      } else {
        arith_decode_flush_no_Data(hPrivateData); /* finish decoding if nothing is read*/
      }

      break;

    case IGF_SCF_SHORT:
      /*                                                            sfe[pos], pos=:   0   1   2   3
       * 4   5   6   7     8   9  10  11 */
      /* sfe data with short blocks now e.g. noShortBlock=3, scfCountShortBlock = 4: e11 e12 e13 e14
       * | e21 e22 e23 e24 | e31 e32 e33 e34 */
      INT w;

      for (w = 0; w < noShortBlock; ++w) {
        decode_sfe_vector(hPrivateData, hPrivateData->t, hPrivateData->prevDSB,
                          hPrivateData->prevSB, sfe, hPrivateData->scfCountShortBlock, igfWinMode,
                          sfeReducePrecisionEnabled);
        /* advancing history: */
        hPrivateData->prevDSB = hPrivateData->prevSB[0];
        FDKmemmove(hPrivateData->prevSB, sfe, sizeof(SCHAR) * hPrivateData->scfCountShortBlock);

        if (sfeReducePrecisionEnabled) {
          INT tt;
          /* rescale the sfe output values to align to the default 1.5 dB quantization step */
          /* according to the formula in the calculateScfEnergiesForIGF function: gain = (int)(0.5 +
           * 4 * log_2(energy)) */
          /* note that the entire decoding process and history buffer management uses the original
           * unscaled values */
          for (tt = 0; tt < hPrivateData->scfCountShortBlock; ++tt) {
            sfe[tt] *= SFE_QUANT_EXTRA;
          }
        }

        sfe += hPrivateData->scfCountShortBlock;
        hPrivateData->t++;
      }
      hPrivateData->lastFrameBlockType = igfWinMode;

      if (hPrivateData->scfCountShortBlock) {
        arith_decode_flush(hPrivateData); /* finish decoding */
      } else {
        arith_decode_flush_no_Data(hPrivateData); /* finish decoding if nothing is read*/
      }

      break;
  }
}
