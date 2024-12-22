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
//  HelpDisplay.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 2/7/15.
//
//

#include "HelpDisplay.h"

#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "TitleBar.h"
#include "EffectChain.h"
#include "UserPrefs.h"
#include "VersionInfo.h"

#include "juce_gui_basics/juce_gui_basics.h"
#include "juce_opengl/juce_opengl.h"

bool HelpDisplay::sShowTooltips = false;
bool HelpDisplay::sTooltipsLoaded = false;
std::list<HelpDisplay::ModuleTooltipInfo> HelpDisplay::sTooltips;

HelpDisplay::HelpDisplay()
{
   LoadHelp();

   sShowTooltips = UserPrefs.show_tooltips_on_load.Get();
   LoadTooltips();
}

void HelpDisplay::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mShowTooltipsCheckbox = new Checkbox(this, "show tooltips", 3, 22, &sShowTooltips);
   mCopyBuildInfoButton = new ClickButton(this, "copy build info", mShowTooltipsCheckbox, kAnchor_Right);
   mDumpModuleInfoButton = new ClickButton(this, "dump module info", 200, 22);
   mDoModuleScreenshotsButton = new ClickButton(this, "do screenshots", mDumpModuleInfoButton, kAnchor_Right);
   mDoModuleDocumentationButton = new ClickButton(this, "do documentation", mDoModuleScreenshotsButton, kAnchor_Right);
   mTutorialVideoLinkButton = new ClickButton(this, "youtu.be/SYBc8X2IxqM", 160, 61);
   mDocsLinkButton = new ClickButton(this, "bespokesynth.com/docs", 95, 80);
   mDiscordLinkButton = new ClickButton(this, "bespoke discord", 304, 80);

   //mDumpModuleInfo->SetShowing(false);
}

HelpDisplay::~HelpDisplay()
{
}

void HelpDisplay::LoadHelp()
{
   juce::File file(ofToResourcePath("help.txt").c_str());
   if (file.existsAsFile())
   {
      std::string help = file.loadFileAsString().toStdString();
      ofStringReplace(help, "\r", "");
      mHelpText = ofSplitString(help, "\n");
   }

   mMaxScrollAmount = (int)mHelpText.size() * 14;
}

void HelpDisplay::DrawModule()
{
   ofPushStyle();
   ofSetColor(50, 50, 50, 200);
   ofFill();
   ofRect(0, 0, mWidth, mHeight);
   ofPopStyle();

   DrawTextRightJustify(GetBuildInfoString(), mWidth - 5, 12);

   mShowTooltipsCheckbox->Draw();
   mCopyBuildInfoButton->Draw();
   mDumpModuleInfoButton->SetShowing(GetKeyModifiers() == kModifier_Shift);
   mDumpModuleInfoButton->Draw();
   mDoModuleScreenshotsButton->SetShowing(GetKeyModifiers() == kModifier_Shift);
   mDoModuleScreenshotsButton->Draw();
   mDoModuleDocumentationButton->SetShowing(GetKeyModifiers() == kModifier_Shift);
   mDoModuleDocumentationButton->Draw();

   DrawTextNormal("video overview available at:", 4, 73);
   mTutorialVideoLinkButton->Draw();
   DrawTextNormal("documentation:", 4, 92);
   mDocsLinkButton->Draw();
   DrawTextNormal("join the ", 260, 92);
   mDiscordLinkButton->Draw();

   mHeight = 100;
   const int kLineHeight = 14;
   ofClipWindow(0, mHeight, mWidth, (int)mHelpText.size() * kLineHeight, true);
   for (size_t i = 0; i < mHelpText.size(); ++i)
   {
      DrawTextNormal(mHelpText[i], 4, mHeight - mScrollOffsetY);
      mHeight += 14;
   }
   ofResetClipWindow();

   if (mScreenshotCountdown <= 0)
   {
      if (mScreenshotState == ScreenshotState::WaitingForScreenshot)
      {
         std::string typeName = mScreenshotsToProcess.begin()->mLabel;
         mScreenshotsToProcess.pop_front();

         ofRectangle rect = mScreenshotModule->GetRect();
         rect.y -= IDrawableModule::TitleBarHeight();
         rect.height += IDrawableModule::TitleBarHeight();
         rect.grow(10);
         RenderScreenshot(rect.x, rect.y, rect.width, rect.height, typeName + ".png");

         mScreenshotCountdown = 10;
         if (!mScreenshotsToProcess.empty())
            mScreenshotState = ScreenshotState::WaitingForSpawn;
         else
            mScreenshotState = ScreenshotState::None;
      }
   }
   else
   {
      --mScreenshotCountdown;
   }
}

