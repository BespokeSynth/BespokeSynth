/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2022 Ryan Challinor (contact: awwbees@gmail.com)

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
//
//  IOSSystemGestures.h
//  Bespoke
//
//  Keeps the screen edges for bespoke: swipes in from any edge (home, app switching, notification center,
//  control center) need a second swipe before the system acts on them. JUCE's view controller doesn't
//  expose this setting, so the method is added to it at runtime.
//  Also turns off shake to undo, and on ipad stops the window from being shrunk below full screen.
//

#pragma once

namespace IOSSystemGestures
{
   //call with the main window's UIView once it's on screen. safe to call repeatedly
   void Install(void* nativeView);
}
