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
#include "IAudioSource.h"
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
#include "Presets.h"
#include "PatchCableSource.h"
#include "nanovg/nanovg.h"
#include "IPulseReceiver.h"
#include "Push2Control.h"

float IDrawableModule::sHueNote = 27;
float IDrawableModule::sHueAudio = 135;
float IDrawableModule::sHueInstrument = 79;
float IDrawableModule::sHueNoteSource = 240;
float IDrawableModule::sSaturation = 145;
float IDrawableModule::sBrightness = 220;

IDrawableModule::IDrawableModule()
: mModuleType(kModuleType_Unknown)
, mMinimized(false)
, mWasMinimizeAreaClicked(false)
, mMinimizeAnimation(0)
, mEnabled(true)
, mEnabledCheckbox(nullptr)
, mUIControlsCreated(false)
, mInitialized(false)
, mMainPatchCableSource(nullptr)
, mOwningContainer(nullptr)
, mTitleLabelWidth(0)
, mShouldDrawOutline(true)
, mHoveringOverResizeHandle(false)
, mDeleted(false)
, mCanReceiveAudio(false)
, mCanReceiveNotes(false)
, mCanReceivePulses(false)
, mDrawDebug(false)
{
}

IDrawableModule::~IDrawableModule()
{
   for (int i=0; i<mUIControls.size(); ++i)
      mUIControls[i]->Delete();
   for (auto source : mPatchCableSources)
      delete source;
}

void IDrawableModule::CreateUIControls()
{
   assert(mUIControlsCreated == false);
   mUIControlsCreated = true;
   mEnabledCheckbox = new Checkbox(this,"enabled",3,-TitleBarHeight()-2,&mEnabled);
   
   ConnectionType type = kConnectionType_Special;
   if (dynamic_cast<IAudioSource*>(this))
      type = kConnectionType_Audio;
   else if (dynamic_cast<INoteSource*>(this))
      type = kConnectionType_Note;
   else if (dynamic_cast<IGridController*>(this))
      type = kConnectionType_Grid;
   else if (dynamic_cast<IPulseSource*>(this))
      type = kConnectionType_Pulse;
   if (type != kConnectionType_Special)
   {
      mMainPatchCableSource = new PatchCableSource(this, type);
      mPatchCableSources.push_back(mMainPatchCableSource);
   }
   
   GetMinimizedWidth(); //update cached width
}

void IDrawableModule::Init()
{
   assert(mUIControlsCreated);  //did you not call CreateUIControls() for this module?
   assert(!mInitialized);
   mInitialized = true;
   
   mModuleType = TheSynth->GetModuleFactory()->GetModuleType(mTypeName);
   if (mModuleType == kModuleType_Other)
   {
      if (dynamic_cast<IAudioEffect*>(this))
         mModuleType = kModuleType_Processor;
   }

   mCanReceiveAudio = (dynamic_cast<IAudioReceiver*>(this) != nullptr);
   mCanReceiveNotes = (dynamic_cast<INoteReceiver*>(this) != nullptr);
   mCanReceivePulses = (dynamic_cast<IPulseReceiver*>(this) != nullptr);
   
   bool wasEnabled = Enabled();
   bool showEnableToggle = false;
   SetEnabled(!wasEnabled);
   if (Enabled() == wasEnabled)  //nothing changed
      showEnableToggle = false; //so don't show toggle, this module doesn't support it
   else
      showEnableToggle = true;
   SetEnabled(wasEnabled);
   
   if (showEnableToggle)
   {
      mEnabledCheckbox->SetDisplayText(false);
      mEnabledCheckbox->UseCircleLook(GetColor(mModuleType));
   }
   else
   {
      RemoveUIControl(mEnabledCheckbox);
      mEnabledCheckbox->Delete();
      mEnabledCheckbox = nullptr;
   }
   
   for (int i=0; i<mUIControls.size(); ++i)
      mUIControls[i]->Init();
   
   for (int i=0; i<mChildren.size(); ++i)
   {
      ModuleContainer* container = GetContainer();
      if (container != nullptr && VectorContains(mChildren[i], container->GetModules()))
         continue;   //stuff in module containers was already initialized
      mChildren[i]->Init();
   }
}

