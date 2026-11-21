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
#include "ModuleFactory.h"
#include "PatchCable.h"
#include "PatchCableSource.h"
#include "PrefabPort.h"
#include "UserPrefs.h"

#include "juce_gui_basics/juce_gui_basics.h"

#include <algorithm>

//static
bool Prefab::sLoadingPrefab = false;
bool Prefab::sLastLoadWasPrefab = false;
IDrawableModule* Prefab::sJustReleasedModule = nullptr;

namespace
{
   const float paddingX = 5;
   const float paddingY = 5;
   const float kEditTopMargin = 50; //room for the two rows of header controls
   const float kPresentationTopMargin = 20; //presentation mode only shows the mode dropdown
   const float kLockedTopMargin = paddingY; //locked mode shows no header controls
   const float kEditMinWidth = 175;
   const float kPresentationMinWidth = 120;
   const float kPortSpacing = 4;

   const int kNumPortChoices = 8;
   const char* kPortChoiceLabels[kNumPortChoices] = { "audio in", "audio out", "note in", "note out", "pulse in", "pulse out", "mod in", "mod out" };
}

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

   mDisplayModeDropdown = new DropdownList(this, "mode", 3, 2, (int*)&mDisplayMode, 85);
   mDisplayModeDropdown->AddLabel("edit", (int)PrefabDisplayMode::Edit);
   mDisplayModeDropdown->AddLabel("presentation", (int)PrefabDisplayMode::Presentation);
   mAddPortDropdown = new DropdownList(this, "addport", mDisplayModeDropdown, kAnchor_Right, &mAddPortIndex, 70);
   mSaveButton = new ClickButton(this, "save", 3, 20);
   mLoadButton = new ClickButton(this, "load", mSaveButton, kAnchor_Right);
   mDisbandButton = new ClickButton(this, "disband", mLoadButton, kAnchor_Right);
   for (int i = 0; i < kNumPortChoices; ++i)
      mAddPortDropdown->AddLabel(kPortChoiceLabels[i], i);
   mAddPortDropdown->SetUnknownItemString("+ in/out");
}

std::string Prefab::GetTitleLabel() const
{
   if (mPrefabName != "")
   {
      if (mPresentationApplied)
         return mPrefabName;
      return "prefab: " + mPrefabName;
   }
   return "prefab";
}

void Prefab::Poll()
{
   PruneInterfaceModules();

   bool wantPresentation = GetDisplayMode() == PrefabDisplayMode::Presentation;
   if (wantPresentation != mPresentationApplied)
      SetPresentationApplied(wantPresentation);
   UpdateHeaderControls();

   float topMargin;
   if (mLocked)
      topMargin = kLockedTopMargin;
   else if (mDisplayMode == PrefabDisplayMode::Presentation)
      topMargin = kPresentationTopMargin;
   else
      topMargin = kEditTopMargin;

   if (mPresentationApplied)
   {
      LayOutPresentation(topMargin);
      return;
   }

   float xMin, yMin;
   GetPosition(xMin, yMin);
   for (auto* module : mModuleContainer.GetModules())
   {
      xMin = MIN(xMin, module->GetPosition().x - paddingX);
      yMin = MIN(yMin, module->GetPosition().y - topMargin);
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
   if (mLocked || mPresentationApplied)
      return false;

   if (IsMouseHovered() && !TheSynth->IsGroupSelecting())
   {
      if (IsAddableModule(TheSynth->GetMoveModule()))
         return true;
      if (IsAddableModule(sJustReleasedModule))
         return true;
      if (!TheSynth->GetGroupSelectedModules().empty())
      {
         for (auto* module : TheSynth->GetGroupSelectedModules())
         {
            if (module == this || module->IsSingleton())
               return false;
            if (IsAddableModule(module))
               return true;
         }
      }
   }
   return false;
}

bool Prefab::IsAddableModule(IDrawableModule* module)
{
   if (module == nullptr || module == this || VectorContains(module, mModuleContainer.GetModules()))
      return false;
   if (module->IsSingleton())
      return false;
   if (dynamic_cast<Prefab*>(module) != nullptr)
      return false;
   if (dynamic_cast<Prefab*>(module->GetParent()) != nullptr)
      return false;
   return true;
}

void Prefab::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   if (y > 0 && !right && !mPresentationApplied && !mLocked)
      TheSynth->SetGroupSelectContext(&mModuleContainer);
}

