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
//  IOSDocuments.mm
//  Bespoke
//

#include "IOSDocuments.h"

#import <UIKit/UIKit.h>
#import <objc/runtime.h>

namespace
{
   std::function<void(const juce::File&)> sOpenFileHandler;
   NSURL* sPendingURL = nil; //opened before a handler was set

   void OpenFile(NSURL* url)
   {
      //files opened in place (e.g. from the Files app) are outside our sandbox, and need explicit access
      bool accessing = [url startAccessingSecurityScopedResource];

      //coordinate the read, which also makes sure cloud files are downloaded first
      NSFileCoordinator* coordinator = [[NSFileCoordinator alloc] initWithFilePresenter:nil];
      NSError* error = nil;
      [coordinator coordinateReadingItemAtURL:url
                                      options:NSFileCoordinatorReadingWithoutChanges
                                        error:&error
                                   byAccessor:^(NSURL* readURL)
                                   {
                                      sOpenFileHandler(juce::File(juce::String::fromUTF8(readURL.path.UTF8String)));
                                   }];
#if !__has_feature(objc_arc)
      [coordinator release];
#endif

      if (accessing)
         [url stopAccessingSecurityScopedResource];
   }

   BOOL OpenURL(id self, SEL _cmd, UIApplication* application, NSURL* url, NSDictionary* options)
   {
      if (url == nil || !url.isFileURL)
         return NO;

      if (sOpenFileHandler)
      {
         OpenFile(url);
      }
      else
      {
#if !__has_feature(objc_arc)
         [sPendingURL release];
         sPendingURL = [url retain];
#else
         sPendingURL = url;
#endif
      }
      return YES;
   }

   //UIKit checks which methods the app delegate implements when UIApplicationMain installs it, so the method has to be
   //added before then, rather than when the app initialises. JUCE's delegate doesn't implement it, so add it at load time
   __attribute__((constructor)) void AddOpenURLToAppDelegate()
   {
      if (Class delegateClass = NSClassFromString(@"JuceAppStartupDelegate"))
         class_addMethod(delegateClass, @selector(application:openURL:options:), (IMP)OpenURL, "B@:@@@");
   }
}

void IOSDocuments::SetOpenFileHandler(std::function<void(const juce::File& file)> handler)
{
   sOpenFileHandler = std::move(handler);

   if (sPendingURL != nil && sOpenFileHandler)
   {
      OpenFile(sPendingURL);
#if !__has_feature(objc_arc)
      [sPendingURL release];
#endif
      sPendingURL = nil;
   }
}
