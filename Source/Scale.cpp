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
//  Scale.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 11/24/12.
//
//

#include "Scale.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "ofxJSONElement.h"
#include "Tunings.h"
#include "libMTSClient.h"

#include "juce_gui_basics/juce_gui_basics.h"

Scale* TheScale = nullptr;

namespace
{
   std::string kChromaticScale = "chromatic";
}

Scale::Scale()
: mRootSelector(nullptr)
, mScaleSelector(nullptr)
, mScaleDegree(0)
, mScaleDegreeSlider(nullptr)
, mNumSeptatonicScales(0)
, mPitchesPerOctave(12)
, mReferenceFreq(440)
, mReferencePitch(69)
, mPitchesPerOctaveEntry(nullptr)
, mReferenceFreqEntry(nullptr)
, mReferencePitchEntry(nullptr)
, mIntonation(kIntonation_Equal)
, mIntonationSelector(nullptr)
{
   assert(TheScale == nullptr);
   TheScale = this;
   SetName("scale");
}

Scale::~Scale()
{
    if (mOddsoundMTSClient)
    {
        MTS_DeregisterClient(mOddsoundMTSClient);
        mOddsoundMTSClient = nullptr;
    }
}

void Scale::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mRootSelector = new DropdownList(this,"root",4,5,&mScale.mScaleRoot);
   mScaleSelector = new DropdownList(this,"scale",58,5,&mScaleIndex);
   mScaleDegreeSlider = new IntSlider(this,"degree",HIDDEN_UICONTROL,HIDDEN_UICONTROL,115,15,&mScaleDegree,-7,7);
   mIntonationSelector = new DropdownList(this,"intonation",58,24,(int*)(&mIntonation));
   mPitchesPerOctaveEntry = new TextEntry(this,"PPO",4,24,2,&mPitchesPerOctave,0,99);
   mReferenceFreqEntry = new TextEntry(this,"tuning",4,43,3,&mReferenceFreq,1,999);
   mReferencePitchEntry = new TextEntry(this,"note",76,43,3,&mReferencePitch,0,127);
   mLoadSCLButton = new ClickButton(this, "load SCL", 4, 62);
   mLoadKBMButton = new ClickButton(this, "load KBM", 74, 62);

   mPitchesPerOctaveEntry->DrawLabel(true);
   mReferenceFreqEntry->DrawLabel(true);
   mReferencePitchEntry->DrawLabel(true);
   
   SetUpRootList();
   
   mIntonationSelector->AddLabel("equal", kIntonation_Equal);
   mIntonationSelector->AddLabel("ratio", kIntonation_Rational);
   mIntonationSelector->AddLabel("just", kIntonation_Just);
   mIntonationSelector->AddLabel("pyth", kIntonation_Pythagorean);
   mIntonationSelector->AddLabel("mean", kIntonation_Meantone);
   mIntonationSelector->AddLabel("scl/kbm", kIntonation_SclFile);
   mIntonationSelector->AddLabel("oddsound", kIntonation_Oddsound);
}

void Scale::SetUpRootList()
{
   mRootSelector->Clear();

   if (mPitchesPerOctave == 12)
   {
      mRootSelector->AddLabel("A", 9);
      mRootSelector->AddLabel("A#/Bb", 10);
      mRootSelector->AddLabel("B", 11);
      mRootSelector->AddLabel("C", 0);
      mRootSelector->AddLabel("C#/Db", 1);
      mRootSelector->AddLabel("D", 2);
      mRootSelector->AddLabel("D#/Eb", 3);
      mRootSelector->AddLabel("E", 4);
      mRootSelector->AddLabel("F", 5);
      mRootSelector->AddLabel("F#/Gb", 6);
      mRootSelector->AddLabel("G", 7);
      mRootSelector->AddLabel("G#/Ab", 8);
   }
   else
   {
      for (int i = 0; i < mPitchesPerOctave; ++i)
         mRootSelector->AddLabel(ofToString(i), i);
      if (mScale.mScaleRoot >= mPitchesPerOctave)
         SetRoot(0);
   }
}

