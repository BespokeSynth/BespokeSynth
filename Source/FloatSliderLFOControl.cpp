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

ADSR FloatSliderLFOControl::sADSR[NUM_GLOBAL_ADSRS];

FloatSliderLFOControl::FloatSliderLFOControl()
: mOwner(NULL)
, mIntervalSelector(NULL)
, mOscSelector(NULL)
, mOffsetSlider(NULL)
, mBiasSlider(NULL)
, mAddSlider(NULL)
, mSoftenSlider(NULL)
, mShuffleSlider(NULL)
, mType(kLFOControlType_LFO)
, mTypeSelector(NULL)
, mADSRDisplay(NULL)
, mADSRIndex(0)
, mADSRSelector(NULL)
, mPinned(false)
, mSliderCable(NULL)
, mADSRLengthMultiplier(1)
, mADSRLengthMultiplierSlider(nullptr)
{
   SetLFOEnabled(false);
   
   mLFOSettings.mInterval = kInterval_1n;
   mLFOSettings.mOscType = kOsc_Sin;
   mLFOSettings.mLFOOffset = 0;
   mLFOSettings.mMin = 0;
   mLFOSettings.mMax = 1;
   mLFOSettings.mBias = .5f;
   mLFOSettings.mAdd = 0;
   mLFOSettings.mSoften = 0;
   mLFOSettings.mShuffle = 0;
   mLFOSettings.mFreeRate = 1;

   mLFO.SetPeriod(mLFOSettings.mInterval);
}

void FloatSliderLFOControl::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mIntervalSelector = new DropdownList(this,"interval",4,74,(int*)(&mLFOSettings.mInterval));
   mOscSelector = new DropdownList(this,"osc",-1,-1,(int*)(&mLFOSettings.mOscType));
   mOffsetSlider = new FloatSlider(this,"off",-1,-1,90,15,&mLFOSettings.mLFOOffset,0,1);
   mFreeRateSlider = new FloatSlider(this,"free rate",-1,-1,90,15,&mLFOSettings.mFreeRate,0,50);
   mBiasSlider = new FloatSlider(this,"bias",-1,-1,90,15,&mLFOSettings.mBias,0,1);
   mTypeSelector = new RadioButton(this,"type",2,16,(int*)(&mType),kRadioHorizontal);
   mADSRDisplay = new ADSRDisplay(this,"adsr",2,40,96,40,&sADSR[0]);
   mADSRSelector = new RadioButton(this,"adsrselector",2,85,&mADSRIndex,kRadioHorizontal);
   mADSRLengthMultiplierSlider = new FloatSlider(this,"length",2,103,90,15,&mADSRLengthMultiplier,.01f,10);
   mMinSlider = new FloatSlider(this,"low",-1,-1,90,15,&mLFOSettings.mMin,0,1);
   mMaxSlider = new FloatSlider(this,"high",-1,-1,90,15,&mLFOSettings.mMax,0,1);
   mAddSlider = new FloatSlider(this,"add",-1,-1,90,15,&mLFOSettings.mAdd,-1,1);
   mSoftenSlider = new FloatSlider(this,"soften",-1,-1,90,15,&mLFOSettings.mSoften,0,1);
   mShuffleSlider = new FloatSlider(this,"shuffle",-1,-1,90,15,&mLFOSettings.mShuffle,0,1);
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
   
   mTypeSelector->AddLabel("lfo", kLFOControlType_LFO);
   mTypeSelector->AddLabel("adsr", kLFOControlType_ADSR);
   //mTypeSelector->AddLabel("draw", kLFOControlType_Drawn);
   
   for (int i=0; i<NUM_GLOBAL_ADSRS; ++i)
      mADSRSelector->AddLabel(ofToString(i).c_str(), i);
   
   mOscSelector->PositionTo(mIntervalSelector, kAnchorDirection_Right);
   mOffsetSlider->PositionTo(mIntervalSelector, kAnchorDirection_Below);
   mFreeRateSlider->PositionTo(mIntervalSelector, kAnchorDirection_Below);
   mBiasSlider->PositionTo(mOffsetSlider, kAnchorDirection_Below);
   mMinSlider->PositionTo(mBiasSlider, kAnchorDirection_Below);
   mMaxSlider->PositionTo(mMinSlider, kAnchorDirection_Below);
   mAddSlider->PositionTo(mMaxSlider, kAnchorDirection_Below);
   mSoftenSlider->PositionTo(mAddSlider, kAnchorDirection_Below);
   mShuffleSlider->PositionTo(mSoftenSlider, kAnchorDirection_Below);
   
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
   mTypeSelector->Draw();
   mIntervalSelector->Draw();
   mOscSelector->Draw();
   mOffsetSlider->Draw();
   mBiasSlider->Draw();
   mFreeRateSlider->Draw();
   mADSRDisplay->Draw();
   mADSRSelector->Draw();
   mMinSlider->Draw();
   mMaxSlider->Draw();
   mAddSlider->Draw();
   mSoftenSlider->Draw();
   mShuffleSlider->Draw();
   mADSRLengthMultiplierSlider->Draw();
   if (!mPinned)
      mPinButton->Draw();
   
   if (mType == kLFOControlType_LFO)
   {
      int x = 5;
      int y = 36;
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
         ofVertex(i + x, ofMap(value,mOwner->GetMax(),mOwner->GetMin(),0,height) + y);
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
               ofMap(GetLFOValue(),mOwner->GetMax(),mOwner->GetMin(),0,height) + y, 2);
   }
}

