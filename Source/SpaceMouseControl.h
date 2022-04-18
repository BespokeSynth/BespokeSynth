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
/*
  ==============================================================================

    SpaceMouseControl.h
    Created: 25 Jan 2021 9:12:45pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include <memory>

class ModularSynth;

//==============================================================================
class SpaceMouseMessageWindow
{
public:
   explicit SpaceMouseMessageWindow(ModularSynth& theSynth);
   ~SpaceMouseMessageWindow();
   void Poll();

#if BESPOKE_SPACEMOUSE_SUPPORT
private:
   struct Impl;
   const std::unique_ptr<Impl> d;
#endif
};

#if !BESPOKE_SPACEMOUSE_SUPPORT
inline void SpaceMouseMessageWindow::Poll()
{
}
#endif