void Scale::Init()
{
   IDrawableModule::Init();
   
   ofxJSONElement root;
   root.open(ofToDataPath("scales.json"));
   
   Json::Value& scales = root["scales"];

   mScales.push_back(ScaleInfo(kChromaticScale, std::vector<int>{}));  //this list is intentionally empty: 0 pitches means "chromatic"
   mScaleSelector->AddLabel(kChromaticScale, 0);

   for (int i=0; i<scales.size(); ++i)
   {
      try
      {
         Json::Value& scale = scales[i];
      
         ScaleInfo scaleInfo;
         scaleInfo.mName = scale.begin().key().asString();
         if (scaleInfo.mName == kChromaticScale) //this is a reserved word now
            continue;

         Json::Value& pitches = scale[scaleInfo.mName];
         for (int j=0; j<pitches.size(); ++j)
         {
            int pitch = pitches[j].asInt();
            scaleInfo.mPitches.push_back(pitch);
         }
         mScales.push_back(scaleInfo);
      
         mScaleSelector->AddLabel(scaleInfo.mName.c_str(), mScaleSelector->GetNumValues());
      
         if (scaleInfo.mPitches.size() == 7)
         {
            ++mNumSeptatonicScales;
            assert(mNumSeptatonicScales == i+1);   //make sure septatonic scales come first (after initial chromatic scale)
         }
      }
      catch (Json::LogicError& e)
      {
         TheSynth->LogEvent(__PRETTY_FUNCTION__ + std::string(" json error: ") + e.what(), kLogEventType_Error);
      }
   }
   
   if (mNumSeptatonicScales == 0) //dumb-but-acceptable handling of an assumption
   {
      mScales.clear();
      mScales.push_back(ScaleInfo(kChromaticScale, std::vector<int>{}));  //this list is intentionally empty: 0 pitches means "chromatic"
      mScaleSelector->AddLabel(kChromaticScale, 0);
      
      mScales.push_back(ScaleInfo("ionian", std::vector<int>{ 0,2,4,5,7,9,11 }));
      mScaleSelector->AddLabel("ionian", 1);

      mNumSeptatonicScales = 1;
   }
   
   SetRoot(gRandom()%TheScale->GetPitchesPerOctave());
   SetRandomSeptatonicScale();
}

