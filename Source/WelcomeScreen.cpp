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
   const float kSaveStateButtonWidth = 200;
   const float kSaveStateButtonHeight = 180;
   const float kSaveStateButtonStartX = 20;
   const float kSaveStateButtonStartY = 110;
   const float kSaveStateButtonPadX = 10;
   const float kSaveStateButtonPadY = 10;
}

void WelcomeScreen::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK(kSaveStateButtonStartX, 30);
   BUTTON(mNewPatchButton, "new patch");
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mLoadPatchButton, "load patch");
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mShowHelpButton, "help");
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mShowSettingsButton, "settings");
   UIBLOCK_SHIFTRIGHT();
   ENDUIBLOCK0();

   mDocsLinkButton = new ClickButton(this, "bespokesynth.com/docs", 111, 53);
   mDiscordLinkButton = new ClickButton(this, "bespoke discord", 324, 53);
   mTutorialVideoLinkButton = new ClickButton(this, "youtu.be/SYBc8X2IxqM", 176, 72);

   mWidth = ofGetWidth() / TheSynth->GetUIScale() - 100;
   mHeight = ofGetHeight() / TheSynth->GetUIScale() - 200;

   const int kMaxDesiredFiles = 10;

   int x = kSaveStateButtonStartX;
   int y = kSaveStateButtonStartY + kSaveStateButtonPadY;

   using namespace juce;

   ofxJSONElement workspaceData;
   workspaceData.open(TheSynth->GetWorkspaceDataPath());
   for (int i = 0; i < workspaceData["recent_files"].size(); ++i)
   {
      RecentFile recentFile;
      recentFile.mFile = File(workspaceData["recent_files"][i]["file"].asString());
      recentFile.mTime = Time::fromISO8601(workspaceData["recent_files"][i]["time"].asString());
      recentFile.mRecentlyOpened = !workspaceData["recent_files"][i]["saved"].asBool();
      if (recentFile.mFile.existsAsFile())
         mRecentFiles.insert(mRecentFiles.begin(), recentFile);
   }

   if (mRecentFiles.size() < kMaxDesiredFiles)
   {
      File dir(ofToDataPath("savestate"));
      Array<File> files;
      dir.findChildFiles(files, File::findFiles, false);
      std::sort(files.begin(), files.end(),
                [](const File& lhs, const File& rhs)
                {
                   return lhs.getLastModificationTime().toMilliseconds() > rhs.getLastModificationTime().toMilliseconds();
                });

      for (int i = 0; i < files.size() && mRecentFiles.size() < kMaxDesiredFiles; ++i)
      {
         if (files[i].getFileExtension() != ".bsk")
            continue;

         bool alreadyInList = false;
         for (int j = 0; j < mRecentFiles.size(); ++j)
         {
            if (mRecentFiles[j].mFile == files[i])
               alreadyInList = true;
         }

         if (alreadyInList)
            continue;

         RecentFile recentFile;
         recentFile.mFile = files[i];
         recentFile.mTime = files[i].getLastModificationTime();
         recentFile.mRecentlyOpened = false;
         mRecentFiles.push_back(recentFile);
      }
   }

   for (auto& recentFile : mRecentFiles)
   {
      if (y + kSaveStateButtonHeight + 20 > mHeight)
         break;

      recentFile.mButton = new ClickButton(this, ("recentfile" + ofToString((int)mRecentFiles.size())).c_str(), x, y);
      recentFile.mButton->SetOverrideDisplayName("");
      recentFile.mButton->SetDimensions(kSaveStateButtonWidth, kSaveStateButtonHeight);

      x += kSaveStateButtonWidth + kSaveStateButtonPadX;
      if (x + kSaveStateButtonWidth + kSaveStateButtonPadX > mWidth)
      {
         x = kSaveStateButtonStartX;
         y += kSaveStateButtonHeight + kSaveStateButtonPadY;
      }
   }

   mCloseButton = new ClickButton(this, "close", 10, mHeight - 20);
}

