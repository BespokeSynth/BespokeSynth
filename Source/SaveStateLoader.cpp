/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2024 Ryan Challinor (contact: awwbees@gmail.com)

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
//  SaveStateLoader.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 7/23/24.
//
//

#include "SaveStateLoader.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"

SaveStateLoader::SaveStateLoader()
{
}

SaveStateLoader::~SaveStateLoader()
{
}

void SaveStateLoader::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   for (size_t i = 0; i < mLoadButtons.size(); ++i)
   {
      BUTTON(mLoadButtons[i].mButton, ("load " + ofToString(i)).c_str());
      mLoadButtons[i].mButton->SetOverrideDisplayName("<select file>");
      mLoadButtons[i].mButton->UpdateWidth();
   }
   ENDUIBLOCK0();
}

void SaveStateLoader::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   for (size_t i = 0; i < mLoadButtons.size(); ++i)
   {
      mLoadButtons[i].mButton->SetShowing(i < mNumDisplayButtons);
      mLoadButtons[i].mButton->Draw();
   }
}

void SaveStateLoader::Poll()
{
   for (int i = 0; i < (int)mLoadButtons.size(); ++i)
   {
      if (mLoadButtons[i].mShouldShowSelectDialog)
      {
         mLoadButtons[i].mShouldShowSelectDialog = false;

         juce::FileChooser chooser("Select state file", juce::File(ofToDataPath("savestate")), "*.bsk;*.bskt", true, false, TheSynth->GetFileChooserParent());
         if (chooser.browseForFileToOpen())
         {
            mLoadButtons[i].mFilePath = chooser.getResult().getFullPathName().replace("\\", "/").toStdString();
            UpdateButtonLabel(i);
         }
      }
   }
}

void SaveStateLoader::GetModuleDimensions(float& w, float& h)
{
   w = 100;
   for (size_t i = 0; i < mLoadButtons.size(); ++i)
   {
      if (i < mNumDisplayButtons)
      {
         float buttonW, buttonH;
         mLoadButtons[i].mButton->GetDimensions(buttonW, buttonH);
         if (buttonW + 6 > w)
            w = buttonW + 6;
      }
   }
   h = 3 + 17 * mNumDisplayButtons;
}

bool SaveStateLoader::HasValidFile(int index) const
{
   return juce::String(mLoadButtons[index].mFilePath).endsWith(".bsk") || juce::String(mLoadButtons[index].mFilePath).endsWith(".bskt");
}

void SaveStateLoader::ButtonClicked(ClickButton* button, double time)
{
   for (int i = 0; i < (int)mLoadButtons.size(); ++i)
   {
      if (button == mLoadButtons[i].mButton)
      {
         if (HasValidFile(i) && !(GetKeyModifiers() & kModifier_Shift))
            TheSynth->LoadState(mLoadButtons[i].mFilePath);
         else
            mLoadButtons[i].mShouldShowSelectDialog = true;
      }
   }
}

void SaveStateLoader::UpdateButtonLabel(int index)
{
   if (HasValidFile(index))
   {
      juce::File file(mLoadButtons[index].mFilePath);
      mLoadButtons[index].mButton->SetOverrideDisplayName(("load " + file.getFileName()).toRawUTF8());
      mLoadButtons[index].mButton->UpdateWidth();
   }
}

void SaveStateLoader::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadInt("num_buttons", moduleInfo, 1, 1, (int)mLoadButtons.size(), true);

   SetUpFromSaveData();
}

void SaveStateLoader::SetUpFromSaveData()
{
   mNumDisplayButtons = mModuleSaveData.GetInt("num_buttons");
}

void SaveStateLoader::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << (int)mLoadButtons.size();
   for (int i = 0; i < (int)mLoadButtons.size(); ++i)
      out << mLoadButtons[i].mFilePath;
}

void SaveStateLoader::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   int count;
   in >> count;
   for (int i = 0; i < (int)mLoadButtons.size() && i < count; ++i)
   {
      in >> mLoadButtons[i].mFilePath;
      UpdateButtonLabel(i);
   }
}