float Scale::PitchToFreq(float pitch)
{
   if (mIntonation == kIntonation_SclFile)
   {
      auto ip = (int)pitch + 128;
      if (ip < 0 || ip > 256) return 440;

      // Interpolate in log space
      auto lt = mTuningTable[ip];
      auto nt = mTuningTable[ip + ((ip != 255) ? 1 : 0)];
      auto fp = (pitch + 128) - ip;
      auto interplt = (1 - fp) * lt + fp * nt;

      // Then pow2 it and multiply by freq0
      return Pow2(interplt) * Tunings::MIDI_0_FREQ;
   }
   
   if (mIntonation == kIntonation_Oddsound)
   {
      if (mOddsoundMTSClient && MTS_HasMaster(mOddsoundMTSClient))
      {
         if (pitch < 0 || pitch > 127)
            return Pow2((pitch - mReferencePitch) / mPitchesPerOctave) * mReferenceFreq; // Improve this obviously
         else
            return MTS_NoteToFrequency(mOddsoundMTSClient, (int)pitch, 0);
      }
      else 
      {
         return Pow2((pitch - mReferencePitch) / mPitchesPerOctave) * mReferenceFreq;
      }
   }

   switch (mIntonation)
   {
      case kIntonation_Equal:
         return Pow2((pitch-mReferencePitch)/mPitchesPerOctave) * mReferenceFreq;
      /*case kIntonation_Rational:
      {
         int referencePitch = ScaleRoot();
         do
         {
            referencePitch += mPitchesPerOctave;
         }while (referencePitch < mReferencePitch && abs(referencePitch - mReferencePitch) > mPitchesPerOctave);
         float referenceFreq = Pow2((referencePitch-mReferencePitch)/mPitchesPerOctave)*mReferenceFreq;
         
         int intPitch = (int)pitch;
         float remainder = pitch - intPitch;
         float ratio1 = RationalizeNumber(Pow2(float(intPitch-referencePitch)/mPitchesPerOctave));
         float ratio2 = RationalizeNumber(Pow2(float((intPitch+1)-referencePitch)/mPitchesPerOctave));
         return ofLerp(ratio1,ratio2,remainder)*referenceFreq;
      }*/
      case kIntonation_Pythagorean:
      case kIntonation_Just:
      case kIntonation_Rational:
      case kIntonation_Meantone:
      {
         int referencePitch = ScaleRoot();
         do
         {
            referencePitch += mPitchesPerOctave;
         }while (referencePitch < mReferencePitch && abs(referencePitch - mReferencePitch) > mPitchesPerOctave);
         float referenceFreq = Pow2((referencePitch-mReferencePitch)/mPitchesPerOctave) * mReferenceFreq;
         
         int intPitch = (int)pitch;
         float remainder = pitch - intPitch;
         float ratio1 = GetTuningTableRatio(intPitch - referencePitch);
         float ratio2 = GetTuningTableRatio((intPitch+1) - referencePitch);
         float freq = MAX(ofLerp(ratio1,ratio2,remainder), .001f)*referenceFreq;
         return freq;
         
         break;
      }
      default:
         assert(false);
   }
   assert(false);
   return 0;
}

float Scale::FreqToPitch(float freq)
{
   //TODO(Ryan) always use equal for now
   //switch (mIntonation)
   //{
   //   case kIntonation_Equal:
         return mReferencePitch + mPitchesPerOctave*log2(freq/mReferenceFreq);
   //   default:
   //      assert(false);
   //}
   //assert(false);
   //return 0;
}

int Scale::MakeDiatonic(int pitch)
{
   if (mScale.GetPitches().empty())   //empty list means "chromatic"
      return pitch;
   
   assert(mScale.mScaleRoot >= 0 && mScale.mScaleRoot < mPitchesPerOctave);
   
   int pitchOut = (pitch - mScale.mScaleRoot) % mPitchesPerOctave; //transform into 0-12 scale space
   
   for (int i=(int)mScale.GetPitches().size() - 1; i >= 0; --i)
   {
      if (mScale.GetScalePitch(i) <= pitchOut)
      {
         pitchOut = mScale.GetScalePitch(i);
         break;
      }
   }
   
   pitchOut += mPitchesPerOctave * ((pitch - mScale.mScaleRoot)/mPitchesPerOctave) + mScale.mScaleRoot; //transform back
   
   return pitchOut;
}

void Scale::GetChordDegreeAndAccidentals(const Chord& chord, int& degree, std::vector<Accidental>& accidentals)
{
   mScale.GetChordDegreeAndAccidentals(chord, degree, accidentals);
}

int Scale::GetPitchFromTone(int n)
{
   return mScale.GetPitchFromTone(n);
}

int Scale::GetToneFromPitch(int pitch)
{
   return mScale.GetToneFromPitch(pitch);
}

void Scale::SetScale(int root, std::string type)
{
   SetRoot(root);
   SetScaleType(type);
}

void Scale::SetRoot(int root, bool force)
{
   if (root == mScale.mScaleRoot && !force)
      return;
   
   mScale.SetRoot(root);

   NotifyListeners();
}

void Scale::SetScaleType(std::string type, bool force)
{
   int oldScaleIndex = mScaleIndex;
   
   for (int i=0; i<mScales.size(); ++i)
   {
      if (mScales[i].mName == type)
      {
         mScaleIndex = i;
         break;
      }
   }
   
   if (mScaleIndex == oldScaleIndex && !force)
      return;
   
   mScale.SetScaleType(type);
   
   NotifyListeners();
}

