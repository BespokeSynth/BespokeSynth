/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

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
//  SearchPanel.cpp
//  Bespoke
//

#include <algorithm>
#include <cctype>
#include "SearchPanel.h"
#include "ModularSynth.h"
#include "ModuleContainer.h"
#include "TitleBar.h"
#include "SynthGlobals.h"
#include "juce_audio_formats/juce_audio_formats.h"

SearchPanel* TheSearchPanel = nullptr;

SearchPanel::SearchPanel()
{
   assert(TheSearchPanel == nullptr);
   TheSearchPanel = this;
}

SearchPanel::~SearchPanel()
{
   StopIndexThread();
   TheSearchPanel = nullptr;
}

void SearchPanel::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mSearchEntry = new TextEntry(this, "search", -1, -1, 24, &mSearchTextBound);
   mSearchEntry->SetRequireEnter(false);
   mSearchEntry->DrawLabel(true);

   mAddLocationButton = new ClickButton(this, "+ add folder", -1, -1);
   mRescanButton = new ClickButton(this, "rescan", -1, -1);

   for (int i = 0; i < kMaxLocationRows; ++i)
   {
      mRemoveLocationButtons[i] = new ClickButton(this, ("removeloc" + ofToString(i)).c_str(), -1, -1);
      mRemoveLocationButtons[i]->SetShowing(false);
   }

   for (int i = 0; i < kMaxResults; ++i)
   {
      mModuleButtons[i] = new ClickButton(this, ("moduleresult" + ofToString(i)).c_str(), -1, -1);
      mModuleButtons[i]->SetShowing(false);
   }

   for (int i = 0; i < kVisibleSampleRows; ++i)
   {
      mSampleButtons[i] = new ClickButton(this, ("sampleresult" + ofToString(i)).c_str(), -1, -1);
      mSampleButtons[i]->SetDisplayStyle(ButtonDisplayStyle::kSampleIcon);
      mSampleButtons[i]->SetShowing(false);

      mPreviewButtons[i] = new ClickButton(this, ("samplepreview" + ofToString(i)).c_str(), -1, -1);
      mPreviewButtons[i]->SetLabel(">"); // ASCII play indicator — always renders
      mPreviewButtons[i]->SetShowing(false);
   }

   LoadLocations();
   LoadWidth();
   //load the cached index from disk (instant); only do a full background scan if there's no cache
   //(first run). After that it's a one-time thing - use the "rescan" button to refresh.
   if (!LoadIndex())
      RebuildIndex();

   //hidden by default - opened/closed on demand via the top-bar search toggle (Bitwig-style),
   //rather than always occupying the right edge of the screen
   SetShowing(false);
}

void SearchPanel::Poll()
{
   IDrawableModule::Poll();

   //dock to the full right edge of the window, Bitwig-browser style, and re-anchor every
   //frame so it stays put (and full height) across window resizes.
   //
   //STICKY FIX: this panel lives in the UI-layer module container, which has its *own*
   //draw scale/offset that are independent of the canvas pan/zoom (the canvas is driven by
   //the global gDrawScale). Anchoring against gDrawScale therefore made the panel drift off
   //the right edge whenever the user zoomed or panned the patch canvas. Measure against the
   //owning container instead - exactly what TitleBar does to stay pinned - so the panel is
   //truly screen-sticky regardless of canvas navigation.
   ModuleContainer* container = GetOwningContainer();
   const float scale = (container != nullptr) ? container->GetDrawScale() : gDrawScale;
   const ofVec2f offset = (container != nullptr) ? container->GetDrawOffset() : ofVec2f();

   //anchor the panel just BELOW the title bar. The title bar gets taller when the window is narrow
   //(it wraps its dropdowns to a second/third row), which is why on a smaller screen the search box
   //ended up hidden behind it. Measure the real title-bar height instead of assuming a fixed 40px.
   float topMargin = 40;
   if (TheTitleBar != nullptr)
      topMargin = MAX(topMargin, TheTitleBar->GetRect(true).height + 4);
   const float rightMargin = 10;
   const float bottomMargin = 10;

   const float rightEdge = ofGetWidth() / scale - rightMargin - offset.x;
   const float panelY = topMargin - offset.y;

   //scrollbar drag is driven here (per frame while the button is held) rather than via MouseMoved,
   //because Bespoke only routes drag-moves to a "grabbed" control and this panel isn't one. Read the
   //mouse in the owning container's space and convert to panel-local Y to set the scroll position.
   if (mDraggingScroll)
   {
      if (!TheSynth->IsMouseButtonHeld(1))
      {
         mDraggingScroll = false;
      }
      else if (mScrollTotal > mScrollVisible && mScrollTrackH > 0)
      {
         float localY = TheSynth->GetMouseY(container) - panelY;
         float frac = ofClamp((localY - mScrollTrackTop) / mScrollTrackH, 0.0f, 1.0f);
         mSampleScroll = (int)(frac * (mScrollTotal - mScrollVisible) + 0.5f);
      }
   }

   mWidth = (int)ofClamp((float)mUserWidth, (float)kMinWidth, (float)kMaxWidth);
   mHeight = (int)MAX(ofGetHeight() / scale - topMargin - bottomMargin, 200.0f);
   SetPosition(rightEdge - mWidth, panelY);

   // Poll whether afplay subprocess has finished
   if (mPreviewPid > 0)
   {
      int status;
      if (waitpid(mPreviewPid, &status, WNOHANG) != 0)
      {
         // process finished or error
         mPreviewPid = -1;
         mPreviewingRow = -1;
      }
   }
}

