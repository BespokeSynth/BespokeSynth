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
#include "UserPrefs.h"

#include "juce_gui_basics/juce_gui_basics.h"

#if BESPOKE_ANDROID
#include "AndroidSupport.h"
#endif

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
{
#if BESPOKE_ANDROID
   auto url = chooser.getURLResult();
   if (AndroidSupport::IsDocument(url))
   {
      mChosenURL = url;
      mPath = AndroidSupport::GetDocumentMirror(url).getFullPathName().toStdString();
      auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
      mWritePath = tempDir.getNonexistentChildFile("bespoke_save", ".tmp", false).getFullPathName().toStdString();
      return;
   }
#endif

   mPath = chooser.getResult().getFullPathName().toStdString();
   mWritePath = mPath;
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
#elif BESPOKE_ANDROID
   //refresh the local copy (mPath), then read that
   if (!mChosenURL.isEmpty())
   {
      auto result = AndroidSupport::CopyDocumentToMirror(mChosenURL);
      if (result.failed())
         ofLog() << "couldn't read " << mChosenURL.toString(false).toStdString() << ", using the last copy of it: " << result.getErrorMessage().toStdString();
   }
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
#elif BESPOKE_ANDROID
   if (!mChosenURL.isEmpty())
   {
      auto result = AndroidSupport::WriteToDocument(mChosenURL, source);
      if (result.failed())
      {
         ReportFailure(AndroidSupport::GetDocumentName(mChosenURL).toStdString(), result.getErrorMessage());
         return false;
      }
      //keep the local copy (mPath) in sync, since that's what gets read
      juce::File mirror(mPath);
      mirror.getParentDirectory().createDirectory();
      if (!source.copyFileTo(mirror))
         ofLog() << "couldn't update " << mPath;
      return true;
   }
#endif

   if (source.copyFileTo(juce::File(ofToDataPath(mPath))))
      return true;
   ReportFailure(mPath, "copy failed");
   return false;
}

namespace
{
#if !JUCE_MODAL_LOOPS_PERMITTED
   //only one chooser can be open at a time. it has to outlive launchAsync(), and is replaced by the next one
   std::unique_ptr<juce::FileChooser> sActiveChooser;
#endif

   void BrowseForFile(bool save, const juce::String& title, const juce::File& initialFileOrDirectory, const juce::String& filePatterns, FileChosenCallback onChosen, bool alwaysUseNativeChooser)
   {
      bool useNativeChooser = alwaysUseNativeChooser || !UserPrefs.force_juce_file_chooser.Get();
      auto chooser = std::make_unique<juce::FileChooser>(title, initialFileOrDirectory, filePatterns, useNativeChooser, false, TheSynth->GetFileChooserParent());

#if JUCE_MODAL_LOOPS_PERMITTED
      if (save ? chooser->browseForFileToSave(true) : chooser->browseForFileToOpen())
         onChosen(*chooser);
#else
      int flags = juce::FileBrowserComponent::canSelectFiles;
      if (save)
         flags |= juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting;
      else
         flags |= juce::FileBrowserComponent::openMode;

      sActiveChooser = std::move(chooser);
      sActiveChooser->launchAsync(flags, [onChosen = std::move(onChosen)](const juce::FileChooser& finishedChooser)
                                  {
                                     if (!finishedChooser.getURLResults().isEmpty())
                                        onChosen(finishedChooser);
                                  });
#endif
   }
}

void BrowseForFileToOpen(const juce::String& title, const juce::File& initialFileOrDirectory, const juce::String& filePatterns, FileChosenCallback onChosen, bool alwaysUseNativeChooser)
{
   BrowseForFile(false, title, initialFileOrDirectory, filePatterns, std::move(onChosen), alwaysUseNativeChooser);
}

void BrowseForFileToSave(const juce::String& title, const juce::File& initialFileOrDirectory, const juce::String& filePatterns, FileChosenCallback onChosen, bool alwaysUseNativeChooser)
{
   BrowseForFile(true, title, initialFileOrDirectory, filePatterns, std::move(onChosen), alwaysUseNativeChooser);
}

juce::File GetChosenFileForReading(const juce::FileChooser& chooser)
{
#if BESPOKE_ANDROID
   auto url = chooser.getURLResult();
   if (AndroidSupport::IsDocument(url))
   {
      auto result = AndroidSupport::CopyDocumentToMirror(url);
      if (result.failed())
      {
         TheSynth->LogEvent("couldn't read " + AndroidSupport::GetDocumentName(url).toStdString() + ": " + result.getErrorMessage().toStdString(), kLogEventType_Error);
         return {};
      }
      return AndroidSupport::GetDocumentMirror(url);
   }
#endif
   return chooser.getResult();
}
