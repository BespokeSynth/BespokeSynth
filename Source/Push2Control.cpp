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
#include "Snapshots.h"
#include "ADSRDisplay.h"
#include "Looper.h"
#include "NoteLooper.h"
#include "ControlRecorder.h"
#include "push2/JuceToPush2DisplayBridge.h"
#include "push2/Push2-Bitmap.h"

bool Push2Control::sDrawingPush2Display = false;
NVGcontext* Push2Control::sVG = nullptr;
NVGLUframebuffer* Push2Control::sFB = nullptr;
IUIControl* Push2Control::sBindToUIControl = nullptr;
namespace
{
   ableton::Push2DisplayBridge ThePushBridge; // The bridge allowing to use juce::graphics for push
}

//https://raw.githubusercontent.com/Ableton/push-interface/master/doc/MidiMapping.png

#include "leathers/push"
#include "leathers/unused-variable"
namespace
{
   const int kTapTempoButton = 3;
   const int kMetronomeButton = 9;
   const int kBelowScreenButtonRow = 20;
   const int kMasterButton = 28;
   const int kStopClipButton = 29;
   const int kSetupButton = 30;
   const int kLayoutButton = 31;
   const int kConvertButton = 35;
   const int kQuantizeButtonSection = 36;
   const int kLeftButton = 44;
   const int kRightButton = 45;
   const int kUpButton = 46;
   const int kDownButton = 47;
   const int kSelectButton = 48;
   const int kShiftButton = 49;
   const int kNoteButton = 50;
   const int kSessionButton = 51;
   const int kAddDeviceButton = 52;
   const int kAddTrackButton = 53;
   const int kOctaveDownButton = 54;
   const int kOctaveUpButton = 55;
   const int kRepeatButton = 56;
   const int kAccentButton = 57;
   const int kScaleButton = 58;
   const int kUserButton = 59;
   const int kMuteButton = 60;
   const int kSoloButton = 61;
   const int kPageLeftButton = 62;
   const int kPageRightButton = 63;
   const int kCornerKnob = 79;
   const int kPlayButton = 85;
   const int kCircleButton = 86;
   const int kNewButton = 87;
   const int kDuplicateButton = 88;
   const int kAutomateButton = 89;
   const int kFixedLengthButton = 90;
   const int kAboveScreenButtonRow = 102;
   const int kDeviceButton = 110;
   const int kBrowseButton = 111;
   const int kMixButton = 112;
   const int kClipButton = 113;
   const int kQuantizeButton = 116;
   const int kDoubleLoopButton = 117;
   const int kDeleteButton = 118;
   const int kUndoButton = 119;

   const int kNumQuantizeButtons = 8;
}
#include "leathers/pop"

Push2Control::Push2Control()
: mSpawnLists(this)
, mDevice(this)
{
   Initialize();
   for (int i = 0; i < 128 * 2; ++i)
      mLedState[i] = -1;
   for (int i = 0; i < (int)mModuleGrid.size(); ++i)
      mModuleGrid[i] = nullptr;
   for (int i = 0; i < 128; ++i)
      mNoteHeldState[i] = 0;
   for (int i = 0; i < kNumQuantizeButtons; ++i)
      mBookmarkSlots.push_back(nullptr);
}

Push2Control::~Push2Control()
{
}

void Push2Control::Exit()
{
   for (int i = 0; i < 128; ++i)
   {
      SetLed(kMidiMessage_Note, i, 0);
      SetLed(kMidiMessage_Control, i, 0);
   }

   if (mPixels != nullptr)
   {
      memset(mPixels, 0, sizeof(unsigned char) * GetNumDisplayPixels());
      ThePushBridge.Flip(mPixels);
   }

   mDevice.DisconnectInput();
   mDevice.DisconnectOutput();
}

void Push2Control::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   DROPDOWN(mModuleGridLayoutStyleDropdown, "grid style", (int*)(&mModuleGridLayoutStyle), 100);
   CHECKBOX(mShowManualGridCheckbox, "show manual grid", &mShowManualGrid);
   ENDUIBLOCK(mWidth, mHeight);

   mModuleGridLayoutStyleDropdown->AddLabel("auto layout", (int)ModuleGridLayoutStyle::Automatic);
   mModuleGridLayoutStyleDropdown->AddLabel("manual layout", (int)ModuleGridLayoutStyle::Manual);

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
   mSpawnModuleControls.push_back(mSpawnLists.mPlugins.GetList());
   mSpawnModuleControls.push_back(mSpawnLists.mOtherModules.GetList());
   mSpawnModuleControls.push_back(mSpawnLists.mPrefabs.GetList());

   for (size_t i = 0; i < mModuleGridManualCables.size(); ++i)
   {
      mModuleGridManualCables[i] = new PatchCableSource(this, kConnectionType_Special);
      ofColor color = IDrawableModule::GetColor(kModuleCategory_Other);
      color.a *= .3f;
      mModuleGridManualCables[i]->SetColor(color);
      mModuleGridManualCables[i]->SetManualPosition((i % 8) * 12 + 8, (i / 8) * 12 + mHeight + 6);
      AddPatchCableSource(mModuleGridManualCables[i]);
   }
}

void Push2Control::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   if (!ThePushBridge.IsInitialized())
   {
      ofSetColor(255, 0, 0, gModuleDrawAlpha);
      DrawTextNormal(mPushBridgeInitErrMsg, 3, 15);

      mModuleGridLayoutStyleDropdown->SetShowing(false);
      mShowManualGridCheckbox->SetShowing(false);

      for (auto& cable : mModuleGridManualCables)
         cable->SetShowing(false);
   }
   else
   {
      mModuleGridLayoutStyleDropdown->SetShowing(true);
      mShowManualGridCheckbox->SetShowing(true);

      for (auto& cable : mModuleGridManualCables)
         cable->SetShowing(mShowManualGrid);
   }

   mModuleGridLayoutStyleDropdown->Draw();
   mShowManualGridCheckbox->Draw();
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
      DrawDisplayModuleRect(mDisplayModule->GetRect(), 3);
      ofPopMatrix();
      ofPopStyle();
   }
}

void Push2Control::DrawDisplayModuleRect(ofRectangle rect, float thickness)
{
   if (mDisplayModule->HasTitleBar())
   {
      rect.y -= IDrawableModule::TitleBarHeight();
      rect.height += IDrawableModule::TitleBarHeight();
   }
   ofSetColor(255, 255, 255, ofMap(sin(gTime / 1000 * PI * 2), -1, 1, 60, 100));
   ofSetLineWidth(thickness);
   ofNoFill();
   ofRect(rect.x - 3, rect.y - 3, rect.width + 6, rect.height + 6);
}

void Push2Control::PostRender()
{
   if (ThePushBridge.IsInitialized())
      RenderPush2Display();
}

void Push2Control::KeyPressed(int key, bool isRepeat)
{
   IDrawableModule::KeyPressed(key, isRepeat);

   if (key == OF_KEY_DOWN || key == OF_KEY_UP || key == OF_KEY_LEFT || key == OF_KEY_RIGHT)
   {
      for (int i = 0; i < (int)mModuleGridManualCables.size(); ++i)
      {
         if (mModuleGridManualCables[i]->IsHovered())
         {
            int x = i % 8;
            int y = i / 8;
            int newX = x;
            int newY = y;
            if (key == OF_KEY_RIGHT)
               newX = ofClamp(x + 1, 0, 7);
            if (key == OF_KEY_LEFT)
               newX = ofClamp(x - 1, 0, 7);
            if (key == OF_KEY_UP)
               newY = ofClamp(y - 1, 0, 7);
            if (key == OF_KEY_DOWN)
               newY = ofClamp(y + 1, 0, 7);

            IClickable* target = mModuleGridManualCables[x + y * 8]->GetTarget();
            mModuleGridManualCables[x + y * 8]->ClearPatchCables();
            mModuleGridManualCables[newX + newY * 8]->SetTarget(target);
         }
      }
   }
}

void Push2Control::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

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
}

void Push2Control::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << (int)mBookmarkSlots.size();
   for (size_t i = 0; i < mBookmarkSlots.size(); ++i)
   {
      if (mBookmarkSlots[i] != nullptr)
         out << mBookmarkSlots[i]->Path();
      else
         out << std::string("");
   }

   out << (int)mFavoriteControls.size();
   for (size_t i = 0; i < mFavoriteControls.size(); ++i)
   {
      if (mFavoriteControls[i] != nullptr)
         out << mFavoriteControls[i]->Path();
      else
         out << std::string("");
   }
}

void Push2Control::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (!ModuleContainer::DoesModuleHaveMoreSaveData(in))
      return; //this was saved before we added versioning, bail out

   LoadStateValidate(rev >= GetModuleSaveStateRev());

   int numBookmarks;
   in >> numBookmarks;
   mBookmarkSlots.resize(numBookmarks);
   for (int i = 0; i < numBookmarks; ++i)
   {
      std::string path;
      in >> path;
      if (path != "")
         mBookmarkSlots[i] = TheSynth->FindModule(path);
      else
         mBookmarkSlots[i] = nullptr;
   }

   int numFaves;
   in >> numFaves;
   mFavoriteControls.resize(numFaves);
   for (int i = 0; i < numFaves; ++i)
   {
      std::string path;
      in >> path;
      if (path != "")
         mFavoriteControls[i] = TheSynth->FindUIControl(path);
      else
         mFavoriteControls[i] = nullptr;
   }
}

