//
//  CanvasControls.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/30/14.
//
//

#include "CanvasControls.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "Profiler.h"
#include "ModularSynth.h"
#include "Canvas.h"
#include "CanvasElement.h"

CanvasControls::CanvasControls()
: mDummyFloat(0)
, mDummyInt(0)
, mCanvas(nullptr)
, mSelectedElement(nullptr)
, mRemoveElementButton(nullptr)
, mNumVisibleRowsEntry(nullptr)
, mClearButton(nullptr)
{
}

CanvasControls::~CanvasControls()
{
}

void CanvasControls::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mRemoveElementButton = new ClickButton(this,"delete",5,1);
   mNumVisibleRowsEntry = new TextEntry(this,"view rows",175,1,3,&mCanvas->mNumVisibleRows,1,9999);
   mClearButton = new ClickButton(this, "clear", 270, 1);
   mDragModeSelector = new DropdownList(this, "drag mode", 310, 1, (int*)(&mCanvas->mDragMode));
   
   mDragModeSelector->AddLabel("drag both", Canvas::kDragBoth);
   mDragModeSelector->AddLabel("horizontal", Canvas::kDragHorizontal);
   mDragModeSelector->AddLabel("vertical", Canvas::kDragVertical);

   mNumVisibleRowsEntry->DrawLabel(true);
   
   SetElement(nullptr);
}

void CanvasControls::SetCanvas(Canvas* canvas)
{
   mCanvas = canvas;
   mWidth = mCanvas->GetWidth();
   mCanvas->SetControls(this);
   StringCopy(NameMutable(), (string(mCanvas->Name())+"_controls").c_str(), MAX_TEXTENTRY_LENGTH);
}

void CanvasControls::SetElement(CanvasElement* element)
{
   if (mSelectedElement)
   {
      for (auto* control : mSelectedElement->GetUIControls())
         control->SetShowing(false);
   }
   
   mSelectedElement = element;
   
   if (mSelectedElement)
   {
      int idx = 0;
      for (auto* control : mSelectedElement->GetUIControls())
      {
         control->SetShowing(true);
         control->SetPosition(5+(idx/4)*110, 20+(idx%4)*18);
         ++idx;
      }
   }
}

void CanvasControls::AllowDragModeSelection(bool allow)
{
   mDragModeSelector->SetShowing(allow);
}

void CanvasControls::PreDrawModule()
{
   float x,y;
   mCanvas->GetPosition(x, y, K(localOnly));
   SetPosition(x,y+13+mCanvas->GetHeight());
}

void CanvasControls::DrawModule()
{
   ofPushStyle();
   ofFill();
   ofSetColor(100, 100, 100);
   ofRect(0, 0, mWidth, 19);
   ofPopStyle();
   mRemoveElementButton->Draw();
   mNumVisibleRowsEntry->Draw();
   mClearButton->Draw();
   mDragModeSelector->Draw();
   if (mSelectedElement)
   {
      for (auto* control : mSelectedElement->GetUIControls())
         control->Draw();
   }
}

void CanvasControls::GetModuleDimensions(float& width, float& height)
{
   width = mWidth;
   height = 92;
}

void CanvasControls::CheckboxUpdated(Checkbox* checkbox)
{
   for (auto* element : mCanvas->GetElements())
   {
      if (element->GetHighlighted())
         element->CheckboxUpdated(checkbox->Name(), checkbox->GetValue() > 0);
   }
}

void CanvasControls::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   for (auto* element : mCanvas->GetElements())
   {
      if (element->GetHighlighted())
         element->FloatSliderUpdated(slider->Name(), oldVal, slider->GetValue());
   }
}

void CanvasControls::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   for (auto* element : mCanvas->GetElements())
   {
      if (element->GetHighlighted())
         element->IntSliderUpdated(slider->Name(), oldVal, slider->GetValue());
   }
}

void CanvasControls::TextEntryComplete(TextEntry* entry)
{
   if (entry == mNumVisibleRowsEntry)
   {
      if (mCanvas->mNumVisibleRows > mCanvas->GetNumRows())
         mCanvas->mNumVisibleRows = mCanvas->GetNumRows();
   }
}

void CanvasControls::ButtonClicked(ClickButton* button)
{
   if (button == mRemoveElementButton)
   {
      mSelectedElement = nullptr;
      vector<CanvasElement*> elementsToDelete;
      for (auto* element : mCanvas->GetElements())
      {
         if (element->GetHighlighted())
            elementsToDelete.push_back(element);
      }
      for (auto* element : elementsToDelete)
      {
         for (auto* control : element->GetUIControls())
         {
            control->SetShowing(false);
            RemoveUIControl(control);
            control->Delete();
         }
         mCanvas->RemoveElement(element);
      }
   }
   if (button == mClearButton)
   {
      mCanvas->Clear();
   }

   std::vector<CanvasElement*> elements = mCanvas->GetElements(); //make a copy of the list, since I may modify the list through the actions below
   for (auto* element : elements)
   {
      if (element->GetHighlighted())
         element->ButtonClicked(button->Name());
   }
}

void CanvasControls::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void CanvasControls::SetUpFromSaveData()
{
}

