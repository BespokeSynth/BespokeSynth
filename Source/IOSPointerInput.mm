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
//  IOSPointerInput.mm
//  Bespoke
//

#include "IOSPointerInput.h"

#include <cmath>
#include <utility>

#import <UIKit/UIKit.h>
#import <UIKit/UIGestureRecognizerSubclass.h>

namespace
{
   void* sInstalledView = nullptr;
   bool sLastPressFromPointer = false;
   bool sPressHeld = false;
   SEL sJuceHoverHandler = NSSelectorFromString(@"onHover:");
   SEL sJuceScrollHandler = NSSelectorFromString(@"onScroll:");
   CGPoint sLastScrollTranslation = CGPointZero;
   bool sPinching = false;
   CGFloat sLastPinchScale = 1;
   const float kTrackpadPinchSpeed = 2.0f / 3; //matched by feel to pinching on a macbook trackpad
   IOSPointerInput::MagnifyHandler sMagnifyHandler;
}

//stands in for the pan recognizer when calling juce's scroll handler. juce sends the recognizer's translation as the
//scroll amount, but that's the total since the gesture began, so every event would repeat all of the earlier movement.
//resetting the translation isn't an option, because juce also subtracts it from the location to find the cursor
@interface BespokeScrollStep : NSObject
{
   UIPanGestureRecognizer* mPan;
   CGPoint mDelta;
}
- (instancetype)initWithPan:(UIPanGestureRecognizer*)pan delta:(CGPoint)delta;
- (CGPoint)translationInView:(UIView*)view;
- (CGPoint)locationInView:(UIView*)view;
@end

@implementation BespokeScrollStep

- (instancetype)initWithPan:(UIPanGestureRecognizer*)pan delta:(CGPoint)delta
{
   if ((self = [super init]))
   {
      mPan = pan;
      mDelta = delta;
   }
   return self;
}

- (CGPoint)translationInView:(UIView*)view
{
   return mDelta;
}

//offset so that juce arrives at the same cursor position that it gets from the recognizer's own values
- (CGPoint)locationInView:(UIView*)view
{
   CGPoint location = [mPan locationInView:view];
   CGPoint total = [mPan translationInView:view];
   return CGPointMake(location.x - total.x + mDelta.x, location.y - total.y + mDelta.y);
}

@end

//only observes: it never recognizes a gesture, and leaves touch delivery to the view untouched
//
//it also sits between juce and its hover recognizer. juce sends hover on the same mouse source as the first touch,
//with whatever buttons are currently held, so a hover during a press looks like that touch dragging to the mouse
//cursor. this happens without moving the mouse too, since iOS ends the hover when it hides the cursor for a touch
@interface BespokePointerObserver : UIGestureRecognizer <UIGestureRecognizerDelegate>
@end

@implementation BespokePointerObserver

- (void)touchesBegan:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
   sPressHeld = true;
   if (@available(iOS 13.4, *))
   {
      for (UITouch* touch in touches)
         sLastPressFromPointer = touch.type == UITouchTypeIndirectPointer;
   }
}

- (void)touchesEnded:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
   [self finishIfAllTouchesEnded:event];
}

- (void)touchesCancelled:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
   [self finishIfAllTouchesEnded:event];
}

//failing any earlier would stop us from seeing touches that start while another is held
- (void)finishIfAllTouchesEnded:(UIEvent*)event
{
   for (UITouch* touch in [event touchesForGestureRecognizer:self])
   {
      if (touch.phase != UITouchPhaseEnded && touch.phase != UITouchPhaseCancelled)
         return;
   }
   self.state = UIGestureRecognizerStateFailed;
}

- (void)reset
{
   [super reset];
   sPressHeld = false;
}

- (void)hoverChanged:(UIGestureRecognizer*)hover
{
   if (!sPressHeld)
      [self.view performSelector:sJuceHoverHandler withObject:hover];
}

