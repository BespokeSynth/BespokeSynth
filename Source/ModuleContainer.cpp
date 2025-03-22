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

    ModuleContainer.cpp
    Created: 16 Oct 2016 3:47:21pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "ModuleContainer.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "TitleBar.h"
#include "PerformanceTimer.h"
#include "SynthGlobals.h"
#include "QuickSpawnMenu.h"
#include "Prefab.h"

#include "juce_core/juce_core.h"

ModuleContainer::ModuleContainer()
{
}

ofVec2f ModuleContainer::GetOwnerPosition() const
{
   if (mOwner)
      return mOwner->GetPosition();
   return ofVec2f();
}

void ModuleContainer::GetAllModules(std::vector<IDrawableModule*>& out)
{
   out.insert(out.begin(), mModules.begin(), mModules.end());
   for (int i = 0; i < mModules.size(); ++i)
   {
      if (mModules[i]->GetContainer())
         mModules[i]->GetContainer()->GetAllModules(out);
   }
}

void ModuleContainer::DrawContents()
{
   DrawPatchCables(!K(parentMinimized), !K(inFront));
   DrawModules();
   DrawPatchCables(!K(parentMinimized), K(inFront));
   DrawUnclipped();
}

void ModuleContainer::DrawModules()
{
   for (int i = (int)mModules.size() - 1; i >= 0; --i)
   {
      if (!mModules[i]->AlwaysOnTop())
         mModules[i]->Draw();
   }

   for (int i = (int)mModules.size() - 1; i >= 0; --i)
   {
      if (mModules[i]->AlwaysOnTop())
         mModules[i]->Draw();
   }
}

void ModuleContainer::DrawUnclipped()
{
   for (int i = (int)mModules.size() - 1; i >= 0; --i)
   {
      if (!mModules[i]->AlwaysOnTop())
         mModules[i]->RenderUnclipped();
   }

   for (int i = (int)mModules.size() - 1; i >= 0; --i)
   {
      if (mModules[i]->AlwaysOnTop())
         mModules[i]->RenderUnclipped();
   }
}

void ModuleContainer::PostRender()
{
   for (int i = (int)mModules.size() - 1; i >= 0; --i)
      mModules[i]->PostRender();
}

void ModuleContainer::DrawPatchCables(bool parentMinimized, bool inFront)
{
   if (mOwner != nullptr && mOwner->Minimized())
      parentMinimized = true;

   for (int i = (int)mModules.size() - 1; i >= 0; --i)
   {
      mModules[i]->DrawPatchCables(parentMinimized, inFront);
      if (mModules[i]->GetContainer())
         mModules[i]->GetContainer()->DrawPatchCables(parentMinimized, inFront);
   }
}

void ModuleContainer::Poll()
{
   if (mOwner != nullptr)
      return;

   for (int i = 0; i < mModules.size(); ++i)
   {
      if (mModules[i]->GetContainer())
         mModules[i]->GetContainer()->Poll();
      mModules[i]->BasePoll();
   }
}

void ModuleContainer::Clear()
{
   std::vector<IDrawableModule*> modulesToDelete = mModules;
   for (auto* module : modulesToDelete)
   {
      if (module)
      {
         if (module->GetContainer())
            module->GetContainer()->Clear();
         if (module->IsSingleton() == false)
            DeleteModule(module);
      }
   }
   mModules.clear();
}

void ModuleContainer::Exit()
{
   for (int i = 0; i < mModules.size(); ++i)
   {
      if (mModules[i]->GetContainer())
         mModules[i]->GetContainer()->Exit();
      mModules[i]->Exit();
   }
}

void ModuleContainer::KeyPressed(int key, bool isRepeat)
{
   for (int i = 0; i < mModules.size(); ++i)
   {
      if (mModules[i]->GetContainer())
         mModules[i]->GetContainer()->KeyPressed(key, isRepeat);
      mModules[i]->KeyPressed(key, isRepeat);
   }
}

void ModuleContainer::KeyReleased(int key)
{
   for (int i = 0; i < mModules.size(); ++i)
   {
      if (mModules[i]->GetContainer())
         mModules[i]->GetContainer()->KeyReleased(key);
      mModules[i]->KeyReleased(key);
   }
}

