/*
  ==============================================================================

    Push2Control.h
    Created: 24 Feb 2020 8:57:57pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "push2/push2/JuceToPush2DisplayBridge.h"
#include "IDrawableModule.h"

class NVGcontext;
class NVGLUframebuffer;

class Push2Control : public IDrawableModule
{
public:
   Push2Control();
   virtual ~Push2Control();
   static IDrawableModule* Create() { return new Push2Control(); }
   
   string GetTitleLabel() override { return "push 2 control"; }
   void CreateUIControls() override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void PostRender() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(int& width, int& height) override { width = mWidth; height = mHeight; }
   
   NBase::Result Initialize();
   void DrawToFramebuffer(NVGcontext* vg, NVGLUframebuffer* fb, float t, float pxRatio);
   void RenderPush2Display();
   
   ableton::Push2DisplayBridge bridge_;      /* The bridge allowing to use juce::graphics for push */
   ableton::Push2Display push2Display_;      /* The low-level push2 class */
   NVGcontext* mVG;
   NVGLUframebuffer* mFB;
   unsigned char* mPixels;
   int mPixelRatio;
   
   int mFontHandle;
   int mFontHandleBold;
   
   int mWidth;
   int mHeight;
   
   IDrawableModule* mDisplayModule;
};
