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
#include "FloatSliderLFOControl.h"
#include "ModuleSaveDataPanel.h"
#include "TitleBar.h"
#include "PerformanceTimer.h"
#include "SynthGlobals.h"
#include "QuickSpawnMenu.h"

ModuleContainer::ModuleContainer()
: mOwner(nullptr)
{
   
}

ofVec2f ModuleContainer::GetOwnerPosition() const
{
   if (mOwner)
      return mOwner->GetPosition();
   return ofVec2f();
}

void ModuleContainer::GetAllModules(vector<IDrawableModule*>& out)
{
   out.insert(out.begin(), mModules.begin(), mModules.end());
   for (int i=0; i<mModules.size(); ++i)
   {
      if (mModules[i]->GetContainer())
         mModules[i]->GetContainer()->GetAllModules(out);
   }
}

void ModuleContainer::Draw()
{
   for (int i = (int)mModules.size()-1; i >= 0; --i)
   {
      if (!mModules[i]->AlwaysOnTop())
         mModules[i]->Draw();
   }
   
   for (int i = (int)mModules.size()-1; i >= 0; --i)
   {
      if (mModules[i]->AlwaysOnTop())
         mModules[i]->Draw();
   }
}

void ModuleContainer::PostRender()
{
   for (int i = (int)mModules.size()-1; i >= 0; --i)
      mModules[i]->PostRender();
}

void ModuleContainer::DrawPatchCables()
{
   if (mOwner != nullptr && mOwner->Minimized())
      return;
   
   for (int i = (int)mModules.size()-1; i >= 0; --i)
   {
      mModules[i]->DrawPatchCables();
      if (mModules[i]->GetContainer())
         mModules[i]->GetContainer()->DrawPatchCables();
   }
}

void ModuleContainer::Poll()
{
   if (mOwner != nullptr) return;
   
   for (int i=0; i<mModules.size(); ++i)
   {
      if (mModules[i]->GetContainer())
         mModules[i]->GetContainer()->Poll();
      mModules[i]->BasePoll();
   }
}

void ModuleContainer::Clear()
{
   vector<IDrawableModule*> modulesToDelete = mModules;
   for (auto* module : modulesToDelete)
   {
      if (module->GetContainer())
         module->GetContainer()->Clear();
      if (module->IsSingleton() == false)
         DeleteModule(module);
   }
   mModules.clear();
}

void ModuleContainer::Exit()
{
   for (int i=0; i<mModules.size(); ++i)
   {
      if (mModules[i]->GetContainer())
         mModules[i]->GetContainer()->Exit();
      mModules[i]->Exit();
   }
}

void ModuleContainer::KeyPressed(int key, bool isRepeat)
{
   for (int i=0; i<mModules.size(); ++i)
   {
      if (mModules[i]->GetContainer())
         mModules[i]->GetContainer()->KeyPressed(key, isRepeat);
      mModules[i]->KeyPressed(key, isRepeat);
   }
}

void ModuleContainer::KeyReleased(int key)
{
   for (int i=0; i<mModules.size(); ++i)
   {
      if (mModules[i]->GetContainer())
         mModules[i]->GetContainer()->KeyReleased(key);
      mModules[i]->KeyReleased(key);
   }
}

void ModuleContainer::MouseMoved(float x, float y)
{
   if (mOwner != nullptr) return;
   
   for (int i=(int)mModules.size()-1; i>=0; --i)  //run this backwards so that we can figure out the top hover control
   {
      ModuleContainer* subcontainer = mModules[i]->GetContainer();
      if (subcontainer)
      {
         subcontainer->MouseMoved(x - subcontainer->GetOwnerPosition().x, y - subcontainer->GetOwnerPosition().y);
      }
      mModules[i]->NotifyMouseMoved(x,y);
   }
}

void ModuleContainer::MouseReleased()
{
   if (mOwner != nullptr) return;
   
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
   for (int i=0; i<mModules.size(); ++i)
   {
      if (mModules[i]->AlwaysOnTop() && mModules[i]->TestClick(x,y,false,true))
      {
         ModuleContainer* subcontainer = mModules[i]->GetContainer();
         if (subcontainer)
         {
            IDrawableModule* contained = subcontainer->GetModuleAt(x - subcontainer->GetOwnerPosition().x, y - subcontainer->GetOwnerPosition().y);
            if (contained)
               return contained;
         }
         return mModules[i];
      }
   }
   for (int i=0; i<mModules.size(); ++i)
   {
      if (!mModules[i]->AlwaysOnTop() && mModules[i]->TestClick(x,y,false,true))
      {
         ModuleContainer* subcontainer = mModules[i]->GetContainer();
         if (subcontainer)
         {
            IDrawableModule* contained = subcontainer->GetModuleAt(x - subcontainer->GetOwnerPosition().x, y - subcontainer->GetOwnerPosition().y);
            if (contained)
               return contained;
         }
         return mModules[i];
      }
   }
   return nullptr;
}