void ModuleContainer::MouseMoved(float x, float y)
{
   if (mOwner != nullptr)
      return;

   for (int i = (int)mModules.size() - 1; i >= 0; --i) //run this backwards so that we can figure out the top hover control
   {
      ModuleContainer* subcontainer = mModules[i]->GetContainer();
      if (subcontainer)
      {
         subcontainer->MouseMoved(x - subcontainer->GetOwnerPosition().x, y - subcontainer->GetOwnerPosition().y);
      }
      mModules[i]->NotifyMouseMoved(x, y);
   }
}

void ModuleContainer::MouseReleased()
{
   if (mOwner != nullptr)
      return;

   for (int i = 0; i < mModules.size(); i++)
   {
      ModuleContainer* subcontainer = mModules[i]->GetContainer();
      if (subcontainer)
         subcontainer->MouseReleased();
      mModules[i]->MouseReleased();
   }
}

IDrawableModule* ModuleContainer::GetModuleAt(float x, float y)
{
   if (mOwner != nullptr)
   {
      float ownerX, ownerY;
      mOwner->GetPosition(ownerX, ownerY);
      x -= ownerX;
      y -= ownerY;
   }

   const auto& modalItems = TheSynth->GetModalFocusItemStack();
   for (int i = (int)modalItems.size() - 1; i >= 0; --i)
   {
      if (modalItems[i]->GetOwningContainer() == this && modalItems[i]->TestClick(x, y, false, true))
         return modalItems[i];
   }

   for (int i = 0; i < mModules.size(); ++i)
   {
      if (mModules[i]->AlwaysOnTop() && mModules[i]->TestClick(x, y, false, true))
      {
         ModuleContainer* subcontainer = mModules[i]->GetContainer();
         if (subcontainer)
         {
            IDrawableModule* contained = subcontainer->GetModuleAt(x - subcontainer->GetOwnerPosition().x, y - subcontainer->GetOwnerPosition().y);
            if (contained)
            {
               return contained;
            }
         }
         return mModules[i];
      }
   }
   for (int i = 0; i < mModules.size(); ++i)
   {
      if (!mModules[i]->AlwaysOnTop() && mModules[i]->TestClick(x, y, false, true))
      {
         ModuleContainer* subcontainer = mModules[i]->GetContainer();
         if (subcontainer)
         {
            IDrawableModule* contained = subcontainer->GetModuleAt(x, y);
            if (contained)
            {
               return contained;
            }
         }
         return mModules[i];
      }
   }
   return nullptr;
}

ofVec2f ModuleContainer::GetDrawOffset()
{
   if (mOwner != nullptr && mOwner->GetOwningContainer() != nullptr)
      return mDrawOffset + mOwner->GetOwningContainer()->GetDrawOffset();
   return mDrawOffset;
}

float ModuleContainer::GetDrawScale() const
{
   if (mOwner != nullptr && mOwner->GetOwningContainer() != nullptr)
      return mDrawScale * mOwner->GetOwningContainer()->GetDrawScale();
   return mDrawScale;
}

void ModuleContainer::GetModulesWithinRect(ofRectangle rect, std::vector<IDrawableModule*>& output, bool ignorePinned /* = false */)
{
   output.clear();
   for (int i = 0; i < mModules.size(); ++i)
   {
      if (mModules[i]->IsWithinRect(rect) && mModules[i] != TheQuickSpawnMenu && mModules[i]->IsShowing() && (ignorePinned && !mModules[i]->Pinned()))
         output.push_back(mModules[i]);
   }
}

void ModuleContainer::MoveToFront(IDrawableModule* module)
{
   if (mOwner && mOwner->GetOwningContainer() != nullptr)
      mOwner->GetOwningContainer()->MoveToFront(mOwner);

   for (int i = 0; i < mModules.size(); ++i)
   {
      if (mModules[i] == module)
      {
         for (int j = i; j > 0; --j)
            mModules[j] = mModules[j - 1];
         mModules[0] = module;

         break;
      }
   }
}

void ModuleContainer::AddModule(IDrawableModule* module)
{
   mModules.push_back(module);
   MoveToFront(module);
   TheSynth->OnModuleAdded(module);
   module->SetOwningContainer(this);
   if (mOwner)
      mOwner->AddChild(module);
}