void Prefab::AddModule(IDrawableModule* module)
{
   if (IsAddableModule(module))
      mModuleContainer.TakeModule(module);
}

void Prefab::MouseReleased()
{
   IDrawableModule::MouseReleased();

   if (CanAddDropModules() && !VectorContains<IDrawableModule*>(this, TheSynth->GetGroupSelectedModules()))
   {
      AddModule(sJustReleasedModule);

      for (auto* module : TheSynth->GetGroupSelectedModules())
         AddModule(module);
   }
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
      ofRect(0, 0, GetRect().width, GetRect().height);
      ofPopStyle();
   }

   //hidden controls skip drawing, see UpdateHeaderControls()
   mDisplayModeDropdown->Draw();
   mAddPortDropdown->Draw();
   mSaveButton->Draw();
   mLoadButton->Draw();
   mDisbandButton->Draw();

   mModuleContainer.DrawModules();
}

void Prefab::DrawModuleUnclipped()
{
   mModuleContainer.DrawUnclipped();

   if (!mPresentationApplied)
   {
      //outline the modules that make up the presentation-mode interface
      ofPushStyle();
      ofNoFill();
      ofSetLineWidth(1.5f);
      ofSetColor(255, 220, 90, 140);
      for (auto* module : GetInterfaceModules())
      {
         ofRectangle rect = module->GetRect(true);
         float titleBar = module->HasTitleBar() ? TitleBarHeight() : 0;
         ofRect(rect.x - 3, rect.y - titleBar - 3, rect.width + 6, rect.height + titleBar + 6, 4);
      }
      ofPopStyle();
   }
}

//static
Prefab* Prefab::GetOwningPrefab(const IDrawableModule* module)
{
   if (module == nullptr || module->GetOwningContainer() == nullptr)
      return nullptr;
   return dynamic_cast<Prefab*>(module->GetOwningContainer()->GetOwner());
}

bool Prefab::IsInterfaceModule(const IDrawableModule* module) const
{
   return module != nullptr && VectorContains(const_cast<IDrawableModule*>(module), mInterfaceModules);
}

void Prefab::SetInterfaceModule(IDrawableModule* module, bool isInterface)
{
   if (module == nullptr || GetOwningPrefab(module) != this || dynamic_cast<PrefabPort*>(module) != nullptr)
      return;
   if (isInterface == IsInterfaceModule(module))
      return;

   //step out of presentation mode while changing membership, so positions and visibility get stored/restored properly
   bool wasApplied = mPresentationApplied;
   if (wasApplied)
      SetPresentationApplied(false);

   if (isInterface)
      mInterfaceModules.push_back(module);
   else
      RemoveFromVector(module, mInterfaceModules);

   if (wasApplied)
      SetPresentationApplied(true);
}

bool Prefab::CanRemoveModule(const IDrawableModule* module) const
{
   return GetOwningPrefab(module) == this && dynamic_cast<const PrefabPort*>(module) == nullptr && GetOwningContainer() != nullptr;
}

void Prefab::RemoveModule(IDrawableModule* module)
{
   if (!CanRemoveModule(module))
      return;

   bool wasApplied = mPresentationApplied;
   if (wasApplied)
      SetPresentationApplied(false);

   RemoveFromVector(module, mInterfaceModules);
   mPresentationPositions.erase(module->Name());
   GetOwningContainer()->TakeModule(module);
   GetOwningContainer()->MoveToFront(this);

   if (wasApplied)
      SetPresentationApplied(true);
}

