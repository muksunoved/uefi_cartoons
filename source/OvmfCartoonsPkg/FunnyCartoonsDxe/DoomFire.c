/** @file
  Simple math header.

  Copyright (C) 2021, Funny Cartoons.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "CartoonsPriv.h"
#include "CartoonsMath.h"

typedef struct {
    UINT8 r;
    UINT8 g;
    UINT8 b;
} RGBS;

#define PALETTE_SIZE 37

// Fire gradient palette
static const RGBS RgbsPalette[PALETTE_SIZE] = {
    { 0x07,0x07,0x07 },
    { 0x1F,0x07,0x07 },
    { 0x2F,0x0F,0x07 },
    { 0x47,0x0F,0x07 },
    { 0x57,0x17,0x07 },
    { 0x67,0x1F,0x07 },
    { 0x77,0x1F,0x07 },
    { 0x8F,0x27,0x07 },
    { 0x9F,0x2F,0x07 },
    { 0xAF,0x3F,0x07 },
    { 0xBF,0x47,0x07 },
    { 0xC7,0x47,0x07 },
    { 0xDF,0x4F,0x07 },
    { 0xDF,0x57,0x07 },
    { 0xDF,0x57,0x07 },
    { 0xD7,0x5F,0x07 },
    { 0xD7,0x5F,0x07 },
    { 0xD7,0x67,0x0F },
    { 0xCF,0x6F,0x0F },
    { 0xCF,0x77,0x0F },
    { 0xCF,0x7F,0x0F },
    { 0xCF,0x87,0x17 },
    { 0xC7,0x87,0x17 },
    { 0xC7,0x8F,0x17 },
    { 0xC7,0x97,0x1F },
    { 0xBF,0x9F,0x1F },
    { 0xBF,0x9F,0x1F },
    { 0xBF,0xA7,0x27 },
    { 0xBF,0xA7,0x27 },
    { 0xBF,0xAF,0x2F },
    { 0xB7,0xAF,0x2F },
    { 0xB7,0xB7,0x2F },
    { 0xB7,0xB7,0x37 },
    { 0xCF,0xCF,0x6F },
    { 0xDF,0xDF,0x9F },
    { 0xEF,0xEF,0xC7 },
    { 0xFF,0xFF,0xFF }
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
  IN UINT32* Buffer
        )
{
  UINTN i,j;
  UINTN coord;
  UINTN coordScale = 0;
  UINT32 color;

  for (i=0; i<FireWidth; i++)  {
      coordScale = i;
      for (j=1; j<FireHeight; j++)  {
        GenerateSprite(j*FireWidth + i);
        coord = j * FireWidth + i;
        color = RgbsPalette[FirePixels[coord]].r << 16 | RgbsPalette[FirePixels[coord]].g<<8 | RgbsPalette[FirePixels[coord]].b;

        // Draw pixel
        if (coordScale < FireWidth*FireHeight - FireWidth)  {
          Buffer[coordScale] = color & 0xFFFFFF;
          Buffer[coordScale+FireWidth] = color & 0xFFFFFF;
        }
        coordScale += FireWidth*2;
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
    GenerateFire(Buffer);

    Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);

    if (!EFI_ERROR(Status)) {
      return ;
    }
  }
}