void ModuleContainer::TakeModule(IDrawableModule* module)
{
   assert(module->GetOwningContainer()); //module must already be in a container
   ofVec2f oldOwnerPos = module->GetOwningContainer()->GetOwnerPosition();
   if (module->GetOwningContainer()->mOwner)
      module->GetOwningContainer()->mOwner->RemoveChild(module);
   RemoveFromVector(module, module->GetOwningContainer()->mModules);

   std::string newName = GetUniqueName(module->Name(), mModules);

   mModules.push_back(module);
   MoveToFront(module);

   ofVec2f offset = oldOwnerPos - GetOwnerPosition();
   module->SetPosition(module->GetPosition(true).x + offset.x,
                       module->GetPosition(true).y + offset.y);
   module->SetOwningContainer(this);
   if (mOwner)
      mOwner->AddChild(module);
   else //root modulecontainer
      module->SetName(newName.c_str());
}

void ModuleContainer::DeleteModule(IDrawableModule* module, bool fail /*= true*/)
{
   if (!module->CanBeDeleted())
      return;

   if (module->HasSpecialDelete())
   {
      module->DoSpecialDelete();
      RemoveFromVector(module, mModules, fail);
      return;
   }

   if (module->GetParent())
      module->GetParent()->GetModuleParent()->RemoveChild(module);

   RemoveFromVector(module, mModules, fail);
   for (const auto iter : mModules)
   {
      if (iter->GetPatchCableSource())
      {
         std::vector<PatchCable*> cablesToDestroy;
         for (auto cable : iter->GetPatchCableSource()->GetPatchCables())
         {
            if (cable->GetTarget() == module)
               cablesToDestroy.push_back(cable);
         }
         for (const auto cable : cablesToDestroy)
            cable->Destroy(false);
      }
   }

   // Remove all cables that targeted controls on this module
   IUIControl::DestroyCablesTargetingControls(module->GetUIControls());

   for (const auto child : module->GetChildren())
   {
      child->MarkAsDeleted();
      child->SetEnabled(false);
      child->Exit();
      TheSynth->OnModuleDeleted(child);
   }

   module->MarkAsDeleted();
   module->SetEnabled(false);
   module->Exit();
   TheSynth->OnModuleDeleted(module);
}

void ModuleContainer::DeleteCablesForControl(const IUIControl* control)
{
   // Remove all cables that targetted control on this module
   std::vector<IDrawableModule*> modules;
   TheSynth->GetAllModules(modules);
   std::vector<PatchCable*> cablesToDestroy;
   for (const auto module_iter : modules)
   {
      for (const auto source : module_iter->GetPatchCableSources())
      {
         for (const auto cable : source->GetPatchCables())
         {
            if (cable->GetTarget() == control)
            {
               cablesToDestroy.push_back(cable);
               break;
            }
         }
      }
   }
   for (const auto cable : cablesToDestroy)
      cable->Destroy(false);
}

IDrawableModule* ModuleContainer::FindModule(std::string name, bool fail)
{
   /*string ownerPath = "";
   if (mOwner)
      ownerPath = mOwner->Path();
   if (strstr(name.c_str(), ownerPath.c_str()) != name.c_str())  //path doesn't start with ownerPath
      return nullptr;
      
   name = name.substr(ownerPath.length());*/

   if (name == "")
      return nullptr;

   for (int i = 0; i < mModules.size(); ++i)
   {
      if (name == mModules[i]->Name())
         return mModules[i];
      std::vector<std::string> tokens = ofSplitString(name, "~");
      if (mModules[i]->GetContainer())
      {
         if (tokens[0] == mModules[i]->Name())
         {
            ofStringReplace(name, tokens[0] + "~", "", true);
            return mModules[i]->GetContainer()->FindModule(name, fail);
         }
      }
      if (tokens.size() == 2 && tokens[0] == mModules[i]->Name())
      {
         IDrawableModule* child = nullptr;
         try
         {
            child = mModules[i]->FindChild(tokens[1], fail);
         }
         catch (UnknownModuleException& e)
         {
         }
         if (child)
            return child;
      }
   }

   if (fail)
      throw UnknownModuleException(name);
   return nullptr;
}

