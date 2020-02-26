/*
  ==============================================================================

    Push2Control.cpp
    Created: 24 Feb 2020 8:57:57pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "Push2Control.h"
#include <cctype>
#include "SynthGlobals.h"
#include "nanovg/nanovg.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg/nanovg_gl.h"
#include "nanovg/nanovg_gl_utils.h"
#include "OpenFrameworksPort.h"
#include "UIControlMacros.h"

bool Push2Control::sDrawingPush2Display = false;

Push2Control::Push2Control()
: mDisplayModule(nullptr)
, mDevice(this)
{
   NBase::Result result = Initialize();
   if (result.Succeeded())
   {
      ofLog() << "push 2 connected";
   }
   else
   {
      ofLog() << "push 2 failed to connect";
   }
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
}
 
void Push2Control::PostRender()
{
   RenderPush2Display();
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

NBase::Result Push2Control::Initialize()
{
   mVG = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
   
   const auto width = ableton::Push2DisplayBitmap::kWidth;
   const auto height = ableton::Push2DisplayBitmap::kHeight;
   mPixelRatio = 1;
   
   mFB = nvgluCreateFramebuffer(mVG, width*mPixelRatio, height*mPixelRatio, 0);
   if (mFB == NULL) {
      printf("Could not create FBO.\n");
   }
   
   // First we initialise the low level push2 object
   NBase::Result result = push2Display_.Init();
   RETURN_IF_FAILED_MESSAGE(result, "Failed to init push2");

   // Then we initialise the juce to push bridge
   result = bridge_.Init(push2Display_);
   RETURN_IF_FAILED_MESSAGE(result, "Failed to init bridge");
   
   mPixels = new unsigned char[3 * (width * mPixelRatio) * (height * mPixelRatio)];
   
   mFontHandle = nvgCreateFont(mVG, ofToDataPath("frabk.ttf").c_str(), ofToDataPath("frabk.ttf").c_str());
   mFontHandleBold = nvgCreateFont(mVG, ofToDataPath("frabk_m.ttf").c_str(), ofToDataPath("frabk_m.ttf").c_str());
   
   const std::vector<string>& devices = mDevice.GetPortList(false);
   for (int i=0; i<devices.size(); ++i)
   {
      if (strcmp(devices[i].c_str(), "Ableton Push 2 Live Port") == 0)
      {
         mDevice.ConnectOutput(i);
         mDevice.ConnectInput(devices[i].c_str());
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
      const float kSpacing = 121;
      
      mModuleColumnOffsetSmoothed = ofLerp(mModuleColumnOffsetSmoothed, mModuleColumnOffset, .3f);
      
      //nvgFontSize(mVG, 16);
      //nvgText(mVG, 10, 10, mDisplayModule->Name(), nullptr);
      float x;
      float y;
      mDisplayModule->GetPosition(x, y, true);
      mDisplayModule->SetPosition(5 - kSpacing * mModuleColumnOffsetSmoothed, 15);
      float titleBarHeight;
      float highlight;
      mDisplayModule->DrawFrame(kSpacing * MAX(1,MAX(mSliderControls.size(), mButtonControls.size())) - 14, 80, false, titleBarHeight, highlight);
      mDisplayModule->SetPosition(x, y);
      
      ofSetColor(IDrawableModule::GetColor(mDisplayModule->GetModuleType()));
      ofNoFill();
      
      nvgFontSize(mVG, 16);
      for (int i=0; i < mSliderControls.size(); ++i)
      {
         if (i - mModuleColumnOffset < -1 || i - mModuleColumnOffset > 8)
            continue;
         
         nvgFontFaceId(mVG, mFontHandleBold);
         nvgFontSize(mVG, 16);
         nvgText(mVG, kSpacing * i + 3, 15, mSliderControls[i]->Name(), nullptr);
         
         float x;
         float y;
         mSliderControls[i]->GetPosition(x, y, true);
         mSliderControls[i]->SetPosition(kSpacing * i + 3, 20);
         mSliderControls[i]->Render();
         mSliderControls[i]->SetPosition(x, y);
      }
      for (int i=0; i < mButtonControls.size(); ++i)
      {
         if (i - mModuleColumnOffset < -1 || i - mModuleColumnOffset > 8)
            continue;
         
         nvgFontFaceId(mVG, mFontHandleBold);
         nvgFontSize(mVG, 16);
         nvgText(mVG, kSpacing * i + 3, 55, mButtonControls[i]->Name(), nullptr);
         
         float x;
         float y;
         mButtonControls[i]->GetPosition(x, y, true);
         mButtonControls[i]->SetPosition(kSpacing * i + 3, 60);
         mButtonControls[i]->Render();
         mButtonControls[i]->SetPosition(x, y);
      }
   }

   nvgEndFrame(vg);
   
   glFinish();
   glReadBuffer(GL_COLOR_ATTACHMENT0);
   glReadPixels(0, 0, winWidth, winHeight, GL_RGB, GL_UNSIGNED_BYTE, mPixels);
   
   nvgluBindFramebuffer(NULL);
}

void Push2Control::RenderPush2Display()
{
   if (gHoveredModule != nullptr)
   {
      if (mDisplayModule != gHoveredModule)
      {
         mDisplayModule = gHoveredModule;
         mModuleColumnOffset = 0;
         mModuleColumnOffsetSmoothed = 0;
         mSliderControls.clear();
         mButtonControls.clear();
         vector<IUIControl*> controls = mDisplayModule->GetUIControls();
         for (int i=0; i < controls.size(); ++i)
         {
            if (controls[i]->IsSliderControl())
               mSliderControls.push_back(controls[i]);
            if (controls[i]->IsButtonControl())
               mButtonControls.push_back(controls[i]);
         }
      }
   }
   
   auto mainVG = gNanoVG;
   gNanoVG = mVG;
   sDrawingPush2Display = true;
   DrawToFramebuffer(mVG, mFB, gTime/300, mPixelRatio);
   sDrawingPush2Display = false;
   gNanoVG = mainVG;

   // Tells the bridge we're done with drawing and the frame can be sent to the display
   bridge_.Flip(mPixels);
}

void Push2Control::OnMidiNote(MidiNote& note)
{
   if (note.mPitch >= 0 && note.mPitch <= 7) //main encoders
   {
      int controlIndex = note.mPitch + mModuleColumnOffset;
      if (note.mVelocity > 0 && controlIndex < mSliderControls.size())
         mSliderControls[controlIndex]->StartBeacon();
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
   else if (control.mControl >= 102 && control.mControl <= 110) //main encoders
   {
      int controlIndex = control.mControl - 102 + mModuleColumnOffset;
      if (control.mValue > 0 && controlIndex < mButtonControls.size())
      {
         float current = mButtonControls[controlIndex]->GetMidiValue();
         float newValue = current > 0 ? 0 : 1;
         mButtonControls[controlIndex]->SetFromMidiCC(newValue);
      }
   }
   else if (control.mControl == 14) //leftmost clicky encoder
   {
      int direction = control.mValue == 127 ? -1 : 1;
      mModuleColumnOffset = (int)ofClamp(mModuleColumnOffset + direction, 0, MAX(0, (int)MAX(mSliderControls.size(), mButtonControls.size()) - 8));
   }
   else
   {
      ofLog() << "control " << control.mControl << " " << control.mValue;
   }
}