- (void)scrollChanged:(UIPanGestureRecognizer*)pan
{
   if (pan.state == UIGestureRecognizerStateBegan)
      sLastScrollTranslation = CGPointZero;

   CGPoint total = [pan translationInView:self.view];
   CGPoint delta = CGPointMake(total.x - sLastScrollTranslation.x, total.y - sLastScrollTranslation.y);
   sLastScrollTranslation = total;

   //fingers drift while pinching, and scrolling the canvas would fight with the pinch's zoom
   if (sPinching || (delta.x == 0 && delta.y == 0))
      return;

   BespokeScrollStep* step = [[BespokeScrollStep alloc] initWithPan:pan delta:delta];
   [self.view performSelector:sJuceScrollHandler withObject:step];
#if !__has_feature(objc_arc)
   [step release];
#endif
}

- (void)pinchChanged:(UIPinchGestureRecognizer*)pinch
{
   UIGestureRecognizerState state = pinch.state;
   sPinching = state == UIGestureRecognizerStateBegan || state == UIGestureRecognizerStateChanged;
   if (!sPinching)
      return;

   if (state == UIGestureRecognizerStateBegan)
      sLastPinchScale = 1;

   //like scrolling, this works from the running total rather than resetting the recognizer
   CGFloat total = pinch.scale;
   float scaleFactor = (float)(total / sLastPinchScale);
   sLastPinchScale = total;

   if (sMagnifyHandler != nullptr && scaleFactor > 0)
   {
      CGPoint location = [pinch locationInView:self.view];
      sMagnifyHandler((float)location.x, (float)location.y, std::pow(scaleFactor, kTrackpadPinchSpeed));
   }
}

//the pinch recognizer is only for trackpads. finger pinches already reach bespoke as touches
- (BOOL)gestureRecognizer:(UIGestureRecognizer*)recognizer shouldReceiveEvent:(UIEvent*)event API_AVAILABLE(ios(13.4))
{
   return event.type == UIEventTypeTransform;
}

//a pinch can start after the scroll recognizer has claimed the same fingers
- (BOOL)gestureRecognizer:(UIGestureRecognizer*)recognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer*)other
{
   return YES;
}

- (BOOL)canPreventGestureRecognizer:(UIGestureRecognizer*)other
{
   return NO;
}

- (BOOL)canBePreventedByGestureRecognizer:(UIGestureRecognizer*)other
{
   return NO;
}

@end

namespace IOSPointerInput
{
   void Install(void* nativeView, MagnifyHandler onTrackpadMagnify)
   {
      if (nativeView == nullptr || nativeView == sInstalledView)
         return;
      sInstalledView = nativeView;
      sMagnifyHandler = std::move(onTrackpadMagnify);

      BespokePointerObserver* observer = [[BespokePointerObserver alloc] initWithTarget:nil action:nil];
      observer.cancelsTouchesInView = NO;
      observer.delaysTouchesBegan = NO;
      observer.delaysTouchesEnded = NO;
      UIView* view = (UIView*)nativeView;
      [view addGestureRecognizer:observer];

      if ([view respondsToSelector:sJuceHoverHandler])
      {
         for (UIGestureRecognizer* recognizer in view.gestureRecognizers)
         {
            if ([recognizer isKindOfClass:[UIHoverGestureRecognizer class]])
            {
               [recognizer removeTarget:nil action:NULL];
               [recognizer addTarget:observer action:@selector(hoverChanged:)];
            }
         }
      }

      if ([view respondsToSelector:sJuceScrollHandler])
      {
         for (UIGestureRecognizer* recognizer in view.gestureRecognizers)
         {
            if ([recognizer isKindOfClass:[UIPanGestureRecognizer class]])
            {
               [recognizer removeTarget:nil action:NULL];
               [recognizer addTarget:observer action:@selector(scrollChanged:)];
            }
         }
      }

      if (@available(iOS 13.4, *))
      {
         UIPinchGestureRecognizer* pinch = [[UIPinchGestureRecognizer alloc] initWithTarget:observer action:@selector(pinchChanged:)];
         pinch.cancelsTouchesInView = NO;
         pinch.delaysTouchesBegan = NO;
         pinch.delaysTouchesEnded = NO;
         pinch.delegate = observer;
         [view addGestureRecognizer:pinch];
#if !__has_feature(objc_arc)
         [pinch release];
#endif
      }
#if !__has_feature(objc_arc)
      [observer release];
#endif
   }

   bool WasLastPressFromPointer()
   {
      return sLastPressFromPointer;
   }
}
