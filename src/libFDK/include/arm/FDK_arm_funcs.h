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

   Author(s):   Arthur Tritthart, Tobias Chalupka

   Description: Functions replacing ARM operations

*******************************************************************************/

/* clang-format off */
#ifndef FDK_ARM_FUNCS_H
#define FDK_ARM_FUNCS_H

#define HASHSIGN #
#define PLUSSIGN +
#define FDK_TEXT(x) x

/*   Do not compile NEON with 3.1 armcc (version=310862)  */
#if defined(__ARM_NEON__)
#include "FDK_neon_funcs.h"
#elif defined(__ARM_AARCH64_NEON__)
#include "arm/FDK_aarch64_neon_funcs.h"
#endif /*  defined(__ARM_NEON__) */

#ifdef __x86__
#undef __GNUC__
#endif

/* ----------------------------------------------------------------------------*/
/*  General ARM core instructions                                              */
/* ----------------------------------------------------------------------------*/
#if defined(__GNUC__)
#define FDK_push(reg)                     "PUSH { " #reg " } \n\t"
#define FDK_pop(reg)                      "POP  { " #reg " } \n\t"
#define FDK_push_lr(reg)                  "PUSH { " #reg ", lr } \n\t"
#define FDK_pop_pc(reg)                   "POP  { " #reg ", pc } \n\t"
#define FDK_mpush(first_reg, last_reg)    "PUSH { " #first_reg "-" #last_reg " } \n\t"
#define FDK_mpop(first_reg, last_reg)     "POP  { " #first_reg "-" #last_reg " } \n\t"
#define FDK_mpush_lr(first_reg, last_reg) "PUSH { " #first_reg "-" #last_reg ", lr } \n\t"
#define FDK_mpop_pc(first_reg, last_reg)  "POP  { " #first_reg "-" #last_reg ", pc } \n\t"
#else
#define FDK_push(reg)
#define FDK_pop(reg)                              ;
#define FDK_push_lr(reg)
#define FDK_pop_pc(reg)
#define FDK_mpush(first_reg, last_reg)
#define FDK_mpop(first_reg, last_reg)              ;
#define FDK_mpush_lr(first_reg, last_reg)
#define FDK_mpop_pc(first_reg, last_reg)           ;
#endif

#if defined(__GNUC__)
#define FDK_ldr(dst, src, offset, name)            "LDR " #dst ", [" #src ", #" #offset "] \n\t"
#define FDK_ldr_reg(dst, src, offset, name)        "LDR " #dst ", [" #src ", " #offset "] \n\t"
#define FDK_ldrsb_reg(dst, src, offset, name)      "LDRSB " #dst ", [" #src ", " #offset "] \n\t"
#define FDK_ldr_reg_oplsl(dst, src, offset, oplsl) "LDR " #dst ", [" #src ", " #offset ", LSL #" #oplsl "] \n\t"
#define FDK_ldr_reg_oplsr(dst, src, offset, oplsr) "LDR " #dst ", [" #src ", " #offset ", LSR #" #oplsr "] \n\t"
#define FDK_ldr_ia(dst, src, offset)               "LDR " #dst ", [" #src "], #" #offset " \n\t"
#define FDK_ldr_cond(cond, dst, src, offset)       "LDR" #cond " " #dst ", [" #src ", #" #offset "] \n\t"
#define FDK_ldr_ia_cond(cond, dst, src, offset)    "LDR" #cond " " #dst ", [" #src "], #" #offset " \n\t"
#define FDK_ldrb_ia(dst, src, offset)              "LDRB " #dst ", [" #src "], #" #offset " \n\t"
#else
/*  example: LDR r2, [sp, #0x20]  <- r2 = call parameter #3 (FIXP_DBL *TimeOut) */
#define FDK_ldr(dst, src, offset, name)            {  dst = name; }
#define FDK_ldr_reg(dst, src, offset, name)        {  dst = name; }
#define FDK_ldrsb_reg(dst, src, offset, name)      {  dst = (SCHAR) name; }
#define FDK_ldr_reg_oplsl(dst, src, offset, oplsl) {  dst = ((INT *)src)[(offset << oplsl)/sizeof(INT)];
#define FDK_ldr_reg_oplsr(dst, src, offset, oplsr) {  dst = ((INT *)src)[(offset >> oplsr)/sizeof(INT)];
/*  example: LDR r2, [r0], #0x4   <- r2 = *ptr; ptr+=2; (FIXP_SGL) */
/*  example: LDR r2, [r0], #0x4   <- r2 = *ptr++;       (FIXP_DBL) */
#define FDK_ldr_ia(dst, src, offset)               { dst = ((INT*)src)[0]; src = &((INT*)src)[4/sizeof(INT)]; }
#define FDK_ldr_cond(cond, dst, src, offset)       { if (__FDK_coreflags_ ## cond) dst = (INT *)src[offset/(INT)sizeof(src)]; }
#define FDK_ldr_ia_cond(cond, dst, src, offset)    { if (__FDK_coreflags_ ## cond)                \
                                                     { dst = (INT *)src[offset/(INT)sizeof(src)]; \
                                                       src = &(INT*)src[offset/sizeof(INT)];      \
                                                   } }
#define FDK_ldrb_ia(dst, src, offset)              { dst = (SCHAR*)src[0]; src += offset/(INT)sizeof(SCHAR); }
#endif

#if defined(__GNUC__)
#define FDK_ldrh(dst, src, offset, name)     "LDRH " #dst ", [" #src ", #" #offset "] \n\t"
#define FDK_ldrh_ia(dst, src, offset)        "LDRH " #dst ", [" #src "], #" #offset " \n\t"
#define FDK_ldrh_pu_add(dst, src, reg)       "LDRH " #dst ", [" #src "], +" #reg " \n\t"
#else
#define FDK_ldrh(dst, src, offset, name)            { dst = src[offset/(INT)sizeof(*src)]; }
#define FDK_ldrh_ia(dst, src, offset)               { dst = *src; src += offset/(INT)sizeof(*src); }
#define FDK_ldrh_pu_add(dst, src, reg)              { dst = *src; src += reg/(INT)sizeof(*src); }
#endif