IUIControl* ModuleContainer::FindUIControl(std::string path)
{
   /*string ownerPath = "";
   if (mOwner)
      ownerPath = mOwner->Path();
   if (strstr(path.c_str(), ownerPath.c_str()) != path.c_str())  //path doesn't start with ownerPath
      return nullptr;
   
   path = path.substr(ownerPath.length());*/

   if (path == "")
      return nullptr;

   std::vector<std::string> tokens = ofSplitString(path, "~");
   std::string control = tokens[tokens.size() - 1];
   std::string modulePath = path.substr(0, path.length() - (control.length() + 1));
   IDrawableModule* module = FindModule(modulePath, false);

   if (module)
   {
      try
      {
         module->OnUIControlRequested(control.c_str());
         return module->FindUIControl(control.c_str());
      }
      catch (UnknownUIControlException& e)
      {
         TheSynth->LogEvent("Couldn't find UI control at path \"" + path + "\"", kLogEventType_Error);
         return nullptr;
      }
   }

   TheSynth->LogEvent("Couldn't find module at path \"" + modulePath + "\"", kLogEventType_Error);
   return nullptr;
}

bool ModuleContainer::IsHigherThan(IDrawableModule* checkFor, IDrawableModule* checkAgainst) const
{
   while (checkFor->GetParent())
      checkFor = dynamic_cast<IDrawableModule*>(checkFor->GetParent());
   for (int i = 0; i < mModules.size(); ++i)
   {
      if (checkFor == mModules[i])
         return true;
      if (checkAgainst == mModules[i])
         return false;
   }
   return false;
}

void ModuleContainer::LoadModules(const ofxJSONElement& modules)
{
   PerformanceTimer timer;
   {
      TimerInstance t("load", timer);

      //two-pass loading for dependencies

      if (mOwner)
         IClickable::SetLoadContext(mOwner);
      {
         TimerInstance t("create", timer);
         for (int i = 0; i < modules.size(); ++i)
         {
            try
            {
               TimerInstance t("create " + modules[i]["name"].asString(), timer);
               IDrawableModule* module = TheSynth->CreateModule(modules[i]);
               if (module != nullptr)
               {
                  //ofLog() << "create " << module->Name();
                  AddModule(module);
               }
            }
            catch (LoadingJSONException& e)
            {
               TheSynth->LogEvent("Couldn't load json", kLogEventType_Error);
            }
         }
      }

      {
         TimerInstance t("setup", timer);
         for (int i = 0; i < modules.size(); ++i)
         {
            try
            {
               TimerInstance t("setup " + modules[i]["name"].asString(), timer);
               IDrawableModule* module = FindModule(modules[i]["name"].asString(), true);
               if (module != nullptr)
               {
                  //ofLog() << "setup " << module->Name();
                  TheSynth->SetUpModule(module, modules[i]);
               }
            }
            catch (LoadingJSONException& e)
            {
               TheSynth->LogEvent("Couldn't set up modules from json", kLogEventType_Error);
            }
            catch (UnknownModuleException& e)
            {
               TheSynth->LogEvent("Couldn't find module \"" + e.mSearchName + "\"", kLogEventType_Error);
            }
         }
      }

      {
         TimerInstance t("init", timer);
         for (int i = 0; i < mModules.size(); ++i)
         {
            TimerInstance t(std::string("init ") + mModules[i]->Name(), timer);
            if (mModules[i]->IsSingleton() == false)
               mModules[i]->Init();
         }
      }

      IClickable::ClearLoadContext();
   }
}

bool ModuleSorter(const IDrawableModule* a, const IDrawableModule* b)
{
   return std::string(a->Name()) < std::string(b->Name());
}

ofxJSONElement ModuleContainer::WriteModules()
{
   if (mOwner)
      IClickable::SetSaveContext(mOwner);

   for (auto i = mModules.begin(); i != mModules.end(); ++i)
      UpdateTarget(*i);

   ofxJSONElement modules;

   std::vector<IDrawableModule*> saveModules;
   for (int i = 0; i < mModules.size(); ++i)
   {
      IDrawableModule* module = mModules[i];
      if (module->IsSaveable())
         saveModules.push_back(module);
   }

   sort(saveModules.begin(), saveModules.end(), ModuleSorter);

   modules.resize((unsigned int)saveModules.size());

   for (int i = 0; i < saveModules.size(); ++i)
   {
      ofxJSONElement moduleInfo;
      saveModules[i]->SaveLayoutBase(moduleInfo);
      modules[i] = moduleInfo;
   }
   IClickable::ClearSaveContext();

   return modules;
}

