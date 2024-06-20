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
//  FillSaveDropdown.h
//  Bespoke
//
//  Created by Ryan Challinor on 2/23/15.
//
//

#pragma once

#include "ModularSynth.h"
#include "DropdownList.h"

template <class T>
void FillDropdown(DropdownList* list)
{
   assert(list);
   std::vector<std::string> modules = TheSynth->GetModuleNames<T>();
   list->AddLabel("", -1);
   for (int i = 0; i < modules.size(); ++i)
      list->AddLabel(modules[i].c_str(), i);
}
