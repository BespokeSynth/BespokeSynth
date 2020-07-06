//
//  EventCanvas.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/28/15.
//
//

#ifndef __Bespoke__EventCanvas__
#define __Bespoke__EventCanvas__

#include "Transport.h"
#include "Checkbox.h"
#include "Canvas.h"
#include "Slider.h"
#include "ClickButton.h"
#include "DropdownList.h"
#include "TextEntry.h"

class CanvasControls;
class PatchCableSource;


class EventCanvas : public IDrawableModule, public ICanvasListener, public IFloatSliderListener, public IAudioPoller, public IIntSliderListener, public IButtonListener, public IDropdownListener, public ITextEntryListener
{
public:
   EventCanvas();
   ~EventCanvas();
   static IDrawableModule* Create() { return new EventCanvas(); }
   
   string GetTitleLabel() override { return "event canvas"; }
   void CreateUIControls() override;
   
   IUIControl* GetUIControlForRow(int row);
   ofColor GetRowColor(int row) const;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   bool MouseScrolled(int x, int y, float scrollX, float scrollY) override;
   
   void OnTransportAdvanced(float amount) override;
   
   void CanvasUpdated(Canvas* canvas) override;
   
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   
   vector<IUIControl*> ControlsToIgnoreInSaveState() const override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void ButtonClicked(ClickButton* button) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void TextEntryComplete(TextEntry* entry) override;
   
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
   
   void UpdateNumColumns();
   void SyncControlCablesToCanvas();
   
   Canvas* mCanvas;
   CanvasControls* mCanvasControls;
   float mScrollPartial;
   TextEntry* mNumMeasuresEntry;
   int mNumMeasures;
   ClickButton* mQuantizeButton;
   NoteInterval mInterval;
   DropdownList* mIntervalSelector;
   float mPosition;
   FloatSlider* mPositionSlider;
   vector<PatchCableSource*> mControlCables;
   vector<ofColor> mRowColors;
   bool mRecord;
   Checkbox* mRecordCheckbox;
   float mPreviousPosition;
   
   struct ControlConnection
   {
      IUIControl* mUIControl;
      float mLastValue;
   };
   
   const int kMaxEventRows = 256;
   vector<ControlConnection> mRowConnections;
};

#endif /* defined(__Bespoke__EventCanvas__) */