void Scale::SetRandomSeptatonicScale()
{
   mScaleIndex = gRandom()%mNumSeptatonicScales + 1;
   mScale.SetScaleType(mScales[mScaleIndex].mName);
}

void Scale::SetAccidentals(const std::vector<Accidental>& accidentals)
{
   if (accidentals == mScale.mAccidentals)
      return;
   
   mScale.SetAccidentals(accidentals);
   
   NotifyListeners();
}

bool Scale::IsRoot(int pitch)
{
   return mScale.IsRoot(pitch);
}

bool Scale::IsInPentatonic(int pitch)
{
   return mScale.IsInPentatonic(pitch);
}

bool Scale::IsInScale(int pitch)
{
   return mScale.IsInScale(pitch);
}

void Scale::AddListener(IScaleListener* listener)
{
   mListeners.push_back(listener);
}

void Scale::RemoveListener(IScaleListener* listener)
{
   mListeners.remove(listener);
}

void Scale::NotifyListeners()
{
   for (std::list<IScaleListener*>::iterator i = mListeners.begin(); i != mListeners.end(); ++i)
   {
      (*i)->OnScaleChanged();
   }
}

void Scale::SetScaleDegree(int degree)
{
   if (degree == mScaleDegree)
      return;
   
   mScaleDegree = degree;
   NotifyListeners();
}

void Scale::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mPitchesPerOctaveEntry->SetShowing(mIntonation != kIntonation_SclFile && mIntonation != kIntonation_Oddsound);
   mReferenceFreqEntry->SetShowing(mIntonation != kIntonation_Oddsound && (mIntonation != kIntonation_SclFile || mKbmContents.empty()));
   mReferencePitchEntry->SetShowing(mIntonation != kIntonation_Oddsound && (mIntonation != kIntonation_SclFile || mKbmContents.empty()));
   mLoadSCLButton->SetShowing(mIntonation == kIntonation_SclFile && mIntonation != kIntonation_Oddsound);
   mLoadKBMButton->SetShowing(mIntonation == kIntonation_SclFile && mIntonation != kIntonation_Oddsound);

   mRootSelector->Draw();
   mScaleSelector->Draw();
   mPitchesPerOctaveEntry->Draw();
   mReferenceFreqEntry->Draw();
   mReferencePitchEntry->Draw();
   mIntonationSelector->Draw();
   mLoadSCLButton->Draw();
   mLoadKBMButton->Draw();

   if (mIntonation == kIntonation_SclFile)
   {
      DrawTextNormal("PPO: "+ofToString(mPitchesPerOctave), 5, 36);
      DrawTextNormal(mCustomScaleDescription, 5, 91);
   }
   if (mIntonation == kIntonation_Oddsound)
   {
      DrawTextNormal("PPO: " + ofToString(mPitchesPerOctave), 5, 36);
      DrawTextNormal(mCustomScaleDescription, 5, 45);
   }
}

void Scale::GetModuleDimensions(float& width, float& height)
{
   width = 164;
   height = mIntonation == kIntonation_SclFile ? 109  : 62;
}

std::vector<int> Scale::GetPitchesForScale(std::string scaleType)
{
   for (int i=0; i<mScales.size(); ++i)
   {
      if (mScales[i].mName == scaleType)
         return mScales[i].mPitches;
   }
   assert(false);
   return std::vector<int>();
}

void Scale::Poll()
{
   ComputeSliders(0);
}

