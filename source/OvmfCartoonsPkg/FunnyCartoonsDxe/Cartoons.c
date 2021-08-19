/** @file
  This driver show funny cartoons via HII.

  Copyright (C) 2021, Funny Cartoons.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/HiiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiHiiServicesLib.h>
#include <Protocol/DevicePath.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiConfigAccess.h>
#include <Guid/MdeModuleHii.h>
#include <Guid/OvmfPlatformConfig.h>
#include <Library/UefiLib.h>

#include "Cartoons.h"
#include "Fractal.h"

//
// The HiiAddPackages() library function requires that any controller (or
// image) handle, to be associated with the HII packages under installation, be
// "decorated" with a device path. The tradition seems to be a vendor device
// path.
//
// We'd like to associate our HII packages with the driver's image handle. The
// first idea is to use the driver image's device path. Unfortunately, loaded
// images only come with an EFI_LOADED_IMAGE_DEVICE_PATH_PROTOCOL (not the
// usual EFI_DEVICE_PATH_PROTOCOL), ie. a different GUID. In addition, even the
// EFI_LOADED_IMAGE_DEVICE_PATH_PROTOCOL interface may be NULL, if the image
// has been loaded from an "unnamed" memory source buffer.
//
// Hence let's just stick with the tradition -- use a dedicated vendor device
// path, with the driver's FILE_GUID.
//
#pragma pack(1)
typedef struct {
  VENDOR_DEVICE_PATH       VendorDevicePath;
  EFI_DEVICE_PATH_PROTOCOL End;
} PKG_DEVICE_PATH;
#pragma pack()

STATIC PKG_DEVICE_PATH mPkgDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)     ),
        (UINT8) (sizeof (VENDOR_DEVICE_PATH) >> 8)
      }
    },
    EFI_CALLER_ID_GUID
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      (UINT8) (END_DEVICE_PATH_LENGTH     ),
      (UINT8) (END_DEVICE_PATH_LENGTH >> 8)
    }
  }
};

//
// The configuration interface between the HII engine (form display etc) and
// this driver.
//
STATIC EFI_HII_CONFIG_ACCESS_PROTOCOL mConfigAccess;

//
// The handle representing our list of packages after installation.
//
STATIC EFI_HII_HANDLE mInstalledPackages;

//
// The arrays below constitute our HII package list. They are auto-generated by
// the VFR compiler and linked into the driver image during the build.
//
// - The strings package receives its C identifier from the driver's BASE_NAME,
//   plus "Strings".
//
// - The forms package receives its C identifier from the VFR file's basename,
//   plus "Bin".
//
//
extern UINT8 FunnyCartoonsDxeStrings[];
extern UINT8 CartoonsFormsBin[];

//
// We want to be notified about GOP installations until we find one GOP
// interface that lets us populate the form.
//
STATIC EFI_EVENT mGopEvent;

//
// The registration record underneath this pointer allows us to iterate through
// the GOP instances one by one.
//
STATIC VOID *mGopTracker;

//
// Cache the resolutions we get from the GOP.
//
typedef struct {
  UINT32 X;
  UINT32 Y;
} GOP_MODE;

STATIC UINTN    mNumGopModes;
STATIC GOP_MODE *mGopModes;


/**
  This function is called by the HII machinery when it fetches the form state.

  See the precise documentation in the UEFI spec.

  @param[in]  This      The Config Access Protocol instance.

  @param[in]  Request   A <ConfigRequest> format UCS-2 string describing the
                        query.

  @param[out] Progress  A pointer into Request on output, identifying the query
                        element where processing failed.

  @param[out] Results   A <MultiConfigAltResp> format UCS-2 string that has
                        all values filled in for the names in the Request
                        string.

  @retval EFI_SUCCESS  Extraction of form state in <MultiConfigAltResp>
                       encoding successful.
  @return              Status codes from underlying functions.

**/
STATIC
EFI_STATUS
EFIAPI
ExtractCartoonsConfig (
  IN CONST  EFI_HII_CONFIG_ACCESS_PROTOCOL  *This,
  IN CONST  EFI_STRING                      Request,
  OUT       EFI_STRING                      *Progress,
  OUT       EFI_STRING                      *Results
)
{
  return EFI_UNSUPPORTED;
}

