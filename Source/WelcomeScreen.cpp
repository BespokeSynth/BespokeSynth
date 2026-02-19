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
   const float kSaveStateButtonPadX = 10;
   const float kSaveStateButtonPadY = 10;
}

void WelcomeScreen::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mDocsLinkButton = new ClickButton(this, "bespokesynth.com/docs", 111, 31);
   mDiscordLinkButton = new ClickButton(this, "bespoke discord", 324, 31);
   mTutorialVideoLinkButton = new ClickButton(this, "youtu.be/SYBc8X2IxqM", 176, 50);

   mWidth = ofGetWidth() / TheSynth->GetUIScale() - 100;
   mHeight = ofGetHeight() / TheSynth->GetUIScale() - 200;

   int x = kSaveStateButtonStartX;
   int y = 100;
   using namespace juce;
   File dir(ofToDataPath("savestate"));
   Array<File> files;
   dir.findChildFiles(files, File::findFiles, false);
   std::sort(files.begin(), files.end(),
             [](const File& lhs, const File& rhs)
             {
                return lhs.getLastModificationTime().toMilliseconds() > rhs.getLastModificationTime().toMilliseconds();
             });
   for (auto& file : files)
   {
      if (mRecentFiles.size() >= 10 || y + kSaveStateButtonHeight + 20 > mHeight)
         break;

      if (file.getFileExtension() == ".bsk")
      {
         RecentFile recentFile;
         recentFile.mFile = file;
         recentFile.mButton = new ClickButton(this, ("recentfile" + ofToString((int)mRecentFiles.size())).c_str(), x, y);
         recentFile.mButton->SetOverrideDisplayName("");
         recentFile.mButton->SetDimensions(kSaveStateButtonWidth, kSaveStateButtonHeight);
         mRecentFiles.push_back(recentFile);

         x += kSaveStateButtonWidth + kSaveStateButtonPadX;
         if (x + kSaveStateButtonWidth + kSaveStateButtonPadX > mWidth)
         {
            x = kSaveStateButtonStartX;
            y += kSaveStateButtonHeight + kSaveStateButtonPadY;
         }
      }
   }

   mCloseButton = new ClickButton(this, "close", 10, mHeight - 20);
}

void WelcomeScreen::Show()
{
   SetPosition(50 / TheSynth->GetUIScale() - TheSynth->GetDrawOffset().x, 150 / TheSynth->GetUIScale() - TheSynth->GetDrawOffset().y);

   SetShowing(true);
}

void WelcomeScreen::DrawModule()
{
   DrawTextBold("welcome to bespoke!", 15, 20, 18);

   DrawTextNormal("documentation:", 20, 43);
   mDocsLinkButton->Draw();
   DrawTextNormal("join the ", 280, 43);
   mDiscordLinkButton->Draw();
   DrawTextNormal("video overview available at:", 20, 62);
   mTutorialVideoLinkButton->Draw();

   DrawTextBold("recent files:", kSaveStateButtonStartX, 90);

   for (int i = 0; i < (int)mRecentFiles.size(); ++i)
   {
      mRecentFiles[i].mButton->Draw();
      ofRectangle rect = mRecentFiles[i].mButton->GetRect(K(local));
      ofPushMatrix();

      ofRectangle imageRect = rect;
      imageRect.height *= float(kScreenshotHeight) / kScreenshotWidth;
      constexpr float padding = 2;
      imageRect.x += padding;
      imageRect.y += padding;
      imageRect.width -= padding * 2;
      imageRect.height -= padding * 2;
      if (mRecentFiles[i].mScreenshotImageHandle == -1)
      {
         FileStreamIn in(ofToDataPath(mRecentFiles[i].mFile.getFullPathName().toStdString()));
         unsigned char* screenshotData = nullptr;
         int screenshotSize = 0;
         std::string jsonLayoutString;
         ModularSynth::LoadStateHeader(in, screenshotData, screenshotSize, jsonLayoutString);
         if (screenshotData != nullptr)
            mRecentFiles[i].mScreenshotImageHandle = nvgCreateImageMem(gNanoVG, 0, screenshotData, screenshotSize);
         else
            mRecentFiles[i].mScreenshotImageHandle = nvgCreateImage(gNanoVG, ofToResourcePath("bespoke_default.png").c_str(), 0);
      }

      if (mRecentFiles[i].mScreenshotImageHandle != -1)
      {
         NVGpaint pattern = nvgImagePattern(gNanoVG, imageRect.x, imageRect.y, imageRect.width, imageRect.height, 0, mRecentFiles[i].mScreenshotImageHandle, 1.0f);
         nvgBeginPath(gNanoVG);
         nvgRoundedRect(gNanoVG, imageRect.x, imageRect.y, imageRect.width, imageRect.height, 5);
         nvgFillPaint(gNanoVG, pattern);
         nvgFill(gNanoVG);
      }

      ofClipWindow(rect.x, rect.y, rect.width, rect.height, K(intersectWithExisting));
      DrawTextNormal(mRecentFiles[i].mFile.getFileNameWithoutExtension().toStdString(), rect.x + 3, rect.getMaxY() - 18);
      juce::RelativeTime timeSinceModified = juce::Time::getCurrentTime() - mRecentFiles[i].mFile.getLastModificationTime();
      DrawTextNormal(("saved " + timeSinceModified.getApproximateDescription() + " ago").toStdString(), rect.x + 3, rect.getMaxY() - 4, 10);
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
   {
      HelpDisplay::OpenTutorialVideoLink();
   }
   if (button == mDocsLinkButton)
   {
      HelpDisplay::OpenDocsLink();
   }
   if (button == mDiscordLinkButton)
   {
      HelpDisplay::OpenDiscordLink();
   }

   if (button == mCloseButton)
      SetShowing(false);
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
