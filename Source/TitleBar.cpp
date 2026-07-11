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
#include "UserPrefs.h"
#include "ModuleSaveDataPanel.h"
#include "HelpDisplay.h"
#include "Prefab.h"
#include "UserPrefsEditor.h"
#include "SearchPanel.h"
#include "UIControlMacros.h"
#include "VSTPlugin.h"
#include "VSTScanner.h"
#include "MidiController.h"
#include "WelcomeScreen.h"

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
   //BUTTON(mLoadStateButton, "load");
   //UIBLOCK_SHIFTRIGHT();
   BUTTON(mSaveStateButton, "save");
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mSaveStateAsButton, "save as");
   UIBLOCK_SHIFTRIGHT();
   UIBLOCK_SHIFTX(10);
   BUTTON(mWriteAudioButton, "export audio");
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mRecordButton, "record");
   UIBLOCK_NEWLINE();
   //BUTTON(mResetLayoutButton, "new patch");
   BUTTON(mWelcomeScreenButton, "load/new");
   UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mEventLookaheadCheckbox, "lookahead (exp.)", &Transport::sDoEventLookahead);
   UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mShouldAutosaveCheckbox, "autosave", &ModularSynth::sShouldAutosave);
   ENDUIBLOCK0();

   mDisplayHelpButton = new ClickButton(this, " ? ", 380, 1);
   mDisplayUserPrefsEditorButton = new ClickButton(this, "settings", 330, 1);
   mHomeButton = new ClickButton(this, "home", 330, 1);
   mSearchToggleButton = new ClickButton(this, "search", 330, 1); //toggles the right-docked search/browser panel open/closed
   mPaletteDropdown = new DropdownList(this, "palette", 330, 1, &mPaletteIndex);
   mPaletteDropdown->AddLabel("classic", 0);
   mPaletteDropdown->AddLabel("neon", 1);
   mPaletteDropdown->AddLabel("pastel", 2);
   mPaletteDropdown->AddLabel("mono", 3);
   mPaletteDropdown->AddLabel("sunset", 4);
   mPaletteDropdown->AddLabel("ocean", 5);
   mPaletteDropdown->AddLabel("acid", 6);
   mPaletteDropdown->AddLabel("vaporwave", 7);
   mPaletteDropdown->AddLabel("toxic", 8);
   mPaletteDropdown->AddLabel("bloodmoon", 9);
   mPaletteDropdown->AddLabel("cyberpunk", 10);
   mPaletteDropdown->AddLabel("glitch", 11);
   mPaletteDropdown->AddLabel("midnight", 12);
   mPaletteDropdown->AddLabel("aurora", 13);
   mPaletteDropdown->AddLabel("forest", 14);
   mPaletteDropdown->AddLabel("candy", 15);
   mPaletteDropdown->AddLabel("ember", 16);
   mPaletteDropdown->AddLabel("arctic", 17);
   mPaletteDropdown->AddLabel("grape", 18);
   mPaletteDropdown->AddLabel("sepia", 19);
   mPaletteDropdown->AddLabel("matrix", 20);
   mPaletteDropdown->AddLabel("dracula", 21);
   mPaletteDropdown->AddLabel("solarized", 22);
   mPaletteDropdown->AddLabel("mono light", 23);
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
, mVisualizerModules(owner, 0, 0, "visualizers:", kModuleCategory_Visualizer, true)
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
   mVisualizerModules.SetList(factory->GetSpawnableModules(kModuleCategory_Visualizer));
   mOtherModules.SetList(factory->GetSpawnableModules(kModuleCategory_Other));

   SetUpPluginsDropdown();
   SetUpPrefabsDropdown();

   mDropdowns.push_back(&mInstrumentModules);
   mDropdowns.push_back(&mNoteModules);
   mDropdowns.push_back(&mSynthModules);
   mDropdowns.push_back(&mAudioModules);
   mDropdowns.push_back(&mModulatorModules);
   mDropdowns.push_back(&mPulseModules);
   mDropdowns.push_back(&mVisualizerModules);
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
   if (mLoadStateButton)
      mLoadStateButton->Draw();
   mWriteAudioButton->Draw();

   mRecordButton->SetName(TheSynth->IsRecordingSession() ? "stop" : "record");
   mRecordButton->Draw();
   if (TheSynth->IsRecordingSession())
   {
      //pulsing red dot next to the record button while a take is being captured
      ofPushStyle();
      float pulse = ofMap(sin(gTime / 300 * PI * 2), -1, 1, .5f, 1);
      ofFill();
      ofSetColor(255, 60, 60, 255 * pulse);
      ofRectangle recordRect = mRecordButton->GetRect();
      ofCircle(recordRect.x - 7, recordRect.y + recordRect.height / 2, 3.5f);
      ofPopStyle();
   }

   mLoadLayoutDropdown->Draw();
   if (mResetLayoutButton)
      mResetLayoutButton->Draw();
   if (mWelcomeScreenButton)
      mWelcomeScreenButton->Draw();
   if (TheSynth->IsAudioPaused())
      mPlayPauseButton->SetDisplayStyle(ButtonDisplayStyle::kPlay);
   else
      mPlayPauseButton->SetDisplayStyle(ButtonDisplayStyle::kPause);
   mPlayPauseButton->Draw();

   float startX = 470; //leaves room on row 1 for the palette dropdown that now sits after 'record'
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

      if (x >= pixelWidth - 340) //reserve extra room on the right so the spawn dropdowns never run
      {                          //into the palette/search/home/settings/help cluster or the cpu stats
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
   //palette (colour presets) now lives in the top-LEFT cluster, right after the 'record' button
   mPaletteDropdown->SetPosition(mRecordButton->GetPosition(true).x + mRecordButton->GetRect().width + 10, mRecordButton->GetPosition(true).y - 1);
   mPaletteDropdown->Draw();
   //search toggle stays on the right, just left of the 'home' button
   mSearchToggleButton->SetPosition(mHomeButton->GetPosition(true).x - mSearchToggleButton->GetRect().width - 5, 4);
   mSearchToggleButton->Draw();
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

   if (list == mPaletteDropdown)
   {
      ApplyColorPalette(mPaletteIndex);
      return;
   }

   for (auto* spawnList : mSpawnLists.GetDropdowns())
      spawnList->OnSelection(list);
}

