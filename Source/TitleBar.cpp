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
#include "Prefab.h"
#include "UserPrefsEditor.h"
#include "UIControlMacros.h"
#include "VSTPlugin.h"
#include "VSTScanner.h"
#include "MidiController.h"

#include "juce_audio_devices/juce_audio_devices.h"

TitleBar* TheTitleBar = nullptr;

//static
bool TitleBar::sShowInitialHelpOverlay = true;

namespace
{
   const std::string kManagePluginsLabel = "manage plugins...";
   const std::string kPluginsDropdownLabel = "plugins:";
}

SpawnList::SpawnList(IDropdownListener* owner, int x, int y, std::string label, ModuleCategory moduleCategory, bool showDecorators)
: mLabel(label)
, mOwner(owner)
, mPos(x, y)
, mModuleCategory(moduleCategory)
, mShowDecorators(showDecorators)
{
}

void SpawnList::SetList(std::vector<ModuleFactory::Spawnable> spawnables)
{
   if (mSpawnList == nullptr)
      mSpawnList = new DropdownList(mOwner, mLabel.c_str(), mPos.x, mPos.y, &mSpawnIndex);

   mSpawnList->Clear();
   mSpawnList->SetUnknownItemString(mLabel);
   mSpawnables = spawnables;
   for (int i = 0; i < mSpawnables.size(); ++i)
   {
      std::string name = mSpawnables[i].mLabel;
      if (mShowDecorators && !mSpawnables[i].mDecorator.empty())
         name += " " + mSpawnables[i].mDecorator;
      if (TheSynth->GetModuleFactory()->IsExperimental(name))
         name += " (exp.)";
      mSpawnList->AddLabel(name, i);
   }
}

namespace
{
   ofVec2f kModuleGrabOffset(-40, 10);
}

void SpawnList::OnSelection(DropdownList* list)
{
   if (list == mSpawnList)
   {
      IDrawableModule* module = Spawn(mSpawnIndex);
      if (module != nullptr)
         TheSynth->SetMoveModule(module, kModuleGrabOffset.x, kModuleGrabOffset.y, true);
      mSpawnIndex = -1;
   }
}

IDrawableModule* SpawnList::Spawn(int index)
{
   if (mLabel == kPluginsDropdownLabel && index == 0)
   {
      TheTitleBar->ManagePlugins();
      return nullptr;
   }

   IDrawableModule* module = TheSynth->SpawnModuleOnTheFly(mSpawnables[index], TheSynth->GetMouseX(TheSynth->GetRootContainer()) + kModuleGrabOffset.x, TheSynth->GetMouseY(TheSynth->GetRootContainer()) + kModuleGrabOffset.y);

   return module;
}

void SpawnList::Draw()
{
   float x, y;
   mSpawnList->GetPosition(x, y, true);
   //DrawTextNormal(mLabel,x,y-2);
   mSpawnList->Draw();
}

void SpawnList::SetPosition(int x, int y)
{
   mSpawnList->SetPosition(x, y);
}

TitleBar::TitleBar()
: mSpawnLists(this)
{
   assert(TheTitleBar == nullptr);
   TheTitleBar = this;

   mHelpDisplay = dynamic_cast<HelpDisplay*>(HelpDisplay::Create());
   mHelpDisplay->SetTypeName("helpdisplay", kModuleCategory_Other);

   mNewPatchConfirmPopup.SetTypeName("newpatchconfirm", kModuleCategory_Other);
   mNewPatchConfirmPopup.SetName("newpatchconfirm");

   SetShouldDrawOutline(false);
}

void TitleBar::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK(140, 1);
   BUTTON_STYLE(mPlayPauseButton, "play/pause", ButtonDisplayStyle::kPause);
   UIBLOCK_SHIFTRIGHT();
   UIBLOCK_SHIFTX(10);
   BUTTON(mLoadStateButton, "load");
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mSaveStateButton, "save");
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mSaveStateAsButton, "save as");
   UIBLOCK_SHIFTRIGHT();
   UIBLOCK_SHIFTX(10);
   BUTTON(mWriteAudioButton, "write audio");
   UIBLOCK_NEWLINE();
   BUTTON(mResetLayoutButton, "new patch");
   UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mEventLookaheadCheckbox, "lookahead (exp.)", &Transport::sDoEventLookahead);
   UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mShouldAutosaveCheckbox, "autosave", &ModularSynth::sShouldAutosave);
   ENDUIBLOCK0();

   mDisplayHelpButton = new ClickButton(this, " ? ", 380, 1);
   mDisplayUserPrefsEditorButton = new ClickButton(this, "settings", 330, 1);
   mHomeButton = new ClickButton(this, "home", 330, 1);
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

