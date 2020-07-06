//
//  NoteCanvas.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/30/14.
//
//

#ifndef __Bespoke__NoteCanvas__
#define __Bespoke__NoteCanvas__

#include "IDrawableModule.h"
#include "INoteSource.h"
#include "Transport.h"
#include "Checkbox.h"
#include "Canvas.h"
#include "Slider.h"
#include "INoteReceiver.h"
#include "ClickButton.h"
#include "DropdownList.h"

class CanvasControls;

class NoteCanvas : public IDrawableModule, public INoteSource, public ICanvasListener, public IFloatSliderListener, public IAudioPoller, public IIntSliderListener, public INoteReceiver, public IButtonListener, public IDropdownListener
{
public:
   NoteCanvas();
   ~NoteCanvas();
   static IDrawableModule* Create() { return new NoteCanvas(); }
   
   string GetTitleLabel() override { return "note canvas"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   bool MouseScrolled(int x, int y, float scrollX, float scrollY) override;
   void KeyPressed(int key, bool isRepeat) override;
   
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}
   
   void Clear();
   NoteCanvasElement* AddNote(double measurePos, int pitch, int velocity, double length, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters());
   
   void OnTransportAdvanced(float amount) override;
   
   void CanvasUpdated(Canvas* canvas) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void ButtonClicked(ClickButton* button) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return mEnabled; }
   
   double GetCurPos(double time) const;
   void UpdateNumColumns();
   void SetRecording(bool rec);
   void SetNumMeasures(int numMeasures);
   bool FreeRecordParityMatched();
   void ClipNotes();
   void QuantizeNotes();
   
   Canvas* mCanvas;
   CanvasControls* mCanvasControls;
   float mScrollPartial;
   vector<CanvasElement*> mNoteChecker{128};
   vector<NoteCanvasElement*> mInputNotes{128};
   vector<NoteCanvasElement*> mCurrentNotes{128};
   IntSlider* mNumMeasuresSlider;
   int mNumMeasures;
   ClickButton* mQuantizeButton;
   ClickButton* mClipButton;
   bool mPlay;
   Checkbox* mPlayCheckbox;
   bool mRecord;
   Checkbox* mRecordCheckbox;
   bool mStopQueued;
   NoteInterval mInterval;
   DropdownList* mIntervalSelector;
   bool mFreeRecord;
   Checkbox* mFreeRecordCheckbox;
   int mFreeRecordStartMeasure;
   
   vector<ModulationParameters> mVoiceModulations;
};

#endif /* defined(__Bespoke__NoteCanvas__) */