void FloatSliderLFOControl::SetLFOEnabled(bool enabled)
{
   if (enabled && !mEnabled)  //if turning on
   {
      if (mOwner)
      {
         mLFOSettings.mMin = mOwner->GetValue();
         mLFOSettings.mMax = mOwner->GetValue();
      }
   }
   mEnabled = enabled;
}

void FloatSliderLFOControl::SetOwner(FloatSlider* owner)
{
   if (mOwner == owner)
      return;
   
   bool hadPriorOwner = false;
   if (mOwner != NULL)
   {
      mOwner->SetLFO(NULL);
      hadPriorOwner = true;
   }
   
   assert(owner != NULL);
   
   owner->SetLFO(this);
      
   mOwner = owner;
   if (!hadPriorOwner)
   {
      mLFOSettings.mMin = mOwner->GetValue();
      mLFOSettings.mMax = mOwner->GetValue();
   }
   mMinSlider->SetExtents(owner->GetMin(), owner->GetMax());
   mMaxSlider->SetExtents(owner->GetMin(), owner->GetMax());
   mAddSlider->SetExtents(owner->GetMin() - owner->GetMax(), owner->GetMax() - owner->GetMin());
   mMinSlider->SetMode(owner->GetMode());
   mMaxSlider->SetMode(owner->GetMode());
   mModuleSaveData.SetExtents("low", owner->GetMin(), owner->GetMax());
   mModuleSaveData.SetExtents("high", owner->GetMin(), owner->GetMax());
}

void FloatSliderLFOControl::PostRepatch(PatchCableSource* cableSource)
{
   SetOwner(dynamic_cast<FloatSlider*>(mSliderCable->GetTarget()));
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
   if (mType == kLFOControlType_LFO)
      return GetLFOValue(samplesIn);
   if (mType == kLFOControlType_ADSR)
      return ofClamp(Interp(sADSR[mADSRIndex].Value(gTime + samplesIn * gInvSampleRateMs), mLFOSettings.mMin, mLFOSettings.mMax) + mLFOSettings.mAdd, mOwner->GetMin(), mOwner->GetMax());
   return 0;
}

float FloatSliderLFOControl::GetLFOValue(int samplesIn /*= 0*/, float forcePhase /*= -1*/)
{
   return ofClamp(Interp(mLFO.Value(samplesIn, forcePhase), mLFOSettings.mMin, mLFOSettings.mMax) + mLFOSettings.mAdd, mOwner->GetMin(), mOwner->GetMax());
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
   mIntervalSelector->SetShowing(mType == kLFOControlType_LFO);
   mOscSelector->SetShowing(mType == kLFOControlType_LFO);
   mOffsetSlider->SetShowing(mType == kLFOControlType_LFO && mLFOSettings.mInterval != kInterval_Free);
   mFreeRateSlider->SetShowing(mType == kLFOControlType_LFO && mLFOSettings.mInterval == kInterval_Free);
   mBiasSlider->SetShowing(mType == kLFOControlType_LFO);
   mSoftenSlider->SetShowing(mType == kLFOControlType_LFO);
   mShuffleSlider->SetShowing(mType == kLFOControlType_LFO);
   mADSRDisplay->SetShowing(mType == kLFOControlType_ADSR);
   mADSRSelector->SetShowing(mType == kLFOControlType_ADSR);
   mADSRLengthMultiplierSlider->SetShowing(mType == kLFOControlType_ADSR);
}

void FloatSliderLFOControl::SetRate(NoteInterval rate)
{
   mLFOSettings.mInterval = rate;
   mLFO.SetPeriod(mLFOSettings.mInterval);
}

