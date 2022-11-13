//
// Created by Ryan Challinor on 11/05/22.
//

#include "SongBuilder.h"
#include "SynthGlobals.h"
#include "UIControlMacros.h"
#include "FileStream.h"
#include "PatchCableSource.h"

namespace
{
   const float kGridStartX = 3;
   const float kGridStartY = 20;
   const float kSectionTabWidth = 160;
   const float kTargetTabHeightTop = 30;
   const float kTargetTabHeightBottom = 10;
   const float kRowHeight = 20;
   const float kColumnWidth = 50;
   const float kSpacingX = 3;
   const float kSpacingY = 3;
}

SongBuilder::SongBuilder()
{
}

void SongBuilder::Init()
{
   IDrawableModule::Init();

   TheTransport->AddListener(this, mInterval, OffsetInfo(0, true), true);

   SetActiveSection(gTime, 0);
}

void SongBuilder::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   DROPDOWN(mIntervalSelector, "interval", (int*)(&mInterval), 40);
   ENDUIBLOCK0();

   mAddTargetButton = new ClickButton(this, "add target", kGridStartX + kSectionTabWidth - 22, kGridStartY + 8, ButtonDisplayStyle::kPlus);

   mIntervalSelector->AddLabel("4", kInterval_4);
   mIntervalSelector->AddLabel("3", kInterval_3);
   mIntervalSelector->AddLabel("2", kInterval_2);
   mIntervalSelector->AddLabel("1n", kInterval_1n);
   mIntervalSelector->SetShowing(false);

   mSections.push_back(new SongSection("intro"));
   mSections.push_back(new SongSection("verse"));
   mSections.push_back(new SongSection("chorus"));
   mSections.push_back(new SongSection("bridge"));
   mSections.push_back(new SongSection("outro"));
   mSections.push_back(new SongSection("done"));
   for (auto* section : mSections)
      section->CreateUIControls(this);
}

SongBuilder::~SongBuilder()
{
   TheTransport->RemoveListener(this);
}

void SongBuilder::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mIntervalSelector->Draw();
   mAddTargetButton->Draw();

   for (int i = 0; i < (int)mSections.size(); ++i)
      mSections[i]->Draw(kGridStartX, kGridStartY + kTargetTabHeightTop + kSpacingY + i * (kRowHeight + kSpacingY), i, mCurrentSection == i);

   for (int i = 0; i < (int)mTargets.size(); ++i)
      mTargets[i]->Draw(kGridStartX + kSectionTabWidth + i * (kColumnWidth + kSpacingX), kGridStartY, (int)mSections.size());

   ofPushStyle();

   ofPopStyle();
}

void SongBuilder::OnTimeEvent(double time)
{
   if (mHasExternalPulseSource)
      return;

   OnStep(time, 1, 0);
}

void SongBuilder::OnPulse(double time, float velocity, int flags)
{
   mHasExternalPulseSource = true;
   OnStep(time, velocity, flags);
}

void SongBuilder::OnStep(double time, float velocity, int flags)
{
   /*if (flags & kPulseFlag_Reset)
      mStepIdx = -1;

   if (mEnabled)
   {
      mStepIdx = (mStepIdx + 1);
   }*/
}

void SongBuilder::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (velocity > 0 && pitch < (int)mSections.size())
      SetActiveSection(time, pitch);
}

void SongBuilder::SetActiveSection(double time, int newSection)
{
   if (newSection < (int)mSections.size())
   {
      for (int i = 0; i < (int)mTargets.size(); ++i)
      {
         for (auto& cable : mTargets[i]->mCable->GetPatchCables())
         {
            IUIControl* target = dynamic_cast<IUIControl*>(cable->GetTarget());
            if (target != nullptr)
            {
               if (mTargets[i]->mIsCheckbox)
                  target->SetValue(mSections[newSection]->mValues[i]->mBoolValue ? 1 : 0, time);
               else
                  target->SetValue(mSections[newSection]->mValues[i]->mValue, time);
            }
         }
      }
   }

   mCurrentSection = newSection;
}

