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
//  SearchPanel.h
//  Bespoke
//
//  A persistent, right-docked panel (modeled on Bitwig's browser sidebar):
//  type to search, and it filters two lists at once - spawnable modules
//  (reusing the same fuzzy lookup as the right-click quick-spawn menu) and
//  audio sample files found under the default samples path plus any number
//  of user-added "Locations" (arbitrary folders anywhere on disk, Bitwig-
//  style), searched recursively by filename. The panel docks to the full
//  right edge of the window and re-anchors itself every frame so it stays
//  put when the window is resized.
//
//  Clicking a module result spawns it attached to the cursor for placement
//  (same flow as quick-spawn). Clicking a sample result "grabs" it (same
//  mechanism the SampleBrowser module uses) so it can be clicked onto an
//  existing module, or dropped on empty canvas to spawn a fresh sample
//  player preloaded with it.
//
//  Locations are added via a native folder-picker ("+ add folder", no
//  manual typing needed) and are persisted in searchlocations.txt in the
//  app's data path, so they survive restarts.
//

#pragma once

#include <array>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include "IDrawableModule.h"
#include "TextEntry.h"
#include "ClickButton.h"
#include "ModuleFactory.h"

class SearchPanel : public IDrawableModule, public ITextEntryListener, public IButtonListener
{
public:
   SearchPanel();
   virtual ~SearchPanel();

   void CreateUIControls() override;

   //public API for other modules (e.g. Tracker): walk the already-scanned sample library. Finds the
   //entry closest to currentPath alphabetically, then returns the one `offset` positions away (wraps).
   //Thread-safe against the background indexer. Returns false if the index is empty.
   bool GetRelativeSamplePath(const std::string& currentPath, int offset, std::string& outPath);

   bool HasTitleBar() const override { return false; }
   bool IsSaveable() override { return false; }
   bool IsSingleton() const override { return true; }
   bool ShouldClipContents() override { return false; }
   //ZOOM FIX: this panel lives in the fixed UI overlay layer, but the base IsVisible() culls a module
   //when its logical rect falls outside the *canvas* draw rect. Zooming the patch shrinks that rect
   //until this narrow right-docked panel no longer intersects it, so it collapsed to a beacon and
   //"disappeared". Being screen-pinned, it is always on-screen - exempt it from canvas-rect culling.
   //(requires IDrawableModule::IsVisible to be virtual)
   bool IsVisible() override { return IsShowing(); }
   bool CanBeMoved() const override { return false; } //docked/pinned - never click-draggable, so it can't be dragged off its docked position

   void TextEntryComplete(TextEntry* entry) override { }
   void ButtonClicked(ClickButton* button, double time) override;

   void Poll() override;

   //width resize via a left-edge drag handle (the panel stays docked to the right edge, so
   //dragging the handle changes the panel's width). CanBeMoved() stays false so the body can't
   //be used to drag the whole panel off its dock.
   bool TestClick(float x, float y, bool right, bool testOnly = false) override;
   void OnClicked(float x, float y, bool right) override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;
   bool MouseScrolled(float x, float y, float scrollX, float scrollY, bool isSmoothScroll, bool isInvertedScroll) override;

private:
   //background audio-file index entry (declared early so index methods below can reference it)
   struct IndexedSample
   {
      std::string path; //full path, used to grab the sample
      std::string lowerName; //lowercased filename, used for fast substring matching
   };

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   void UpdateResults();
   //background sample indexing (Bitwig/Ableton-style): folders are walked ONCE off the UI thread
   //into an in-memory index; searching then just filters that index, so it never touches disk per
   //keystroke (which was freezing/crashing the app on large libraries).
   void RebuildIndex();
   void StopIndexThread();
   void IndexWorker(std::vector<std::string> locations);
   //the index is cached to disk so startup just loads it instantly instead of re-walking folders;
   //a fresh walk only happens on first run, on rescan, or when locations change
   bool LoadIndex();
   void SaveIndex(const std::vector<IndexedSample>& entries);
   static std::string GetIndexFilePath();
   void BrowseForLocation();
   void RemoveLocation(int index);
   void LoadLocations();
   void SaveLocations();
   void SaveWidth();
   void LoadWidth();
   static std::string GetLocationsFilePath();
   static std::string GetWidthFilePath();
   static std::string TruncatePathForDisplay(const std::string& path, int maxChars);

   static constexpr int kMaxResults = 10; //module results
   static constexpr int kVisibleSampleRows = 30; //sample rows drawn on screen at once (virtualized window)
   static constexpr int kMaxSampleMatches = 4000; //full match list you can scroll through
   static constexpr int kMaxLocationRows = 8;
   static constexpr int kMinWidth = 150;
   static constexpr int kMaxWidth = 640;
   static constexpr int kHandleWidth = 9; //left-edge scrollbar strip
   static constexpr int kContentX = 14; //content starts to the right of the scrollbar strip

   std::string mSearchTextBound; //required by TextEntry's constructor; the live typed text is polled every frame from mSearchEntry->GetText() instead (see DrawModule), since the bound var is only updated on enter/blur
   std::string mLastSearchText;
   int mLastResultsIndexCount{ -1 }; //so the browse list refreshes when the index finishes loading
   TextEntry* mSearchEntry{ nullptr };

   std::vector<ModuleFactory::Spawnable> mModuleResults;
   std::vector<juce::String> mSampleResults; //full ranked match list (scrollable)
   int mSampleScroll{ 0 }; //index of the first sample row shown (virtualized list)

   std::array<ClickButton*, kMaxResults> mModuleButtons{ nullptr };
   std::array<ClickButton*, kVisibleSampleRows> mSampleButtons{ nullptr };
   std::array<ClickButton*, kVisibleSampleRows> mPreviewButtons{ nullptr }; //▶ buttons beside each sample row

   //Bitwig-style "Locations": user-added root folders searched in addition to the default samples path
   std::vector<std::string> mSearchLocations;
   ClickButton* mAddLocationButton{ nullptr };
   ClickButton* mRescanButton{ nullptr };
   std::array<ClickButton*, kMaxLocationRows> mRemoveLocationButtons{ nullptr };

   //background audio-file index (built once off the UI thread, cached to disk, filtered per keystroke)
   static constexpr int kMaxIndexed = 200000; //safety cap on library size
   std::vector<IndexedSample> mSampleIndex;
   std::mutex mIndexMutex;
   std::thread mIndexThread;
   std::atomic<bool> mIndexing{ false };
   std::atomic<bool> mStopIndex{ false };
   std::atomic<int> mIndexedCount{ 0 };

   int mWidth{ 230 };
   int mHeight{ 260 };

   int mUserWidth{ 230 }; //panel width
   bool mDraggingResize{ false };
   bool mHoveringResizeHandle{ false };

   //left-edge strip repurposed as a draggable scrollbar for the results list (click/drag to scrub
   //through all matches quickly instead of wheel-scrolling). Track geometry captured each draw.
   bool mDraggingScroll{ false };
   float mScrollTrackTop{ 0 };
   float mScrollTrackH{ 0 };
   int mScrollTotal{ 0 };
   int mScrollVisible{ 0 };

   // Sample preview via afplay subprocess (macOS native, correct threading)
   pid_t mPreviewPid{ -1 };
   int mPreviewingRow{ -1 };
   void StartPreview(const std::string& path);
   void StopPreview();
};

extern SearchPanel* TheSearchPanel;
