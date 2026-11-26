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
/*
  ==============================================================================

    WelcomeScreen.cpp
    Created: 31 Jan 2026
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "WelcomeScreen.h"
#include "ModularSynth.h"
#include "SynthGlobals.h"
#include "UserPrefs.h"
#include "PatchCable.h"
#include "HelpDisplay.h"
#include "TitleBar.h"
#include "UIControlMacros.h"
#include "UserPrefsEditor.h"

#include <tuple>

#include "juce_opengl/juce_opengl.h"
using namespace juce::gl;

#include "nanovg/nanovg.h"
#define NANOVG_GLES2_IMPLEMENTATION
#include "nanovg/nanovg_gl.h"

WelcomeScreen::WelcomeScreen()
: IDrawableModule(1150, 50)
{
}

WelcomeScreen::~WelcomeScreen()
{
}

namespace
{
   const float kSaveStateButtonStartX = 20;
   const float kSaveStateButtonStartY = 110;
   const float kSaveStateButtonPadX = 10;
   const float kSaveStateButtonPadY = 10;
   const float kSaveStateButtonLabelHeight = 32;
   const float kSaveStateButtonImagePad = 2;
   const float kSaveStateGridTop = kSaveStateButtonStartY + kSaveStateButtonPadY;
   const float kMinSaveStateGridHeight = 110; //keep enough space for at least one usable row
   const float kMinButtonWidth = 100; //keep labels readable
   const float kPreferredImageWidth = 200; //beyond this, showing more files is better than showing bigger files
   const float kMaxImageWidth = WelcomeScreen::kScreenshotWidth * 2; //don't blow screenshots up too much if there are only a few files
   const float kImageAspect = float(WelcomeScreen::kScreenshotWidth) / WelcomeScreen::kScreenshotHeight;

   //width of the largest screenshot that can fit in a button, alongside the label
   float GetFitImageWidth(float buttonWidth, float buttonHeight)
   {
      float availableWidth = buttonWidth - kSaveStateButtonImagePad * 2;
      float availableHeight = buttonHeight - kSaveStateButtonLabelHeight - kSaveStateButtonImagePad * 2;
      return MAX(0, MIN(availableWidth, availableHeight * kImageAspect));
   }
}

void WelcomeScreen::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK(kSaveStateButtonStartX, 30);
   BUTTON(mShowHelpButton, "help");
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mShowSettingsButton, "settings");
   UIBLOCK_SHIFTRIGHT();
   ENDUIBLOCK0();

   mDocsLinkButton = new ClickButton(this, "bespokesynth.com/docs", 111, 53);
   mDiscordLinkButton = new ClickButton(this, "bespoke discord", 324, 53);
   mTutorialVideoLinkButton = new ClickButton(this, "youtu.be/SYBc8X2IxqM", 176, 72);

   for (int i = 0; i < kMaxRecentFiles; ++i)
   {
      mRecentFileButtons[i] = new ClickButton(this, ("recentfile" + ofToString(i + 1)).c_str(), kSaveStateButtonStartX, kSaveStateGridTop);
      mRecentFileButtons[i]->SetOverrideDisplayName("");
   }

   mCloseButton = new ClickButton(this, "close", mWidth - 40, 10);

   UpdateRecentFiles();
   UpdateLayout();
}

void WelcomeScreen::UpdateRecentFiles()
{
   const int kMaxDesiredFiles = 10;

   using namespace juce;

   std::vector<RecentFile> recentFiles;
   auto alreadyHasFile = [&recentFiles](const File& file)
   {
      return std::any_of(recentFiles.begin(), recentFiles.end(), [&file](const RecentFile& recentFile)
                         {
                            return recentFile.mFile == file;
                         });
   };

   ofxJSONElement workspaceData;
   workspaceData.open(TheSynth->GetWorkspaceDataPath());
   for (int i = 0; i < workspaceData["recent_files"].size(); ++i)
   {
      std::string path = workspaceData["recent_files"][i]["file"].asString();
#if BESPOKE_IOS
      //our data folder can move when the app gets updated
      const std::string kDataFolder = "/Documents/BespokeSynth/";
      size_t dataFolderPos = path.find(kDataFolder);
      if (dataFolderPos != std::string::npos && path.find("/Containers/Data/Application/") != std::string::npos)
         path = ofToDataPath(path.substr(dataFolderPos + kDataFolder.length()));
#endif

      RecentFile recentFile;
      recentFile.mTarget = SaveTarget(path, workspaceData["recent_files"][i]["bookmark"].asString());
      recentFile.mFile = File(recentFile.mTarget.GetPath());
      recentFile.mTime = Time::fromISO8601(workspaceData["recent_files"][i]["time"].asString());
      recentFile.mRecentlyOpened = !workspaceData["recent_files"][i]["saved"].asBool();
      if (!recentFile.mTarget.IsAccessible())
         continue;
      if (recentFile.mTarget.GetBookmark().empty() && !recentFile.mFile.existsAsFile())
         continue;

      //this one is more recent than everything that we've gathered so far
      recentFiles.erase(std::remove_if(recentFiles.begin(), recentFiles.end(), [&recentFile](const RecentFile& other)
                                       {
                                          return other.mFile == recentFile.mFile;
                                       }),
                        recentFiles.end());
      recentFiles.insert(recentFiles.begin(), recentFile);
   }

   //include the most recent autosave as the first entry
   File autosaveDir(ofToDataPath("savestate/autosave"));
   Array<File> autosaveFiles;
   autosaveDir.findChildFiles(autosaveFiles, File::findFiles, false, "*.bsk;*.bskt");
   if (!autosaveFiles.isEmpty())
   {
      std::sort(autosaveFiles.begin(), autosaveFiles.end(),
                [](const File& lhs, const File& rhs)
                {
                   return lhs.getLastModificationTime().toMilliseconds() > rhs.getLastModificationTime().toMilliseconds();
                });
      RecentFile autosaveFile;
      autosaveFile.mFile = autosaveFiles[0];
      autosaveFile.mTarget = SaveTarget(autosaveFile.mFile.getFullPathName().toStdString());
      autosaveFile.mTime = autosaveFiles[0].getLastModificationTime();
      autosaveFile.mRecentlyOpened = false;
      for (auto iter = recentFiles.begin(); iter != recentFiles.end(); ++iter)
      {
         if (autosaveFile.mTime > iter->mTime)
         {
            if (!alreadyHasFile(autosaveFile.mFile))
               recentFiles.insert(iter, autosaveFile);
            break;
         }
      }
   }

   if (recentFiles.size() < kMaxDesiredFiles)
   {
      File dir(ofToDataPath("savestate"));
      Array<File> files;
      dir.findChildFiles(files, File::findFiles, false);
      std::sort(files.begin(), files.end(),
                [](const File& lhs, const File& rhs)
                {
                   return lhs.getLastModificationTime().toMilliseconds() > rhs.getLastModificationTime().toMilliseconds();
                });

      for (int i = 0; i < files.size() && recentFiles.size() < kMaxDesiredFiles; ++i)
      {
         if (files[i].getFileExtension() != ".bsk")
            continue;

         if (alreadyHasFile(files[i]))
            continue;

         RecentFile recentFile;
         recentFile.mFile = files[i];
         recentFile.mTarget = SaveTarget(recentFile.mFile.getFullPathName().toStdString());
         recentFile.mTime = files[i].getLastModificationTime();
         recentFile.mRecentlyOpened = false;
         recentFiles.push_back(recentFile);
      }
   }

   if (recentFiles.size() > kMaxRecentFiles)
      recentFiles.resize(kMaxRecentFiles);

   std::lock_guard<std::recursive_mutex> renderLock(TheSynth->GetRenderLock());

   //hold on to the screenshots of files that haven't changed
   for (const auto& previousFile : mRecentFiles)
   {
      if (previousFile.mScreenshotImageHandle == -1)
         continue;

      auto match = std::find_if(recentFiles.begin(), recentFiles.end(), [&previousFile](const RecentFile& recentFile)
                                {
                                   return recentFile.mFile == previousFile.mFile && recentFile.mTime == previousFile.mTime && recentFile.mTarget.IsAvailableLocally() == previousFile.mTarget.IsAvailableLocally();
                                });
      if (match != recentFiles.end())
         match->mScreenshotImageHandle = previousFile.mScreenshotImageHandle;
      else
         mImagesToDelete.push_back(previousFile.mScreenshotImageHandle);
   }

   mRecentFiles = std::move(recentFiles);
}

void WelcomeScreen::UpdateLayout()
{
   float uiScreenWidth = ofGetWidth() / TheSynth->GetUIScale();
   float uiScreenHeight = ofGetHeight() / TheSynth->GetUIScale();
   mWidth = uiScreenWidth - 100;
   mHeight = uiScreenHeight - 150;

   //on small screens, use up more of the vertical margin to make room for a row of recent files
   const float kMinHeight = kSaveStateGridTop + kMinSaveStateGridHeight + kSaveStateButtonPadY;
   if (mHeight < kMinHeight)
      mHeight = MAX(mHeight, MIN(kMinHeight, uiScreenHeight - 20));

   SetPosition(uiScreenWidth / 2 - mWidth / 2, uiScreenHeight / 2 - mHeight / 2);
   mCloseButton->SetPosition(mWidth - 40, 10);

   const int numFiles = (int)mRecentFiles.size();
   for (int i = numFiles; i < kMaxRecentFiles; ++i)
      mRecentFileButtons[i]->SetShowing(false);
   if (numFiles == 0)
      return;

   const float gridWidth = mWidth - kSaveStateButtonStartX * 2;
   const float gridHeight = mHeight - kSaveStateGridTop - kSaveStateButtonPadY;

   //find the grid that covers the most space with screenshots, without letting oversized screenshots crowd out other files
   int numCols = 1;
   int numRows = 1;
   float bestScore = -1;
   float bestArea = -1;
   int bestNumShown = 0;
   for (int cols = 1; cols <= numFiles; ++cols)
   {
      const int maxRows = (numFiles + cols - 1) / cols;
      for (int rows = 1; rows <= maxRows; ++rows)
      {
         float buttonWidth = (gridWidth - kSaveStateButtonPadX * (cols - 1)) / cols;
         float buttonHeight = (gridHeight - kSaveStateButtonPadY * (rows - 1)) / rows;
         if ((cols > 1 && buttonWidth < kMinButtonWidth) || (rows > 1 && buttonHeight < kSaveStateButtonLabelHeight))
            continue;

         float imageWidth = GetFitImageWidth(buttonWidth, buttonHeight);
         float cappedWidth = MIN(imageWidth, kPreferredImageWidth);
         int numShown = MIN(numFiles, cols * rows);
         float score = numShown * cappedWidth * cappedWidth;
         float area = numShown * imageWidth * imageWidth;
         //if there's no space for screenshots at all, every score is zero, so this comes down to showing the most files
         if (std::tie(score, area, numShown) > std::tie(bestScore, bestArea, bestNumShown))
         {
            bestScore = score;
            bestArea = area;
            bestNumShown = numShown;
            numCols = cols;
            numRows = rows;
         }
      }
   }

   float buttonWidth = (gridWidth - kSaveStateButtonPadX * (numCols - 1)) / numCols;
   float buttonHeight = (gridHeight - kSaveStateButtonPadY * (numRows - 1)) / numRows;
   buttonWidth = ofClamp(buttonWidth, 1, kMaxImageWidth + kSaveStateButtonImagePad * 2);
   buttonHeight = ofClamp(buttonHeight, kSaveStateButtonLabelHeight, kMaxImageWidth / kImageAspect + kSaveStateButtonLabelHeight + kSaveStateButtonImagePad * 2);

   for (int i = 0; i < numFiles; ++i)
   {
      int col = i % numCols;
      int row = i / numCols;
      ClickButton* button = mRecentFileButtons[i];
      button->SetShowing(row < numRows);
      button->SetPosition(kSaveStateButtonStartX + col * (buttonWidth + kSaveStateButtonPadX), kSaveStateGridTop + row * (buttonHeight + kSaveStateButtonPadY));
      button->SetDimensions(buttonWidth, buttonHeight);
   }
}

void WelcomeScreen::Show()
{
   UpdateRecentFiles();
   UpdateLayout();
   TheSynth->PushModalFocusItem(this);
}

void WelcomeScreen::DrawModule()
{
   ofPushStyle();
   ofFill();
   ofColor color = GetColor(kModuleCategory_Other);
   color.r *= .25f;
   color.g *= .25f;
   color.b *= .25f;
   ofSetColor(color);
   ofRect(0, 0, mWidth, mHeight);
   ofPopStyle();

   DrawTextBold("welcome to bespoke!", 15, 20, 18);

   mShowHelpButton->Draw();
   mShowSettingsButton->Draw();

   DrawTextNormal("documentation:", 20, 65);
   mDocsLinkButton->Draw();
   DrawTextNormal("join the ", 280, 65);
   mDiscordLinkButton->Draw();
   DrawTextNormal("video overview available at:", 20, 84);
   mTutorialVideoLinkButton->Draw();

   DrawTextBold("recent files:", kSaveStateButtonStartX, kSaveStateButtonStartY);

   if (mScreenshotContextGeneration != gNanoVGContextGeneration)
   {
      //the screenshots belonged to the old nanovg context
      mScreenshotContextGeneration = gNanoVGContextGeneration;
      for (auto& recentFile : mRecentFiles)
         recentFile.mScreenshotImageHandle = -1;
      mImagesToDelete.clear();
   }

   for (int image : mImagesToDelete)
      nvgDeleteImage(gNanoVG, image);
   mImagesToDelete.clear();

   for (int i = 0; i < (int)mRecentFiles.size(); ++i)
   {
      if (!mRecentFileButtons[i]->IsShowing())
         continue;

      mRecentFileButtons[i]->Draw();
      ofRectangle rect = mRecentFileButtons[i]->GetRect(K(local));
      ofPushMatrix();

      ofRectangle imageRect;
      imageRect.width = GetFitImageWidth(rect.width, rect.height);
      imageRect.height = imageRect.width / kImageAspect;
      imageRect.x = rect.x + (rect.width - imageRect.width) / 2;
      imageRect.y = rect.y + (rect.height - kSaveStateButtonLabelHeight - imageRect.height) / 2;
      if (mRecentFiles[i].mScreenshotImageHandle == -1)
      {
         unsigned char* screenshotData = nullptr;
         int screenshotSize = 0;
         //reading a file that isn't downloaded would block until it is
         std::unique_ptr<juce::InputStream> stream;
         if (mRecentFiles[i].mTarget.IsAvailableLocally())
            stream = mRecentFiles[i].mTarget.CreateInputStream();
         if (stream != nullptr)
         {
            FileStreamIn in(std::move(stream));
            std::string jsonLayoutString;
            ModularSynth::LoadStateHeader(in, screenshotData, screenshotSize, jsonLayoutString);
            FileStreamIn::s32BitMode = false;
         }
         if (screenshotData != nullptr)
            mRecentFiles[i].mScreenshotImageHandle = nvgCreateImageMem(gNanoVG, 0, screenshotData, screenshotSize);
         else
            mRecentFiles[i].mScreenshotImageHandle = nvgCreateImage(gNanoVG, ofToResourcePath("bespoke_default.png").c_str(), 0);
         delete[] screenshotData;
      }

      if (mRecentFiles[i].mScreenshotImageHandle != -1 && imageRect.width > 0)
      {
         NVGpaint pattern = nvgImagePattern(gNanoVG, imageRect.x, imageRect.y, imageRect.width, imageRect.height, 0, mRecentFiles[i].mScreenshotImageHandle, 1.0f);
         nvgBeginPath(gNanoVG);
         nvgRoundedRect(gNanoVG, imageRect.x, imageRect.y, imageRect.width, imageRect.height, 5);
         nvgFillPaint(gNanoVG, pattern);
         nvgFill(gNanoVG);
      }

      ofClipWindow(rect.x, rect.y, rect.width, rect.height, K(intersectWithExisting));
      DrawTextNormal(mRecentFiles[i].mFile.getFileNameWithoutExtension().toStdString(), rect.x + 3, rect.getMaxY() - 18);
      juce::RelativeTime timeSince = juce::Time::getCurrentTime() - mRecentFiles[i].mTime;
      juce::String prefix;
      if (mRecentFiles[i].mRecentlyOpened)
         prefix = "opened ";
      else
         prefix = "saved ";
      DrawTextNormal((prefix + timeSince.getApproximateDescription() + " ago").toStdString(), rect.x + 3, rect.getMaxY() - 4, 10);
      ofPopMatrix();
   }

   mCloseButton->Draw();
}

bool WelcomeScreen::MouseScrolled(float x, float y, float scrollX, float scrollY, bool isSmoothScroll, bool isInvertedScroll)
{
   mRecentFilesScrollOffset = ofClamp(mRecentFilesScrollOffset - (scrollX + scrollY) * 30, 0, ((int)mRecentFiles.size() - 1) * 105);
   return false;
}

void WelcomeScreen::ButtonClicked(ClickButton* button, double time)
{
   for (int i = 0; i < (int)mRecentFiles.size(); ++i)
   {
      if (button == mRecentFileButtons[i])
      {
         mRecentFiles[i].mTarget.EnsureAvailable([](const SaveTarget& target)
                                                 {
                                                    TheSynth->LoadState(target);
                                                 });
         return; //loading can replace this screen
      }
   }

   if (button == mTutorialVideoLinkButton)
      HelpDisplay::OpenTutorialVideoLink();
   if (button == mDocsLinkButton)
      HelpDisplay::OpenDocsLink();
   if (button == mDiscordLinkButton)
      HelpDisplay::OpenDiscordLink();
   if (button == mShowHelpButton)
   {
      TheSynth->PopModalFocusItem();
      TheTitleBar->ShowHelp();
   }
   if (button == mShowSettingsButton)
   {
      TheSynth->PopModalFocusItem();
      TheSynth->GetUserPrefsEditor()->Show();
   }

   if (button == mCloseButton)
      TheSynth->PopModalFocusItem();
}

void WelcomeScreen::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void WelcomeScreen::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void WelcomeScreen::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void WelcomeScreen::TextEntryComplete(TextEntry* entry)
{
}

void WelcomeScreen::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
}

void WelcomeScreen::RadioButtonUpdated(RadioButton* radio, int oldVal, double time)
{
}

std::vector<IUIControl*> WelcomeScreen::ControlsToNotSetDuringLoadState() const
{
   return GetUIControls();
}

std::vector<IUIControl*> WelcomeScreen::ControlsToIgnoreInSaveState() const
{
   return GetUIControls();
}
