/*
  ==============================================================================

    Push2Control.cpp
    Created: 24 Feb 2020 8:57:57pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#ifdef BESPOKE_WINDOWS
#include <GL/glew.h>
#endif

#include "Push2Control.h"
#include <cctype>
#include "SynthGlobals.h"
#include "nanovg/nanovg.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg/nanovg_gl.h"
#include "nanovg/nanovg_gl_utils.h"
#include "OpenFrameworksPort.h"
#include "UIControlMacros.h"
#include "TitleBar.h"
#include "ModuleSaveDataPanel.h"
#include "QuickSpawnMenu.h"
#include "PatchCableSource.h"

bool Push2Control::sDrawingPush2Display = false;
NVGcontext* Push2Control::sVG = nullptr;
NVGLUframebuffer* Push2Control::sFB = nullptr;

Push2Control::Push2Control()
: mDisplayInitialized(false)
, mDisplayModule(nullptr)
, mDevice(this)
, mModuleColumnOffset(0)
, mModuleColumnOffsetSmoothed(0)
, mModuleListOffset(0)
, mModuleListOffsetSmoothed(0)
, mNewButtonHeld(false)
, mDeleteButtonHeld(false)
, mHeldModule(nullptr)
, mAllowRepatch(false)
{
   NBase::Result result = Initialize();
   if (result.Succeeded())
   {
      ofLog() << "push 2 connected";
      mDisplayInitialized = true;
   }
   else
   {
      ofLog() << "push 2 failed to connect";
      mDisplayInitialized = false;
   }
   for (int i=0; i<128*2; ++i)
      mLedState[i] = -1;
   for (int i=0; i<8*8; ++i)
      mModuleGrid[i] = nullptr;
}

Push2Control::~Push2Control()
{
}

void Push2Control::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   //UIBLOCK0();
   //ENDUIBLOCK(mWidth, mHeight);
   mWidth = 100;
   mHeight = 20;
}

void Push2Control::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   if (!mDisplayInitialized)
   {
      ofSetColor(255, 0, 0, gModuleDrawAlpha);
      DrawTextNormal("no push 2 found", 3, 15);
   }
}

void Push2Control::DrawModuleUnclipped()
{
   if (mDisplayModule != nullptr && !sDrawingPush2Display)
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
   if (mDisplayInitialized)
      RenderPush2Display();
}

void Push2Control::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x,y,right);
   
   if (!mDisplayInitialized)
   {
      NBase::Result result = Initialize();
      if (result.Succeeded())
      {
         ofLog() << "push 2 connected";
         mDisplayInitialized = true;
      }
      else
      {
         ofLog() << "push 2 failed to connect";
         mDisplayInitialized = false;
      }
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
   sVG = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
   assert(sVG);

   const auto width = ableton::Push2DisplayBitmap::kWidth;
   const auto height = ableton::Push2DisplayBitmap::kHeight;
   const int pixelRatio = 1;

   sFB = nvgluCreateFramebuffer(sVG, width*pixelRatio, height*pixelRatio, 0);
   assert(sFB);
}

NBase::Result Push2Control::Initialize()
{
   // First we initialise the low level push2 object
   NBase::Result result = push2Display_.Init();
   RETURN_IF_FAILED_MESSAGE(result, "Failed to init push2");

   // Then we initialise the juce to push bridge
   result = bridge_.Init(push2Display_);
   RETURN_IF_FAILED_MESSAGE(result, "Failed to init bridge");
   
   const auto width = ableton::Push2DisplayBitmap::kWidth;
   const auto height = ableton::Push2DisplayBitmap::kHeight;
   
   mPixels = new unsigned char[3 * (width * kPixelRatio) * (height * kPixelRatio)];
   
   mFontHandle = nvgCreateFont(sVG, ofToDataPath("frabk.ttf").c_str(), ofToDataPath("frabk.ttf").c_str());
   mFontHandleBold = nvgCreateFont(sVG, ofToDataPath("frabk_m.ttf").c_str(), ofToDataPath("frabk_m.ttf").c_str());
   
   const std::vector<string>& devices = mDevice.GetPortList(false);
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
         
         SetLed(kMidiMessage_Control, 87, 127);
         SetLed(kMidiMessage_Control, 118, 127);
         break;
      }
   }

   return NBase::Result::NoError;
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
   glClear(GL_COLOR_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
   nvgBeginFrame(vg, winWidth, winHeight, pxRatio);
   
   nvgLineCap(vg, NVG_ROUND);
   nvgLineJoin(vg, NVG_ROUND);
   static float sSpacing = -.3f;
   nvgTextLetterSpacing(vg, sSpacing);
   
   ofSetColor(255,255,255);
   if (mDisplayModule != nullptr)
   {
      mModuleColumnOffsetSmoothed = ofLerp(mModuleColumnOffsetSmoothed, mModuleColumnOffset, .3f);
      
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
      DrawControls(mSliderControls, 20);
      DrawControls(mButtonControls, 60);
      
      int topRowLedColors[8] = {0,0,0,0,0,0,0,0};
      for (int i=0; i < mButtonControls.size(); ++i)
      {
         if (i - mModuleColumnOffset >= 0 && i - mModuleColumnOffset < 8)
            topRowLedColors[i-mModuleColumnOffset] = GetColorForType(mButtonControls[i]->GetModuleParent()->GetModuleType());
      }
      for (int i=0; i<8; ++i)
         SetLed(kMidiMessage_Control, i+102, topRowLedColors[i]);
      
      ofPopMatrix();
      ofPopStyle();
   }
   
   mModules.clear();
   for (int i=0; i<8*8; ++i)
      mModuleGrid[i] = nullptr;
   
   vector<IDrawableModule*> modules;
   TheSynth->GetAllModules(modules);
   
   float minX = 0;
   float minY = 0;
   float maxX = ofGetWidth();
   float maxY = ofGetHeight();
   for (int i=0; i<modules.size(); ++i)
   {
      if (!IsIgnorableModule(modules[i]))
      {
         mModules.push_back(modules[i]);
         ofVec2f pos = modules[i]->GetPosition();
         if (pos.x > maxX)
            maxX = pos.x;
         if (pos.y > maxY)
            maxY = pos.y;
         if (pos.x < minX)
            minX = pos.x;
         if (pos.y < minY)
            minY = pos.y;
      }
   }
   maxX += 1;
   maxY += 1;
   
   mModules = SortModules(mModules);
   
   for (int i=0; i<mModules.size(); ++i)
   {
      ofVec2f pos = mModules[i]->GetRect().getCenter();
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
   
   for (int i=0; i<8*8; ++i)
   {
      int gridX = i % 8;
      int gridY = i / 8;
      int gridIndex = gridX + (7-gridY) * 8;
      int padNumber = 36 + i;
      if (mModuleGrid[gridIndex] != nullptr)
         SetLed(kMidiMessage_Note, padNumber, GetColorForType(mModuleGrid[gridIndex]->GetModuleType()), mModuleGrid[gridIndex] == mDisplayModule ? 10 : 1);
      else
         SetLed(kMidiMessage_Note, padNumber, 0);
   }
   
   mModuleListOffsetSmoothed = ofLerp(mModuleListOffsetSmoothed, round(mModuleListOffset), .3f);
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
         bottomRowLedColors[i-(int)round(mModuleListOffset)] = GetColorForType(mModules[i]->GetModuleType());
      
      ofPopMatrix();
      ofPopStyle();
   }
   
   for (int i=0; i<8; ++i)
      SetLed(kMidiMessage_Control, i+20, bottomRowLedColors[i]);
   
   SetLed(kMidiMessage_Control, 35, mAllowRepatch ? 127 : 5, mAllowRepatch ? 10 : 1);
   
   //test led colors
   //SetLed(kMidiMessage_Note, 92, (int)mModuleListOffset);
   //ofLog() << (int)mModuleListOffset;

   nvgEndFrame(vg);
   
   glFinish();
   glReadBuffer(GL_COLOR_ATTACHMENT0);
   glReadPixels(0, 0, winWidth, winHeight, GL_RGB, GL_UNSIGNED_BYTE, mPixels);
   
   nvgluBindFramebuffer(NULL);
}

int Push2Control::GetColorForType(ModuleType type)
{
   int color;
   switch (type)
   {
      case kModuleType_Instrument:
         color = 115;
         break;
      case kModuleType_Note:
         color = 8;
         break;
      case kModuleType_Synth:
         color = 11;
         break;
      case kModuleType_Audio:
         color = 20;
         break;
      case kModuleType_Modulator:
         color = 22;
         break;
      default:
         color = 123;
         break;
   }
   return color;
}

void Push2Control::DrawControls(vector<IUIControl*> controls, float yPos)
{
   for (int i=0; i < controls.size(); ++i)
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
      ofSetColor(IDrawableModule::GetColor(controls[i]->GetModuleParent()->GetModuleType()));
      
      if (mDisplayModule == this)
         DrawTextBold(juce::String(controls[i]->Path()).replace("~","\n").toRawUTF8(), kColumnSpacing * i + 3, yPos-12, 10);
      else
         DrawTextBold(controls[i]->Name(), kColumnSpacing * i + 3, yPos-5, 16);
      
      ofPopStyle();
   }
}

void Push2Control::RenderPush2Display()
{
   if (gHoveredModule != nullptr)
   {
      if (mDisplayModule != gHoveredModule && gHoveredModule != mLastModuleSetFromHover && !IsIgnorableModule(gHoveredModule))
      {
         SetDisplayModule(gHoveredModule);
         mLastModuleSetFromHover = gHoveredModule;
      }
   }
   
   auto mainVG = gNanoVG;
   gNanoVG = sVG;
   sDrawingPush2Display = true;
   DrawToFramebuffer(sVG, sFB, gTime/300, kPixelRatio);
   sDrawingPush2Display = false;
   gNanoVG = mainVG;

   // Tells the bridge we're done with drawing and the frame can be sent to the display
   bridge_.Flip(mPixels);
}

void Push2Control::SetDisplayModule(IDrawableModule* module)
{
   mDisplayModule = module;
   mModuleColumnOffset = 0;
   mModuleColumnOffsetSmoothed = 0;
   UpdateControlList();
}

void Push2Control::UpdateControlList()
{
   mSliderControls.clear();
   mButtonControls.clear();
   vector<IUIControl*> controls;
   if (mDisplayModule == this)
      controls = mFavoriteControls;
   else
      controls = mDisplayModule->GetUIControls();
   for (int i=0; i < controls.size(); ++i)
   {
      if (controls[i]->IsSliderControl())
         mSliderControls.push_back(controls[i]);
      if (controls[i]->IsButtonControl())
         mButtonControls.push_back(controls[i]);
   }
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

void Push2Control::SetLed(MidiMessageType type, int index, int color, int channel)
{
   if (type == kMidiMessage_Control)
      index += 128;
   assert(index >= 0 && index < 128 * 2);
   
   int stateValue = color | channel << 8;
   if (mLedState[index] != stateValue)
   {
      bool isPulse = (channel >= 7 && channel <= 11);
      mLedState[index] = stateValue;
      if (index < 128)
      {
         if (isPulse)
            mDevice.SendNote(index, color == 122 || color == 127 ? 0 : 122);
         mDevice.SendNote(index, color, false, channel);
      }
      else
      {
         if (isPulse)
            mDevice.SendCC(index-128, color == 122 || color == 127 ? 0 : 122);
         mDevice.SendCC(index-128, color, channel);
      }
   }
}

void Push2Control::OnMidiNote(MidiNote& note)
{
   if (note.mPitch >= 0 && note.mPitch <= 7) //main encoders
   {
      int controlIndex = note.mPitch + mModuleColumnOffset;
      if (note.mVelocity > 0 && controlIndex < mSliderControls.size())
      {
         mSliderControls[controlIndex]->StartBeacon();
         
         if (mNewButtonHeld)
            AddFavoriteControl(mSliderControls[controlIndex]);
         if (mDeleteButtonHeld)
            RemoveFavoriteControl(mSliderControls[controlIndex]);
      }
   }
   else if (note.mPitch >= 36 && note.mPitch <= 99)  //pads
   {
      if (note.mVelocity > 0)
      {
         int gridIndex = note.mPitch - 36;
         int gridX = gridIndex % 8;
         int gridY = gridIndex / 8;
         gridIndex = gridX + (7-gridY) * 8;
         if (mModuleGrid[gridIndex] != nullptr)
            SetDisplayModule(mModuleGrid[gridIndex]);
         
         if (mHeldModule != nullptr)
         {
            if (mHeldModule->GetPatchCableSource() != nullptr)
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
   else
   {
      ofLog() << "note " << note.mPitch << " " << note.mVelocity;
   }
}

void Push2Control::OnMidiControl(MidiControl& control)
{
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
   else if (control.mControl >= 102 && control.mControl <= 110) //buttons below encoders
   {
      int controlIndex = control.mControl - 102 + mModuleColumnOffset;
      if (control.mValue > 0 && controlIndex < mButtonControls.size())
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
         else
         {
            float current = mButtonControls[controlIndex]->GetMidiValue();
            float newValue = current > 0 ? 0 : 1;
            mButtonControls[controlIndex]->SetFromMidiCC(newValue);
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
   else if (control.mControl >= 20 && control.mControl <= 27) //buttons below screen
   {
      int moduleIndex = control.mControl - 20 + round(mModuleListOffset);
      if (control.mValue > 0 && moduleIndex < mModules.size())
      {
         SetDisplayModule(mModules[moduleIndex]);
      }
   }
   else if (control.mControl == 87) //"new"
   {
      mNewButtonHeld = control.mValue > 0;
   }
   else if (control.mControl == 118) //"delete"
   {
      mDeleteButtonHeld = control.mValue > 0;
   }
   else if (control.mControl == 35) //"convert"
   {
      if (control.mValue > 0)
         mAllowRepatch = !mAllowRepatch;
   }
   else
   {
      ofLog() << "control " << control.mControl << " " << control.mValue;
   }
}

bool Push2Control::IsIgnorableModule(IDrawableModule* module)
{
   return module == TheTitleBar || module == TheSaveDataPanel || module == TheQuickSpawnMenu;
}

vector<IDrawableModule*> Push2Control::SortModules(vector<IDrawableModule*> modules)
{
   vector<IDrawableModule*> output;
    
   for (int i=0; i<modules.size(); ++i)
      AddModuleChain(modules[i], modules, output);
   
   return output;
}

void Push2Control::AddModuleChain(IDrawableModule* module, vector<IDrawableModule*>& modules, vector<IDrawableModule*>& output)
{
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
            AddModuleChain(modules[i], modules, output);
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
            AddModuleChain(modules[i], modules, output);
         }
      }
   }
}
