/** @file
  Simple math header.

  Copyright (C) 2021, Funny Cartoons.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef _CARTOONS_MATH_H_
#define _CARTOONS_MATH_H_

#include "CartoonsMathPriv.h"

__attribute__((always_inline)) static inline void 
Floor(
  double *x
 )  
{
  INT32 i0,i1,j0;
  UINT32 i,j;
  double huge = 1.0e300;

  EXTRACT_WORDS(i0,i1,*x);

  j0 = ((i0>>20)&0x7ff) - 0x3ff;

  if ( j0 < 20 ) {
    if ( j0 < 0 ) {  /* raise inexact if x != 0 */
      if ( huge + *x > 0.0) {/* return 0*sign(x) if |x|<1 */
        if( i0 >= 0 ) { 
            i0=i1=0;
        } else if ((( i0&0x7fffffff)|i1)!=0) { 
            i0=0xbff00000;i1=0;
        }
      }
    } else {
        i = (0x000fffff)>>j0;
        if(((i0&i)|i1)==0) return ; /* x is integral */
        if(huge + *x > 0.0) {  /* raise inexact flag */
          if(i0<0) i0 += (0x00100000)>>j0;
          i0 &= (~i); i1=0;
        }
      }
  } else if (j0>51) {
      if(j0==0x400) { *x += *x; return ; } /* inf or NaN */
      else return ;    /* x is integral */
  } else {
      i = ((UINT32)(0xffffffff))>>(j0-20);
      if((i1&i)==0) return ; /* x is integral */
      if(huge + *x>0.0) {    /* raise inexact flag */
    if(i0<0) {
        if(j0==20) i0+=1;
        else {
      j = i1+(1<<(52-j0));
      if((UINT32)j<i1) i0 +=1 ;   /* got a carry */
      i1=j;
        }
    }
    i1 &= (~i);
      }
  }

  INSERT_WORDS(*x,i0,i1);

  return ;
}

#endif // _CARTOONS_MATH_H_
