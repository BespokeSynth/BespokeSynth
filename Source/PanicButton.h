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
//  PanicButton.h
//  Bespoke
//
//  Created by Ryan Challinor on 3/31/14.
//
//

#ifndef __Bespoke__PanicButton__
#define __Bespoke__PanicButton__

#include <iostream>
#include "IDrawableModule.h"
#include "OpenFrameworksPort.h"

class PanicButton : public IDrawableModule
{
public:
   PanicButton();
   ~PanicButton();
   static IDrawableModule* Create() { return new PanicButton(); }

private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 300;
      height = 150;
   }
   void OnClicked(float x, float y, bool right) override;
};

#endif /* defined(__Bespoke__PanicButton__) */
