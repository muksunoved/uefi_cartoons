/** @file
  Simple math header.

  Copyright (C) 2021, Funny Cartoons.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "CartoonsPriv.h"
#include "CartoonsMath.h"


#define PALETTE_SIZE 37

// Fire gradient palette
static const UINT32 RgbsPalette[PALETTE_SIZE] = {
     0x070707,
     0x1F0707,
     0x2F0F07,
     0x470F07,
     0x571707,
     0x671F07,
     0x771F07,
     0x8F2707,
     0x9F2F07,
     0xAF3F07,
     0xBF4707,
     0xC74707,
     0xDF4F07,
     0xDF5707,
     0xDF5707,
     0xD75F07,
     0xD75F07,
     0xD7670F,
     0xCF6F0F,
     0xCF770F,
     0xCF7F0F,
     0xCF8717,
     0xC78717,
     0xC78F17,
     0xC7971F,
     0xBF9F1F,
     0xBF9F1F,
     0xBFA727,
     0xBFA727,
     0xBFAF2F,
     0xB7AF2F,
     0xB7B72F,
     0xB7B737,
     0xCFCF6F,
     0xDFDF9F,
     0xEFEFC7,
     0xFFFFFF
};

static UINT8 *FirePixels;
static INT32 FireWidth;
static INT32 FireHeight;

STATIC
VOID
InitFirePixels(
  IN INT32 ScrWidth,
  IN INT32 ScrHeight
        )
{
  UINTN i;
  FireWidth  = ScrWidth;
  FireHeight = ScrHeight;
  FirePixels = NULL;

  FirePixels = AllocateZeroPool (FireHeight * FireWidth);  
  ASSERT (FirePixels != NULL);

  for (i=0; i<FireWidth; i++)  {
      FirePixels[(FireHeight - 1)*FireWidth + i] = PALETTE_SIZE - 1;
  }
  Srand(0x0F0F0F0F);
}

STATIC
VOID
GenerateSprite(
  UINTN Order
  )
{
    UINTN RandNum;
    UINTN NewOrder;
    double Divisor = 2147483647.5;
    double Result;

    UINT8 GradientLevel = FirePixels[Order];

    if (GradientLevel == 0 && Order >= FireWidth) {
        FirePixels[Order - FireWidth] = 0;

    }  else if (GradientLevel != 0 && Order >= FireWidth) {
        Result = (double)Rand() / Divisor * 4.0;
        RandNum = Result;
        NewOrder = Order - RandNum + 1;
        FirePixels[NewOrder - FireWidth] = GradientLevel - (RandNum & 1);
    }
}    

STATIC
VOID
GenerateFire(
  IN UINT32* Buffer,
  IN UINT32 PixelsPerScanLine
        )
{
  UINT32 x,y, CoordScale, Coord;
  UINT32 Color;

  for (x=0; x<FireWidth; ++x)  {
    CoordScale = x;

    for (y=1; y<FireHeight; ++y)  {
      GenerateSprite(y*FireWidth + x);
    }
  }

  for (x=0; x<FireWidth; ++x)  {
    CoordScale = x;

    for (y=1; y<FireHeight; ++y)  {
      Coord = y * FireWidth + x;
      Color = RgbsPalette[FirePixels[Coord]];
      Color &= 0xFFFFFF;

      // Draw color to 2 vertical pixels
      if (CoordScale < (FireWidth*FireHeight - FireWidth-1) && CoordScale >=0 )  {
        Buffer[CoordScale] = Color;
        CoordScale += FireWidth;
        Buffer[CoordScale] = Color;
        //CoordScale += FireWidth;
      }
    }
  }
}


VOID 
EFIAPI
DrawDoomFire(
  IN INT32 ScrWidth,
  IN INT32 ScrHeight,
  IN UINT32 PixelsPerScanLine,
  IN UINT32* Buffer
  )
{
  EFI_STATUS        Status;
  EFI_INPUT_KEY     Key;

  InitFirePixels(ScrWidth, ScrHeight);

  for (;;)  {
    GenerateFire(Buffer, PixelsPerScanLine);

    Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);

    if (!EFI_ERROR(Status)) {
      return ;
    }
  }
}