void SongBuilder::GetModuleDimensions(float& width, float& height)
{
   width = kGridStartX + kSectionTabWidth + (int)mTargets.size() * (kColumnWidth + kSpacingX) + 3;
   height = kGridStartY + kTargetTabHeightTop + kTargetTabHeightBottom + kSpacingY + (int)mSections.size() * (kRowHeight + kSpacingY) + 3;
}

void SongBuilder::PostRepatch(PatchCableSource* cable, bool fromUserClick)
{
   int targetIndex = -1;
   for (int i = 0; i < (int)mTargets.size(); ++i)
   {
      if (cable == mTargets[i]->mCable)
      {
         mTargets[i]->TargetControlUpdated();
         targetIndex = i;
         break;
      }
   }

   if (fromUserClick && targetIndex != -1)
   {
      for (int i = 0; i < (int)mSections.size(); ++i)
         mSections[i]->TargetControlUpdated(mTargets[targetIndex], targetIndex, true);

      if (mTargets[targetIndex]->GetTarget() == nullptr)
      {
         mTargets[targetIndex]->CleanUp();
         mTargets.erase(mTargets.begin() + targetIndex);
      }
   }
}

void SongBuilder::ButtonClicked(ClickButton* button, double time)
{
   for (int i = 0; i < (int)mSections.size(); ++i)
   {
      if (button == mSections[i]->mActivateButton)
         SetActiveSection(time, i);
   }

   for (int i = 0; i < (int)mTargets.size(); ++i)
   {
      if (button == mTargets[i]->mMoveLeftButton)
      {
         if (i > 0)
         {
            ControlTarget* target = mTargets[i];
            mTargets.erase(mTargets.begin() + i);
            mTargets.insert(mTargets.begin() + (i - 1), target);
            for (auto* section : mSections)
               section->MoveValue(i, -1);
            gHoveredUIControl = nullptr;
         }
         break;
      }
      if (button == mTargets[i]->mMoveRightButton)
      {
         if (i < (int)mTargets.size() - 1)
         {
            ControlTarget* target = mTargets[i];
            mTargets.erase(mTargets.begin() + i);
            mTargets.insert(mTargets.begin() + (i + 1), target);
            for (auto* section : mSections)
               section->MoveValue(i, 1);
            gHoveredUIControl = nullptr;
         }
         break;
      }
   }

   if (button == mAddTargetButton)
      AddTarget();
}

void SongBuilder::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mIntervalSelector)
   {
      TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
      if (transportListenerInfo != nullptr)
         transportListenerInfo->mInterval = mInterval;
   }
   for (int i = 0; i < (int)mSections.size(); ++i)
   {
      if (list == mSections[i]->mContextMenu)
      {
         switch (mSections[i]->mContextMenuSelection)
         {
            case SongSection::ContextMenuItems::kDuplicate:
               DuplicateSection(i);
               break;
            case SongSection::ContextMenuItems::kDelete:
               if (mSections.size() > 1)
               {
                  mSections[i]->CleanUp();
                  mSections.erase(mSections.begin() + i);
                  i = -1;
               }
               break;
            case SongSection::ContextMenuItems::kMoveUp:
               if (i > 0)
               {
                  SongSection* section = mSections[i];
                  mSections.erase(mSections.begin() + i);
                  --i;
                  mSections.insert(mSections.begin() + i, section);
               }
               break;
            case SongSection::ContextMenuItems::kMoveDown:
               if (i < (int)mSections.size() - 1)
               {
                  SongSection* section = mSections[i];
                  mSections.erase(mSections.begin() + i);
                  ++i;
                  mSections.insert(mSections.begin() + i, section);
               }
               break;
            case SongSection::ContextMenuItems::kNone:
               break;
         }

         if (i != -1)
            mSections[i]->mContextMenuSelection = SongSection::ContextMenuItems::kNone;

         break;
      }
   }
}

void SongBuilder::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void SongBuilder::LoadLayout(const ofxJSONElement& moduleInfo)
{
}

void SongBuilder::SetUpFromSaveData()
{
}

