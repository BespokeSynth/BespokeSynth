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
      UIView* mScaler{ nil }; //inside mContainer, holds the plugin view, scaled down when the module is smaller than the plugin's natural size
      UIViewController* mParentController{ nil }; //the view controller the plugin's controller is a child of, not retained
      CGSize mSize{ 0, 0 };
      IOSPluginViewHost::ReadyHandler mOnReady;
   };

   using ViewControllerCallback = void (^)(UIViewController*);

   UIViewController* FindViewController(UIView* view)
   {
      for (UIResponder* responder = view; responder != nil; responder = responder.nextResponder)
      {
         if ([responder isKindOfClass:[UIViewController class]])
            return (UIViewController*)responder;
      }
      return view.window.rootViewController;
   }

   //makes the plugin's view controller a child of the canvas's, so it gets trait and appearance updates like any view
   //controller shown inside an app
   void AdoptController(State& state, UIView* parentView)
   {
      if (state.mParentController != nil)
         return;
      UIViewController* parentController = FindViewController(parentView);
      if (parentController == nil || state.mController.parentViewController != nil)
         return;
      [parentController addChildViewController:state.mController];
      [state.mController didMoveToParentViewController:parentController];
      state.mParentController = parentController;
   }

   void ReleaseController(State& state)
   {
      if (state.mParentController == nil)
         return;
      [state.mController willMoveToParentViewController:nil];
      [state.mController removeFromParentViewController];
      state.mParentController = nil;
   }

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
                                             UIView* scaler = [[UIView alloc] initWithFrame:CGRectZero];
                                             [scaler addSubview:view];
                                             [container addSubview:scaler];
                                             state->mContainer = container;
                                             state->mScaler = scaler;

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
      ReleaseController(state);
      [state.mController.view removeFromSuperview];
      [state.mController release];
      state.mController = nil;
   }
   if (state.mScaler != nil)
   {
      [state.mScaler release];
      state.mScaler = nil;
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
   AdoptController(state, parent);

   if (width < 1 || height < 1)
      visible = false;
   state.mContainer.hidden = !visible;
   if (!visible)
      return;

   CGRect containerFrame = CGRectMake(x, y - clipTop, width, height);
   if (!CGRectEqualToRect(state.mContainer.frame, containerFrame))
      state.mContainer.frame = containerFrame;

   //the plugin lays itself out to its view's frame. below its natural size, many plugins don't shrink everything (some
   //keep their controls at a fixed size, so they overlap), so the view is laid out at least as big as the natural size
   //and the scaler shrinks it to fit. the transform is on the scaler, never the plugin's own view: the plugin sees only
   //its frame, so a plugin that fits its content to its frame isn't scaled twice
   const CGSize natural = state.mSize;
   const CGFloat scale = std::min({ (CGFloat)1, width / natural.width, height / natural.height });
   const CGRect layoutBounds = CGRectMake(0, 0, width / scale, height / scale);

   UIView* scaler = state.mScaler;
   const CGAffineTransform transform = CGAffineTransformMakeScale(scale, scale);
   if (!CGRectEqualToRect(scaler.bounds, layoutBounds))
      scaler.bounds = layoutBounds;
   if (!CGAffineTransformEqualToTransform(scaler.transform, transform))
      scaler.transform = transform;
   const CGPoint center = CGPointMake(width / 2, height / 2);
   if (!CGPointEqualToPoint(scaler.center, center))
      scaler.center = center;

   UIView* view = state.mController.view;
   if (view.superview != scaler) //the plugin's one view was taken by a plugin window, so take it back
   {
      [view removeFromSuperview];
      [scaler addSubview:view];
   }
   if (view.hidden)
      view.hidden = NO;
   if (!CGAffineTransformIsIdentity(view.transform))
      view.transform = CGAffineTransformIdentity;
   if (!CGRectEqualToRect(view.frame, layoutBounds))
      view.frame = layoutBounds;
}
