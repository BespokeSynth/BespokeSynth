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

Prefab::Prefab()
{
}

Prefab::~Prefab()
{
}

void Prefab::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mSaveButton = new ClickButton(this, "save", 95, 2);
   mLoadButton = new ClickButton(this, "load", -1, -1);
   mLoadButton->PositionTo(mSaveButton, kAnchorDirection_Right);
   
   mModuleCable = new PatchCableSource(this, kConnectionType_Special);
   mModuleCable->SetManualPosition(10, 10);
   AddPatchCableSource(mModuleCable);
}

namespace
{
   const float paddingX = 10;
   const float paddingY = 10;
}

void Prefab::PreDrawModule()
{
   if (mModules.empty())
      return;
   
   float x=FLT_MAX;
   float y=FLT_MAX;
   for (auto* module : mModules)
   {
      if (module->GetPosition().x - paddingX < x)
         x = module->GetPosition().x - paddingX;
      if (module->GetPosition().y - paddingY - 22 < y)
         y = module->GetPosition().y - paddingY - 22;
   }
   SetPosition(x, y);
}

void Prefab::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mSaveButton->Draw();
   mLoadButton->Draw();
   DrawText("add/remove", 18, 14);
}

void Prefab::PostRepatch(PatchCableSource* cableSource)
{
   IDrawableModule* module = dynamic_cast<IDrawableModule*>(cableSource->GetTarget());
   if (module)
   {
      if (ListContains(module, mModules))
         mModules.remove(module);
      else
         mModules.push_back(module);
   }
   cableSource->Clear();
}

void Prefab::Move(float moveX, float moveY)
{
   for (auto* module : mModules)
   {
      float moduleX,moduleY;
      module->GetPosition(moduleX, moduleY);
      module->SetPosition(moduleX+moveX, moduleY+moveY);
   }
   
   IDrawableModule::Move(moveX, moveY);
}

void Prefab::GetModuleDimensions(int &width, int &height)
{
   float x,y;
   GetPosition(x, y);
   width = 162;
   height = 20;
   for (auto* module : mModules)
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
   if (button == mSaveButton)
   {
      FileChooser chooser("Save prefab as...", File(ofToDataPath("prefabs/prefab.pfb")));
      if (chooser.browseForFileToSave(true))
      {
         string savePath = chooser.getResult().getRelativePathFrom(File(ofToDataPath(""))).toStdString();
         SavePrefab(savePath);
      }
   }
   
   if (button == mLoadButton)
   {
      FileChooser chooser("Load prefab...", File(ofToDataPath("prefabs/prefab.pfb")));
      if (chooser.browseForFileToOpen())
      {
         string loadPath = chooser.getResult().getRelativePathFrom(File(ofToDataPath(""))).toStdString();
         LoadPrefab(ofToDataPath(loadPath));
      }
   }
}

void Prefab::SavePrefab(string savePath)
{
   ofxJSONElement root;
   
   ofxJSONElement modules;
   
   vector<IDrawableModule*> saveModules;
   for (auto* module : mModules)
   {
      if (module->IsSaveable())
         saveModules.push_back(module);
   }
   
   modules.resize(saveModules.size());
   
   for (int i=0; i<saveModules.size(); ++i)
   {
      ofxJSONElement moduleInfo;
      saveModules[i]->SaveLayout(moduleInfo);
      
      moduleInfo["position"][0u] = moduleInfo["position"][0u].asInt() - GetPosition().x;
      moduleInfo["position"][1u] = moduleInfo["position"][1u].asInt() - GetPosition().y;
      
      modules[i] = moduleInfo;
   }
   
   root["modules"] = modules;
   root.save(savePath, true);
}

void Prefab::LoadPrefab(string loadPath)
{
   ScopedMutex mutex(TheSynth->GetAudioMutex(), "LoadPrefab()");
   ScopedLock renderLock(*TheSynth->GetRenderLock());
   
   for (auto* module : mModules)
      TheSynth->DeleteModule(module);
   mModules.clear();
   
   ofxJSONElement root;
   bool loaded = root.open(loadPath);
   
   if (!loaded)
   {
      TheSynth->LogEvent("Couldn't load, error parsing "+loadPath, kLogEventType_Error);
      TheSynth->LogEvent("Try loading it up in a json validator", kLogEventType_Error);
      return;
   }
   
   //two-pass loading for dependencies
   ofxJSONElement readModules = root["modules"];

   for (int i=0; i<readModules.size(); ++i)
   {
      try
      {
         readModules[i]["position"][0u] = readModules[i]["position"][0u].asInt() + GetPosition().x;
         readModules[i]["position"][1u] = readModules[i]["position"][1u].asInt() + GetPosition().y;
         
         IDrawableModule* module = TheSynth->CreateModule(readModules[i]);
         mModules.push_back(module);
      }
      catch (LoadingJSONException& e)
      {
         TheSynth->LogEvent("Couldn't load "+loadPath, kLogEventType_Error);
      }
   }

   for (int i=0; i<readModules.size(); ++i)
   {
      try
      {
         TheSynth->SetUpModule(readModules[i]);
      }
      catch (LoadingJSONException& e)
      {
         TheSynth->LogEvent("Couldn't set up "+loadPath, kLogEventType_Error);
      }
      catch (UnknownModuleException& e)
      {
         TheSynth->LogEvent("Couldn't find module \""+e.mSearchName+"\"", kLogEventType_Error);
      }
   }

   for (auto module : mModules)
      module->Init();
}

void Prefab::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
}

void Prefab::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void Prefab::SetUpFromSaveData()
{
}