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
//  MultitrackRecorder.h
//  Bespoke
//
//  Created by Ryan Challinor on 5/13/14.
//
//

#ifndef __Bespoke__MultitrackRecorder__
#define __Bespoke__MultitrackRecorder__

#include <iostream>
#include "IAudioReceiver.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "ClickButton.h"
#include "RollingBuffer.h"
#include "Ramp.h"
#include "Checkbox.h"
#include "NamedMutex.h"
#include "ClipArranger.h"

#define RECORD_CHUNK_SIZE 10*gSampleRate
#define MAX_NUM_MEASURES 1000

class MultitrackRecorder : public IDrawableModule, public IFloatSliderListener, public IButtonListener
{
public:
   MultitrackRecorder();
   virtual ~MultitrackRecorder();
   static IDrawableModule* Create() { return new MultitrackRecorder(); }
   
   string GetTitleLabel() override { return "multitrack"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void Poll() override;
   void Process(double time, float* left, float* right, int bufferSize);
   
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;
   
   void FilesDropped(vector<string> files, int x, int y) override;
   
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void ButtonClicked(ClickButton* button) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   static const int NUM_CLIP_ARRANGERS = 4;
   
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return mEnabled; }
   void OnClicked(int x, int y, bool right) override;
   
   struct BufferControls
   {
      BufferControls();
      
      float mVol;
      FloatSlider* mVolSlider;
      bool mMute;
      Checkbox* mMuteCheckbox;
   };
   
   struct RecordBuffer
   {
      RecordBuffer(int length);
      ~RecordBuffer();
      
      float* mLeft;
      float* mRight;
      int mLength;
      BufferControls mControls;
   };
   
   void AddRecordBuffer();
   int GetRecordIdx();
   bool IsRecordingStructure();
   void RecordStructure(int offset);
   void ApplyStructure();
   void ResetAll();
   void FixLengths();
   void DeleteBuffer(int idx);
   void CopyRecordBufferContents(RecordBuffer* dst, RecordBuffer* src);
   
   float MeasureToPos(int measure);
   int PosToMeasure(float pos);
   float MouseXToBufferPos(float mouseX);
   
   
   int mRecordingLength;
   int mPlayhead;
   bool mRecording;
   
   Checkbox* mRecordCheckbox;
   Checkbox* mPlayCheckbox;
   ClickButton* mAddTrackButton;
   ClickButton* mResetPlayheadButton;
   ClickButton* mFixLengthsButton;
   float mBufferWidth;
   float mBufferHeight;
   int mRecordIdx;
   int mMaxRecordedLength;
   int mNumMeasures;
   ClickButton* mUndoRecordButton;
   
   vector<RecordBuffer*> mRecordBuffers;
   RecordBuffer* mUndoBuffer;
   
   float* mMeasurePos;
   struct StructureInfo
   {
      int mSample;
      
      int mScaleRoot;
      string mScaleType;
      int mTimeSigTop;
      int mTimeSigBottom;
      float mTempo;
      float mSwing;
   };
   vector<StructureInfo> mStructureInfoPoints;
   int mActiveStructureIdx;
   
   int mMeasures[MAX_NUM_MEASURES];
   bool mSelecting;
   int mSelectedMeasureStart;
   int mSelectedMeasureEnd;
   int mMergeBufferIdx;
   
   NamedMutex mMutex;
   
   ClipArranger mClipArranger[NUM_CLIP_ARRANGERS];
};

extern MultitrackRecorder* TheMultitrackRecorder;

#endif /* defined(__Bespoke__MultitrackRecorder__) */

