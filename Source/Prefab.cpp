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
/*
  ==============================================================================

    Prefab.cpp
    Created: 25 Sep 2016 10:14:16am
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "Prefab.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"

#include "juce_gui_basics/juce_gui_basics.h"

//static
bool Prefab::sLoadingPrefab = false;
//static
IDrawableModule* Prefab::sJustReleasedModule = nullptr;

Prefab::Prefab()
{
   mModuleContainer.SetOwner(this);
   mPrefabName = "";
}

Prefab::~Prefab()
{
}

void Prefab::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mSaveButton = new ClickButton(this, "save", 95, 2);
   mLoadButton = new ClickButton(this, "load", mSaveButton, kAnchor_Right);
   mDisbandButton = new ClickButton(this, "disband", mLoadButton, kAnchor_Right);
   
   mRemoveModuleCable = new PatchCableSource(this, kConnectionType_Special);
   mRemoveModuleCable->SetManualPosition(10, 10);
   AddPatchCableSource(mRemoveModuleCable);
}

std::string Prefab::GetTitleLabel() const
{
   if (mPrefabName != "")
      return "prefab: "+mPrefabName;
   return "prefab";
}

void Prefab::Poll()
{
   float xMin,yMin;
   GetPosition(xMin, yMin);
   for (auto* module : mModuleContainer.GetModules())
   {
      xMin = MIN(xMin, module->GetPosition().x);
      yMin = MIN(yMin, module->GetPosition().y - 30);
   }
   
   int xOffset = GetPosition().x - xMin;
   int yOffset = GetPosition().y - yMin;
   for (auto* module : mModuleContainer.GetModules())
      module->SetPosition(module->GetPosition(true).x + xOffset, module->GetPosition(true).y + yOffset);
   
   if (abs(GetPosition().x - xMin) >= 1 || abs(GetPosition().y - yMin) >= 1)
      SetPosition(xMin, yMin);
}

bool Prefab::IsMouseHovered()
{
   return GetRect(false).contains(TheSynth->GetMouseX(GetOwningContainer()), TheSynth->GetMouseY(GetOwningContainer()));
}

bool Prefab::CanAddDropModules()
{
   if (IsMouseHovered())
   {
      if (TheSynth->GetMoveModule() != nullptr && !VectorContains(TheSynth->GetMoveModule(), mModuleContainer.GetModules()))
         return true;
      if (sJustReleasedModule != nullptr && !VectorContains(sJustReleasedModule, mModuleContainer.GetModules()))
         return true;
      if (!TheSynth->GetGroupSelectedModules().empty())
      {
         for (auto* module : TheSynth->GetGroupSelectedModules())
         {
            if (module == this)
               return false;
            if (!VectorContains(module, mModuleContainer.GetModules()))
               return true;
         }
      }
   }
   return false;
}

void Prefab::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);
   
   if (y > 0 && !right)
      TheSynth->SetGroupSelectContext(&mModuleContainer);
}

void Prefab::MouseReleased()
{
   IDrawableModule::MouseReleased();

   if (CanAddDropModules())
   {
      if (sJustReleasedModule != nullptr && !VectorContains(sJustReleasedModule, mModuleContainer.GetModules()))
         mModuleContainer.TakeModule(sJustReleasedModule);

      for(auto* module : TheSynth->GetGroupSelectedModules())
      {
         if (!VectorContains(module, mModuleContainer.GetModules()))
            mModuleContainer.TakeModule(module);
      }
   }
}

namespace
{
   const float paddingX = 10;
   const float paddingY = 10;
}

void Prefab::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   if (CanAddDropModules() && TheSynth->IsMouseButtonHeld(1))
   {
      ofPushStyle();
      ofSetColor(255, 255, 255, 80);
      ofFill();
      ofRect(0,0,GetRect().width,GetRect().height);
      ofPopStyle();
   }

   mSaveButton->Draw();
   mLoadButton->Draw();
   mDisbandButton->Draw();
   DrawTextNormal("remove", 18, 14);
   
   mModuleContainer.Draw();
}

void Prefab::DrawModuleUnclipped()
{
   mModuleContainer.DrawUnclipped();
}

void Prefab::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   if (cableSource == mRemoveModuleCable)
   {
      IDrawableModule* module = dynamic_cast<IDrawableModule*>(cableSource->GetTarget());
      if (module)
      {
         if (VectorContains(module, mModuleContainer.GetModules()))
         {
            GetOwningContainer()->TakeModule(module);
            GetOwningContainer()->MoveToFront(this);
         }
      }
      cableSource->Clear();
   }
}

void Prefab::GetModuleDimensions(float& width, float& height)
{
   float x,y;
   GetPosition(x, y);
   width = 215;
   height = 40;
   
   //if (PatchCable::sActivePatchCable && PatchCable::sActivePatchCable->GetOwningModule() == this)
   //   return;
      
   for (auto* module : mModuleContainer.GetModules())
   {
      ofRectangle rect = module->GetRect();
      if (rect.x - x + rect.width + paddingX > width)
         width = rect.x - x + rect.width + paddingX;
      if (rect.y - y + rect.height + paddingY > height)
         height = rect.y - y + rect.height + paddingY;
   }
}

void Prefab::ButtonClicked(ClickButton* button)
{
   using namespace juce;
   if (button == mSaveButton)
   {
      FileChooser chooser("Save prefab as...", File(ofToDataPath("prefabs/prefab.pfb")), "*.pfb", true, false, TheSynth->GetMainComponent()->getTopLevelComponent());
      if (chooser.browseForFileToSave(true))
      {
         std::string savePath = chooser.getResult().getRelativePathFrom(File(ofToDataPath(""))).toStdString();
         SavePrefab(savePath);
      }
   }
   
   if (button == mLoadButton)
   {
      FileChooser chooser("Load prefab...", File(ofToDataPath("prefabs")), "*.pfb", true, false, TheSynth->GetMainComponent()->getTopLevelComponent());
      if (chooser.browseForFileToOpen())
      {
         std::string loadPath = chooser.getResult().getRelativePathFrom(File(ofToDataPath(""))).toStdString();
         LoadPrefab(ofToDataPath(loadPath));
      }
   }
   
   if (button == mDisbandButton)
   {
      auto modules = mModuleContainer.GetModules();
      for (auto* module : modules)
         GetOwningContainer()->TakeModule(module);
      GetOwningContainer()->DeleteModule(this);
   }
}

void Prefab::SavePrefab(std::string savePath)
{
   ofxJSONElement root;
   
   root["modules"] = mModuleContainer.WriteModules();
   
   std::stringstream ss(root.getRawString(true));
   std::string line;
   std::string lines;
   while (getline(ss,line,'\n'))
   {
      const char* pos = strstr(line.c_str(), " : \"$");
      if (pos != nullptr)
      {
         bool endsWithComma = line[line.length()-1] == ',';
         ofStringReplace(line, pos, " : \"\"");
         if (endsWithComma)
            line += ",";
      }
      lines += line + '\n';
   }
   
   UpdatePrefabName(savePath);
   
   FileStreamOut out(ofToDataPath(savePath));
   
   out << lines;
   mModuleContainer.SaveState(out);
}

void Prefab::LoadPrefab(std::string loadPath)
{
   sLoadingPrefab = true;

   ScopedMutex mutex(TheSynth->GetAudioMutex(), "LoadPrefab()");
   std::lock_guard<std::recursive_mutex> renderLock(TheSynth->GetRenderLock());
   
   mModuleContainer.Clear();
   
   FileStreamIn in(ofToDataPath(loadPath));

   assert(in.OpenedOk());
   
   std::string jsonString;
   in >> jsonString;
   ofxJSONElement root;
   bool loaded = root.parse(jsonString);
   
   if (!loaded)
   {
      TheSynth->LogEvent("Couldn't load, error parsing "+loadPath, kLogEventType_Error);
      TheSynth->LogEvent("Try loading it up in a json validator", kLogEventType_Error);
      return;
   }
   
   UpdatePrefabName(loadPath);
   
   mModuleContainer.LoadModules(root["modules"]);
   
   mModuleContainer.LoadState(in);

   sLoadingPrefab = false;
}

void Prefab::UpdatePrefabName(std::string path)
{
   std::vector<std::string> tokens = ofSplitString(path, GetPathSeparator());
   mPrefabName = tokens[tokens.size() - 1];
   ofStringReplace(mPrefabName, ".pfb", "");
}

void Prefab::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   moduleInfo["modules"] = mModuleContainer.WriteModules();
}

void Prefab::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleContainer.LoadModules(moduleInfo["modules"]);
   SetUpFromSaveData();
}

void Prefab::SetUpFromSaveData()
{
}

namespace
{
   const int kSaveStateRev = 0;
}

void Prefab::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);

   out << kSaveStateRev;
   out << mPrefabName;
}

void Prefab::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);

   if (!ModuleContainer::DoesModuleHaveMoreSaveData(in))
      return;  //this was saved before we added versioning, bail out

   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);

   in >> mPrefabName;
}