void TitleBar::ManagePlugins()
{
   if (mPluginListWindow == nullptr)
      mPluginListWindow.reset(new PluginListWindow(TheSynth->GetAudioPluginFormatManager(), this));

   mPluginListWindow->toFront(true);
}

void TitleBar::OnWindowClosed()
{
   mPluginListWindow.reset(nullptr);

   TheSynth->GetKnownPluginList().createXml()->writeTo(juce::File(ofToDataPath("vst/found_vsts.xml")));
}

SpawnListManager::SpawnListManager(IDropdownListener* owner)
: mInstrumentModules(owner, 500, 16, "instruments:", kModuleCategory_Instrument, true)
, mNoteModules(owner, 0, 0, "note effects:", kModuleCategory_Note, true)
, mSynthModules(owner, 0, 0, "synths:", kModuleCategory_Synth, true)
, mAudioModules(owner, 0, 0, "audio effects:", kModuleCategory_Audio, true)
, mModulatorModules(owner, 0, 0, "modulators:", kModuleCategory_Modulator, true)
, mPulseModules(owner, 0, 0, "pulse:", kModuleCategory_Pulse, true)
, mPlugins(owner, 0, 0, kPluginsDropdownLabel, kModuleCategory_Synth, true)
, mOtherModules(owner, 0, 0, "other:", kModuleCategory_Other, true)
, mPrefabs(owner, 0, 0, "prefabs:", kModuleCategory_Other, false)
{
}

void SpawnListManager::SetModuleFactory(ModuleFactory* factory)
{
   mInstrumentModules.SetList(factory->GetSpawnableModules(kModuleCategory_Instrument));
   mNoteModules.SetList(factory->GetSpawnableModules(kModuleCategory_Note));
   mSynthModules.SetList(factory->GetSpawnableModules(kModuleCategory_Synth));
   mAudioModules.SetList(factory->GetSpawnableModules(kModuleCategory_Audio));
   mModulatorModules.SetList(factory->GetSpawnableModules(kModuleCategory_Modulator));
   mPulseModules.SetList(factory->GetSpawnableModules(kModuleCategory_Pulse));
   mOtherModules.SetList(factory->GetSpawnableModules(kModuleCategory_Other));

   SetUpPluginsDropdown();
   SetUpPrefabsDropdown();

   mDropdowns.push_back(&mInstrumentModules);
   mDropdowns.push_back(&mNoteModules);
   mDropdowns.push_back(&mSynthModules);
   mDropdowns.push_back(&mAudioModules);
   mDropdowns.push_back(&mModulatorModules);
   mDropdowns.push_back(&mPulseModules);
   mDropdowns.push_back(&mPlugins);
   mDropdowns.push_back(&mOtherModules);
   mDropdowns.push_back(&mPrefabs);
}

void SpawnListManager::SetUpPrefabsDropdown()
{
   std::vector<ModuleFactory::Spawnable> prefabs;
   ModuleFactory::GetPrefabs(prefabs);
   mPrefabs.SetList(prefabs);
}

void SpawnListManager::SetUpPluginsDropdown()
{
   std::vector<ModuleFactory::Spawnable> list;

   ModuleFactory::Spawnable scanDummy;
   scanDummy.mLabel = kManagePluginsLabel;
   list.push_back(scanDummy);

   std::vector<juce::PluginDescription> recentPlugins;
   VSTLookup::GetRecentPlugins(recentPlugins, 8);
   for (auto& pluginDesc : recentPlugins)
   {
      ModuleFactory::Spawnable spawnable{};
      spawnable.mLabel = pluginDesc.name.toStdString();
      spawnable.mDecorator = "[" + ModuleFactory::Spawnable::GetPluginLabel(pluginDesc) + "]";
      spawnable.mPluginDesc = pluginDesc;
      spawnable.mSpawnMethod = ModuleFactory::SpawnMethod::Plugin;
      list.push_back(spawnable);
   }

   std::vector<juce::PluginDescription> vsts;
   VSTLookup::GetAvailableVSTs(vsts);
   for (auto& pluginDesc : vsts)
   {
      ModuleFactory::Spawnable spawnable{};
      spawnable.mLabel = pluginDesc.name.toStdString();
      spawnable.mDecorator = "[" + ModuleFactory::Spawnable::GetPluginLabel(pluginDesc) + "]";
      spawnable.mPluginDesc = pluginDesc;
      spawnable.mSpawnMethod = ModuleFactory::SpawnMethod::Plugin;
      list.push_back(spawnable);
   }

   mPlugins.SetList(list);
   mPlugins.GetList()->ClearSeparators();
   mPlugins.GetList()->AddSeparator(1);
   if (!recentPlugins.empty())
      mPlugins.GetList()->AddSeparator((int)recentPlugins.size() + 1);
}