void HelpDisplay::Poll()
{
   if (mScreenshotState == ScreenshotState::WaitingForSpawn)
   {
      if (mScreenshotModule != nullptr)
         mScreenshotModule->GetOwningContainer()->DeleteModule(mScreenshotModule);
      mScreenshotModule = TheSynth->SpawnModuleOnTheFly(mScreenshotsToProcess.front(), 100, 300);

      if (mScreenshotsToProcess.front().mLabel == "drumplayer")
         mScreenshotModule->FindUIControl("edit")->SetValue(1, gTime);

      mScreenshotState = ScreenshotState::WaitingForScreenshot;
      mScreenshotCountdown = 10;
   }
}

bool HelpDisplay::MouseScrolled(float x, float y, float scrollX, float scrollY, bool isSmoothScroll, bool isInvertedScroll)
{
   mScrollOffsetY = ofClamp(mScrollOffsetY - scrollY * 10, 0, mMaxScrollAmount);
   return true;
}

void HelpDisplay::GetModuleDimensions(float& w, float& h)
{
   if (mScreenshotsToProcess.size() > 0)
   {
      w = 10;
      h = 10;
   }
   else
   {
      w = mWidth;
      h = mHeight;
   }
}

void HelpDisplay::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mShowTooltipsCheckbox)
   {
      if (sShowTooltips) // && !mTooltipsLoaded)
         LoadTooltips();
   }
}

void HelpDisplay::LoadTooltips()
{
   sTooltips.clear();

   ModuleTooltipInfo moduleInfo;
   UIControlTooltipInfo controlInfo;

   juce::File tooltipsFile(ofToResourcePath(UserPrefs.tooltips.Get()));
   if (tooltipsFile.existsAsFile())
   {
      juce::StringArray lines;
      tooltipsFile.readLines(lines);

      for (int i = 0; i < lines.size(); ++i)
      {
         if (lines[i].isNotEmpty())
         {
            juce::String line = lines[i].replace("\\n", "\n");
            std::vector<std::string> tokens = ofSplitString(line.toStdString(), "~");
            if (tokens.size() == 2)
            {
               if (!moduleInfo.module.empty())
                  sTooltips.push_back(moduleInfo); //add this one and start a new one
               moduleInfo.module = tokens[0];
               moduleInfo.tooltip = tokens[1];
               moduleInfo.controlTooltips.clear();
            }
            else if (tokens.size() == 3)
            {
               controlInfo.controlName = tokens[1];
               controlInfo.tooltip = tokens[2];
               moduleInfo.controlTooltips.push_back(controlInfo);
            }
         }
      }
   }

   sTooltips.push_back(moduleInfo); //get the last one

   sTooltipsLoaded = true;
}

HelpDisplay::ModuleTooltipInfo* HelpDisplay::FindModuleInfo(std::string moduleTypeName)
{
   for (auto& info : sTooltips)
   {
      if (info.module == moduleTypeName)
         return &info;
   }
   return nullptr;
}

namespace
{
   bool StringMatch(juce::String pattern, juce::String target)
   {
      /*if (pattern.endsWithChar('*'))
      {
         ppattern = pattern.removeCharacters("*");
         return target.startsWith(pattern);
      }
      else
      {
         return pattern == target;
      }*/

      int m = pattern.length();
      int n = target.length();

      // empty pattern can only match with
      // empty string
      if (m == 0)
         return (n == 0);

      // lookup table for storing results of
      // subproblems
      std::vector<std::vector<bool>> lookup(n + 1, std::vector<bool>(m + 1));

      // empty pattern can match with empty string
      lookup[0][0] = true;

      // Only '*' can match with empty string
      for (int j = 1; j <= m; j++)
         if (pattern[j - 1] == '*')
            lookup[0][j] = lookup[0][j - 1];

      // fill the table in bottom-up fashion
      for (int i = 1; i <= n; i++)
      {
         for (int j = 1; j <= m; j++)
         {
            // Two cases if we see a '*'
            // a) We ignore ‘*’ character and move
            //    to next  character in the pattern,
            //     i.e., ‘*’ indicates an empty sequence.
            // b) '*' character matches with ith
            //     character in input
            if (pattern[j - 1] == '*')
               lookup[i][j] = lookup[i][j - 1] || lookup[i - 1][j];

            else if (target[i - 1] == pattern[j - 1])
               lookup[i][j] = lookup[i - 1][j - 1];

            // If characters don't match
            else
               lookup[i][j] = false;
         }
      }

      return lookup[n][m];
   }
}

