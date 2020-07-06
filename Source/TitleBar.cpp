//
//  TitleBar.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 11/30/14.
//
//

#include "TitleBar.h"
#include "ModularSynth.h"
#include "SynthGlobals.h"
#include "Profiler.h"
#include "ModuleFactory.h"
#include "ModuleSaveDataPanel.h"
#include "HelpDisplay.h"
#include "VSTPlugin.h"
#include "Prefab.h"

TitleBar* TheTitleBar = nullptr;

namespace
{
   const string kRescanPluginsLabel = "rescan VSTs...";
}

SpawnList::SpawnList(IDropdownListener* owner, SpawnListManager* listManager, int x, int y, string label)
: mLabel(label)
, mSpawnIndex(-1)
, mSpawnList(nullptr)
, mOwner(owner)
, mListManager(listManager)
, mPos(x,y)
{
}

void SpawnList::SetList(vector<string> spawnables, string overrideModuleType)
{
   mOverrideModuleType = overrideModuleType;
   if (mSpawnList == nullptr)
      mSpawnList = new DropdownList(mOwner,mLabel.c_str(),mPos.x,mPos.y,&mSpawnIndex);
   mSpawnList->SetNoHover(true);
   
   mSpawnList->Clear();
   mSpawnList->SetUnknownItemString(mLabel);
   mSpawnables = spawnables;
   for (int i=0; i<mSpawnables.size(); ++i)
   {
      string name = mSpawnables[i].c_str();
      if (mOverrideModuleType == "" && TheSynth->GetModuleFactory()->IsExperimental(name))
         name += " (exp.)";
      if (mOverrideModuleType == "vstplugin" && name != kRescanPluginsLabel)
         name = juce::File(name).getFileName().toStdString();
      mSpawnList->AddLabel(name,i);
   }
}

namespace
{
   ofVec2f moduleGrabOffset(-40,10);
}

void SpawnList::OnSelection(DropdownList* list)
{
   if (list == mSpawnList)
   {
      IDrawableModule* module = Spawn();
      if (module != nullptr)
         TheSynth->SetMoveModule(module, moduleGrabOffset.x, moduleGrabOffset.y);
      mSpawnIndex = -1;
   }
}

IDrawableModule* SpawnList::Spawn()
{
   string moduleType = mSpawnables[mSpawnIndex];
   if (mOverrideModuleType != "")
      moduleType = mOverrideModuleType;

   if (mOverrideModuleType == "vstplugin")
   {
      if (mSpawnables[mSpawnIndex] == kRescanPluginsLabel)
      {
         mListManager->SetUpVstDropdown(true);
         return nullptr;
      }
   }
   
   IDrawableModule* module = TheSynth->SpawnModuleOnTheFly(moduleType, TheSynth->GetMouseX() + moduleGrabOffset.x, TheSynth->GetMouseY() + moduleGrabOffset.y);
   
   if (mOverrideModuleType == "vstplugin")
   {
      VSTPlugin* plugin = dynamic_cast<VSTPlugin*>(module);
      plugin->SetVST(mSpawnables[mSpawnIndex]);
   }
   
   if (mOverrideModuleType == "prefab")
   {
      Prefab* prefab = dynamic_cast<Prefab*>(module);
      prefab->LoadPrefab("prefabs/"+mSpawnables[mSpawnIndex]);
   }
   
   return module;
}

void SpawnList::Draw()
{
   float x,y;
   mSpawnList->GetPosition(x, y, true);
   //DrawTextNormal(mLabel,x,y-2);
   mSpawnList->Draw();
}

void SpawnList::SetPosition(int x, int y)
{
   mSpawnList->SetPosition(x, y);
}

void SpawnList::SetPositionRelativeTo(SpawnList* list)
{
   float x,y,w,h;
   list->mSpawnList->GetPosition(x, y, true);
   list->mSpawnList->GetDimensions(w, h);
   mSpawnList->SetPosition(x + w + 5, y);
}

