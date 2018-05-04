//
//  FloatSliderLFOControl.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 2/22/13.
//
//

#include "FloatSliderLFOControl.h"
#include "Slider.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"

FloatSliderLFOControl::FloatSliderLFOControl()
: mIntervalSelector(nullptr)
, mOscSelector(nullptr)
, mOffsetSlider(nullptr)
, mBiasSlider(nullptr)
, mSpreadSlider(nullptr)
, mSoftenSlider(nullptr)
, mShuffleSlider(nullptr)
, mPinned(false)
{
   SetLFOEnabled(false);
   
   mLFOSettings.mInterval = kInterval_1n;
   mLFOSettings.mOscType = kOsc_Sin;
   mLFOSettings.mLFOOffset = 0;
   mLFOSettings.mBias = .5f;
   mLFOSettings.mSpread = 0;
   mLFOSettings.mSoften = 0;
   mLFOSettings.mShuffle = 0;
   mLFOSettings.mFreeRate = 1;

   mLFO.SetPeriod(mLFOSettings.mInterval);
}

void FloatSliderLFOControl::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mIntervalSelector = new DropdownList(this,"interval",4,58,(int*)(&mLFOSettings.mInterval));
   mOscSelector = new DropdownList(this,"osc",-1,-1,(int*)(&mLFOSettings.mOscType));
   mOffsetSlider = new FloatSlider(this,"off",mIntervalSelector, kAnchor_Below,90,15,&mLFOSettings.mLFOOffset,0,1);
   mFreeRateSlider = new FloatSlider(this,"free rate",mIntervalSelector, kAnchor_Below,90,15,&mLFOSettings.mFreeRate,0,50);
   mBiasSlider = new FloatSlider(this,"bias",mOffsetSlider, kAnchor_Below,90,15,&mLFOSettings.mBias,0,1);
   mMinSlider = new FloatSlider(this,"low",mBiasSlider, kAnchor_Below,90,15,&mDummyMin,0,1);
   mMaxSlider = new FloatSlider(this,"high",mMinSlider, kAnchor_Below,90,15,&mDummyMax,0,1);
   mSpreadSlider = new FloatSlider(this,"spread",mMaxSlider, kAnchor_Below,90,15,&mLFOSettings.mSpread,0,1);
   mSoftenSlider = new FloatSlider(this,"soften",mSpreadSlider, kAnchor_Below,90,15,&mLFOSettings.mSoften,0,1);
   mShuffleSlider = new FloatSlider(this,"shuffle",mSoftenSlider, kAnchor_Below,90,15,&mLFOSettings.mShuffle,0,1);
   mPinButton = new ClickButton(this,"pin",70,2);
   mEnableLFOCheckbox = new Checkbox(this,"enable",5,2,&mEnabled);
   
   mIntervalSelector->AddLabel("free", kInterval_Free);
   mIntervalSelector->AddLabel("64", kInterval_64);
   mIntervalSelector->AddLabel("32", kInterval_32);
   mIntervalSelector->AddLabel("16", kInterval_16);
   mIntervalSelector->AddLabel("8", kInterval_8);
   mIntervalSelector->AddLabel("4", kInterval_4);
   mIntervalSelector->AddLabel("3", kInterval_3);
   mIntervalSelector->AddLabel("2", kInterval_2);
   mIntervalSelector->AddLabel("1n", kInterval_1n);
   mIntervalSelector->AddLabel("2n", kInterval_2n);
   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("4nt", kInterval_4nt);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("8nt", kInterval_8nt);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("16nt", kInterval_16nt);
   mIntervalSelector->AddLabel("32n", kInterval_32n);
   
   mOscSelector->AddLabel("sin",kOsc_Sin);
   mOscSelector->AddLabel("saw",kOsc_Saw);
   mOscSelector->AddLabel("-saw",kOsc_NegSaw);
   mOscSelector->AddLabel("squ",kOsc_Square);
   mOscSelector->AddLabel("tri",kOsc_Tri);
   mOscSelector->AddLabel("rand",kOsc_Random);
   mOscSelector->AddLabel("drnk",kOsc_Drunk);
   
   mOscSelector->PositionTo(mIntervalSelector, kAnchor_Right);
   
   mFreeRateSlider->SetMode(FloatSlider::kBezier);
   mFreeRateSlider->SetBezierControl(1);
   
   UpdateVisibleControls();
}