HelpDisplay::UIControlTooltipInfo* HelpDisplay::FindControlInfo(IUIControl* control)
{
   ModuleTooltipInfo* moduleInfo = nullptr;
   IDrawableModule* parent = dynamic_cast<IDrawableModule*>(control->GetParent());
   if (parent != nullptr)
      moduleInfo = FindModuleInfo(parent->GetTypeName());
   if (moduleInfo)
   {
      std::string controlName = control->Name();
      for (auto& info : moduleInfo->controlTooltips)
      {
         if (StringMatch(info.controlName, controlName))
            return &info;
      }
   }

   //didn't find it, try again with the "module parent"
   parent = control->GetModuleParent();
   if (parent != nullptr)
      moduleInfo = FindModuleInfo(parent->GetTypeName());
   if (moduleInfo)
   {
      std::string controlName = control->Name();
      for (auto& info : moduleInfo->controlTooltips)
      {
         if (StringMatch(info.controlName, controlName))
            return &info;
      }
   }

   return nullptr;
}

std::string HelpDisplay::GetUIControlTooltip(IUIControl* control)
{
   std::string name = control->Name();
   std::string tooltip;

   UIControlTooltipInfo* controlInfo = FindControlInfo(control);
   if (controlInfo && controlInfo->tooltip.size() > 0)
      tooltip = controlInfo->tooltip;
   else
      tooltip = "[no tooltip found]";

   return name + ": " + tooltip;
}

std::string HelpDisplay::GetModuleTooltip(IDrawableModule* module)
{
   if (module == TheTitleBar)
      return "";

   return GetModuleTooltipFromName(module->GetTypeName());
}

std::string HelpDisplay::GetModuleTooltipFromName(std::string moduleTypeName)
{
   std::string tooltip;

   ModuleTooltipInfo* moduleInfo = FindModuleInfo(moduleTypeName);
   if (moduleInfo && moduleInfo->tooltip.size() > 0)
      tooltip = moduleInfo->tooltip;
   else
      tooltip = "[no tooltip found]";

   return moduleTypeName + ": " + tooltip;
}

