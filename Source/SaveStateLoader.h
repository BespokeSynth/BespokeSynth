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
//  SaveStateLoader.h
//  Bespoke
//
//  Created by Ryan Challinor on 7/23/24.
//
//

#pragma once

#include <utility>
#include "IDrawableModule.h"
#include "OpenFrameworksPort.h"
#include "ClickButton.h"

class SaveStateLoader : public IDrawableModule, public IButtonListener
{
public:
   SaveStateLoader();
   virtual ~SaveStateLoader();
   static IDrawableModule* Create() { return new SaveStateLoader(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Poll() override;

   void ButtonClicked(ClickButton* button, double time) override;

   bool IsEnabled() const override { return true; }

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 1; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;

   bool HasValidFile(int index) const;
   void UpdateButtonLabel(int index);

   struct LoadButton
   {
      std::string mFilePath{ "" };
      ClickButton* mButton{ nullptr };
      bool mShouldShowSelectDialog{ false };
   };

   std::array<LoadButton, 30> mLoadButtons;
   int mNumDisplayButtons{ 1 };
};