//static
void Push2Control::CreateStaticFramebuffer()
{
   sVG = nvgCreateGLES2(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
   assert(sVG);

   const auto width = ableton::Push2DisplayBitmap::kWidth;
   const auto height = ableton::Push2DisplayBitmap::kHeight;
   const int pixelRatio = 1;

   sFB = nvgluCreateFramebuffer(sVG, width * pixelRatio, height * pixelRatio, 0);
   assert(sFB);
}

bool Push2Control::Initialize()
{
   if (!ThePushBridge.IsInitialized())
   {
      if (auto result = ThePushBridge.Init(); result.Failed())
      {
         mPushBridgeInitErrMsg = result.GetDescription();
         ofLog() << mPushBridgeInitErrMsg;
         return false;
      }
      ofLog() << "push 2 connected";
   }

   mPixels = new unsigned char[GetNumDisplayPixels()];

   mFontHandle = nvgCreateFont(sVG, ofToResourcePath("frabk.ttf").c_str(), ofToResourcePath("frabk.ttf").c_str());
   mFontHandleBold = nvgCreateFont(sVG, ofToResourcePath("frabk_m.ttf").c_str(), ofToResourcePath("frabk_m.ttf").c_str());

   const std::vector<std::string>& devices = mDevice.GetPortList(false);
   for (int i = 0; i < devices.size(); ++i)
   {
#if JUCE_WINDOWS
      if (strcmp(devices[i].c_str(), "Ableton Push 2") == 0)
#else
      if (strcmp(devices[i].c_str(), "Ableton Push 2 Live Port") == 0)
#endif
      {
         mDevice.ConnectOutput(i);
         mDevice.ConnectInput(devices[i].c_str());

         std::string touchStripConfig = { 0x00, 0x21, 0x1D, 0x01, 0x01, 0x17, 0x03 };
         mDevice.SendSysEx(touchStripConfig);
         break;
      }
   }

   return true;
}

int Push2Control::GetNumDisplayPixels() const
{
   return 3 * (ableton::Push2DisplayBitmap::kWidth * kPixelRatio) * (ableton::Push2DisplayBitmap::kHeight * kPixelRatio);
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
   glClear(juce::gl::GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
   nvgBeginFrame(vg, winWidth, winHeight, pxRatio);

   nvgLineCap(vg, NVG_ROUND);
   nvgLineJoin(vg, NVG_ROUND);
   static float sSpacing = -.3f;
   nvgTextLetterSpacing(vg, sSpacing);

   mModules.clear();
   std::vector<IDrawableModule*> modules;
   TheSynth->GetAllModules(modules);
   for (int i = 0; i < modules.size(); ++i)
   {
      if (!IsIgnorableModule(modules[i]))
         mModules.push_back(modules[i]);
   }
   mModules = SortModules(mModules);

   SetModuleGridLights();

   mModuleViewOffsetSmoothed = ofLerp(mModuleViewOffsetSmoothed, mModuleViewOffset, .3f);
   mModuleListOffsetSmoothed = ofLerp(mModuleListOffsetSmoothed, round(mModuleListOffset), .3f);

   if (mScreenDisplayMode == ScreenDisplayMode::kNormal || mScreenDisplayMode == ScreenDisplayMode::kMap)
   {
      DrawLowerModuleSelector();
      DrawDisplayModuleControls();

      std::string stateInfo = "";
      if (gTime - mTextPopupTime < 1000)
         stateInfo = mTextPopup;
      else if (AllowRepatch() && mHeldModule != nullptr)
         stateInfo = "repatch mode: tap destination for " + std::string(mHeldModule->Name());
      else if (mNewButtonHeld)
         stateInfo = "tap control to add favorite...";
      else if (mDeleteButtonHeld)
         stateInfo = "tap control to remove favorite...";
      else if (mLFOButtonHeld)
         stateInfo = "tap a control to add/edit LFO...";
      else if (mAutomateButtonHeld && mCurrentControlRecorder == nullptr)
         stateInfo = "move a control to record automation...";
      else if (mAutomateButtonHeld && mCurrentControlRecorder != nullptr)
         stateInfo = "recording automation, length = " + ofToString(mCurrentControlRecorder->GetLength(), 2);
      else if (mAddModuleBookmarkButtonHeld && mDisplayModule != nullptr)
         stateInfo = "tap a button in the column below this button to bookmark the \"" + std::string(mDisplayModule->Name()) + "\" module...";
      else if (mInMidiControllerBindMode)
         stateInfo = "MIDI bind mode: hold a knob or button, then move/press a MIDI control to bind to that module control";
      else if (mAddTrackHeld && mDisplayModule != nullptr && mDisplayModuleSnapshots == nullptr)
         stateInfo = "press one of the 4 buttons to the right of this screen to create a snapshot";
      else if (mAddTrackHeld && mDisplayModule != nullptr && mDisplayModuleSnapshots != nullptr)
         stateInfo = "press one of the 4 buttons to the right of this screen to store a snapshot";

      if (stateInfo != "")
      {
         ofPushStyle();

         ofFill();
         ofColor bgColor(175, 255, 221);
         bgColor = bgColor * ofMap(sin(gTime / 300 * PI * 2), -1, 1, .7f, 1);
         ofSetColor(bgColor);
         ofRect(1, 120, ableton::Push2DisplayBitmap::kWidth - 2, ableton::Push2DisplayBitmap::kHeight - 120);

         ofNoFill();
         ofSetColor(255, 0, 0);
         ofRect(1, 1, ableton::Push2DisplayBitmap::kWidth - 2, ableton::Push2DisplayBitmap::kHeight - 2);

         ofSetColor(0, 0, 0);
         DrawTextBold(stateInfo, 10, 147, 18);

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
            ofSetColor(IDrawableModule::GetColor(TheSynth->GetModuleFactory()->GetModuleCategory(moduleTypeToSpawn)));
            text = "\ntap grid to spawn " + moduleTypeToSpawn;
         }
         DrawTextBold(text, 5, 80, 18);

         ofSetColor(IDrawableModule::GetColor(kModuleCategory_Other));
         ofNoFill();

         ofTranslate(-kColumnSpacing * mModuleViewOffsetSmoothed, 0);

         nvgFontSize(sVG, 12);
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
         for (int i = mModuleViewOffset * 8; i < list->GetNumValues(); ++i)
         {
            int x = (i % 8) * boxWidth;
            int y = (i / 8) * boxHeight + 10 - mModuleViewOffsetSmoothed * boxHeight;
            ofPushMatrix();
            ofClipWindow(x, y, boxWidth, boxHeight, false);
            ofSetColor(GetSpawnGridColor(i, GetModuleTypeForSpawnList(list)));
            ofRect(x, y, boxWidth, boxHeight);
            ofSetColor(255, 255, 255);
            DrawTextBold(list->GetLabel(i), x + 4, y + 12);
            ofPopMatrix();
         }
      }
      ofPopMatrix();
      ofPopStyle();

      for (int i = 0; i < 8; ++i)
      {
         if (mSelectedGridSpawnListIndex != -1 && i != mSelectedGridSpawnListIndex)
            SetLed(kMidiMessage_Control, i + kAboveScreenButtonRow, 0);
         else if (i < (int)mSpawnModuleControls.size())
            SetLed(kMidiMessage_Control, i + kAboveScreenButtonRow, GetPadColorForType(GetModuleTypeForSpawnList(mSpawnModuleControls[i]), true));
         else
            SetLed(kMidiMessage_Control, i + kAboveScreenButtonRow, 0);
         SetLed(kMidiMessage_Control, i + kBelowScreenButtonRow, 0);
      }
   }
   else if (mScreenDisplayMode == ScreenDisplayMode::kRouting)
   {
      DrawRoutingDisplay();
   }

   if (mScreenDisplayMode == ScreenDisplayMode::kMap)
   {
      ofPushStyle();
      ofPushMatrix();

      ofPushStyle();
      ofFill();
      ofSetColor(0, 0, 0, 150);
      ofRect(0, 0, ableton::Push2DisplayBitmap::kWidth * kPixelRatio, ableton::Push2DisplayBitmap::kHeight * kPixelRatio);
      ofPopStyle();

      float screenScale = .2f;
      ofScale(screenScale * gDrawScale, screenScale * gDrawScale, screenScale * gDrawScale);
      ofTranslate(TheSynth->GetDrawOffset().x, TheSynth->GetDrawOffset().y);
      ofTranslate(1500 / gDrawScale, -100 / gDrawScale); //center on display

      TheSynth->GetRootContainer()->DrawContents();

      if (mDisplayModule != nullptr && !mDisplayModule->IsDeleted() &&
          mDisplayModule->GetOwningContainer() != nullptr && mDisplayModule->IsShowing())
         DrawDisplayModuleRect(mDisplayModule->GetRect(), 6);

      ofPopMatrix();
      ofPopStyle();

      /*for (int i = 0; i < 8; ++i)
      {
         SetLed(kMidiMessage_Control, i + kAboveScreenButtonRow, 0);
         SetLed(kMidiMessage_Control, i + kBelowScreenButtonRow, 0);
      }*/
   }

   bool isHoveringOverNewModule = (gHoveredModule != mDisplayModule && gHoveredModule != nullptr);
   SetLed(kMidiMessage_Control, kPlayButton, TheSynth->IsAudioPaused() ? 127 : 120);
   if (mDisplayModule != nullptr)
      SetLed(kMidiMessage_Control, kCircleButton, mDisplayModule->IsEnabled() ? 126 : 127);
   else
      SetLed(kMidiMessage_Control, kCircleButton, 0);
   SetLed(kMidiMessage_Control, kTapTempoButton, isHoveringOverNewModule ? 127 : 0, isHoveringOverNewModule ? 32 : 0);
   SetLed(kMidiMessage_Control, kMetronomeButton, mDisplayModule == this ? 127 : 8);
   SetLed(kMidiMessage_Control, kConvertButton, mDisplayModule != nullptr ? 127 : 0);
   SetLed(kMidiMessage_Control, kDoubleLoopButton, mDisplayModule != nullptr && (mDisplayModule->GetTypeName() == "looper" || mDisplayModule->GetTypeName() == "notelooper") ? 127 : 0);
   SetLed(kMidiMessage_Control, kNewButton, 127, mNewButtonHeld ? 0 : -1);
   SetLed(kMidiMessage_Control, kDeleteButton, 127, mDeleteButtonHeld ? 0 : -1);
   SetLed(kMidiMessage_Control, kFixedLengthButton, 127, mLFOButtonHeld ? 0 : -1);
   SetLed(kMidiMessage_Control, kAutomateButton, GetPadColorForType(kModuleCategory_Modulator, true), mAutomateButtonHeld ? 0 : -1);
   SetLed(kMidiMessage_Control, kMasterButton, 127, mAddModuleBookmarkButtonHeld ? 0 : -1);
   SetLed(kMidiMessage_Control, kAddDeviceButton, 127, mScreenDisplayMode == ScreenDisplayMode::kAddModule ? 0 : -1);
   SetLed(kMidiMessage_Control, kAddTrackButton, mDisplayModule != nullptr ? 127 : 0, mAddTrackHeld ? 0 : -1);
   SetLed(kMidiMessage_Control, kUserButton, 127, mScreenDisplayMode == ScreenDisplayMode::kMap ? 0 : -1);
   if (mDisplayModuleSnapshots != nullptr)
   {
      SetLed(kMidiMessage_Control, kDeviceButton, mDisplayModuleSnapshots->HasSnapshot(0) ? 127 : 8, mDisplayModuleSnapshots->GetCurrentSnapshot() == 0 ? 0 : -1);
      SetLed(kMidiMessage_Control, kMixButton, mDisplayModuleSnapshots->HasSnapshot(1) ? 127 : 8, mDisplayModuleSnapshots->GetCurrentSnapshot() == 1 ? 0 : -1);
      SetLed(kMidiMessage_Control, kBrowseButton, mDisplayModuleSnapshots->HasSnapshot(2) ? 127 : 8, mDisplayModuleSnapshots->GetCurrentSnapshot() == 2 ? 0 : -1);
      SetLed(kMidiMessage_Control, kClipButton, mDisplayModuleSnapshots->HasSnapshot(3) ? 127 : 8, mDisplayModuleSnapshots->GetCurrentSnapshot() == 3 ? 0 : -1);
   }
   else
   {
      SetLed(kMidiMessage_Control, kDeviceButton, 0, mAddTrackHeld && mDisplayModule != nullptr ? 8 : -1);
      SetLed(kMidiMessage_Control, kMixButton, 0, mAddTrackHeld && mDisplayModule != nullptr ? 8 : -1);
      SetLed(kMidiMessage_Control, kBrowseButton, 0, mAddTrackHeld && mDisplayModule != nullptr ? 8 : -1);
      SetLed(kMidiMessage_Control, kClipButton, 0, mAddTrackHeld && mDisplayModule != nullptr ? 8 : -1);
   }
   SetLed(kMidiMessage_Control, kUpButton, mShiftHeld ? 0 : 127, 127);
   SetLed(kMidiMessage_Control, kDownButton, mShiftHeld ? 0 : 127, 127);
   SetLed(kMidiMessage_Control, kLeftButton, mShiftHeld ? 0 : 127, 127);
   SetLed(kMidiMessage_Control, kRightButton, mShiftHeld ? 0 : 127, 127);
   if (mHeldKnobIndex == -1)
   {
      SetLed(kMidiMessage_Control, kPageLeftButton, mModuleHistoryPosition > 0 ? 127 : 0);
      SetLed(kMidiMessage_Control, kPageRightButton, mModuleHistoryPosition < mModuleHistory.size() - 1 ? 127 : 0);
      SetLed(kMidiMessage_Control, kOctaveUpButton, 127);
      SetLed(kMidiMessage_Control, kOctaveDownButton, 127);
      SetLed(kMidiMessage_Control, kSelectButton, mDisplayModule != nullptr ? 127 : 0);
   }
   else
   {
      SetLed(kMidiMessage_Control, kPageLeftButton, 0, 127);
      SetLed(kMidiMessage_Control, kPageRightButton, 0, 127);
      SetLed(kMidiMessage_Control, kOctaveUpButton, 0, 127);
      SetLed(kMidiMessage_Control, kOctaveDownButton, 0, 127);
      SetLed(kMidiMessage_Control, kSelectButton, 0, 127);
   }
   SetLed(kMidiMessage_Control, kSetupButton, mInMidiControllerBindMode ? 127 : 32, mInMidiControllerBindMode ? 0 : 32);
   if (mGridControlModule != nullptr)
   {
      SetLed(kMidiMessage_Control, kNoteButton, 127, 10);
      SetLed(kMidiMessage_Control, kSessionButton, 10);
      SetLed(kMidiMessage_Control, kScaleButton, mDisplayModule == mGridControlModule ? 127 : 10);
   }
   else
   {
      SetLed(kMidiMessage_Control, kNoteButton, mDisplayModuleCanControlGrid ? 10 : 0);
      SetLed(kMidiMessage_Control, kSessionButton, 127);
      SetLed(kMidiMessage_Control, kScaleButton, 0);
   }
   if (mDisplayModule != nullptr)
      SetLed(kMidiMessage_Control, kLayoutButton, mScreenDisplayMode == ScreenDisplayMode::kRouting ? 127 : 10, mScreenDisplayMode == ScreenDisplayMode::kRouting ? 0 : -1);
   else
      SetLed(kMidiMessage_Control, kLayoutButton, 0);
   for (int i = 0; i < kNumQuantizeButtons; ++i)
   {
      int color = 0;
      if (mBookmarkSlots[i] != nullptr && !mBookmarkSlots[i]->IsDeleted())
         color = GetPadColorForType(mBookmarkSlots[i]->GetModuleCategory(), mBookmarkSlots[i]->IsEnabled());
      SetLed(kMidiMessage_Control, kQuantizeButtonSection + i, color, mDisplayModule == mBookmarkSlots[i] ? 0 : -1);
   }
   SetLed(kMidiMessage_Control, kShiftButton, 127, mShiftHeld ? 0 : -1);

   //test led colors
   //SetLed(kMidiMessage_Note, 92, (int)mModuleListOffset);
   //ofLog() << (int)mModuleListOffset;

   nvgEndFrame(vg);

   glFinish();
   glReadBuffer(juce::gl::GL_COLOR_ATTACHMENT0);
   glReadPixels(0, 0, winWidth, winHeight, juce::gl::GL_RGB, GL_UNSIGNED_BYTE, mPixels);

   nvgluBindFramebuffer(NULL);
}

ofColor Push2Control::GetSpawnGridColor(int index, ModuleCategory moduleType) const
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

int Push2Control::GetSpawnGridPadColor(int index, ModuleCategory moduleType) const
{
   bool bright = false;
   if ((index / 4) % 4 == 0 || (index / 4) % 4 == 3)
      bright = true;

   return GetPadColorForType(moduleType, bright);
}

void Push2Control::SetModuleGridLights()
{
   float minX = 0;
   float minY = 0;
   float maxX = ofGetWidth();
   float maxY = ofGetHeight();
   for (int i = 0; i < mModules.size(); ++i)
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

   if (mModuleGridLayoutStyle == ModuleGridLayoutStyle::Automatic || mScreenDisplayMode == ScreenDisplayMode::kAddModule)
   {
      for (int i = 0; i < (int)mModuleGrid.size(); ++i)
         mModuleGrid[i] = nullptr;

      for (int i = 0; i < mModules.size(); ++i)
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
   }
   else if (mModuleGridLayoutStyle == ModuleGridLayoutStyle::Manual)
   {
      for (int i = 0; i < (int)mModuleGrid.size(); ++i)
         mModuleGrid[i] = static_cast<IDrawableModule*>(mModuleGridManualCables[i]->GetTarget());
   }

   if (mScreenDisplayMode == ScreenDisplayMode::kAddModule && mSelectedGridSpawnListIndex != -1 && mSelectedGridSpawnListIndex < (int)mSpawnLists.GetDropdowns().size())
   {
      auto* list = mSpawnLists.GetDropdowns()[mSelectedGridSpawnListIndex]->GetList();
      for (int i = 0; i < 8 * 8; ++i)
      {
         int gridX = i % 8;
         int gridY = i / 8;
         int gridIndex = gridX + (7 - gridY) * 8 + 36;
         int moduleIndex = i + mModuleViewOffset * 8;
         if (moduleIndex < list->GetNumValues())
            SetLed(kMidiMessage_Note, gridIndex, GetSpawnGridPadColor(moduleIndex, GetModuleTypeForSpawnList(list)));
         else
            SetLed(kMidiMessage_Note, gridIndex, 0);
      }
   }
   else if (mGridControlInterface != nullptr)
   {
      mGridControlInterface->UpdatePush2Leds(this);
   }
   else
   {
      for (int i = 0; i < (int)mModuleGrid.size(); ++i)
      {
         int gridX = i % 8;
         int gridY = i / 8;
         int gridIndex = gridX + (7 - gridY) * 8;
         int padNumber = 36 + i;
         if (mModuleGrid[gridIndex] != nullptr)
            SetLed(kMidiMessage_Note, padNumber, GetPadColorForType(mModuleGrid[gridIndex]->GetModuleCategory(), mModuleGrid[gridIndex]->IsEnabled()), mModuleGrid[gridIndex] == mDisplayModule ? 0 : -1);
         else
            SetLed(kMidiMessage_Note, padNumber, 0);
      }

      //all touchstrip LEDs off
      std::string touchStripLights = { 0x00, 0x21, 0x1D, 0x01, 0x01, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
      GetDevice()->SendSysEx(touchStripLights);
   }
}

void Push2Control::DrawDisplayModuleControls()
{
   if (mDisplayModule != nullptr && mDisplayModule->IsDeleted())
   {
      SetDisplayModule(nullptr, false);
      return;
   }

   ofSetColor(255, 255, 255);
   if (mDisplayModule != nullptr)
   {
      ofPushMatrix();
      ofPushStyle();

      //nvgFontSize(mVG, 16);
      //nvgText(mVG, 10, 10, mDisplayModule->Name(), nullptr);
      float x;
      float y;
      mDisplayModule->GetPosition(x, y, true);
      mDisplayModule->SetPosition(5 - kColumnSpacing * mModuleViewOffsetSmoothed, 15);
      float titleBarHeight;
      float highlight;
      mDisplayModule->DrawFrame(kColumnSpacing * MAX(1, MAX(mSliderControls.size(), mButtonControls.size())) - 14, 80, false, titleBarHeight, highlight);
      mDisplayModule->SetPosition(x, y);

      ofSetColor(IDrawableModule::GetColor(mDisplayModule->GetModuleCategory()));
      ofNoFill();

      nvgFontSize(sVG, 16);
      bool screenDrawingHandled = mDisplayModule->DrawToPush2Screen();
      if (!screenDrawingHandled)
      {
         DrawControls(mButtonControls, false, 60);
         DrawControls(mSliderControls, true, 20);
      }

      int topRowLedColors[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
      for (int i = 0; i < mButtonControls.size(); ++i)
      {
         if (i - mModuleViewOffset >= 0 && i - mModuleViewOffset < 8)
            topRowLedColors[i - mModuleViewOffset] = GetPadColorForType(mButtonControls[i]->GetModuleParent()->GetModuleCategory(), true);
      }
      for (int i = 0; i < 8; ++i)
         SetLed(kMidiMessage_Control, i + kAboveScreenButtonRow, topRowLedColors[i]);

      ofPopMatrix();
      ofPopStyle();

      ofPushStyle();
      ofSetLineWidth(.5f);
      int length = MAX((int)mButtonControls.size(), (int)mSliderControls.size());
      if (length > 8)
      {
         ofRectangle bar(ableton::Push2DisplayBitmap::kWidth * kPixelRatio - 100, 3, 80, 10);
         ofNoFill();
         ofSetColor(100, 100, 100);
         ofRect(bar);
         ofFill();
         ofSetColor(255, 255, 255);
         bar.x += bar.width * mModuleViewOffsetSmoothed / length;
         bar.width *= 8.0f / length;
         ofRect(bar);
      }
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
   int bottomRowLedColors[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
   for (int i = 0; i < mModules.size(); ++i)
   {
      if (i - mModuleListOffset < -1 || i - mModuleListOffset > 8)
         continue;

      ofPushMatrix();
      ofPushStyle();

      ofClipWindow(kColumnSpacing * (i - mModuleListOffsetSmoothed), 0, kColumnSpacing, ableton::Push2DisplayBitmap::kHeight * kPixelRatio, true);

      float x;
      float y;
      mModules[i]->GetPosition(x, y, true);
      mModules[i]->SetPosition(3 + kColumnSpacing * (i - mModuleListOffsetSmoothed), 120);
      float titleBarHeight;
      float highlight;
      mModules[i]->DrawFrame(kColumnSpacing - 14, 80, true, titleBarHeight, highlight);
      if (mModules[i] == mDisplayModule)
         DrawDisplayModuleRect(ofRectangle(0, 0, kColumnSpacing - 14, 80), 3);
      mModules[i]->SetPosition(x, y);

      if (i - round(mModuleListOffset) >= 0 && i - round(mModuleListOffset) < 8)
         bottomRowLedColors[i - (int)round(mModuleListOffset)] = GetPadColorForType(mModules[i]->GetModuleCategory(), true);

      ofPopMatrix();
      ofPopStyle();
   }

   for (int i = 0; i < 8; ++i)
      SetLed(kMidiMessage_Control, i + kBelowScreenButtonRow, bottomRowLedColors[i]);
}

void Push2Control::DrawRoutingDisplay()
{
   int topRowLedColors[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
   int bottomRowLedColors[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
   for (int i = 0; i < mRoutingInputModules.size(); ++i)
   {
      ofPushMatrix();
      ofPushStyle();

      ofSetColor(mRoutingInputModules[i].mConnectionColor);
      ofLine(kColumnSpacing * (i + .5f), 37, kColumnSpacing * .5f, 60);

      ofClipWindow(kColumnSpacing * i, 0, kColumnSpacing, ableton::Push2DisplayBitmap::kHeight * kPixelRatio, true);

      float x;
      float y;
      mRoutingInputModules[i].mModule->GetPosition(x, y, true);
      mRoutingInputModules[i].mModule->SetPosition(3 + kColumnSpacing * i, 12);
      float titleBarHeight;
      float highlight;
      mRoutingInputModules[i].mModule->DrawFrame(kColumnSpacing - 14, 25, true, titleBarHeight, highlight);
      mRoutingInputModules[i].mModule->SetPosition(x, y);

      if (i >= 0 && i < 8)
         topRowLedColors[i] = GetPadColorForType(mRoutingInputModules[i].mModule->GetModuleCategory(), true);

      ofPopMatrix();
      ofPopStyle();
   }

   for (int i = 0; i < mRoutingOutputModules.size(); ++i)
   {
      ofPushMatrix();
      ofPushStyle();

      ofSetColor(mRoutingOutputModules[i].mConnectionColor);
      ofLine(kColumnSpacing * .5f, 97, kColumnSpacing * (i + .5f), 120);

      ofClipWindow(kColumnSpacing * i, 0, kColumnSpacing, ableton::Push2DisplayBitmap::kHeight * kPixelRatio, true);

      float x;
      float y;
      mRoutingOutputModules[i].mModule->GetPosition(x, y, true);
      mRoutingOutputModules[i].mModule->SetPosition(3 + kColumnSpacing * i, 132);
      float titleBarHeight;
      float highlight;
      mRoutingOutputModules[i].mModule->DrawFrame(kColumnSpacing - 14, 25, true, titleBarHeight, highlight);
      mRoutingOutputModules[i].mModule->SetPosition(x, y);

      if (i >= 0 && i < 8)
         bottomRowLedColors[i] = GetPadColorForType(mRoutingOutputModules[i].mModule->GetModuleCategory(), true);

      ofPopMatrix();
      ofPopStyle();
   }

   {
      ofPushMatrix();
      ofPushStyle();

      ofClipWindow(0, 0, kColumnSpacing, ableton::Push2DisplayBitmap::kHeight * kPixelRatio, true);

      float x;
      float y;
      mDisplayModule->GetPosition(x, y, true);
      mDisplayModule->SetPosition(3, 72);
      float titleBarHeight;
      float highlight;
      mDisplayModule->DrawFrame(kColumnSpacing - 14, 25, true, titleBarHeight, highlight);
      mDisplayModule->SetPosition(x, y);

      ofPopMatrix();
      ofPopStyle();
   }

   for (int i = 0; i < 8; ++i)
   {
      SetLed(kMidiMessage_Control, i + kAboveScreenButtonRow, topRowLedColors[i]);
      SetLed(kMidiMessage_Control, i + kBelowScreenButtonRow, bottomRowLedColors[i]);
   }
}

int Push2Control::GetPadColorForType(ModuleCategory type, bool enabled) const
{
   int color;
   switch (type)
   {
      case kModuleCategory_Instrument:
         color = enabled ? 26 : 116;
         break;
      case kModuleCategory_Note:
         color = enabled ? 8 : 80;
         break;
      case kModuleCategory_Synth:
         color = enabled ? 11 : 86;
         break;
      case kModuleCategory_Audio:
         color = enabled ? 18 : 96;
         break;
      case kModuleCategory_Modulator:
         color = enabled ? 22 : 110;
         break;
      case kModuleCategory_Pulse:
         color = enabled ? 9 : 82;
         break;
      default:
         color = enabled ? 118 : 119;
         break;
   }
   return color;
}

ModuleCategory Push2Control::GetModuleTypeForSpawnList(IUIControl* control)
{
   ModuleCategory moduleType = kModuleCategory_Other;
   if (control == mSpawnLists.mInstrumentModules.GetList())
      moduleType = kModuleCategory_Instrument;
   if (control == mSpawnLists.mNoteModules.GetList())
      moduleType = kModuleCategory_Note;
   if (control == mSpawnLists.mSynthModules.GetList())
      moduleType = kModuleCategory_Synth;
   if (control == mSpawnLists.mAudioModules.GetList())
      moduleType = kModuleCategory_Audio;
   if (control == mSpawnLists.mModulatorModules.GetList())
      moduleType = kModuleCategory_Modulator;
   if (control == mSpawnLists.mPulseModules.GetList())
      moduleType = kModuleCategory_Pulse;
   if (control == mSpawnLists.mOtherModules.GetList())
      moduleType = kModuleCategory_Other;
   if (control == mSpawnLists.mPlugins.GetList())
      moduleType = kModuleCategory_Synth;
   if (control == mSpawnLists.mPrefabs.GetList())
      moduleType = kModuleCategory_Other;
   return moduleType;
}

void Push2Control::DrawControls(std::vector<IUIControl*> controls, bool sliders, float yPos)
{
   for (int i = (int)controls.size() - 1; i >= 0; --i)
   {
      if (i - mModuleViewOffset < -1 || i - mModuleViewOffset > 8)
         continue;

      ADSRDisplay* adsr = dynamic_cast<ADSRDisplay*>(controls[i]);

      ofRectangle originalRect = controls[i]->GetRect(true);
      if (adsr != nullptr)
      {
         adsr->SetDimensions(80, 30);
         controls[i]->SetPosition(kColumnSpacing * i + 3, yPos - 13);
      }
      else
      {
         controls[i]->SetPosition(kColumnSpacing * i + 3, yPos);
      }
      ofPushMatrix();
      ofClipWindow(kColumnSpacing * i, yPos - 15, kColumnSpacing, 100, true);

      ofPushStyle();
      ModuleCategory moduleType = controls[i]->GetModuleParent()->GetModuleCategory();
      if (mScreenDisplayMode == ScreenDisplayMode::kAddModule)
         moduleType = GetModuleTypeForSpawnList(controls[i]);
      if (controls[i]->IsShowing())
         ofSetColor(IDrawableModule::GetColor(moduleType));
      else
         ofSetColor(100, 100, 100);
      if (adsr == nullptr)
      {
         if (mDisplayModule == this)
            DrawTextBold(juce::String(controls[i]->Path()).replace("~", "\n").toRawUTF8(), kColumnSpacing * i + 3, yPos - 12, 8);
         else
            DrawTextBold(controls[i]->Name(), kColumnSpacing * i + 3, yPos - 5, 14);
      }
      controls[i]->Render();
      ofPopStyle();

      ofPopMatrix();
      controls[i]->SetPosition(originalRect.x, originalRect.y);
      if (adsr != nullptr)
         adsr->SetDimensions(originalRect.width, originalRect.height);

      ofPushStyle();
      int pushControlIndex = i - mModuleViewOffset;
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
            ofColor color = IDrawableModule::GetColor(controls[i]->GetModuleParent()->GetModuleCategory());
            color.a = 25;
            ofSetColor(color);
            //ofCircle(w - 4, h * controls[i]->GetMidiValue(), 2);
            ofRect(0, h * controls[i]->GetMidiValue() - kCentering, w, controls[i]->GetDimensions().y);
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
   DrawToFramebuffer(sVG, sFB, gTime / 300, kPixelRatio);
   sDrawingPush2Display = false;
   gNanoVG = mainVG;

   // Tells the bridge we're done with drawing and the frame can be sent to the display
   ThePushBridge.Flip(mPixels);
}

void Push2Control::Poll()
{
   if (mPendingSpawnPitch != -1)
   {
      int padNum = mPendingSpawnPitch - 36;
      int gridX = padNum % 8;
      int gridY = padNum / 8;
      int gridIndex = gridX + (7 - gridY) * 8;
      IDrawableModule* existingModule = mModuleGrid[gridIndex];
      ofVec2f newModulePos;
      if (existingModule != nullptr)
      {
         ofRectangle existingModuleRect = existingModule->GetRect();
         newModulePos = ofVec2f(existingModuleRect.getMinX(), existingModuleRect.getMaxY() + 40);
      }
      else
      {
         newModulePos = ofVec2f(ofMap(gridX, 0, 7, mModuleGridRect.getMinX(), mModuleGridRect.getMaxX()), ofMap(gridY, 7, 0, mModuleGridRect.getMinY(), mModuleGridRect.getMaxY()));
      }

      for (int i = 0; i < mSpawnLists.GetDropdowns().size(); ++i)
      {
         if (mSpawnLists.GetDropdowns()[i]->GetList()->GetValue() != -1)
         {
            IDrawableModule* module = mSpawnLists.GetDropdowns()[i]->Spawn(mSpawnLists.GetDropdowns()[i]->GetList()->GetValue());
            module->SetPosition(newModulePos.x, newModulePos.y);
            mSpawnLists.GetDropdowns()[i]->GetList()->SetValue(-1, gTime);
            mScreenDisplayMode = ScreenDisplayMode::kNormal;
            SetDisplayModule(module, true);

            if (existingModule != nullptr && existingModule->GetPatchCableSource() != nullptr)
            {
               IClickable* insertTarget = existingModule->GetPatchCableSource()->GetTarget();

               existingModule->GetPatchCableSource()->FindValidTargets();
               if (existingModule->GetPatchCableSource()->IsValidTarget(module))
                  existingModule->SetTarget(module);

               if (module->GetPatchCableSource())
               {
                  module->GetPatchCableSource()->FindValidTargets();
                  if (insertTarget != nullptr && module->GetPatchCableSource()->IsValidTarget(insertTarget))
                     module->SetTarget(insertTarget);
               }
            }
            break;
         }
      }

      mPendingSpawnPitch = -1;
   }

   bool controlsChanged = false;
   if (mDisplayModule != nullptr && mDisplayModule->HasPush2OverrideControls())
   {
      std::vector<IUIControl*> desiredControls;
      mDisplayModule->GetPush2OverrideControls(desiredControls);
      if (desiredControls.size() != mDisplayedControls.size())
      {
         controlsChanged = true;
      }
      else
      {
         for (size_t i = 0; i < desiredControls.size(); ++i)
         {
            if (desiredControls[i] != mDisplayedControls[i])
            {
               controlsChanged = true;
               break;
            }
         }
      }
   }

   if (mDisplayModule != nullptr && mDisplayModule->HasPush2OverrideControls() != mDisplayModuleIsShowingOverrideControls)
   {
      controlsChanged = true;
      mDisplayModuleIsShowingOverrideControls = mDisplayModule->HasPush2OverrideControls();
   }

   if (controlsChanged)
      UpdateControlList();
}

bool Push2Control::AllowRepatch() const
{
   if (mHeldModule)
      return gTime - mModuleHeldTime > 300;
   return false;
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
   mModuleViewOffset = 0;
   mModuleViewOffsetSmoothed = 0;
   UpdateControlList();

   mDisplayModuleSnapshots = nullptr;
   std::vector<IDrawableModule*> modules;
   TheSynth->GetAllModules(modules);
   for (auto* searchModule : modules)
   {
      Snapshots* snapshotsModule = dynamic_cast<Snapshots*>(searchModule);
      if (snapshotsModule != nullptr)
      {
         if (snapshotsModule->IsTargetingModule(module))
            mDisplayModuleSnapshots = snapshotsModule;
      }
   }

   if (module != nullptr && addToHistory && (mModuleHistory.empty() || mModuleHistory[mModuleHistory.size() - 1] != module))
   {
      while (mModuleHistory.size() > mModuleHistoryPosition + 1)
         mModuleHistory.pop_back();
      mModuleHistory.push_back(module);
      ++mModuleHistoryPosition;
   }

   if (mScreenDisplayMode == ScreenDisplayMode::kRouting)
      UpdateRoutingModules();
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

   for (int i = 0; i < controls.size(); ++i)
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
            mDevice.SendCC(index - 128, flashColor);
         mDevice.SendCC(index - 128, color, channel);
      }
   }
}

void Push2Control::SetGridControlInterface(IPush2GridController* controller, IDrawableModule* module)
{
   mGridControlInterface = controller;
   mGridControlModule = module;
   SetLed(kMidiMessage_Control, GetGridControllerOption1Control(), 0);
   SetLed(kMidiMessage_Control, GetGridControllerOption2Control(), 0);
}

void Push2Control::OnMidiNote(MidiNote& note)
{
   if (mGridControlInterface != nullptr)
   {
      bool handled = mGridControlInterface->OnPush2Control(this, kMidiMessage_Note, note.mPitch, note.mVelocity);
      if (handled)
         return;
   }

   if (note.mPitch >= 0 && note.mPitch <= 7) //main encoders
   {
      if (mScreenDisplayMode == ScreenDisplayMode::kNormal || mScreenDisplayMode == ScreenDisplayMode::kMap)
      {
         int controlIndex = note.mPitch + mModuleViewOffset;
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
               else if (mLFOButtonHeld)
               {
                  FloatSlider* slider = dynamic_cast<FloatSlider*>(mSliderControls[controlIndex]);
                  if (slider != nullptr)
                  {
                     bool hadLFO = (slider->GetLFO() != nullptr);
                     FloatSliderLFOControl* lfo = slider->AcquireLFO();
                     if (!hadLFO)
                        lfo->SetLFOEnabled(true);
                     SetDisplayModule(lfo, true);
                  }
               }
               else if (mAutomateButtonHeld)
               {
                  if (mSliderControls[controlIndex] != nullptr && mCurrentControlRecorder == nullptr)
                  {
                     std::vector<IDrawableModule*> modules;
                     TheSynth->GetAllModules(modules);
                     for (auto* searchModule : modules)
                     {
                        ControlRecorder* controlRecorderModule = dynamic_cast<ControlRecorder*>(searchModule);
                        if (controlRecorderModule != nullptr)
                        {
                           if (controlRecorderModule->GetPatchCableSource()->GetTarget() == mSliderControls[controlIndex])
                              mCurrentControlRecorder = controlRecorderModule;
                        }
                     }

                     if (mCurrentControlRecorder == nullptr) //couldn't find one, so make one
                     {
                        ModuleFactory::Spawnable spawnable;
                        spawnable.mLabel = "controlrecorder";
                        mCurrentControlRecorder = dynamic_cast<ControlRecorder*>(TheSynth->SpawnModuleOnTheFly(spawnable, mSliderControls[controlIndex]->GetRect().getMaxX() + 40, mSliderControls[controlIndex]->GetPosition().y));
                        mCurrentControlRecorder->SetTarget(mSliderControls[controlIndex]);
                     }

                     mCurrentControlRecorder->SetEnabled(true);
                     mCurrentControlRecorder->SetRecording(true);
                  }
               }
               else if (mInMidiControllerBindMode)
               {
                  sBindToUIControl = mSliderControls[controlIndex];
               }

               if (mHeldModule && AllowRepatch())
               {
                  PatchCableSource* cable = mHeldModule->GetPatchCableSource();
                  if (mHeldModulePatchCableIndex > 0)
                     cable = mHeldModule->GetPatchCableSources()[mHeldModulePatchCableIndex];
                  if (cable != nullptr)
                  {
                     cable->FindValidTargets();
                     if (cable->IsValidTarget(mSliderControls[controlIndex]))
                        cable->SetTarget(mSliderControls[controlIndex]);
                  }
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
            for (int i = 0; i < mSpawnLists.GetDropdowns().size(); ++i)
            {
               if (i != note.mPitch)
                  mSpawnLists.GetDropdowns()[i]->GetList()->SetValue(-1, gTime);
            }
         }
      }

      if (note.mVelocity > 0)
         mHeldKnobIndex = note.mPitch;
      else
         mHeldKnobIndex = -1;
   }
   else if (note.mPitch >= 36 && note.mPitch <= 99 && mGridControlInterface == nullptr) //pads
   {
      if (mScreenDisplayMode == ScreenDisplayMode::kAddModule && mSelectedGridSpawnListIndex != -1 && mSelectedGridSpawnListIndex < (int)mSpawnLists.GetDropdowns().size())
      {
         if (note.mVelocity > 0)
         {
            auto* list = mSpawnLists.GetDropdowns()[mSelectedGridSpawnListIndex]->GetList();
            int padNum = note.mPitch - 36;
            int gridX = padNum % 8;
            int gridY = padNum / 8;
            int gridIndex = gridX + (7 - gridY) * 8 + mModuleViewOffset * 8;
            if (gridIndex < list->GetNumValues())
            {
               list->SetValueDirect(gridIndex, gTime);
               mSelectedGridSpawnListIndex = -1;
            }
         }
      }
      else if (mScreenDisplayMode == ScreenDisplayMode::kAddModule)
      {
         if (note.mVelocity > 0)
            mPendingSpawnPitch = note.mPitch;
      }
      else if (mGridControlInterface == nullptr)
      {
         int padNum = note.mPitch - 36;
         int gridX = padNum % 8;
         int gridY = padNum / 8;
         int gridIndex = gridX + (7 - gridY) * 8;
         if (mModuleGrid[gridIndex] != nullptr)
         {
            if (note.mVelocity > 0 && mHeldModule == nullptr)
               mModuleHeldTime = gTime;
            if (note.mVelocity == 0 && !mRepatchedHeldModule)
               SetDisplayModule(mModuleGrid[gridIndex], true);
         }

         if (note.mVelocity > 0)
         {
            if (mHeldModule != nullptr)
            {
               PatchCableSource* heldModuleCable = mHeldModule->GetPatchCableSource();
               if (mHeldModulePatchCableIndex > 0)
                  heldModuleCable = mHeldModule->GetPatchCableSources()[mHeldModulePatchCableIndex];
               if (AllowRepatch() && heldModuleCable != nullptr)
               {
                  IDrawableModule* touchedModule = mModuleGrid[gridIndex];

                  heldModuleCable->FindValidTargets();
                  if (heldModuleCable->IsValidTarget(touchedModule))
                  {
                     //insert
                     if (mShiftHeld && touchedModule != nullptr && touchedModule->GetPatchCableSource() != nullptr)
                     {
                        IClickable* oldTarget = heldModuleCable->GetTarget();

                        touchedModule->GetPatchCableSource()->FindValidTargets();
                        if (touchedModule->GetPatchCableSource()->IsValidTarget(oldTarget))
                           touchedModule->SetTarget(oldTarget);
                     }

                     heldModuleCable->SetTarget(touchedModule);
                  }
                  else
                  {
                     heldModuleCable->ClearPatchCables();
                  }
                  mRepatchedHeldModule = true;
               }
            }
            else
            {
               mHeldModule = mModuleGrid[gridIndex];
               mRepatchedHeldModule = false;
               mHeldModulePatchCableIndex = 0;
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
   if (mGridControlInterface != nullptr)
   {
      bool handled = mGridControlInterface->OnPush2Control(this, kMidiMessage_Control, control.mControl, control.mValue);
      if (handled)
         return;
   }

   if (control.mControl >= 71 && control.mControl <= 78) //main encoders
   {
      int controlIndex = control.mControl - 71 + mModuleViewOffset;
      bool justResetParameter = gTime - mLastResetTime < 1000;
      if (controlIndex < mSliderControls.size() && !justResetParameter)
      {
         float currentNormalized = mSliderControls[controlIndex]->GetMidiValue();
         float increment = control.mValue < 64 ? control.mValue : control.mValue - 128;
         increment *= mShiftHeld ? .0005f : .005f;

         FloatSlider* floatSlider = dynamic_cast<FloatSlider*>(mSliderControls[controlIndex]);
         if (floatSlider && floatSlider->GetModulator() && floatSlider->GetModulator()->Active() && floatSlider->GetModulator()->CanAdjustRange())
         {
            IModulator* modulator = floatSlider->GetModulator();
            float min = floatSlider->GetMin();
            float max = floatSlider->GetMax();
            float modMin = ofMap(modulator->GetMin(), min, max, 0, 1);
            float modMax = ofMap(modulator->GetMax(), min, max, 0, 1);

            modulator->GetMin() = ofMap(modMin - increment, 0, 1, min, max, K(clamp));
            modulator->GetMax() = ofMap(modMax + increment, 0, 1, min, max, K(clamp));
         }
         else
         {
            mSliderControls[controlIndex]->SetFromMidiCC(currentNormalized + increment, NextBufferTime(false), false);
         }
      }
   }
   else if (control.mControl >= kAboveScreenButtonRow && control.mControl < kAboveScreenButtonRow + 8) //buttons below encoders
   {
      int controlIndex = control.mControl - kAboveScreenButtonRow;
      if (mScreenDisplayMode == ScreenDisplayMode::kNormal || mScreenDisplayMode == ScreenDisplayMode::kMap)
      {
         controlIndex += mModuleViewOffset;
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
                  if (dynamic_cast<ClickButton*>(mButtonControls[controlIndex]) != nullptr)
                     newValue = 1; //always "press" a button

                  mButtonControls[controlIndex]->SetFromMidiCC(newValue, NextBufferTime(false), false);
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
               mSpawnLists.GetDropdowns()[i]->GetList()->SetValue(-1, gTime);

            mModuleViewOffset = 0;
            mModuleViewOffsetSmoothed = 0;
         }
      }

      if (mScreenDisplayMode == ScreenDisplayMode::kRouting)
      {
         int index = control.mControl - kAboveScreenButtonRow;
         if (control.mValue > 0 && index < mRoutingInputModules.size())
            SetDisplayModule(mRoutingInputModules[index].mModule, true);
      }
   }
   else if (control.mControl == 14) //leftmost clicky encoder
   {
      int increment = control.mValue < 64 ? control.mValue : control.mValue - 128;
      if (mScreenDisplayMode == ScreenDisplayMode::kAddModule)
         mModuleViewOffset = std::max(0, mModuleViewOffset + increment);
      else
         mModuleViewOffset = (int)ofClamp(mModuleViewOffset + increment, 0, MAX(0, (int)MAX(mSliderControls.size(), mButtonControls.size()) - 8));
   }
   else if (control.mControl == 15) //encoder next to above encoder
   {
      float increment = control.mValue < 64 ? control.mValue : control.mValue - 128;
      increment *= .05f;
      mModuleListOffset += increment;
   }
   else if (control.mControl >= kBelowScreenButtonRow && control.mControl < kBelowScreenButtonRow + 8) //buttons below screen
   {
      if (mScreenDisplayMode == ScreenDisplayMode::kNormal || mScreenDisplayMode == ScreenDisplayMode::kMap)
      {
         int moduleIndex = control.mControl - kBelowScreenButtonRow + round(mModuleListOffset);
         if (control.mValue > 0 && moduleIndex < mModules.size())
            SetDisplayModule(mModules[moduleIndex], true);
      }

      if (mScreenDisplayMode == ScreenDisplayMode::kRouting)
      {
         int index = control.mControl - kBelowScreenButtonRow;
         if (control.mValue > 0 && index < mRoutingOutputModules.size())
            SetDisplayModule(mRoutingOutputModules[index].mModule, true);
      }
   }
   else if (control.mControl == kSetupButton && control.mValue > 0)
   {
      mInMidiControllerBindMode = !mInMidiControllerBindMode;
   }
   else if (control.mControl == kNewButton)
   {
      mNewButtonHeld = control.mValue > 0;
   }
   else if (control.mControl == kDeleteButton)
   {
      mDeleteButtonHeld = control.mValue > 0;
   }
   else if (control.mControl == kFixedLengthButton)
   {
      mLFOButtonHeld = control.mValue > 0;
   }
   else if (control.mControl == kAutomateButton)
   {
      mAutomateButtonHeld = control.mValue > 0;

      if (!mAutomateButtonHeld && mCurrentControlRecorder != nullptr)
      {
         mCurrentControlRecorder->SetRecording(false);
         mCurrentControlRecorder = nullptr;
      }
   }
   else if (control.mControl == kMasterButton)
   {
      mAddModuleBookmarkButtonHeld = control.mValue > 0;
   }
   else if (control.mControl == kTapTempoButton)
   {
      if (gHoveredModule != nullptr && gHoveredModule != mDisplayModule)
         SetDisplayModule(gHoveredModule, true);
   }
   else if (control.mControl == kMetronomeButton)
   {
      SetDisplayModule(this, true);
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
            SetGridControlInterface(nullptr, nullptr);
            for (int i = 0; i < mSpawnLists.GetDropdowns().size(); ++i)
               mSpawnLists.GetDropdowns()[i]->GetList()->SetValue(-1, gTime);
            mSelectedGridSpawnListIndex = -1;
         }

         mModuleViewOffset = 0;
         mModuleViewOffsetSmoothed = 0;
         mModuleListOffset = 0;
         mModuleListOffsetSmoothed = 0;
         UpdateControlList();
      }
   }
   else if (control.mControl == kUserButton)
   {
      if (control.mValue > 0)
      {
         if (mScreenDisplayMode == ScreenDisplayMode::kMap)
         {
            mScreenDisplayMode = ScreenDisplayMode::kNormal;
         }
         else
         {
            mScreenDisplayMode = ScreenDisplayMode::kMap;
         }
      }
   }
   else if (control.mControl == kConvertButton)
   {
      if (control.mValue > 0 && mDisplayModule != nullptr)
      {
         if (mDisplayModule->GetPatchCableSource() != nullptr &&
             mDisplayModule->GetPatchCableSource()->GetConnectionType() == kConnectionType_Audio &&
             mDisplayModule->GetPatchCableSource()->GetTarget() != nullptr &&
             dynamic_cast<IDrawableModule*>(mDisplayModule->GetPatchCableSource()->GetTarget())->GetTypeName() != "looper")
         {
            ModuleFactory::Spawnable spawnable;
            spawnable.mLabel = "looper";
            IDrawableModule* looper = TheSynth->SpawnModuleOnTheFly(spawnable, mDisplayModule->GetRect().getMinX(), mDisplayModule->GetRect().getMaxY() + 40);
            looper->SetTarget(mDisplayModule->GetPatchCableSource()->GetTarget());
            mDisplayModule->SetTarget(looper);
            SetDisplayModule(looper);
         }

         if (mDisplayModule->GetPatchCableSource() != nullptr &&
             mDisplayModule->GetPatchCableSource()->GetConnectionType() == kConnectionType_Note &&
             mDisplayModule->GetPatchCableSource()->GetTarget() != nullptr &&
             dynamic_cast<IDrawableModule*>(mDisplayModule->GetPatchCableSource()->GetTarget())->GetTypeName() != "notelooper")
         {
            ModuleFactory::Spawnable spawnable;
            spawnable.mLabel = "notelooper";
            IDrawableModule* notelooper = TheSynth->SpawnModuleOnTheFly(spawnable, mDisplayModule->GetRect().getMinX(), mDisplayModule->GetRect().getMaxY() + 40);
            notelooper->SetTarget(mDisplayModule->GetPatchCableSource()->GetTarget());
            mDisplayModule->SetTarget(notelooper);
            SetDisplayModule(notelooper);
         }
      }
   }
   else if (control.mControl == kDoubleLoopButton)
   {
      if (control.mValue > 0)
      {
         Looper* looper = dynamic_cast<Looper*>(mDisplayModule);
         if (looper != nullptr)
            looper->SetNumBars(looper->GetNumBars() * 2);

         NoteLooper* noteLooper = dynamic_cast<NoteLooper*>(mDisplayModule);
         if (noteLooper != nullptr)
            noteLooper->SetNumMeasures(noteLooper->GetNumMeasures() * 2);
      }
   }
   else if (control.mControl == kDeviceButton || control.mControl == kMixButton || control.mControl == kBrowseButton || control.mControl == kClipButton)
   {
      if (control.mValue > 0)
      {
         int index = 0;
         if (control.mControl == kDeviceButton)
            index = 0;
         if (control.mControl == kMixButton)
            index = 1;
         if (control.mControl == kBrowseButton)
            index = 2;
         if (control.mControl == kClipButton)
            index = 3;

         if (mAddTrackHeld)
         {
            if (mDisplayModuleSnapshots == nullptr)
            {
               ModuleFactory::Spawnable spawnable;
               spawnable.mLabel = "snapshots";
               mDisplayModuleSnapshots = dynamic_cast<Snapshots*>(TheSynth->SpawnModuleOnTheFly(spawnable, mDisplayModule->GetRect().getMaxX() + 40, mDisplayModule->GetPosition().y));
               mDisplayModuleSnapshots->AddSnapshotTarget(mDisplayModule);
            }

            if (mDisplayModuleSnapshots != nullptr)
               mDisplayModuleSnapshots->StoreSnapshot(index, true);
         }
         else
         {
            if (mDisplayModuleSnapshots != nullptr)
               mDisplayModuleSnapshots->SetSnapshot(index, gTime);
         }
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

         if (mShiftHeld && mDisplayModule)
         {
            ofVec2f pos = mDisplayModule->GetPosition();
            pos += direction * 50;
            mDisplayModule->SetPosition(pos.x, pos.y);
         }
         else
         {
            TheSynth->PanView(direction.x * -100, direction.y * -100);
         }
      }
   }
   else if (control.mControl == kPageLeftButton ||
            control.mControl == kPageRightButton ||
            control.mControl == kOctaveUpButton ||
            control.mControl == kOctaveDownButton)
   {
      if (control.mValue > 0)
      {
         if (mHeldKnobIndex == -1)
         {
            int direction = 0;
            if (control.mControl == kPageLeftButton)
               direction -= 1;
            if (control.mControl == kPageRightButton)
               direction += 1;

            if (direction != 0)
            {
               int newHistoryPos = mModuleHistoryPosition + direction;
               if (newHistoryPos >= 0 && newHistoryPos < mModuleHistory.size())
               {
                  mModuleHistoryPosition = newHistoryPos;
                  SetDisplayModule(mModuleHistory[mModuleHistoryPosition], false);
               }
            }

            if (control.mControl == kOctaveUpButton)
            {
               std::vector<IDrawableModule*> modules;
               TheSynth->GetAllModules(modules);
               for (auto* module : modules)
               {
                  if (module->GetPatchCableSource() != nullptr &&
                      module->GetPatchCableSource()->GetTarget() == mDisplayModule &&
                      dynamic_cast<Snapshots*>(module) == nullptr) //don't jump up to a snapshot, just things targeting as input
                  {
                     SetDisplayModule(module, true);
                     break;
                  }
               }
            }

            if (control.mControl == kOctaveDownButton && mDisplayModule != nullptr)
            {
               IDrawableModule* target = (mDisplayModule->GetPatchCableSource() != nullptr) ? dynamic_cast<IDrawableModule*>(mDisplayModule->GetPatchCableSource()->GetTarget()) : nullptr;
               if (target)
                  SetDisplayModule(target, true);
            }
         }
         else
         {
            int controlIndex = mHeldKnobIndex + mModuleViewOffset;
            if (controlIndex < mSliderControls.size() && (mScreenDisplayMode == ScreenDisplayMode::kNormal || mScreenDisplayMode == ScreenDisplayMode::kMap))
            {
               if (control.mControl == kPageRightButton)
                  mSliderControls[controlIndex]->Increment(1);
               if (control.mControl == kPageLeftButton)
                  mSliderControls[controlIndex]->Increment(-1);
               if (control.mControl == kOctaveUpButton)
                  mSliderControls[controlIndex]->Double();
               if (control.mControl == kOctaveDownButton)
                  mSliderControls[controlIndex]->Halve();
            }
         }
      }
   }
   else if (control.mControl == kNoteButton)
   {
      if (control.mValue > 0)
      {
         IPush2GridController* controller = dynamic_cast<IPush2GridController*>(mDisplayModule);
         if (controller != nullptr && controller != mGridControlInterface)
         {
            SetGridControlInterface(controller, mDisplayModule);

            for (int i = 36; i <= 99; ++i)
               SetLed(kMidiMessage_Note, i, 0);
            //turn touch strip off
            std::string touchStripLights = { 0x00, 0x21, 0x1D, 0x01, 0x01, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
            GetDevice()->SendSysEx(touchStripLights);
            mGridControlInterface->OnPush2Connect();

            mScreenDisplayMode = ScreenDisplayMode::kNormal;
            UpdateControlList();
         }
      }
   }
   else if (control.mControl == kSessionButton)
   {
      if (control.mValue > 0)
         SetGridControlInterface(nullptr, nullptr);
   }
   else if (control.mControl == kScaleButton)
   {
      if (control.mValue > 0 && mGridControlModule != nullptr)
      {
         SetDisplayModule(mGridControlModule, true);
      }
   }
   else if (control.mControl == kLayoutButton && mDisplayModule != nullptr)
   {
      if (control.mValue > 0)
      {
         if (mScreenDisplayMode == ScreenDisplayMode::kRouting)
         {
            mScreenDisplayMode = ScreenDisplayMode::kNormal;
         }
         else
         {
            mScreenDisplayMode = ScreenDisplayMode::kRouting;

            UpdateRoutingModules();
         }
      }
   }
   else if (control.mControl >= kQuantizeButtonSection && control.mControl < kQuantizeButtonSection + kNumQuantizeButtons) //quantization level buttons to the right of the main grid, used to bookmark modules
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
   else if (control.mControl == kShiftButton)
   {
      mShiftHeld = control.mValue > 0;
   }
   else if (control.mControl == kAddTrackButton)
   {
      mAddTrackHeld = control.mValue > 0;
   }
   else if (control.mControl == kSelectButton)
   {
      if (control.mValue > 0 && mDisplayModule != nullptr)
      {
         if (mHeldModule != nullptr && mHeldModule->GetPatchCableSources().size() > 1)
         {
            mHeldModulePatchCableIndex = (mHeldModulePatchCableIndex + 1) % mHeldModule->GetPatchCableSources().size();
            mTextPopup = "setting selected cable output index to " + ofToString(mHeldModulePatchCableIndex);
            mTextPopupTime = gTime;
         }
         else if (mHeldKnobIndex == -1)
         {
            ofRectangle rect = mDisplayModule->GetRect();
            TheSynth->PanTo(rect.getCenter().x, rect.getCenter().y);
         }
         else
         {
            int controlIndex = mHeldKnobIndex + mModuleViewOffset;
            if (controlIndex < mSliderControls.size() && (mScreenDisplayMode == ScreenDisplayMode::kNormal || mScreenDisplayMode == ScreenDisplayMode::kMap))
            {
               mSliderControls[controlIndex]->ResetToOriginal();
               mLastResetTime = gTime;
            }
         }
      }
   }
   else if (control.mControl == kPlayButton)
   {
      if (control.mValue > 0)
      {
         if (TheSynth->IsAudioPaused())
            TheTransport->Reset();
         else
            TheSynth->SetAudioPaused(true);
      }
   }
   else if (control.mControl == kCircleButton)
   {
      if (control.mValue > 0 && mDisplayModule != nullptr)
      {
         Checkbox* enabledCheckbox = mDisplayModule->GetEnabledCheckbox();
         if (enabledCheckbox != nullptr)
            enabledCheckbox->SetValue(mDisplayModule->IsEnabled() ? 0 : 1, gTime);
         else
            mDisplayModule->SetEnabled(!mDisplayModule->IsEnabled());
      }
   }
   else
   {
      ofLog() << "control " << control.mControl << " " << control.mValue;
   }
}

void Push2Control::OnMidiPitchBend(MidiPitchBend& pitchBend)
{
   if (mGridControlInterface != nullptr)
   {
      bool handled = mGridControlInterface->OnPush2Control(this, kMidiMessage_PitchBend, pitchBend.mChannel, pitchBend.mValue);
      if (handled)
         return;
   }

   float value = pitchBend.mValue / MidiDevice::kPitchBendMax;
   TheSynth->SetZoomLevel(pow(2, value * 2 - 1) + .1f);

   //ofLog() << "pitchbend " << pitchBend.mChannel << " " << pitchBend.mValue;
}

int Push2Control::GetGridControllerOption1Control() const
{
   return kRepeatButton;
}

int Push2Control::GetGridControllerOption2Control() const
{
   return kAccentButton;
}

void Push2Control::UpdateRoutingModules()
{
   mRoutingInputModules.clear();
   mRoutingOutputModules.clear();

   std::vector<IDrawableModule*> modules;
   TheSynth->GetAllModules(modules);
   for (auto* module : modules)
   {
      for (auto* source : module->GetPatchCableSources())
      {
         if (source->GetTarget() == mDisplayModule)
            mRoutingInputModules.push_back(Routing(module, module->GetPatchCableSource()->GetColor()));
      }
   }

   for (auto* source : mDisplayModule->GetPatchCableSources())
   {
      for (auto* cable : source->GetPatchCables())
      {
         IDrawableModule* target = dynamic_cast<IDrawableModule*>(cable->GetTarget());
         if (target != nullptr)
            mRoutingOutputModules.push_back(Routing(target, cable->GetOwner()->GetColor()));
      }
   }
}

bool Push2Control::IsIgnorableModule(IDrawableModule* module)
{
   return module == TheTitleBar || module == TheSaveDataPanel || module == TheQuickSpawnMenu || module == TheSynth->GetUserPrefsEditor() || module == TheQuickSpawnMenu->GetMainContainerFollower();
}

std::vector<IDrawableModule*> Push2Control::SortModules(std::vector<IDrawableModule*> modules)
{
   std::vector<IDrawableModule*> output;

   for (int i = 0; i < modules.size(); ++i)
      AddModuleChain(modules[i], modules, output, 0);

   return output;
}

void Push2Control::AddModuleChain(IDrawableModule* module, std::vector<IDrawableModule*>& modules, std::vector<IDrawableModule*>& output, int depth)
{
   if (depth > 100) //avoid infinite recursion if there's a patching loop
      return;

   if (!VectorContains(module, output))
   {
      //look for parents
      for (int i = 0; i < modules.size(); ++i)
      {
         IClickable* target = nullptr;
         if (modules[i]->GetPatchCableSource() != nullptr)
            target = modules[i]->GetPatchCableSource()->GetTarget();
         if (target != nullptr &&
             target == dynamic_cast<IClickable*>(module))
         {
            AddModuleChain(modules[i], modules, output, depth + 1);
         }
      }

      if (VectorContains(module, output)) //got added above
         return;

      output.push_back(module);

      //look for children
      for (int i = 0; i < modules.size(); ++i)
      {
         IClickable* target = nullptr;
         if (module->GetPatchCableSource() != nullptr)
            target = module->GetPatchCableSource()->GetTarget();
         if (target != nullptr &&
             target == dynamic_cast<IClickable*>(modules[i]))
         {
            AddModuleChain(modules[i], modules, output, depth + 1);
         }
      }
   }
}