float Scale::RationalizeNumber(float input)
{
   int m[2][2];
   float x = input;
   int maxden = 32;
   int ai;
   
   /* initialize matrix */
   m[0][0] = m[1][1] = 1;
   m[0][1] = m[1][0] = 0;
   
   /* loop finding terms until denom gets too big */
   while (m[1][0] *  ( ai = (int)x ) + m[1][1] <= maxden)
   {
      int t;
      t = m[0][0] * ai + m[0][1];
      m[0][1] = m[0][0];
      m[0][0] = t;
      t = m[1][0] * ai + m[1][1];
      m[1][1] = m[1][0];
      m[1][0] = t;
      
      if(x == (float)ai)
         break;     // AF: division by zero
      
      x = 1/(x - (float) ai);
      
      if(x>(float)0x7FFFFFFF)
         break;  // AF: representation failure
   }
   
   int numerator = m[0][0];
   int denominator = m[1][0];
   
   if (m[1][0] == 0) //avoid div by zero
      return input;
   
   ai = (maxden - m[1][1]) / m[1][0];
   int otherNumerator = m[0][0] * ai + m[0][1];
   int otherDenominator = m[1][0] * ai + m[1][1];
   
   if (otherDenominator < denominator)
   {
      numerator = otherNumerator;
      denominator = otherDenominator;
   //   printf("*");
   }
   
   if (denominator == 0) //avoid div by zero
      return input;
   
   float output = float(numerator) / denominator;
   
   //printf("input: %f, output: %f, %d/%d, error = %e\n", input, output, numerator, denominator, input - output);
   
   return output;
}