TitleBar::TitleBar()
: mSaveLayoutButton(nullptr)
, mResetLayoutButton(nullptr)
, mSaveStateButton(nullptr)
, mLoadStateButton(nullptr)
, mWriteAudioButton(nullptr)
, mQuitButton(nullptr)
, mLoadLayoutDropdown(nullptr)
, mLoadLayoutIndex(-1)
, mSpawnLists(this)
{
   assert(TheTitleBar == nullptr);
   TheTitleBar = this;
   
   mHelpDisplay = dynamic_cast<HelpDisplay*>(HelpDisplay::Create());
   
   SetShouldDrawOutline(false);
}

void TitleBar::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mSaveLayoutButton = new ClickButton(this,"save layout",280,19);
   mLoadStateButton = new ClickButton(this,"load state",140,1);
   mSaveStateButton = new ClickButton(this,"save state",205,1);
   mResetLayoutButton = new ClickButton(this,"reset layout",140,19);
   mWriteAudioButton = new ClickButton(this,"write audio",280,1);
   mQuitButton = new ClickButton(this,"quit",400,1);
   mDisplayHelpButton = new ClickButton(this," ? ",380,1);
   mLoadLayoutDropdown = new DropdownList(this, "load layout", 140, 20, &mLoadLayoutIndex);
   mEventLookaheadCheckbox = new Checkbox(this, "do event lookahead (exp.)", mResetLayoutButton, kAnchor_Right, &Transport::sDoEventLookahead);
   
   mLoadLayoutDropdown->SetShowing(false);
   mSaveLayoutButton->SetShowing(false);
   
   mHelpDisplay->CreateUIControls();
   
   ListLayouts();
}

TitleBar::~TitleBar()
{
   assert(TheTitleBar == this);
   TheTitleBar = nullptr;
}

SpawnListManager::SpawnListManager(IDropdownListener* owner)
: mInstrumentModules(owner,this,500,16,"instruments:")
, mNoteModules(owner,this,0,0,"note effects:")
, mSynthModules(owner,this,0,0,"synths:")
, mAudioModules(owner,this,0,0,"audio effects:")
, mModulatorModules(owner,this,0,0,"modulators:")
, mOtherModules(owner,this,0,0,"other:")
, mVstPlugins(owner,this,0,0,"vst plugins:")
, mPrefabs(owner,this,0,0,"prefabs:")
{
}

void SpawnListManager::SetModuleFactory(ModuleFactory* factory)
{
   mInstrumentModules.SetList(factory->GetSpawnableModules(kModuleType_Instrument), "");
   mNoteModules.SetList(factory->GetSpawnableModules(kModuleType_Note), "");
   mSynthModules.SetList(factory->GetSpawnableModules(kModuleType_Synth), "");
   mAudioModules.SetList(factory->GetSpawnableModules(kModuleType_Audio), "");
   mModulatorModules.SetList(factory->GetSpawnableModules(kModuleType_Modulator), "");
   mOtherModules.SetList(factory->GetSpawnableModules(kModuleType_Other), "");
   
   SetUpVstDropdown(false);
   
   File dir(ofToDataPath("prefabs"));
   Array<File> files;
   dir.findChildFiles(files, File::findFiles, false);
   vector<string> prefabs;
   for (auto file : files)
   {
      if (file.getFileExtension() == ".pfb")
         prefabs.push_back(file.getFileName().toStdString());
   }
   mPrefabs.SetList(prefabs, "prefab");
   
   mDropdowns.push_back(&mInstrumentModules);
   mDropdowns.push_back(&mNoteModules);
   mDropdowns.push_back(&mSynthModules);
   mDropdowns.push_back(&mAudioModules);
   mDropdowns.push_back(&mModulatorModules);
   mDropdowns.push_back(&mOtherModules);
   mDropdowns.push_back(&mVstPlugins);
   mDropdowns.push_back(&mPrefabs);
}

void SpawnListManager::SetUpVstDropdown(bool rescan)
{
   vector<string> vsts;
   VSTLookup::GetAvailableVSTs(vsts, rescan);
   vsts.push_back(kRescanPluginsLabel);
   mVstPlugins.SetList(vsts, "vstplugin");
}

