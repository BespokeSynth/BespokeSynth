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

    Push2Control.cpp
    Created: 24 Feb 2020 8:57:57pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "juce_opengl/juce_opengl.h"
using namespace juce::gl;

#include "Push2Control.h"
#include <cctype>
#include "SynthGlobals.h"
#include "nanovg/nanovg.h"
#define NANOVG_GLES2_IMPLEMENTATION
#include "nanovg/nanovg_gl.h"
#include "nanovg/nanovg_gl_utils.h"
#include "OpenFrameworksPort.h"
#include "UIControlMacros.h"
#include "TitleBar.h"
#include "ModuleSaveDataPanel.h"
#include "QuickSpawnMenu.h"
#include "PatchCableSource.h"
#include "DropdownList.h"
#include "FloatSliderLFOControl.h"
#include "UserPrefsEditor.h"
#include "push2/JuceToPush2DisplayBridge.h"
#include "push2/Push2-Bitmap.h"

bool Push2Control::sDrawingPush2Display = false;
NVGcontext* Push2Control::sVG = nullptr;
NVGLUframebuffer* Push2Control::sFB = nullptr;
IUIControl* Push2Control::sBindToUIControl = nullptr;
namespace {
   ableton::Push2DisplayBridge ThePushBridge; // The bridge allowing to use juce::graphics for push
}

//https://raw.githubusercontent.com/Ableton/push-interface/master/doc/MidiMapping.png

#include "leathers/push"
#include "leathers/unused-variable"
namespace
{
   const int kTapTempoButton = 3;
   const int kMetronomeButton = 9;
   const int kNewButton = 87;
   const int kDeleteButton = 118;
   const int kAutomateButton = 89;
   const int kCircleButton = 86;
   const int kAddDeviceButton = 52;
   const int kAboveScreenButtonRow = 102;
   const int kBelowScreenButtonRow = 20;
   const int kUpButton = 46;
   const int kDownButton = 47;
   const int kLeftButton = 44;
   const int kRightButton = 45;
   const int kNoteButton = 50;
   const int kSessionButton = 51;
   const int kPageLeftButton = 62;
   const int kPageRightButton = 63;
   const int kMasterButton = 28;
   const int kQuantizeButtonSection = 36;
   const int kNumQuantizeButtons = 8;
   const int kSetupButton = 30;
   const int kUserButton = 59;
}
#include "leathers/pop"

Push2Control::Push2Control()
: mDisplayModule(nullptr)
, mDevice(this)
, mModuleColumnOffset(0)
, mModuleColumnOffsetSmoothed(0)
, mModuleListOffset(0)
, mModuleListOffsetSmoothed(0)
, mNewButtonHeld(false)
, mDeleteButtonHeld(false)
, mModulationButtonHeld(false)
, mAddModuleBookmarkButtonHeld(false)
, mHeldModule(nullptr)
, mAllowRepatch(false)
, mModuleHistoryPosition(-1)
, mInMidiControllerBindMode(false)
, mScreenDisplayMode(ScreenDisplayMode::kNormal)
, mGridControlModule(nullptr)
, mDisplayModuleCanControlGrid(false)
, mSpawnLists(this)
, mSelectedGridSpawnListIndex(-1)
{
   Initialize();
   for (int i=0; i<128*2; ++i)
      mLedState[i] = -1;
   for (int i=0; i<8*8; ++i)
      mModuleGrid[i] = nullptr;
   for (int i=0; i<128; ++i)
      mNoteHeldState[i] = 0;
   for (int i = 0; i < kNumQuantizeButtons; ++i)
      mBookmarkSlots.push_back(nullptr);
}

Push2Control::~Push2Control()
{
   for (int i=0; i<128; ++i)
   {
      SetLed(kMidiMessage_Note, i, 0);
      SetLed(kMidiMessage_Control, i, 0);
   }
}

void Push2Control::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   //UIBLOCK0();
   //ENDUIBLOCK(mWidth, mHeight);
   mWidth = 100;
   mHeight = 20;
   
   mSpawnLists.SetModuleFactory(TheSynth->GetModuleFactory());
   mSpawnLists.mNoteModules.GetList()->SetMaxPerColumn(9999);
   for (int i = 0; i < mSpawnLists.GetDropdowns().size(); ++i)
   {
      mSpawnLists.GetDropdowns()[i]->GetList()->SetWidth(100);
      mSpawnLists.GetDropdowns()[i]->GetList()->SetPosition(-999, -999);
   }
   mSpawnModuleControls.push_back(mSpawnLists.mInstrumentModules.GetList());
   mSpawnModuleControls.push_back(mSpawnLists.mNoteModules.GetList());
   mSpawnModuleControls.push_back(mSpawnLists.mSynthModules.GetList());
   mSpawnModuleControls.push_back(mSpawnLists.mAudioModules.GetList());
   mSpawnModuleControls.push_back(mSpawnLists.mModulatorModules.GetList());
   mSpawnModuleControls.push_back(mSpawnLists.mPulseModules.GetList());
   mSpawnModuleControls.push_back(mSpawnLists.mOtherModules.GetList());
   mSpawnModuleControls.push_back(mSpawnLists.mVstPlugins.GetList());
   mSpawnModuleControls.push_back(mSpawnLists.mPrefabs.GetList());
}

void Push2Control::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   if (!ThePushBridge.IsInitialized())
   {
      ofSetColor(255, 0, 0, gModuleDrawAlpha);
      DrawTextNormal(mPushBridgeInitErrMsg, 3, 15);
   }
}

void Push2Control::DrawModuleUnclipped()
{
   if (mDisplayModule != nullptr && !mDisplayModule->IsDeleted() && !sDrawingPush2Display &&
       mDisplayModule->GetOwningContainer() != nullptr && mDisplayModule->IsShowing())
   {
      ofPushMatrix();
      ofPushStyle();
      ofVec2f pos = GetPosition();
      ofTranslate(-pos.x, -pos.y);
      DrawDisplayModuleRect(mDisplayModule->GetRect());
      ofPopMatrix();
      ofPopStyle();
   }
}

void Push2Control::DrawDisplayModuleRect(ofRectangle rect)
{
   if (mDisplayModule->HasTitleBar())
   {
      rect.y -= IDrawableModule::TitleBarHeight();
      rect.height += IDrawableModule::TitleBarHeight();
   }
   ofSetColor(255, 255, 255, ofMap(sin(gTime / 1000 * PI * 2),-1,1,60,100));
   ofSetLineWidth(3);
   ofNoFill();
   ofRect(rect.x-3, rect.y-3, rect.width+6, rect.height+6);
}
 