#if defined(__GNUC__)
#define FDK_ldrd(dst1, dst2, src, offset, name1, name2)       "LDRD " #dst1 ", " #dst2 ", [" #src ", #" #offset "] \n\t"
#define FDK_ldrd_reg(dst1, dst2, src, offset)                 "LDRD " #dst1 ", " #dst2 ", [" #src ", " #offset "] \n\t"
#define FDK_ldrd_reg_oplsl(dst1, dst2, src, offset, oplsl)    "LDRD " #dst1 ", " #dst2 ", [" #src ", " #offset ", LSL #" #oplsl "] \n\t"
#define FDK_ldrd_ia(dst1, dst2, src, offset)                  "LDRD " #dst1 ", " #dst2 ", [" #src "], #" #offset " \n\t"
#define FDK_ldrd_ia_reg(dst1, dst2, src, offset)              "LDRD " #dst1 ", " #dst2 ", [" #src "], " #offset " \n\t"

#ifdef __llvm__
#define FDK_ldrd_ia_cond(cond, dst1, dst2, src, offset)       "LDRD" #cond " " #dst1 ", " #dst2 ", [" #src "], #" #offset " \n\t"
#define FDK_ldrd_ia_cond_reg(cond, dst1, dst2, src, reg)      "LDRD" #cond " " #dst1 ", " #dst2 ", [" #src "], " #reg " \n\t"
#else
#define FDK_ldrd_ia_cond(cond, dst1, dst2, src, offset)       "LDR" #cond "D " #dst1 ", " #dst2 ", [" #src "], #" #offset " \n\t"
#define FDK_ldrd_ia_cond_reg(cond, dst1, dst2, src, reg)      "LDR" #cond "D " #dst1 ", " #dst2 ", [" #src "], " #reg " \n\t"
#endif
#else
#define FDK_ldrd(dst1, dst2, src, offset, name1, name2)       { dst1 = name1; \
                                                                dst2 = name2; }
#define FDK_ldrd_reg(dst1, dst2, src, offset)                 { dst1 = *((INT *)src)+(offset+0)sizeof(INT); \
                                                                dst2 = *((INT *)src)+(offset+4)sizeof(INT); }
#define FDK_ldrd_reg_oplsl(dst1, dst2, src, reg, oplsl)       { dst1 = (INT *)src[((reg << oplsl) + 0)/sizeof(INT)]; \
                                                                dst2 = (INT *)src[((reg << oplsl) + 4)/sizeof(INT)];
#define FDK_ldrd_ia(dst1, dst2, src, offset)                  { dst1 = (INT *)src[0/sizeof(INT)]; \
                                                                dst2 = (INT *)src[4/sizeof(INT)]; \
                                                                src = &(INT*)src[offset/sizeof(INT)]; }
#define FDK_ldrd_ia_reg(dst1, dst2, src, reg)                 { dst1 = (INT *)src[0/sizeof(INT)]; \
                                                                dst2 = (INT *)src[4/sizeof(INT)]; \
                                                                src = &(INT*)src[offset/sizeof(INT)]; }
#define FDK_ldrd_ia_cond(cond, dst1, dst2, src, offset)       { if (__FDK_coreflags_ ## cond) FDK_ldrd_ia(dst1, dst2, src, offset); }
#define FDK_ldrd_ia_cond_reg(cond, dst1, dst2, src, reg)      { if (__FDK_coreflags_ ## cond) FDK_ldrd_ia_reg(dst1, dst2, src, reg); }
#endif

#if defined(__GNUC__)
#define FDK_str_reg_op_lsl(src, dst, offset, scale, name)       "STR " #src ", [" #dst ", " #offset ", lsl #" #scale "] \n\t"
#else
#define FDK_str_reg_op_lsl(src, dst, offset, scale, name)       name = src;
#endif

#if defined(__GNUC__)
#define FDK_str(src, dst, offset, name)       "STR " #src ", [" #dst ", #" #offset "] \n\t"
#define FDK_str_ia(src, dst, offset)          "STR " #src ", [" #dst "], #" #offset " \n\t"
#else
#define FDK_str(src, dst, offset, name)       name = src;
#define FDK_str_ia(src, dst, offset)          { *(INT*)dst = src; dst += offset/(INT)sizeof(dst); }
#endif

#if defined(__GNUC__)
#define FDK_strh(dst, src, offset, name)     "STRH " #dst ", [" #src ", #" #offset "] \n\t"
#define FDK_strh_ia(dst, src, offset)        "STRH " #dst ", [" #src "], #" #offset " \n\t"
#define FDK_strh_ia_reg(dst, src, reg) "STRH " #dst ", [" #src "], " #reg " \n\t"
#define FDK_strh_pu_add(dst, src, reg)       "STRH " #dst ", [" #src "], +" #reg " \n\t"
#else
#define FDK_strh(dst, src, offset, name)            { dst = src[offset/(INT)sizeof(*src)]; }
#define FDK_strh_ia(dst, src, offset)               { dst = *src; src += offset/(INT)sizeof(*src); }
#define FDK_strh_ia_reg(dst, src, reg)              { dst = *src; src += reg/(INT)sizeof(*src); }
#define FDK_strh_pu_add(dst, src, reg)              { dst = *src; src += reg/(INT)sizeof(*src); }
#endif