void TitleBar::ListLayouts()
{
   mLoadLayoutDropdown->Clear();
   
   DirectoryIterator dir(File(ofToDataPath("layouts")), false);
   int layoutIdx = 0;
   while (dir.next())
   {
      File file = dir.getFile();
      if (file.getFileExtension() == ".json")
      {
         mLoadLayoutDropdown->AddLabel(file.getFileNameWithoutExtension().toRawUTF8(), layoutIdx);
         
         if (file.getRelativePathFrom(File(ofToDataPath(""))).toStdString() == TheSynth->GetLoadedLayout())
            mLoadLayoutIndex = layoutIdx;
         
         ++layoutIdx;
      }
   }
   
   mSaveLayoutButton->PositionTo(mLoadLayoutDropdown, kAnchor_Right);
}

void TitleBar::DrawModule()
{
   if (HiddenByZoom())
      return;
   
   ofSetColor(255,255,255);
   DrawTextBold("bespoke", 2, 28, 36);
   
   string info;
   if (TheSynth->GetMoveModule())
      info += " (moving module \"" + string(TheSynth->GetMoveModule()->Name()) + "\")";
   if (IKeyboardFocusListener::GetActiveKeyboardFocus())
      info += " (entering text)";
   
   DrawTextLeftJustify(info, ofGetWidth()/gDrawScale - 60, 16);
   
   if (ofGetWidth() / gDrawScale >= 620)
   {
      mSaveLayoutButton->Draw();
      mSaveStateButton->Draw();
      mLoadStateButton->Draw();
      mWriteAudioButton->Draw();
      mLoadLayoutDropdown->Draw();
      mResetLayoutButton->Draw();
   }
   
   if (ofGetWidth() / gDrawScale >= 920)
   {
      if (ofGetWidth() / gDrawScale >= 1280)
      {
         mSpawnLists.mInstrumentModules.SetPosition(400,2);
         mSpawnLists.mNoteModules.SetPositionRelativeTo(&mSpawnLists.mInstrumentModules);
         mSpawnLists.mSynthModules.SetPositionRelativeTo(&mSpawnLists.mNoteModules);
         mSpawnLists.mAudioModules.SetPositionRelativeTo(&mSpawnLists.mSynthModules);
         mSpawnLists.mModulatorModules.SetPositionRelativeTo(&mSpawnLists.mAudioModules);
         mSpawnLists.mOtherModules.SetPositionRelativeTo(&mSpawnLists.mModulatorModules);
         if (ofGetWidth() / gDrawScale >= 1550)
            mSpawnLists.mVstPlugins.SetPositionRelativeTo(&mSpawnLists.mOtherModules);
         else
            mSpawnLists.mVstPlugins.SetPosition(400,18);
         mSpawnLists.mPrefabs.SetPositionRelativeTo(&mSpawnLists.mVstPlugins);
      }
      else
      {
         mSpawnLists.mInstrumentModules.SetPosition(400,2);
         mSpawnLists.mNoteModules.SetPositionRelativeTo(&mSpawnLists.mInstrumentModules);
         mSpawnLists.mSynthModules.SetPositionRelativeTo(&mSpawnLists.mNoteModules);
         mSpawnLists.mAudioModules.SetPosition(400, 18);
         mSpawnLists.mModulatorModules.SetPositionRelativeTo(&mSpawnLists.mAudioModules);
         mSpawnLists.mOtherModules.SetPositionRelativeTo(&mSpawnLists.mModulatorModules);
         mSpawnLists.mVstPlugins.SetPositionRelativeTo(&mSpawnLists.mOtherModules);
         mSpawnLists.mPrefabs.SetPositionRelativeTo(&mSpawnLists.mVstPlugins);
      }
      
      //temporarily fake the module type to get the colors we want for each dropdown
      auto type = GetModuleType();
      mModuleType = kModuleType_Instrument;
      mSpawnLists.mInstrumentModules.Draw();
      mModuleType = kModuleType_Note;
      mSpawnLists.mNoteModules.Draw();
      mModuleType = kModuleType_Synth;
      mSpawnLists.mSynthModules.Draw();
      mModuleType = kModuleType_Audio;
      mSpawnLists.mAudioModules.Draw();
      mModuleType = kModuleType_Modulator;
      mSpawnLists.mModulatorModules.Draw();
      mModuleType = kModuleType_Other;
      mSpawnLists.mOtherModules.Draw();
      mModuleType = kModuleType_Synth;
      mSpawnLists.mVstPlugins.Draw();
      mModuleType = kModuleType_Other;
      mSpawnLists.mPrefabs.Draw();
      mModuleType = type;
   }
   
   float usage = TheSynth->GetGlobalManagers()->mDeviceManager.getCpuUsage();
   string stats;
   stats += "fps:" + ofToString(ofGetFrameRate(),0);
   stats += "  audio cpu:" + ofToString(usage * 100,1);
   if (usage > 1)
      ofSetColor(255,150,150);
   else
      ofSetColor(255,255,255);
   DrawTextLeftJustify(stats, ofGetWidth()/gDrawScale - 5, 33);
   mQuitButton->SetPosition(ofGetWidth()/gDrawScale - 31, 4);
   mQuitButton->Draw();
   mDisplayHelpButton->SetPosition(ofGetWidth()/gDrawScale - 51, 4);
   mDisplayHelpButton->Draw();
   mEventLookaheadCheckbox->Draw();
}

