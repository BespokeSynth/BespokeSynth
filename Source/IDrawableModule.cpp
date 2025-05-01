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
//
//  IDrawableModule.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 11/26/12.
//
//

#include "IDrawableModule.h"
#include "RollingBuffer.h"
#include "IAudioSource.h"
#include "INoteSource.h"
#include "INoteReceiver.h"
#include "IAudioReceiver.h"
#include "IAudioEffect.h"
#include "IUIControl.h"
#include "Slider.h"
#include "ofxJSONElement.h"
#include "ModularSynth.h"
#include "SynthGlobals.h"
#include "TitleBar.h"
#include "ModuleSaveDataPanel.h"
#include "GridController.h"
#include "ControlSequencer.h"
#include "PatchCableSource.h"
#include "nanovg/nanovg.h"
#include "IPulseReceiver.h"
#include "Push2Control.h"
#include "UIGrid.h"
#include "UserPrefs.h"
#include "Prefab.h"

float IDrawableModule::sHueNote = 27;
float IDrawableModule::sHueAudio = 135;
float IDrawableModule::sHueInstrument = 79;
float IDrawableModule::sHueNoteSource = 240;
float IDrawableModule::sSaturation = 145;
float IDrawableModule::sBrightness = 220;

IDrawableModule::IDrawableModule()
{
}

IDrawableModule::~IDrawableModule()
{
   for (int i = 0; i < mUIControls.size(); ++i)
      mUIControls[i]->Delete();
   for (auto source : mPatchCableSources)
      delete source;
}

void IDrawableModule::CreateUIControls()
{
   assert(mUIControlsCreated == false);
   mUIControlsCreated = true;
   mEnabledCheckbox = new Checkbox(this, "enabled", 3, -TitleBarHeight() - 2, &mEnabled);

   ConnectionType type = kConnectionType_Special;
   if (dynamic_cast<IAudioSource*>(this))
      type = kConnectionType_Audio;
   else if (dynamic_cast<INoteSource*>(this))
      type = kConnectionType_Note;
   else if (dynamic_cast<IGridController*>(this))
      type = kConnectionType_Grid;
   else if (dynamic_cast<IPulseSource*>(this))
      type = kConnectionType_Pulse;

   if (type != kConnectionType_Special && !ShouldSuppressAutomaticOutputCable())
   {
      mPatchCableSources.push_back(new PatchCableSource(this, type));
   }

   GetMinimizedWidth(); //update cached width
}

void IDrawableModule::Init()
{
   assert(mUIControlsCreated); //did you not call CreateUIControls() for this module?
   assert(!mInitialized);
   mInitialized = true;

   ModuleFactory::ModuleInfo moduleInfo = TheSynth->GetModuleFactory()->GetModuleInfo(mTypeName);
   mModuleCategory = moduleInfo.mCategory;
   if (mModuleCategory == kModuleCategory_Unknown)
   {
      if (dynamic_cast<IAudioEffect*>(this))
         mModuleCategory = kModuleCategory_Processor;
   }

   mCanReceiveAudio = moduleInfo.mCanReceiveAudio;
   mCanReceiveNotes = moduleInfo.mCanReceiveNotes;
   mCanReceivePulses = moduleInfo.mCanReceivePulses;

   // if you hit these asserts, it means that, for example, your module's
   // AcceptsPulses() returns false, but it inherits IPulseReceiver.
   // the fix is to make AcceptsPulses() (or the appropriate method) match the
   // list of inherited classes.
   assert(mCanReceiveAudio == (dynamic_cast<IAudioReceiver*>(this) != nullptr));
   assert(mCanReceiveNotes == (dynamic_cast<INoteReceiver*>(this) != nullptr));
   assert(mCanReceivePulses == (dynamic_cast<IPulseReceiver*>(this) != nullptr));

   bool wasEnabled = IsEnabled();
   bool showEnableToggle = false;
   SetEnabled(!wasEnabled);
   if (IsEnabled() == wasEnabled) //nothing changed
      showEnableToggle = false; //so don't show toggle, this module doesn't support it
   else
      showEnableToggle = true;
   SetEnabled(wasEnabled);

   if (showEnableToggle)
   {
      mEnabledCheckbox->SetDisplayText(false);
      mEnabledCheckbox->UseCircleLook(GetColor(mModuleCategory));
   }
   else
   {
      RemoveUIControl(mEnabledCheckbox, false);
      mEnabledCheckbox->Delete();
      mEnabledCheckbox = nullptr;
   }

   for (int i = 0; i < mUIControls.size(); ++i)
      mUIControls[i]->Init();

   for (int i = 0; i < mChildren.size(); ++i)
   {
      ModuleContainer* container = GetContainer();
      if (container != nullptr && VectorContains(mChildren[i], container->GetModules()))
         continue; //stuff in module containers was already initialized
      mChildren[i]->Init();
   }

   mKeyboardFocusListener = dynamic_cast<IKeyboardFocusListener*>(this);
}

void IDrawableModule::BasePoll()
{
   Poll();
   for (int i = 0; i < mUIControls.size(); ++i)
      mUIControls[i]->Poll();
   for (int i = 0; i < mChildren.size(); ++i)
      mChildren[i]->BasePoll();
}

bool IDrawableModule::IsWithinRect(const ofRectangle& rect)
{
   float x, y;
   GetPosition(x, y);
   float w, h;
   GetDimensions(w, h);

   float titleBarHeight = mTitleBarHeight;
   if (!HasTitleBar())
      titleBarHeight = 0;

   return rect.intersects(ofRectangle(x, y - titleBarHeight, w, h + titleBarHeight));
}

bool IDrawableModule::IsVisible()
{
   return IsWithinRect(TheSynth->GetDrawRect());
}