#if defined(__GNUC__)
#define FDK_strd_ia(src1, src2, dst, offset)             "STRD " #src1 ", " #src2 ", [" #dst "], #" #offset " \n\t"
#define FDK_strd_ia_reg(src1, src2, dst, reg)            "STRD " #src1 ", " #src2 ", [" #dst "], " #reg " \n\t"
#define FDK_strd(src1, src2, dst, offset, name1, name2)  "STRD " #src1 ", " #src2 ", [" #dst ", #" #offset "] \n\t"
#else
#define FDK_strd_ia(src1, src2, dst, offset)      dst[0] = INT(src1); dst[1] = INT(src2); dst += offset/INT(sizeof(*dst));
#define FDK_strd_ia_reg(src1, src2, dst, reg)     dst[0] = INT(src1); dst[1] = INT(src2); dst += reg/INT(sizeof(*dst));

/*  Subsequent FDK_strd to be used only for stack access */
#define FDK_strd(src1, src2, dst, offset, name1, name2)   { name1 = src1; name2 = src2; }
#endif


#if defined(__GNUC__)
#define FDK_mov_reg(dst, src)    "MOV " #dst ", " #src "\n\t"
#define FDK_movs_reg(dst, src)   "MOVS " #dst ", " #src "\n\t"
#else
#define FDK_mov_reg(dst, src)    dst = src;
#define FDK_movs_reg(dst, src)            { dst = src; \
                                           __FDK_coreflags_NE = (dst!=0) ? 1 : 0; \
                                           __FDK_coreflags_EQ = (dst==0) ? 1 : 0; \
                                           __FDK_coreflags_GT = (dst> 0) ? 1 : 0; \
                                           __FDK_coreflags_PL = (dst>=0) ? 1 : 0; \
                                           __FDK_coreflags_MI = (dst< 0) ? 1 : 0; \
                                           __FDK_coreflags_LE = (dst<=0) ? 1 : 0; \
                                          }
#endif

#if defined(__GNUC__)
#define __FDK_mov_imm(dst, imm)    "MOV " #dst ", # " #imm "\n\t"
/*  GNUC does not allow const expressions instead of constants, we need one layer inbetween */
#define FDK_mov_imm(dst,imm)       __FDK_mov_imm(dst,imm)
#else
#define FDK_mov_imm(dst, imm)    dst = imm;
#endif

#if defined(__GNUC__)
#define FDK_tst_imm(src,imm)     "TST " #src ", # " #imm "\n\t"
#else
#define FDK_tst_imm(src, imm)    { __FDK_coreflags_NE = (src & imm) ? 1 : 0; \
                                   __FDK_coreflags_EQ = (src & imm) ? 0 : 1; }
#endif

#if defined(__GNUC__)
#define FDK_lsl_imm(dst, src, imm)      "LSL " #dst ", " #src ", # " #imm " \n\t"
#define FDK_lsls_imm(dst, src, imm)     "LSLS " #dst ", " #src ", # " #imm " \n\t"
#define FDK_lsl(Rd, Rm, Rs)             "LSL " #Rd ", " #Rm ", " #Rs " \n\t"
#define FDK_lsl_cond(cond, Rd, Rm, Rs)  "LSL" #cond " " #Rd ", " #Rm ", " #Rs " \n\t"
#define FDK_lsls(Rd, Rm, Rs)            "LSLS " #Rd ", " #Rm ", " #Rs " \n\t"
#else
#define FDK_lsl_imm(dst, src, imm)      {  FDK_ASSERT(imm > 0);   \
                                           FDK_ASSERT(imm < 8*(INT)sizeof(src)); \
                                           dst = src << imm; }
#define FDK_lsls_imm(dst, src, imm)     {  FDK_ASSERT(imm > 0);   \
                                           FDK_ASSERT(imm < 8*(INT)sizeof(src)); \
                                           __FDK_coreflags_CS = (src & (1<<(imm-1)) ? 1 : 0); \
                                           __FDK_coreflags_CC = (src & (1<<(imm-1)) ? 0 : 1); \
                                           dst = src << imm; \
                                           __FDK_coreflags_NE = (dst!=0) ? 1 : 0; \
                                           __FDK_coreflags_EQ = (dst==0) ? 1 : 0; \
                                           __FDK_coreflags_PL = (dst>=0) ? 1 : 0; \
                                           __FDK_coreflags_MI = (dst< 0) ? 1 : 0; \
                                           dst = src << imm; }
#define FDK_lsl(Rd, Rm, Rs)             {  FDK_ASSERT(Rs > 0);   \
                                           FDK_ASSERT(Rs < 32); \
                                           Rd = Rm << Rs; }
#define FDK_lsl_cond(Rd, Rm, Rs)        {  if (__FDK_coreflags_ ## cond) {\
                                           FDK_ASSERT(Rs > 0);   \
                                           FDK_ASSERT(Rs < 32); \
                                           Rd = Rm << Rs; } }
#define FDK_lsls(Rd, Rm, Rs)            {  FDK_ASSERT(Rs > 0);   \
                                           FDK_ASSERT(Rs < 32); \
                                           __FDK_coreflags_CS = (Rm & (1<<(Rs-1)) ? 1 : 0); \
                                           __FDK_coreflags_CC = (Rm & (1<<(Rs-1)) ? 0 : 1); \
                                           Rd = Rm << Rs; \
                                           __FDK_coreflags_NE = (dst!=0) ? 1 : 0; \
                                           __FDK_coreflags_EQ = (dst==0) ? 1 : 0; \
                                           __FDK_coreflags_PL = (dst>=0) ? 1 : 0; \
                                           __FDK_coreflags_MI = (dst< 0) ? 1 : 0; \
                                        }
#endif


#if defined(__GNUC__)
#define FDK_asr(dst, src, reg)     "ASR " #dst ", " #src ", " #reg " \n\t"
#else
#define FDK_asr(dst, src, reg)      {  FDK_ASSERT(reg > 0);   \
                                           FDK_ASSERT(reg < 8*(INT)sizeof(src)); \
                                           dst = src >> reg; }
#endif

