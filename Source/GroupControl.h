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
//  GroupControl.h
//  Bespoke
//
//  Created by Ryan Challinor on 2/7/16.
//
//

#ifndef __Bespoke__GroupControl__
#define __Bespoke__GroupControl__

#include "IDrawableModule.h"
#include "Checkbox.h"

class PatchCableSource;

class GroupControl : public IDrawableModule
{
public:
   GroupControl();
   ~GroupControl();
   static IDrawableModule* Create() { return new GroupControl(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;

   Checkbox* mGroupCheckbox{ nullptr };
   bool mGroupEnabled{ false };

   std::vector<PatchCableSource*> mControlCables;
};

#endif /* defined(__Bespoke__GroupControl__) */