void IDrawableModule::DrawFrame(float w, float h, bool drawModule, float& titleBarHeight, float& highlight)
{
   if (mPinned)
      ForcePosition();
   titleBarHeight = mTitleBarHeight;
   if (!HasTitleBar())
      titleBarHeight = 0;

   ofTranslate(mX, mY, 0);

   ofColor color = GetColor(mModuleCategory);

   highlight = 0;

   if (IsEnabled())
   {
      IAudioSource* audioSource = dynamic_cast<IAudioSource*>(this);
      if (audioSource)
      {
         RollingBuffer* vizBuff = audioSource->GetVizBuffer();
         int numSamples = std::min(500, vizBuff->Size());
         float sample;
         float mag = 0;
         for (int ch = 0; ch < vizBuff->NumChannels(); ++ch)
         {
            for (int i = 0; i < numSamples; ++i)
            {
               sample = vizBuff->GetSample(i, ch);
               mag += sample * sample;
            }
         }
         mag /= numSamples * vizBuff->NumChannels();
         mag = sqrtf(mag);
         mag = sqrtf(mag);
         mag *= 3;
         mag = ofClamp(mag, 0, 1);

         if (UserPrefs.draw_module_highlights.Get())
            highlight = mag * .15f;
      }

      if (GetPatchCableSource() != nullptr)
      {
         float elapsed = float(gTime - GetPatchCableSource()->GetHistory().GetLastOnEventTime()) / NOTE_HISTORY_LENGTH;
         if (UserPrefs.draw_module_highlights.Get())
            highlight = MAX(highlight, .15f * ofClamp(1 - elapsed, 0, 1));
      }
   }

   const bool kUseDropshadow = false;
   if (kUseDropshadow && GetParent() == nullptr && GetModuleCategory() != kModuleCategory_Other)
   {
      const float shadowSize = 20;
      float shadowStrength = .2f + highlight;
      NVGpaint shadowPaint = nvgBoxGradient(gNanoVG, -shadowSize / 2, -titleBarHeight - shadowSize / 2, w + shadowSize, h + titleBarHeight + shadowSize, gCornerRoundness * shadowSize * .5f, shadowSize, nvgRGBA(color.r * shadowStrength, color.g * shadowStrength, color.b * shadowStrength, 128), nvgRGBA(0, 0, 0, 0));
      nvgBeginPath(gNanoVG);
      nvgRect(gNanoVG, -shadowSize, -shadowSize - titleBarHeight, w + shadowSize * 2, h + titleBarHeight + shadowSize * 2);
      nvgFillPaint(gNanoVG, shadowPaint);
      nvgFill(gNanoVG);
   }

   ofFill();
   float backgroundAlpha = IsEnabled() ? 180 : 120;
   if (dynamic_cast<Prefab*>(this) != nullptr)
      backgroundAlpha = 60;
   if (IsEnabled())
      ofSetColor(color.r * (.25f + highlight), color.g * (.25f + highlight), color.b * (.25f + highlight), backgroundAlpha);
   else
      ofSetColor(color.r * .2f, color.g * .2f, color.b * .2f, backgroundAlpha);
   //gModuleShader.begin();
   const float kHighlightGrowAmount = 40;
   ofRect(0 - highlight * kHighlightGrowAmount, -titleBarHeight - highlight * kHighlightGrowAmount,
          w + highlight * kHighlightGrowAmount * 2, h + titleBarHeight + highlight * kHighlightGrowAmount * 2, 3 + highlight * kHighlightGrowAmount);
   //gModuleShader.end();
   ofNoFill();

   if (IsEnabled())
      gModuleDrawAlpha = 255;
   else
      gModuleDrawAlpha = 100;

   bool dimModule = TheSynth->ShouldDimModule(this);

   if (dimModule)
      gModuleDrawAlpha *= .2f;

   float enableToggleOffset = 0;
   if (HasTitleBar())
   {
      ofPushStyle();
      ofSetColor(color, 50);
      ofFill();
      ofPushMatrix();
      ofClipWindow(0, -titleBarHeight, w, titleBarHeight, true);
      ofRect(0, -titleBarHeight, w, titleBarHeight * 2);
      ofPopMatrix();

      if (mEnabledCheckbox)
      {
         enableToggleOffset = TitleBarHeight();
         mEnabledCheckbox->Draw();
      }

      if (mKeyboardFocusListener != nullptr && mKeyboardFocusListener->CanTakeFocus())
      {
         if ((gHoveredModule == this && IKeyboardFocusListener::GetActiveKeyboardFocus() == nullptr) ||
             IKeyboardFocusListener::GetActiveKeyboardFocus() == mKeyboardFocusListener)
            ofSetColor(255, 255, 255, gModuleDrawAlpha);
         else
            ofSetColor(color.r, color.g, color.b, gModuleDrawAlpha);
         float squareSize = titleBarHeight / 2 - 1;
         ofRect(w - 25, -titleBarHeight + 1, squareSize, squareSize, 1);
         ofRect(w - 25, -titleBarHeight / 2 + 1, squareSize, squareSize, 1);
         ofRect(w - 25 - squareSize - 1, -titleBarHeight / 2 + 1, squareSize, squareSize, 1);
         ofRect(w - 25 + squareSize + 1, -titleBarHeight / 2 + 1, squareSize, squareSize, 1);

         if (IKeyboardFocusListener::GetActiveKeyboardFocus() == mKeyboardFocusListener)
         {
            ofPushStyle();
            ofSetLineWidth(.5f);
            ofNoFill();
            ofRect(w - 25 - squareSize - 2 - 1, -titleBarHeight,
                   2 + squareSize + 1 + squareSize + 1 + squareSize + 2, titleBarHeight, 2);
            ofPopStyle();
         }
      }

      if (IsSaveable() && !Minimized())
      {
         ofSetColor(color.r, color.g, color.b, gModuleDrawAlpha);
         if (TheSaveDataPanel->GetModule() == this)
         {
            ofTriangle(w - 9, -titleBarHeight + 2,
                       w - 9, -2,
                       w - 2, -titleBarHeight / 2);
         }
         else
         {
            ofTriangle(w - 10, -titleBarHeight + 3,
                       w - 2, -titleBarHeight + 3,
                       w - 6, -2);
         }
      }
      ofPopStyle();
   }

   const bool kDrawInnerFade = true;
   if (kDrawInnerFade && !Push2Control::sDrawingPush2Display)
   {
      float fadeRoundness = 100;
      float fadeLength = w / 3;
      const float kFadeStrength = .9f;
      NVGpaint shadowPaint = nvgBoxGradient(gNanoVG, 0, -titleBarHeight, w, h + titleBarHeight, fadeRoundness, fadeLength, nvgRGBA(color.r * .2f, color.g * .2f, color.b * .2f, backgroundAlpha * kFadeStrength), nvgRGBA(0, 0, 0, 0));
      nvgBeginPath(gNanoVG);
      nvgRect(gNanoVG, 0, -titleBarHeight, w, h + titleBarHeight);
      nvgFillPaint(gNanoVG, shadowPaint);
      nvgFill(gNanoVG);
   }

   if (drawModule)
   {
      ofSetColor(color, gModuleDrawAlpha);
      ofPushMatrix();
      if (ShouldClipContents())
         ofClipWindow(0, 0, w, h, true);
      else
         ofResetClipWindow();
      DrawModule();
      ofPopMatrix();
   }

   if (HasTitleBar())
   {
      ofSetColor(color * (1 - GetBeaconAmount()) + ofColor::yellow * GetBeaconAmount(), gModuleDrawAlpha);
      DrawTextBold(GetTitleLabel(), 5 + enableToggleOffset, 10 - titleBarHeight, 14);
   }

   bool groupSelected = !TheSynth->GetGroupSelectedModules().empty() && VectorContains(this, TheSynth->GetGroupSelectedModules());
   if ((IsEnabled() || groupSelected || TheSynth->GetMoveModule() == this) && mShouldDrawOutline)
   {
      ofPushStyle();
      ofNoFill();

      if (groupSelected)
      {
         float pulse = ofMap(sin(gTime / 500 * PI * 2), -1, 1, .2f, 1);
         ofSetColor(ofLerp(color.r, 255, pulse), ofLerp(color.g, 255, pulse), ofLerp(color.b, 255, pulse), 255);
         ofSetLineWidth(1.5f);
      }
      else if (TheSynth->GetMoveModule() == this)
      {
         ofSetColor(255, 255, 255);
         ofSetLineWidth(.5f);
      }
      else
      {
         ofSetColor(color.r * (.5f + highlight), color.g * (.5f + highlight), color.b * (.5f + highlight), 200);
         ofSetLineWidth(.5f);
      }
      ofRect(-.5f, -titleBarHeight - .5f, w + 1, h + titleBarHeight + 1, 4);
      ofPopStyle();
   }

   const float kPinRadius = 2;
   if (mPinned)
   {
      ofFill();
      ofSetColor(color, 120);
      ofCircle(0, -titleBarHeight, kPinRadius);
      ofCircle(w, -titleBarHeight, kPinRadius);
      ofCircle(w, h, kPinRadius);
      ofCircle(0, h, kPinRadius);
      ofCircle(0, -titleBarHeight, kPinRadius / 2);
      ofCircle(w, -titleBarHeight, kPinRadius / 2);
      ofCircle(w, h, kPinRadius / 2);
      ofCircle(0, h, kPinRadius / 2);
   }
}

