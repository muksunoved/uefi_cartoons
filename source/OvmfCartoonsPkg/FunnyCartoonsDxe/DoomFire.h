/** @file
  Doom Fire generator header.

  Copyright (C) 2021, Funny Cartoons.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef _DOOM_FIRE_H_
#define _DOOM_FIRE_H_


VOID 
EFIAPI
DrawDoomFire (
  IN INT32 ScrWidth,
  IN INT32 ScrHigh,
  IN UINT32 PixelsPerScanLine,
  IN UINT32* Buffer
  );

#endif // _FRACTAL_H_
