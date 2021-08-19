/** @file
  This driver show funny cartoons via HII.

  Copyright (C) 2021, Funny Cartoons.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "CartoonsMath.h"

STATIC
UINT32
HSVtoRGB(
  IN INT32 HValue, 
  IN INT32 SValue, 
  IN INT32 VValue
  )
{
  double h = (double)HValue / 255.0, s = (double)SValue / 255.0, v = (double)VValue / 255.0;
  double r = 0;
  double g = 0;
  double b = 0;

  if (s == 0)  {
    r = v;
    g = v;
    b = v;
  }  else  {
    double varH = h * 6;
    double varI = varH;

    Floor(&varI);

    double var1 = v * (1 - s);
    double var2 = v * (1 - (s * (varH - varI)));
    double var3 = v * (1 - (s * (1 - (varH - varI))));

    if (varI == 0) {
        r = v;
        g = var3;
        b = var1;
    } else if (varI == 1)  {
        r = var2;
        g = v;
        b = var1;
    }  else if (varI == 2)  {
        r = var1;
        g = v;
        b = var3;
    }  else if (varI == 3)  {
        r = var1;
        g = var2;
        b = v;
    }  else if (varI == 4)  {
        r = var3;
        g = var1;
        b = v;
    }  else  {
        r = v;
        g = var1;
        b = var2;
    }
  }

  return ((INT32)(r * 255) << 16) | ((INT32)(g * 255) << 8) | (INT32)(b * 255);
}

VOID 
EFIAPI
DrawFractal(
  IN INT32 ScrWidth,
  IN INT32 ScrHigh,
  IN UINT32 PixelsPerScanLine,
  IN UINT32* Buffer
  )
{
  INT32 x = 0, y = 0, w = 0, h = 0;
  UINTN coord;
  UINTN i;
  INT32 color;
  UINTN maxIterations;

  w = ScrWidth;
  h = ScrHigh;

  double cRe, cIm;
  double newRe, newIm, oldRe, oldIm;
  double zoom = 1, moveX = 0, moveY = 0;
  
  maxIterations = 300;

  cRe = -0.7;
  cIm = 0.27015;

  for(x = 0; x < w; x++)  {
    for(y = 0; y < h; y++)  {
      newRe = 1.5 * (x - w / 2) / (0.5 * zoom * w) + moveX;
      newIm = (y - h / 2) / (0.5 * zoom * h) + moveY;

      for(i = 0; i < maxIterations; i++)
      {
          oldRe = newRe;
          oldIm = newIm;
          newRe = oldRe * oldRe - oldIm * oldIm + cRe;
          newIm = 2 * oldRe * oldIm + cIm;
          if((newRe * newRe + newIm * newIm) > 4) break;
      }
      color = HSVtoRGB(i % 256, 255, 255 * (i < maxIterations));

      coord = y * PixelsPerScanLine + x;

      // Draw pixel
      Buffer[coord] = color & 0xFFFFFF;
    }
  }
}