void HelpDisplay::ButtonClicked(ClickButton* button, double time)
{
   if (button == mTutorialVideoLinkButton)
   {
      juce::URL("https://youtu.be/SYBc8X2IxqM").launchInDefaultBrowser();
   }
   if (button == mDocsLinkButton)
   {
      juce::URL("http://bespokesynth.com/docs").launchInDefaultBrowser();
   }
   if (button == mDiscordLinkButton)
   {
      juce::URL("https://discord.gg/YdTMkvvpZZ").launchInDefaultBrowser();
   }
   if (button == mCopyBuildInfoButton)
   {
      juce::SystemClipboard::copyTextToClipboard("bespoke " + juce::String(GetBuildInfoString()) + " - " + juce::SystemStats::getOperatingSystemName() + " - " + Bespoke::GIT_BRANCH + " " + Bespoke::GIT_HASH);
   }
   if (button == mDumpModuleInfoButton)
   {
      LoadTooltips();

      std::vector<ModuleCategory> moduleTypes = {
         kModuleCategory_Note,
         kModuleCategory_Synth,
         kModuleCategory_Audio,
         kModuleCategory_Instrument,
         kModuleCategory_Processor,
         kModuleCategory_Modulator,
         kModuleCategory_Pulse,
         kModuleCategory_Other
      };
      for (auto type : moduleTypes)
      {
         const auto& modules = TheSynth->GetModuleFactory()->GetSpawnableModules(type);
         for (auto toSpawn : modules)
            TheSynth->SpawnModuleOnTheFly(toSpawn, 0, 0);
      }

      std::string output;
      std::vector<IDrawableModule*> modules;
      TheSynth->GetAllModules(modules);

      for (auto* topLevelModule : modules)
      {
         if (topLevelModule->GetTypeName() == "effectchain")
         {
            EffectChain* effectChain = dynamic_cast<EffectChain*>(topLevelModule);
            std::vector<std::string> effects = TheSynth->GetEffectFactory()->GetSpawnableEffects();
            for (std::string effect : effects)
               effectChain->AddEffect(effect, effect, !K(onTheFly));
         }

         std::vector<IDrawableModule*> toDump;
         toDump.push_back(topLevelModule);
         for (auto* child : topLevelModule->GetChildren())
            toDump.push_back(child);

         std::list<std::string> addedModuleNames;
         for (auto* module : toDump)
         {
            if (ListContains(module->GetTypeName(), addedModuleNames) || module->GetTypeName().length() == 0)
               continue;
            addedModuleNames.push_back(module->GetTypeName());

            std::string moduleTooltip = "[no tooltip]";
            ModuleTooltipInfo* moduleInfo = FindModuleInfo(module->GetTypeName());
            if (moduleInfo)
            {
               moduleTooltip = moduleInfo->tooltip;
               ofStringReplace(moduleTooltip, "\n", "\\n");
            }
            output += "\n\n\n" + module->GetTypeName() + "~" + moduleTooltip + "\n";
            std::vector<IUIControl*> controls = module->GetUIControls();
            std::list<std::string> addedControlNames;
            for (auto* control : controls)
            {
               if (control->GetParent() != module && VectorContains(dynamic_cast<IDrawableModule*>(control->GetParent()), toDump))
                  continue; //we'll print this control's info when we are printing for the specific parent module

               std::string controlName = control->Name();
               if (controlName != "enabled")
               {
                  std::string controlTooltip = "[no tooltip]";
                  UIControlTooltipInfo* controlInfo = FindControlInfo(control);
                  if (controlInfo)
                  {
                     controlName = controlInfo->controlName;
                     controlTooltip = controlInfo->tooltip;
                     ofStringReplace(controlTooltip, "\n", "\\n");
                  }
                  if (!ListContains(controlName, addedControlNames))
                  {
                     output += "~" + controlName + "~" + controlTooltip + "\n";
                     addedControlNames.push_back(controlName);
                  }
               }
            }
         }
      }

      juce::File moduleDump(ofToDataPath("module_dump.txt"));
      moduleDump.replaceWithText(output);
   }
   if (button == mDoModuleScreenshotsButton)
   {
      gDrawScale = 1.0f;

      std::vector<ModuleCategory> moduleTypes = {
         kModuleCategory_Note,
         kModuleCategory_Synth,
         kModuleCategory_Audio,
         kModuleCategory_Instrument,
         kModuleCategory_Processor,
         kModuleCategory_Modulator,
         kModuleCategory_Pulse,
         kModuleCategory_Other
      };
      for (auto type : moduleTypes)
      {
         const auto& modules = TheSynth->GetModuleFactory()->GetSpawnableModules(type);
         for (auto toSpawn : modules)
            mScreenshotsToProcess.push_back(toSpawn);
      }

      for (auto effect : TheSynth->GetEffectFactory()->GetSpawnableEffects())
      {
         ModuleFactory::Spawnable spawnable;
         spawnable.mLabel = effect;
         spawnable.mSpawnMethod = ModuleFactory::SpawnMethod::EffectChain;
         mScreenshotsToProcess.push_back(spawnable);
      }

      /*mScreenshotsToProcess.clear();
      mScreenshotsToProcess.push_back(TheSynth->GetModuleFactory()->GetSpawnableModules("sampleplayer", true)[0]);
      mScreenshotsToProcess.push_back(TheSynth->GetModuleFactory()->GetSpawnableModules("drumplayer", true)[0]);
      mScreenshotsToProcess.push_back(TheSynth->GetModuleFactory()->GetSpawnableModules("notesequencer", true)[0]);*/

      mScreenshotState = ScreenshotState::WaitingForSpawn;
   }
   if (button == mDoModuleDocumentationButton)
   {
      ofxJSONElement docs;

      LoadTooltips();
      for (auto moduleType : sTooltips)
      {
         docs[moduleType.module]["description"] = moduleType.tooltip;
         for (auto control : moduleType.controlTooltips)
         {
            docs[moduleType.module]["controls"][control.controlName] = control.tooltip;
         }

         std::string typeName = "unknown";
         if (moduleType.module == "scale" || moduleType.module == "transport" || moduleType.module == "vstplugin")
            typeName = "other";

         docs[moduleType.module]["type"] = typeName;
         docs[moduleType.module]["canReceiveAudio"] = false;
         docs[moduleType.module]["canReceiveNote"] = false;
         docs[moduleType.module]["canReceivePulses"] = false;
      }

      std::vector<ModuleCategory> moduleCategories = {
         kModuleCategory_Note,
         kModuleCategory_Synth,
         kModuleCategory_Audio,
         kModuleCategory_Instrument,
         kModuleCategory_Processor,
         kModuleCategory_Modulator,
         kModuleCategory_Pulse,
         kModuleCategory_Other
      };
      for (auto category : moduleCategories)
      {
         const auto& modules = TheSynth->GetModuleFactory()->GetSpawnableModules(category);
         for (auto toSpawn : modules)
         {
            IDrawableModule* module = TheSynth->SpawnModuleOnTheFly(toSpawn, 100, 300);

            std::string moduleType;
            switch (module->GetModuleCategory())
            {
               case kModuleCategory_Note: moduleType = "note effects"; break;
               case kModuleCategory_Synth: moduleType = "synths"; break;
               case kModuleCategory_Audio: moduleType = "audio effects"; break;
               case kModuleCategory_Instrument: moduleType = "instruments"; break;
               case kModuleCategory_Processor: moduleType = "effect chain"; break;
               case kModuleCategory_Modulator: moduleType = "modulators"; break;
               case kModuleCategory_Pulse: moduleType = "pulse"; break;
               case kModuleCategory_Other: moduleType = "other"; break;
               case kModuleCategory_Unknown: moduleType = "unknown"; break;
            }

            docs[toSpawn.mLabel]["type"] = moduleType;
            docs[toSpawn.mLabel]["canReceiveAudio"] = module->CanReceiveAudio();
            docs[toSpawn.mLabel]["canReceiveNote"] = module->CanReceiveNotes();
            docs[toSpawn.mLabel]["canReceivePulses"] = module->CanReceivePulses();

            module->GetOwningContainer()->DeleteModule(module);
         }
      }

      for (auto effect : TheSynth->GetEffectFactory()->GetSpawnableEffects())
         docs[effect]["type"] = "effect chain";

      docs.save(ofToDataPath("module_documentation.json"), true);
   }
}

