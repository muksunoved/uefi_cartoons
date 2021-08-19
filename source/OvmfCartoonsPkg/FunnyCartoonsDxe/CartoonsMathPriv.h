/** @file
  Simple math header.

  Copyright (C) 2021, Funny Cartoons.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef _CARTOONS_MATH_PRIV_H_
#define _CARTOONS_MATH_PRIV_H_


#if (defined __BYTE_ORDER__ && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) \
    || ((__i386 || __i386__ || _M_IX86 || ECB_GCC_AMD64 || ECB_MSVC_AMD64) && !__VOS__)
  typedef union  {
    double value;
    struct  {
      UINT32 lsw;
      UINT32 msw;
    } parts;
  } ieee_double_shape_type;
#elif (defined __BYTE_ORDER__ && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) \
      || ((__AARCH64EB__ || __MIPSEB__ || __ARMEB__) && !__VOS__)
  typedef union  {
    double value;
    struct  {
      UINT32 msw;
      UINT32 lsw;
    } parts;
  } ieee_double_shape_type;
#endif


// Get two 32 bit ints from a double.  

#define EXTRACT_WORDS(ix0,ix1,d)       \
do {                \
  ieee_double_shape_type ew_u;          \
  ew_u.value = (d);           \
  (ix0) = ew_u.parts.msw;     \
  (ix1) = ew_u.parts.lsw;     \
} while (0)

#define INSERT_WORDS(d,ix0,ix1)         \
do {                \
  ieee_double_shape_type iw_u;          \
  iw_u.parts.msw = (ix0);         \
  iw_u.parts.lsw = (ix1);         \
  (d) = iw_u.value;           \
} while (0)

#endif // _CARTOONS_MATH_PRIV_H_