std::vector<IDrawableModule*> Prefab::GetInterfaceModules() const
{
   //in container order, so the presentation layout is stable
   std::vector<IDrawableModule*> ret;
   for (auto* module : mModuleContainer.GetModules())
   {
      if (IsInterfaceModule(module))
         ret.push_back(module);
   }
   return ret;
}

//static
bool Prefab::IsHiddenByPresentation(const IDrawableModule* module)
{
   if (module == nullptr || module->IsShowing())
      return false;
   ModuleContainer* container = module->GetOwningContainer();
   if (container == nullptr)
      return false;
   Prefab* prefab = dynamic_cast<Prefab*>(container->GetOwner());
   return prefab != nullptr && prefab->mPresentationApplied;
}

//static
bool Prefab::SuppressesCableSourcesOf(const IDrawableModule* module)
{
   //in presentation mode only the ports are patchable, everything else inside acts like its cable sources aren't showing
   if (module == nullptr)
      return false;
   ModuleContainer* container = module->GetOwningContainer();
   if (container == nullptr)
      return false;
   Prefab* prefab = dynamic_cast<Prefab*>(container->GetOwner());
   return prefab != nullptr && prefab->mPresentationApplied && dynamic_cast<const PrefabPort*>(module) == nullptr;
}

void Prefab::PruneInterfaceModules()
{
   //drop anything that's been deleted or has left the prefab some other way
   const auto& members = mModuleContainer.GetModules();
   for (auto iter = mInterfaceModules.begin(); iter != mInterfaceModules.end();)
   {
      if (!VectorContains(*iter, members) || (*iter)->IsDeleted())
         iter = mInterfaceModules.erase(iter);
      else
         ++iter;
   }
}

float Prefab::GetWidestPortWidth(bool inlets) const
{
   float widest = 0;
   for (auto* module : mModuleContainer.GetModules())
   {
      PrefabPort* port = dynamic_cast<PrefabPort*>(module);
      if (port != nullptr && port->IsInlet() == inlets)
      {
         float w, h;
         port->GetDimensions(w, h);
         widest = MAX(widest, w);
      }
   }
   return widest;
}

float Prefab::GetPresentationLeftMargin() const
{
   float widestInlet = GetWidestPortWidth(true);
   if (widestInlet > 0)
      return paddingX + widestInlet + paddingX;
   return paddingX;
}

void Prefab::UpdateHeaderControls()
{
   mDisplayModeDropdown->SetShowing(!mLocked);

   bool editing = GetDisplayMode() == PrefabDisplayMode::Edit && !mLocked;
   mSaveButton->SetShowing(editing);
   mLoadButton->SetShowing(editing);
   mDisbandButton->SetShowing(editing);
   mAddPortDropdown->SetShowing(editing);
}

