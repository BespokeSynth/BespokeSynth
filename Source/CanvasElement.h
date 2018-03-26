//
//  CanvasElement.h
//  Bespoke
//
//  Created by Ryan Challinor on 1/4/15.
//
//

#ifndef __Bespoke__CanvasElement__
#define __Bespoke__CanvasElement__

#include "Canvas.h"
#include "Curve.h"
#include "ModulationChain.h"

class Canvas;
class Sample;
class IUIControl;
class FloatSlider;
class IntSlider;
class TextEntry;
class FileStreamIn;
class FileStreamOut;
class PatchCableSource;
class EventCanvas;

class CanvasElement
{
public:
   CanvasElement(Canvas* canvas, int col, int row, float offset, float length);
   virtual ~CanvasElement() {}
   void Draw();
   void SetHighlight(bool highlight) { mHighlighted = highlight; }
   bool GetHighlighted() const { return mHighlighted; }
   ofRectangle GetRect(bool clamp, bool wrapped) const;
   float GetStart() const;
   void SetStart(float start);
   virtual float GetEnd() const;
   void SetEnd(float end);
   vector<IUIControl*>& GetUIControls() { return mUIControls; }
   
   virtual ofColor GetColor() const { return ofColor::grey; }
   virtual bool IsResizable() const { return true; }
   virtual CanvasElement* CreateDuplicate() const = 0;
   
   virtual void CheckboxUpdated(string label, bool value);
   virtual void FloatSliderUpdated(string label, float oldVal, float newVal);
   virtual void IntSliderUpdated(string label, int oldVal, float newVal);
   
   virtual void SaveState(FileStreamOut& out);
   virtual void LoadState(FileStreamIn& in);
   
   int mRow;
   int mCol;
   float mOffset;
   float mLength;
   
protected:
   virtual void DrawContents() = 0;
   void DrawElement(bool wrapped);
   void AddElementUIControl(IUIControl* control);
   
   Canvas* mCanvas;
   bool mHighlighted;
   vector<IUIControl*> mUIControls;
};

class NoteCanvasElement : public CanvasElement
{
public:
   NoteCanvasElement(Canvas* canvas, int col, int row, float offset, float length);
   static CanvasElement* Create(Canvas* canvas, int col, int row) { return new NoteCanvasElement(canvas,col,row,0,1); }
   void SetVelocity(float vel) { mVelocity = vel; }
   float GetVelocity() const { return mVelocity; }
   void SetVoiceIdx(int voiceIdx) { mVoiceIdx = voiceIdx; }
   int GetVoiceIdx() const { return mVoiceIdx; }
   ModulationChain* GetPitchBend() { return &mPitchBend; }
   ModulationChain* GetModWheel() { return &mModWheel; }
   ModulationChain* GetPressure() { return &mPressure; }
   float GetPan() { return mPan; }
   void UpdateModulation(float pos);
   void WriteModulation(float pos, float pitchBend, float modWheel, float pressure, float pan);
   
   CanvasElement* CreateDuplicate() const override;
   
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
private:
   void DrawContents() override;
   
   float mVelocity;
   FloatSlider* mElementOffsetSlider;
   FloatSlider* mElementLengthSlider;
   IntSlider* mElementRowSlider;
   IntSlider* mElementColSlider;
   FloatSlider* mVelocitySlider;
   int mVoiceIdx;
   ModulationChain mPitchBend;
   ModulationChain mModWheel;
   ModulationChain mPressure;
   float mPan;
   Curve mPitchBendCurve;
   Curve mModWheelCurve;
   Curve mPressureCurve;
   Curve mPanCurve;
};

class SampleCanvasElement : public CanvasElement
{
public:
   SampleCanvasElement(Canvas* canvas, int col, int row, float offset, float length);
   ~SampleCanvasElement();
   static CanvasElement* Create(Canvas* canvas, int col, int row) { return new SampleCanvasElement(canvas,col,row,0,1); }
   void SetSample(Sample* sample);
   Sample* GetSample() const { return mSample; }
   void SetNumLoops(int numLoops) { mNumLoops = numLoops; }
   int GetNumLoops() const { return mNumLoops; }
   float GetVolume() const { return mVolume; }
   bool ShouldMeasureSync() const { return mMeasureSync; }
   
   CanvasElement* CreateDuplicate() const override;
   
   void CheckboxUpdated(string label, bool value) override;
   
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
private:
   void DrawContents() override;
   
   Sample* mSample;
   int mNumLoops;
   int mNumBars;
   IntSlider* mNumLoopsSlider;
   TextEntry* mNumBarsEntry;
   FloatSlider* mElementOffsetSlider;
   float mVolume;
   FloatSlider* mVolumeSlider;
   bool mMeasureSync;
   Checkbox* mMeasureSyncCheckbox;
};

class EventCanvasElement : public CanvasElement
{
public:
   EventCanvasElement(Canvas* canvas, int col, int row, float offset);
   ~EventCanvasElement();
   static CanvasElement* Create(Canvas* canvas, int col, int row) { return new EventCanvasElement(canvas,col,row,0); }
   
   CanvasElement* CreateDuplicate() const override;
   
   void SetUIControl(IUIControl* control);
   void SetValue(float value) { mValue = value; }
   void Trigger();
   void TriggerEnd();
   
   ofColor GetColor() const override;
   bool IsResizable() const override { return mIsCheckbox; }
   float GetEnd() const override;
   
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
private:
   void DrawContents() override;
   
   IUIControl* mUIControl;
   float mValue;
   TextEntry* mValueEntry;
   EventCanvas* mEventCanvas;
   bool mIsCheckbox;
};

#endif /* defined(__Bespoke__CanvasElement__) */
