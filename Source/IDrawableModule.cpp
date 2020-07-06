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

float IDrawableModule::sHueNote = 27;
float IDrawableModule::sHueAudio = 135;
float IDrawableModule::sHueInstrument = 79;
float IDrawableModule::sHueNoteSource = 240;
float IDrawableModule::sSaturation = 145;
float IDrawableModule::sBrightness = 220;

IDrawableModule::IDrawableModule()
: mModuleType(kModuleType_Unknown)
, mMinimized(false)
, mMinimizeAreaClicked(false)
, mMinimizeAnimation(0)
, mEnabled(true)
, mEnabledCheckbox(nullptr)
, mUIControlsCreated(false)
, mInitialized(false)
, mMainPatchCableSource(nullptr)
, mOwningContainer(nullptr)
, mTitleLabelWidth(0)
, mShouldDrawOutline(true)
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
   if (dynamic_cast<INoteSource*>(this))
      type = kConnectionType_Note;
   if (dynamic_cast<IGridController*>(this))
      type = kConnectionType_Grid;
   if (dynamic_cast<IPulseSource*>(this))
      type = kConnectionType_Pulse;
   if (type != kConnectionType_Special)
   {
      mMainPatchCableSource = new PatchCableSource(this, type);
      mPatchCableSources.push_back(mMainPatchCableSource);
   }
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
         int numSamples = min(500,vizBuff->Size());
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
   ofRect(0, -titleBarHeight, w, h+titleBarHeight);
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
       PatchCable::sActivePatchCable->GetConnectionType() != kConnectionType_UIControl &&
       !PatchCable::sActivePatchCable->IsValidTarget(this))
   {
      dimModule = true;
   }
   
   if (dimModule)
      gModuleDrawAlpha *= .2f;
   
   if (drawModule)
   {
      ofSetColor(color, gModuleDrawAlpha);
      ofPushMatrix();
      ofClipWindow(0, 0, w, h);
      DrawModule();
      ofPopMatrix();
      DrawModuleUnclipped();
   }
   
   float enableToggleOffset = 0;
   if (HasTitleBar())
   {
      ofSetColor(color, 50);
      ofFill();
      ofPushMatrix();
      ofClipWindow(0, -titleBarHeight, w, titleBarHeight);
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
   }
   ofSetColor(color * (1-GetBeaconAmount()) + ofColor::yellow * GetBeaconAmount(), gModuleDrawAlpha);
   DrawTextBold(GetTitleLabel(),5+enableToggleOffset,10-titleBarHeight,16);
   
   if (Enabled() && mShouldDrawOutline)
   {
      ofPushStyle();
      ofNoFill();
      ofSetColor(color.r*(.5f+highlight),color.g*(.5f+highlight),color.b*(.5f+highlight),255);
      ofSetLineWidth(1);
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
   
   if (IsResizable() && !Minimized())
   {
      ofColor color = GetColor(mModuleType);
      ofSetColor(color, 255);
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
}

void IDrawableModule::DrawPatchCables()
{
   for (auto source : mPatchCableSources)
   {
      source->UpdatePosition();
      source->Draw();
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

void IDrawableModule::SetUpPatchCables(string targets)
{
   assert(mMainPatchCableSource != nullptr);
   vector<string> targetVec = ofSplitString(targets, ",");
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
   
   if (y < 0)
   {
      if (mEnabledCheckbox && x < 20)
      {
         //"enabled" area
      }
      else if (x < GetMinimizedWidth())
      {
         mMinimizeAreaClicked = true;
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
   
   if (IsResizable() && x > w - sResizeCornerSize && y > h - sResizeCornerSize)
   {
      TheSynth->SetResizeModule(this);
      return;
   }
   
   for (int i=0; i<mUIControls.size(); ++i)
      mUIControls[i]->TestClick(x,y,right);
   for (int i=0; i<mChildren.size(); ++i)
      mChildren[i]->TestClick(x, y, right);
   
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
   if (CanMinimize() && mMinimizeAreaClicked &&
       TheSynth->HasNotMovedMouseSinceClick())
      ToggleMinimized();
   mMinimizeAreaClicked = false;
   for (int i=0; i<mUIControls.size(); ++i)
      mUIControls[i]->MouseReleased();
   for (int i=0; i<mChildren.size(); ++i)
      mChildren[i]->MouseReleased();
   for (auto source : mPatchCableSources)
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

vector<IUIControl*> IDrawableModule::GetUIControls() const
{
   vector<IUIControl*> controls = mUIControls;
   for (int i=0; i<mChildren.size(); ++i)
   {
      vector<IUIControl*> childControls = mChildren[i]->GetUIControls();
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
   string titleLabel = GetTitleLabel();
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
      string name = control->Name();
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

void IDrawableModule::FindClosestSides(float xThis,float yThis,float wThis,float hThis,float xThat,float yThat,float wThat,float hThat, float& startX,float& startY,float& endX,float& endY)
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
   if (endY > vThis[3].y)
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

void IDrawableModule::LoadBasics(const ofxJSONElement& moduleInfo, string typeName)
{
   int x = moduleInfo["position"][0u].asInt();
   int y = moduleInfo["position"][1u].asInt();
   SetPosition(x,y);
   
   SetName(moduleInfo["name"].asString().c_str());
   
   SetMinimized(moduleInfo["start_minimized"].asBool());
   
   if (mMinimized)
      mMinimizeAnimation = 1;
   
   if (moduleInfo["draw_lissajous"].asBool())
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
   
   vector<IUIControl*> controlsToSave;
   for (auto* control : mUIControls)
   {
      if (!VectorContains(control, ControlsToIgnoreInSaveState()) && control->GetShouldSaveState())
         controlsToSave.push_back(control);
   }
   
   out << (int)controlsToSave.size();
   for (auto* control : controlsToSave)
   {
      //ofLog() << "Saving control " << control->Name();
      out << string(control->Name());
      control->SaveState(out);
      for (int i=0; i<kControlSeparatorLength; ++i)
         out << kControlSeparator[i];
   }
   
   if (GetContainer())
      GetContainer()->SaveState(out);
   
   out << (int)mChildren.size();
   
   for (auto* child : mChildren)
   {
      out << string(child->Name());
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
      string uicontrolname;
      in >> uicontrolname;
      
      bool threwException = false;
      try
      {
         //ofLog() << "loading control " << uicontrolname;
         auto* control = FindUIControl(uicontrolname.c_str());
      
         float setValue = true;
         if (VectorContains(control, ControlsToNotSetDuringLoadState()))
            setValue = false;
         control->LoadState(in, setValue);
         
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
               string nextFewChars = "Next 10 characters are:";
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
         TheSynth->LogEvent("Error in module \""+string(Name())+"\" loading state for control \""+uicontrolname+"\"", kLogEventType_Error);
         
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
   LoadStateValidate(numChildren == mChildren.size());
   
   for (int i=0; i<numChildren; ++i)
   {
      string childName;
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
      PatchCableSource* dummy = new PatchCableSource(this, kConnectionType_Special);
      for (int i=0; i<numPatchCableSources; ++i)
      {
         PatchCableSource* readIn;
         if (i < mPatchCableSources.size())
            readIn = mPatchCableSources[i];
         else
            readIn = dummy;
         readIn->LoadState(in);
      }
   }
}

vector<IUIControl*> IDrawableModule::ControlsToNotSetDuringLoadState() const
{
   return vector<IUIControl*>(); //empty
}

vector<IUIControl*> IDrawableModule::ControlsToIgnoreInSaveState() const
{
   return vector<IUIControl*>(); //empty
}
