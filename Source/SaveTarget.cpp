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
//  SaveTarget.cpp
//  Bespoke
//

#include "SaveTarget.h"
#include "ModularSynth.h"
#include "SynthGlobals.h"

#include "juce_gui_basics/juce_gui_basics.h"

namespace
{
#if BESPOKE_IOS
   juce::Result WriteThroughURL(const juce::URL& url, const juce::File& source)
   {
      juce::FileInputStream input(source);
      if (!input.openedOk())
         return input.getStatus();

      auto output = url.createOutputStream();
      auto* fileOutput = dynamic_cast<juce::FileOutputStream*>(output.get());
      if (fileOutput == nullptr)
         return juce::Result::fail("not a local file");
      if (!fileOutput->openedOk())
         return fileOutput->getStatus();

      //juce opens existing files to append to them
      fileOutput->setPosition(0);
      fileOutput->truncate();

      auto bytesWritten = fileOutput->writeFromInputStream(input, -1);
      fileOutput->flush();
      if (fileOutput->getStatus().failed())
         return fileOutput->getStatus();
      if (bytesWritten != source.getSize())
         return juce::Result::fail("only wrote " + juce::String(bytesWritten) + " of " + juce::String(source.getSize()) + " bytes");
      return juce::Result::ok();
   }
#endif

   void ReportFailure(const std::string& path, const juce::String& reason)
   {
      ofLog() << "failed to save " << path << ": " << reason.toStdString();
      TheSynth->LogEvent("couldn't save " + path, kLogEventType_Error);
   }
}

SaveTarget::SaveTarget(const std::string& path)
: mPath(path)
, mWritePath(path)
{
}

SaveTarget::SaveTarget(const juce::FileChooser& chooser)
: SaveTarget(chooser.getResult().getFullPathName().toStdString())
{
#if BESPOKE_IOS
   mChosenURL = chooser.getURLResult();
   auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
   mWritePath = tempDir.getNonexistentChildFile("bespoke_save", ".tmp", false).getFullPathName().toStdString();
#endif
}

std::unique_ptr<juce::InputStream> SaveTarget::CreateInputStream() const
{
   std::unique_ptr<juce::InputStream> stream;
#if BESPOKE_IOS
   if (!mChosenURL.isEmpty())
      stream = mChosenURL.createInputStream(juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress));
#endif
   if (stream == nullptr)
      stream = std::make_unique<juce::FileInputStream>(juce::File(ofToDataPath(mPath)));

   auto* fileStream = dynamic_cast<juce::FileInputStream*>(stream.get());
   if (fileStream != nullptr && !fileStream->openedOk())
   {
      ofLog() << "failed to open " << mPath << ": " << fileStream->getStatus().getErrorMessage().toStdString();
      return nullptr;
   }
   return stream;
}

bool SaveTarget::Commit() const
{
   if (mWritePath == mPath)
   {
      if (juce::File(ofToDataPath(mPath)).existsAsFile())
         return true;
      ReportFailure(mPath, "file wasn't written");
      return false;
   }

   juce::File written(mWritePath);
   if (!CommitFrom(written))
      return false; //leave the written file alone, it's the only copy of the data
   written.deleteFile();
   return true;
}

bool SaveTarget::CommitFrom(const juce::File& source) const
{
#if BESPOKE_IOS
   if (!mChosenURL.isEmpty())
   {
      auto result = WriteThroughURL(mChosenURL, source);
      if (result.failed())
         ReportFailure(mPath, result.getErrorMessage());
      return result.wasOk();
   }
#endif

   if (source.copyFileTo(juce::File(ofToDataPath(mPath))))
      return true;
   ReportFailure(mPath, "copy failed");
   return false;
}