void IDrawableModule::BasePoll()
{
   Poll();
   for (int i=0; i<mUIControls.size(); ++i)
      mUIControls[i]->Poll();
   for (int i=0; i<mChildren.size(); ++i)
      mChildren[i]->BasePoll();
}

bool IDrawableModule::IsWithinRect(const ofRectangle& rect)
{
   float x,y;
   GetPosition(x, y);
   float w, h;
   GetDimensions(w,h);
   
   float titleBarHeight = mTitleBarHeight;
   if (!HasTitleBar())
      titleBarHeight = 0;
   
   return rect.intersects(ofRectangle(x,y-titleBarHeight,w,h+titleBarHeight));
}

bool IDrawableModule::IsVisible()
{
   return IsWithinRect(TheSynth->GetDrawRect());
}

void IDrawableModule::DrawFrame(float w, float h, bool drawModule, float& titleBarHeight, float& highlight)
{
   titleBarHeight = mTitleBarHeight;
   if (!HasTitleBar())
      titleBarHeight = 0;
   
   ofTranslate(mX, mY, 0);

   ofColor color = GetColor(mModuleType);
   
   highlight = 0;
   
   if (Enabled())
   {
      IAudioSource* audioSource = dynamic_cast<IAudioSource*>(this);
      if (audioSource)
      {
         RollingBuffer* vizBuff = audioSource->GetVizBuffer();
         int numSamples = std::min(500,vizBuff->Size());
         float sample;
         float mag = 0;
         for (int ch=0; ch<vizBuff->NumChannels(); ++ch)
         {
            for (int i=0; i<numSamples; ++i)
            {
               sample = vizBuff->GetSample(i, ch);
               mag += sample*sample;
            }
         }
         mag /= numSamples * vizBuff->NumChannels();
         mag = sqrtf(mag);
         mag = sqrtf(mag);
         mag *= 3;
         mag = ofClamp(mag,0,1);
         
         highlight = mag*.15f;
      }
      
      if (GetPatchCableSource() != nullptr)
      {
         float elapsed = float(gTime - GetPatchCableSource()->GetHistory().GetLastOnEventTime()) / NOTE_HISTORY_LENGTH;
         highlight = MAX(highlight, .15f * ofClamp(1 - elapsed, 0, 1));
      }
   }
   
   const bool kUseDropshadow = false;
   if (kUseDropshadow && GetParent() == nullptr && GetModuleType() != kModuleType_Other)
   {
      const float shadowSize = 20;
      float shadowStrength = .2f + highlight;
      NVGpaint shadowPaint = nvgBoxGradient(gNanoVG, -shadowSize/2,-titleBarHeight-shadowSize/2, w+shadowSize,h+titleBarHeight+shadowSize, gCornerRoundness*shadowSize*.5f, shadowSize, nvgRGBA(color.r*shadowStrength,color.g*shadowStrength,color.b*shadowStrength,128), nvgRGBA(0,0,0,0));
      nvgBeginPath(gNanoVG);
      nvgRect(gNanoVG, -shadowSize,-shadowSize-titleBarHeight, w+shadowSize*2,h+titleBarHeight+shadowSize*2);
      nvgFillPaint(gNanoVG, shadowPaint);
      nvgFill(gNanoVG);
   }
   
   ofFill();
   if (Enabled())
      ofSetColor(color.r*(.25f+highlight),color.g*(.25f+highlight),color.b*(.25f+highlight),210);
   else
      ofSetColor(color.r*.2f,color.g*.2f,color.b*.2f,120);
   //gModuleShader.begin();
   const float kHighlightGrowAmount = 40;
   ofRect(0 - highlight * kHighlightGrowAmount, -titleBarHeight - highlight * kHighlightGrowAmount,
          w + highlight * kHighlightGrowAmount * 2, h + titleBarHeight + highlight * kHighlightGrowAmount * 2, 3 + highlight * kHighlightGrowAmount);
   //gModuleShader.end();
   ofNoFill();

   if (Enabled())
      gModuleDrawAlpha = 255;
   else
      gModuleDrawAlpha = 100;
   
   bool dimModule = false;
   
   if (TheSynth->GetGroupSelectedModules().empty() == false)
   {
      if (!VectorContains(GetModuleParent(), TheSynth->GetGroupSelectedModules()))
         dimModule = true;
   }
   
   if (PatchCable::sActivePatchCable &&
       (PatchCable::sActivePatchCable->GetConnectionType() != kConnectionType_Modulator || PatchCable::sActivePatchCable->GetConnectionType() != kConnectionType_UIControl) &&
       !PatchCable::sActivePatchCable->IsValidTarget(this))
   {
      dimModule = true;
   }

   if (TheSynth->GetHeldSample() != nullptr && !CanDropSample())
      dimModule = true;
   
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
      ofRect(0,-titleBarHeight,w,titleBarHeight*2);
      ofPopMatrix();
      
      if (mEnabledCheckbox)
      {
         enableToggleOffset = TitleBarHeight();
         mEnabledCheckbox->Draw();
      }
      
      if (IsSaveable() && !Minimized())
      {
         ofSetColor(color.r, color.g, color.b, gModuleDrawAlpha);
         if (TheSaveDataPanel->GetModule() == this)
         {
            ofTriangle(w-9, -titleBarHeight+2,
                       w-9, -2,
                       w-2, -titleBarHeight / 2);
         }
         else
         {
            ofTriangle(w-10, -titleBarHeight+3,
                       w-2,  -titleBarHeight+3,
                       w-6,  -2);
         }
      }
      ofPopStyle();
   }
   
   const bool kDrawInnerFade = true;
   if (kDrawInnerFade && !Push2Control::sDrawingPush2Display)
   {
      float fadeRoundness = 100;
      float fadeLength = w / 3;
      const float kFadeStrength = .75f;
      NVGpaint shadowPaint = nvgBoxGradient(gNanoVG, 0, -titleBarHeight, w,h+titleBarHeight, fadeRoundness, fadeLength, nvgRGBA(color.r * .2f, color.g * .2f, color.b * .2f, 255 * kFadeStrength), nvgRGBA(0,0,0,0));
      nvgBeginPath(gNanoVG);
      nvgRect(gNanoVG, 0, -titleBarHeight, w,h+titleBarHeight);
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
      DrawTextBold(GetTitleLabel(), 5 + enableToggleOffset, 10 - titleBarHeight, 16);
   }

   bool groupSelected = !TheSynth->GetGroupSelectedModules().empty() && VectorContains(this, TheSynth->GetGroupSelectedModules());
   if ((Enabled() || groupSelected) && mShouldDrawOutline)
   {
      ofPushStyle();
      ofNoFill();

      if (groupSelected)
      {
         float pulse = ofMap(sin(gTime / 500 * PI * 2), -1, 1, .2f, 1);
         ofSetColor(ofLerp(color.r, 255, pulse), ofLerp(color.g, 255, pulse), ofLerp(color.b, 255, pulse), 255);
         ofSetLineWidth(2);
      }
      else
      {
         ofSetColor(color.r*(.5f + highlight), color.g*(.5f + highlight), color.b*(.5f + highlight), 255);
         ofSetLineWidth(1);
      }      
      ofRect(-.5f, -titleBarHeight-.5f, w+1, h+titleBarHeight+1, 4);
      ofPopStyle();
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
   GetDimensions(w,h);
   
   ofPushMatrix();
   ofPushStyle();
   
   float titleBarHeight;
   float highlight;
   DrawFrame(w,h,true,titleBarHeight,highlight);

   if (Minimized() || IsVisible() == false)
      DrawBeacon(30,-titleBarHeight/2);

   ofPushStyle();
   const float kPipWidth = 8;
   const float kPipSpacing = 2;
   float receiveIndicatorX = w - (kPipWidth + kPipSpacing);
   ofFill();
   
   if (CanReceiveAudio())
   {
      ofSetColor(GetColor(kModuleType_Audio));
      ofRect(receiveIndicatorX, -titleBarHeight - 2, kPipWidth, 3, 1.0f);
      receiveIndicatorX -= kPipWidth + kPipSpacing;
   }
   if (CanReceiveNotes())
   {
      ofSetColor(GetColor(kModuleType_Note));
      ofRect(receiveIndicatorX, -titleBarHeight - 2, kPipWidth, 3, 1.0f);
      receiveIndicatorX -= kPipWidth + kPipSpacing;
   }
   if (CanReceivePulses())
   {
      ofSetColor(GetColor(kModuleType_Pulse));
      ofRect(receiveIndicatorX, -titleBarHeight - 2, kPipWidth, 3, 1.0f);
      receiveIndicatorX -= kPipWidth + kPipSpacing;
   }
   ofPopStyle();
   
   if (IsResizable() && !Minimized())
   {
      ofColor color = GetColor(mModuleType);
      ofSetColor(color, 255);
      if (mHoveringOverResizeHandle)
         ofSetLineWidth(4);
      else
         ofSetLineWidth(2);
      ofLine(w-sResizeCornerSize, h, w, h);
      ofLine(w, h-sResizeCornerSize, w, h);
   }
   
   gModuleDrawAlpha = 255;
   
   ofFill();
   
   if (TheSynth->ShouldAccentuateActiveModules() && GetParent() == nullptr)
   {
      ofSetColor(0,0,0,ofMap(highlight,0,.1f,200,0,K(clamp)));
      ofRect(0,-titleBarHeight,w,h+titleBarHeight);
   }
   
   ofPopMatrix();
   ofPopStyle();

   for (auto source : mPatchCableSources)
   {
      source->UpdatePosition(false);
      source->DrawSource();
   }
}

