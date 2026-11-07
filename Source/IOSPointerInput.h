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
//  IOSPointerInput.h
//  Bespoke
//
//  Tells mouse and trackpad clicks apart from finger touches on iOS.
//  JUCE reports all of them as touches, so this watches UIKit's touches directly (see IOSPointerInput.mm).
//  Also keeps mouse hover events from disturbing touches that are in progress, and makes trackpad scrolling and
//  pinching behave as they do on a mac.
//

#pragma once

#include <functional>

namespace IOSPointerInput
{
   //receives a trackpad pinch, as a position in the view and the scale change since the previous call
   using MagnifyHandler = std::function<void(float x, float y, float scaleFactor)>;

   //starts watching touches on the supplied UIView. safe to call repeatedly
   void Install(void* nativeView, MagnifyHandler onTrackpadMagnify);

   //whether the most recent press came from a mouse or trackpad. UIKit shows touches to gesture recognizers
   //before the view, so this is already up to date when juce delivers the corresponding mouseDown
   bool WasLastPressFromPointer();
}