void IDrawableModule::Render()
{
   if (!mShowing)
      return;

   PreDrawModule();

   if (mMinimized)
      mMinimizeAnimation += ofGetLastFrameTime() * 5;
   else
      mMinimizeAnimation -= ofGetLastFrameTime() * 5;
   mMinimizeAnimation = ofClamp(mMinimizeAnimation, 0, 1);

   float w, h;
   GetDimensions(w, h);

   ofPushMatrix();
   ofPushStyle();

   float titleBarHeight;
   float highlight;
   DrawFrame(w, h, true, titleBarHeight, highlight);

   if (Minimized() || IsVisible() == false)
      DrawBeacon(30, -titleBarHeight / 2);

   ofPushStyle();
   const float kPipWidth = 8;
   const float kPipSpacing = 2;
   float receiveIndicatorX = w - (kPipWidth + kPipSpacing);
   ofFill();

   if (CanReceiveAudio())
   {
      ofSetColor(GetColor(kModuleCategory_Audio));
      ofRect(receiveIndicatorX, -titleBarHeight - 2, kPipWidth, 3, 1.0f);
      receiveIndicatorX -= kPipWidth + kPipSpacing;
   }
   if (CanReceiveNotes())
   {
      ofSetColor(GetColor(kModuleCategory_Note));
      ofRect(receiveIndicatorX, -titleBarHeight - 2, kPipWidth, 3, 1.0f);
      receiveIndicatorX -= kPipWidth + kPipSpacing;
   }
   if (CanReceivePulses())
   {
      ofSetColor(GetColor(kModuleCategory_Pulse));
      ofRect(receiveIndicatorX, -titleBarHeight - 2, kPipWidth, 3, 1.0f);
      receiveIndicatorX -= kPipWidth + kPipSpacing;
   }
   ofPopStyle();

   if (IsResizable() && !Minimized())
   {
      ofColor color = GetColor(mModuleCategory);
      ofSetColor(color, 255);
      if (mHoveringOverResizeHandle)
         ofSetLineWidth(4);
      else
         ofSetLineWidth(2);
      ofLine(w - sResizeCornerSize, h, w, h);
      ofLine(w, h - sResizeCornerSize, w, h);
   }

   gModuleDrawAlpha = 255;

   ofFill();

   if (TheSynth->ShouldAccentuateActiveModules() && GetParent() == nullptr)
   {
      ofSetColor(0, 0, 0, ofMap(highlight, 0, .1f, 200, 0, K(clamp)));
      ofRect(0, -titleBarHeight, w, h + titleBarHeight);
   }

   ofPopMatrix();
   ofPopStyle();

   if (!Push2Control::sDrawingPush2Display)
   {
      for (auto source : mPatchCableSources)
      {
         source->UpdatePosition(false);
         source->DrawSource();
      }
   }
}