void IDrawableModule::RenderUnclipped()
{
   if (!mShowing)
      return;

   ofPushMatrix();
   ofPushStyle();

   ofTranslate(mX, mY, 0);
   ofColor color = GetColor(mModuleType);
   ofSetColor(color);

   DrawModuleUnclipped();

   ofPopMatrix();
   ofPopStyle();
}

void IDrawableModule::DrawPatchCables(bool parentMinimized)
{
   for (auto source : mPatchCableSources)
   {
      source->UpdatePosition(parentMinimized);
      source->DrawCables(parentMinimized);
   }
}

//static
ofColor IDrawableModule::GetColor(ModuleType type)
{
   ofColor color;
   color.setHsb(0, 0, sBrightness);
   if (type == kModuleType_Note)
      color.setHsb(sHueNote, sSaturation, sBrightness);
   if (type == kModuleType_Synth)
      color.setHsb(sHueInstrument, sSaturation, sBrightness);
   if (type == kModuleType_Audio)
      color.setHsb(sHueAudio, sSaturation, sBrightness);
   if (type == kModuleType_Instrument)
      color.setHsb(sHueNoteSource, sSaturation, sBrightness);
   if (type == kModuleType_Processor)
      color.setHsb(170, 100, 255);
   if (type == kModuleType_Modulator)
      color.setHsb(200, 100, 255);
   if (type == kModuleType_Pulse)
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
   
   ofColor lineColor = GetColor(kModuleType_Other);
   lineColor.setBrightness(lineColor.getBrightness() * .8f);
   ofColor lineColorAlphaed = lineColor;
   lineColorAlphaed.a = lineAlpha;
   
   float wThis,hThis,xThis,yThis;
   GetDimensions(wThis,hThis);
   GetPosition(xThis,yThis);

   ofSetLineWidth(plugWidth);
   ofSetColor(lineColor);
   ofLine(cable.plug.x,cable.plug.y,cable.end.x,cable.end.y);
   
   ofSetLineWidth(lineWidth);
   ofSetColor(lineColorAlphaed);
   ofLine(cable.start.x,cable.start.y,cable.plug.x,cable.plug.y);
   
   ofPopStyle();
   ofPopMatrix();
}

