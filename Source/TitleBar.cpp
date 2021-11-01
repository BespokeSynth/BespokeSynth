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
#include "UserPrefsEditor.h"
#include "UIControlMacros.h"
#include "VSTScanner.h"

#include "juce_audio_devices/juce_audio_devices.h"

TitleBar* TheTitleBar = nullptr;

//static
bool TitleBar::sShowInitialHelpOverlay = true;

namespace
{
   const std::string kManageVSTsLabel = "manage VSTs...";
}

SpawnList::SpawnList(IDropdownListener* owner, SpawnListManager* listManager, int x, int y, std::string label)
: mLabel(label)
, mSpawnIndex(-1)
, mSpawnList(nullptr)
, mOwner(owner)
, mListManager(listManager)
, mPos(x,y)
{
}

void SpawnList::SetList(std::vector<std::string> spawnables, std::string overrideModuleType)
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
      std::string name = mSpawnables[i].c_str();
      if (mOverrideModuleType == "" && TheSynth->GetModuleFactory()->IsExperimental(name))
         name += " (exp.)";
      if (mOverrideModuleType == "vstplugin" && name != kManageVSTsLabel)
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
         TheSynth->SetMoveModule(module, moduleGrabOffset.x, moduleGrabOffset.y, true);
      mSpawnIndex = -1;
   }
}

IDrawableModule* SpawnList::Spawn()
{
   std::string moduleType = mSpawnables[mSpawnIndex];
   if (mOverrideModuleType != "")
      moduleType = mOverrideModuleType;

   if (mOverrideModuleType == "vstplugin")
   {
      if (mSpawnables[mSpawnIndex] == kManageVSTsLabel)
      {
         TheTitleBar->ManageVSTs();
         return nullptr;
      }
   }
   
   IDrawableModule* module = TheSynth->SpawnModuleOnTheFly(moduleType, TheSynth->GetMouseX(TheSynth->GetRootContainer()) + moduleGrabOffset.x, TheSynth->GetMouseY(TheSynth->GetRootContainer()) + moduleGrabOffset.y);
   
   if (mOverrideModuleType == "vstplugin")
   {
      VSTPlugin* plugin = dynamic_cast<VSTPlugin*>(module);
      plugin->SetVST(mSpawnables[mSpawnIndex]);
   }
   
   if (mOverrideModuleType == "prefab")
   {
      Prefab* prefab = dynamic_cast<Prefab*>(module);
      prefab->LoadPrefab("prefabs"+GetPathSeparator()+mSpawnables[mSpawnIndex]);
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
, mSaveStateAsButton(nullptr)
, mLoadStateButton(nullptr)
, mWriteAudioButton(nullptr)
, mLoadLayoutDropdown(nullptr)
, mLoadLayoutIndex(-1)
, mSpawnLists(this)
, mLeftCornerHovered(false)
{
   assert(TheTitleBar == nullptr);
   TheTitleBar = this;
   
   mHelpDisplay = dynamic_cast<HelpDisplay*>(HelpDisplay::Create());
   mHelpDisplay->SetTypeName("helpdisplay");

   mNewPatchConfirmPopup.SetTypeName("newpatchconfirm");
   mNewPatchConfirmPopup.SetName("newpatchconfirm");
   
   SetShouldDrawOutline(false);
}

void TitleBar::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK(140, 1);
   BUTTON_STYLE(mPlayPauseButton, "play/pause", ButtonDisplayStyle::kPause); UIBLOCK_SHIFTRIGHT(); UIBLOCK_SHIFTX(10);
   BUTTON(mLoadStateButton, "load"); UIBLOCK_SHIFTRIGHT();
   BUTTON(mSaveStateButton,"save"); UIBLOCK_SHIFTRIGHT();
   BUTTON(mSaveStateAsButton, "save as"); UIBLOCK_SHIFTRIGHT(); UIBLOCK_SHIFTX(10);
   BUTTON(mWriteAudioButton, "write audio");
   UIBLOCK_NEWLINE();
   BUTTON(mResetLayoutButton,"new patch"); UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mEventLookaheadCheckbox, "lookahead (exp.)", &Transport::sDoEventLookahead); UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mShouldAutosaveCheckbox, "autosave", &ModularSynth::sShouldAutosave);
   ENDUIBLOCK0();

   mDisplayHelpButton = new ClickButton(this," ? ",380,1);
   mDisplayUserPrefsEditorButton = new ClickButton(this, "settings", 330, 1);   
   mLoadLayoutDropdown = new DropdownList(this, "load layout", 140, 20, &mLoadLayoutIndex);
   mSaveLayoutButton = new ClickButton(this, "save layout", 280, 19);
   
   mLoadLayoutDropdown->SetShowing(false);
   mSaveLayoutButton->SetShowing(false);
   
   mHelpDisplay->CreateUIControls();
   
   ListLayouts();

   mNewPatchConfirmPopup.CreateUIControls();
}

