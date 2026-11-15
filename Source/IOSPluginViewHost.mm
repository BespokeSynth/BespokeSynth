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
//  IOSPluginViewHost.mm
//  Bespoke
//

#include "IOSPluginViewHost.h"

#include <algorithm>
#include <memory>

#import <UIKit/UIKit.h>
#import <AudioToolbox/AudioToolbox.h>

//holds every embedded plugin view, clipped to the canvas area below the title bar. touches that don't land on a plugin
//view fall through to the canvas
@interface BespokePluginOverlayRoot : UIView
@end

@implementation BespokePluginOverlayRoot
- (UIView*)hitTest:(CGPoint)point withEvent:(UIEvent*)event
{
   UIView* hit = [super hitTest:point withEvent:event];
   return hit == self ? nil : hit;
}
@end

namespace
{
   BespokePluginOverlayRoot* sRoot = nil;

   //the plugin's view controller arrives asynchronously, maybe after the overlay is destroyed, so it lands here
   struct State
   {
      bool mDestroyed{ false };
      UIViewController* mController{ nil };
      UIView* mContainer{ nil }; //clips the plugin view to the module's area
      CGSize mSize{ 0, 0 };
      IOSPluginViewHost::ReadyHandler mOnReady;
   };

   using ViewControllerCallback = void (^)(UIViewController*);

   CGSize GetNaturalSize(UIViewController* controller)
   {
      CGSize size = controller.preferredContentSize;
      if (size.width < 1 || size.height < 1)
         size = controller.view.frame.size;
      if (size.width < 50 || size.height < 50) //no useful size from the plugin
         size = CGSizeMake(480, 320);
      return size;
   }
}

struct IOSPluginViewHost::Overlay
{
   std::shared_ptr<State> mState{ std::make_shared<State>() };
   ViewControllerCallback mCallback{ nil };
};

IOSPluginViewHost::Overlay* IOSPluginViewHost::Create(void* audioUnit, ReadyHandler onReady)
{
   auto unit = static_cast<AudioUnit>(audioUnit);
   if (unit == nullptr)
      return nullptr;

   UInt32 dataSize = 0;
   Boolean isWritable = false;
   if (AudioUnitGetPropertyInfo(unit, kAudioUnitProperty_RequestViewController, kAudioUnitScope_Global, 0, &dataSize, &isWritable) != noErr ||
       dataSize != sizeof(ViewControllerCallback))
      return nullptr;

   auto* overlay = new Overlay();
   overlay->mState->mOnReady = std::move(onReady);

   std::shared_ptr<State> state = overlay->mState;
   overlay->mCallback = [^(UIViewController* controller)
                        {
                           dispatch_async(dispatch_get_main_queue(), ^
                                          {
                                             if (state->mDestroyed || controller == nil || state->mController != nil)
                                                return;

                                             state->mController = [controller retain];
                                             state->mSize = GetNaturalSize(controller);

                                             UIView* container = [[UIView alloc] initWithFrame:CGRectZero];
                                             container.clipsToBounds = YES;
                                             container.hidden = YES; //until the first Update places it
                                             UIView* view = controller.view;
                                             [view removeFromSuperview];
                                             view.hidden = NO; //an AUv3 hands out the same view every time, and juce hides it when a plugin window closes
                                             view.transform = CGAffineTransformIdentity;
                                             view.frame = CGRectMake(0, 0, state->mSize.width, state->mSize.height);
                                             [container addSubview:view];
                                             state->mContainer = container;

                                             if (state->mOnReady)
                                                state->mOnReady((float)state->mSize.width, (float)state->mSize.height);
                                          });
                        } copy];

   if (AudioUnitSetProperty(unit, kAudioUnitProperty_RequestViewController, kAudioUnitScope_Global, 0, &overlay->mCallback, dataSize) != noErr)
   {
      Destroy(overlay);
      return nullptr;
   }

   return overlay;
}

void IOSPluginViewHost::Destroy(Overlay* overlay)
{
   if (overlay == nullptr)
      return;

   auto& state = *overlay->mState;
   state.mDestroyed = true;
   state.mOnReady = nullptr;
   if (state.mController != nil)
   {
      [state.mController.view removeFromSuperview];
      [state.mController release];
      state.mController = nil;
   }
   if (state.mContainer != nil)
   {
      [state.mContainer removeFromSuperview];
      [state.mContainer release];
      state.mContainer = nil;
   }
   [overlay->mCallback release];
   delete overlay;
}

void IOSPluginViewHost::Update(Overlay* overlay, void* parentView, float clipTop, float x, float y, float width, float height, bool visible)
{
   if (overlay == nullptr || parentView == nullptr)
      return;
   auto& state = *overlay->mState;
   if (state.mContainer == nil)
      return; //view hasn't arrived yet

   UIView* parent = (UIView*)parentView;
   if (sRoot == nil)
   {
      sRoot = [[BespokePluginOverlayRoot alloc] initWithFrame:CGRectZero];
      sRoot.clipsToBounds = YES;
      sRoot.backgroundColor = [UIColor clearColor];
   }
   if (sRoot.superview != parent)
   {
      [sRoot removeFromSuperview];
      [parent addSubview:sRoot];
   }
   if (parent.subviews.lastObject != sRoot) //stay above the canvas's GL view
      [parent bringSubviewToFront:sRoot];
   CGRect rootFrame = CGRectMake(0, clipTop, parent.bounds.size.width, std::max(0.0, (double)parent.bounds.size.height - clipTop));
   if (!CGRectEqualToRect(sRoot.frame, rootFrame))
      sRoot.frame = rootFrame;

   if (state.mContainer.superview != sRoot)
      [sRoot addSubview:state.mContainer];

   state.mContainer.hidden = !visible;
   if (!visible)
      return;

   CGRect containerFrame = CGRectMake(x, y - clipTop, width, height);
   if (!CGRectEqualToRect(state.mContainer.frame, containerFrame))
      state.mContainer.frame = containerFrame;

   //size the plugin's view by its frame, like other AUv3 hosts do, and let the plugin lay itself out. scaling it with a
   //transform instead double-scales plugins that fit their content to the size they're shown at
   UIView* view = state.mController.view;
   if (view.superview != state.mContainer) //the plugin's one view was taken by a plugin window, so take it back
   {
      [view removeFromSuperview];
      [state.mContainer addSubview:view];
   }
   if (view.hidden)
      view.hidden = NO;
   CGRect viewFrame = CGRectMake(0, 0, containerFrame.size.width, containerFrame.size.height);
   if (!CGRectEqualToRect(view.frame, viewFrame))
      view.frame = viewFrame;
}
