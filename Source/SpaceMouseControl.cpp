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

    SpaceMouseControl.cpp
    Created: 25 Jan 2021 9:12:45pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "SpaceMouseControl.h"

#if BESPOKE_SPACEMOUSE_SUPPORT
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"

#include <Windows.h>

#include "3dxware/spwmacro.h"  /* Common macros used by SpaceWare functions. */
#include "3dxware/si.h"        /* Required for any SpaceWare support within an app.*/
#include "3dxware/siapp.h"     /* Required for siapp.lib symbols */

#include "juce_core/juce_core.h"

#ifdef _MSC_VER
#pragma warning(disable:4700)
#endif

struct SpaceMouseMessageWindow::Impl
{
   Impl(ModularSynth &synth);
   ~Impl();

   static LRESULT CALLBACK MyWndCBProc(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam);

   int SbInit(HWND hwndC);

   void ApplyDeadZone(float &var, float deadzone);

   void SbMotionEvent(SiSpwEvent *pEvent);
   void SbZeroEvent();
   void SbButtonPressEvent(int buttonnumber);
   void SbButtonReleaseEvent(int buttonnumber);
   void HandleDeviceChangeEvent(SiSpwEvent *pEvent);

   LPCTSTR getClassNameFromAtom() const noexcept;

   ATOM atom;
   HWND hwnd;

   SiHdl       devHdl;       /* Handle to 3D Mouse Device */
   SiOpenData oData;
   ModularSynth& mSynth;
   bool mIsPanningOrZooming;
   bool mIsTwisting;

   static inline Impl *sInstance = nullptr;
};

SpaceMouseMessageWindow::SpaceMouseMessageWindow(ModularSynth& theSynth)
   : d(std::make_unique<Impl>(theSynth))
{
}

SpaceMouseMessageWindow::Impl::Impl(ModularSynth& theSynth)
   : mSynth(theSynth)
   , mIsPanningOrZooming(false)
   , mIsTwisting(false)
{
   sInstance = this;

   juce::String className("JUCE_");
   className << juce::String::toHexString(juce::Time::getHighResolutionTicks());

   HMODULE moduleHandle = (HMODULE)juce::Process::getCurrentModuleInstanceHandle();

   WNDCLASSEX wc = { 0 };
   wc.cbSize = sizeof(wc);
   wc.lpfnWndProc = MyWndCBProc;
   wc.cbWndExtra = 4;
   wc.hInstance = moduleHandle;
   wc.lpszClassName = (LPCSTR)className.toWideCharPointer();

   atom = RegisterClassEx(&wc);
   jassert(atom != 0);

   hwnd = CreateWindow(getClassNameFromAtom(), "SpaceMouseReader",
      0, 0, 0, 0, 0, 0, 0, moduleHandle, 0);
   jassert(hwnd != 0);

   /* Initialise 3DxWare access / call to SbInit() */
   SbInit(hwnd);

   /* Implement message loop */
   /*int bRet;
   MSG msg;      //incoming message to be evaluated
   while (bRet = GetMessage(&msg, NULL, 0, 0))
   {
      if (bRet == -1) {
         //handle the error and possibly exit
         return;
      }
      else {
         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }
   }*/
}

SpaceMouseMessageWindow::Impl::~Impl()
{
   DestroyWindow(hwnd);
   UnregisterClass(getClassNameFromAtom(), 0);
}