#if defined(__GNUC__)
#define FDK_asr_imm(dst, src, imm)         "ASR " #dst ", " #src ", # " #imm " \n\t"
#define FDK_asr_cond(cond, dst, src, Rs)   "ASR" #cond " " #dst ", " #src ", " #Rs " \n\t"
#define FDK_asrs_imm(dst, src, imm)        "ASRS " #dst ", " #src ", # " #imm " \n\t"
#else
#define FDK_asr_imm(dst, src, imm)      {  FDK_ASSERT(imm > 0);   \
                                           FDK_ASSERT(imm < 8*(INT)sizeof(src)); \
                                           dst = src >> imm; }
#define FDK_asr_cond(cond, dst, src, Rs) {  if (__FDK_coreflags_ ## cond) {\
                                           FDK_ASSERT(Rs > 0);   \
                                           FDK_ASSERT(Rs < 8*(INT)sizeof(src)); \
                                           dst = src >> Rs; } }
#define FDK_asrs_imm(dst, src, imm)     {  FDK_ASSERT(imm > 0);                              \
                                           FDK_ASSERT(imm < 8*(INT)sizeof(src));             \
                                           __FDK_coreflags_CS = (src & (1>>(imm-1)) ? 1 : 0);\
                                           __FDK_coreflags_CC = (src & (1>>(imm-1)) ? 0 : 1);\
                                           dst = src >> imm;                                 \
                                           __FDK_coreflags_NE = (dst!=0) ? 1 : 0;            \
                                           __FDK_coreflags_EQ = (dst==0) ? 1 : 0;            \
                                           __FDK_coreflags_PL = (dst>=0) ? 1 : 0;            \
                                           __FDK_coreflags_MI = (dst< 0) ? 1 : 0;            \
                                         }
#endif

#if defined(__GNUC__)
#define FDK_movs_asr_imm(dst, src, imm)     "MOVS " #dst ", " #src ", ASR # " #imm " \n\t"
#else
#define FDK_movs_asr_imm(dst, src, imm)  {   FDK_ASSERT(imm > 0); \
                                             FDK_ASSERT(imm < (8*(INT)sizeof(src))); \
                                             __FDK_coreflags_CS = (src & (1<<(imm-1)) ? 1 : 0); \
                                             __FDK_coreflags_CC = (src & (1<<(imm-1)) ? 0 : 1); \
                                               dst = src >> imm; \
                                             __FDK_coreflags_NE = (dst!=0) ? 1 : 0; \
                                             __FDK_coreflags_EQ = (dst==0) ? 1 : 0; \
                                             __FDK_coreflags_PL = (dst>=0) ? 1 : 0; \
                                             __FDK_coreflags_MI = (dst< 0) ? 1 : 0; \
                                         }
#endif

#if defined(__GNUC__)
#define FDK_movs_lsl_imm(dst, src, imm)     "MOVS " #dst ", " #src ", LSL # " #imm " \n\t"
#else
#define FDK_movs_lsl_imm(dst, src, imm)  {   FDK_ASSERT(imm > 0); \
                                             FDK_ASSERT(imm < (8*(INT)sizeof(src))); \
                                             __FDK_coreflags_CS = (src & (1>>(imm-1)) ? 1 : 0); \
                                             __FDK_coreflags_CC = (src & (1>>(imm-1)) ? 0 : 1); \
                                               dst = src >> imm; \
                                             __FDK_coreflags_NE = (dst!=0) ? 1 : 0; \
                                             __FDK_coreflags_EQ = (dst==0) ? 1 : 0; \
                                             __FDK_coreflags_PL = (dst>=0) ? 1 : 0; \
                                             __FDK_coreflags_MI = (dst< 0) ? 1 : 0; \
                                         }
#endif

#if defined(__GNUC__)
#define FDK_pld(Rn, offset, name)                 "PLD [" #Rn ", #" #offset "] \n\t"
#define FDK_pld_label(name)                       "PLD # " #name " \n\t"
#define FDK_pld_add_op_lsl(Rn, Rm, scale, name)   "PLD [" #Rn ", +" #Rm ", LSL # " #scale "] \n\t"
#define FDK_pld_add_op_asr(Rn, Rm, scale, name)   "PLD [" #Rn ", +" #Rm ", ASR # " #scale "] \n\t"
#define FDK_pld_sub_op_lsl(Rn, Rm, scale, name)   "PLD [" #Rn ", -" #Rm ", LSL # " #scale "] \n\t"
#define FDK_pld_sub_op_asr(Rn, Rm, scale, name)   "PLD [" #Rn ", -" #Rm ", ASR # " #scale "] \n\t"
#define FDK_pli_label(name)                       "PLI # " #name " \n\t"
#else
                                                        /*  L2 cache data preload, core executes a nop */
#define FDK_pld(Rn, offset, name)
#define FDK_pld_label(name)
#define FDK_pld_add_op_lsl(Rn, Rm, scale, name)
#define FDK_pld_add_op_asr(Rn, Rm, scale, name)
#define FDK_pld_sub_op_lsl(Rn, Rm, scale, name)
#define FDK_pld_sub_op_asr(Rn, Rm, scale, name)
#define FDK_pli_label(name)
#endif



#if defined(__GNUC__)
#define FDK_mov_cond_imm(cond, dst, imm)     "MOV" #cond " " #dst ", # " #imm " \n\t"
#define FDK_mvn_cond_imm(cond, dst, imm)     "MVN" #cond " " #dst ", # " #imm " \n\t"
#define FDK_mov_cond(cond, dst, src)         "MOV" #cond " " #dst ",   " #src " \n\t"
#else
#define FDK_mov_cond_imm(cond, dst, imm)     {   if (__FDK_coreflags_ ## cond) dst =  imm; }
#define FDK_mvn_cond_imm(cond, dst, imm)     {   if (__FDK_coreflags_ ## cond) dst = -imm; }
#define FDK_mov_cond(cond, dst, src)         {   if (__FDK_coreflags_ ## cond) dst = src;  }
#endif