void Push2Control::PostRender()
{
   if (ThePushBridge.IsInitialized())
      RenderPush2Display();
}

void Push2Control::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x,y,right);
   
   if (!ThePushBridge.IsInitialized())
   {
      Initialize();
   }
}

void Push2Control::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void Push2Control::SetUpFromSaveData()
{
}

void Push2Control::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
}

//static
void Push2Control::CreateStaticFramebuffer()
{
   sVG = nvgCreateGLES2(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
   assert(sVG);

   const auto width = ableton::Push2DisplayBitmap::kWidth;
   const auto height = ableton::Push2DisplayBitmap::kHeight;
   const int pixelRatio = 1;

   sFB = nvgluCreateFramebuffer(sVG, width*pixelRatio, height*pixelRatio, 0);
   assert(sFB);
}

bool Push2Control::Initialize()
{
   if (!ThePushBridge.IsInitialized())
   {
      if (auto result = ThePushBridge.Init(); result.Failed()) {
         mPushBridgeInitErrMsg = result.GetDescription();
         ofLog() << mPushBridgeInitErrMsg;
         return false;
      }
      ofLog() << "push 2 connected";
   }
   
   const auto width = ableton::Push2DisplayBitmap::kWidth;
   const auto height = ableton::Push2DisplayBitmap::kHeight;
   
   mPixels = new unsigned char[3 * (width * kPixelRatio) * (height * kPixelRatio)];
   
   mFontHandle = nvgCreateFont(sVG, ofToResourcePath("frabk.ttf").c_str(), ofToResourcePath("frabk.ttf").c_str());
   mFontHandleBold = nvgCreateFont(sVG, ofToResourcePath("frabk_m.ttf").c_str(), ofToResourcePath("frabk_m.ttf").c_str());
   
   const std::vector<std::string>& devices = mDevice.GetPortList(false);
   for (int i=0; i<devices.size(); ++i)
   {
#if JUCE_WINDOWS
      if (strcmp(devices[i].c_str(), "Ableton Push 2") == 0)
#else
      if (strcmp(devices[i].c_str(), "Ableton Push 2 Live Port") == 0)
#endif
      {
         mDevice.ConnectOutput(i);
         mDevice.ConnectInput(devices[i].c_str());
         break;
      }
   }

   return true;
}

void Push2Control::DrawToFramebuffer(NVGcontext* vg, NVGLUframebuffer* fb, float t, float pxRatio)
{
   int winWidth, winHeight;
   int fboWidth, fboHeight;

   if (fb == NULL)
      return;

   nvgImageSize(vg, fb->image, &fboWidth, &fboHeight);
   winWidth = (int)(fboWidth / pxRatio);
   winHeight = (int)(fboHeight / pxRatio);

   nvgluBindFramebuffer(fb);
   glViewport(0, 0, fboWidth, fboHeight);
   glClearColor(0, 0, 0, 0);
   glClear(juce::gl::GL_COLOR_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
   nvgBeginFrame(vg, winWidth, winHeight, pxRatio);
   
   nvgLineCap(vg, NVG_ROUND);
   nvgLineJoin(vg, NVG_ROUND);
   static float sSpacing = -.3f;
   nvgTextLetterSpacing(vg, sSpacing);
   
   mModules.clear();
   std::vector<IDrawableModule*> modules;
   TheSynth->GetAllModules(modules);
   for (int i=0; i<modules.size(); ++i)
   {
      if (!IsIgnorableModule(modules[i]))
         mModules.push_back(modules[i]);
   }
   mModules = SortModules(mModules);
   
   SetModuleGridLights();

   mModuleColumnOffsetSmoothed = ofLerp(mModuleColumnOffsetSmoothed, mModuleColumnOffset, .3f);
   mModuleListOffsetSmoothed = ofLerp(mModuleListOffsetSmoothed, round(mModuleListOffset), .3f);
   
   if (mScreenDisplayMode == ScreenDisplayMode::kNormal)
   {
      DrawLowerModuleSelector();
      DrawDisplayModuleControls();

      std::string stateInfo = "";
      if (mAllowRepatch && mHeldModule == nullptr)
         stateInfo = "repatch mode: hold a source module and tap the destination module";
      else if (mAllowRepatch && mHeldModule != nullptr)
         stateInfo = "repatch mode: now tap destination for " + std::string(mHeldModule->Name());
      else if (mNewButtonHeld)
         stateInfo = "tap control to add favorite...";
      else if (mDeleteButtonHeld)
         stateInfo = "tap control to remove favorite...";
      else if (mModulationButtonHeld)
         stateInfo = "tap a control to add/edit LFO...";
      else if (mAddModuleBookmarkButtonHeld)
         stateInfo = "tap a button in the column below this button to bookmark the current module...";
      else if (mInMidiControllerBindMode)
         stateInfo = "MIDI bind mode: hold a knob or button, then move/press a MIDI control to bind to that module control";

      if (stateInfo != "")
      {
         ofPushStyle();

         ofFill();
         ofColor bgColor(175, 255, 221);
         bgColor = bgColor * ofMap(sin(gTime / 300 * PI * 2), -1, 1, .7f, 1);
         ofSetColor(bgColor);
         ofRect(1, 120, ableton::Push2DisplayBitmap::kWidth-2, ableton::Push2DisplayBitmap::kHeight - 120);

         ofNoFill();
         ofSetColor(255,0,0);
         ofRect(1, 1, ableton::Push2DisplayBitmap::kWidth - 2, ableton::Push2DisplayBitmap::kHeight - 2);

         ofSetColor(0, 0, 0);
         DrawTextBold(stateInfo, 10, 147, 20);

         ofPopStyle();
      }
   }
   else if (mScreenDisplayMode == ScreenDisplayMode::kAddModule)
   {      
      ofPushMatrix();
      ofPushStyle();

      if (mSelectedGridSpawnListIndex == -1)
      {
         ofSetColor(255, 255, 255);
         std::string text = "choose a module, then tap a grid square";
         std::string moduleTypeToSpawn = GetModuleTypeToSpawn();
         if (moduleTypeToSpawn != "")
         {
            ofSetColor(IDrawableModule::GetColor(TheSynth->GetModuleFactory()->GetModuleType(moduleTypeToSpawn)));
            text = "\ntap grid to spawn " + moduleTypeToSpawn;
         }
         DrawTextBold(text, 5, 80, 20);
      
         ofSetColor(IDrawableModule::GetColor(kModuleType_Other));
         ofNoFill();

         ofTranslate(-kColumnSpacing * mModuleColumnOffsetSmoothed, 0);
      
         nvgFontSize(sVG, 16);
         DrawControls(mButtonControls, false, 60);
         DrawControls(mSliderControls, true, 20);
      }
      else if (mSelectedGridSpawnListIndex < (int)mSpawnLists.GetDropdowns().size())
      {
         auto* list = mSpawnLists.GetDropdowns()[mSelectedGridSpawnListIndex]->GetList();
         int boxWidth = 120;
         int boxHeight = 18;
         ofFill();
         ofSetColor(IDrawableModule::GetColor(GetModuleTypeForSpawnList(list)));
         ofRect(mSelectedGridSpawnListIndex * boxWidth, -5, boxWidth, 12);
         for (int i = 0; i < list->GetNumValues(); ++i)
         {
            int x = (i % 8) * boxWidth;
            int y = (i / 8) * boxHeight + 10;
            ofPushMatrix();
            ofClipWindow(x, y, boxWidth, boxHeight, false);
            ofSetColor(GetSpawnGridColor(i, GetModuleTypeForSpawnList(list)));
            ofRect(x, y, boxWidth, boxHeight);
            ofSetColor(255, 255, 255);
            DrawTextBold(list->GetLabel(i), x+4, y+12);
            ofPopMatrix();
         }
      }
      ofPopMatrix();
      ofPopStyle();
      
      for (int i=0; i<8; ++i)
      {
         if (i < (int)mSpawnModuleControls.size())
            SetLed(kMidiMessage_Control, i + kAboveScreenButtonRow, GetPadColorForType(GetModuleTypeForSpawnList(mSpawnModuleControls[i])));
         else
            SetLed(kMidiMessage_Control, i+kAboveScreenButtonRow, 0);
         SetLed(kMidiMessage_Control, i+kBelowScreenButtonRow, 0);
      }
   }
   
   bool isHoveringOverNewModule = (gHoveredModule != mDisplayModule && gHoveredModule != nullptr);
   SetLed(kMidiMessage_Control, kTapTempoButton, isHoveringOverNewModule ? 127 : 0, isHoveringOverNewModule ? 32 : 0);
   SetLed(kMidiMessage_Control, kNewButton, 127, mNewButtonHeld ? 0 : -1);
   SetLed(kMidiMessage_Control, kDeleteButton, 127, mDeleteButtonHeld ? 0 : -1);
   SetLed(kMidiMessage_Control, kAutomateButton, 126, mModulationButtonHeld ? 0 : -1);
   SetLed(kMidiMessage_Control, kMasterButton, 127, mAddModuleBookmarkButtonHeld ? 0 : -1);
   SetLed(kMidiMessage_Control, kCircleButton, 8, mAllowRepatch ? 11 : -1);
   SetLed(kMidiMessage_Control, kAddDeviceButton, 127, mScreenDisplayMode == ScreenDisplayMode::kAddModule ? 0 : -1);
   SetLed(kMidiMessage_Control, kUpButton, 127);
   SetLed(kMidiMessage_Control, kDownButton, 127);
   SetLed(kMidiMessage_Control, kLeftButton, 127);
   SetLed(kMidiMessage_Control, kRightButton, 127);
   SetLed(kMidiMessage_Control, kPageLeftButton, mModuleHistoryPosition > 0 ? 127 : 0);
   SetLed(kMidiMessage_Control, kPageRightButton, mModuleHistoryPosition < mModuleHistory.size()-1 ? 127 : 0);
   SetLed(kMidiMessage_Control, kSetupButton, mInMidiControllerBindMode ? 127 : 32, mInMidiControllerBindMode ? 0 : 32);
   if (mGridControlModule != nullptr)
   {
      SetLed(kMidiMessage_Control, kNoteButton, 127, 10);
      SetLed(kMidiMessage_Control, kSessionButton, 10);
   }
   else
   {
      SetLed(kMidiMessage_Control, kNoteButton, mDisplayModuleCanControlGrid ? 10 : 0);
      SetLed(kMidiMessage_Control, kSessionButton, 127);
   }
   for (int i = 0; i < kNumQuantizeButtons; ++i)
   {
      int color = 0;
      if (mBookmarkSlots[i] != nullptr && !mBookmarkSlots[i]->IsDeleted())
         color = GetPadColorForType(mBookmarkSlots[i]->GetModuleType());
      SetLed(kMidiMessage_Control, kQuantizeButtonSection + i, color);
   }
   
   //test led colors
   //SetLed(kMidiMessage_Note, 92, (int)mModuleListOffset);
   //ofLog() << (int)mModuleListOffset;

   nvgEndFrame(vg);
   
   glFinish();
   glReadBuffer(juce::gl::GL_COLOR_ATTACHMENT0);
   glReadPixels(0, 0, winWidth, winHeight, juce::gl::GL_RGB, GL_UNSIGNED_BYTE, mPixels);
   
   nvgluBindFramebuffer(NULL);
}

ofColor Push2Control::GetSpawnGridColor(int index, ModuleType moduleType) const
{
   bool bright = false;
   if ((index / 4) % 4 == 0 || (index / 4) % 4 == 3)
      bright = true;

   ofColor color = IDrawableModule::GetColor(moduleType);
   if (bright)
      color = color * .8f;
   else
      color = color * .4f;

   return color;
}

int Push2Control::GetSpawnGridPadColor(int index, ModuleType moduleType) const
{
   bool bright = false;
   if ((index / 4) % 4 == 0 || (index / 4) % 4 == 3)
      bright = true;

   int color;
   switch (moduleType)
   {
   case kModuleType_Instrument:
      color = bright ? 26 : 116;
      break;
   case kModuleType_Note:
      color = bright ? 8 : 80;
      break;
   case kModuleType_Synth:
      color = bright ? 11 : 86;
      break;
   case kModuleType_Audio:
      color = bright ? 18 : 96;
      break;
   case kModuleType_Modulator:
      color = bright ? 22 : 112;
      break;
   case kModuleType_Pulse:
      color = bright ? 9 : 82;
      break;
   default:
      color = bright ? 118 : 124;
      break;
   }

   return color;
}

void Push2Control::SetModuleGridLights()
{
   for (int i=0; i<8*8; ++i)
      mModuleGrid[i] = nullptr;
   
   float minX = 0;
   float minY = 0;
   float maxX = ofGetWidth();
   float maxY = ofGetHeight();
   for (int i=0; i<mModules.size(); ++i)
   {
      ofVec2f pos = mModules[i]->GetPosition();
      if (pos.x > maxX)
         maxX = pos.x;
      if (pos.y > maxY)
         maxY = pos.y;
      if (pos.x < minX)
         minX = pos.x;
      if (pos.y < minY)
         minY = pos.y;
   }
   maxX += 1;
   maxY += 1;
   mModuleGridRect.set(minX, minY, maxX - minX, maxY - minY);
   
   for (int i=0; i<mModules.size(); ++i)
   {
      ofVec2f pos = mModules[i]->GetPosition();
      int gridX = (pos.x - minX) / (maxX - minX) * 8;
      int gridY = (pos.y - minY) / (maxY - minY) * 8;
      while (gridX < 8 && gridY < 8)
      {
         int index = gridX + gridY * 8;
         if (mModuleGrid[index] == nullptr)
         {
            mModuleGrid[index] = mModules[i];
            break;
         }
         else
         {
            //IDrawableModule* otherModule = mModuleGrid[index];
            int peekGridIndex;
            if (GetGridIndex(gridX + 1, gridY, peekGridIndex) && mModuleGrid[peekGridIndex] == nullptr)
               ++gridX;
            else if (GetGridIndex(gridX, gridY + 1, peekGridIndex) && mModuleGrid[peekGridIndex] == nullptr)
               ++gridY;
            else
               ++gridX;
         }
      }
   }
   
   if (mScreenDisplayMode == ScreenDisplayMode::kAddModule && mSelectedGridSpawnListIndex != -1 && mSelectedGridSpawnListIndex < (int)mSpawnLists.GetDropdowns().size())
   {
      auto* list = mSpawnLists.GetDropdowns()[mSelectedGridSpawnListIndex]->GetList();
      for (int i = 0; i < 8 * 8; ++i)
      {
         int gridX = i % 8;
         int gridY = i / 8;
         int gridIndex = gridX + (7 - gridY) * 8 + 36;
         if (i < list->GetNumValues())
            SetLed(kMidiMessage_Note, gridIndex, GetSpawnGridPadColor(i, GetModuleTypeForSpawnList(list)));
         else
            SetLed(kMidiMessage_Note, gridIndex, 0);
      }
   }
   else if (mGridControlModule != nullptr)
   {
      mGridControlModule->UpdatePush2Leds(this);
   }
   else
   {
      for (int i=0; i<8*8; ++i)
      {
         int gridX = i % 8;
         int gridY = i / 8;
         int gridIndex = gridX + (7-gridY) * 8;
         int padNumber = 36 + i;
         if (mModuleGrid[gridIndex] != nullptr)
            SetLed(kMidiMessage_Note, padNumber, GetPadColorForType(mModuleGrid[gridIndex]->GetModuleType()), mModuleGrid[gridIndex] == mDisplayModule ? 122 : -1);
         else
            SetLed(kMidiMessage_Note, padNumber, 0);
      }
   }
}

void Push2Control::DrawDisplayModuleControls()
{
   if (mDisplayModule != nullptr && mDisplayModule->IsDeleted())
   {
      SetDisplayModule(nullptr, false);
      return;
   }

   ofSetColor(255,255,255);
   if (mDisplayModule != nullptr)
   {      
      ofPushMatrix();
      ofPushStyle();
      
      //nvgFontSize(mVG, 16);
      //nvgText(mVG, 10, 10, mDisplayModule->Name(), nullptr);
      float x;
      float y;
      mDisplayModule->GetPosition(x, y, true);
      mDisplayModule->SetPosition(5 - kColumnSpacing * mModuleColumnOffsetSmoothed, 15);
      float titleBarHeight;
      float highlight;
      mDisplayModule->DrawFrame(kColumnSpacing * MAX(1,MAX(mSliderControls.size(), mButtonControls.size())) - 14, 80, false, titleBarHeight, highlight);
      mDisplayModule->SetPosition(x, y);
      
      ofSetColor(IDrawableModule::GetColor(mDisplayModule->GetModuleType()));
      ofNoFill();
      
      nvgFontSize(sVG, 16);
      DrawControls(mButtonControls, false, 60);
      DrawControls(mSliderControls, true, 20);
      
      int topRowLedColors[8] = {0,0,0,0,0,0,0,0};
      for (int i=0; i < mButtonControls.size(); ++i)
      {
         if (i - mModuleColumnOffset >= 0 && i - mModuleColumnOffset < 8)
            topRowLedColors[i-mModuleColumnOffset] = GetPadColorForType(mButtonControls[i]->GetModuleParent()->GetModuleType());
      }
      for (int i=0; i<8; ++i)
         SetLed(kMidiMessage_Control, i+kAboveScreenButtonRow, topRowLedColors[i]);
      
      ofPopMatrix();
      ofPopStyle();
   }
   else
   {
      for (int i = 0; i < 8; ++i)
         SetLed(kMidiMessage_Control, i + kAboveScreenButtonRow, 0);
   }
}

void Push2Control::DrawLowerModuleSelector()
{
   int bottomRowLedColors[8] = {0,0,0,0,0,0,0,0};
   for (int i=0; i<mModules.size(); ++i)
   {
      if (i - mModuleListOffset < -1 || i - mModuleListOffset > 8)
         continue;
      
      ofPushMatrix();
      ofPushStyle();
      
      float x;
      float y;
      mModules[i]->GetPosition(x, y, true);
      mModules[i]->SetPosition(3 + kColumnSpacing * (i - mModuleListOffsetSmoothed), 120);
      float titleBarHeight;
      float highlight;
      mModules[i]->DrawFrame(kColumnSpacing - 14, 80, true, titleBarHeight, highlight);
      if (mModules[i] == mDisplayModule)
         DrawDisplayModuleRect(ofRectangle(0,0,kColumnSpacing-14,80));
      mModules[i]->SetPosition(x, y);
      
      if (i - round(mModuleListOffset) >= 0 && i - round(mModuleListOffset) < 8)
         bottomRowLedColors[i-(int)round(mModuleListOffset)] = GetPadColorForType(mModules[i]->GetModuleType());
      
      ofPopMatrix();
      ofPopStyle();
   }
   
   for (int i=0; i<8; ++i)
      SetLed(kMidiMessage_Control, i+kBelowScreenButtonRow, bottomRowLedColors[i]);
}

int Push2Control::GetPadColorForType(ModuleType type)
{
   int color;
   switch (type)
   {
      case kModuleType_Instrument:
         color = 26;
         break;
      case kModuleType_Note:
         color = 8;
         break;
      case kModuleType_Synth:
         color = 11;
         break;
      case kModuleType_Audio:
         color = 18;
         break;
      case kModuleType_Modulator:
         color = 22;
         break;
      case kModuleType_Pulse:
         color = 9;
         break;
      default:
         color = 118;
         break;
   }
   return color;
}

ModuleType Push2Control::GetModuleTypeForSpawnList(IUIControl* control)
{
   ModuleType moduleType = kModuleType_Other;
   if (control == mSpawnLists.mInstrumentModules.GetList())
      moduleType = kModuleType_Instrument;
   if (control == mSpawnLists.mNoteModules.GetList())
      moduleType = kModuleType_Note;
   if (control == mSpawnLists.mSynthModules.GetList())
      moduleType = kModuleType_Synth;
   if (control == mSpawnLists.mAudioModules.GetList())
      moduleType = kModuleType_Audio;
   if (control == mSpawnLists.mModulatorModules.GetList())
      moduleType = kModuleType_Modulator;
   if (control == mSpawnLists.mPulseModules.GetList())
      moduleType = kModuleType_Pulse;
   if (control == mSpawnLists.mOtherModules.GetList())
      moduleType = kModuleType_Other;
   if (control == mSpawnLists.mVstPlugins.GetList())
      moduleType = kModuleType_Synth;
   if (control == mSpawnLists.mPrefabs.GetList())
      moduleType = kModuleType_Other;
   return moduleType;
}

void Push2Control::DrawControls(std::vector<IUIControl*> controls, bool sliders, float yPos)
{
   for (int i=(int)controls.size()-1; i >= 0; --i)
   {
      if (i - mModuleColumnOffset < -1 || i - mModuleColumnOffset > 8)
         continue;
      
      float x;
      float y;
      controls[i]->GetPosition(x, y, true);
      controls[i]->SetPosition(kColumnSpacing * i + 3, yPos);
      controls[i]->Render();
      controls[i]->SetPosition(x, y);
      
      ofPushStyle();
      ModuleType moduleType = controls[i]->GetModuleParent()->GetModuleType();
      if (mScreenDisplayMode == ScreenDisplayMode::kAddModule)
         moduleType = GetModuleTypeForSpawnList(controls[i]);
      if (controls[i]->IsShowing())
         ofSetColor(IDrawableModule::GetColor(moduleType));
      else
         ofSetColor(100, 100, 100);
      
      if (mDisplayModule == this)
         DrawTextBold(juce::String(controls[i]->Path()).replace("~","\n").toRawUTF8(), kColumnSpacing * i + 3, yPos-12, 10);
      else
         DrawTextBold(controls[i]->Name(), kColumnSpacing * i + 3, yPos-5, 16);
      
      int pushControlIndex = i - mModuleColumnOffset;
      if (sliders && pushControlIndex >= 0 && pushControlIndex < 8 && mNoteHeldState[pushControlIndex])
      {
         DropdownList* dropdown = dynamic_cast<DropdownList*>(mSliderControls[i]);
         if (dropdown != nullptr)
         {
            const float kCentering = 7;
            float w = dropdown->GetMaxItemWidth();
            float h = dropdown->GetNumValues() * DropdownList::kItemSpacing;
            ofPushMatrix();
            ofTranslate(kColumnSpacing * i + 3, yPos + kCentering - h * controls[i]->GetMidiValue());
            dropdown->DrawDropdown(w, h, true);
            ofFill();
            ofColor color = IDrawableModule::GetColor(controls[i]->GetModuleParent()->GetModuleType());
            color.a = 25;
            ofSetColor(color);
            //ofCircle(w - 4, h * controls[i]->GetMidiValue(), 2);
            ofRect(0,h * controls[i]->GetMidiValue() - kCentering,w,controls[i]->GetDimensions().y);
            ofPopMatrix();
         }
      }
      
      ofPopStyle();
   }
}

void Push2Control::RenderPush2Display()
{   
   auto mainVG = gNanoVG;
   gNanoVG = sVG;
   sDrawingPush2Display = true;
   DrawToFramebuffer(sVG, sFB, gTime/300, kPixelRatio);
   sDrawingPush2Display = false;
   gNanoVG = mainVG;

   // Tells the bridge we're done with drawing and the frame can be sent to the display
   ThePushBridge.Flip(mPixels);
}

void Push2Control::Poll()
{
   if (mPendingSpawnPitch != -1)
   {
      int gridIndex = mPendingSpawnPitch - 36;
      int gridX = gridIndex % 8;
      int gridY = gridIndex / 8;
      ofVec2f newModulePos = ofVec2f(ofMap(gridX, 0, 7, mModuleGridRect.getMinX(), mModuleGridRect.getMaxX()), ofMap(gridY, 7, 0, mModuleGridRect.getMinY(), mModuleGridRect.getMaxY()));
      
      for (int i=0; i<mSpawnLists.GetDropdowns().size(); ++i)
      {
         if (mSpawnLists.GetDropdowns()[i]->GetList()->GetValue() != -1)
         {
            IDrawableModule* module = mSpawnLists.GetDropdowns()[i]->Spawn();
            ofRectangle rect = module->GetRect();
            module->SetPosition(newModulePos.x, newModulePos.y);
            mSpawnLists.GetDropdowns()[i]->GetList()->SetValue(-1);
            mScreenDisplayMode = ScreenDisplayMode::kNormal;
            SetDisplayModule(module, true);
            break;
         }
      }
      
      mPendingSpawnPitch = -1;
   }

   if (mDisplayModule != nullptr && mDisplayModule->HasPush2OverrideControls())
   {
      std::vector<IUIControl*> desiredControls;
      mDisplayModule->GetPush2OverrideControls(desiredControls);
      bool changed = false;
      if (desiredControls.size() != mDisplayedControls.size())
      {
         changed = true;
      }
      else
      {
         for (size_t i = 0; i < desiredControls.size(); ++i)
         {
            if (desiredControls[i] != mDisplayedControls[i])
            {
               changed = true;
               break;
            }
         }
      }

      if (changed)
         UpdateControlList();
   }
}

std::string Push2Control::GetModuleTypeToSpawn()
{
   for (int i = 0; i < mSpawnLists.GetDropdowns().size(); ++i)
   {
      if (mSpawnLists.GetDropdowns()[i]->GetList()->GetValue() != -1)
         return mSpawnLists.GetDropdowns()[i]->GetList()->GetDisplayValue(mSpawnLists.GetDropdowns()[i]->GetList()->GetValue());
   }

   return "";
}

void Push2Control::SetDisplayModule(IDrawableModule* module, bool addToHistory)
{
   mDisplayModule = module;
   if (dynamic_cast<IPush2GridController*>(mDisplayModule) != nullptr)
      mDisplayModuleCanControlGrid = true;
   else
      mDisplayModuleCanControlGrid = false;
   mModuleColumnOffset = 0;
   mModuleColumnOffsetSmoothed = 0;
   UpdateControlList();

   if (module != nullptr && addToHistory)
   {
      while (mModuleHistory.size() > mModuleHistoryPosition + 1)
         mModuleHistory.pop_back();
      mModuleHistory.push_back(module);
      ++mModuleHistoryPosition;
   }
}

void Push2Control::UpdateControlList()
{
   mSliderControls.clear();
   mButtonControls.clear();
   std::vector<IUIControl*> controls;
   if (mScreenDisplayMode == ScreenDisplayMode::kAddModule)
   {
      controls = mSpawnModuleControls;
   }
   else if (mDisplayModule == this)
   {
      controls = mFavoriteControls;
   }
   else if (mDisplayModule != nullptr)
   {
      if (mDisplayModule->HasPush2OverrideControls())
         mDisplayModule->GetPush2OverrideControls(controls);
      else
         controls = mDisplayModule->GetUIControls();
   }

   for (int i=0; i < controls.size(); ++i)
   {
      if (controls[i]->IsSliderControl() && controls[i]->GetShouldSaveState())
         mSliderControls.push_back(controls[i]);
      if (controls[i]->IsButtonControl() && (controls[i]->GetShouldSaveState() || dynamic_cast<ClickButton*>(controls[i]) != nullptr))
         mButtonControls.push_back(controls[i]);
   }
   mDisplayedControls = controls;
}

void Push2Control::AddFavoriteControl(IUIControl* control)
{
   if (!VectorContains(control, mFavoriteControls))
   {
      mFavoriteControls.push_back(control);
      UpdateControlList();
   }
}

void Push2Control::RemoveFavoriteControl(IUIControl* control)
{
   auto iter = std::find(mFavoriteControls.begin(), mFavoriteControls.end(), control);
   if (iter != mFavoriteControls.end())
   {
      mFavoriteControls.erase(iter);
      UpdateControlList();
   }
}

void Push2Control::BookmarkModuleToSlot(int slotIndex, IDrawableModule* module)
{
   mBookmarkSlots[slotIndex] = module;
   mAddModuleBookmarkButtonHeld = false;
}

void Push2Control::SwitchToBookmarkedModule(int slotIndex)
{
   if (mBookmarkSlots[slotIndex] != nullptr && !mBookmarkSlots[slotIndex]->IsDeleted())
      SetDisplayModule(mBookmarkSlots[slotIndex], true);
}

void Push2Control::SetLed(MidiMessageType type, int index, int color, int flashColor /*=-1*/)
{
   if (type == kMidiMessage_Control)
      index += 128;
   assert(index >= 0 && index < 128 * 2);
   
   int channel = 1;
   if (flashColor != -1)
      channel = 10;
   int stateValue = color | channel << 8;
   if (mLedState[index] != stateValue)
   {
      //bool isPulse = (channel >= 7 && channel <= 11);
      mLedState[index] = stateValue;
      if (index < 128)
      {
         if (flashColor != -1)
            mDevice.SendNote(gTime, index, flashColor, false, -1);
         mDevice.SendNote(gTime, index, color, false, channel);
      }
      else
      {
         if (flashColor != -1)
            mDevice.SendCC(index-128, flashColor);
         mDevice.SendCC(index-128, color, channel);
      }
   }
}

void Push2Control::OnMidiNote(MidiNote& note)
{
   if (mGridControlModule != nullptr)
   {
      bool handled = mGridControlModule->OnPush2Control(kMidiMessage_Note, note.mPitch, note.mVelocity);
      if (handled)
         return;
   }
   
   if (note.mPitch >= 0 && note.mPitch <= 7) //main encoders
   {
      if (mScreenDisplayMode == ScreenDisplayMode::kNormal)
      {
         int controlIndex = note.mPitch + mModuleColumnOffset;
         if (controlIndex < mSliderControls.size())
         {
            if (note.mVelocity > 0)
            {
               mSliderControls[controlIndex]->StartBeacon();

               if (mNewButtonHeld)
               {
                  AddFavoriteControl(mSliderControls[controlIndex]);
               }
               else if (mDeleteButtonHeld)
               {
                  RemoveFavoriteControl(mSliderControls[controlIndex]);
               }
               else if (mModulationButtonHeld)
               {
                  FloatSlider* slider = dynamic_cast<FloatSlider*>(mSliderControls[controlIndex]);
                  if (slider != nullptr)
                  {
                     bool hadLFO = (slider->GetLFO() != nullptr);
                     FloatSliderLFOControl* lfo = slider->AcquireLFO();
                     if (!hadLFO)
                        lfo->SetEnabled(true);
                     SetDisplayModule(lfo, true);
                  }
               }
               else if (mInMidiControllerBindMode)
               {
                  sBindToUIControl = mSliderControls[controlIndex];
               }
            }
            else
            {
               if (sBindToUIControl == mSliderControls[controlIndex])
                  sBindToUIControl = nullptr;
            }
         }
      }
      
      if (mScreenDisplayMode == ScreenDisplayMode::kAddModule)
      {
         if (note.mVelocity > 0)
         {
            for (int i=0; i<mSpawnLists.GetDropdowns().size(); ++i)
            {
               if (i != note.mPitch)
                  mSpawnLists.GetDropdowns()[i]->GetList()->SetValue(-1);
            }
         }
      }
   }
   else if (note.mPitch >= 36 && note.mPitch <= 99 && mGridControlModule == nullptr)  //pads
   {
      if (mScreenDisplayMode == ScreenDisplayMode::kAddModule && mSelectedGridSpawnListIndex != -1 && mSelectedGridSpawnListIndex < (int)mSpawnLists.GetDropdowns().size())
      {
         if (note.mVelocity > 0)
         {
            auto* list = mSpawnLists.GetDropdowns()[mSelectedGridSpawnListIndex]->GetList();
            int gridIndex = note.mPitch - 36;
            int gridX = gridIndex % 8;
            int gridY = gridIndex / 8;
            gridIndex = gridX + (7 - gridY) * 8;
            if (gridIndex < list->GetNumValues())
            {
               list->SetValueDirect(gridIndex);
               mSelectedGridSpawnListIndex = -1;
            }
         }
      }
      else if (mScreenDisplayMode == ScreenDisplayMode::kAddModule)
      {
         if (note.mVelocity > 0)
            mPendingSpawnPitch = note.mPitch;
      }
      else if (mGridControlModule == nullptr)
      {
         if (note.mVelocity > 0)
         {
            int gridIndex = note.mPitch - 36;
            int gridX = gridIndex % 8;
            int gridY = gridIndex / 8;
            gridIndex = gridX + (7 - gridY) * 8;
            if (mModuleGrid[gridIndex] != nullptr)
               SetDisplayModule(mModuleGrid[gridIndex], true);

            if (mHeldModule != nullptr)
            {
               if (mAllowRepatch && mHeldModule->GetPatchCableSource() != nullptr)
               {
                  mHeldModule->GetPatchCableSource()->FindValidTargets();
                  if (mHeldModule->GetPatchCableSource()->IsValidTarget(mModuleGrid[gridIndex]))
                     mHeldModule->SetTarget(mModuleGrid[gridIndex]);
                  else
                     mHeldModule->GetPatchCableSource()->ClearPatchCables();
               }
            }
            else
            {
               mHeldModule = mModuleGrid[gridIndex];
            }
         }
         else
         {
            mHeldModule = nullptr;
         }
      }
   }
   else
   {
      //ofLog() << "note " << note.mPitch << " " << note.mVelocity;
   }
   
   mNoteHeldState[note.mPitch] = note.mVelocity > 0;
}

void Push2Control::OnMidiControl(MidiControl& control)
{
   if (mGridControlModule != nullptr)
   {
      bool handled = mGridControlModule->OnPush2Control(kMidiMessage_Control, control.mControl, control.mValue);
      if (handled)
         return;
   }
   
   if (control.mControl >= 71 && control.mControl <= 78) //main encoders
   {
      int controlIndex = control.mControl - 71 + mModuleColumnOffset;
      if (controlIndex < mSliderControls.size())
      {
         float currentNormalized = mSliderControls[controlIndex]->GetMidiValue();
         float increment = control.mValue < 64 ? control.mValue : control.mValue - 128;
         increment *= .005f;
         mSliderControls[controlIndex]->SetFromMidiCC(currentNormalized + increment);
      }
   }
   else if (control.mControl >= kAboveScreenButtonRow && control.mControl < kAboveScreenButtonRow + 8) //buttons below encoders
   {
      int controlIndex = control.mControl - kAboveScreenButtonRow + mModuleColumnOffset;
      if (mScreenDisplayMode == ScreenDisplayMode::kNormal)
      {
         if (controlIndex < mButtonControls.size())
         {
            if (control.mValue > 0)
            {
               if (mNewButtonHeld)
               {
                  mButtonControls[controlIndex]->StartBeacon();
                  AddFavoriteControl(mButtonControls[controlIndex]);
               }
               else if (mDeleteButtonHeld)
               {
                  mButtonControls[controlIndex]->StartBeacon();
                  RemoveFavoriteControl(mButtonControls[controlIndex]);
               }
               else if (mInMidiControllerBindMode)
               {
                  sBindToUIControl = mButtonControls[controlIndex];
               }
               else
               {
                  float current = mButtonControls[controlIndex]->GetMidiValue();
                  float newValue = current > 0 ? 0 : 1;
                  mButtonControls[controlIndex]->SetFromMidiCC(newValue);
               }
            }
            else
            {
               if (sBindToUIControl == mButtonControls[controlIndex])
                  sBindToUIControl = nullptr;
            }
         }
      }

      if (mScreenDisplayMode == ScreenDisplayMode::kAddModule)
      {
         if (control.mValue > 0 && controlIndex < mSpawnModuleControls.size())
         {
            if (mSelectedGridSpawnListIndex != controlIndex)
               mSelectedGridSpawnListIndex = controlIndex;
            else
               mSelectedGridSpawnListIndex = -1;

            for (int i = 0; i < mSpawnLists.GetDropdowns().size(); ++i)
               mSpawnLists.GetDropdowns()[i]->GetList()->SetValue(-1);
         }
      }
   }
   else if (control.mControl == 14) //leftmost clicky encoder
   {
      int increment = control.mValue < 64 ? control.mValue : control.mValue - 128;
      mModuleColumnOffset = (int)ofClamp(mModuleColumnOffset + increment, 0, MAX(0, (int)MAX(mSliderControls.size(), mButtonControls.size()) - 8));
   }
   else if (control.mControl == 15) //encoder next to above encoder
   {
      float increment = control.mValue < 64 ? control.mValue : control.mValue - 128;
      increment *= .05f;
      mModuleListOffset += increment;
   }
   else if (control.mControl >= kBelowScreenButtonRow && control.mControl < kBelowScreenButtonRow + 8) //buttons below screen
   {
      int moduleIndex = control.mControl - kBelowScreenButtonRow + round(mModuleListOffset);
      if (control.mValue > 0 && moduleIndex < mModules.size())
      {
         SetDisplayModule(mModules[moduleIndex], true);
      }
   }
   else if (control.mControl == kSetupButton && control.mValue > 0)
   {
      mInMidiControllerBindMode = !mInMidiControllerBindMode;
      mAllowRepatch = false;
   }
   else if (control.mControl == kNewButton)
   {
      mNewButtonHeld = control.mValue > 0;
   }
   else if (control.mControl == kDeleteButton)
   {
      mDeleteButtonHeld = control.mValue > 0;
   }
   else if (control.mControl == kAutomateButton)
   {
      mModulationButtonHeld = control.mValue > 0;
   }
   else if (control.mControl == kMasterButton)
   {
      mAddModuleBookmarkButtonHeld = control.mValue > 0;
   }
   else if (control.mControl == kCircleButton)
   {
      if (control.mValue > 0)
      {
         mAllowRepatch = !mAllowRepatch;
         mInMidiControllerBindMode = false;
      }
   }
   else if (control.mControl == kTapTempoButton)
   {
      if (gHoveredModule != nullptr && gHoveredModule != mDisplayModule)
         SetDisplayModule(gHoveredModule, true);
   }
   else if (control.mControl == kAddDeviceButton)
   {
      if (control.mValue > 0)
      {
         if (mScreenDisplayMode == ScreenDisplayMode::kAddModule)
         {
            mScreenDisplayMode = ScreenDisplayMode::kNormal;
         }
         else
         {
            mScreenDisplayMode = ScreenDisplayMode::kAddModule;
            mGridControlModule = nullptr;
            for (int i = 0; i < mSpawnLists.GetDropdowns().size(); ++i)
               mSpawnLists.GetDropdowns()[i]->GetList()->SetValue(-1);
            mSelectedGridSpawnListIndex = -1;
         }

         mModuleColumnOffset = 0;
         mModuleColumnOffsetSmoothed = 0;
         mModuleListOffset = 0;
         mModuleListOffsetSmoothed = 0;
         UpdateControlList();
      }
   }
   else if (control.mControl == kUpButton || control.mControl == kDownButton || control.mControl == kLeftButton || control.mControl == kRightButton)
   {
      if (control.mValue > 0)
      {
         ofVec2f direction;
         if (control.mControl == kUpButton)
            direction.y -= 1;
         if (control.mControl == kDownButton)
            direction.y += 1;
         if (control.mControl == kLeftButton)
            direction.x -= 1;
         if (control.mControl == kRightButton)
            direction.x += 1;
         
         if (mDisplayModule)
         {
            ofVec2f pos = mDisplayModule->GetPosition();
            pos += direction * 50;
            mDisplayModule->SetPosition(pos.x, pos.y);
         }
      }
   }
   else if (control.mControl == kPageLeftButton || control.mControl == kPageRightButton)
   {
      if (control.mValue > 0)
      {
         int direction = 0;
         if (control.mControl == kPageLeftButton)
            direction -= 1;
         if (control.mControl == kPageRightButton)
            direction += 1;

         int newHistoryPos = mModuleHistoryPosition + direction;
         if (newHistoryPos >= 0 && newHistoryPos < mModuleHistory.size())
         {
            mModuleHistoryPosition = newHistoryPos;
            SetDisplayModule(mModuleHistory[mModuleHistoryPosition], false);
         }
      }
   }
   else if (control.mControl == kNoteButton)
   {
      if (control.mValue > 0)
      {
         IPush2GridController* controller = dynamic_cast<IPush2GridController*>(mDisplayModule);
         if (controller != nullptr && controller != mGridControlModule)
         {
            mGridControlModule = controller;
            
            for (int i=36; i<=99; ++i)
               SetLed(kMidiMessage_Note, i, 0);
            mGridControlModule->OnPush2Connect();
            
            mScreenDisplayMode = ScreenDisplayMode::kNormal;
            UpdateControlList();
         }
      }
   }
   else if (control.mControl == kSessionButton)
   {
      if (control.mValue > 0)
         mGridControlModule = nullptr;
   }
   else if (control.mControl >= kQuantizeButtonSection && control.mControl < kQuantizeButtonSection + kNumQuantizeButtons)  //quantization level buttons to the right of the main grid, used to bookmark modules
   {
      if (control.mValue > 0)
      {
         int i = control.mControl - kQuantizeButtonSection;
         if (mAddModuleBookmarkButtonHeld)
            BookmarkModuleToSlot(i, mDisplayModule);
         if (mDeleteButtonHeld)
            BookmarkModuleToSlot(i, nullptr);
         else
            SwitchToBookmarkedModule(i);
      }
   }
   else
   {
      ofLog() << "control " << control.mControl << " " << control.mValue;
   }
}

void Push2Control::OnMidiPitchBend(MidiPitchBend& pitchBend)
{
   if (mGridControlModule != nullptr)
   {
      bool handled = mGridControlModule->OnPush2Control(kMidiMessage_PitchBend, pitchBend.mChannel, pitchBend.mValue);
      if (handled)
         return;
   }
   
   ofLog() << "pitchbend " << pitchBend.mChannel << " " << pitchBend.mValue;
}

bool Push2Control::IsIgnorableModule(IDrawableModule* module)
{
   return module == TheTitleBar || module == TheSaveDataPanel || module == TheQuickSpawnMenu || module == TheSynth->GetUserPrefsEditor();
}

std::vector<IDrawableModule*> Push2Control::SortModules(std::vector<IDrawableModule*> modules)
{
   std::vector<IDrawableModule*> output;
    
   for (int i=0; i<modules.size(); ++i)
      AddModuleChain(modules[i], modules, output, 0);
   
   return output;
}

void Push2Control::AddModuleChain(IDrawableModule* module, std::vector<IDrawableModule*>& modules, std::vector<IDrawableModule*>& output, int depth)
{
   if (depth > 100)  //avoid infinite recursion if there's a patching loop
      return;

   if (!VectorContains(module, output))
   {
      //look for parents
      for (int i=0; i<modules.size(); ++i)
      {
         IClickable* target = nullptr;
         if (modules[i]->GetPatchCableSource() != nullptr)
            target = modules[i]->GetPatchCableSource()->GetTarget();
         if (target != nullptr &&
             target == dynamic_cast<IClickable*>(module))
         {
            AddModuleChain(modules[i], modules, output, depth+1);
         }
      }
      
      if (VectorContains(module, output)) //got added above
         return;
      
      output.push_back(module);
   
      //look for children
      for (int i=0; i<modules.size(); ++i)
      {
         IClickable* target = nullptr;
         if (module->GetPatchCableSource() != nullptr)
            target = module->GetPatchCableSource()->GetTarget();
         if (target != nullptr &&
             target == dynamic_cast<IClickable*>(modules[i]))
         {
            AddModuleChain(modules[i], modules, output, depth+1);
         }
      }
   }
}
