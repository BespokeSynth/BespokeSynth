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
//  IOSSystemGestures.mm
//  Bespoke
//

#include "IOSSystemGestures.h"

#import <UIKit/UIKit.h>
#import <objc/runtime.h>

namespace
{
   bool sInstalled = false;
   CGSize sRestrictedSize = CGSizeZero;

   UIRectEdge DeferredEdges(id, SEL)
   {
      return UIRectEdgeAll;
   }
}

namespace
{
   //in ipad's windowed and stage manager modes, stops the window from being shrunk below the full screen.
   //the screen's bounds swap when the ipad rotates, so this keeps the restriction matched to them
   void RestrictWindowToFullScreen(UIWindow* window)
   {
      UIWindowScene* scene = window.windowScene;
      if (scene == nil || scene.sizeRestrictions == nil)
         return;

      CGSize screenSize = scene.screen.bounds.size;
      if (CGSizeEqualToSize(screenSize, sRestrictedSize))
         return;

      scene.sizeRestrictions.minimumSize = screenSize;
      sRestrictedSize = screenSize;
   }
}

void IOSSystemGestures::Install(void* nativeView)
{
   if (nativeView == nullptr)
      return;

   UIWindow* window = ((__bridge UIView*)nativeView).window;
   UIViewController* controller = window.rootViewController;
   if (controller == nil)
      return; //not on screen yet

   RestrictWindowToFullScreen(window);

   if (sInstalled)
      return;

   Class controllerClass = [controller class];
   class_replaceMethod(controllerClass, @selector(preferredScreenEdgesDeferringSystemGestures), (IMP)DeferredEdges, "Q@:");
   //the home indicator is deliberately left visible: while it's auto-hidden, ios ignores the deferral on the bottom edge

   [controller setNeedsUpdateOfScreenEdgesDeferringSystemGestures];

   //shaking the device would otherwise pop up the system's "undo typing" alert while a text field has focus
   [UIApplication sharedApplication].applicationSupportsShakeToEdit = NO;

   sInstalled = true;
}