#if defined(__GNUC__)
#define FDK_lsr_imm(dst, src, imm)             "LSR " #dst ", " #src ", # " #imm " \n\t"
#define FDK_lsr_op_lsl(dst, src1, src2, imm)   "LSR " #dst ", " #src1 ", " #src2 ", LSL # " #imm "\n\t"
#define FDK_lsr(dst, src1, src2)               "LSR " #dst ", " #src1 ", " #src2 "\n\t"
#else
#define FDK_lsr_imm(dst, src, imm)            dst = (ULONG) src >> imm;
#define FDK_lsr_op_lsl(dst, src1, src2, imm)  dst = (ULONG) src1 >> (src2 << imm);
#define FDK_lsr(dst, src1, src2)              dst = (ULONG) src1 >> src2;
#endif

#if defined(__GNUC__)
#define FDK_add(dst, src1, src2)    "ADD " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_adds(dst, src1, src2)   "ADDS " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_qadd(dst, src1, src2)   "QADD " #dst ", " #src1 ", " #src2 " \n\t"
#else
#define FDK_add(dst, src1, src2)    dst = src1 + src2;
#define FDK_qadd(dst, src1, src2)   dst = src1 + src2;
#endif

#if defined(__GNUC__)
#define FDK_mul(dst, src1, src2)    "MUL " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_muls(dst, src1, src2)   "MULS " #dst ", " #src1 ", " #src2 " \n\t"
#else
#define FDK_mul(dst, src1, src2)    dst = src1 * src2;
#define FDK_muls(dst, src1, src2)   {    dst = src1 * src2;                                  \
                                         __FDK_coreflags_EQ = (INT(src1*src2) == 0) ? 1 : 0; \
                                         __FDK_coreflags_NE = (INT(src1*src2) != 0) ? 1 : 0; \
                                         __FDK_coreflags_GE = (INT(src1*src2) >= 0) ? 1 : 0; \
                                         __FDK_coreflags_LT = (INT(src1*src2) < 0) ? 1 : 0;  \
                                         __FDK_coreflags_GT = (INT(src1*src2) > 0) ? 1 : 0;  \
                                         __FDK_coreflags_LE = (INT(src1*src2) <= 0) ? 1 : 0;   }
#endif

#if defined(__GNUC__)
#define FDK_and_imm(dst, src, imm)    "AND " #dst ", " #src ",  # " #imm "  \n\t"
#define FDK_and(dst, src1, src2)      "AND " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_bic(dst, src1, src2)      "BIC " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_bic_imm(dst, src, imm)    "BIC " #dst ", " #src ",  # " #imm "  \n\t"
#define FDK_eor(dst, src1, src2)      "EOR " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_eor_op_asr(dst, src1, src2, imm, scale)      "EOR " #dst ", " #src1 ", " #src2 ", ASR #" #imm " \n\t"
#define FDK_eor_imm(dst, src, imm)    "EOR " #dst ", " #src ",  # " #imm "  \n\t"
#define FDK_orr(dst, src1, src2)      "ORR " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_orn(dst, src1, src2)      "ORN " #dst ", " #src1 ", " #src2 " \n\t"
#else
#define FDK_and_imm(dst, src, imm)    dst = src  & imm;
#define FDK_and(dst, src1, src2)      dst = src1 &  src2;
#define FDK_bic(dst, src1, src2)      dst = src1 & (~src2);
#define FDK_bic_imm(dst, src, imm)    dst = src & (~(imm));
#define FDK_eor(dst, src1, src2)      dst = src1 ^  src2;
#define FDK_eor_op_asr(dst, src1, src2)      dst = src1 ^ (src2>>(imm-scale));
#define FDK_eor_imm(dst, src, imm)    dst = src ^ imm;
#define FDK_orr(dst, src1, src2)      dst = src1 |  src2;
#define FDK_orn(dst, src1, src2)      dst = src1 | (~src2);
#endif

#if defined(__GNUC__)
#define FDK_clz(Rd, Rm)              "CLZ " #Rd ", " #Rm " \n\t"
#else
#define FDK_clz(Rd, Rm)               Rd = fNormz((FIXP_DBL)Rm);
#endif