void ModuleContainer::GetModulesWithinRect(ofRectangle rect, vector<IDrawableModule*>& output)
{
   output.clear();
   for (int i=0; i<mModules.size(); ++i)
   {
      if (mModules[i]->IsWithinRect(rect) && mModules[i] != TheQuickSpawnMenu)
         output.push_back(mModules[i]);
   }
}

void ModuleContainer::MoveToFront(IDrawableModule* module)
{
   if (mOwner)
      mOwner->GetOwningContainer()->MoveToFront(mOwner);
   
   for (int i=0; i<mModules.size(); ++i)
   {
      if (mModules[i] == module)
      {
         for (int j=i; j>0; --j)
            mModules[j] = mModules[j-1];
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
   
   mModules.push_back(module);
   MoveToFront(module);
   
   ofVec2f offset = oldOwnerPos - GetOwnerPosition();
   module->SetPosition(module->GetPosition(true).x + offset.x,
                       module->GetPosition(true).y + offset.y);
   module->SetOwningContainer(this);
   if (mOwner)
      mOwner->AddChild(module);
}

void ModuleContainer::DeleteModule(IDrawableModule* module)
{
   if (module->IsSingleton())
      return;
   
   RemoveFromVector(module, mModules, K(fail));
   for (auto iter : mModules)
   {
      if (iter->GetPatchCableSource())
      {
         vector<PatchCable*> cablesToDestroy;
         for (auto cable : iter->GetPatchCableSource()->GetPatchCables())
         {
            if (cable->GetTarget() == module)
               cablesToDestroy.push_back(cable);
         }
         for (auto cable : cablesToDestroy)
            cable->Destroy();
      }
   }
   module->SetEnabled(false);
   module->Exit();
   TheSynth->OnModuleDeleted(module);
}

IDrawableModule* ModuleContainer::FindModule(string name, bool fail)
{
   /*string ownerPath = "";
   if (mOwner)
      ownerPath = mOwner->Path();
   if (strstr(name.c_str(), ownerPath.c_str()) != name.c_str())  //path doesn't start with ownerPath
      return nullptr;
      
   name = name.substr(ownerPath.length());*/
   
   if (name == "")
      return nullptr;
   
   for (int i=0; i<mModules.size(); ++i)
   {
      if (name == mModules[i]->Name())
         return mModules[i];
      vector<string> tokens = ofSplitString(name, "~");
      if (mModules[i]->GetContainer())
      {
         if (tokens[0] == mModules[i]->Name())
         {
            ofStringReplace(name, tokens[0]+"~", "", true);
            return mModules[i]->GetContainer()->FindModule(name, fail);
         }
      }
      if (tokens.size() == 2 && tokens[0] == mModules[i]->Name())
      {
         IDrawableModule* child = nullptr;
         try
         {
            child = mModules[i]->FindChild(tokens[1].c_str());
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

IUIControl* ModuleContainer::FindUIControl(string path)
{
   /*string ownerPath = "";
   if (mOwner)
      ownerPath = mOwner->Path();
   if (strstr(path.c_str(), ownerPath.c_str()) != path.c_str())  //path doesn't start with ownerPath
      return nullptr;
   
   path = path.substr(ownerPath.length());*/
   
   if (path == "")
      return nullptr;
   
   vector<string> tokens = ofSplitString(path,"~");
   string control = tokens[tokens.size()-1];
   string modulePath = path.substr(0, path.length() - (control.length() + 1));
   IDrawableModule* module = FindModule(modulePath, false);
   
   if (module)
   {
      try
      {
         return module->FindUIControl(control.c_str());
      }
      catch (UnknownUIControlException& e)
      {
         TheSynth->LogEvent("Couldn't find UI control at path \""+path+"\"", kLogEventType_Error);
         return nullptr;
      }
   }
   
   TheSynth->LogEvent("Couldn't find module at path \""+modulePath+"\"", kLogEventType_Error);
   return nullptr;
}

bool ModuleContainer::IsHigherThan(IDrawableModule* checkFor, IDrawableModule* checkAgainst) const
{
   while (checkFor->GetParent())
      checkFor = dynamic_cast<IDrawableModule*>(checkFor->GetParent());
   for (int i=0; i<mModules.size(); ++i)
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
         for (int i=0; i<modules.size(); ++i)
         {
            try
            {
               TimerInstance t("create "+modules[i]["name"].asString(), timer);
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
         for (int i=0; i<modules.size(); ++i)
         {
            try
            {
               TimerInstance t("setup "+modules[i]["name"].asString(), timer);
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
               TheSynth->LogEvent("Couldn't find module \""+e.mSearchName+"\"", kLogEventType_Error);
            }
         }
      }
      
      {
         TimerInstance t("init", timer);
         for (int i=0; i<mModules.size(); ++i)
         {
            TimerInstance t(string("init ")+mModules[i]->Name(), timer);
            if (mModules[i]->IsSingleton() == false)
               mModules[i]->Init();
         }
      }
      
      IClickable::ClearLoadContext();
   }
}

bool ModuleSorter(const IDrawableModule* a, const IDrawableModule* b)
{
   return string(a->Name()) < string(b->Name());
}

ofxJSONElement ModuleContainer::WriteModules()
{
   if (mOwner)
      IClickable::SetSaveContext(mOwner);
   
   for (auto i = mModules.begin(); i!= mModules.end(); ++i)
      UpdateTarget(*i);
   
   ofxJSONElement modules;
   
   vector<IDrawableModule*> saveModules;
   for (int i=0; i<mModules.size(); ++i)
   {
      IDrawableModule* module = mModules[i];
      if (module->IsSaveable())
         saveModules.push_back(module);
   }
   
   sort(saveModules.begin(), saveModules.end(), ModuleSorter);
   
   modules.resize((unsigned int)saveModules.size());
   
   for (int i=0; i<saveModules.size(); ++i)
   {
      ofxJSONElement moduleInfo;
      saveModules[i]->SaveLayout(moduleInfo);
      modules[i] = moduleInfo;
   }
   IClickable::ClearSaveContext();
   
   return modules;
}

namespace
{
   const int kSaveStateRev = 420;
}

void ModuleContainer::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;
   
   int savedModules = 0;
   for (auto* module : mModules)
   {
      if (module != TheSaveDataPanel && module != TheTitleBar)
         ++savedModules;
   }
   
   out << savedModules;
   
   for (auto* module : mModules)
   {
      if (module != TheSaveDataPanel && module != TheTitleBar)
      {
         //ofLog() << "Saving " << module->Name();
         out << string(module->Name());
         module->SaveState(out);
         for (int i=0; i<GetModuleSeparatorLength(); ++i)
            out << GetModuleSeparator()[i];
      }
   }
}

void ModuleContainer::LoadState(FileStreamIn& in)
{
   int header;
   in >> header;
   assert(header == kSaveStateRev);
   
   int savedModules;
   in >> savedModules;
   
   for (int i=0; i<savedModules; ++i)
   {
      string moduleName;
      in >> moduleName;
      //ofLog() << "Loading " << moduleName;
      IDrawableModule* module = FindModule(moduleName, false);
      assert(module);
      try
      {
         module->LoadState(in);
         
         for (int j=0; j<GetModuleSeparatorLength(); ++j)
         {
            char separatorChar;
            in >> separatorChar;
            if (separatorChar != GetModuleSeparator()[j])
            {
               ofLog() << "Error loading state for " << module->Name();
               //something went wrong, let's print some info to try to figure it out
               ofLog() << "Read char " + ofToString(separatorChar) + " but expected " + GetModuleSeparator()[j] + "!";
               ofLog() << "Save state file position is " + ofToString(in.GetFilePosition()) + ", EoF is " + (in.Eof() ? "true" : "false");
               string nextFewChars = "Next 10 characters are:";
               for (int c=0;c<10;++c)
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
         TheSynth->LogEvent("Error loading state for module \""+moduleName+"\"", kLogEventType_Error);
         
         //read through the rest of the module until we find the spacer, so we can continue loading the next module
         int separatorProgress = 0;
         while (!in.Eof())
         {
            char val;
            in >> val;
            if (val == GetModuleSeparator()[separatorProgress])
               ++separatorProgress;
            else
               separatorProgress = 0;
            if (separatorProgress == GetModuleSeparatorLength())
               break;   //we did it!
         }
      }
   }
   
   for (auto module : mModules)
      module->PostLoadState();
}

//static
bool ModuleContainer::DoesModuleHaveMoreSaveData(FileStreamIn& in)
{
   char test[GetModuleSeparatorLength()+1];
   in.Peek(test, GetModuleSeparatorLength());
   test[GetModuleSeparatorLength()] = 0;
   if (strcmp(GetModuleSeparator(), test) == 0)
      return false;
   return true;
}