void SearchPanel::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   //TextEntry only writes back to its bound std::string on enter/blur, so for live-as-you-type
   //filtering we poll the raw typed text directly every frame instead
   std::string currentText = mSearchEntry->GetText();
   int curIndexCount = mIndexedCount.load();
   if (currentText != mLastSearchText || curIndexCount != mLastResultsIndexCount)
   {
      mLastSearchText = currentText;
      mLastResultsIndexCount = curIndexCount; //refresh browse list once the index has (re)loaded
      UpdateResults();
   }

   const float rowHeight = 17;
   const float topY = 20;

   //left-edge scrollbar rail (the draggable thumb is drawn in the results section below, once the
   //track geometry for this frame is known)
   ofPushStyle();
   ofFill();
   ofSetColor(50, 50, 55, 70);
   ofRect(0, 0, (float)kHandleWidth, (float)mHeight, 0);
   ofPopStyle();

   //number of chars a truncated path/label can show at the current width
   const int labelChars = MAX(6, (mWidth - kContentX - 22) / 7);

   //search field fills the top row and is clamped to the panel width so it never spills outside
   mSearchEntry->SetOverrideWidth((float)(mWidth - kContentX - 4));
   mSearchEntry->SetPosition(kContentX, 2);
   mSearchEntry->Draw();

   float rowY = topY;

   //Locations section (Bitwig-style: user-added root folders searched alongside the default
   //samples path). Folders are added via a native picker, no manual typing.
   DrawTextNormal("locations", kContentX, rowY + 11);
   rowY += rowHeight;

   for (int i = 0; i < kMaxLocationRows; ++i)
   {
      if (i < (int)mSearchLocations.size())
      {
         DrawTextNormal(TruncatePathForDisplay(mSearchLocations[i], labelChars), kContentX, rowY + 11);
         mRemoveLocationButtons[i]->SetShowing(true);
         mRemoveLocationButtons[i]->SetLabel("x");
         mRemoveLocationButtons[i]->SetPosition(mWidth - 20, rowY);
         mRemoveLocationButtons[i]->Draw();
         rowY += rowHeight;
      }
      else
      {
         mRemoveLocationButtons[i]->SetShowing(false);
      }
   }

   mAddLocationButton->SetShowing(true);
   mAddLocationButton->SetPosition(kContentX, rowY);
   mAddLocationButton->Draw();
   rowY += rowHeight;

   //rescan button + index status (Bitwig/Ableton-style background indexing)
   mRescanButton->SetShowing(true);
   mRescanButton->SetPosition(kContentX, rowY);
   mRescanButton->Draw();
   int idxCount = mIndexedCount.load();
   if (mIndexing)
      DrawTextNormal("indexing... " + ofToString(idxCount), kContentX + 54, rowY + 11, 11);
   else if (idxCount >= kMaxIndexed)
      DrawTextNormal(ofToString(kMaxIndexed) + "+ (capped)", kContentX + 54, rowY + 11, 11);
   else
      DrawTextNormal(ofToString(idxCount) + " samples", kContentX + 54, rowY + 11, 11);
   rowY += rowHeight + 6; //extra gap before results

   if (!mLastSearchText.empty() && mModuleResults.empty() && mSampleResults.empty())
      DrawTextNormal("no matches", kContentX, rowY + 11);

   for (int i = 0; i < kMaxResults; ++i)
   {
      if (i < (int)mModuleResults.size())
      {
         mModuleButtons[i]->SetShowing(true);
         mModuleButtons[i]->SetLabel(mModuleResults[i].mLabel.c_str());
         mModuleButtons[i]->SetPosition(kContentX, rowY);
         mModuleButtons[i]->Draw();
         rowY += rowHeight;
      }
      else
      {
         mModuleButtons[i]->SetShowing(false);
      }
   }

   if (!mModuleResults.empty() && !mSampleResults.empty())
      rowY += 6; //gap between the module results and the sample results

   //samples: a VIRTUALIZED, scrollable list. Only the rows currently visible are drawn (mouse-wheel
   //to scroll through the full match list), so tens of thousands of matches never bog down the UI.
   int totalSamples = (int)mSampleResults.size();
   if (totalSamples > 0)
   {
      int fitRows = (int)((mHeight - rowY - rowHeight - 4) / rowHeight); //rows that fit below the count line
      int visible = MIN(kVisibleSampleRows, MAX(1, fitRows));
      visible = MIN(visible, totalSamples);

      int maxScroll = MAX(0, totalSamples - visible);
      if (mSampleScroll > maxScroll)
         mSampleScroll = maxScroll;
      if (mSampleScroll < 0)
         mSampleScroll = 0;

      DrawTextNormal(ofToString(totalSamples) + " results (" + ofToString(mSampleScroll + 1) + "-" + ofToString(mSampleScroll + visible) + ") \xE2\x86\x95 drag left strip", kContentX, rowY + 11, 11);
      rowY += rowHeight;

      //capture the scrollbar track for this frame (left strip spans the drawn rows)
      mScrollTrackTop = rowY;
      mScrollTrackH = visible * rowHeight;
      mScrollTotal = totalSamples;
      mScrollVisible = visible;

      for (int i = 0; i < kVisibleSampleRows; ++i)
      {
         int resultIdx = mSampleScroll + i;
         if (i < visible && resultIdx < totalSamples)
         {
            const float kPlayBtnW = 16;
            const float kGap = 2;

            // ▶ preview button — show > while stopped, || while playing
            bool isPlaying = (mPreviewingRow == i);
            mPreviewButtons[i]->SetShowing(true);
            mPreviewButtons[i]->SetLabel(isPlaying ? "||" : ">");
            mPreviewButtons[i]->SetPosition(kContentX, rowY);
            mPreviewButtons[i]->SetDimensions(kPlayBtnW, rowHeight - 1);
            mPreviewButtons[i]->Draw();

            // sample name button pushed right of the play button
            mSampleButtons[i]->SetShowing(true);
            mSampleButtons[i]->SetLabel(juce::File(mSampleResults[resultIdx]).getFileName().toStdString().c_str());
            mSampleButtons[i]->SetPosition(kContentX + kPlayBtnW + kGap, rowY);
            mSampleButtons[i]->SetDimensions(mWidth - kContentX - kPlayBtnW - kGap - 4, rowHeight - 1);
            mSampleButtons[i]->Draw();
            rowY += rowHeight;
         }
         else
         {
            mSampleButtons[i]->SetShowing(false);
            mPreviewButtons[i]->SetShowing(false);
         }
      }

      //draggable scrollbar thumb on the left strip - drag it to scrub the whole list fast
      if (totalSamples > visible && mScrollTrackH > 0)
      {
         float th = MAX(16.0f, mScrollTrackH * (float)visible / totalSamples);
         float ty = mScrollTrackTop + (mScrollTrackH - th) * (float)mSampleScroll / MAX(1, totalSamples - visible);
         ofPushStyle();
         ofFill();
         //bright, clearly-grabbable thumb on the left strip
         if (mDraggingScroll)
            ofSetColor(120, 210, 255, 240);
         else
            ofSetColor(120, 200, 255, 170);
         ofRect(1, ty, (float)kHandleWidth - 2, th, 3);
         ofPopStyle();
      }
   }
   else
   {
      mScrollTotal = 0;
      mScrollVisible = 0;
      for (int i = 0; i < kVisibleSampleRows; ++i)
      {
         mSampleButtons[i]->SetShowing(false);
         mPreviewButtons[i]->SetShowing(false);
      }
   }

   //mWidth/mHeight are docked and driven by Poll(), not by content, so this panel stays a
   //stable full-height sidebar like Bitwig's browser rather than growing/shrinking per-search
}