/*  Example:                                                                                               */
/*   addition of integer: r0 = r1 + (r2<<3)              => FDK_add_op_lsl(r0, r1, r2, 3, 0)               */
/*                                                       => ADD r0, r1, r2, LSL #3                         */
/*   indexing a pointer:  r0 = (FIXP_DBL *) r1 + (r2<<3) => FDK_add_op_lsl(r0, r1, r2, 5, 2)               */
/*                                                       => ARM: ADD r0, r1, r2, LSL #5                    */
/*   addition of integer: r0 = r1 + (r2>>r3)             => FDK_add_op_asr_reg(r0, r1, r2, r3, 0)          */
/*                                                       => ADD r0, r1, r2, r3                             */
/*                                                       => C++: FIXP_DBL r0 = r1 + r2 >> r3               */
/*   addition of pointer: r0 = r1 + (r2>>r3)             => FDK_add_op_asr_reg(r0, r1, r2, r3, 2)          */
/*                                                       => ADD r0, r1, r2, r3                             */
/*                                                       => C++: FIXP_DBL *r0 = &r1[r2 >> r3]              */
#if defined(__GNUC__)
#define FDK_add_op_lsl(dst, src1, src2, imm, scale)            "ADD " #dst ", " #src1 ", " #src2 ", LSL # " #imm "\n\t"
#define FDK_sub_op_lsl(dst, src1, src2, imm, scale)            "SUB " #dst ", " #src1 ", " #src2 ", LSL # " #imm "\n\t"
#define FDK_sub_cond_op_lsl(cond, dst, src1, src2, imm, scale) "SUB" #cond " " #dst ", " #src1 ", " #src2 ", LSL # " #imm "\n\t"
#define FDK_add_cond_op_lsl(cond, dst, src1, src2, imm, scale) "ADD" #cond " " #dst ", " #src1 ", " #src2 ", LSL # " #imm "\n\t"
#define FDK_add_op_asr(dst, src1, src2, imm, scale)            "ADD " #dst ", " #src1 ", " #src2 ", ASR # " #imm "\n\t"
#define FDK_sub_op_asr(dst, src1, src2, imm, scale)            "SUB " #dst ", " #src1 ", " #src2 ", ASR # " #imm "\n\t"
#define FDK_add_op_asr_reg(dst, src1, src2, reg, scale)        "ADD " #dst ", " #src1 ", " #src2 ", ASR " #reg "\n\t"
#define FDK_sub_op_asr_reg(dst, src1, src2, reg, scale)        "SUB " #dst ", " #src1 ", " #src2 ", ASR " #reg "\n\t"
#else
#define FDK_add_op_lsl(dst, src1, src2, imm, scale)             dst = src1 + ((src2<<imm)>>scale);
#define FDK_add_cond_op_lsl(cond, dst, src1, src2, imm, scale)  { if (__FDK_coreflags_ ## cond) dst = src1 + ((src2<<imm)>>scale); }
#define FDK_sub_op_lsl(dst, src1, src2, imm, scale)             dst = src1 - ((src2<<imm)>>scale);
#define FDK_sub_cond_op_lsl(cond, dst, src1, src2, imm, scale)  { if (__FDK_coreflags_ ## cond) dst = src1 - ((src2<<imm)>>scale); }

#define FDK_add_op_asr(dst, src1, src2, imm, scale)             dst = src1 + ((src2>>imm)>>scale);
#define FDK_sub_op_asr(dst, src1, src2, imm, scale)             dst = src1 - ((src2>>imm)>>scale);
#define FDK_add_op_asr_reg(dst, src1, src2, reg, scale)         dst = src1 + ((src2>>reg)>>scale);
#define FDK_sub_op_asr_reg(dst, src1, src2, reg, scale)         dst = src1 - ((src2>>reg)>>scale);
#endif

#if defined(__GNUC__)
#define FDK_sub(dst, src1, src2)                               "SUB " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_rsb_imm(dst, src, imm)                             "RSB " #dst ", " #src ", # " #imm "\n\t"
#define FDK_rsb_cond_imm(cond, dst, src, imm)                  "RSB" #cond " " #dst ", " #src ", # " #imm " \n\t"
#else
#define FDK_sub(dst, src1, src2)      dst = src1 - src2;
#define FDK_rsb_imm(dst, src, imm)    dst = imm - src;
#define FDK_rsb_cond_imm(cond, dst, src, imm)                   { if (__FDK_coreflags_ ## cond) dst = imm-src; }
#endif


#if defined(__GNUC__)
#define __FDK_add_imm(dst, src, immediate, scale)              "ADD " #dst ", " #src ", # " #immediate "\n\t"
#define FDK_qadd_imm(dst, src, immediate, scale)               "QADD " #dst ", " #src ", # " #immediate "\n\t"
#define FDK_add_cond_imm(cond, dst, src, imm)                  "ADD" #cond " " #dst ", " #src ", # " #imm " \n\t"
#define __FDK_sub_imm(dst, src, immediate, scale)              "SUB " #dst ", " #src ", # " #immediate "\n\t"
#define FDK_sub_cond_imm(cond, dst, src, imm)                  "SUB" #cond " " #dst ", " #src ", # " #imm " \n\t"
#define FDK_add_imm(dst, src, immediate, scale)                __FDK_add_imm(dst, src, immediate, scale)
#define FDK_sub_imm(dst, src, immediate, scale)                __FDK_sub_imm(dst, src, immediate, scale)
#else
#define FDK_add_imm(dst, src, immediate, scale)  { dst = src + (immediate>>scale); }
#define FDK_qadd_imm(dst, src, immediate, scale) { dst = src + (immediate>>scale); }
#define FDK_add_cond_imm(cond, dst, src, imm)     {   if (__FDK_coreflags_ ## cond) dst = src+imm;  }
#define FDK_sub_imm(dst, src, immediate, scale)  { dst = src - (immediate>>scale); }
#define FDK_sub_cond_imm(cond, dst, src, imm)     {   if (__FDK_coreflags_ ## cond) dst = src-imm;  }
#endif


#if defined(__GNUC__)
#define FDK_subs_imm(dst, src, immediate)  "SUBS " #dst ", " #src ", # " #immediate "\n\t"
#define FDK_rsbs_imm(dst, src, immediate)  "RSBS " #dst ", " #src ", # " #immediate "\n\t"
#define FDK_subs(dst, src1, src2)          "SUBS " #dst ", " #src1 ", " #src2 "\n\t"
#define FDK_adds_imm(dst, src, immediate)  "ADDS " #dst ", " #src ", # " #immediate "\n\t"
#else
#define FDK_subs_imm(dst, src, immediate)  { dst = src - immediate; \
                                             __FDK_coreflags_PL = (dst>=0) ? 1 : 0; \
                                             __FDK_coreflags_LE = (dst<=0) ? 1 : 0; \
                                             __FDK_coreflags_GT = (dst> 0) ? 1 : 0; \
                                             __FDK_coreflags_MI = (dst< 0) ? 1 : 0; \
                                             __FDK_coreflags_NE = dst ? 1 : 0; \
                                             __FDK_coreflags_EQ = dst ? 0 : 1; }