void IDrawableModule::RenderUnclipped()
{
   if (!mShowing)
      return;

   ofPushMatrix();
   ofPushStyle();

   ofTranslate(mX, mY, 0);
   ofColor color = GetColor(mModuleCategory);
   ofSetColor(color);

   DrawModuleUnclipped();

   ofPopMatrix();
   ofPopStyle();
}

void IDrawableModule::DrawPatchCables(bool parentMinimized, bool inFront)
{
   if (mPinned)
      ForcePosition();
   for (auto source : mPatchCableSources)
   {
      ConnectionType type = source->GetConnectionType();
      bool isHeld = false;
      if (PatchCable::sActivePatchCable != nullptr)
         isHeld = (PatchCable::sActivePatchCable->GetOwner() == source);
      bool shouldDrawInFront = isHeld || (type != kConnectionType_Note && type != kConnectionType_Pulse && type != kConnectionType_Audio);
      if (type == kConnectionType_Pulse)
      {
         for (auto const cable : source->GetPatchCables())
         {
            if (cable->GetTarget() != nullptr &&
                dynamic_cast<IUIControl*>(cable->GetTarget()) != nullptr)
            {
               shouldDrawInFront = true;
               break;
            }
         }
      }
      if ((inFront && !shouldDrawInFront) || (!inFront && shouldDrawInFront))
         continue;

      source->UpdatePosition(parentMinimized);
      source->DrawCables(parentMinimized);
   }
}

//static
ofColor IDrawableModule::GetColor(ModuleCategory type)
{
   ofColor color;
   color.setHsb(0, 0, sBrightness);
   if (type == kModuleCategory_Note)
      color.setHsb(sHueNote, sSaturation, sBrightness);
   if (type == kModuleCategory_Synth)
      color.setHsb(sHueInstrument, sSaturation, sBrightness);
   if (type == kModuleCategory_Audio)
      color.setHsb(sHueAudio, sSaturation, sBrightness);
   if (type == kModuleCategory_Instrument)
      color.setHsb(sHueNoteSource, sSaturation, sBrightness);
   if (type == kModuleCategory_Processor)
      color.setHsb(170, 100, 255);
   if (type == kModuleCategory_Modulator)
      color.setHsb(200, 100, 255);
   if (type == kModuleCategory_Pulse)
      color.setHsb(43, sSaturation, sBrightness);
   return color;
}

void IDrawableModule::DrawConnection(IClickable* target)
{
   float lineWidth = 1;
   float plugWidth = 4;
   int lineAlpha = 100;

   IDrawableModule* targetModule = dynamic_cast<IDrawableModule*>(target);

   bool fatCable = false;
   if (TheSynth->GetLastClickedModule() == this ||
       TheSynth->GetLastClickedModule() == targetModule)
      fatCable = true;

   if (fatCable)
   {
      lineWidth = 3;
      plugWidth = 8;
      lineAlpha = 255;
   }

   ofPushMatrix();
   ofPushStyle();

   PatchCableOld cable = GetPatchCableOld(target);

   ofColor lineColor = GetColor(kModuleCategory_Other);
   lineColor.setBrightness(lineColor.getBrightness() * .8f);
   ofColor lineColorAlphaed = lineColor;
   lineColorAlphaed.a = lineAlpha;

   float wThis, hThis, xThis, yThis;
   GetDimensions(wThis, hThis);
   GetPosition(xThis, yThis);

   ofSetLineWidth(plugWidth);
   ofSetColor(lineColor);
   ofLine(cable.plug.x, cable.plug.y, cable.end.x, cable.end.y);

   ofSetLineWidth(lineWidth);
   ofSetColor(lineColorAlphaed);
   ofLine(cable.start.x, cable.start.y, cable.plug.x, cable.plug.y);

   ofPopStyle();
   ofPopMatrix();
}

void IDrawableModule::SetTarget(IClickable* target)
{
   if (!mPatchCableSources.empty())
      mPatchCableSources[0]->SetTarget(target);
}

void IDrawableModule::SetUpPatchCables(std::string targets)
{
   PatchCableSource* source = GetPatchCableSource();
   assert(source != nullptr);
   std::vector<std::string> targetVec = ofSplitString(targets, ",");
   if (targetVec.empty() || targets == "")
   {
      source->Clear();
   }
   else
   {
      for (int i = 0; i < targetVec.size(); ++i)
      {
         IClickable* target = dynamic_cast<IClickable*>(TheSynth->FindModule(targetVec[i]));
         if (target)
            source->AddPatchCable(target);
      }
   }
}

void IDrawableModule::AddPatchCableSource(PatchCableSource* source)
{
   mPatchCableSources.push_back(source);
}

void IDrawableModule::RemovePatchCableSource(PatchCableSource* source)
{
   RemoveFromVector(source, mPatchCableSources);
   delete source;
}

void IDrawableModule::Exit()
{
   for (auto source : mPatchCableSources)
   {
      source->Clear();
   }
}

bool IDrawableModule::TestClick(float x, float y, bool right, bool testOnly /*=false*/)
{
   if (IsResizable() && mHoveringOverResizeHandle)
   {
      if (!testOnly)
         TheSynth->SetResizeModule(this);
      return true;
   }

   for (auto source : mPatchCableSources)
   {
      if (source->TestClick(x, y, right, testOnly))
         return true;
   }

   if (IClickable::TestClick(x, y, right, testOnly))
      return true;

   return false;
}