bool SearchPanel::MouseScrolled(float x, float y, float scrollX, float scrollY, bool isSmoothScroll, bool isInvertedScroll)
{
   if (!IsShowing())
      return false;

   //wheel scrolls the sample results list (3 rows per notch); draw() re-clamps to a valid window
   int step = (scrollY > 0) ? -3 : 3;
   mSampleScroll += step;
   if (mSampleScroll < 0)
      mSampleScroll = 0;
   int maxScroll = MAX(0, (int)mSampleResults.size() - 1);
   if (mSampleScroll > maxScroll)
      mSampleScroll = maxScroll;
   return true;
}

void SearchPanel::StopIndexThread()
{
   if (mIndexThread.joinable())
   {
      mStopIndex = true;
      mIndexThread.join();
   }
   mStopIndex = false;
}

void SearchPanel::RebuildIndex()
{
   StopIndexThread();

   //collect the folders to scan: the default samples path + every user-added location
   std::vector<std::string> locations;
   locations.push_back(std::string(ofToSamplePath("")));
   for (auto& loc : mSearchLocations)
      locations.push_back(loc);

   mIndexedCount = 0;
   mIndexing = true;
   mStopIndex = false;
   mIndexThread = std::thread(&SearchPanel::IndexWorker, this, locations);
}