/**
  This function is called by the HII machinery when it wants the driver to
  interpret and persist the form state.

  See the precise documentation in the UEFI spec.

  @param[in]  This           The Config Access Protocol instance.

  @param[in]  Configuration  A <ConfigResp> format UCS-2 string describing the
                             form state.

  @param[out] Progress       A pointer into Configuration on output,
                             identifying the element where processing failed.

  @retval EFI_SUCCESS  Configuration verified, state permanent.

  @return              Status codes from underlying functions.
**/
STATIC
EFI_STATUS
EFIAPI
RouteCartoonsConfig (
  IN CONST  EFI_HII_CONFIG_ACCESS_PROTOCOL  *This,
  IN CONST  EFI_STRING                      Configuration,
  OUT       EFI_STRING                      *Progress
)
{
  return EFI_UNSUPPORTED;
}

STATIC
EFI_STATUS
EFIAPI
GetGopSettings (
  OUT INT32*  ScrWidth,
  OUT INT32*  ScrHigh,
  OUT UINT32* PixelsPerScanLine,
  OUT UINT32** Buffer
  )
{
  
  EFI_STATUS                   Status;
  EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;

  Status = gBS->LocateProtocol (&gEfiGraphicsOutputProtocolGuid, mGopTracker,
                    (VOID **) &Gop);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  *ScrHigh  = Gop->Mode->Info->VerticalResolution;
  *ScrWidth = Gop->Mode->Info->HorizontalResolution;
  *PixelsPerScanLine = Gop->Mode->Info->PixelsPerScanLine;
  *Buffer = (UINT32*)(UINTN)Gop->Mode->FrameBufferBase;

  return Status;
}    

VOID 
EFIAPI
CleanScreen(
  IN INT32 ScrWidth,
  IN INT32 ScrHigh,
  IN UINT32 PixelsPerScanLine,
  IN UINT32* Buffer
  )
{
  INT32 x = 0, y = 0, w = 0, h = 0;
  UINTN coord;

  w = ScrWidth;
  h = ScrHigh;

  for(x = 0; x < w; x++)  {
    for(y = 0; y < h; y++)  {
      coord = y * PixelsPerScanLine + x;
      Buffer[coord] = 0x0;
    }
  }
}