void Prefab::SetPresentationApplied(bool apply)
{
   if (apply == mPresentationApplied)
      return;

   const auto& modules = mModuleContainer.GetModules();

   if (apply)
   {
      mEditPositions.clear();
      for (auto* module : modules)
         mEditPositions[module->Name()] = module->GetPosition(true);

      //modules that don't have a remembered presentation position keep their relative layout, packed into the corner
      std::vector<IDrawableModule*> interfaceModules = GetInterfaceModules();
      float minX = FLT_MAX;
      float minY = FLT_MAX;
      for (auto* module : interfaceModules)
      {
         ofVec2f pos = module->GetPosition(true);
         minX = MIN(minX, pos.x);
         minY = MIN(minY, pos.y - (module->HasTitleBar() ? TitleBarHeight() : 0));
      }
      ofVec2f defaultOffset(GetPresentationLeftMargin() - minX, (mLocked ? kLockedTopMargin : kPresentationTopMargin) - minY);

      for (auto* module : modules)
      {
         PrefabPort* port = dynamic_cast<PrefabPort*>(module);
         if (port != nullptr)
         {
            port->SetCompact(true);
            port->SetShowing(true);
         }
         else if (VectorContains(module, interfaceModules))
         {
            module->SetShowing(true);
            auto stored = mPresentationPositions.find(module->Name());
            if (stored != mPresentationPositions.end())
               module->SetPosition(stored->second.x, stored->second.y);
            else
               module->SetPosition(module->GetPosition(true).x + defaultOffset.x, module->GetPosition(true).y + defaultOffset.y);
         }
         else
         {
            module->SetShowing(false);
         }
      }

      mPresentationApplied = true;
      LayOutPresentation(mLocked ? kLockedTopMargin : kPresentationTopMargin);
   }
   else
   {
      for (auto* module : GetInterfaceModules())
         mPresentationPositions[module->Name()] = module->GetPosition(true);

      for (auto* module : modules)
      {
         module->SetShowing(true);
         PrefabPort* port = dynamic_cast<PrefabPort*>(module);
         if (port != nullptr)
            port->SetCompact(false);
         auto edit = mEditPositions.find(module->Name());
         if (edit != mEditPositions.end())
            module->SetPosition(edit->second.x, edit->second.y);
      }

      mEditPositions.clear();
      mPresentationApplied = false;
   }

   UpdateHeaderControls();
}

void Prefab::LayOutPresentation(float topMargin)
{
   const auto& modules = mModuleContainer.GetModules();
   std::vector<IDrawableModule*> interfaceModules = GetInterfaceModules();
   float left = GetPresentationLeftMargin();

   float contentRight = left;
   float contentBottom = topMargin;
   if (!interfaceModules.empty())
   {
      //hug the interface: keep it tucked against the margins, moving ourselves to compensate so nothing jumps on screen
      float minX = FLT_MAX;
      float minY = FLT_MAX;
      for (auto* module : interfaceModules)
      {
         ofVec2f pos = module->GetPosition(true);
         minX = MIN(minX, pos.x);
         minY = MIN(minY, pos.y - (module->HasTitleBar() ? TitleBarHeight() : 0));
      }
      float dx = left - minX;
      float dy = topMargin - minY;
      if (fabsf(dx) >= 1 || fabsf(dy) >= 1)
      {
         for (auto* module : modules)
            module->SetPosition(module->GetPosition(true).x + dx, module->GetPosition(true).y + dy);
         for (auto& edit : mEditPositions)
            edit.second += ofVec2f(dx, dy);
         SetPosition(GetPosition(true).x - dx, GetPosition(true).y - dy);
      }

      for (auto* module : interfaceModules)
      {
         ofRectangle rect = module->GetRect(true);
         contentRight = MAX(contentRight, rect.x + rect.width);
         contentBottom = MAX(contentBottom, rect.y + rect.height);
      }
   }

   //inlets go down the left edge and outlets down the right, ordered by where they sit in the edit layout
   std::vector<PrefabPort*> inlets;
   std::vector<PrefabPort*> outlets;
   for (auto* module : modules)
   {
      PrefabPort* port = dynamic_cast<PrefabPort*>(module);
      if (port != nullptr)
         (port->IsInlet() ? inlets : outlets).push_back(port);
   }
   auto editOrder = [this](PrefabPort* a, PrefabPort* b)
   {
      ofVec2f posA = mEditPositions.count(a->Name()) ? mEditPositions[a->Name()] : a->GetPosition(true);
      ofVec2f posB = mEditPositions.count(b->Name()) ? mEditPositions[b->Name()] : b->GetPosition(true);
      if (posA.y != posB.y)
         return posA.y < posB.y;
      return posA.x < posB.x;
   };
   std::sort(inlets.begin(), inlets.end(), editOrder);
   std::sort(outlets.begin(), outlets.end(), editOrder);

   //each side's margin is sized to fit the widest port on that side
   float widestOutlet = GetWidestPortWidth(false);
   float width = contentRight + paddingX;
   if (!outlets.empty())
      width += widestOutlet + paddingX;
   width = MAX(width, kPresentationMinWidth);

   //inlets are left-justified against the left edge, outlets right-justified against the right edge
   float height = contentBottom + paddingY;
   auto stackPorts = [&height](const std::vector<PrefabPort*>& ports, float edgeX, bool rightJustify, float topMargin)
   {
      float y = topMargin + 2;
      for (auto* port : ports)
      {
         float w, h;
         port->GetDimensions(w, h);
         port->SetPosition(rightJustify ? edgeX - w : edgeX, y);
         y += h + kPortSpacing;
      }
      height = MAX(height, y - kPortSpacing + paddingY);
   };
   stackPorts(inlets, paddingX, false, topMargin);
   stackPorts(outlets, width - paddingX, true, topMargin);

   mPresentationWidth = width;
   mPresentationHeight = MAX(height, 40);
}

