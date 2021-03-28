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
#include "DropdownList.h"

class CanvasControls;
class CanvasTimeline;
class CanvasScrollbar;

class SampleCanvas : public IDrawableModule, public IAudioSource, public ICanvasListener, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener
{
public:
   SampleCanvas();
   ~SampleCanvas();
   static IDrawableModule* Create() { return new SampleCanvas(); }
   
   string GetTitleLabel() override { return "sample canvas"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   
   void Process(double time) override;
   NoteInterval GetInterval() const { return mInterval; }
   
   void CanvasUpdated(Canvas* canvas) override;
   
   void OnClicked(int x, int y, bool right) override;
   
   void FilesDropped(vector<string> files, int x, int y) override;
   void SampleDropped(int x, int y, Sample* sample) override;
   bool CanDropSample() const override { return true; }
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return mEnabled; }

   void SetNumMeasures(int numMeasures);
   void UpdateNumColumns();
   double GetCurPos(double time) const;
   
   Canvas* mCanvas;
   CanvasControls* mCanvasControls;
   CanvasTimeline* mCanvasTimeline;
   CanvasScrollbar* mCanvasScrollbarHorizontal;
   CanvasScrollbar* mCanvasScrollbarVertical;

   IntSlider* mNumMeasuresSlider;
   int mNumMeasures;
   NoteInterval mInterval;
   DropdownList* mIntervalSelector;
};

#endif /* defined(__Bespoke__SampleCanvas__) */