void FloatSliderLFOControl::RadioButtonUpdated(RadioButton* radio, int oldVal)
{
   if (radio == mTypeSelector)
      UpdateVisibleControls();
   if (radio == mADSRSelector)
      mADSRDisplay->SetADSR(&sADSR[mADSRIndex]);
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
   if (slider == mADSRLengthMultiplierSlider)
      mADSRDisplay->SetMaxTime(1000 * mADSRLengthMultiplier);
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
         
         //set up save data
         mModuleSaveData.SetString("target", mOwner->Path());
         mModuleSaveData.SetBool("enabled", true);
         mModuleSaveData.SetEnumMapFromList("interval", mIntervalSelector);
         mModuleSaveData.SetInt("interval", mLFOSettings.mInterval, 0, 0, false);
         mModuleSaveData.SetEnumMapFromList("osc", mOscSelector);
         mModuleSaveData.SetInt("osc", mLFOSettings.mOscType, 0, 0, false);
         mModuleSaveData.SetFloat("offset", mLFOSettings.mLFOOffset, mOffsetSlider->GetMin(), mOffsetSlider->GetMax(), false);
         mModuleSaveData.SetFloat("bias", mLFOSettings.mBias, mBiasSlider->GetMin(), mBiasSlider->GetMax(), false);
         mModuleSaveData.SetFloat("low", mLFOSettings.mMin, mOwner->GetMin(), mOwner->GetMax(), false);
         mModuleSaveData.SetFloat("high", mLFOSettings.mMax, mOwner->GetMin(), mOwner->GetMax(), false);
         mModuleSaveData.SetFloat("add", mLFOSettings.mAdd, mOwner->GetMin(), mOwner->GetMax(), false);
         mModuleSaveData.SetFloat("soften", mLFOSettings.mSoften, 0, 1, false);
         mModuleSaveData.SetFloat("soften", mLFOSettings.mShuffle, 0, 1, false);
         mModuleSaveData.SetFloat("free_rate", mLFOSettings.mFreeRate, 0, 50, false);
         
         if (mSliderCable == NULL)
         {
            mSliderCable = new PatchCableSource(this, kConnectionType_UIControl);
            AddPatchCableSource(mSliderCable);
            mSliderCable->SetTarget(mOwner);
         }
      }
   }
}

void FloatSliderLFOControl::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   if (mOwner)
      moduleInfo["target"] = mOwner->Path();
}

void FloatSliderLFOControl::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadBool("enabled", moduleInfo, true);
   mModuleSaveData.LoadEnum<NoteInterval>("interval", moduleInfo, kInterval_1n, mIntervalSelector);
   mModuleSaveData.LoadEnum<OscillatorType>("osc", moduleInfo, kOsc_Sin, mOscSelector);
   mModuleSaveData.LoadFloat("offset", moduleInfo, 0);
   mModuleSaveData.LoadFloat("bias", moduleInfo, .5f);
   mModuleSaveData.LoadFloat("low", moduleInfo, 0, mMinSlider);
   mModuleSaveData.LoadFloat("high", moduleInfo, 1, mMaxSlider);
   mModuleSaveData.LoadFloat("add", moduleInfo, 0, mAddSlider);
   mModuleSaveData.LoadFloat("soften", moduleInfo, 0, mSoftenSlider);
   mModuleSaveData.LoadFloat("shuffle", moduleInfo, 0, mShuffleSlider);
   mModuleSaveData.LoadFloat("free_rate", moduleInfo, 0, mFreeRateSlider);
   
   SetUpFromSaveData();
}

void FloatSliderLFOControl::SetUpFromSaveData()
{
   mPinned = true; //only pinned sliders get saved
   SetOwner(dynamic_cast<FloatSlider*>(TheSynth->FindUIControl(mModuleSaveData.GetString("target"))));
   if (mOwner)
      mOwner->SetLFO(this);
   
   mLFOSettings.mInterval = mModuleSaveData.GetEnum<NoteInterval>("interval");
   mLFOSettings.mOscType = mModuleSaveData.GetEnum<OscillatorType>("osc");
   mLFOSettings.mLFOOffset = mModuleSaveData.GetFloat("offset");
   mLFOSettings.mBias = mModuleSaveData.GetFloat("bias");
   mLFOSettings.mMin = mModuleSaveData.GetFloat("low");
   mLFOSettings.mMax = mModuleSaveData.GetFloat("high");
   mLFOSettings.mAdd = mModuleSaveData.GetFloat("add");
   mLFOSettings.mSoften = mModuleSaveData.GetFloat("soften");
   mLFOSettings.mShuffle = mModuleSaveData.GetFloat("shuffle");
   mLFOSettings.mFreeRate = mModuleSaveData.GetFloat("free_rate");
   
   UpdateFromSettings();
   
   mEnabled = mModuleSaveData.GetBool("enabled");
   
   if (mSliderCable == NULL)
   {
      mSliderCable = new PatchCableSource(this, kConnectionType_UIControl);
      AddPatchCableSource(mSliderCable);
      mSliderCable->SetTarget(mOwner);
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
      sInitialized = true;
   }
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
      sLFOPool[index]->GetOwner()->RevokeLFO();
   sLFOPool[index]->Reset();
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
   out << mMin;
   out << mMax;
   out << mBias;
   out << mAdd;
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
   in >> mMin;
   in >> mMax;
   in >> mBias;
   if (rev >= 1)
      in >> mAdd;
   if (rev >= 2)
   {
      in >> mSoften;
      in >> mShuffle;
   }
   if (rev >= 3)
      in >> mFreeRate;
}