void Prefab::GetModuleDimensions(float& width, float& height)
{
   if (mPresentationApplied)
   {
      width = mPresentationWidth;
      height = mPresentationHeight;
      return;
   }

   float x, y;
   GetPosition(x, y);
   width = kEditMinWidth;
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

void Prefab::SpawnPort(int index)
{
   if (index < 0 || index >= kNumPortChoices)
      return;

   PrefabPortType type = (PrefabPortType)(index / 2);
   bool isInlet = (index % 2) == 0;

   std::string name = GetUniqueName(PrefabPort::GetDefaultName(type, isInlet), mModuleContainer.GetModules());

   //drop it in below the current contents, inlets on the left and outlets on the right
   float w, h;
   GetDimensions(w, h);
   ofVec2f pos = GetPosition();
   float x = pos.x + (isInlet ? paddingX : MAX(paddingX, w - paddingX - PrefabPort::kCompactWidthPadding));
   float y = pos.y + h + TitleBarHeight() + 5;

   ModuleFactory::Spawnable spawnable;
   spawnable.mLabel = PrefabPort::GetTypeName(type, isInlet);
   IDrawableModule* module = TheSynth->SpawnModuleOnTheFly(spawnable, x, y, true, name);
   if (module == nullptr)
      return;

   mModuleContainer.TakeModule(module);

   //the spawn made the name unique against the root container, make sure it's unique in here instead
   std::vector<IDrawableModule*> others = mModuleContainer.GetModules();
   RemoveFromVector(module, others);
   module->SetName(GetUniqueName(name, others).c_str());
}

void Prefab::BridgePortConnections()
{
   //when disbanding, splice whatever was patched into an audio/note/pulse port straight through to the port's targets.
   //multiple passes handle ports patched directly into other ports.
   for (int pass = 0; pass < 4; ++pass)
   {
      bool changed = false;
      std::vector<IDrawableModule*> allModules;
      TheSynth->GetAllModules(allModules);

      for (auto* module : mModuleContainer.GetModules())
      {
         PrefabPort* port = dynamic_cast<PrefabPort*>(module);
         if (port == nullptr || port->GetPortType() == PrefabPortType::Modulation)
            continue;
         PatchCableSource* portOutput = port->GetPatchCableSource();
         if (portOutput == nullptr)
            continue;

         std::vector<IClickable*> downstream;
         for (auto* cable : portOutput->GetPatchCables())
         {
            if (cable != nullptr && cable->GetTarget() != nullptr && cable->GetTarget() != port)
               downstream.push_back(cable->GetTarget());
         }
         if (downstream.empty())
            continue;

         for (auto* other : allModules)
         {
            if (other == port || other->IsDeleted())
               continue;
            for (auto* source : other->GetPatchCableSources())
            {
               std::vector<PatchCable*> cables = source->GetPatchCables();
               for (auto* cable : cables)
               {
                  if (cable == nullptr || cable->GetTarget() != port)
                     continue;
                  source->SetPatchCableTarget(cable, downstream[0], false);
                  for (size_t i = 1; i < downstream.size(); ++i)
                     source->AddPatchCable(downstream[i]);
                  changed = true;
               }
            }
         }
      }

      if (!changed)
         break;
   }
}

void Prefab::Disband()
{
   mDisplayMode = PrefabDisplayMode::Edit;
   if (mPresentationApplied)
      SetPresentationApplied(false);

   {
      ScopedMutex mutex(TheSynth->GetAudioMutex(), "Prefab::Disband()");
      BridgePortConnections();
   }

   auto modules = mModuleContainer.GetModules();
   for (auto* module : modules)
   {
      if (dynamic_cast<PrefabPort*>(module) != nullptr)
         mModuleContainer.DeleteModule(module); //ports only make sense inside a prefab
      else
         GetOwningContainer()->TakeModule(module);
   }
   GetOwningContainer()->DeleteModule(this);
}

void Prefab::OpenSaveDialog()
{
   auto onChosen = [this](const juce::FileChooser& chooser)
   {
      SavePrefab(SaveTarget(chooser));
   };
   BrowseForFileToSave("Save prefab as...", juce::File(ofToDataPath("prefabs/prefab.pfb")), "*.pfb", onChosen);
}

void Prefab::ButtonClicked(ClickButton* button, double time)
{
   using namespace juce;
   if (button == mSaveButton)
      OpenSaveDialog();

   if (button == mLoadButton)
   {
      auto onChosen = [this](const juce::FileChooser& chooser)
      {
         std::string loadPath = GetChosenFileForReading(chooser).getFullPathName().toStdString();
         LoadPrefab(ofToDataPath(loadPath));
      };
      BrowseForFileToOpen("Load prefab...", File(ofToDataPath("prefabs")), "*.pfb", onChosen);
   }

   if (button == mDisbandButton)
      Disband();
}

void Prefab::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
   {
      for (auto& module : mModuleContainer.GetModules())
      {
         module->SetEnabled(mEnabled);
      }
   }
}