void HelpDisplay::ScreenshotModule(IDrawableModule* module)
{
   mScreenshotModule = module;
}

void HelpDisplay::RenderScreenshot(int x, int y, int width, int height, std::string filename)
{
   float scale = gDrawScale * TheSynth->GetPixelRatio();
   x = (x + TheSynth->GetDrawOffset().x) * scale;
   y = (y + TheSynth->GetDrawOffset().y) * scale;
   width = width * scale;
   height = height * scale;

   unsigned char* pixels = new unsigned char[3 * (width) * (height)];

   juce::gl::glReadBuffer(juce::gl::GL_BACK);
   int oldAlignment;
   juce::gl::glGetIntegerv(juce::gl::GL_PACK_ALIGNMENT, &oldAlignment);
   juce::gl::glPixelStorei(juce::gl::GL_PACK_ALIGNMENT, 1); //tight packing
   juce::gl::glReadPixels(x, ofGetHeight() * scale - y - height, width, height, juce::gl::GL_RGB, juce::gl::GL_UNSIGNED_BYTE, pixels);
   juce::gl::glPixelStorei(juce::gl::GL_PACK_ALIGNMENT, oldAlignment);

   juce::Image image(juce::Image::RGB, width, height, true);
   for (int ix = 0; ix < width; ++ix)
   {
      for (int iy = 0; iy < height; ++iy)
      {
         int pos = (ix + (height - 1 - iy) * width) * 3;
         image.setPixelAt(ix, iy, juce::Colour(pixels[pos], pixels[pos + 1], pixels[pos + 2]));
      }
   }

   {
      juce::File(ofToDataPath("screenshots")).createDirectory();
      juce::File pngFile(ofToDataPath("screenshots/" + filename));
      if (pngFile.existsAsFile())
         pngFile.deleteFile();
      juce::FileOutputStream stream(pngFile);
      juce::PNGImageFormat pngWriter;
      pngWriter.writeImageToStream(image, stream);
   }

   /*{
      juce::File rawFile(ofToDataPath("screenshot.txt"));
      if (rawFile.existsAsFile())
         rawFile.deleteFile();
      FileOutputStream stream(rawFile);
      for (int i = 0; i < width*height * 3; ++i)
         stream << pixels[i] << ' ';
   }*/
}
