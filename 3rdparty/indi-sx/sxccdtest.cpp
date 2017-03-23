/*
 Starlight Xpress CCD INDI Driver

 Copyright (c) 2012-2013 Cloudmakers, s. r. o.
 All Rights Reserved.

 Code is based on SX INDI Driver by Gerry Rozema and Jasem Mutlaq
 Copyright(c) 2010 Gerry Rozema.
 Copyright(c) 2012 Jasem Mutlaq.
 All rights reserved.

 This program is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the Free
 Software Foundation; either version 2 of the License, or (at your option)
 any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 more details.

 You should have received a copy of the GNU General Public License along with
 this program; if not, write to the Free Software Foundation, Inc., 59
 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

 The full GNU General Public License is included in this distribution in the
 file called LICENSE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <iostream>

#include "sxconfig.h"
#include "sxccdusb.h"

using namespace std;

int n;
DEVICE devices[20];
const char* names[20];

HANDLE handles[20];
struct t_sxccd_params params;

#define NX  10
#define NY  10
unsigned short pixels[NX*NY];

int use_P2 = 1;   // set to 0 if you don't want the P2 header info for a PGM header descriptor.

int main() {
  int i;
  unsigned int ui;
  unsigned short us;
  unsigned long ul;

  sxDebug(true);

  cout << "sx_ccd_test version " << VERSION_MAJOR << "." << VERSION_MINOR << endl << endl;
  n = sxList(devices, names, 20);
  cout << "sxList() -> " << n << endl << endl;

  for (int j = 0; j < n; j++) {
    HANDLE handle;

    cout << "testing " << names[j] << " -----------------------------------" << endl << endl;

    i = sxOpen(devices[j], &handle);
    cout << "sxOpen() -> " << i << endl << endl;
    cout << "  handle:" << handle << endl;

    //i = sxReset(handle);
    //cout << "sxReset() -> " << i << endl << endl;

    us = sxGetCameraModel(handle);
    cout << "sxGetCameraModel() -> " << us << endl << endl;

    //ul = sxGetFirmwareVersion(handle);
    //cout << "sxGetFirmwareVersion() -> " << ul << endl << endl;

    //us = sxGetBuildNumber(handle);
    //cout << "sxGetBuildNumber() -> " << us << endl << endl;

    memset(&params, 0, sizeof(params));
    i = sxGetCameraParams(handle, 0, &params);
    cout << "sxGetCameraParams(..., 0,...) -> " << i << endl << endl;


    i = sxSetTimer(handle, 900);
    cout << "sxSetTimer(900) -> " << i << endl << endl;

    while ((i = sxGetTimer(handle))>0) {
      cout << "sxGetTimer() -> " << i << endl << endl;
      sleep(1);
    }
    cout << "sxGetTimer() -> " << i << endl << endl;

    if (params.extra_caps & SXUSB_CAPS_SHUTTER) {
      i = sxSetShutter(handle, 0);
      cout << "sxSetShutter(0) -> " << i << endl << endl;
      sleep(1);
      i = sxSetShutter(handle, 1);
      cout << "sxSetShutter(1) -> " << i << endl << endl;
    }

    if (params.extra_caps & SXUSB_CAPS_COOLER) {
      unsigned short int temp;
      unsigned char status;
      i = sxSetCooler(handle, 1, (-10 + 273) * 10, &status, &temp);
      cout << "sxSetCooler() -> " << i << endl << endl;
    }

    i = sxClearPixels(handle, 0, 0);
    cout << "sxClearPixels(..., 0) -> " << i << endl << endl;

    usleep(1000);
    //                             x0 y0   W   H xb yb
    i = sxLatchPixels(handle, 0, 0, 0, 0, NX, NY, 1, 1);
    cout << "sxLatchPixels(..., 0, ...) -> " << i << endl << endl;

    i = sxReadPixels(handle, pixels, 2*NX*NY);     //  16bit/pixel data (camindex = 0)
    cout << "sxReadPixels() -> " << i << endl << endl;

    if (use_P2) {
      cout << "P2" << endl;       
      cout << NX << " " << NY << endl;
      cout << "65535" << endl;
    }

    for (int y=0; y<NY; y++) {
      for (int x=0; x<NX; x++)
        cout << pixels[x*NY+y] << " ";
      cout << endl;
    }
    cout << endl;
    if (use_P2) cout << "P2-end" << endl;

    if (params.extra_caps & SXCCD_CAPS_GUIDER) {
      memset(&params, 0, sizeof(params));
      i = sxGetCameraParams(handle, 1, &params);
      cout << "sxGetCameraParams(..., 1,...) -> " << i << endl << endl;

      i = sxClearPixels(handle, 0, 1);
      cout << "sxClearPixels(..., 1) -> " << i << endl << endl;

      usleep(1000);

      i = sxLatchPixels(handle, 0, 1, 0, 0, NX, NY, 1, 1);
      cout << "sxLatchPixels(..., 1, ...) -> " << i << endl << endl;

      i = sxReadPixels(handle, pixels, NX*NY);   //  8bit/pixel guider head (camindex = 1)
      cout << "sxReadPixels() -> " << i << endl << endl;

      for (int y=0; y<NY; y++) {
	for (int x=0; x<NX; x++)
          cout << pixels[x*NY+y] << " ";
        cout << endl;
      }
      cout << endl;
    }

    sxClose(&handle);
    cout << "sxClose() " << endl << endl;
  }
}

