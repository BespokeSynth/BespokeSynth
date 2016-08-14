//
//  FillSaveDropdown.h
//  Bespoke
//
//  Created by Ryan Challinor on 2/23/15.
//
//

#ifndef Bespoke_FillSaveDropdown_h
#define Bespoke_FillSaveDropdown_h

#include "ModularSynth.h"
#include "DropdownList.h"

template <class T> void FillDropdown(DropdownList* list)
{
   assert(list);
   vector<string> modules = TheSynth->GetModuleNames<T>();
   list->AddLabel("",-1);
   for (int i=0; i<modules.size(); ++i)
      list->AddLabel(modules[i].c_str(), i);
}

#endif
