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

   LPCTSTR getClassNameFromAtom() noexcept { return (LPCTSTR)(pointer_sized_uint)atom; }
};
#endif