#define FDK_rsbs_imm(dst, src, immediate)  { dst = immediate - src; \
                                             __FDK_coreflags_PL = (dst>=0) ? 1 : 0; \
                                             __FDK_coreflags_LE = (dst<=0) ? 1 : 0; \
                                             __FDK_coreflags_GT = (dst> 0) ? 1 : 0; \
                                             __FDK_coreflags_MI = (dst< 0) ? 1 : 0; \
                                             __FDK_coreflags_NE = dst ? 1 : 0; \
                                             __FDK_coreflags_EQ = dst ? 0 : 1; }

#define FDK_subs(dst, src1, src2)          { dst = src1 - src2; \
                                             __FDK_coreflags_PL = (dst>=0) ? 1 : 0; \
                                             __FDK_coreflags_LE = (dst<=0) ? 1 : 0; \
                                             __FDK_coreflags_GT = (dst> 0) ? 1 : 0; \
                                             __FDK_coreflags_MI = (dst< 0) ? 1 : 0; \
                                             __FDK_coreflags_NE = dst ? 1 : 0; \
                                             __FDK_coreflags_EQ = dst ? 0 : 1; }
#define FDK_adds_imm(dst, src, immediate)  { dst = src + immediate; \
                                             __FDK_coreflags_PL = (dst>=0) ? 1 : 0; \
                                             __FDK_coreflags_LE = (dst<=0) ? 1 : 0; \
                                             __FDK_coreflags_GT = (dst> 0) ? 1 : 0; \
                                             __FDK_coreflags_MI = (dst< 0) ? 1 : 0; \
                                             __FDK_coreflags_NE = dst ? 1 : 0; \
                                             __FDK_coreflags_EQ = dst ? 0 : 1; }
#endif

#if defined(__GNUC__)
#define FDK_ssat_op_asr_imm(dst, sat, src, imm)  "SSAT " #dst ", #" #sat ", " #src ", ASR #" #imm "\n\t"
#else
#define FDK_ssat_op_asr_imm(dst, sat, src, imm)  { dst = (src>>imm)>(1<<(sat-1)-1) ? (1<<(sat-1)-1) : ((src>>imm)<((-1)<<(sat-1)) ? : ((-1)<<(sat-1)) src>>imm); }
#endif

#if defined(__GNUC__)
#define FDK_smulwb(dst, src1, src2)  "SMULWB " #dst ", " #src1 ", " #src2 "\n\t"
#else
#define FDK_smulwb(dst, src1, src2)  { dst = src1 * src2 ; }
#endif

#if defined(__GNUC__)
#define FDK_smlawb(dst, src1, src2, src3)  "SMLAWB " #dst ", " #src1 ", " #src2 ", " #src3 "\n\t"
#else
#define FDK_smlawb(dst, src1, src2, src3)  { dst = src1 * src2 + src3; }
#endif

#if defined(__GNUC__)
#define FDK_smultt(dst, src1, src2)  "SMULTT " #dst ", " #src1 ", " #src2 "\n\t"
#else
#define FDK_smultt(dst, src1, src2)  { dst = src1 * src2 ; }
#endif

#if defined(__GNUC__)
#define FDK_smlatt(dst, src1, src2, src3)  "SMLATT " #dst ", " #src1 ", " #src2 ", " #src3 "\n\t"
#else
#define FDK_smlatt(dst, src1, src2, src3)  { dst = src1 * src2 + src3; }
#endif

#if defined(__GNUC__)
#define FDK_smull(dst1, dst2, src1, src2)  "SMULL " #dst1 ", " #dst2 ", " #src1 ", " #src2 "\n\t"
#else
#define FDK_smull(dst1, dst2, src1, src2)  { dst1 = src1 * src2 ; }
#endif

#if defined(__GNUC__)
#define FDK_smmul(dst, src1, src2)  "SMMUL " #dst ", " #src1 ", " #src2 "\n\t"
#else
#define FDK_smmul(dst, src1, src2)  { dst = src1 * src2 ; }
#endif

#if defined(__GNUC__)
#define FDK_smlal(dst, src1, src2, src3)  "SMLAL " #dst ", " #src1 ", " #src2 ", " #src3 "\n\t"
#else
#define FDK_smlal(dst, src1, src2, src3)  { dst = src1 * src2 + src3; }
#endif


#if defined(__GNUC__)
#define FDK_pkhbt_lsl_imm(Rd, Rn, Rm, imm)  "PKHBT " #Rd ", " #Rn ", " #Rm ", LSL #" #imm " \n\t"
#else
#define FDK_pkhbt_lsl_imm(Rd, Rn, Rm, imm) FDK_ASSERT(imm >= 0 && imm < 32); Rd = INT(Rn & 0x0000FFFF) | (INT((Rm << imm) & INT(0xFFFF0000)));
#endif


#if defined(__GNUC__)
#define   FDK_cmp(src1, src2)           "CMP " #src1 ", " #src2 "\n\t"
#define __FDK_cmp_imm(src, immediate)   "CMP " #src ", # " #immediate "\n\t"
#define FDK_cmp_imm(src, immediate)   __FDK_cmp_imm(src, (immediate))  
#else
#define FDK_cmp(src1, src2)          {   __FDK_coreflags_EQ = (INT(src1) == (INT)(src2)) ? 1 : 0; \
                                         __FDK_coreflags_NE = (INT(src1) != (INT)(src2)) ? 1 : 0; \
                                         __FDK_coreflags_GE = (INT(src1) >= (INT)(src2)) ? 1 : 0; \
                                         __FDK_coreflags_LT = (INT(src1) <  (INT)(src2)) ? 1 : 0; \
                                         __FDK_coreflags_GT = (INT(src1) >  (INT)(src2)) ? 1 : 0; \
                                         __FDK_coreflags_LE = (INT(src1) <= (INT)(src2)) ? 1 : 0;  }
