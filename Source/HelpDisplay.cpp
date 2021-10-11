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

#include "juce_gui_basics/juce_gui_basics.h"
#include "juce_opengl/juce_opengl.h"

bool HelpDisplay::sShowTooltips = false;
bool HelpDisplay::sTooltipsLoaded = false;
std::list<HelpDisplay::ModuleTooltipInfo> HelpDisplay::sTooltips;

HelpDisplay::HelpDisplay()
: mShowTooltipsCheckbox(nullptr)
, mWidth(700)
, mHeight(700)
, mScreenshotModule(nullptr)
{
   LoadHelp();

   sShowTooltips = TheSynth->GetUserPrefs()["show_tooltips_on_load"].isNull() ? true : (TheSynth->GetUserPrefs()["show_tooltips_on_load"].asInt() > 0);
   LoadTooltips();
}

void HelpDisplay::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mShowTooltipsCheckbox = new Checkbox(this, "show tooltips", 3, 22, &sShowTooltips);
   mDumpModuleInfoButton = new ClickButton(this, "dump module info", 110, 22);
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
}

void HelpDisplay::DrawModule()
{
   ofPushStyle();
   ofSetColor(50,50,50,200);
   ofFill();
   ofRect(0,0,mWidth,mHeight);
   ofPopStyle();

   DrawTextLeftJustify(juce::JUCEApplication::getInstance()->getApplicationVersion().toStdString() + " (" + std::string(__DATE__) + " " + std::string(__TIME__) + ")", mWidth-5, 12);
   
   mShowTooltipsCheckbox->Draw();
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
   for (size_t i = 0; i < mHelpText.size(); ++i)
   {
      DrawTextNormal(mHelpText[i], 4, mHeight);
      mHeight += 14;
   }

   static int sScreenshotDelay = 0;
   if (sScreenshotDelay <= 0)
   {
      if (mScreenshotModule != nullptr)
      {
         std::string typeName = mScreenshotModule->GetTypeName();
         if (!mScreenshotsToProcess.empty())
         {
            typeName = *mScreenshotsToProcess.begin();
            ofStringReplace(typeName, " " + std::string(ModuleFactory::kEffectChainSuffix), "");   //strip this suffix if it's there
            mScreenshotsToProcess.pop_front();
         }

         ofRectangle rect = mScreenshotModule->GetRect();
         rect.y -= IDrawableModule::TitleBarHeight();
         rect.height += IDrawableModule::TitleBarHeight();
         rect.grow(10);
         RenderScreenshot(rect.x, rect.y, rect.width, rect.height, typeName + ".png");

         mScreenshotModule->GetOwningContainer()->DeleteModule(mScreenshotModule);
         mScreenshotModule = nullptr;

         sScreenshotDelay = 10;
      }
      else if (!mScreenshotsToProcess.empty())
      {
         mScreenshotModule = TheSynth->SpawnModuleOnTheFly(mScreenshotsToProcess.front(), 100, 300);

         if (mScreenshotsToProcess.front() == "drumplayer")
            mScreenshotModule->FindUIControl("edit")->SetValue(1);

         sScreenshotDelay = 10;
      }
   }
   else
   {
      --sScreenshotDelay;
   }
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

void HelpDisplay::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mShowTooltipsCheckbox)
   {
      if (sShowTooltips)// && !mTooltipsLoaded)
         LoadTooltips();
   }
}