void SongBuilder::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   out << (int)mTargets.size();
   for (auto* target : mTargets)
   {
      target->mCable->SaveState(out);
      out << target->mIsCheckbox;
   }

   out << (int)mSections.size();
   for (auto* section : mSections)
   {
      out << section->mName;
      out << section->mId;
      out << (int)section->mValues.size();
      for (auto* value : section->mValues)
      {
         out << value->mId;
         out << value->mValue;
         out << value->mBoolValue;
      }
   }

   IDrawableModule::SaveState(out);

   out << (int)mInterval;
   out << mHasExternalPulseSource;
}

void SongBuilder::LoadState(FileStreamIn& in, int rev)
{
   int numTargets;
   in >> numTargets;
   mTargets.resize(numTargets);
   for (int i = 0; i < numTargets; ++i)
   {
      mTargets[i] = new ControlTarget();
      mTargets[i]->CreateUIControls(this);
      mTargets[i]->mCable->LoadState(in);
      in >> mTargets[i]->mIsCheckbox;
   }

   for (auto* section : mSections)
      section->CleanUp();

   int numSections;
   in >> numSections;
   mSections.resize(numSections);
   for (int i = 0; i < numSections; ++i)
   {
      mSections[i] = new SongSection("");
      in >> mSections[i]->mName;
      in >> mSections[i]->mId;
      mSections[i]->CreateUIControls(this);

      int numValues;
      in >> numValues;
      mSections[i]->mValues.resize(numValues);
      for (int j = 0; j < numValues; ++j)
      {
         mSections[i]->mValues[j] = new ControlValue();
         in >> mSections[i]->mValues[j]->mId;
         in >> mSections[i]->mValues[j]->mValue;
         in >> mSections[i]->mValues[j]->mBoolValue;
         mSections[i]->mValues[j]->CreateUIControls(this);
         mSections[i]->TargetControlUpdated(mTargets[j], j, false);
      }
   }

   IDrawableModule::LoadState(in, rev);

   int interval;
   in >> interval;
   mInterval = (NoteInterval)interval;

   in >> mHasExternalPulseSource;
}

void SongBuilder::DuplicateSection(int sectionIndex)
{
   std::vector<std::string> sectionNames;
   for (auto* section : mSections)
      sectionNames.push_back(section->mName);
   std::string numberless = mSections[sectionIndex]->mName;
   while (numberless.size() > 1 && isdigit(numberless[numberless.size() - 1]))
      numberless = numberless.substr(0, numberless.size() - 1);
   std::string newSectionName = GetUniqueName(numberless, sectionNames);
   SongSection* newSection = new SongSection(newSectionName);
   newSection->CreateUIControls(this);
   mSections.insert(mSections.begin() + sectionIndex + 1, newSection);
   for (auto* value : mSections[sectionIndex]->mValues)
   {
      newSection->AddValue(this);
      auto* newValue = newSection->mValues[newSection->mValues.size() - 1];
      newValue->mValue = value->mValue;
      newValue->mBoolValue = value->mBoolValue;
   }
}

void SongBuilder::AddTarget()
{
   ControlTarget* target = new ControlTarget();
   target->CreateUIControls(this);
   mTargets.push_back(target);

   for (int i = 0; i < (int)mSections.size(); ++i)
      mSections[i]->AddValue(this);
}

void SongBuilder::SongSection::CreateUIControls(SongBuilder* owner)
{
   if (mId == -1)
   {
      //find unique id
      mId = 0;
      for (auto* section : owner->mSections)
      {
         if (section != this && mId <= section->mId)
            mId = section->mId + 1;
      }
   }

#undef UIBLOCK_OWNER
#define UIBLOCK_OWNER owner //change owner
   UIBLOCK0();
   BUTTON_STYLE(mActivateButton, ("go" + ofToString(mId)).c_str(), ButtonDisplayStyle::kPlay);
   TEXTENTRY(mNameEntry, ("name" + ofToString(mId)).c_str(), 12, &mName);
   DROPDOWN(mContextMenu, ("context " + ofToString(mId)).c_str(), (int*)(&mContextMenuSelection), 20);
   ENDUIBLOCK0();
#undef UIBLOCK_OWNER
#define UIBLOCK_OWNER this //reset

   //mContextMenu->SetNoHover(true);
   mContextMenu->AddLabel("duplicate", (int)ContextMenuItems::kDuplicate);
   mContextMenu->AddLabel("delete", (int)ContextMenuItems::kDelete);
   mContextMenu->AddLabel("move up", (int)ContextMenuItems::kMoveUp);
   mContextMenu->AddLabel("move down", (int)ContextMenuItems::kMoveDown);
   mContextMenu->SetDisplayStyle(DropdownDisplayStyle::kHamburger);
}

