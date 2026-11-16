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
//  SaveTarget.h
//  Bespoke
//
//  Where a save is going. iOS only allows writing outside of the app's own folders (e.g. to iCloud Drive) through the
//  URL returned by the file chooser, so a plain path isn't enough there. On Android, the file chooser returns
//  content:// URLs, which aren't files at all, so those get a local copy to read from (see AndroidSupport.h).
//
//  To save: write to GetWritePath(), close the file, and then call Commit(). Failures are reported to the user.
//  Reading is the same story, so files picked to load use this too, which also lets them be saved back to later.
//

#pragma once

#include "juce_core/juce_core.h"

#include <functional>
#include <memory>
#include <string>

namespace juce
{
   class FileChooser;
}

class SaveTarget
{
public:
   SaveTarget() = default;
   explicit SaveTarget(const std::string& path);
   explicit SaveTarget(const juce::FileChooser& chooser); //chooser must have a result

   const std::string& GetPath() const { return mPath; } //where the file ends up
   const std::string& GetWritePath() const { return mWritePath; } //where to write it

   bool Commit() const; //call after writing to GetWritePath()
   bool CommitFrom(const juce::File& source) const; //use source instead of GetWritePath(). leaves source in place

   //returns nullptr if the file can't be opened. permission to read lasts for as long as the stream does
   std::unique_ptr<juce::InputStream> CreateInputStream() const;

private:
   std::string mPath;
   std::string mWritePath;
#if BESPOKE_MOBILE
   juce::URL mChosenURL; //must stay this object or a copy of it, since it carries the permission to write
#endif
};

//show a file chooser, and call onChosen if the user picked a file. on platforms with modal loops, this blocks until
//the chooser closes, like FileChooser::browseForFileToOpen()/browseForFileToSave(true). android doesn't have them, so
//there it returns right away, and onChosen is called later on the message thread.
//the chooser is the OS's native one unless the user prefers JUCE's (force_juce_file_chooser), or alwaysUseNativeChooser
using FileChosenCallback = std::function<void(const juce::FileChooser& chooser)>;
void BrowseForFileToOpen(const juce::String& title, const juce::File& initialFileOrDirectory, const juce::String& filePatterns, FileChosenCallback onChosen, bool alwaysUseNativeChooser = false);
void BrowseForFileToSave(const juce::String& title, const juce::File& initialFileOrDirectory, const juce::String& filePatterns, FileChosenCallback onChosen, bool alwaysUseNativeChooser = false);

//a local file with the chosen file's contents, for code that reads files by path. on android, the chosen document is
//copied into app storage first. returns an empty File if that fails
juce::File GetChosenFileForReading(const juce::FileChooser& chooser);