TitleBar::~TitleBar()
{
   assert(TheTitleBar == this);
   TheTitleBar = nullptr;
}

void TitleBar::ManageVSTs()
{
   if (mPluginListWindow == nullptr)
      mPluginListWindow.reset(new PluginListWindow(VSTPlugin::sFormatManager, this));

   mPluginListWindow->toFront(true);
}

void TitleBar::OnWindowClosed()
{
   mPluginListWindow.reset(nullptr);

   VSTPlugin::sPluginList.createXml()->writeTo(juce::File(ofToDataPath("vst/found_vsts.xml")));
}

SpawnListManager::SpawnListManager(IDropdownListener* owner)
: mInstrumentModules(owner,this,500,16,"instruments:")
, mNoteModules(owner,this,0,0,"note effects:")
, mSynthModules(owner,this,0,0,"synths:")
, mAudioModules(owner,this,0,0,"audio effects:")
, mModulatorModules(owner,this,0,0,"modulators:")
, mPulseModules(owner, this, 0, 0, "pulse:")
, mVstPlugins(owner, this, 0, 0, "vst plugins:")
, mOtherModules(owner,this,0,0,"other:")
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
   mPulseModules.SetList(factory->GetSpawnableModules(kModuleType_Pulse), "");
   mOtherModules.SetList(factory->GetSpawnableModules(kModuleType_Other), "");
   
   SetUpVstDropdown();
   
   std::vector<std::string> prefabs;
   ModuleFactory::GetPrefabs(prefabs);
   mPrefabs.SetList(prefabs, "prefab");
   
   mDropdowns.push_back(&mInstrumentModules);
   mDropdowns.push_back(&mNoteModules);
   mDropdowns.push_back(&mSynthModules);
   mDropdowns.push_back(&mAudioModules);
   mDropdowns.push_back(&mModulatorModules);
   mDropdowns.push_back(&mPulseModules);
   mDropdowns.push_back(&mVstPlugins);
   mDropdowns.push_back(&mOtherModules);
   mDropdowns.push_back(&mPrefabs);
}

void SpawnListManager::SetUpVstDropdown()
{
   std::vector<std::string> vsts;
   VSTLookup::GetAvailableVSTs(vsts);
   vsts.insert(vsts.begin(), kManageVSTsLabel);
   mVstPlugins.SetList(vsts, "vstplugin");
}

void TitleBar::ListLayouts()
{
   mLoadLayoutDropdown->Clear();
   
   int layoutIdx = 0;
   for (const auto& entry : juce::RangedDirectoryIterator{juce::File{ofToDataPath("layouts")}, false, "*.json"})
   {
      const auto& file = entry.getFile();
      mLoadLayoutDropdown->AddLabel(file.getFileNameWithoutExtension().toRawUTF8(), layoutIdx);

      if (file.getRelativePathFrom(juce::File{ofToDataPath("")}).toStdString() == TheSynth->GetLoadedLayout())
         mLoadLayoutIndex = layoutIdx;

      ++layoutIdx;
   }
   
   mSaveLayoutButton->PositionTo(mLoadLayoutDropdown, kAnchor_Right);
}

void TitleBar::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x,y,right);
   
   if (mLeftCornerHovered)
      TheSynth->GetLocationZoomer()->EnterVanityPanningMode();
}

bool TitleBar::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x,y);
   
   if (x < 130 && y < 36)
      mLeftCornerHovered = true;
   else
      mLeftCornerHovered = false;
   
   return false;
}

namespace
{
   const float kDoubleHeightThreshold = 1200;
}

float TitleBar::GetPixelWidth() const
{
   return ofGetWidth() / GetOwningContainer()->GetDrawScale();
}

