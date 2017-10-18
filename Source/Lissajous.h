//
//  Lissajous.h
//  Bespoke
//
//  Created by Ryan Challinor on 6/26/14.
//
//

#ifndef __Bespoke__Lissajous__
#define __Bespoke__Lissajous__

#include <iostream>
#include "IAudioProcessor.h"
#include "IDrawableModule.h"

#define NUM_LISSAJOUS_POINTS 4000

class Lissajous : public IAudioProcessor, public IDrawableModule
{
public:
   Lissajous();
   virtual ~Lissajous();
   static IDrawableModule* Create() { return new Lissajous(); }
   
   string GetTitleLabel() override { return "lissajous"; }
   
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& w, int&h) override { w=mWidth; h=mHeight; }
   bool Enabled() const override { return mEnabled; }
   
   float mWidth;
   float mHeight;
   
   ofVec2f mLissajousPoints[NUM_LISSAJOUS_POINTS];
   int mOffset;
   bool mSingleInputMode;
};


#endif /* defined(__Bespoke__Lissajous__) */
