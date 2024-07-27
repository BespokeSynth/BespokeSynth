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
//  CanvasControls.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/30/14.
//
//

#include "CanvasControls.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Canvas.h"
#include "CanvasElement.h"

CanvasControls::CanvasControls()
{
}

CanvasControls::~CanvasControls()
{
}

void CanvasControls::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mRemoveElementButton = new ClickButton(this, "delete", 5, 1);
   mNumVisibleRowsEntry = new TextEntry(this, "view rows", 175, 1, 3, &mCanvas->mNumVisibleRows, 1, 9999);
   mClearButton = new ClickButton(this, "clear", 270, 1);
   mDragModeSelector = new DropdownList(this, "drag mode", 310, 1, (int*)(&mCanvas->mDragMode));

   mDragModeSelector->AddLabel("drag both", Canvas::kDragBoth);
   mDragModeSelector->AddLabel("horizontal", Canvas::kDragHorizontal);
   mDragModeSelector->AddLabel("vertical", Canvas::kDragVertical);

   mNumVisibleRowsEntry->DrawLabel(true);

   // Block modulator cables from connecting to these controls.
   mRemoveElementButton->SetCableTargetable(false);
   mNumVisibleRowsEntry->SetCableTargetable(false);
   mClearButton->SetCableTargetable(false);
   mDragModeSelector->SetCableTargetable(false);
   mNumVisibleRowsEntry->SetCableTargetable(false);

   SetElement(nullptr);
}

void CanvasControls::SetCanvas(Canvas* canvas)
{
   mCanvas = canvas;
   mWidth = mCanvas->GetWidth();
   mCanvas->SetControls(this);
   StringCopy(NameMutable(), (std::string(mCanvas->Name()) + "_controls").c_str(), MAX_TEXTENTRY_LENGTH);
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
         control->SetPosition(5 + (idx / 4) * 110, 20 + (idx % 4) * 18);
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
   float x, y;
   mCanvas->GetPosition(x, y, K(localOnly));
   SetPosition(x, y + 13 + mCanvas->GetHeight());
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

void CanvasControls::CheckboxUpdated(Checkbox* checkbox, double time)
{
   for (auto* element : mCanvas->GetElements())
   {
      if (element->GetHighlighted())
         element->CheckboxUpdated(checkbox->Name(), checkbox->GetValue() > 0, time);
   }
}

void CanvasControls::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   for (auto* element : mCanvas->GetElements())
   {
      if (element->GetHighlighted())
         element->FloatSliderUpdated(slider->Name(), oldVal, slider->GetValue(), time);
   }
}

void CanvasControls::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   for (auto* element : mCanvas->GetElements())
   {
      if (element->GetHighlighted())
         element->IntSliderUpdated(slider->Name(), oldVal, slider->GetValue(), time);
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

void CanvasControls::ButtonClicked(ClickButton* button, double time)
{
   if (button == mRemoveElementButton)
   {
      mSelectedElement = nullptr;
      std::vector<CanvasElement*> elementsToDelete;
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
         element->ButtonClicked(button->Name(), time);
   }
}

void CanvasControls::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void CanvasControls::SetUpFromSaveData()
{
}