FloatSliderLFOControl::~FloatSliderLFOControl()
{
}

void FloatSliderLFOControl::DrawModule()
{
   if (!mPinned)
   {
      int w,h;
      GetDimensions(w,h);

      ofPushStyle();
      ofSetColor(0,0,0);
      ofFill();
      ofSetLineWidth(.5f);
      ofRect(0,0,w,h);
      ofNoFill();
      ofSetColor(255,255,255);
      ofRect(0,0,w,h);
      ofPopStyle();
   }
   
   if (Minimized())
      return;
   
   mEnableLFOCheckbox->Draw();
   mIntervalSelector->Draw();
   mOscSelector->Draw();
   mOffsetSlider->Draw();
   mBiasSlider->Draw();
   mFreeRateSlider->Draw();
   mMinSlider->Draw();
   mMaxSlider->Draw();
   mSpreadSlider->Draw();
   mSoftenSlider->Draw();
   mShuffleSlider->Draw();
   if (!mPinned)
      mPinButton->Draw();
   
   int x = 5;
   int y = 20;
   int height = 35;
   int width = 90;
   
   ofSetColor(100,100,.8f*gModuleDrawAlpha);
   ofSetLineWidth(.5f);
   ofRect(x,y,width,height, 0);
   
   ofSetColor(245, 58, 0, gModuleDrawAlpha);
   ofSetLineWidth(1);
   
   ofBeginShape();
   
   for (float i=0; i<width; i+=(.25f/gDrawScale))
   {
      float phase = i/width;
      if (mLFO.GetOsc()->GetShuffle() > 0)
         phase *= 2;
      phase += mLFOSettings.mLFOOffset;
      float value = GetLFOValue(0, phase);
      ofVertex(i + x, ofMap(value,mTarget->GetMax(),mTarget->GetMin(),0,height) + y);
   }
   ofEndShape(false);
   
   float currentPhase = mLFO.CalculatePhase();
   float squeeze;
   if (mLFO.GetOsc()->GetShuffle() == 0)
   {
      squeeze = 1;
      currentPhase -= (int)currentPhase;
   }
   else
   {
      squeeze = 2;
   }
   currentPhase -= mLFOSettings.mLFOOffset;
   if (currentPhase < 0)
      currentPhase += squeeze;
   ofCircle(currentPhase / squeeze * width + x,
            ofMap(GetLFOValue(),mTarget->GetMax(),mTarget->GetMin(),0,height) + y, 2);
}

void FloatSliderLFOControl::SetLFOEnabled(bool enabled)
{
   if (enabled && !mEnabled)  //if turning on
   {
      if (mTarget)
      {
         GetMin() = mTarget->GetValue();
         GetMax() = mTarget->GetValue();
      }
   }
   mEnabled = enabled;
}

void FloatSliderLFOControl::SetOwner(FloatSlider* owner)
{
   if (mTarget == owner)
      return;
   
   if (mTarget != nullptr)
   {
      mTarget->SetLFO(nullptr);
   }
   
   assert(owner != nullptr);
   
   owner->SetLFO(this);
   
   mTarget = owner;
   InitializeRange();
}

void FloatSliderLFOControl::RandomizeSettings()
{
   switch (rand() % 8)
   {
      case 0: mLFOSettings.mInterval = kInterval_2; break;
      case 1: mLFOSettings.mInterval = kInterval_1n; break;
      case 2: mLFOSettings.mInterval = kInterval_2n; break;
      case 3: mLFOSettings.mInterval = kInterval_4n; break;
      case 4: mLFOSettings.mInterval = kInterval_4nt; break;
      case 5: mLFOSettings.mInterval = kInterval_8n; break;
      case 6: mLFOSettings.mInterval = kInterval_8nt; break;
      case 7:
         mLFOSettings.mInterval = kInterval_Free;
         mLFOSettings.mFreeRate = ofRandom(.1f, 20);
         break;
   }
   UpdateFromSettings();
   UpdateVisibleControls();
}