void Scale::UpdateTuningTable()
{
   if (mIntonation == kIntonation_Oddsound)
   {
      //TODO(Ryan): should be smarter about making scale choices based on incoming pitches-per-octave from oddsound
      SetScaleType(kChromaticScale);

      if (mOddsoundMTSClient == nullptr)
      {
         ofLog() << "Connecting to oddsound mts";
         mOddsoundMTSClient = MTS_RegisterClient();
         mCustomScaleDescription = "connected to oddsound";
      }

      if (mOddsoundMTSClient == nullptr)
      {
         mIntonation = kIntonation_Equal;
         mCustomScaleDescription = "connection to oddsound failed";
      }
   }
   else if (mIntonation == kIntonation_SclFile)
   {
      try
      {
         Tunings::Scale scale;
         Tunings::KeyboardMapping mapping;
         if (mSclContents.empty())
            scale = Tunings::evenTemperament12NoteScale();
         else
            scale = Tunings::parseSCLData(mSclContents);

         if (scale.count != 12)
            SetScaleType(kChromaticScale);

         if (mPitchesPerOctave != scale.count)
         {
            mPitchesPerOctave = scale.count;
            SetUpRootList();
            NotifyListeners();
         }

         if (mKbmContents.empty())
            mapping = Tunings::startScaleOnAndTuneNoteTo(60, (int)mReferencePitch, mReferenceFreq);
         else
            mapping = Tunings::parseKBMData(mKbmContents);

         auto tuning = Tunings::Tuning(scale, mapping);
         for (int i=0; i<256; ++i)
         {
            mTuningTable[i] = tuning.logScaledFrequencyForMidiNote(i-128);
         }

         mCustomScaleDescription = "scale: " + scale.description + "\nmapping: " + mapping.rawText;
      }
      catch(const Tunings::TuningError &e)
      {
         mSclContents.clear();
         mKbmContents.clear();
         mIntonation = kIntonation_Equal;
         mPitchesPerOctave = 12;
         ofLog() << e.what();
      }
   }
   else if (mIntonation == kIntonation_Equal)
   {
      //no table
   }
   else if (mIntonation == kIntonation_Rational)
   {
      for (int i=0; i<256; ++i)
         mTuningTable[i] = RationalizeNumber(Pow2(float(i-128)/mPitchesPerOctave));
   }
   else if (mIntonation == kIntonation_Pythagorean ||
       mIntonation == kIntonation_Just ||
       mIntonation == kIntonation_Meantone)
   {
      if (mPitchesPerOctave != 12)
      {
         mPitchesPerOctave = 12;  //only 12-PPO supported for these
         SetUpRootList();
         NotifyListeners();
      }
      
      float tunings[12];
      if (mIntonation == kIntonation_Pythagorean)
      {
         tunings[0] = 1;
         tunings[1] = 256.0f/243.0f;
         tunings[2] = 9.0f/8.0f;
         tunings[3] = 32.0f/27.0f;
         tunings[4] = 81.0f/64.0f;
         tunings[5] = 4.0f/3.0f;
         tunings[6] = 729.f/512.f;
         tunings[7] = 3.0f/2.0f;
         tunings[8] = 128.0f/81.0f;
         tunings[9] = 27.0f/16.0f;
         tunings[10] = 16.0f/9.0f;
         tunings[11] = 243.0f/128.0f;
      }
      if (mIntonation == kIntonation_Just)
      {
         tunings[0] = 1;
         tunings[1] = 25.0f/24.0f;
         tunings[2] = 9.0f/8.0f;
         tunings[3] = 6.0f/5.0f;
         tunings[4] = 5.0f/4.0f;
         tunings[5] = 4.0f/3.0f;
         tunings[6] = 45.0f/32.0f;
         tunings[7] = 3.0f/2.0f;
         tunings[8] = 8.0f/5.0f;
         tunings[9] = 5.0f/3.0f;
         tunings[10] = 9.0f/5.0f;
         tunings[11] = 15.0f/8.0f;
      }
      if (mIntonation == kIntonation_Meantone)
      {
         float fifth = pow(5,.25f);
         float rootFive = sqrtf(5);
         tunings[0] = 1;
         tunings[1] = 8*fifth*rootFive/25;
         tunings[2] = rootFive/2;
         tunings[3] = 4*fifth/5;
         tunings[4] = 5.0f/4.0f;
         tunings[5] = 2*rootFive*fifth/5;
         tunings[6] = 16*rootFive/25;
         tunings[7] = fifth;
         tunings[8] = 8.0f/5.0f;
         tunings[9] = rootFive*fifth/2;
         tunings[10] = 4*rootFive/5;
         tunings[11] = 5*fifth/4;
      }
      
      for (int i=0; i<256; ++i)
      {
         int octave = floor((i-128) / 12.0f);
         float ratio = powf(2,octave);
         mTuningTable[i] = tunings[(i-128+144)%12] * ratio; //+144 to keep modulo arithmetic positive
      }

      /*
       * At this point we need to make sure the tuning table matches the reference pitch
       */
      auto idx = (int)mReferencePitch + 128;
      if (idx >= 0 && idx < 256) {
          auto ttRP = mTuningTable[idx];
          auto lf   = mReferenceFreq / Tunings::MIDI_0_FREQ;
          auto ratio = lf / ttRP;
          for (int i=0; i<256; ++i)
              mTuningTable[i] *= ratio;
      }
   }
}

float Scale::GetTuningTableRatio(int semitonesFromCenter)
{
   return mTuningTable[CLAMP(128+semitonesFromCenter,0,255)];
}

void Scale::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mRootSelector)
      SetRoot(mScale.mScaleRoot, true);
   if (list == mScaleSelector)
      SetScaleType(list->GetLabel(mScaleIndex), true);
   if (list == mIntonationSelector)
      UpdateTuningTable();
}

void Scale::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void Scale::IntSliderUpdated(IntSlider* slider, int oldVal)
{
}

void Scale::CheckboxUpdated(Checkbox *checkbox)
{
}

void Scale::TextEntryComplete(TextEntry* entry)
{
   if (entry == mPitchesPerOctaveEntry)
   {
      UpdateTuningTable();
      SetUpRootList();
      NotifyListeners();
   }
   if (entry == mReferenceFreqEntry || entry == mReferencePitchEntry)
      UpdateTuningTable();
}