void IDrawableModule::OnClicked(float x, float y, bool right)
{
   float w, h;
   GetModuleDimensions(w, h);

   if (IsResizable() && mHoveringOverResizeHandle)
   {
      TheSynth->SetResizeModule(this);
      return;
   }

   if (y < 0)
   {
      if (mEnabledCheckbox && x < 20)
      {
         //"enabled" area
      }
      else if (x < GetMinimizedWidth() && CanMinimize())
      {
         mWasMinimizeAreaClicked = true;
         return;
      }
      else if (!Minimized() && IsSaveable())
      {
         if (x > w - 10)
         {
            if (TheSaveDataPanel->GetModule() == this)
               TheSaveDataPanel->SetModule(nullptr);
            else
               TheSaveDataPanel->SetModule(this);
         }
         else if (x > w - 30 && mKeyboardFocusListener != nullptr && mKeyboardFocusListener->CanTakeFocus())
         {
            IKeyboardFocusListener::SetActiveKeyboardFocus(mKeyboardFocusListener);
         }
      }
   }

   for (int i = 0; i < mUIControls.size(); ++i)
   {
      if (mUIControls[i]->TestClick(x, y, right))
         break;
   }
   for (int i = 0; i < mChildren.size(); ++i)
   {
      if (mChildren[i]->TestClick(x, y, right))
         break;
   }

   for (auto* seq : ControlSequencer::sControlSequencers)
   {
      for (int i = 0; i < mUIControls.size(); ++i)
      {
         if (mUIControls[i] == seq->GetUIControl())
            TheSynth->MoveToFront(seq);
      }
   }
   TheSynth->MoveToFront(this);
}

bool IDrawableModule::MouseMoved(float x, float y)
{
   float w, h;
   GetModuleDimensions(w, h);
   if (mShowing && !Minimized() && IsResizable() && x > w - sResizeCornerSize && y > h - sResizeCornerSize && x <= w && y <= h)
      mHoveringOverResizeHandle = true;
   else
      mHoveringOverResizeHandle = false;

   if (!mShowing)
      return false;
   for (auto source : mPatchCableSources)
      source->NotifyMouseMoved(x, y);
   if (Minimized())
      return false;
   for (int i = 0; i < mUIControls.size(); ++i)
      mUIControls[i]->NotifyMouseMoved(x, y);
   for (int i = 0; i < mChildren.size(); ++i)
      mChildren[i]->NotifyMouseMoved(x, y);

   return false;
}

void IDrawableModule::MouseReleased()
{
   mWasMinimizeAreaClicked = false;
   for (int i = 0; i < mUIControls.size(); ++i)
      mUIControls[i]->MouseReleased();
   for (int i = 0; i < mChildren.size(); ++i)
      mChildren[i]->MouseReleased();
   auto sources = mPatchCableSources;
   for (auto source : sources)
      source->MouseReleased();
}

IUIControl* IDrawableModule::FindUIControl(const char* name, bool fail /*=true*/) const
{
   if (name != 0)
   {
      for (int i = 0; i < mUIControls.size(); ++i)
      {
         if (strcmp(mUIControls[i]->Name(), name) == 0)
            return mUIControls[i];
      }
      for (int i = 0; i < mUIGrids.size(); ++i)
      {
         if (strcmp(mUIGrids[i]->Name(), name) == 0)
            return mUIGrids[i];
      }
   }
   if (fail)
      throw UnknownUIControlException();
   return nullptr;
}

IDrawableModule* IDrawableModule::FindChild(const std::string name, bool fail) const
{
   if (name.empty())
      return nullptr;
   for (int i = 0; i < mChildren.size(); ++i)
   {
      if (strcmp(mChildren[i]->Name(), name.c_str()) == 0)
         return mChildren[i];
   }
   if (mTypeName == "effectchain") // Due to an issue in the past where child modules of the effectchain module weren't saving their names correctly we are going to try and fix the loading here.
   {
      auto child = FindChild(name.substr(0, name.length() - 1), false);
      if (child)
         return child;
   }
   if (fail)
      throw UnknownModuleException(name);
   return nullptr;
}

void IDrawableModule::AddChild(IDrawableModule* child)
{
   if (dynamic_cast<IDrawableModule*>(child->GetParent()))
      dynamic_cast<IDrawableModule*>(child->GetParent())->RemoveChild(child);
   child->SetParent(this);
   if (child->Name()[0] == 0)
      child->SetName(("child" + ofToString(mChildren.size())).c_str());

   for (int i = 0; i < mChildren.size(); ++i)
   {
      if (strcmp(mChildren[i]->Name(), child->Name()) == 0)
      {
         child->SetName(GetUniqueName(child->Name(), mChildren).c_str());
         break;
      }
   }

   mChildren.push_back(child);
}

void IDrawableModule::RemoveChild(IDrawableModule* child)
{
   child->SetParent(nullptr);
   RemoveFromVector(child, mChildren);
}

std::vector<IUIControl*> IDrawableModule::GetUIControls() const
{
   std::vector<IUIControl*> controls = mUIControls;
   for (int i = 0; i < mChildren.size(); ++i)
   {
      std::vector<IUIControl*> childControls = mChildren[i]->GetUIControls();
      controls.insert(controls.end(), childControls.begin(), childControls.end());
   }
   return controls;
}

std::vector<UIGrid*> IDrawableModule::GetUIGrids() const
{
   std::vector<UIGrid*> grids = mUIGrids;
   for (int i = 0; i < mChildren.size(); ++i)
   {
      std::vector<UIGrid*> childGrids = mChildren[i]->GetUIGrids();
      grids.insert(grids.end(), childGrids.begin(), childGrids.end());
   }
   return grids;
}

void IDrawableModule::GetDimensions(float& width, float& height)
{
   int minimizedWidth = GetMinimizedWidth();
   int minimizedHeight = 0;

   float moduleWidth, moduleHeight;
   GetModuleDimensions(moduleWidth, moduleHeight);
   if (moduleWidth == 1 && moduleHeight == 1)
   {
      width = 1;
      height = 1;
      return; //special case for repatch stub, let it be 1 pixel
   }
   ofVec2f minimumDimensions = GetMinimumDimensions();
   moduleWidth = MAX(moduleWidth, minimumDimensions.x);
   moduleHeight = MAX(moduleHeight, minimumDimensions.y);

   width = EaseIn(moduleWidth, minimizedWidth, mMinimizeAnimation);
   height = EaseOut(moduleHeight, minimizedHeight, mMinimizeAnimation);
}

