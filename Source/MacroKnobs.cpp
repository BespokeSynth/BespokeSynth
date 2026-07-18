#include "MacroKnobs.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "Slider.h"
#include <cmath>
#include <set>

MacroKnobs::MacroKnobs()
{
}

MacroKnobs::~MacroKnobs()
{
   for (auto knob : mKnobs)
      delete knob;
}

void MacroKnobs::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   for (int i = 0; i < kNumKnobs; ++i)
   {
      Knob* knob = new Knob(this, i);
      mKnobs.push_back(knob);

      int col = i % 2;
      int row = i / 2;
      float cx = kMarginX + col * kKnobSpacingX;
      float cy = kMarginY + row * kKnobSpacingY;

      TextEntry* label = new TextEntry(this, ("label" + ofToString(i)).c_str(), cx - 30, cy + kKnobRadius + 5, 12, &mKnobs[i]->mLabel);
      label->SetText("macro " + ofToString(i + 1));
      mLabels.push_back(label);
   }
}

void MacroKnobs::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   for (int i = 0; i < kNumKnobs; ++i)
   {
      int col = i % 2;
      int row = i / 2;
      float cx = kMarginX + col * kKnobSpacingX;
      float cy = kMarginY + row * kKnobSpacingY;

      ofPushStyle();

      // Knob background
      ofSetColor(0, 0, 0, 150);
      ofCircle(cx, cy, kKnobRadius);

      // Hover overlay
      if (mHoveredKnob == i || mDraggingKnob == i)
      {
         ofSetColor(255, 255, 255, 30);
         ofCircle(cx, cy, kKnobRadius);
      }

      // Knob outline
      ofSetColor(100, 100, 100);
      ofNoFill();
      ofSetLineWidth(2);
      ofCircle(cx, cy, kKnobRadius);

      // Arc showing value
      ofSetColor(mDraggingKnob == i ? 200 : 150, mDraggingKnob == i ? 200 : 150, 200);
      ofSetLineWidth(4);
      ofNoFill();
      ofBeginShape();

      float startAngle = 135;
      float endAngle = startAngle + mKnobs[i]->mValue * 270;
      int numSegments = 32;
      for (int s = 0; s <= numSegments; ++s)
      {
         float angle = startAngle + (endAngle - startAngle) * s / numSegments;
         float angleRad = angle * M_PI / 180.0f;
         float px = cx + cos(angleRad) * (kKnobRadius - 4);
         float py = cy + sin(angleRad) * (kKnobRadius - 4);
         ofVertex(px, py);
      }
      ofEndShape();

      // Knob pointer
      ofSetColor(255, 255, 255);
      ofFill();
      float angleRad = endAngle * M_PI / 180.0f;
      float px = cx + cos(angleRad) * (kKnobRadius - 10);
      float py = cy + sin(angleRad) * (kKnobRadius - 10);
      ofCircle(px, py, 3);

      ofPopStyle();

      mLabels[i]->Draw();
   }
}

void MacroKnobs::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   if (right)
      return;

   for (int i = 0; i < kNumKnobs; ++i)
   {
      int col = i % 2;
      int row = i / 2;
      float cx = kMarginX + col * kKnobSpacingX;
      float cy = kMarginY + row * kKnobSpacingY;

      if (std::hypot(x - cx, y - cy) <= kKnobRadius)
      {
         mDraggingKnob = i;
         mDragStartY = TheSynth->GetMouseY(GetOwningContainer());
         mDragStartValue = mKnobs[i]->mValue;
         break;
      }
   }
}

bool MacroKnobs::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);

   mHoveredKnob = -1;
   for (int i = 0; i < kNumKnobs; ++i)
   {
      int col = i % 2;
      int row = i / 2;
      float cx = kMarginX + col * kKnobSpacingX;
      float cy = kMarginY + row * kKnobSpacingY;

      if (std::hypot(x - cx, y - cy) <= kKnobRadius)
      {
         mHoveredKnob = i;
         break;
      }
   }

   if (mDraggingKnob >= 0)
   {
      float deltaY = mDragStartY - TheSynth->GetMouseY(GetOwningContainer());
      float deltaValue = deltaY * 0.005f; // Sensitivity

      if (GetKeyModifiers() == kModifier_Shift) // Fine tune
         deltaValue *= 0.1f;

      mKnobs[mDraggingKnob]->mValue = ofClamp(mDragStartValue + deltaValue, 0.0f, 1.0f);
      mKnobs[mDraggingKnob]->UpdateControl();
   }

   return false;
}

void MacroKnobs::MouseReleased()
{
   IDrawableModule::MouseReleased();
   mDraggingKnob = -1;
}

void MacroKnobs::SaveLayout(ofxJSONElement& moduleInfo)
{
   for (int i = 0; i < kNumKnobs; ++i)
   {
      moduleInfo["knob" + ofToString(i)] = mKnobs[i]->mValue;
      moduleInfo["label" + ofToString(i)] = mKnobs[i]->mLabel;
   }
}

void MacroKnobs::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("label0", moduleInfo, "macro 1");
   mModuleSaveData.LoadString("label1", moduleInfo, "macro 2");
   mModuleSaveData.LoadString("label2", moduleInfo, "macro 3");
   mModuleSaveData.LoadString("label3", moduleInfo, "macro 4");
   mModuleSaveData.LoadString("label4", moduleInfo, "macro 5");
   mModuleSaveData.LoadString("label5", moduleInfo, "macro 6");

   SetUpFromSaveData();

   for (int i = 0; i < kNumKnobs; ++i)
   {
      if (moduleInfo.isMember("knob" + ofToString(i)))
      {
         mKnobs[i]->mValue = moduleInfo["knob" + ofToString(i)].asFloat();
         mKnobs[i]->UpdateControl();
      }
   }
}