STATIC
EFI_STATUS
EFIAPI
CartoonsCallback (
  IN     CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN     EFI_BROWSER_ACTION                     Action,
  IN     EFI_QUESTION_ID                        QuestionId,
  IN     UINT8                                  Type,
  IN OUT EFI_IFR_TYPE_VALUE                     *Value,
  OUT    EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
{
  INT32             ScrWidth = 0;
  INT32             ScrHigh = 0;
  UINT32            PixelsPerScanLine = 0;
  UINT32*           Buffer = NULL;
  EFI_STATUS        Status;
  EFI_INPUT_KEY     Key;
  CHAR16                        *StringBuffer1;
  CHAR16                        *StringBuffer2;

  UINTN                         Size;

  Size = 200;
  StringBuffer1 = AllocateZeroPool (Size * sizeof(CHAR16));
  ASSERT (StringBuffer1 != NULL);
  StringBuffer2 = AllocateZeroPool (Size * sizeof(CHAR16));
  ASSERT (StringBuffer2 != NULL);

  DEBUG ((DEBUG_VERBOSE, "%a: Action=0x%Lx QuestionId=%d Type=%d\n",
    __FUNCTION__, (UINT64) Action, QuestionId, Type));

  if (Action != EFI_BROWSER_ACTION_CHANGED) {
    return EFI_UNSUPPORTED;
  }

  switch (QuestionId) {
  case QUESTION_SAVE_EXIT:
    *ActionRequest = EFI_BROWSER_ACTION_REQUEST_FORM_SUBMIT_EXIT;
    break;

  case FRACTAL_KEY:
    Status = GetGopSettings(&ScrWidth, &ScrHigh, &PixelsPerScanLine, &Buffer);

    if (EFI_ERROR (Status))  {
      StrCpyS (StringBuffer1, Size, L"Eroor get GOP settings ");
      StrCpyS (StringBuffer2, Size, L"Enter (YES)  /   Esc (NO)");

      // 
      // Popup a menu to notice user
      //
      do {
        CreatePopUp (EFI_LIGHTGRAY | EFI_BACKGROUND_BLUE, &Key, StringBuffer1, StringBuffer2, NULL);      
      } while ((Key.ScanCode != SCAN_ESC) && (Key.UnicodeChar != CHAR_CARRIAGE_RETURN));

      *ActionRequest = EFI_BROWSER_ACTION_REQUEST_NONE;
      return EFI_SUCCESS;
    }

    CleanScreen(ScrWidth,ScrHigh, PixelsPerScanLine, Buffer);
    DrawFractal(ScrWidth,ScrHigh, PixelsPerScanLine, Buffer);

    
    for (;;)  {
      gBS->Stall((UINTN)(65536 / (105. / 88.)));  
      
      Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
      if (!EFI_ERROR(Status)) {
        CleanScreen(ScrWidth,ScrHigh, PixelsPerScanLine, Buffer);
        *ActionRequest = EFI_BROWSER_ACTION_REQUEST_NONE;
        return EFI_SUCCESS;
      }
    }

    *ActionRequest = EFI_BROWSER_ACTION_REQUEST_FORM_SUBMIT_EXIT;
    break;

  default:
    break;
  }

  return EFI_SUCCESS;
}


/**
  Query and save all resolutions supported by the GOP.

  @param[in]  Gop          The Graphics Output Protocol instance to query.

  @param[out] NumGopModes  The number of modes supported by the GOP. On output,
                           this parameter will be positive.

  @param[out] GopModes     On output, a dynamically allocated array containing
                           the resolutions returned by the GOP. The caller is
                           responsible for freeing the array after use.

  @retval EFI_UNSUPPORTED       No modes found.
  @retval EFI_OUT_OF_RESOURCES  Failed to allocate GopModes.
  @return                       Error codes from Gop->QueryMode().

**/
STATIC
EFI_STATUS
EFIAPI
QueryGopModes (
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop,
  OUT UINTN                        *NumGopModes,
  OUT GOP_MODE                     **GopModes
  )
{
  EFI_STATUS Status;
  UINT32     ModeNumber;

  if (Gop->Mode->MaxMode == 0) {
    return EFI_UNSUPPORTED;
  }
  *NumGopModes = Gop->Mode->MaxMode;

  *GopModes = AllocatePool (Gop->Mode->MaxMode * sizeof **GopModes);
  if (*GopModes == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  for (ModeNumber = 0; ModeNumber < Gop->Mode->MaxMode; ++ModeNumber) {
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
    UINTN                                SizeOfInfo;

    Status = Gop->QueryMode (Gop, ModeNumber, &SizeOfInfo, &Info);
    if (EFI_ERROR (Status)) {
      goto FreeGopModes;
    }

    (*GopModes)[ModeNumber].X = Info->HorizontalResolution;
    (*GopModes)[ModeNumber].Y = Info->VerticalResolution;
    FreePool (Info);
  }

  return EFI_SUCCESS;

FreeGopModes:
  FreePool (*GopModes);

  return Status;
}


/**
  Notification callback for GOP interface installation.

  @param[in] Event    Event whose notification function is being invoked.

  @param[in] Context  The pointer to the notification function's context, which
                      is implementation-dependent.
**/
STATIC
VOID
EFIAPI
GopInstalled (
  IN EFI_EVENT Event,
  IN VOID      *Context
  )
{
  EFI_STATUS                   Status;
  EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;

  ASSERT (Event == mGopEvent);

  //
  // Check further GOPs.
  //
  for (;;) {
    mNumGopModes = 0;
    mGopModes = NULL;

    Status = gBS->LocateProtocol (&gEfiGraphicsOutputProtocolGuid, mGopTracker,
                    (VOID **) &Gop);
    if (EFI_ERROR (Status)) {
      return;
    }

    Status = QueryGopModes (Gop, &mNumGopModes, &mGopModes);
    if (EFI_ERROR (Status)) {
      continue;
    }

    break;
  }

  //
  // Success -- so uninstall this callback. Closing the event removes all
  // pending notifications and all protocol registrations.
  //
  Status = gBS->CloseEvent (mGopEvent);
  ASSERT_EFI_ERROR (Status);
  mGopEvent = NULL;
  mGopTracker = NULL;
}


/**
  Entry point for this driver.

  @param[in] ImageHandle  Image handle of this driver.
  @param[in] SystemTable  Pointer to SystemTable.

  @retval EFI_SUCESS            Driver has loaded successfully.
  @retval EFI_OUT_OF_RESOURCES  Failed to install HII packages.
  @return                       Error codes from lower level functions.

**/
EFI_STATUS
EFIAPI
CartoonsInit (
  IN  EFI_HANDLE        ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS Status;

  mConfigAccess.ExtractConfig = &ExtractCartoonsConfig;
  mConfigAccess.RouteConfig   = &RouteCartoonsConfig;
  mConfigAccess.Callback      = &CartoonsCallback;

  //
  // Declare ourselves suitable for HII communication.
  //
  Status = gBS->InstallMultipleProtocolInterfaces (&ImageHandle,
                  &gEfiDevicePathProtocolGuid,      &mPkgDevicePath,
                  &gEfiHiiConfigAccessProtocolGuid, &mConfigAccess,
                  NULL);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Publish the HII package list to HII Database.
  //
  mInstalledPackages = HiiAddPackages (
                         &gEfiCallerIdGuid,  // PackageListGuid
                         ImageHandle,        // associated DeviceHandle
                         FunnyCartoonsDxeStrings, // 1st package
                         CartoonsFormsBin,   // 2nd package
                         NULL                // terminator
                         );
  if (mInstalledPackages == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto UninstallProtocols;
  }

  Status = gBS->CreateEvent (EVT_NOTIFY_SIGNAL, TPL_CALLBACK, &GopInstalled,
                  NULL /* Context */, &mGopEvent);
  if (EFI_ERROR (Status)) {
    goto RemovePackages;
  }

  Status = gBS->RegisterProtocolNotify (&gEfiGraphicsOutputProtocolGuid,
                  mGopEvent, &mGopTracker);
  if (EFI_ERROR (Status)) {
    goto CloseGopEvent;
  }

  //
  // Check already installed GOPs.
  //
  Status = gBS->SignalEvent (mGopEvent);
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;

CloseGopEvent:
  gBS->CloseEvent (mGopEvent);

RemovePackages:
  HiiRemovePackages (mInstalledPackages);

UninstallProtocols:
  gBS->UninstallMultipleProtocolInterfaces (ImageHandle,
         &gEfiDevicePathProtocolGuid,      &mPkgDevicePath,
         &gEfiHiiConfigAccessProtocolGuid, &mConfigAccess,
         NULL);
  return Status;
}

/**
  Unload the driver.

  @param[in]  ImageHandle  Handle that identifies the image to evict.

  @retval EFI_SUCCESS  The image has been unloaded.
**/
EFI_STATUS
EFIAPI
CartoonsUnload (
  IN  EFI_HANDLE  ImageHandle
  )
{
  if (mGopEvent == NULL) {
    //
    // The GOP callback ran successfully and unregistered itself. Release the
    // resources allocated there.
    //
    ASSERT (mGopModes != NULL);
    FreePool (mGopModes);
  } else {
    //
    // Otherwise we need to unregister the callback.
    //
    ASSERT (mGopModes == NULL);
    gBS->CloseEvent (mGopEvent);
  }

  //
  // Release resources allocated by the entry point.
  //
  HiiRemovePackages (mInstalledPackages);
  gBS->UninstallMultipleProtocolInterfaces (ImageHandle,
         &gEfiDevicePathProtocolGuid,      &mPkgDevicePath,
         &gEfiHiiConfigAccessProtocolGuid, &mConfigAccess,
         NULL);
  return EFI_SUCCESS;
}