void TitleBar::ListLayouts()
{
   mLoadLayoutDropdown->Clear();

   int layoutIdx = 0;
   for (const auto& entry : juce::RangedDirectoryIterator{ juce::File{ ofToDataPath("layouts") }, false, "*.json" })
   {
      const auto& file = entry.getFile();
      mLoadLayoutDropdown->AddLabel(file.getFileNameWithoutExtension().toRawUTF8(), layoutIdx);

      if (file.getRelativePathFrom(juce::File{ ofToDataPath("") }).toStdString() == TheSynth->GetLoadedLayout())
         mLoadLayoutIndex = layoutIdx;

      ++layoutIdx;
   }

   mSaveLayoutButton->PositionTo(mLoadLayoutDropdown, kAnchor_Right);
}

void TitleBar::Poll()
{
   mHelpDisplay->Poll();
}

void TitleBar::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   if (mLeftCornerHovered)
      TheSynth->GetLocationZoomer()->EnterVanityPanningMode();
}

bool TitleBar::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);

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

   ofSetColor(255, 255, 255);

   ofPushStyle();
   if (gHoveredModule == this && mLeftCornerHovered)
      ofSetColor(ofColor::lerp(ofColor::black, ofColor::white, ofMap(sin(gTime / 1000 * PI * 2), -1, 1, .7f, .9f)));
   DrawTextBold("bespoke", 2, 28, 34);
#if BESPOKE_NIGHTLY && !BESPOKE_SUPPRESS_NIGHTLY_LABEL
   DrawTextNormal("nightly", 90, 35, 8);
#endif
#if DEBUG
   ofFill();
   ofSetColor(0, 0, 0, 180);
   ofRect(13, 12, 90, 17);
   ofSetColor(255, 0, 0);
   DrawTextBold("debug build", 17, 25, 17);
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
   for (auto* spawnList : mSpawnLists.GetDropdowns())
   {
      spawnList->SetPosition(x, y);
      float w, h;
      spawnList->GetList()->GetDimensions(w, h);
      x += w + 5;

      if (x >= pixelWidth - 260)
      {
         x = startX;
         y += 18;
      }
   }

   //temporarily fake the module type to get the colors we want for each dropdown
   auto type = GetModuleCategory();
   for (auto* spawnList : mSpawnLists.GetDropdowns())
   {
      mModuleCategory = spawnList->GetCategory();
      spawnList->Draw();
   }
   mModuleCategory = type;

   float usage = TheSynth->GetAudioDeviceManager().getCpuUsage();
   std::string stats;
   stats += "fps:" + ofToString(ofGetFrameRate(), 0);
   stats += "  audio cpu:" + ofToString(usage * 100, 1);
   if (usage > 1)
      ofSetColor(255, 150, 150);
   else
      ofSetColor(255, 255, 255);
   DrawTextRightJustify(stats, ofGetWidth() / GetOwningContainer()->GetDrawScale() - 5, 33);
   mDisplayHelpButton->SetPosition(ofGetWidth() / GetOwningContainer()->GetDrawScale() - 20, 4);
   mDisplayHelpButton->Draw();
   mDisplayUserPrefsEditorButton->SetPosition(mDisplayHelpButton->GetPosition(true).x - mDisplayUserPrefsEditorButton->GetRect().width - 5, 4);
   mDisplayUserPrefsEditorButton->Draw();
   mHomeButton->SetPosition(mDisplayUserPrefsEditorButton->GetPosition(true).x - mHomeButton->GetRect().width - 5, 4);
   mHomeButton->Draw();
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
      gFontBold.DrawString("please close plugin manager to continue", 48, x, y);
      ofPopStyle();
      return;
   }

   float displayMessageCooldown = 1 - ofClamp((gTime - mDisplayMessageTime) / 1000, 0, 1);
   if (displayMessageCooldown > 0)
   {
      ofPushStyle();
      ofSetColor(255, 255, 255, displayMessageCooldown * 255);
      float titleBarWidth, titleBarHeight;
      TheTitleBar->GetDimensions(titleBarWidth, titleBarHeight);
      float x = 100;
      float y = 40 + titleBarHeight;
      gFontBold.DrawString(mDisplayMessage, 48, x, y);
      ofPopStyle();
   }

   if (HiddenByZoom())
      return;

   if (sShowInitialHelpOverlay)
   {
      ofPushStyle();
      ofSetColor(255, 255, 255);
      std::string text = "click ? to view help and toggle tooltips";
      float size = 26;
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

   //midicontroller
   {
      const float kDisplayMs = 500;
      std::string displayString;

      IUIControl* drawControl = nullptr;
      if (gTime < MidiController::sLastBoundControlTime + kDisplayMs)
      {
         drawControl = MidiController::sLastBoundUIControl;
         if (drawControl != nullptr)
            displayString = drawControl->Path() + " bound!";
      }
      else if (gTime < MidiController::sLastConnectedActivityTime + kDisplayMs)
      {
         drawControl = MidiController::sLastActivityUIControl;
         if (drawControl != nullptr)
            displayString = drawControl->Path(false, true) + ": " + drawControl->GetDisplayValue(drawControl->GetValue());
      }

      if (!displayString.empty() && drawControl != nullptr)
      {
         ofPushStyle();
         ofFill();
         ofVec2f pos(50, ofGetHeight() / GetOwningContainer()->GetDrawScale() - 100);
         const float kWidth = 600;
         const float kHeight = 70;
         ofSetColor(80, 80, 80, 150);
         ofRect(pos.x, pos.y, kWidth, kHeight);
         ofSetColor(120, 120, 120, 150);
         ofRect(pos.x, pos.y, kWidth * drawControl->GetMidiValue(), kHeight);
         ofSetColor(255, 255, 255);
         DrawTextBold(displayString, pos.x + 20, pos.y + 50, 38);
         ofPopStyle();
      }
   }
}

