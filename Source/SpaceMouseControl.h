/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
/*
  ==============================================================================

    SpaceMouseControl.h
    Created: 25 Jan 2021 9:12:45pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#ifdef BESPOKE_WINDOWS
#include "ModularSynth.h"

#include <Windows.h>

#include "3DxWare/Inc/spwmacro.h"  /* Common macros used by SpaceWare functions. */
#include "3DxWare/Inc/si.h"        /* Required for any SpaceWare support within an app.*/
#include "3DxWare/Inc/siapp.h"     /* Required for siapp.lib symbols */

#pragma warning(disable:4700)

//==============================================================================
class SpaceMouseMessageWindow
{
public:
   static SpaceMouseMessageWindow* sInstance;

   SpaceMouseMessageWindow(ModularSynth* theSynth);
   ~SpaceMouseMessageWindow();

   void Poll();

   static LRESULT CALLBACK MyWndCBProc(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam);

   int SbInit(HWND hwndC);

   void ApplyDeadZone(float &var, float deadzone);

   void SbMotionEvent(SiSpwEvent *pEvent);
   void SbZeroEvent();
   void SbButtonPressEvent(int buttonnumber);
   void SbButtonReleaseEvent(int buttonnumber);
   void HandleDeviceChangeEvent(SiSpwEvent *pEvent);      

   inline HWND getHWND() const noexcept { return hwnd; }

private:
   ATOM atom;
   HWND hwnd;

   SiHdl       devHdl;       /* Handle to 3D Mouse Device */
   SiOpenData oData;
   ModularSynth* mSynth;
   bool mIsPanningOrZooming;
   bool mIsTwisting;

   LPCTSTR getClassNameFromAtom() const noexcept;
};
#endif