void HelpDisplay::LoadTooltips()
{
   std::string tooltipsPath;
   if (TheSynth->GetUserPrefs()["tooltips"].isNull() || !juce::File(ofToResourcePath(TheSynth->GetUserPrefs()["tooltips"].asString())).existsAsFile())
      tooltipsPath = ofToResourcePath("tooltips_eng.txt");
   else
      tooltipsPath = ofToResourcePath(TheSynth->GetUserPrefs()["tooltips"].asString());

   sTooltips.clear();

   ModuleTooltipInfo moduleInfo;
   UIControlTooltipInfo controlInfo;

   juce::File tooltipsFile(tooltipsPath);
   if (tooltipsFile.existsAsFile())
   {
      juce::StringArray lines;
      tooltipsFile.readLines(lines);

      for (int i=0; i<lines.size(); ++i)
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
      std::vector<std::vector<bool>> lookup(n+1, std::vector<bool>(m+1));

      // empty pattern can match with empty string
      lookup[0][0] = true;

      // Only '*' can match with empty string
      for (int j = 1; j <= m; j++)
         if (pattern[j - 1] == '*')
            lookup[0][j] = lookup[0][j - 1];

      // fill the table in bottom-up fashion
      for (int i = 1; i <= n; i++) {
         for (int j = 1; j <= m; j++) {
            // Two cases if we see a '*'
            // a) We ignore ‘*’ character and move
            //    to next  character in the pattern,
            //     i.e., ‘*’ indicates an empty sequence.
            // b) '*' character matches with ith
            //     character in input
            if (pattern[j - 1] == '*')
               lookup[i][j]
               = lookup[i][j - 1] || lookup[i - 1][j];

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

void HelpDisplay::ButtonClicked(ClickButton* button)
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
   if (button == mDumpModuleInfoButton)
   {
      LoadTooltips();

      std::vector<ModuleType> moduleTypes = {
                                          kModuleType_Note,
                                          kModuleType_Synth,
                                          kModuleType_Audio,
                                          kModuleType_Instrument,
                                          kModuleType_Processor,
                                          kModuleType_Modulator,
                                          kModuleType_Pulse,
                                          kModuleType_Other
                                       };
      for (auto type : moduleTypes)
      {
         const auto& spawnable = TheSynth->GetModuleFactory()->GetSpawnableModules(type);
         for (auto toSpawn : spawnable)
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
               effectChain->AddEffect(effect);
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
               ofStringReplace(moduleTooltip,"\n","\\n");
            }
            output += "\n\n\n" + module->GetTypeName() + "~"+moduleTooltip+"\n";
            std::vector<IUIControl*> controls = module->GetUIControls();
            std::list<std::string> addedControlNames;
            for (auto* control : controls)
            {
               if (control->GetParent() != module && VectorContains(dynamic_cast<IDrawableModule*>(control->GetParent()), toDump))
                  continue;   //we'll print this control's info when we are printing for the specific parent module
               
               std::string controlName = control->Name();
               if (controlName != "enabled")
               {
                  std::string controlTooltip = "[no tooltip]";
                  UIControlTooltipInfo* controlInfo = FindControlInfo(control);
                  if (controlInfo)
                  {
                     controlName = controlInfo->controlName;
                     controlTooltip = controlInfo->tooltip;
                     ofStringReplace(controlTooltip,"\n","\\n");
                  }
                  if (!ListContains(controlName, addedControlNames))
                  {
                     output += "~" + controlName + "~"+controlTooltip+"\n";
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
      /*mScreenshotsToProcess.push_back("sampleplayer");
      mScreenshotsToProcess.push_back("drumplayer");
      mScreenshotsToProcess.push_back("notesequencer");*/

      std::vector<ModuleType> moduleTypes = {
                                          kModuleType_Note,
                                          kModuleType_Synth,
                                          kModuleType_Audio,
                                          kModuleType_Instrument,
                                          kModuleType_Processor,
                                          kModuleType_Modulator,
                                          kModuleType_Pulse,
                                          kModuleType_Other
      };
      for (auto type : moduleTypes)
      {
         const auto& spawnable = TheSynth->GetModuleFactory()->GetSpawnableModules(type);
         for (auto toSpawn : spawnable)
            mScreenshotsToProcess.push_back(toSpawn);
      }

      for (auto effect : TheSynth->GetEffectFactory()->GetSpawnableEffects())
         mScreenshotsToProcess.push_back(effect + " " + ModuleFactory::kEffectChainSuffix);
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

      std::vector<ModuleType> moduleTypes = {
                                          kModuleType_Note,
                                          kModuleType_Synth,
                                          kModuleType_Audio,
                                          kModuleType_Instrument,
                                          kModuleType_Processor,
                                          kModuleType_Modulator,
                                          kModuleType_Pulse,
                                          kModuleType_Other
      };
      for (auto type : moduleTypes)
      {
         const auto& spawnable = TheSynth->GetModuleFactory()->GetSpawnableModules(type);
         for (auto toSpawn : spawnable)
         {
            IDrawableModule* module = TheSynth->SpawnModuleOnTheFly(toSpawn, 100, 300);

            std::string moduleType;
            switch (module->GetModuleType())
            {
               case kModuleType_Note: moduleType = "note effects"; break;
               case kModuleType_Synth: moduleType = "synths"; break;
               case kModuleType_Audio: moduleType = "audio effects"; break;
               case kModuleType_Instrument: moduleType = "instruments"; break;
               case kModuleType_Processor: moduleType = "effect chain"; break;
               case kModuleType_Modulator: moduleType = "modulators"; break;
               case kModuleType_Pulse: moduleType = "pulse"; break;
               case kModuleType_Other: moduleType = "other"; break;
               case kModuleType_Unknown: moduleType = "unknown"; break;
            }

            docs[toSpawn]["type"] = moduleType;
            docs[toSpawn]["canReceiveAudio"] = module->CanReceiveAudio();
            docs[toSpawn]["canReceiveNote"] = module->CanReceiveNotes();
            docs[toSpawn]["canReceivePulses"] = module->CanReceivePulses();

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
   juce::gl::glPixelStorei(juce::gl::GL_PACK_ALIGNMENT, 1);   //tight packing
   juce::gl::glReadPixels(x, ofGetHeight()*scale-y-height, width, height, juce::gl::GL_RGB, juce::gl::GL_UNSIGNED_BYTE, pixels);
   juce::gl::glPixelStorei(juce::gl::GL_PACK_ALIGNMENT, oldAlignment);

   juce::Image image(juce::Image::RGB, width, height, true);
   for (int x = 0; x < width; ++x)
   {
      for (int y = 0; y < height; ++y)
      {
         int pos = (x + (height - 1 - y) * width) * 3;
         image.setPixelAt(x, y, juce::Colour(pixels[pos], pixels[pos + 1], pixels[pos + 2]));
      }
   }

   {
      juce::File(ofToDataPath("screenshots")).createDirectory();
      juce::File pngFile(ofToDataPath("screenshots/"+filename));
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