void TitleBar::DrawModule()
{
   if (HiddenByZoom())
      return;
   
   ofSetColor(255,255,255);
   
   ofPushStyle();
   if (gHoveredModule == this && mLeftCornerHovered)
      ofSetColor(ofColor::lerp(ofColor::black, ofColor::white, ofMap(sin(gTime / 1000 * PI * 2),-1,1,.7f,.9f)));
   DrawTextBold("bespoke", 2, 28, 36);
#if DEBUG
   ofFill();
   ofSetColor(0, 0, 0, 180);
   ofRect(13, 12, 90, 17);
   ofSetColor(255, 0, 0);
   DrawTextBold("debug build", 17, 25, 19);
#endif
   ofPopStyle();
   
   std::string info;
   if (TheSynth->GetMoveModule())
      info += " (moving module \"" + std::string(TheSynth->GetMoveModule()->Name()) + "\")";
   if (IKeyboardFocusListener::GetActiveKeyboardFocus())
      info += " (entering text)";

   float pixelWidth = GetPixelWidth();
   
   DrawTextRightJustify(info, pixelWidth - 140, 32);
   
   mSaveLayoutButton->Draw();
   mSaveStateButton->Draw();
   mSaveStateAsButton->Draw();
   mLoadStateButton->Draw();
   mWriteAudioButton->Draw();
   mLoadLayoutDropdown->Draw();
   mResetLayoutButton->Draw();
   if (TheSynth->IsAudioPaused())
      mPlayPauseButton->SetDisplayStyle(ButtonDisplayStyle::kPlay);
   else
      mPlayPauseButton->SetDisplayStyle(ButtonDisplayStyle::kPause);
   mPlayPauseButton->Draw();

   float startX = 400;
   float startY = 2;

   if (pixelWidth < kDoubleHeightThreshold)
   {
      startX = 10;
      startY += 16 * 2 + 4;
   }

   float x = startX;
   float y = startY;
   std::array<SpawnList*, 9> lists = { &mSpawnLists.mInstrumentModules,
                                  &mSpawnLists.mNoteModules,
                                  &mSpawnLists.mSynthModules,
                                  &mSpawnLists.mAudioModules,
                                  &mSpawnLists.mModulatorModules,
                                  &mSpawnLists.mPulseModules,
                                  &mSpawnLists.mVstPlugins,
                                  &mSpawnLists.mOtherModules,
                                  &mSpawnLists.mPrefabs };

   for (auto list : lists)
   {
      list->SetPosition(x, y);
      float w, h;
      list->GetList()->GetDimensions(w, h);
      x += w + 5;

      if (x >= pixelWidth - 260)
      {
         x = startX;
         y += 18;
      }
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
   mModuleType = kModuleType_Pulse;
   mSpawnLists.mPulseModules.Draw();
   mModuleType = kModuleType_Synth;
   mSpawnLists.mVstPlugins.Draw();
   mModuleType = kModuleType_Other;
   mSpawnLists.mOtherModules.Draw();
   mModuleType = kModuleType_Other;
   mSpawnLists.mPrefabs.Draw();
   mModuleType = type;
   
   float usage = TheSynth->GetAudioDeviceManager().getCpuUsage();
   std::string stats;
   stats += "fps:" + ofToString(ofGetFrameRate(),0);
   stats += "  audio cpu:" + ofToString(usage * 100,1);
   if (usage > 1)
      ofSetColor(255,150,150);
   else
      ofSetColor(255,255,255);
   DrawTextRightJustify(stats, ofGetWidth()/GetOwningContainer()->GetDrawScale() - 5, 33);
   mDisplayHelpButton->SetPosition(ofGetWidth()/GetOwningContainer()->GetDrawScale() - 20, 4);
   mDisplayHelpButton->Draw();
   mDisplayUserPrefsEditorButton->SetPosition(mDisplayHelpButton->GetPosition(true).x - 55, 4);
   mDisplayUserPrefsEditorButton->Draw();
   mEventLookaheadCheckbox->Draw();
   mShouldAutosaveCheckbox->Draw();
}

void TitleBar::DrawModuleUnclipped()
{
   if (mPluginListWindow != nullptr)
   {
      ofPushStyle();
      ofSetColor(255, 255, 255);
      float titleBarWidth, titleBarHeight;
      TheTitleBar->GetDimensions(titleBarWidth, titleBarHeight);
      float x = 100;
      float y = 50 + titleBarHeight;
      gFontBold.DrawString("please close VST manager to continue", 50, x, y);
      ofPopStyle();
      return;
   }

   float saveCooldown = 1 - ofClamp((gTime - TheSynth->GetLastSaveTime()) / 1000, 0, 1);
   if (saveCooldown > 0)
   {
      ofPushStyle();
      ofSetColor(255, 255, 255, saveCooldown * 255);
      float titleBarWidth, titleBarHeight;
      TheTitleBar->GetDimensions(titleBarWidth, titleBarHeight);
      float x = 100;
      float y = 40 + titleBarHeight;
      std::string filename = juce::File(TheSynth->GetLastSavePath()).getFileName().toStdString();
      gFontBold.DrawString("saved "+filename, 50, x, y);
      ofPopStyle();
   }

   if (HiddenByZoom())
      return;

   if (sShowInitialHelpOverlay)
   {
      ofPushStyle();
      ofSetColor(255, 255, 255);
      std::string text = "click ? to view help and toggle tooltips";
      float size = 28;
      float titleBarWidth, titleBarHeight;
      TheTitleBar->GetDimensions(titleBarWidth, titleBarHeight);
      ofRectangle helpButtonRect = mDisplayHelpButton->GetRect(true);
      float x = helpButtonRect.getCenter().x;
      float y = helpButtonRect.getCenter().y + 15 + titleBarHeight;
      gFontBold.DrawString(text, size, x - gFontBold.GetStringWidth(text, size) - 15 * GetOwningContainer()->GetDrawScale(), y);
      ofSetLineWidth(2);
      float scale = GetOwningContainer()->GetDrawScale();
      ofLine(x - 10, y - 6 * scale, x, y - 6 * scale);
      ofLine(x, y - 6 * scale, x, y - 18 * scale);
      ofLine(x - 3 * scale, y - 15 * scale, x, y - 18 * scale);
      ofLine(x + 3 * scale, y - 15 * scale, x, y - 18 * scale);
      ofPopStyle();
   }
}

bool TitleBar::HiddenByZoom() const
{
   return false; //ofGetWidth() / GetOwningContainer()->GetDrawScale() < 620;
}

void TitleBar::GetModuleDimensions(float& width, float& height)
{
   if (HiddenByZoom())
   {
      width = 0;
      height = 0;
      return;
   }
   
   width = ofGetWidth() / GetOwningContainer()->GetDrawScale() + 5;
   if (GetPixelWidth() < kDoubleHeightThreshold)
      height = 36 * 2;
   else
      height = 36;
}

void TitleBar::CheckboxUpdated(Checkbox* checkbox)
{
}

void TitleBar::DropdownClicked(DropdownList* list)
{
   if (list == mSpawnLists.mVstPlugins.GetList())
      mSpawnLists.SetUpVstDropdown();
}

void TitleBar::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mLoadLayoutDropdown)
   {
      std::string layout = mLoadLayoutDropdown->GetLabel(mLoadLayoutIndex);
      TheSynth->LoadLayoutFromFile(ofToDataPath("layouts/"+layout+".json"));
      return;
   }
   
   mSpawnLists.mInstrumentModules.OnSelection(list);
   mSpawnLists.mNoteModules.OnSelection(list);
   mSpawnLists.mSynthModules.OnSelection(list);
   mSpawnLists.mAudioModules.OnSelection(list);
   mSpawnLists.mModulatorModules.OnSelection(list);
   mSpawnLists.mPulseModules.OnSelection(list);
   mSpawnLists.mVstPlugins.OnSelection(list);
   mSpawnLists.mOtherModules.OnSelection(list);
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
      TheSynth->SaveCurrentState();
   if (button == mSaveStateAsButton)
      TheSynth->SaveStatePopup();
   if (button == mLoadStateButton)
      TheSynth->LoadStatePopup();
   if (button == mWriteAudioButton)
      TheSynth->SaveOutput();
   if (button == mDisplayHelpButton)
   {
      float x, y, w, h, butW, butH;
      mDisplayHelpButton->GetPosition(x, y);
      mDisplayHelpButton->GetDimensions(butW, butH);
      mHelpDisplay->GetDimensions(w, h);
      mHelpDisplay->SetPosition(x - w + butW, y + butH);
      mHelpDisplay->SetOwningContainer(GetOwningContainer());
      TheSynth->PushModalFocusItem(mHelpDisplay);
      sShowInitialHelpOverlay = false;
   }
   if (button == mDisplayUserPrefsEditorButton)
      TheSynth->GetUserPrefsEditor()->Show();
   if (button == mResetLayoutButton)
   {
      auto buttonRect = mResetLayoutButton->GetRect();
      mNewPatchConfirmPopup.SetOwningContainer(GetOwningContainer());
      mNewPatchConfirmPopup.SetPosition(buttonRect.x, buttonRect.y + buttonRect.height + 2);
      TheSynth->PushModalFocusItem(&mNewPatchConfirmPopup);
   }
   if (button == mPlayPauseButton)
      TheSynth->ToggleAudioPaused();
}

void NewPatchConfirmPopup::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK(3, 20);
   BUTTON(mConfirmButton, "confirm"); UIBLOCK_SHIFTRIGHT(); UIBLOCK_SHIFTX(5);
   BUTTON(mCancelButton, "cancel");
   ENDUIBLOCK(mWidth, mHeight);
}

void NewPatchConfirmPopup::DrawModule()
{
   DrawTextNormal("clear this patch?", 3, 14);

   mConfirmButton->Draw();
   mCancelButton->Draw();
}

void NewPatchConfirmPopup::ButtonClicked(ClickButton* button)
{
   if (button == mConfirmButton)
      TheSynth->ReloadInitialLayout();
   if (button == mCancelButton)
      TheSynth->PopModalFocusItem();
}