#define FDK_cmp_imm(src, immediate)  {   __FDK_coreflags_EQ = (INT(src) == (INT)(immediate)) ? 1 : 0; \
                                         __FDK_coreflags_NE = (INT(src) != (INT)(immediate)) ? 1 : 0; \
                                         __FDK_coreflags_GE = (INT(src) >= (INT)(immediate)) ? 1 : 0; \
                                         __FDK_coreflags_LT = (INT(src) <  (INT)(immediate)) ? 1 : 0; \
                                         __FDK_coreflags_GT = (INT(src) >  (INT)(immediate)) ? 1 : 0; \
                                         __FDK_coreflags_LE = (INT(src) <= (INT)(immediate)) ? 1 : 0;  }
#endif

#if defined(__GNUC__)
#define FDK_cmn_imm(src, immediate)  "CMN " #src ", # " #immediate "\n\t"
#else
#define FDK_cmn_imm(src, immediate)  {   __FDK_coreflags_EQ = (INT(src) == (INT)(-immediate)) ? 1 : 0; \
                                         __FDK_coreflags_NE = (INT(src) != (INT)(-immediate)) ? 1 : 0; \
                                         __FDK_coreflags_GE = (INT(src) >= (INT)(-immediate)) ? 1 : 0; \
                                         __FDK_coreflags_LT = (INT(src) <  (INT)(-immediate)) ? 1 : 0; \
                                         __FDK_coreflags_GT = (INT(src) >  (INT)(-immediate)) ? 1 : 0; \
                                         __FDK_coreflags_LE = (INT(src) <= (INT)(-immediate)) ? 1 : 0;  }
#endif

#if  defined(__GNUC__)
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__)  /*  thumb* */
#define FDK_it(cond)                     "IT " #cond "\n\t"
#define FDK_itt(cond)                    "ITT " #cond "\n\t"
#else
#define FDK_it(cond)
#define FDK_itt(cond)
#endif
#else
#define FDK_it(cond)
#define FDK_itt(cond)
#endif

#if defined(__GNUC__)
#define FDK_cbz(src, label)  "CBZ " #src ", # " #label "\n\t"
#else
#define FDK_cbz(src, label)    if (!src) goto label;
#define FDK_cbnz(src, label)   if (src) goto label;
#endif

#if defined(__GNUC__)
#define FDK_return()         "BX lr \n\t"
#else
#define FDK_return()
#endif

#if defined(__GNUC__)
/* #define FDK_label(name)       "" #name "%=: \n\t" */
#define FDK_label(name)       "" #name ": \n\t"
#else
#define FDK_label(name)       name:
#endif

/* For parameter 'cond' please refer to FDK_neon_regs.h */
#if defined(__GNUC__)
/* #define FDK_branch(cond,name)        "B" #cond " " #name "%= \n\t" */ 
#define FDK_branch(cond,name)        "B" #cond " " #name " \n\t"
#else
#define FDK_branch(cond,name)        if (__FDK_coreflags_ ## cond) goto name;
#endif

#if defined(__GNUC__)
/* #define FDK_branch(cond,name)        "B" #cond " " #name "%= \n\t"  */
#define FDK_goto(name)        "B " #name " \n\t"
#else
#define FDK_goto(name)        goto name;
#endif


/* ----------------------------------------------------------------------------*/
/*  Pseudo instructions to allow embedded assembly                             */
/* ----------------------------------------------------------------------------*/


#if defined (__GNUC__)
#define FDK_ASM_ROUTINE         __attribute__((noinline))
#define FDK_INLINE_ASM_ROUTINE  __attribute__((always_inline)) static
#endif

#if defined (__GNUC__)
#define FDK_ASM_START()   __asm__ (
#define FDK_ASM_START_RETURN()  INT result;  __asm__ (
#define FDK_ASM_END()        ::: );
#define FDK_ASM_END_RETURN() "mov %0, r0;\n" : "=r"(result) :: ); return result;
#else
#define FDK_ASM_START()
#define FDK_ASM_START_RETURN()
#define FDK_ASM_END()
#define FDK_ASM_END_RETURN()
#endif


#if defined (__GNUC__)
#ifdef __llvm__
#ifdef __cplusplus
#define FDK_ASM_ROUTINE_START(proc_type, proc_name, proc_args)   \
                             extern "C" { proc_type proc_name proc_args;   }  \
                             asm ( "\n\t" \
                             ".text\n\t" \
                             "" #proc_name ": \n\t"
#else
#define FDK_ASM_ROUTINE_START(proc_type, proc_name, proc_args)   \
                             proc_type proc_name proc_args;  \
                             __asm__ ( "\n\t" \
                             ".text\n\t" \
                             "" #proc_name ": \n\t"
#endif /* __cplusplus */
#else
#ifdef __cplusplus
#define FDK_ASM_ROUTINE_START(proc_type, proc_name, proc_args)   \
                             extern "C" { proc_type proc_name proc_args;   }  \
                             asm ( "\n\t" \
                             ".section .text\n\t" \
                             "" #proc_name ": \n\t"
#else
#define FDK_ASM_ROUTINE_START(proc_type, proc_name, proc_args)   \
                             proc_type proc_name proc_args;   \
                             __asm__ ( "\n\t" \
                             ".section .text\n\t" \
                             "" #proc_name ": \n\t"
#endif /* __cplusplus */
#endif /* __llvm__ */
#define FDK_ASM_ROUTINE_END()              );
#define FDK_ASM_ROUTINE_RETURN(proc_type)  );
#define FDK_ASM_ROUTINE_RETURN_64(proc_type)  );
#else
#define FDK_ASM_ROUTINE_START(proc_type, proc_name, proc_args)   \
                              proc_type  proc_name  proc_args {
#define FDK_ASM_ROUTINE_END()              }
#define FDK_ASM_ROUTINE_RETURN(proc_type)  return (proc_type) r0; }
#define FDK_ASM_ROUTINE_RETURN_64(proc_type)  return (proc_type) ((UINT64) r1 << 32) | (UINT64) ((UINT)r0); }
#endif


#endif  /* FDK_ARM_FUNCS_H */