void TitleBar::DisplayTemporaryMessage(std::string message)
{
   mDisplayMessage = message;
   mDisplayMessageTime = gTime;
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

void TitleBar::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void TitleBar::DropdownClicked(DropdownList* list)
{
   if (list == mSpawnLists.mPlugins.GetList())
      mSpawnLists.SetUpPluginsDropdown();

   if (list == mSpawnLists.mPrefabs.GetList())
      mSpawnLists.SetUpPrefabsDropdown();
}

void TitleBar::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mLoadLayoutDropdown)
   {
      std::string layout = mLoadLayoutDropdown->GetLabel(mLoadLayoutIndex);
      TheSynth->LoadLayoutFromFile(ofToDataPath("layouts/" + layout + ".json"));
      return;
   }

   for (auto* spawnList : mSpawnLists.GetDropdowns())
      spawnList->OnSelection(list);
}

void TitleBar::ButtonClicked(ClickButton* button, double time)
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
      mHelpDisplay->Show();
      TheSynth->PushModalFocusItem(mHelpDisplay);
      sShowInitialHelpOverlay = false;
   }
   if (button == mDisplayUserPrefsEditorButton)
      TheSynth->GetUserPrefsEditor()->Show();
   if (button == mHomeButton)
      TheSynth->GetLocationZoomer()->GoHome();
   if (button == mResetLayoutButton)
   {
      auto buttonRect = mResetLayoutButton->GetRect();
      mNewPatchConfirmPopup.SetOwningContainer(GetOwningContainer());
      mNewPatchConfirmPopup.SetPosition(buttonRect.x, buttonRect.y + buttonRect.height + 2);
      TheSynth->PushModalFocusItem(&mNewPatchConfirmPopup);
   }
   if (button == mPlayPauseButton)
      TheSynth->SetAudioPaused(!TheSynth->IsAudioPaused());
}

void NewPatchConfirmPopup::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK(3, 20);
   BUTTON(mConfirmButton, "confirm");
   UIBLOCK_SHIFTRIGHT();
   UIBLOCK_SHIFTX(5);
   BUTTON(mCancelButton, "cancel");
   ENDUIBLOCK(mWidth, mHeight);
}

void NewPatchConfirmPopup::DrawModule()
{
   DrawTextNormal("clear this patch?", 3, 14);

   mConfirmButton->Draw();
   mCancelButton->Draw();
}

void NewPatchConfirmPopup::ButtonClicked(ClickButton* button, double time)
{
   if (button == mConfirmButton)
      TheSynth->ReloadInitialLayout();
   if (button == mCancelButton)
      TheSynth->PopModalFocusItem();
}