void ModuleContainer::SaveState(FileStreamOut& out)
{
   out << ModularSynth::kSaveStateRev;

   int savedModules = 0;
   for (auto* module : mModules)
   {
      if (module->IsSaveable())
         ++savedModules;
   }

   out << savedModules;

   if (mOwner)
      IClickable::SetSaveContext(mOwner);

   for (auto* module : mModules)
   {
      if (module->IsSaveable())
      {
         //ofLog() << "Saving " << module->Name();
         out << std::string(module->Name());
         module->SaveState(out);
         for (int i = 0; i < GetModuleSeparatorLength(); ++i)
            out << GetModuleSeparator()[i];
      }
   }

   IClickable::ClearSaveContext();
}

void ModuleContainer::LoadState(FileStreamIn& in)
{
   Prefab::sLastLoadWasPrefab = Prefab::sLoadingPrefab;

   bool wasLoadingState = TheSynth->IsLoadingState();
   TheSynth->SetIsLoadingState(true);

   static int sModuleContainerLoadStack = 0;
   ++sModuleContainerLoadStack;

   int header;
   in >> header;
   assert(header <= ModularSynth::kSaveStateRev);
   ModularSynth::sLoadingFileSaveStateRev = header;
   ModularSynth::sLastLoadedFileSaveStateRev = header;

   int savedModules;
   in >> savedModules;

   if (mOwner)
      IClickable::SetLoadContext(mOwner);

   for (int i = 0; i < savedModules; ++i)
   {
      std::string moduleName;
      in >> moduleName;
      //ofLog() << "Loading " << moduleName;
      IDrawableModule* module = FindModule(moduleName, false);
      try
      {
         if (module == nullptr)
            throw LoadStateException();

         module->LoadState(in, module->LoadModuleSaveStateRev(in));

         for (int j = 0; j < GetModuleSeparatorLength(); ++j)
         {
            char separatorChar;
            in >> separatorChar;
            if (separatorChar != GetModuleSeparator()[j])
            {
               ofLog() << "Error loading state for " << module->Name();
               //something went wrong, let's print some info to try to figure it out
               ofLog() << "Read char " + ofToString(separatorChar) + " but expected " + GetModuleSeparator()[j] + "!";
               ofLog() << "Save state file position is " + ofToString(in.GetFilePosition()) + ", EoF is " + (in.Eof() ? "true" : "false");
               std::string nextFewChars = "Next 10 characters are:";
               for (int c = 0; c < 10; ++c)
               {
                  char ch;
                  in >> ch;
                  nextFewChars += ofToString(ch);
               }
               ofLog() << nextFewChars;
            }
            assert(separatorChar == GetModuleSeparator()[j]);
         }
      }
      catch (LoadStateException& e)
      {
         TheSynth->LogEvent("Error loading state for module \"" + moduleName + "\"", kLogEventType_Error);

         //read through the rest of the module until we find the spacer, so we can continue loading the next module
         int separatorProgress = 0;
         juce::uint64 safetyCheck = 0;
         while (!in.Eof() && safetyCheck < 1000000)
         {
            char val;
            in >> val;
            if (val == GetModuleSeparator()[separatorProgress])
               ++separatorProgress;
            else
               separatorProgress = 0;
            if (separatorProgress == GetModuleSeparatorLength())
               break; //we did it!
            ++safetyCheck;
         }
      }
   }

   for (auto module : mModules)
      module->PostLoadState();

   IClickable::ClearLoadContext();
   TheSynth->SetIsLoadingState(wasLoadingState);

   --sModuleContainerLoadStack;

   if (sModuleContainerLoadStack <= 0)
      ModularSynth::sLoadingFileSaveStateRev = ModularSynth::kSaveStateRev; //reset to current
}

//static
bool ModuleContainer::DoesModuleHaveMoreSaveData(FileStreamIn& in)
{
   char test[GetModuleSeparatorLength() + 1];
   in.Peek(test, GetModuleSeparatorLength());
   test[GetModuleSeparatorLength()] = 0;
   if (strcmp(GetModuleSeparator(), test) == 0)
      return false;
   return true;
}