void SearchPanel::IndexWorker(std::vector<std::string> locations)
{
   //runs OFF the UI thread. Manual directory walk (a stack, non-recursive per dir) so we can bail
   //promptly via mStopIndex (e.g. on rescan or app quit) instead of blocking inside a big recursive
   //findChildFiles call.
   juce::String matcher = TheSynth->GetAudioFormatManager().getWildcardForAllFormats();
   juce::StringArray wildcards;
   wildcards.addTokens(matcher, ";,", "\"'");
   wildcards.trim();
   wildcards.removeEmptyStrings();

   std::vector<IndexedSample> local;
   std::vector<juce::File> stack;
   for (auto& loc : locations)
   {
      juce::String locPath(loc);
      juce::File dir(locPath);
      if (dir.isDirectory())
         stack.push_back(dir);
   }

   while (!stack.empty() && !mStopIndex && (int)local.size() < kMaxIndexed)
   {
      juce::File dir = stack.back();
      stack.pop_back();

      juce::Array<juce::File> children = dir.findChildFiles(juce::File::findFilesAndDirectories | juce::File::ignoreHiddenFiles, false);
      for (auto& child : children)
      {
         if (mStopIndex || (int)local.size() >= kMaxIndexed)
            break;

         if (child.isDirectory())
         {
            //don't follow symlinked/aliased directories - they can form loops that balloon the walk
            //(this is what could push the count all the way to the 200k safety cap)
            if (!child.isSymbolicLink())
               stack.push_back(child);
            continue;
         }

         juce::String name = child.getFileName();
         bool isAudio = false;
         for (auto& w : wildcards)
         {
            if (name.matchesWildcard(w, true))
            {
               isAudio = true;
               break;
            }
         }
         if (!isAudio)
            continue;

         IndexedSample entry;
         entry.path = child.getFullPathName().toStdString();
         entry.lowerName = name.toLowerCase().toStdString();
         local.push_back(std::move(entry));
         mIndexedCount = (int)local.size();
      }
   }

   if (!mStopIndex)
   {
      SaveIndex(local); //cache to disk so future launches load instantly (off the UI thread, fine)
      std::lock_guard<std::mutex> lock(mIndexMutex);
      mSampleIndex = std::move(local);
      mIndexedCount = (int)mSampleIndex.size();
   }
   mIndexing = false;
}

