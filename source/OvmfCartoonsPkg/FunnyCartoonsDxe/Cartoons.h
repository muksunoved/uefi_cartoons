/** @file
  This driver show funny cartoons via HII.

  Copyright (C) 2021, Funny Cartoons.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef _CARTOONS_H_
#define _CARTOONS_H_

//
// Macro and type definitions that connect the form with the HII driver code.
//
#define FORMSTATEID_MAIN_FORM  1
#define FORMID_MAIN_FORM       1

#define QUESTION_RES_CUR       1
#define MAXSIZE_RES_CUR       16

#define FRACTAL_KEY            1
#define DOOM_FIRE_KEY          2

#define QUESTION_SAVE_EXIT     3
#define QUESTION_DISCARD_EXIT  4

//
// This structure describes the form state. Its fields relate strictly to the
// visual widgets on the form.
//
typedef struct {
  UINT16 CurrentPreferredResolution[MAXSIZE_RES_CUR];
  UINT32 NextPreferredResolution;
} MAIN_FORM_STATE;

#endif // _CARTOONS_H_