void Scale::ButtonClicked(ClickButton *button)
{
    if (button == mLoadSCLButton || button == mLoadKBMButton)
    {
        std::string prompt = "Load ";
        prompt += (button == mLoadSCLButton) ? "SCL" : "KBM";
        std::string pat = (button == mLoadSCLButton) ? "*.scl" : "*.kbm";
        juce::FileChooser chooser( prompt, juce::File(""), pat, true, false, TheSynth->GetMainComponent()->getTopLevelComponent());
        if (chooser.browseForFileToOpen())
        {
            auto file = chooser.getResult();
            std::cout << file.getFullPathName().toStdString() << std::endl;
            if (button == mLoadSCLButton)
            {
                mSclContents = file.loadFileAsString().toStdString();
            }
            else
            {
                mKbmContents = file.loadFileAsString().toStdString();
            }
            UpdateTuningTable();
        }
    }
}

namespace
{
   const int kSaveStateRev = 1;
}

void Scale::SaveState(FileStreamOut &out)
{
   IDrawableModule::SaveState(out);

   out << kSaveStateRev;

   out << mIntonation;
   out << mSclContents;
   out << mKbmContents;
}

void Scale::LoadState(FileStreamIn &in)
{
   IDrawableModule::LoadState(in);

   if (!ModuleContainer::DoesModuleHaveMoreSaveData(in))
      return;  //this was saved before we added versioning, bail out

   int rev;
   in >> rev;
   LoadStateValidate(rev >= kSaveStateRev);

   int inton;
   in >> inton;
   mIntonation = (Scale::IntonationMode)inton;
   in >> mSclContents;
   in >> mKbmContents;

   if (!(mSclContents.empty() && mKbmContents.empty()))
   {
      ofLog() << "Restoring SCL/KBM from streaming";
      UpdateTuningTable();
   }
}

void ScalePitches::SetRoot(int root)
{
   assert(root >= 0);
   mScaleRoot = root % TheScale->GetPitchesPerOctave();
}

void ScalePitches::SetScaleType(std::string type)
{
   mScaleType = type;

   int newFlip = (mScalePitchesFlip == 0) ? 1 : 0;
   mScalePitches[newFlip] = TheScale->GetPitchesForScale(type);
   mScalePitchesFlip = newFlip;   
}

void ScalePitches::SetAccidentals(const std::vector<Accidental>& accidentals)
{
   mAccidentals = accidentals;
}

void ScalePitches::GetChordDegreeAndAccidentals(const Chord& chord, int& degree, std::vector<Accidental>& accidentals) const
{
   int pitch = chord.mRootPitch;
   ChordType type = chord.mType;
   
   //assert(IsInScale(pitch));  //don't support nondiatonic roots yet until I find an example
   degree = GetToneFromPitch(pitch);
   
   std::vector<int> chordForm;
   if (type == kChord_Maj)
   {
      chordForm.push_back(0);
      chordForm.push_back(4);
      chordForm.push_back(7);
   }
   if (type == kChord_Min)
   {
      chordForm.push_back(0);
      chordForm.push_back(3);
      chordForm.push_back(7);
   }
   if (type == kChord_Aug)
   {
      chordForm.push_back(0);
      chordForm.push_back(4);
      chordForm.push_back(8);
   }
   if (type == kChord_Dim)
   {
      chordForm.push_back(0);
      chordForm.push_back(3);
      chordForm.push_back(6);
   }
   
   for (int i=0; i<chordForm.size(); ++i)
   {
      int chordPitch = (chordForm[i]+pitch-mScaleRoot+TheScale->GetPitchesPerOctave())%TheScale->GetPitchesPerOctave();
      if (!VectorContains(chordPitch, mScalePitches[mScalePitchesFlip]))
      {
         if (type == kChord_Maj || type == kChord_Aug)
         {
            if (VectorContains(chordPitch-1, mScalePitches[mScalePitchesFlip]))
               accidentals.push_back(Accidental(chordPitch-1, 1)); //sharpen
            else if (VectorContains(chordPitch+1, mScalePitches[mScalePitchesFlip]))
               accidentals.push_back(Accidental(chordPitch+1, -1)); //flatten
         }
         else if (type == kChord_Min || type == kChord_Dim)
         {
            if (VectorContains(chordPitch+1, mScalePitches[mScalePitchesFlip]))
               accidentals.push_back(Accidental(chordPitch+1, -1)); //flatten
            else if (VectorContains(chordPitch-1, mScalePitches[mScalePitchesFlip]))
               accidentals.push_back(Accidental(chordPitch-1, 1)); //sharpen
         }
         else
         {
            assert(false);
         }
      }
   }
}