//static
std::string SearchPanel::GetIndexFilePath()
{
   return ofToDataPath("searchindex.txt");
}

bool SearchPanel::LoadIndex()
{
   juce::File file(GetIndexFilePath());
   if (!file.existsAsFile())
      return false;

   juce::StringArray lines;
   file.readLines(lines);

   std::vector<IndexedSample> loaded;
   loaded.reserve((size_t)lines.size());
   for (auto& line : lines)
   {
      std::string path = line.toStdString();
      if (path.empty())
         continue;
      //derive the lowercased filename from the path (cheap - no juce::File construction)
      size_t slash = path.find_last_of("/\\");
      std::string fname = (slash == std::string::npos) ? path : path.substr(slash + 1);
      std::transform(fname.begin(), fname.end(), fname.begin(), [](unsigned char c)
                     {
                        return (char)std::tolower(c);
                     });

      IndexedSample entry;
      entry.path = std::move(path);
      entry.lowerName = std::move(fname);
      loaded.push_back(std::move(entry));
      if ((int)loaded.size() >= kMaxIndexed)
         break;
   }

   if (loaded.empty())
      return false;

   {
      std::lock_guard<std::mutex> lock(mIndexMutex);
      mSampleIndex = std::move(loaded);
      mIndexedCount = (int)mSampleIndex.size();
   }
   mIndexing = false;
   return true;
}

void SearchPanel::SaveIndex(const std::vector<IndexedSample>& entries)
{
   std::string buf;
   buf.reserve(entries.size() * 48);
   for (const auto& e : entries)
   {
      buf += e.path;
      buf += '\n';
   }
   juce::File file(GetIndexFilePath());
   file.replaceWithText(juce::String(buf));
}

void SearchPanel::UpdateResults()
{
   mModuleResults.clear();
   mSampleResults.clear();
   mSampleScroll = 0;

   //empty search box => BROWSE the whole library: fill the (scrollable) list with the entire index
   //so you can scrub through everything with the left strip without typing anything
   if (mLastSearchText.empty())
   {
      std::lock_guard<std::mutex> lock(mIndexMutex);
      for (const auto& entry : mSampleIndex)
      {
         if ((int)mSampleResults.size() >= kMaxSampleMatches)
            break;
         mSampleResults.push_back(juce::String(entry.path));
      }
      return;
   }

   //modules: reuses the exact same fuzzy multi-source lookup (built-in modules, VSTs, prefabs,
   //midi controllers, and effect-chain effects) that the right-click quick-spawn menu uses
   auto spawnables = TheSynth->GetModuleFactory()->GetSpawnableModules(mLastSearchText, true);
   for (int i = 0; i < (int)spawnables.size() && (int)mModuleResults.size() < kMaxResults; ++i)
      mModuleResults.push_back(spawnables[i]);

   //samples: pure in-memory substring match against the pre-built index - NO disk access, so it's
   //instant no matter how large the library is. We collect the FULL match list (up to a generous
   //cap) ranked so filename-prefix matches come first, then anywhere-matches. The panel shows a
   //scrollable window over this list, so you can browse them all without drawing thousands of rows.
   mSampleScroll = 0;

   std::string needle = mLastSearchText;
   std::transform(needle.begin(), needle.end(), needle.begin(), [](unsigned char c)
                  {
                     return (char)std::tolower(c);
                  });

   std::vector<juce::String> prefixMatches;
   std::vector<juce::String> otherMatches;

   {
      std::lock_guard<std::mutex> lock(mIndexMutex);
      for (const auto& entry : mSampleIndex)
      {
         size_t pos = entry.lowerName.find(needle);
         if (pos == std::string::npos)
            continue;
         if (pos == 0 && (int)prefixMatches.size() < kMaxSampleMatches)
            prefixMatches.push_back(juce::String(entry.path));
         else if ((int)otherMatches.size() < kMaxSampleMatches)
            otherMatches.push_back(juce::String(entry.path));
         if ((int)(prefixMatches.size() + otherMatches.size()) >= kMaxSampleMatches * 2)
            break;
      }
   }

   //prefix matches first (real "kick.wav" before "trap_kick_layer.wav"), then the rest, capped
   mSampleResults = std::move(prefixMatches);
   for (auto& p : otherMatches)
   {
      if ((int)mSampleResults.size() >= kMaxSampleMatches)
         break;
      mSampleResults.push_back(p);
   }
}

