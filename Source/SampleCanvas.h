//
//  SampleCanvas.h
//  Bespoke
//
//  Created by Ryan Challinor on 6/24/15.
//
//

#ifndef __Bespoke__SampleCanvas__
#define __Bespoke__SampleCanvas__

#include "IDrawableModule.h"
#include "IAudioSource.h"
#include "Transport.h"
#include "Checkbox.h"
#include "Canvas.h"
#include "Slider.h"

class CanvasControls;

class SampleCanvas : public IDrawableModule, public IAudioSource, public ICanvasListener, public IFloatSliderListener, public IIntSliderListener
{
public:
   SampleCanvas();
   ~SampleCanvas();
   static IDrawableModule* Create() { return new SampleCanvas(); }
   
   string GetTitleLabel() override { return "sample canvas"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void Process(double time) override;
   
   void CanvasUpdated(Canvas* canvas) override;
   
   void OnClicked(int x, int y, bool right) override;
   bool MouseScrolled(int x, int y, float scrollX, float scrollY) override;
   
   void FilesDropped(vector<string> files, int x, int y) override;
   void SampleDropped(int x, int y, Sample* sample) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width=810; height=210; }
   bool Enabled() const override { return mEnabled; }
   
   Canvas* mCanvas;
   CanvasControls* mCanvasControls;
};

#endif /* defined(__Bespoke__SampleCanvas__) */