int ScalePitches::NumTonesInScale() const
{
   int numTones = (int)mScalePitches[mScalePitchesFlip].size();
   if (numTones == 0)
      numTones = TheScale->GetPitchesPerOctave();
   return numTones;
}

int ScalePitches::GetScalePitch(int index) const
{
   if (mScalePitches[mScalePitchesFlip].empty())
      return index;

   assert(index >= 0 && index < mScalePitches[mScalePitchesFlip].size());
   int pitch = mScalePitches[mScalePitchesFlip][index];
   
   for (int i=0; i<mAccidentals.size(); ++i)
   {
      if (mAccidentals[i].mPitch == pitch)
         pitch += mAccidentals[i].mDirection;
   }
   
   return pitch;
}

bool ScalePitches::IsRoot(int pitch) const
{
   pitch -= mScaleRoot;
   pitch += TheScale->GetPitchesPerOctave();
   assert(pitch >= 0);
   pitch %= TheScale->GetPitchesPerOctave();
   
   return pitch == GetScalePitch(0);
}

bool ScalePitches::IsInPentatonic(int pitch) const
{
   if (!IsInScale(pitch))
      return false;
   
   pitch -= mScaleRoot;
   pitch += TheScale->GetPitchesPerOctave();
   assert(pitch >= 0);
   pitch %= TheScale->GetPitchesPerOctave();
   
   bool isMinor = IsInScale(mScaleRoot+3);
   
   if (isMinor)
      return pitch == 0 || pitch == 3 || pitch == 5 || pitch == 7 || pitch == 10;
   else
      return pitch == 0 || pitch == 2 || pitch == 4 || pitch == 7 || pitch == 9;
}

bool ScalePitches::IsInScale(int pitch) const
{
   if (mScalePitches[mScalePitchesFlip].empty())
      return true;

   pitch -= mScaleRoot;
   pitch += TheScale->GetPitchesPerOctave();
   if (pitch < 0)
      return false;
   pitch %= TheScale->GetPitchesPerOctave();
   
   for (int i=0; i<mScalePitches[mScalePitchesFlip].size(); ++i)
   {
      if (pitch == GetScalePitch(i))
         return true;
   }
   
   return false;
}

int ScalePitches::GetPitchFromTone(int n) const
{
   int numTones = NumTonesInScale();

   int octave = n / numTones;
   while (n<0)
   {
      n += numTones;
      --octave;
   }
   int degree = n % numTones;
   
   return GetScalePitch(degree) + TheScale->GetPitchesPerOctave()*octave + mScaleRoot;
}

int ScalePitches::GetToneFromPitch(int pitch) const
{
   if (mScalePitches[mScalePitchesFlip].empty())      //empty list means "chromatic"
      return pitch;

   assert(mScaleRoot >= 0 && mScaleRoot < TheScale->GetPitchesPerOctave());
   
   int numTones = (int)mScalePitches[mScalePitchesFlip].size();
   int rootRel = pitch - mScaleRoot;
   while (rootRel < 0)
      rootRel += TheScale->GetPitchesPerOctave();
   int tone = 0;
   
   for (int i=0; i<999; ++i)
   {
      tone = i;
      
      int octave = i/numTones;
      if (GetScalePitch(i%numTones) + octave*TheScale->GetPitchesPerOctave() >= rootRel)
         break;
   }
   
   return tone;
}