void SongBuilder::SongSection::AddValue(SongBuilder* owner)
{
   ControlValue* value = new ControlValue();
   value->CreateUIControls(owner);
   int index = (int)mValues.size();
   mValues.push_back(value);
   TargetControlUpdated(owner->mTargets[index], index, false);
}

void SongBuilder::SongSection::TargetControlUpdated(SongBuilder::ControlTarget* target, int targetIndex, bool wasManuallyPatched)
{
   IUIControl* control = target->GetTarget();
   if (control != nullptr)
   {
      if (target->mIsCheckbox)
         mValues[targetIndex]->mBoolValue = control->GetValue() > 0;
      else
         mValues[targetIndex]->mValue = control->GetValue();

      mValues[targetIndex]->mValueEntry->SetShowing(!target->mIsCheckbox);
      mValues[targetIndex]->mCheckbox->SetShowing(target->mIsCheckbox);
   }
   else if (wasManuallyPatched) //user intentionally deleted connection
   {
      mValues[targetIndex]->CleanUp();
      mValues.erase(mValues.begin() + targetIndex);
   }
   else
   {
      mValues[targetIndex]->mValueEntry->SetShowing(false);
      mValues[targetIndex]->mCheckbox->SetShowing(false);
   }
}

void SongBuilder::SongSection::Draw(float x, float y, int sectionIndex, bool isCurrentSection)
{
   float width = GetWidth();
   float height = kRowHeight;
   ofPushStyle();
   ofNoFill();
   ofSetColor(ofColor(150, 150, 150));
   ofRect(x, y, width, height);
   if (isCurrentSection)
   {
      ofFill();
      ofSetColor(ofColor(130, 130, 130, 130));
      ofRect(x, y, width, height);
   }
   ofPopStyle();
   mActivateButton->SetPosition(x + 3, y + 3);
   mActivateButton->Draw();
   mNameEntry->PositionTo(mActivateButton, kAnchor_Right);
   mNameEntry->Draw();
   mContextMenu->PositionTo(mNameEntry, kAnchor_Right);
   mContextMenu->Draw();

   for (int i = 0; i < (int)mValues.size(); ++i)
      mValues[i]->Draw(x + kSectionTabWidth + i * (kColumnWidth + kSpacingX), y, sectionIndex, i);
}

void SongBuilder::SongSection::MoveValue(int index, int amount)
{
   if (index + amount >= 0 && index + amount < (int)mValues.size())
   {
      ControlValue* value = mValues[index];
      mValues.erase(mValues.begin() + index);
      index += amount;
      mValues.insert(mValues.begin() + index, value);
   }
}

float SongBuilder::SongSection::GetWidth() const
{
   return kSectionTabWidth + (int)mValues.size() * (kColumnWidth + kSpacingX);
}

void SongBuilder::SongSection::CleanUp()
{
   mNameEntry->RemoveFromOwner();
   mActivateButton->RemoveFromOwner();
   mContextMenu->RemoveFromOwner();
   for (auto& value : mValues)
      value->CleanUp();
}

void SongBuilder::ControlTarget::CreateUIControls(SongBuilder* owner)
{
   mCable = new PatchCableSource(owner, kConnectionType_UIControl);
   owner->AddPatchCableSource(mCable);
   mCable->SetOverrideCableDir(ofVec2f(0, 1), PatchCableSource::Side::kBottom);
   mMoveLeftButton = new ClickButton(owner, "", -1, -1, ButtonDisplayStyle::kArrowLeft);
   mMoveRightButton = new ClickButton(owner, "", -1, -1, ButtonDisplayStyle::kArrowRight);
}