float IDrawableModule::GetMinimizedWidth()
{
   std::string titleLabel = GetTitleLabel();
   if (titleLabel != mLastTitleLabel)
   {
      mLastTitleLabel = titleLabel;
      mTitleLabelWidth = gFont.GetStringWidth(GetTitleLabel(), 14);
   }
   float width = mTitleLabelWidth;
   width += 10; //padding
   if (mEnabledCheckbox)
      width += TitleBarHeight();
   return MAX(width, 50);
}

ofVec2f IDrawableModule::GetMinimumDimensions()
{
   return ofVec2f(GetMinimizedWidth() + 10, 10);
}

void IDrawableModule::KeyPressed(int key, bool isRepeat)
{
   for (auto source : mPatchCableSources)
      source->KeyPressed(key, isRepeat);
   for (auto control : mUIControls)
      control->KeyPressed(key, isRepeat);
}

void IDrawableModule::KeyReleased(int key)
{
}

void IDrawableModule::AddUIControl(IUIControl* control)
{
   try
   {
      std::string name = control->Name();
      if (CanModuleTypeSaveState() && name.empty() == false)
      {
         IUIControl* dupe = FindUIControl(name.c_str(), false);
         if (dupe != nullptr)
         {
            if (dynamic_cast<ClickButton*>(control) != nullptr && dynamic_cast<ClickButton*>(dupe) != nullptr)
            {
               //they're both just buttons, this is fine
            }
            else if (!dupe->GetShouldSaveState())
            {
               //we're duplicating the name of a non-saving ui control, assume that we are also not saving (hopefully that's true), and this is fine
            }
            else
            {
               assert(false); //can't have multiple ui controls with the same name!
            }
         }
      }
   }
   catch (UnknownUIControlException& e)
   {
   }

   mUIControls.push_back(control);
   FloatSlider* slider = dynamic_cast<FloatSlider*>(control);
   if (slider)
   {
      mSliderMutex.lock();
      mFloatSliders.push_back(slider);
      mSliderMutex.unlock();
   }
}

void IDrawableModule::RemoveUIControl(IUIControl* control, bool cleanUpReferences /* = true */)
{
   if (cleanUpReferences)
      IUIControl::DestroyCablesTargetingControls(std::vector<IUIControl*>{ control });

   RemoveFromVector(control, mUIControls, K(fail));
   FloatSlider* slider = dynamic_cast<FloatSlider*>(control);
   if (slider)
   {
      mSliderMutex.lock();
      RemoveFromVector(slider, mFloatSliders, K(fail));
      mSliderMutex.unlock();
   }
}

void IDrawableModule::AddUIGrid(UIGrid* grid)
{
   mUIGrids.push_back(grid);
}

void IDrawableModule::ComputeSliders(int samplesIn)
{
   //mSliderMutex.lock(); TODO(Ryan) mutex acquisition is slow, how can I do this faster?
   for (int i = 0; i < mFloatSliders.size(); ++i)
      mFloatSliders[i]->Compute(samplesIn);
   //mSliderMutex.unlock();
}

PatchCableOld IDrawableModule::GetPatchCableOld(IClickable* target)
{
   float wThis, hThis, xThis, yThis, wThat, hThat, xThat, yThat;
   GetDimensions(wThis, hThis);
   GetPosition(xThis, yThis);
   if (target)
   {
      target->GetDimensions(wThat, hThat);
      target->GetPosition(xThat, yThat);
   }
   else
   {
      wThat = 0;
      hThat = 0;
      xThat = xThis + wThis / 2;
      yThat = yThis + hThis + 20;
   }

   ofTranslate(-xThis, -yThis);

   if (HasTitleBar())
   {
      yThis -= mTitleBarHeight;
      hThis += mTitleBarHeight;
   }
   IDrawableModule* targetModule = dynamic_cast<IDrawableModule*>(target);
   if (targetModule && targetModule->HasTitleBar())
   {
      yThat -= mTitleBarHeight;
      hThat += mTitleBarHeight;
   }

   float startX, startY, endX, endY;
   FindClosestSides(xThis, yThis, wThis, hThis, xThat, yThat, wThat, hThat, startX, startY, endX, endY);

   float diffX = endX - startX;
   float diffY = endY - startY;
   float length = sqrtf(diffX * diffX + diffY * diffY);
   float endCap = MIN(.5f, 20 / length);
   int plugX = int((startX - endX) * endCap) + endX;
   int plugY = int((startY - endY) * endCap) + endY;

   PatchCableOld cable;
   cable.start.x = startX;
   cable.start.y = startY;
   cable.end.x = endX;
   cable.end.y = endY;
   cable.plug.x = plugX;
   cable.plug.y = plugY;

   return cable;
}

void IDrawableModule::ForcePosition()
{
   if (TheSynth->GetMoveModule() != this)
   {
      auto pos = mPinnedPosition - TheSynth->GetDrawOffset();
      mX = pos.x;
      mY = pos.y;
   }
   else if (mPinned) // Moved while pinned.
      SetPinned(true);
}

