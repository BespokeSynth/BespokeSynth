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
//  URL returned by the file chooser, so a plain path isn't enough there.
//
//  To save: write to GetWritePath(), close the file, and then call Commit(). Failures are reported to the user.
//  Reading is the same story, so files picked to load use this too, which also lets them be saved back to later.
//

#pragma once

#include "juce_core/juce_core.h"

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
#if BESPOKE_IOS
   juce::URL mChosenURL; //must stay this object or a copy of it, since it carries the permission to write
#endif
};