void MacroKnobs::SetUpFromSaveData()
{
   for (int i = 0; i < kNumKnobs; ++i)
   {
      mLabels[i]->SetText(mModuleSaveData.GetString("label" + ofToString(i)));
   }
}

void MacroKnobs::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   // First, build a set of all targets claimed by OTHER knobs (not the one being patched).
   // Then remove any cable on the patched knob that conflicts with an already-used target.
   for (auto knob : mKnobs)
   {
      if (knob->GetCableSource() != cableSource)
         continue;

      // Collect targets owned by all other knobs on this module.
      std::set<IUIControl*> usedByOthers;
      for (auto other : mKnobs)
      {
         if (other->GetCableSource() == cableSource)
            continue;
         for (auto* cable : other->GetCableSource()->GetPatchCables())
         {
            IUIControl* t = dynamic_cast<IUIControl*>(cable->GetTarget());
            if (t != nullptr)
               usedByOthers.insert(t);
         }
      }

      // Remove any cable on this knob whose target is already taken by another knob.
      auto& cables = cableSource->GetPatchCables();
      for (int i = (int)cables.size() - 1; i >= 0; --i)
      {
         IUIControl* t = dynamic_cast<IUIControl*>(cables[i]->GetTarget());
         if (t != nullptr && usedByOthers.count(t))
         {
            cableSource->RemovePatchCable(cables[i]);
            // cables vector has been modified; restart scan.
            i = (int)cableSource->GetPatchCables().size();
         }
      }

      knob->UpdateControl();
      break;
   }
}

void MacroKnobs::GetModuleDimensions(float& width, float& height)
{
   width = kMarginX * 2 + kKnobSpacingX + kKnobRadius * 2 + 10;
   height = kMarginY * 2 + (kNumKnobs / 2 - 1) * kKnobSpacingY + kKnobRadius * 2 + 15;
}


MacroKnobs::Knob::Knob(MacroKnobs* owner, int index)
: mOwner(owner)
, mIndex(index)
{
   mTargetCableSource = new PatchCableSource(mOwner, kConnectionType_Modulator);
   mTargetCableSource->SetModulatorOwner(this);

   int col = mIndex % 2;
   int row = mIndex / 2;
   float cx = kMarginX + col * kKnobSpacingX;
   float cy = kMarginY + row * kKnobSpacingY;

   mTargetCableSource->SetManualPosition(cx + kKnobRadius + 15, cy);
   mTargetCableSource->SetOverrideCableDir(ofVec2f(1, 0), PatchCableSource::Side::kRight);
   mTargetCableSource->SetDefaultPatchBehavior(kDefaultPatchBehavior_Add);
   mOwner->AddPatchCableSource(mTargetCableSource);
}

MacroKnobs::Knob::~Knob()
{
   mTargetCableSource->ClearPatchCables();
   mOwner->RemovePatchCableSource(mTargetCableSource);
}

void MacroKnobs::Knob::UpdateControl()
{
   // Populate mTargets from our cable source WITHOUT calling SetModulator().
   // If SetModulator(this) is called on FloatSliders, their Compute() would invoke
   // Value() and assign that literal value to *mVar — which is wrong for multi-target
   // because Value() can only return one range-mapped result.
   // Instead, we bypass Compute() entirely and push values via SetFromMidiCC in Poll().

   // First, clear modulator on any old targets that had us set.
   for (auto& t : mTargets)
   {
      if (t.mSliderTarget != nullptr && t.mSliderTarget->GetModulator() == this)
         t.mSliderTarget->SetModulator(nullptr);
      t.mUIControlTarget = nullptr;
      t.mSliderTarget = nullptr;
   }

   // Re-populate from current cables.
   if (mTargetCableSource != nullptr)
   {
      const auto& cables = mTargetCableSource->GetPatchCables();
      for (size_t i = 0; i < cables.size() && i < mTargets.size(); ++i)
      {
         IUIControl* target = dynamic_cast<IUIControl*>(cables[i]->GetTarget());
         mTargets[i].mUIControlTarget = target;
         mTargets[i].mSliderTarget = dynamic_cast<FloatSlider*>(target);
         // NOTE: We intentionally do NOT call mSliderTarget->SetModulator(this).
         // All updates are driven by our Poll() via SetFromMidiCC.
      }
   }

   // Ensure we are registered as a poller.
   TheSynth->RemoveExtraPoller(this);
   TheSynth->AddExtraPoller(this);
}

float MacroKnobs::Knob::Value(int samplesIn)
{
   // Return our normalized 0-1 value.
   // Each target slider is responsible for mapping this through its own range
   // via SetFromMidiCC() in Poll().
   return mValue;
}

void MacroKnobs::Knob::Poll()
{
   if (Active())
   {
      mLastPollValue = mValue;
      const float kBlendRate = -9.65784f;
      float blend = exp2(kBlendRate / ofGetFrameRate()); //framerate-independent blend
      mSmoothedValue = mSmoothedValue * blend + mLastPollValue * (1 - blend);

      // Drive every connected target independently.
      // For FloatSliders, SetFromMidiCC(0-1) calls PosToVal() which correctly maps
      // through each slider's own min/max/mode — regardless of what range other
      // connected targets use. This is the correct multi-target normalization.
      for (int i = 0; i < GetNumTargets(); ++i)
      {
         IUIControl* target = mTargets[i].mUIControlTarget;
         if (target == nullptr)
            continue;

         // SetFromMidiCC maps a 0-1 value through the target's own range.
         // This correctly handles logarithmic sliders (e.g. filter cutoff),
         // square-mode sliders, and linear sliders all independently.
         target->SetFromMidiCC(mValue, NextBufferTime(false), SetValueMethod::Modulator);
      }
   }
}