void TitleBar::ApplyColorPalette(int index)
{
   //Each palette now themes the WHOLE app: the 7 module-category hues + module saturation/
   //brightness (as before), PLUS the window background and the animated lissajous scope, which
   //are derived from a per-palette background hue (bgHue/bgSat/bgBright) and lissajous brightness
   //(lissBright). Background/lissajous are set both on the live statics (so it changes instantly)
   //and on UserPrefs (so it persists).
   //
   //   module hues .......... hueNote, hueSynth, hueAudio, hueInstrument, hueProcessor, hueModulator, huePulse
   //   module look .......... saturation, brightness
   //   window theming ....... bgHue, bgSat, bgBright (dark tint), lissBright (scope glow)
   //all values are in the 0-255 range that ofColor::setHsb expects.
   struct Palette
   {
      float hueNote, hueSynth, hueAudio, hueInstrument, hueProcessor, hueModulator, huePulse, saturation, brightness;
      float bgHue, bgSat, bgBright, lissBright;
   };

   static const Palette kPalettes[] = {
      { 27, 79, 135, 240, 170, 200, 43, 130, 205, 160, 30, 23, 80 }, //classic
      { 330, 140, 190, 260, 30, 90, 55, 230, 235, 190, 120, 26, 150 }, //neon
      { 27, 79, 135, 240, 170, 200, 43, 70, 235, 190, 40, 30, 95 }, //pastel
      { 0, 0, 0, 0, 0, 0, 0, 0, 200, 0, 0, 20, 80 }, //mono
      { 15, 350, 30, 340, 10, 45, 25, 190, 220, 12, 130, 26, 100 }, //sunset - warm maroon bg
      { 195, 170, 220, 190, 205, 180, 160, 170, 210, 140, 130, 24, 95 }, //ocean - deep teal bg
      { 90, 310, 55, 170, 20, 280, 130, 255, 255, 60, 120, 22, 120 }, //acid - toxic lime/magenta/yellow
      { 320, 185, 280, 340, 200, 260, 170, 200, 235, 205, 110, 28, 120 }, //vaporwave - pink/cyan/purple
      { 75, 110, 45, 150, 90, 130, 60, 245, 245, 60, 140, 20, 110 }, //toxic - radioactive green
      { 5, 15, 350, 25, 10, 340, 0, 210, 160, 2, 160, 20, 80 }, //bloodmoon - deep dim reds
      { 195, 320, 50, 280, 210, 300, 45, 235, 230, 185, 140, 24, 130 }, //cyberpunk - blue/magenta/yellow
      { 10, 200, 95, 310, 55, 250, 150, 255, 200, 150, 60, 22, 110 }, //glitch - wide clashing hues
      { 170, 190, 150, 200, 160, 210, 175, 150, 220, 170, 150, 20, 85 }, //midnight - deep navy, cool blues
      { 130, 90, 160, 200, 110, 260, 140, 190, 235, 130, 120, 24, 110 }, //aurora - teal/green/purple glow
      { 75, 60, 100, 40, 90, 130, 55, 160, 210, 75, 90, 22, 85 }, //forest - greens/browns
      { 235, 130, 45, 210, 160, 280, 50, 220, 245, 220, 90, 30, 120 }, //candy - pink/cyan/yellow on plum
      { 8, 20, 350, 30, 15, 40, 25, 200, 225, 15, 150, 22, 100 }, //ember - near-black warm reds/gold
      { 150, 170, 130, 190, 140, 200, 160, 90, 240, 150, 50, 30, 95 }, //arctic - icy blue-grey
      { 200, 215, 180, 230, 195, 260, 210, 180, 225, 200, 130, 24, 100 }, //grape - deep purple
      { 28, 35, 20, 40, 30, 45, 25, 120, 210, 28, 80, 28, 85 }, //sepia - warm brown vintage
      { 90, 95, 85, 100, 90, 110, 80, 220, 235, 90, 180, 16, 130 }, //matrix - black-green terminal
      { 215, 130, 90, 265, 175, 280, 60, 170, 235, 175, 40, 34, 100 }, //dracula - classic dark editor
      { 170, 130, 90, 40, 20, 215, 60, 150, 220, 130, 120, 30, 90 }, //solarized - solarized-dark base
      { 0, 0, 0, 0, 0, 0, 0, 0, 90, 0, 0, 225, 160 } //mono light - dark modules on a light bg
   };

   if (index < 0 || index >= (int)(sizeof(kPalettes) / sizeof(kPalettes[0])))
      return;

   const Palette& p = kPalettes[index];
   UserPrefs.hue_note.Get() = p.hueNote;
   UserPrefs.hue_synth.Get() = p.hueSynth;
   UserPrefs.hue_audio.Get() = p.hueAudio;
   UserPrefs.hue_instrument.Get() = p.hueInstrument;
   UserPrefs.hue_processor.Get() = p.hueProcessor;
   UserPrefs.hue_modulator.Get() = p.hueModulator;
   UserPrefs.hue_pulse.Get() = p.huePulse;
   UserPrefs.module_saturation.Get() = p.saturation;
   UserPrefs.module_brightness.Get() = p.brightness;

   //derive the window background (a dark tint) and the lissajous scope colour from the palette's
   //background hue, then push them to both the live render statics and UserPrefs
   ofColor bg;
   bg.setHsb((int)p.bgHue, (int)p.bgSat, (int)p.bgBright);
   ofColor liss;
   liss.setHsb((int)p.bgHue, (int)(p.bgSat * 0.85f), (int)p.lissBright);

   ModularSynth::sBackgroundR = bg.r / 255.0f;
   ModularSynth::sBackgroundG = bg.g / 255.0f;
   ModularSynth::sBackgroundB = bg.b / 255.0f;
   UserPrefs.background_r.Get() = ModularSynth::sBackgroundR;
   UserPrefs.background_g.Get() = ModularSynth::sBackgroundG;
   UserPrefs.background_b.Get() = ModularSynth::sBackgroundB;

   ModularSynth::sBackgroundLissajousR = liss.r / 255.0f;
   ModularSynth::sBackgroundLissajousG = liss.g / 255.0f;
   ModularSynth::sBackgroundLissajousB = liss.b / 255.0f;
   UserPrefs.lissajous_r.Get() = ModularSynth::sBackgroundLissajousR;
   UserPrefs.lissajous_g.Get() = ModularSynth::sBackgroundLissajousG;
   UserPrefs.lissajous_b.Get() = ModularSynth::sBackgroundLissajousB;

   DisplayTemporaryMessage("palette applied: " + mPaletteDropdown->GetLabel(index));
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
   if (button == mRecordButton)
      TheSynth->ToggleRecording();
   if (button == mDisplayHelpButton)
      ShowHelp();
   if (button == mDisplayUserPrefsEditorButton)
      TheSynth->GetUserPrefsEditor()->Show();
   if (button == mHomeButton)
      TheSynth->GetLocationZoomer()->GoHome();
   if (button == mSearchToggleButton && TheSearchPanel != nullptr)
      TheSearchPanel->SetShowing(!TheSearchPanel->IsShowing()); //Bitwig-style: open/close the browser panel
   if (button == mResetLayoutButton)
   {
      auto buttonRect = mResetLayoutButton->GetRect();
      mNewPatchConfirmPopup.SetOwningContainer(GetOwningContainer());
      mNewPatchConfirmPopup.SetPosition(buttonRect.x, buttonRect.y + buttonRect.height + 2);
      TheSynth->PushModalFocusItem(&mNewPatchConfirmPopup);
   }
   if (button == mWelcomeScreenButton)
      TheSynth->GetWelcomeScreen()->Show();
   if (button == mPlayPauseButton)
      TheSynth->SetAudioPaused(!TheSynth->IsAudioPaused());
}

void TitleBar::ShowHelp()
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