void IDrawableModule::SetTarget(IClickable* target)
{
   mMainPatchCableSource->SetTarget(target);
}

void IDrawableModule::SetUpPatchCables(std::string targets)
{
   assert(mMainPatchCableSource != nullptr);
   std::vector<std::string> targetVec = ofSplitString(targets, ",");
   if (targetVec.empty() || targets == "")
   {
      mMainPatchCableSource->Clear();
   }
   else
   {
      for (int i=0; i<targetVec.size(); ++i)
      {
         IClickable* target = dynamic_cast<IClickable*>(TheSynth->FindModule(targetVec[i]));
         if (target)
            mMainPatchCableSource->AddPatchCable(target);
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

bool IDrawableModule::TestClick(int x, int y, bool right, bool testOnly /*=false*/)
{
   for (auto source : mPatchCableSources)
   {
      if (source->TestClick(x, y, right, testOnly))
         return true;
   }
   
   if (IClickable::TestClick(x, y, right, testOnly))
      return true;
   
   return false;
}

void IDrawableModule::OnClicked(int x, int y, bool right)
{
   float w,h;
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
      else if (!Minimized() && IsSaveable() &&
               x > w - 10)
      {
         if (TheSaveDataPanel->GetModule() == this)
            TheSaveDataPanel->SetModule(nullptr);
         else
            TheSaveDataPanel->SetModule(this);
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
      for (int i=0; i<mUIControls.size(); ++i)
      {
         if (mUIControls[i] == seq->GetUIControl())
            TheSynth->MoveToFront(seq);
      }
   }
   TheSynth->MoveToFront(this);
}

bool IDrawableModule::MouseMoved(float x, float y)
{
   float w,h;
   GetModuleDimensions(w, h);
   if (mShowing && !Minimized() && IsResizable() && x > w - sResizeCornerSize && y > h - sResizeCornerSize && x <= w && y <= h)
      mHoveringOverResizeHandle = true;
   else
      mHoveringOverResizeHandle = false;
   
   if (!mShowing)
      return false;
   for (auto source : mPatchCableSources)
      source->NotifyMouseMoved(x,y);
   if (Minimized())
      return false;
   for (int i=0; i<mUIControls.size(); ++i)
      mUIControls[i]->NotifyMouseMoved(x,y);
   for (int i=0; i<mChildren.size(); ++i)
      mChildren[i]->NotifyMouseMoved(x, y);
   
   return false;
}

void IDrawableModule::MouseReleased()
{
   mWasMinimizeAreaClicked = false;
   for (int i=0; i<mUIControls.size(); ++i)
      mUIControls[i]->MouseReleased();
   for (int i=0; i<mChildren.size(); ++i)
      mChildren[i]->MouseReleased();
   auto sources = mPatchCableSources;
   for (auto source : sources)
      source->MouseReleased();
}

IUIControl* IDrawableModule::FindUIControl(const char* name, bool fail /*=true*/) const
{
   if (name != 0)
   {
      for (int i=0; i<mUIControls.size(); ++i)
      {
         if (strcmp(mUIControls[i]->Name(),name) == 0)
            return mUIControls[i];
      }
   }
   if (fail)
      throw UnknownUIControlException();
   return nullptr;
}

IDrawableModule* IDrawableModule::FindChild(const char* name) const
{
   for (int i=0; i<mChildren.size(); ++i)
   {
      if (strcmp(mChildren[i]->Name(),name) == 0)
         return mChildren[i];
   }
   throw UnknownModuleException(name);
   return nullptr;
}

void IDrawableModule::AddChild(IDrawableModule* child)
{
   if (dynamic_cast<IDrawableModule*>(child->GetParent()))
      dynamic_cast<IDrawableModule*>(child->GetParent())->RemoveChild(child);
   child->SetParent(this);
   if (child->Name()[0] == 0)
      child->SetName(("child"+ofToString(mChildren.size())).c_str());
   
   for (int i=0; i<mChildren.size(); ++i)
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
   for (int i=0; i<mChildren.size(); ++i)
   {
      std::vector<IUIControl*> childControls = mChildren[i]->GetUIControls();
      controls.insert(controls.end(), childControls.begin(), childControls.end());
   }
   return controls;
}

void IDrawableModule::GetDimensions(float& width, float& height)
{
   int minimizedWidth = GetMinimizedWidth();
   int minimizedHeight = 0;
   
   float moduleWidth,moduleHeight;
   GetModuleDimensions(moduleWidth, moduleHeight);
   if (moduleWidth == 1 && moduleHeight == 1)
   {
      width = 1; height = 1; return;   //special case for repatch stub, let it be 1 pixel
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
      mTitleLabelWidth = gFont.GetStringWidth(GetTitleLabel(), 16);
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
      if (CanSaveState() && name.empty() == false)
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

void IDrawableModule::RemoveUIControl(IUIControl* control)
{
   RemoveFromVector(control, mUIControls, K(fail));
   FloatSlider* slider = dynamic_cast<FloatSlider*>(control);
   if (slider)
   {
      mSliderMutex.lock();
      RemoveFromVector(slider, mFloatSliders, K(fail));
      mSliderMutex.unlock();
   }
}

void IDrawableModule::ComputeSliders(int samplesIn)
{
   //mSliderMutex.lock(); TODO(Ryan) mutex acquisition is slow, how can I do this faster?
   for (int i=0; i<mFloatSliders.size(); ++i)
      mFloatSliders[i]->Compute(samplesIn);
   //mSliderMutex.unlock();
}

PatchCableOld IDrawableModule::GetPatchCableOld(IClickable* target)
{
   float wThis,hThis,xThis,yThis,wThat,hThat,xThat,yThat;
   GetDimensions(wThis,hThis);
   GetPosition(xThis,yThis);
   if (target)
   {
      target->GetDimensions(wThat,hThat);
      target->GetPosition(xThat,yThat);
   }
   else
   {
      wThat = 0;
      hThat = 0;
      xThat = xThis + wThis/2;
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
   
   float startX,startY,endX,endY;
   FindClosestSides(xThis,yThis,wThis,hThis,xThat,yThat,wThat,hThat, startX,startY,endX,endY);
   
   float diffX = endX-startX;
   float diffY = endY-startY;
   float length = sqrtf(diffX*diffX + diffY*diffY);
   float endCap = MIN(.5f,20/length);
   int plugX = int((startX-endX)*endCap)+endX;
   int plugY = int((startY-endY)*endCap)+endY;
   
   PatchCableOld cable;
   cable.start.x = startX;
   cable.start.y = startY;
   cable.end.x = endX;
   cable.end.y = endY;
   cable.plug.x = plugX;
   cable.plug.y = plugY;
   
   return cable;
}

void IDrawableModule::FindClosestSides(float xThis, float yThis, float wThis, float hThis, float xThat, float yThat, float wThat, float hThat, float& startX, float& startY, float& endX, float& endY, bool sidesOnly /*= false*/)
{
   ofVec2f vDirs[4];
   vDirs[0].set(-1,0);
   vDirs[1].set(1,0);
   vDirs[2].set(0,-1);
   vDirs[3].set(0,1);

   ofVec2f vThis[4];
   vThis[0].set(xThis,yThis+hThis/2);        //left
   vThis[1].set(xThis+wThis,yThis+hThis/2);  //right
   vThis[2].set(xThis+wThis/2,yThis);        //top
   vThis[3].set(xThis+wThis/2,yThis+hThis);  //bottom
   ofVec2f vThat[4];
   vThat[0].set(xThat,yThat+hThat/2);
   vThat[1].set(xThat+wThat,yThat+hThat/2);
   vThat[2].set(xThat+wThat/2,yThat);
   vThat[3].set(xThat+wThat/2,yThat+hThat);

   float closest = FLT_MAX;
   int closestPair = 0;
   for (int i=0; i<4; ++i)
   {
      if (i == 2) //skip top
         continue;

      if (sidesOnly && i == 3)
         continue;   //skip bottom
      
      for (int j=0; j<4; ++j)
      {
         ofVec2f vDiff = vThat[j]-vThis[i];
         float distSq = vDiff.lengthSquared();
         if (distSq < closest &&
             //i/2 == j/2 &&   //only connect horizontal-horizontal, vertical-vertical
             //((i/2 == 0 && fabs(vDiff.x) > 10) ||
             // (i/2 == 1 && fabs(vDiff.y) > 10)
             //) &&
             vDiff.dot(vDirs[i]) > 0 &&
             vDiff.dot(vDirs[j]) < 0
            )
         {
            closest = distSq;
            closestPair = i + j*4;
         }
      }
   }

   startX = vThis[closestPair%4].x;
   startY = vThis[closestPair%4].y;
   endX = vThat[closestPair/4].x;
   endY = vThat[closestPair/4].y;
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

bool IDrawableModule::CheckNeedsDraw()
{
   if (IClickable::CheckNeedsDraw())
      return true;
   
   for (int i=0; i<mUIControls.size(); ++i)
   {
      if (mUIControls[i]->CheckNeedsDraw())
         return true;
   }
   
   for (int i=0; i<mChildren.size(); ++i)
   {
      if (mChildren[i]->CheckNeedsDraw())
         return true;
   }
   
   return false;
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

   
   SetPosition(x,y);
   
   SetName(name.c_str());
   
   SetMinimized(start_minimized);
   
   if (mMinimized)
      mMinimizeAnimation = 1;
   
   if (draw_lissajous)
      TheSynth->AddLissajousDrawer(this);
   
   mTypeName = typeName;
}

void IDrawableModule::SaveLayout(ofxJSONElement& moduleInfo)
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
}

namespace
{
   const int kSaveStateRev = 1;
   const int kControlSeparatorLength = 16;
   const char kControlSeparator[kControlSeparatorLength+1] = "controlseparator";
}

void IDrawableModule::SaveState(FileStreamOut& out)
{
   if (!CanSaveState())
      return;
   
   out << kSaveStateRev;
   
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
      control->SaveState(out);
      for (int i=0; i<kControlSeparatorLength; ++i)
         out << kControlSeparator[i];
   }
   
   if (GetContainer())
      GetContainer()->SaveState(out);
   
   out << (int)mChildren.size();
   
   for (auto* child : mChildren)
   {
      out << std::string(child->Name());
      child->SaveState(out);
   }
   
   out << (int)mPatchCableSources.size();
   for (auto* cable : mPatchCableSources)
      cable->SaveState(out);
}

void IDrawableModule::LoadState(FileStreamIn& in)
{
   if (!CanSaveState())
      return;
   
   int rev;
   in >> rev;
   
   int numUIControls;
   in >> numUIControls;
   for (int i=0; i<numUIControls; ++i)
   {
      std::string uicontrolname;
      in >> uicontrolname;

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
         
         for (int j=0; j<kControlSeparatorLength; ++j)
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
               for (int c=0;c<10;++c)
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
         TheSynth->LogEvent("Error in module \""+std::string(Name())+"\" loading state for control \""+uicontrolname+"\"", kLogEventType_Error);
         
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
               break;   //we did it!
         }
      }
   }
   
   if (GetContainer())
      GetContainer()->LoadState(in);
   
   int numChildren;
   in >> numChildren;
   LoadStateValidate(numChildren <= mChildren.size());
   
   for (int i=0; i<numChildren; ++i)
   {
      std::string childName;
      in >> childName;
      //ofLog() << "Loading " << childName;
      IDrawableModule* child = FindChild(childName.c_str());
      LoadStateValidate(child);
      child->LoadState(in);
   }
   
   if (rev >= 1)
   {
      int numPatchCableSources;
      in >> numPatchCableSources;
      PatchCableSource* dummy = nullptr;
      for (int i=0; i<numPatchCableSources; ++i)
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
