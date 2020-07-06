//
//  CanvasControls.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/30/14.
//
//

#ifndef __Bespoke__CanvasControls__
#define __Bespoke__CanvasControls__

#include "IDrawableModule.h"
#include "TextEntry.h"
#include "ClickButton.h"
#include "Slider.h"
#include "DropdownList.h"

class Canvas;
class CanvasElement;

class CanvasControls : public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public IButtonListener, public ITextEntryListener, public IDropdownListener
{
public:
   CanvasControls();
   ~CanvasControls();
   static IDrawableModule* Create() { return new CanvasControls(); }
   
   string GetTitleLabel() override { return ""; }
   void CreateUIControls() override;
   bool HasTitleBar() const override { return false; }
   bool CanMinimize() override { return false; }
   
   void SetCanvas(Canvas* canvas);
   void SetElement(CanvasElement* element);
   
   void AllowDragModeSelection(bool allow);
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void ButtonClicked(ClickButton* button) override;
   void TextEntryComplete(TextEntry* entry) override {}
   void DropdownUpdated(DropdownList* list, int oldVal) override {}
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   bool CanSaveState() const override { return false; }
private:
   //IDrawableModule
   void PreDrawModule() override;
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return true; }
   
   float mWidth;
   Canvas* mCanvas;
   CanvasElement* mSelectedElement;
   ClickButton* mAddElementButton;
   ClickButton* mRemoveElementButton;
   TextEntry* mCanvasLengthEntry;
   TextEntry* mNumVisibleRowsEntry;
   ClickButton* mClearButton;
   float mDummyFloat;
   int mDummyInt;
   DropdownList* mDragModeSelector;
};

#endif /* defined(__Bespoke__CanvasControls__) */
