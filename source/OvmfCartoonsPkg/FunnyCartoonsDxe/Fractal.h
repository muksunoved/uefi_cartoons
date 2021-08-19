/** @file
  Simple math header.

  Copyright (C) 2021, Funny Cartoons.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef _FRACTAL_H_
#define _FRACTAL_H_


VOID 
EFIAPI
DrawFractal(
  IN INT32 ScrWidth,
  IN INT32 ScrHigh,
  IN UINT32 PixelsPerScanLine,
  IN UINT32* Buffer
  );

#endif