void IDrawableModule::FindClosestSides(float xThis, float yThis, float wThis, float hThis, float xThat, float yThat, float wThat, float hThat, float& startX, float& startY, float& endX, float& endY, bool sidesOnly /*= false*/)
{
   ofVec2f vDirs[4];
   vDirs[0].set(-1, 0);
   vDirs[1].set(1, 0);
   vDirs[2].set(0, -1);
   vDirs[3].set(0, 1);

   ofVec2f vThis[4];
   vThis[0].set(xThis, yThis + hThis / 2); //left
   vThis[1].set(xThis + wThis, yThis + hThis / 2); //right
   vThis[2].set(xThis + wThis / 2, yThis); //top
   vThis[3].set(xThis + wThis / 2, yThis + hThis); //bottom
   ofVec2f vThat[4];
   vThat[0].set(xThat, yThat + hThat / 2);
   vThat[1].set(xThat + wThat, yThat + hThat / 2);
   vThat[2].set(xThat + wThat / 2, yThat);
   vThat[3].set(xThat + wThat / 2, yThat + hThat);

   float closest = FLT_MAX;
   int closestPair = 0;
   for (int i = 0; i < 4; ++i)
   {
      if (i == 2) //skip top
         continue;

      if (sidesOnly && i == 3)
         continue; //skip bottom

      for (int j = 0; j < 4; ++j)
      {
         ofVec2f vDiff = vThat[j] - vThis[i];
         float distSq = vDiff.lengthSquared();
         if (distSq < closest &&
             //i/2 == j/2 &&   //only connect horizontal-horizontal, vertical-vertical
             //((i/2 == 0 && fabs(vDiff.x) > 10) ||
             // (i/2 == 1 && fabs(vDiff.y) > 10)
             //) &&
             vDiff.dot(vDirs[i]) > 0 &&
             vDiff.dot(vDirs[j]) < 0)
         {
            closest = distSq;
            closestPair = i + j * 4;
         }
      }
   }

   startX = vThis[closestPair % 4].x;
   startY = vThis[closestPair % 4].y;
   endX = vThat[closestPair / 4].x;
   endY = vThat[closestPair / 4].y;
   if (endY > vThis[3].y && !sidesOnly)
   {
      startX = vThis[3].x;
      startY = vThis[3].y;
   }
}

void IDrawableModule::ToggleMinimized()
{
   mMinimized = !mMinimized;
   if (mMinimized)
   {
      if (TheSaveDataPanel->GetModule() == this)
         TheSaveDataPanel->SetModule(nullptr);
   }
}

void IDrawableModule::TogglePinned()
{
   SetPinned(!mPinned);
}

void IDrawableModule::SetPinned(bool pinned)
{
   if (!HasTitleBar())
      return;
   mPinned = pinned;
   if (mPinned)
      mPinnedPosition = GetPosition() + TheSynth->GetDrawOffset();
}

bool IDrawableModule::CheckNeedsDraw()
{
   if (IClickable::CheckNeedsDraw())
      return true;

   for (int i = 0; i < mUIControls.size(); ++i)
   {
      if (mUIControls[i]->CheckNeedsDraw())
         return true;
   }

   for (int i = 0; i < mChildren.size(); ++i)
   {
      if (mChildren[i]->CheckNeedsDraw())
         return true;
   }

   return false;
}

void IDrawableModule::AddDebugLine(std::string text, int maxLines)
{
   std::vector<std::string> lines = ofSplitString(mDebugDisplayText, "\n");
   mDebugDisplayText = "";
   for (int i = 0; i < maxLines - 1; ++i)
   {
      int lineIndex = (int)lines.size() - (maxLines - 1) + i;
      if (lineIndex >= 0)
         mDebugDisplayText += lines[lineIndex] + "\n";
   }
   mDebugDisplayText += text;
   ofLog() << text;
}

void IDrawableModule::LoadBasics(const ofxJSONElement& moduleInfo, std::string typeName)
{
   int x = 0;
   int y = 0;
   std::string name = "error";
   bool start_minimized = false;
   bool draw_lissajous = false;

   try
   {
      x = moduleInfo["position"][0u].asInt();
      y = moduleInfo["position"][1u].asInt();
      name = moduleInfo["name"].asString();
      start_minimized = moduleInfo["start_minimized"].asBool();
      draw_lissajous = moduleInfo["draw_lissajous"].asBool();
   }
   catch (Json::LogicError& e)
   {
      TheSynth->LogEvent(__PRETTY_FUNCTION__ + std::string(" json error: ") + e.what(), kLogEventType_Error);
   }


   SetPosition(x, y);

   SetName(name.c_str());

   SetMinimized(start_minimized, false);

   if (draw_lissajous)
      TheSynth->AddLissajousDrawer(this);

   mTypeName = typeName;
}

void IDrawableModule::LoadLayoutBase(const ofxJSONElement& moduleInfo)
{
   LoadLayout(moduleInfo);

   ITimeListener* timeListener = dynamic_cast<ITimeListener*>(this);
   if (timeListener)
   {
      mModuleSaveData.LoadInt("transport_priority", moduleInfo, timeListener->mTransportPriority, -9999, 9999, K(isTextField));
      timeListener->mTransportPriority = mModuleSaveData.GetInt("transport_priority");
   }
}

void IDrawableModule::SaveLayoutBase(ofxJSONElement& moduleInfo)
{
   moduleInfo["position"][0u] = mX;
   moduleInfo["position"][1u] = mY;
   moduleInfo["name"] = Name();
   moduleInfo["type"] = mTypeName;
   if (mMinimized)
      moduleInfo["start_minimized"] = true;
   if (TheSynth->IsLissajousDrawer(this))
      moduleInfo["draw_lissajous"] = true;
   mModuleSaveData.Save(moduleInfo);

   SaveLayout(moduleInfo);
}

void IDrawableModule::SetUpFromSaveDataBase()
{
   ITimeListener* timeListener = dynamic_cast<ITimeListener*>(this);
   if (timeListener)
      timeListener->mTransportPriority = mModuleSaveData.GetInt("transport_priority");

   SetUpFromSaveData();
}

namespace
{
   const int kBaseSaveStateRev = 3;
   const int kControlSeparatorLength = 16;
   const char kControlSeparator[kControlSeparatorLength + 1] = "controlseparator";
}