void FloatSliderLFOControl::PostRepatch(PatchCableSource* cableSource)
{
   SetOwner(dynamic_cast<FloatSlider*>(mTargetCable->GetTarget()));
   OnModulatorRepatch();
}

void FloatSliderLFOControl::Load(LFOSettings settings)
{
   mLFOSettings = settings;
   UpdateFromSettings();
   mEnabled = true;
}

float FloatSliderLFOControl::Value(int samplesIn /*= 0*/)
{
   ComputeSliders(samplesIn);
   return GetLFOValue(samplesIn);
}

float FloatSliderLFOControl::GetLFOValue(int samplesIn /*= 0*/, float forcePhase /*= -1*/)
{
   float val = mLFO.Value(samplesIn, forcePhase);
   if (mLFOSettings.mSpread > 0)
      val = val * (1-mLFOSettings.mSpread) + (-cosf(val * FPI) + 1) * .5f * mLFOSettings.mSpread;
   return ofClamp(Interp(val, GetMin(), GetMax()), mTarget->GetMin(), mTarget->GetMax());
}

void FloatSliderLFOControl::UpdateFromSettings()
{
   mLFO.SetPeriod(mLFOSettings.mInterval);
   mLFO.SetType(mLFOSettings.mOscType);
   mLFO.SetOffset(mLFOSettings.mLFOOffset);
   mLFO.SetPulseWidth(1-mLFOSettings.mBias);
   mLFO.GetOsc()->SetSoften(mLFOSettings.mSoften);
   mLFO.GetOsc()->SetShuffle(mLFOSettings.mShuffle);
   mLFO.SetFreeRate(mLFOSettings.mFreeRate);
}

void FloatSliderLFOControl::UpdateVisibleControls()
{
   mOffsetSlider->SetShowing(mLFOSettings.mInterval != kInterval_Free);
   mFreeRateSlider->SetShowing(mLFOSettings.mInterval == kInterval_Free);
}

void FloatSliderLFOControl::SetRate(NoteInterval rate)
{
   mLFOSettings.mInterval = rate;
   mLFO.SetPeriod(mLFOSettings.mInterval);
}

void FloatSliderLFOControl::RadioButtonUpdated(RadioButton* radio, int oldVal)
{
}

void FloatSliderLFOControl::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mIntervalSelector)
   {
      if (mLFOSettings.mInterval == kInterval_Free)
      {
         NoteInterval oldInterval = (NoteInterval)oldVal;
         mLFOSettings.mFreeRate = 1000 / TheTransport->GetDuration(oldInterval);
         mLFO.SetFreeRate(mLFOSettings.mFreeRate);
      }
      mLFO.SetPeriod(mLFOSettings.mInterval);
      UpdateVisibleControls();
   }
   if (list == mOscSelector)
      mLFO.SetType(mLFOSettings.mOscType);
}

void FloatSliderLFOControl::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mOffsetSlider)
      mLFO.SetOffset(mLFOSettings.mLFOOffset);
   if (slider == mBiasSlider)
      mLFO.SetPulseWidth(1-mLFOSettings.mBias);
   if (slider == mSoftenSlider)
      mLFO.GetOsc()->SetSoften(mLFOSettings.mSoften);
   if (slider == mShuffleSlider)
      mLFO.GetOsc()->SetShuffle(mLFOSettings.mShuffle);
   if (slider == mFreeRateSlider)
      mLFO.SetFreeRate(mLFOSettings.mFreeRate);
}

void FloatSliderLFOControl::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
   {
      mEnabled = !mEnabled;
      SetLFOEnabled(!mEnabled);  //make sure it sets as a toggle
   }
}