void SearchPanel::BrowseForLocation()
{
   if ((int)mSearchLocations.size() >= kMaxLocationRows)
      return;

   juce::FileChooser chooser("Add search location...", juce::File(ofToSamplePath("")), "", true, false, TheSynth->GetFileChooserParent());
   if (chooser.browseForDirectory())
   {
      std::string path = chooser.getResult().getFullPathName().toStdString();
      if (std::find(mSearchLocations.begin(), mSearchLocations.end(), path) == mSearchLocations.end())
      {
         mSearchLocations.push_back(path);
         SaveLocations();
         RebuildIndex(); //re-scan in the background to include the new folder
      }
   }
}

void SearchPanel::RemoveLocation(int index)
{
   if (index >= 0 && index < (int)mSearchLocations.size())
   {
      mSearchLocations.erase(mSearchLocations.begin() + index);
      SaveLocations();
      RebuildIndex();
   }
}

//static
std::string SearchPanel::GetLocationsFilePath()
{
   return ofToDataPath("searchlocations.txt");
}

void SearchPanel::LoadLocations()
{
   mSearchLocations.clear();

   juce::File file(GetLocationsFilePath());
   if (!file.existsAsFile())
      return;

   juce::StringArray lines;
   file.readLines(lines);
   for (auto& line : lines)
   {
      juce::String trimmed = line.trim();
      if (trimmed.isNotEmpty())
         mSearchLocations.push_back(trimmed.toStdString());
   }
}

void SearchPanel::SaveLocations()
{
   juce::File file(GetLocationsFilePath());
   juce::String content;
   for (auto& loc : mSearchLocations)
      content += juce::String(loc) + "\n";
   file.replaceWithText(content);
}

//static
std::string SearchPanel::GetWidthFilePath()
{
   return ofToDataPath("searchpanelwidth.txt");
}

void SearchPanel::LoadWidth()
{
   juce::File file(GetWidthFilePath());
   if (file.existsAsFile())
   {
      int w = file.loadFileAsString().trim().getIntValue();
      if (w > 0)
         mUserWidth = (int)ofClamp((float)w, (float)kMinWidth, (float)kMaxWidth);
   }
}

void SearchPanel::SaveWidth()
{
   juce::File file(GetWidthFilePath());
   file.replaceWithText(juce::String(mUserWidth));
}

//left-edge grab strip starts a width drag; everything else falls through to the base so the
//search field, buttons, and result rows keep working normally
bool SearchPanel::TestClick(float x, float y, bool right, bool testOnly)
{
   if (!IsShowing()) //when the panel is toggled closed, don't intercept clicks in its screen region
      return false;
   return IDrawableModule::TestClick(x, y, right, testOnly);
}

void SearchPanel::OnClicked(float x, float y, bool right)
{
   //body click (not on a child button). If it's on the left scrollbar strip within the results
   //track, start scrubbing (Poll() then follows the mouse while the button is held).
   if (!right && x >= 0 && x <= (float)kHandleWidth && mScrollTotal > mScrollVisible && y >= mScrollTrackTop && y <= mScrollTrackTop + mScrollTrackH)
   {
      mDraggingScroll = true;
      float frac = ofClamp((y - mScrollTrackTop) / MAX(1.0f, mScrollTrackH), 0.0f, 1.0f);
      mSampleScroll = (int)(frac * (mScrollTotal - mScrollVisible) + 0.5f);
      return;
   }
   IDrawableModule::OnClicked(x, y, right);
}

bool SearchPanel::MouseMoved(float x, float y)
{
   if (!IsShowing())
      return false;

   //while dragging the left strip, map the mouse position to a scroll position over the whole list
   if (mDraggingScroll && mScrollTotal > mScrollVisible)
   {
      float frac = ofClamp((y - mScrollTrackTop) / MAX(1.0f, mScrollTrackH), 0.0f, 1.0f);
      mSampleScroll = (int)(frac * (mScrollTotal - mScrollVisible) + 0.5f);
      return true;
   }
   return IDrawableModule::MouseMoved(x, y);
}