void WelcomeScreen::Show()
{
   SetPosition(50 / TheSynth->GetUIScale() - TheSynth->GetDrawOffset().x, 150 / TheSynth->GetUIScale() - TheSynth->GetDrawOffset().y);

   SetShowing(true);
   TheSynth->MoveToFront(this);
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

   mNewPatchButton->Draw();
   mLoadPatchButton->Draw();
   mShowHelpButton->Draw();
   mShowSettingsButton->Draw();

   DrawTextNormal("documentation:", 20, 65);
   mDocsLinkButton->Draw();
   DrawTextNormal("join the ", 280, 65);
   mDiscordLinkButton->Draw();
   DrawTextNormal("video overview available at:", 20, 84);
   mTutorialVideoLinkButton->Draw();

   DrawTextBold("recent files:", kSaveStateButtonStartX, kSaveStateButtonStartY);

   for (int i = 0; i < (int)mRecentFiles.size(); ++i)
   {
      if (mRecentFiles[i].mButton == nullptr)
         continue;

      auto& project = mRecentFiles[i];

      project.mButton->Draw();
      ofRectangle rect = mRecentFiles[i].mButton->GetRect(K(local));
      ofPushMatrix();

      ofRectangle imageRect = rect;
      imageRect.height *= float(kScreenshotHeight) / kScreenshotWidth;
      constexpr float padding = 2;
      imageRect.x += padding;
      imageRect.y += padding;
      imageRect.width -= padding * 2;
      imageRect.height -= padding * 2;
      if (project.mScreenshotImageHandle == -1)
      {
         FileStreamIn in(ofToDataPath(mRecentFiles[i].mFile.getFullPathName().toStdString()));
         unsigned char* screenshotData = nullptr;
         int screenshotSize = 0;
         std::string jsonLayoutString;
         ModularSynth::LoadStateHeader(in, screenshotData, screenshotSize, jsonLayoutString);
         //Demo projects use custom screenshots.
         std::string demoScreenshot = TryGetDemoFileScreenshot(project.mFile.getFileName());
         bool isDemo = !demoScreenshot.empty();
         if (!isDemo)
         {
            if (screenshotData != nullptr)
               project.mScreenshotImageHandle = nvgCreateImageMem(gNanoVG, 0, screenshotData, screenshotSize);
            else
               project.mScreenshotImageHandle = nvgCreateImage(gNanoVG, ofToResourcePath("bespoke_default.png").c_str(), 0);
         }
         else
         {
            project.mScreenshotImageHandle = nvgCreateImage(gNanoVG, ofToResourcePath("example_project_images/" + demoScreenshot).c_str(), 0);
         }
      }

      if (project.mScreenshotImageHandle != -1)
      {
         NVGpaint pattern = nvgImagePattern(gNanoVG, imageRect.x, imageRect.y, imageRect.width, imageRect.height, 0, mRecentFiles[i].mScreenshotImageHandle, 1.0f);
         nvgBeginPath(gNanoVG);
         nvgRoundedRect(gNanoVG, imageRect.x, imageRect.y, imageRect.width, imageRect.height, 5);
         nvgFillPaint(gNanoVG, pattern);
         nvgFill(gNanoVG);
      }

      ofClipWindow(rect.x, rect.y, rect.width, rect.height, K(intersectWithExisting));
      DrawTextNormal(project.mFile.getFileNameWithoutExtension().toStdString(), rect.x + 3, rect.getMaxY() - 18);
      juce::RelativeTime timeSince = juce::Time::getCurrentTime() - mRecentFiles[i].mTime;
      juce::String prefix;
      if (project.mRecentlyOpened)
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
      if (button == mRecentFiles[i].mButton)
         TheSynth->LoadState(mRecentFiles[i].mFile.getFullPathName().toStdString());
   }

   if (button == mTutorialVideoLinkButton)
      HelpDisplay::OpenTutorialVideoLink();
   if (button == mDocsLinkButton)
      HelpDisplay::OpenDocsLink();
   if (button == mDiscordLinkButton)
      HelpDisplay::OpenDiscordLink();
   if (button == mNewPatchButton)
      TheSynth->ReloadInitialLayout();
   if (button == mLoadPatchButton)
      TheSynth->LoadStatePopup();
   if (button == mShowHelpButton)
      TheTitleBar->ShowHelp();
   if (button == mShowSettingsButton)
      TheSynth->GetUserPrefsEditor()->Show();

   if (button == mCloseButton)
      SetShowing(false);
}

std::string WelcomeScreen::TryGetDemoFileScreenshot(juce::String fileName)
{
   if (fileName == "example__looper_recorder.bsk")
      return "example__looper_recorder.png";
   if (fileName == "example__scripting.bsk")
      return "example__scripting.png";
   if (fileName == "example__feedback.bsk")
      return "example__feedback.png";
   if (fileName == "example__feedback_distortion_pluck_bass_echo.bsk")
      return "example__feedback_distortion_pluck_bass_echo.png";
   if (fileName == "example__drumsynth_chance_sequence.bsk")
      return "example__drumsynth_chance_sequence.png";
   if (fileName == "example__sequencing_sequencers.bsk")
      return "example__sequencing_sequencers.png";
   if (fileName == "example__arpeggiation.bsk")
      return "example__arpeggiation.png";
   if (fileName == "example__dj_turntables.bsk")
      return "example__dj_turntables.png";
   return "";
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
