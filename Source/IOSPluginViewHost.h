/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2026 Ryan Challinor (contact: awwbees@gmail.com)

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
//  IOSPluginViewHost.h
//  Bespoke
//
//  Shows an AUv3's own UI inside a module on the canvas.
//  An AUv3's view lives in the plugin's process, so it can't be drawn into the canvas. Instead the view stays a real
//  UIView, layered over the canvas and moved every frame to sit inside the module, so its controls stay touchable.
//

#pragma once

#include <functional>

namespace IOSPluginViewHost
{
   struct Overlay;

   //receives the plugin view's natural size in points, on the main thread, once the plugin has delivered its view
   using ReadyHandler = std::function<void(float width, float height)>;

   //asks the AudioUnit (an AudioUnit handle, from AudioPluginInstance::getPlatformSpecificData()) for its view.
   //returns nullptr if it doesn't offer one
   Overlay* Create(void* audioUnit, ReadyHandler onReady);

   //removes the view from the screen and frees the overlay. safe to call before the view has arrived
   void Destroy(Overlay* overlay);

   //places the view over the canvas. parentView is the canvas's UIView; x/y/width/height are where the view goes, in its
   //points. nothing is drawn above clipTop, so the view stays under the title bar. resizing the plugin's view is slow
   //(it lives in another process), so avoid calling this every frame while the canvas is moving: pass visible=false then
   void Update(Overlay* overlay, void* parentView, float clipTop, float x, float y, float width, float height, bool visible);
}