bool TitleBar::HiddenByZoom() const
{
   return ofGetWidth() / gDrawScale < 620;
}

void TitleBar::GetModuleDimensions(float& width, float& height)
{
   if (HiddenByZoom())
   {
      width = 0;
      height = 0;
      return;
   }
   
   width = ofGetWidth() / gDrawScale + 5;
   height = 36;
}

void TitleBar::CheckboxUpdated(Checkbox* checkbox)
{
}

void TitleBar::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mLoadLayoutDropdown)
   {
      string layout = mLoadLayoutDropdown->GetLabel(mLoadLayoutIndex);
      TheSynth->LoadLayoutFromFile(ofToDataPath("layouts/"+layout+".json"));
      return;
   }
   
   mSpawnLists.mInstrumentModules.OnSelection(list);
   mSpawnLists.mNoteModules.OnSelection(list);
   mSpawnLists.mSynthModules.OnSelection(list);
   mSpawnLists.mAudioModules.OnSelection(list);
   mSpawnLists.mModulatorModules.OnSelection(list);
   mSpawnLists.mOtherModules.OnSelection(list);
   mSpawnLists.mVstPlugins.OnSelection(list);
   mSpawnLists.mPrefabs.OnSelection(list);
}

void TitleBar::ButtonClicked(ClickButton* button)
{
   if (button == mSaveLayoutButton)
   {
      if (GetKeyModifiers() == kModifier_Shift)
         TheSynth->SaveLayoutAsPopup();
      else
         TheSynth->SaveLayout();
   }
   if (button == mSaveStateButton)
      TheSynth->SaveStatePopup();
   if (button == mLoadStateButton)
      TheSynth->LoadStatePopup();
   if (button == mWriteAudioButton)
      TheSynth->SaveOutput();
   if (button == mQuitButton)
      TheSynth->Exit();
   if (button == mDisplayHelpButton)
   {
      float x,y,w,h,butW,butH;
      mDisplayHelpButton->GetPosition(x, y);
      mDisplayHelpButton->GetDimensions(butW, butH);
      mHelpDisplay->GetDimensions(w, h);
      mHelpDisplay->SetPosition(x-w+butW,y+butH);
      TheSynth->PushModalFocusItem(mHelpDisplay);
   }
   if (button == mResetLayoutButton)
      TheSynth->LoadLayoutFromFile(ofToDataPath(TheSynth->GetUserPrefs()["layout"].asString()));
}