void SongBuilder::ControlTarget::Draw(float x, float y, int numRows)
{
   ofPushStyle();
   ofFill();
   ofSetColor(ofColor(130, 130, 130, 130));
   ofRect(x, y, kColumnWidth, kTargetTabHeightTop);
   ofRect(x, y + kTargetTabHeightTop + kSpacingY + numRows * (kRowHeight + kSpacingY), kColumnWidth, kTargetTabHeightBottom);
   ofPopStyle();

   IUIControl* target = GetTarget();
   if (target)
   {
      ofPushMatrix();
      ofClipWindow(x, y, kColumnWidth, kTargetTabHeightTop, true);
      std::string text = target->Path(false, true);
      if (mIsCheckbox)
         ofStringReplace(text, "~enabled", "");
      std::string displayString;
      const int kSliceSize = 11;
      int cursor = 0;
      for (; cursor + kSliceSize < (int)text.size(); cursor += kSliceSize)
         displayString += text.substr(cursor, kSliceSize) + "\n";
      displayString += text.substr(cursor, (int)text.size() - cursor);
      DrawTextNormal(displayString, x + 2, y + 9, 9);
      ofPopMatrix();
   }
   float bottomY = y + kTargetTabHeightTop + kSpacingY + numRows * (kRowHeight + kSpacingY);
   mCable->SetManualPosition(x + kColumnWidth * .5f, bottomY + 5);
   mMoveLeftButton->SetPosition(x, bottomY - 3);
   if (gHoveredUIControl == mMoveLeftButton)
      mMoveLeftButton->Draw();
   mMoveRightButton->SetPosition(x + kColumnWidth - 20, bottomY - 3);
   if (gHoveredUIControl == mMoveRightButton)
      mMoveRightButton->Draw();
}

IUIControl* SongBuilder::ControlTarget::GetTarget() const
{
   IUIControl* target = nullptr;
   if (!mCable->GetPatchCables().empty())
      target = dynamic_cast<IUIControl*>(mCable->GetPatchCables()[0]->GetTarget());
   return target;
}

void SongBuilder::ControlTarget::TargetControlUpdated()
{
   IUIControl* target = GetTarget();
   mIsCheckbox = (dynamic_cast<Checkbox*>(target) != nullptr);
}

void SongBuilder::ControlTarget::CleanUp()
{
   mCable->GetOwner()->RemovePatchCableSource(mCable);
   mMoveLeftButton->RemoveFromOwner();
   mMoveRightButton->RemoveFromOwner();
}

void SongBuilder::ControlValue::CreateUIControls(SongBuilder* owner)
{
   if (mId == -1)
   {
      //find unique id
      mId = 0;
      for (auto* section : owner->mSections)
      {
         for (auto* value : section->mValues)
         {
            if (value != this && mId <= value->mId)
               mId = value->mId + 1;
         }
      }
   }

   mValueEntry = new TextEntry(owner, ("value " + ofToString(mId)).c_str(), -1, -1, 4, &mValue, -9999, 9999);
   mCheckbox = new Checkbox(owner, ("checkbox " + ofToString(mId)).c_str(), -1, -1, &mBoolValue);
   mCheckbox->SetDisplayText(false);
}

void SongBuilder::ControlValue::Draw(float x, float y, int sectionIndex, int targetIndex)
{
   ofPushStyle();
   ofFill();
   ofSetColor(ofColor(130, 130, 130, 130));
   ofRect(x, y + 2, kColumnWidth, kRowHeight - 4);
   ofPopStyle();

   mValueEntry->SetPosition(x + 7, y + 3);
   mValueEntry->Draw();
   mCheckbox->SetPosition(x + 20, y + 3);
   mCheckbox->Draw();
}

void SongBuilder::ControlValue::CleanUp()
{
   mValueEntry->RemoveFromOwner();
   mCheckbox->RemoveFromOwner();
}