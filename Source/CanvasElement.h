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
//  CanvasElement.h
//  Bespoke
//
//  Created by Ryan Challinor on 1/4/15.
//
//

#pragma once

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
   void Draw(ofVec2f offset);
   void DrawOffscreen();
   void SetHighlight(bool highlight) { mHighlighted = highlight; }
   bool GetHighlighted() const { return mHighlighted; }
   ofRectangle GetRect(bool clamp, bool wrapped, ofVec2f offset = ofVec2f(0, 0)) const;
   float GetStart() const;
   void SetStart(float start, bool preserveLength);
   virtual float GetEnd() const;
   void SetEnd(float end);
   std::vector<IUIControl*>& GetUIControls() { return mUIControls; }
   void MoveElementByDrag(ofVec2f dragOffset);

   virtual bool IsResizable() const { return true; }
   virtual CanvasElement* CreateDuplicate() const = 0;

   virtual void CheckboxUpdated(std::string label, bool value, double time);
   virtual void FloatSliderUpdated(std::string label, float oldVal, float newVal, double time);
   virtual void IntSliderUpdated(std::string label, int oldVal, float newVal, double time);
   virtual void ButtonClicked(std::string label, double time);

   virtual void SaveState(FileStreamOut& out);
   virtual void LoadState(FileStreamIn& in);

   int mRow;
   int mCol;
   float mOffset;
   float mLength;

protected:
   virtual void DrawContents(bool clamp, bool wrapped, ofVec2f offset) = 0;
   void DrawElement(bool clamp, bool wrapped, ofVec2f offset);
   void AddElementUIControl(IUIControl* control);
   void GetDragDestinationData(ofVec2f dragOffset, int& newRow, int& newCol, float& newOffset) const;
   ofRectangle GetRectAtDestination(bool clamp, bool wrapped, ofVec2f dragOffset) const;
   float GetStart(int col, float offset) const;
   float GetEnd(int col, float offset, float length) const;

   Canvas* mCanvas{ nullptr };
   bool mHighlighted{ false };
   std::vector<IUIControl*> mUIControls;
};

class NoteCanvasElement : public CanvasElement
{
public:
   NoteCanvasElement(Canvas* canvas, int col, int row, float offset, float length);
   static CanvasElement* Create(Canvas* canvas, int col, int row) { return new NoteCanvasElement(canvas, col, row, 0, 1); }
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
   void DrawContents(bool clamp, bool wrapped, ofVec2f offset) override;

   float mVelocity{ .8 };
   FloatSlider* mElementOffsetSlider{ nullptr };
   FloatSlider* mElementLengthSlider{ nullptr };
   IntSlider* mElementRowSlider{ nullptr };
   IntSlider* mElementColSlider{ nullptr };
   FloatSlider* mVelocitySlider{ nullptr };
   int mVoiceIdx{ -1 };
   ModulationChain mPitchBend{ ModulationParameters::kDefaultPitchBend };
   ModulationChain mModWheel{ ModulationParameters::kDefaultModWheel };
   ModulationChain mPressure{ ModulationParameters::kDefaultPressure };
   float mPan{ 0 };
   Curve mPitchBendCurve{ ModulationParameters::kDefaultPitchBend };
   Curve mModWheelCurve{ ModulationParameters::kDefaultModWheel };
   Curve mPressureCurve{ ModulationParameters::kDefaultPressure };
   Curve mPanCurve{ 0 };
};

class SampleCanvasElement : public CanvasElement
{
public:
   SampleCanvasElement(Canvas* canvas, int col, int row, float offset, float length);
   ~SampleCanvasElement();
   static CanvasElement* Create(Canvas* canvas, int col, int row) { return new SampleCanvasElement(canvas, col, row, 0, 1); }
   void SetSample(Sample* sample);
   Sample* GetSample() const { return mSample; }
   float GetVolume() const { return mVolume; }
   bool IsMuted() const { return mMute; }

   CanvasElement* CreateDuplicate() const override;

   void CheckboxUpdated(std::string label, bool value, double time) override;
   void ButtonClicked(std::string label, double time) override;

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;

private:
   void DrawContents(bool clamp, bool wrapped, ofVec2f offset) override;

   Sample* mSample{ nullptr };
   FloatSlider* mElementOffsetSlider{ nullptr };
   float mVolume{ 1 };
   FloatSlider* mVolumeSlider{ nullptr };
   bool mMute{ false };
   Checkbox* mMuteCheckbox{ nullptr };
   ClickButton* mSplitSampleButton{ nullptr };
   ClickButton* mResetSpeedButton{ nullptr };
};

class EventCanvasElement : public CanvasElement
{
public:
   EventCanvasElement(Canvas* canvas, int col, int row, float offset);
   ~EventCanvasElement();
   static CanvasElement* Create(Canvas* canvas, int col, int row) { return new EventCanvasElement(canvas, col, row, 0); }

   CanvasElement* CreateDuplicate() const override;

   void SetUIControl(IUIControl* control);
   void SetValue(float value) { mValue = value; }
   void Trigger(double time);
   void TriggerEnd(double time);

   bool IsResizable() const override { return mIsCheckbox; }
   float GetEnd() const override;

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;

private:
   void DrawContents(bool clamp, bool wrapped, ofVec2f offset) override;

   IUIControl* mUIControl{ nullptr };
   float mValue{ 0 };
   TextEntry* mValueEntry{ nullptr };
   EventCanvas* mEventCanvas{ nullptr };
   bool mIsCheckbox{ false };
   bool mIsButton{ false };
};