//static
LRESULT CALLBACK SpaceMouseMessageWindow::Impl::MyWndCBProc(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
{
   SiSpwEvent     Event;    // SpaceWare Event
   SiGetEventData EData;    // SpaceWare Event Data

   // initialize Window platform specific data for a call to SiGetEvent
   SiGetEventWinInit(&EData, wm, wParam, lParam);

   // check whether wm was a 3D mouse event and process it
   //if (SiGetEvent (devHdl, SI_AVERAGE_EVENTS, &EData, &Event) == SI_IS_EVENT)
   SpwRetVal retval = SiGetEvent(sInstance->devHdl, 0, &EData, &Event);

   if (retval == SI_IS_EVENT)
   {
      if (Event.type == SI_MOTION_EVENT)
      {
         sInstance->SbMotionEvent(&Event);        // process 3D mouse motion event
      }
      else if (Event.type == SI_ZERO_EVENT)
      {
         sInstance->SbZeroEvent();                // process 3D mouse zero event
      }
      else if (Event.type == SI_BUTTON_PRESS_EVENT)
      {
         sInstance->SbButtonPressEvent(Event.u.hwButtonEvent.buttonNumber);  // process button press event
      }
      else if (Event.type == SI_BUTTON_RELEASE_EVENT)
      {
         sInstance->SbButtonReleaseEvent(Event.u.hwButtonEvent.buttonNumber); // process button release event
      }
      else if (Event.type == SI_DEVICE_CHANGE_EVENT)
      {
         //SbHandleDeviceChangeEvent(&Event); // process 3D mouse device change event
      }
   }

   return DefWindowProc(hwnd, wm, wParam, lParam);
}

int SpaceMouseMessageWindow::Impl::SbInit(HWND hwndC)
{
   int res;                             /* result of SiOpen, to be returned  */

   /*init the SpaceWare input library */
   if (SiInitialize() == SPW_DLL_LOAD_ERROR) {
      std::cout << "Error: Could not load SiAppDll dll files" << std::endl;
   }
   else {
      //std::cout << "SiInitialize() done " << std::endl;
   }

   SiOpenWinInit(&oData, hwndC);    /* init Win. platform specific data  */

   /* open data, which will check for device type and return the device handle to be used by this function */
   if ((devHdl = SiOpen("AppSpaceMouse.exe", SI_ANY_DEVICE, SI_NO_MASK, SI_EVENT, &oData)) == NULL) {
      std::cout << "SiOpen error:" << std::endl;
      SiTerminate();  /* called to shut down the SpaceWare input library */
      std::cout << "SiTerminate()" << std::endl;
      res = 0;        /* could not open device */
      return res;
   }

   SiDeviceName pname;
   SiGetDeviceName(devHdl, &pname);
   //std::cout << "devicename =  " << pname.name << std::endl;

   //SiSetUiMode(devHdl, SI_UI_ALL_CONTROLS); /* Config SoftButton Win Display */
   SiGrabDevice(devHdl, SPW_TRUE); /* PREVENTS OTHER APPLICATIONS FROM RECEIVING 3D CONNEXION DATA !!! */
   res = 1;        /* opened device successfully */
   return res;
}

void SpaceMouseMessageWindow::Impl::ApplyDeadZone(float &var, float deadzone)
{
   if (abs(var) < deadzone)
      var = 0;
   else
      var -= (var > 0 ? 1 : -1) * deadzone;
}

void SpaceMouseMessageWindow::Impl::SbMotionEvent(SiSpwEvent *pEvent) {
   const float kMax = 2100.0f;
   float tx = ofClamp(pEvent->u.spwData.mData[SI_TX] / kMax, -1, 1);
   float ty = ofClamp(pEvent->u.spwData.mData[SI_TY] / kMax, -1, 1);
   float tz = ofClamp(pEvent->u.spwData.mData[SI_TZ] / kMax, -1, 1);
   float rx = ofClamp(pEvent->u.spwData.mData[SI_RX] / kMax, -1, 1);
   float ry = ofClamp(pEvent->u.spwData.mData[SI_RY] / kMax, -1, 1);
   float rz = ofClamp(pEvent->u.spwData.mData[SI_RZ] / kMax, -1, 1);

   float rawTwist = ry;
   float rawZoom = ty;
   ofVec2f rawPan(rz + tx, rx - tz);

   float panMag = sqrtf(tx*tx + tz * tz);
   float panAngle = atan2(-tz, tx);
   float tiltMag = sqrtf(rz*rz + rx * rx);
   float tiltAngle = atan2(rx, rz);

   const float kPanDeadZone = .15f;
   const float kTiltDeadZone = .15f;
   const float kTwistDeadZone = .15f;
   const float kZoomDownDeadZone = .2f;
   const float kZoomUpDeadZone = .05f;
   ApplyDeadZone(panMag, kPanDeadZone);
   ApplyDeadZone(tiltMag, kTiltDeadZone);
   ApplyDeadZone(ry, kTwistDeadZone);
   if (ty < 0)
      ApplyDeadZone(ty, kZoomDownDeadZone);
   else
      ApplyDeadZone(ty, kZoomUpDeadZone);

   const float kPow = 1.5f;
   panMag = pow(abs(panMag), kPow) * (panMag > 0 ? 1 : -1);
   tiltMag = pow(abs(tiltMag), kPow) * (tiltMag > 0 ? 1 : -1);
   ry = pow(abs(ry), kPow) * (ry > 0 ? 1 : -1);
   ty = pow(abs(ty), kPow) * (ty > 0 ? 1 : -1);

   tx = cos(panAngle) * panMag;
   tz = -sin(panAngle) * panMag;
   rz = cos(tiltAngle) * tiltMag;
   rx = sin(tiltAngle) * tiltMag;

   if (mIsTwisting) //if twisting, allow nothing else
      tx = ty = tz = rx = rz = 0;
   if (mIsPanningOrZooming)   //if panning or zooming, allow no twisting
      ry = 0;

   //ofLog() << "TX=" << tx << " TY=" << ty << " TZ=" << tz << " RX=" << rx << " RY=" << ry << " RZ=" << rz;

   const float kPanScale = -42;
   const float kZoomScale = -.063f;
   const float kTwistScale = -3.0f;

   bool usingPan = false;
   bool usingZoom = false;
   bool usingTwist = false;
   if (rz != 0 || tx != 0 || rx != 0 || tz != 0)
   {
      mSynth.PanView((rz + tx) * kPanScale, (rx - tz) * kPanScale);
      usingPan = true;
      mIsPanningOrZooming = true;
   }
   if (ty != 0)
   {
      mSynth.ZoomView(ty * kZoomScale, false);
      usingZoom = true;
      mIsPanningOrZooming = true;
   }
   if (ry != 0)
   {
      mSynth.MouseScrolled(0, ry * kTwistScale, false);
      usingTwist = true;
      mIsTwisting = true;
   }

   if (!mIsTwisting)
   {
      mSynth.SetRawSpaceMouseTwist(0, false);
      mSynth.SetRawSpaceMouseZoom(rawZoom, usingZoom);
      mSynth.SetRawSpaceMousePan(rawPan.x, rawPan.y, usingPan);
   }

   if (!mIsPanningOrZooming)
   {
      mSynth.SetRawSpaceMouseTwist(rawTwist, usingTwist);
      mSynth.SetRawSpaceMouseZoom(0, false);
      mSynth.SetRawSpaceMousePan(0, 0, false);
   }
}
void SpaceMouseMessageWindow::Impl::SbZeroEvent() {
   mSynth.SetRawSpaceMouseTwist(0, false);
   mSynth.SetRawSpaceMouseZoom(0, false);
   mSynth.SetRawSpaceMousePan(0, 0, false);
   mIsPanningOrZooming = false;
   mIsTwisting = false;
}
void SpaceMouseMessageWindow::Impl::SbButtonPressEvent(int buttonnumber) {
   std::cout << "Buttonnumber : " << buttonnumber << std::endl;
}
void SpaceMouseMessageWindow::Impl::SbButtonReleaseEvent(int buttonnumber) {
   std::cout << "Buttonnumber : " << buttonnumber << std::endl;
}
void SpaceMouseMessageWindow::Impl::HandleDeviceChangeEvent(SiSpwEvent *pEvent) {
   std::cout << "HandleDeviceChangeEvent : " << std::endl;

}

void SpaceMouseMessageWindow::Poll()
{
   int bRet;
   MSG msg;      //incoming message to be evaluated
   while (PeekMessage(&msg, (HWND)0, 0, 0, PM_NOREMOVE))
   {
      if (bRet = GetMessage(&msg, NULL, 0, 0))
      {
         if (bRet == -1) {
            //handle the error and possibly exit
            return;
         }
         else {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
         }
      }
      else
      {
         break;
      }
   }
}

LPCTSTR SpaceMouseMessageWindow::Impl::getClassNameFromAtom() const noexcept
{
   return (LPCTSTR)(juce::pointer_sized_uint)atom;
}

#else
SpaceMouseMessageWindow::SpaceMouseMessageWindow(ModularSynth&) {}
#endif // BESPOKE_SPACEMOUSE_SUPPORT

SpaceMouseMessageWindow::~SpaceMouseMessageWindow() = default;
