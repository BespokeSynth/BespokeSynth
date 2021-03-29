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

bool HelpDisplay::sShowTooltips = false;

HelpDisplay::HelpDisplay()
: mShowTooltipsCheckbox(nullptr)
, mWidth(700)
, mHeight(700)
, mTooltipsLoaded(false)
{
   LoadHelp();
}

void HelpDisplay::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mShowTooltipsCheckbox = new Checkbox(this, "show tooltips", 3, 22, &sShowTooltips);
   mDumpModuleInfo = new ClickButton(this, "dump module info", 110, 22);
   mTutorialVideoLinkButton = new ClickButton(this, "youtu.be/SYBc8X2IxqM", 160, 63);

   //mDumpModuleInfo->SetShowing(false);
}

HelpDisplay::~HelpDisplay()
{
}

void HelpDisplay::LoadHelp()
{
   File file(ofToDataPath("help.txt").c_str());
   if (file.existsAsFile())
   {
      string help = file.loadFileAsString().toStdString();
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
   
   mShowTooltipsCheckbox->Draw();
   mDumpModuleInfo->Draw();
   
   DrawTextNormal("video overview available at:", 4, 75);
   mTutorialVideoLinkButton->Draw();
   
   mHeight = 100;
   for (size_t i = 0; i < mHelpText.size(); ++i)
   {
      DrawTextNormal(mHelpText[i], 4, mHeight);
      mHeight += 14;
   }
}

void HelpDisplay::GetModuleDimensions(float& w, float& h)
{
   w = mWidth;
   h = mHeight;
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
   string tooltipsPath;
   if (TheSynth->GetUserPrefs()["tooltips"].isNull())
      tooltipsPath = ofToDataPath("internal/tooltips_eng.txt");
   else
      tooltipsPath = ofToDataPath(TheSynth->GetUserPrefs()["tooltips"].asString());

   mTooltips.clear();

   ModuleTooltipInfo moduleInfo;
   UIControlTooltipInfo controlInfo;

   File tooltipsFile(tooltipsPath);
   if (tooltipsFile.existsAsFile())
   {
      juce::StringArray lines;
      tooltipsFile.readLines(lines);

      for (int i=0; i<lines.size(); ++i)
      {
         if (lines[i].isNotEmpty())
         {
            juce::String line = lines[i].replace("\\n", "\n");
            vector<string> tokens = ofSplitString(line.toStdString(), "~");
            if (tokens.size() == 2)
            {
               if (!moduleInfo.module.empty())
                  mTooltips.push_back(moduleInfo); //add this one and start a new one
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

   mTooltips.push_back(moduleInfo); //get the last one

   mTooltipsLoaded = true;
}

HelpDisplay::ModuleTooltipInfo* HelpDisplay::FindModuleInfo(string moduleTypeName)
{
   for (auto& info : mTooltips)
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
         pattern = pattern.removeCharacters("*");
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
      std::vector<std::vector<bool>> lookup(n+1, vector<bool>(m+1));

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

            // Current characters are considered as
            // matching in two cases
            // (a) current character of pattern is '?'
            // (b) characters actually match
            else if (pattern[j - 1] == '?'
               || target[i - 1] == pattern[j - 1])
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
      string controlName = control->Name();
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
      string controlName = control->Name();
      for (auto& info : moduleInfo->controlTooltips)
      {
         if (StringMatch(info.controlName, controlName))
            return &info;
      }
   }

   return nullptr;
}

string HelpDisplay::GetUIControlTooltip(IUIControl* control)
{
   string name = control->Name();
   string tooltip;

   UIControlTooltipInfo* controlInfo = FindControlInfo(control);
   if (controlInfo && controlInfo->tooltip.size() > 0)
      tooltip = controlInfo->tooltip;
   else
      tooltip = "[no tooltip found]";

   return name + ": " + tooltip;
}

string HelpDisplay::GetModuleTooltip(IDrawableModule* module)
{
   if (module == TheTitleBar)
      return "";
   
   return GetModuleTooltipFromName(module->GetTypeName());
}

string HelpDisplay::GetModuleTooltipFromName(string moduleTypeName)
{
   string tooltip;

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
      URL("https://youtu.be/SYBc8X2IxqM").launchInDefaultBrowser();
   }
   if (button == mDumpModuleInfo)
   {
      LoadTooltips();

      vector<ModuleType> moduleTypes = {
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
         vector<string> spawnable = TheSynth->GetModuleFactory()->GetSpawnableModules(type);
         for (auto toSpawn : spawnable)
            TheSynth->SpawnModuleOnTheFly(toSpawn, 0, 0);
      }

      string output;
      vector<IDrawableModule*> modules;
      TheSynth->GetAllModules(modules);

      for (auto* topLevelModule : modules)
      {
         vector<IDrawableModule*> toDump;
         toDump.push_back(topLevelModule);
         for (auto* child : topLevelModule->GetChildren())
            toDump.push_back(child);

         for (auto* module : toDump)
         {
            string moduleTooltip = "[no tooltip]";
            ModuleTooltipInfo* moduleInfo = FindModuleInfo(module->GetTypeName());
            if (moduleInfo)
               moduleTooltip = moduleInfo->tooltip;
            output += "\n\n\n" + module->GetTypeName() + "~"+moduleTooltip+"\n";
            vector<IUIControl*> controls = module->GetUIControls();
            for (auto* control : controls)
            {
               string name = control->Name();
               if (name != "enabled")
               {
                  string controlTooltip = "[no tooltip]";
                  UIControlTooltipInfo* controlInfo = FindControlInfo(control);
                  if (controlInfo)
                     controlTooltip = controlInfo->tooltip;
                  output += "~" + name + "~"+controlTooltip+"\n";
               }
            }
         }
      }

      File moduleDump(ofToDataPath("module_dump.txt"));
      moduleDump.replaceWithText(output);
   }
}
