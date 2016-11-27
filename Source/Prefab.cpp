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
   mModuleContainer.SetOwner(this);
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

void Prefab::Poll()
{
   int xMin,yMin;
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
   
   SetPosition(xMin, yMin);
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
   
   mSaveButton->Draw();
   mLoadButton->Draw();
   DrawText("add/remove", 18, 14);
   
   mModuleContainer.Draw();
}

void Prefab::PostRepatch(PatchCableSource* cableSource)
{
   IDrawableModule* module = dynamic_cast<IDrawableModule*>(cableSource->GetTarget());
   if (module)
   {
      if (!VectorContains(module, mModuleContainer.GetModules()))
         mModuleContainer.TakeModule(module);
      else
         GetOwningContainer()->TakeModule(module);
   }
   cableSource->Clear();
}

void Prefab::GetModuleDimensions(int &width, int &height)
{
   float x,y;
   GetPosition(x, y);
   width = 162;
   height = 20;
   
   if (PatchCable::sActivePatchCable && PatchCable::sActivePatchCable->GetOwningModule() == this)
      return;
      
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
      FileChooser chooser("Load prefab...", File(ofToDataPath("prefabs")));
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
   
   root["modules"] = mModuleContainer.WriteModules();
   
   FileStreamOut out(ofToDataPath(savePath).c_str());
   
   out << root.getRawString(true);
   mModuleContainer.SaveState(out);
}

void Prefab::LoadPrefab(string loadPath)
{
   ScopedMutex mutex(TheSynth->GetAudioMutex(), "LoadPrefab()");
   ScopedLock renderLock(*TheSynth->GetRenderLock());
   
   mModuleContainer.Clear();
   
   FileStreamIn in(ofToDataPath(loadPath).c_str());
   
   string jsonString;
   in >> jsonString;
   ofxJSONElement root;
   bool loaded = root.parse(jsonString);
   
   if (!loaded)
   {
      TheSynth->LogEvent("Couldn't load, error parsing "+loadPath, kLogEventType_Error);
      TheSynth->LogEvent("Try loading it up in a json validator", kLogEventType_Error);
      return;
   }
   
   mModuleContainer.LoadModules(root["modules"]);
   
   mModuleContainer.LoadState(in);
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