void FloatSliderLFOControl::ButtonClicked(ClickButton* button)
{
   if (button == mPinButton)
   {
      if (!mPinned)
      {
         mPinned = true;
         TheSynth->AddDynamicModule(this);
         TheSynth->PopModalFocusItem();
         
         SetName(GetUniqueName("lfo", TheSynth->GetModuleNames<FloatSliderLFOControl*>()).c_str());
         
         if (mTargetCable == nullptr)
         {
            mTargetCable = new PatchCableSource(this, kConnectionType_UIControl);
            AddPatchCableSource(mTargetCable);
            mTargetCable->SetTarget(mTarget);
         }
      }
   }
}

void FloatSliderLFOControl::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   if (mTarget)
      moduleInfo["target"] = mTarget->Path();
}

void FloatSliderLFOControl::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadBool("enabled", moduleInfo);
   
   SetUpFromSaveData();
}

void FloatSliderLFOControl::SetUpFromSaveData()
{
   mPinned = true; //only pinned sliders get saved
   SetOwner(dynamic_cast<FloatSlider*>(TheSynth->FindUIControl(mModuleSaveData.GetString("target"))));
   if (mTarget)
      mTarget->SetLFO(this);
   
   UpdateFromSettings();
   UpdateVisibleControls();
   
   mEnabled = mModuleSaveData.GetBool("enabled");
   
   if (mTargetCable == nullptr)
   {
      mTargetCable = new PatchCableSource(this, kConnectionType_UIControl);
      AddPatchCableSource(mTargetCable);
      mTargetCable->SetTarget(mTarget);
   }
}

FloatSliderLFOControl* LFOPool::sLFOPool[LFO_POOL_SIZE];
int LFOPool::sNextLFOIndex = 0;
bool LFOPool::sInitialized = false;

void LFOPool::Init()
{
   for (int i=0; i<LFO_POOL_SIZE; ++i)
   {
      sLFOPool[i] = new FloatSliderLFOControl();
      sLFOPool[i]->CreateUIControls();
      sLFOPool[i]->Init();
      sLFOPool[i]->SetType("lfo");
   }
   sInitialized = true;
}

void LFOPool::Shutdown()
{
   if (sInitialized)
   {
      for (int i=0; i<LFO_POOL_SIZE; ++i)
         sLFOPool[i]->Delete();
      sInitialized = false;
   }
}

FloatSliderLFOControl* LFOPool::GetLFO(FloatSlider* owner)
{
   int index = sNextLFOIndex;
   for (int i=0; i<LFO_POOL_SIZE; ++i) //search for the next one that isn't enabled, but only one loop around
   {
      if (!sLFOPool[index]->Enabled() && !sLFOPool[index]->IsPinned())
         break;   //found a disabled one
      index = (index+1) % LFO_POOL_SIZE;
   }
   sNextLFOIndex = (index+1) % LFO_POOL_SIZE;
   if (sLFOPool[index]->GetOwner())
      sLFOPool[index]->GetOwner()->SetLFO(nullptr);   //revoke LFO
   sLFOPool[index]->RandomizeSettings();
   sLFOPool[index]->SetOwner(owner);
   return sLFOPool[index];
}

namespace
{
   const int kSaveStateRev = 3;
   const int kFixNonRevvedData = 999;
}

void LFOSettings::SaveState(FileStreamOut& out) const
{
   out << kFixNonRevvedData;
   out << kSaveStateRev;
   
   out << (int)mInterval;
   out << (int)mOscType;
   out << mLFOOffset;
   out << mBias;
   out << mSpread;
   out << mSoften;
   out << mShuffle;
   out << mFreeRate;
}

void LFOSettings::LoadState(FileStreamIn& in)
{
   int rev = 0;
   bool isDataRevved = false;
   int temp;
   in >> temp;
   if (temp == kFixNonRevvedData) //hack to fix data that I didn't revision
   {
      isDataRevved = true;
      in >> rev;
      LoadStateValidate(rev <= kSaveStateRev);
      in >> temp;
   }
   mInterval = (NoteInterval)temp;
   in >> temp; mOscType = (OscillatorType)temp;
   in >> mLFOOffset;
   in >> mBias;
   if (rev >= 1)
      in >> mSpread;
   if (rev >= 2)
   {
      in >> mSoften;
      in >> mShuffle;
   }
   if (rev >= 3)
      in >> mFreeRate;
}