void IDrawableModule::SaveState(FileStreamOut& out)
{
   if (!CanModuleTypeSaveState())
      return;

   out << GetModuleSaveStateRev();

   out << kBaseSaveStateRev;

   out << mPinned;
   out << mPinnedPosition.x;
   out << mPinnedPosition.y;

   std::vector<IUIControl*> controlsToSave;
   for (auto* control : mUIControls)
   {
      if (!VectorContains(control, ControlsToIgnoreInSaveState()) && control->GetShouldSaveState())
         controlsToSave.push_back(control);
   }

   out << (int)controlsToSave.size();
   for (auto* control : controlsToSave)
   {
      //ofLog() << "Saving control " << control->Name();
      out << std::string(control->Name());
      out << control->GetValue(); //save raw value to make it easier to port old values when we change versions
      control->SaveState(out);
      for (int i = 0; i < kControlSeparatorLength; ++i)
         out << kControlSeparator[i];
   }

   if (GetContainer())
   {
      GetContainer()->SaveState(out);
   }
   else
   {
      out << (int)mChildren.size();

      for (auto* child : mChildren)
      {
         out << std::string(child->Name());
         child->SaveState(out);
      }
   }

   if (ShouldSavePatchCableSources())
   {
      out << (int)mPatchCableSources.size();
      for (auto* cable : mPatchCableSources)
         cable->SaveState(out);
   }
   else
   {
      out << 0; //no patch cable sources
   }
}

int IDrawableModule::LoadModuleSaveStateRev(FileStreamIn& in)
{
   int rev = -1;

   if (CanModuleTypeSaveState() && ModularSynth::sLoadingFileSaveStateRev >= 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   return rev;
}

void IDrawableModule::LoadState(FileStreamIn& in, int rev)
{
   if (!CanModuleTypeSaveState())
      return;

   if (rev != -1 && ModularSynth::sLoadingFileSaveStateRev >= 423)
   {
      int moduleRev;
      in >> moduleRev;
      LoadStateValidate(moduleRev == rev);
   }

   int baseRev;
   in >> baseRev;

   if (baseRev > 2)
   {
      in >> mPinned;
      in >> mPinnedPosition.x;
      in >> mPinnedPosition.y;
   }

   int numUIControls;
   in >> numUIControls;
   for (int i = 0; i < numUIControls; ++i)
   {
      std::string uicontrolname;
      in >> uicontrolname;
      if (baseRev >= 2)
      {
         float rawValue;
         in >> rawValue; //we don't use this here, but it'll likely be useful in the future if an option is renamed/removed and we need to port the old data
      }
      UpdateOldControlName(uicontrolname);

      bool threwException = false;
      try
      {
         if (LoadOldControl(in, uicontrolname))
         {
            //old data loaded, we're good now!
         }
         else
         {
            //ofLog() << "loading control " << uicontrolname;
            auto* control = FindUIControl(uicontrolname.c_str(), false);

            if (control == nullptr)
               throw UnknownUIControlException();

            bool setValue = true;
            if (VectorContains(control, ControlsToNotSetDuringLoadState()))
               setValue = false;
            control->LoadState(in, setValue);
         }

         for (int j = 0; j < kControlSeparatorLength; ++j)
         {
            char separatorChar;
            in >> separatorChar;
            if (separatorChar != kControlSeparator[j])
            {
               ofLog() << "Error loading state for " << uicontrolname;
               //something went wrong, let's print some info to try to figure it out
               ofLog() << "Read char " + ofToString(separatorChar) + " but expected " + kControlSeparator[j] + "!";
               ofLog() << "Save state file position is " + ofToString(in.GetFilePosition()) + ", EoF is " + (in.Eof() ? "true" : "false");
               std::string nextFewChars = "Next 10 characters are:";
               for (int c = 0; c < 10; ++c)
               {
                  char ch;
                  in >> ch;
                  nextFewChars += ofToString(ch);
               }
               ofLog() << nextFewChars;
            }
            assert(separatorChar == kControlSeparator[j]);
         }
      }
      catch (UnknownUIControlException& e)
      {
         threwException = true;
      }
      catch (LoadStateException& e)
      {
         threwException = true;
      }

      if (threwException)
      {
         TheSynth->LogEvent("Error in module \"" + std::string(Name()) + "\" loading state for control \"" + uicontrolname + "\"", kLogEventType_Error);

         //read through the rest of the module until we find the spacer, so we can continue loading the next module
         int separatorProgress = 0;
         while (!in.Eof())
         {
            char val;
            in >> val;
            if (val == kControlSeparator[separatorProgress])
               ++separatorProgress;
            else
               separatorProgress = 0;
            if (separatorProgress == kControlSeparatorLength)
               break; //we did it!
         }
      }
   }

   if (GetContainer())
      GetContainer()->LoadState(in);

   if (!GetContainer() || ModularSynth::sLoadingFileSaveStateRev < 425)
   {
      int numChildren;
      in >> numChildren;
      LoadStateValidate(numChildren <= mChildren.size());

      for (int i = 0; i < numChildren; ++i)
      {
         std::string childName;
         in >> childName;
         //ofLog() << "Loading " << childName;
         IDrawableModule* child = FindChild(childName, true);
         LoadStateValidate(child);
         child->LoadState(in, child->LoadModuleSaveStateRev(in));
      }
   }

   if (baseRev >= 1)
   {
      int numPatchCableSources;
      in >> numPatchCableSources;
      PatchCableSource* dummy = nullptr;
      for (int i = 0; i < numPatchCableSources; ++i)
      {
         PatchCableSource* readIn;
         if (i < mPatchCableSources.size())
         {
            readIn = mPatchCableSources[i];
         }
         else
         {
            if (dummy == nullptr)
               dummy = new PatchCableSource(this, kConnectionType_Special);
            readIn = dummy;
         }
         readIn->LoadState(in);
      }
   }
}

std::vector<IUIControl*> IDrawableModule::ControlsToNotSetDuringLoadState() const
{
   return std::vector<IUIControl*>(); //empty
}

std::vector<IUIControl*> IDrawableModule::ControlsToIgnoreInSaveState() const
{
   return std::vector<IUIControl*>(); //empty
}

bool IDrawableModule::IsSpawningOnTheFly(const ofxJSONElement& moduleInfo)
{
   if (moduleInfo["onthefly"].isNull())
      return false;
   return moduleInfo["onthefly"].asBool();
}