void Prefab::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   //mDisplayModeDropdown is applied in Poll(), so that loading state gets the same treatment as picking a mode
   if (list == mAddPortDropdown)
   {
      SpawnPort(mAddPortIndex);
      mAddPortIndex = -1;
   }
}

ofxJSONElement Prefab::WriteModulesWithEditPositions()
{
   ofxJSONElement modules = mModuleContainer.WriteModules();

   //always save the edit layout, presentation positions are stored separately
   if (mPresentationApplied)
   {
      for (unsigned int i = 0; i < modules.size(); ++i)
      {
         auto edit = mEditPositions.find(modules[i]["name"].asString());
         if (edit != mEditPositions.end())
         {
            modules[i]["position"][0u] = edit->second.x;
            modules[i]["position"][1u] = edit->second.y;
         }
      }
   }

   return modules;
}

void Prefab::SavePresentationSettings(ofxJSONElement& info)
{
   std::map<std::string, ofVec2f> positions = mPresentationPositions;
   if (mPresentationApplied)
   {
      for (auto* module : GetInterfaceModules())
         positions[module->Name()] = module->GetPosition(true);
   }

   ofxJSONElement positionsJson(Json::objectValue);
   for (const auto& entry : positions)
   {
      if (mModuleContainer.FindModule(entry.first, false) == nullptr)
         continue;
      positionsJson[entry.first][0u] = entry.second.x;
      positionsJson[entry.first][1u] = entry.second.y;
   }
   info["presentation_positions"] = positionsJson;

   ofxJSONElement interfaceNames(Json::arrayValue);
   for (auto* module : GetInterfaceModules())
      interfaceNames.append(module->Name());
   info["interface"] = interfaceNames;
}

void Prefab::LoadPresentationSettings(const ofxJSONElement& info)
{
   mPresentationPositions.clear();
   if (info.isMember("presentation_positions"))
   {
      const Json::Value& positions = info["presentation_positions"];
      for (const auto& name : positions.getMemberNames())
         mPresentationPositions[name] = ofVec2f(positions[name][0u].asFloat(), positions[name][1u].asFloat());
   }

   mInterfaceModules.clear();
   if (info.isMember("interface"))
      AddInterfaceModulesByName(info["interface"]);
}