void SearchPanel::MouseReleased()
{
   IDrawableModule::MouseReleased();
   mDraggingScroll = false;
}

//static
std::string SearchPanel::TruncatePathForDisplay(const std::string& path, int maxChars)
{
   if (maxChars < 5)
      maxChars = 5;
   if ((int)path.size() <= maxChars)
      return path;
   return "..." + path.substr(path.size() - (maxChars - 3));
}

void SearchPanel::ButtonClicked(ClickButton* button, double time)
{
   if (button == mAddLocationButton)
   {
      BrowseForLocation();
      return;
   }

   if (button == mRescanButton)
   {
      RebuildIndex();
      return;
   }

   for (int i = 0; i < kMaxLocationRows; ++i)
   {
      if (button == mRemoveLocationButtons[i] && i < (int)mSearchLocations.size())
      {
         RemoveLocation(i);
         return;
      }
   }

   for (int i = 0; i < kMaxResults; ++i)
   {
      if (button == mModuleButtons[i] && i < (int)mModuleResults.size())
      {
         ofVec2f grabOffset(-40, 10);
         IDrawableModule* module = TheSynth->SpawnModuleOnTheFly(mModuleResults[i], TheSynth->GetMouseX(TheSynth->GetRootContainer()) + grabOffset.x, TheSynth->GetMouseY(TheSynth->GetRootContainer()) + grabOffset.y);
         TheSynth->SetMoveModule(module, grabOffset.x, grabOffset.y, true);
      }
   }

   //sample buttons are a scrolled window over the full match list - map the clicked row back
   //through the scroll offset to the actual result
   for (int i = 0; i < kVisibleSampleRows; ++i)
   {
      if (button == mPreviewButtons[i])
      {
         if (mPreviewingRow == i)
         {
            // second click on same row = stop
            StopPreview();
         }
         else
         {
            int resultIdx = mSampleScroll + i;
            if (resultIdx >= 0 && resultIdx < (int)mSampleResults.size())
            {
               StopPreview();
               StartPreview(mSampleResults[resultIdx].toStdString());
               mPreviewingRow = i;
            }
         }
         return;
      }

      if (button == mSampleButtons[i])
      {
         int resultIdx = mSampleScroll + i;
         if (resultIdx >= 0 && resultIdx < (int)mSampleResults.size())
            TheSynth->GrabSample(mSampleResults[resultIdx].toStdString());
         return;
      }
   }
}

bool SearchPanel::GetRelativeSamplePath(const std::string& currentPath, int offset, std::string& outPath)
{
   std::lock_guard<std::mutex> lock(mIndexMutex);
   if (mSampleIndex.empty())
      return false;

   //alphabetical view of the scanned library so "next" is deterministic and "closest" is meaningful
   std::vector<std::string> paths;
   paths.reserve(mSampleIndex.size());
   for (const auto& e : mSampleIndex)
      paths.push_back(e.path);
   std::sort(paths.begin(), paths.end());

   const int n = (int)paths.size();
   //locate the current sample (or the closest alphabetical position if it isn't in the index)
   int idx = 0;
   auto it = std::lower_bound(paths.begin(), paths.end(), currentPath);
   if (it != paths.end())
      idx = (int)(it - paths.begin());
   else
      idx = n - 1;

   int newIdx = ((idx + offset) % n + n) % n;
   outPath = paths[newIdx];
   return true;
}

void SearchPanel::StartPreview(const std::string& path)
{
   StopPreview();

   // Spawn afplay as a child process (macOS built-in, handles all formats
   // and sample-rate conversion natively, no threading issues)
   pid_t pid = fork();
   if (pid == 0)
   {
      // child — exec afplay then exit
      execlp("afplay", "afplay", path.c_str(), (char*)nullptr);
      _exit(1);
   }
   else if (pid > 0)
   {
      mPreviewPid = pid;
   }
}

void SearchPanel::StopPreview()
{
   if (mPreviewPid > 0)
   {
      kill(mPreviewPid, SIGTERM);
      waitpid(mPreviewPid, nullptr, WNOHANG);
      mPreviewPid = -1;
   }
   mPreviewingRow = -1;
}