void Prefab::AddInterfaceModulesByName(const ofxJSONElement& names)
{
   for (unsigned int i = 0; i < names.size(); ++i)
   {
      IDrawableModule* module = mModuleContainer.FindModule(names[i].asString(), false);
      if (module != nullptr && VectorContains(module, mModuleContainer.GetModules()) && dynamic_cast<PrefabPort*>(module) == nullptr && !IsInterfaceModule(module))
         mInterfaceModules.push_back(module);
   }
}

void Prefab::SavePrefab(const SaveTarget& target)
{
   ofxJSONElement root;

   root["modules"] = WriteModulesWithEditPositions();

   //the prefab module itself isn't part of a .pfb, so carry its presentation setup along explicitly
   SavePresentationSettings(root);
   root["display_mode"] = (int)mDisplayMode;

   std::stringstream ss(root.getRawString(true));
   std::string line;
   std::string lines;
   while (getline(ss, line, '\n'))
   {
      const char* pos = strstr(line.c_str(), " : \"$");
      if (pos != nullptr)
      {
         bool endsWithComma = line[line.length() - 1] == ',';
         ofStringReplace(line, pos, " : \"\"");
         if (endsWithComma)
            line += ",";
      }
      lines += line + '\n';
   }

   UpdatePrefabName(target.GetPath());

   {
      FileStreamOut out(ofToDataPath(target.GetWritePath()));

      out << lines;
      mModuleContainer.SaveState(out);
   }

   target.Commit();
}

void Prefab::LoadPrefab(std::string loadPath)
{
   sLoadingPrefab = true;

   ScopedMutex mutex(TheSynth->GetAudioMutex(), "LoadPrefab()");
   std::lock_guard<std::recursive_mutex> renderLock(TheSynth->GetRenderLock());

   mDisplayMode = PrefabDisplayMode::Edit;
   if (mPresentationApplied)
      SetPresentationApplied(false);
   mInterfaceModules.clear();

   mModuleContainer.Clear();

   FileStreamIn in(ofToDataPath(loadPath));

   assert(in.OpenedOk());

   std::string jsonString;
   in >> jsonString;
   ofxJSONElement root;
   bool loaded = root.parse(jsonString);

   if (!loaded)
   {
      TheSynth->LogEvent("Couldn't load, error parsing " + loadPath, kLogEventType_Error);
      TheSynth->LogEvent("Try loading it up in a json validator", kLogEventType_Error);
      sLoadingPrefab = false;
      return;
   }

   UpdatePrefabName(loadPath);

   mModuleContainer.LoadModules(root["modules"]);

   mModuleContainer.LoadState(in);

   LoadPresentationSettings(root);
   //applied on the next Poll()
   if (root.isMember("display_mode"))
   {
      mDisplayMode = PrefabDisplayMode(root["display_mode"].asInt());
   }

   mLocked = mDisplayMode == PrefabDisplayMode::Presentation;
   mModuleSaveData.SetBool("lock_prefab", mLocked);

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
   moduleInfo["modules"] = WriteModulesWithEditPositions();
   SavePresentationSettings(moduleInfo);
}

void Prefab::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadBool("lock_prefab", moduleInfo, false);
   mModuleContainer.LoadModules(moduleInfo["modules"]);
   LoadPresentationSettings(moduleInfo);
   SetUpFromSaveData();
}

void Prefab::SetUpFromSaveData()
{
   mLocked = mModuleSaveData.GetBool("lock_prefab");
}

void Prefab::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << mPrefabName;
}

void Prefab::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (!ModuleContainer::DoesModuleHaveMoreSaveData(in))
      return; //this was saved before we added versioning, bail out

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   in >> mPrefabName;
}
