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

Scale* TheScale = nullptr;

Scale::Scale()
: mRootSelector(nullptr)
, mScaleSelector(nullptr)
, mScaleDegree(0)
, mScaleDegreeSlider(nullptr)
, mNumSeptatonicScales(0)
, mTet(12)
, mReferenceFreq(440)
, mReferencePitch(69)
, mTetEntry(nullptr)
, mReferenceFreqEntry(nullptr)
, mReferencePitchEntry(nullptr)
, mIntonation(kIntonation_Equal)
, mIntonationSelector(nullptr)
{
   assert(TheScale == nullptr);
   TheScale = this;
   SetName("scale");
}

void Scale::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mRootSelector = new DropdownList(this,"root",4,5,&mScale.mScaleRoot);
   mScaleSelector = new DropdownList(this,"scale",58,5,&mScaleIndex);
   mScaleDegreeSlider = new IntSlider(this,"degree",HIDDEN_UICONTROL,HIDDEN_UICONTROL,115,15,&mScaleDegree,-7,7);
   mIntonationSelector = new DropdownList(this,"intonation",58,24,(int*)(&mIntonation));
   mTetEntry = new TextEntry(this,"tet",4,24,2,&mTet,0,99);
   mReferenceFreqEntry = new TextEntry(this,"tuning",4,43,3,&mReferenceFreq,1,999);
   mReferencePitchEntry = new TextEntry(this,"note",72,43,3,&mReferencePitch,0,127);
   
   mTetEntry->DrawLabel(true);
   mReferenceFreqEntry->DrawLabel(true);
   mReferencePitchEntry->DrawLabel(true);
   
   mRootSelector->AddLabel("A",9);
   mRootSelector->AddLabel("A#/Bb",10);
   mRootSelector->AddLabel("B",11);
   mRootSelector->AddLabel("C",0);
   mRootSelector->AddLabel("C#/Db",1);
   mRootSelector->AddLabel("D",2);
   mRootSelector->AddLabel("D#/Eb",3);
   mRootSelector->AddLabel("E",4);
   mRootSelector->AddLabel("F",5);
   mRootSelector->AddLabel("F#/Gb",6);
   mRootSelector->AddLabel("G",7);
   mRootSelector->AddLabel("G#/Ab",8);
   
   mIntonationSelector->AddLabel("equal", kIntonation_Equal);
   mIntonationSelector->AddLabel("ratio", kIntonation_Rational);
   mIntonationSelector->AddLabel("just", kIntonation_Just);
   mIntonationSelector->AddLabel("pyth", kIntonation_Pythagorean);
   mIntonationSelector->AddLabel("mean", kIntonation_Meantone);
}

void Scale::Init()
{
   IDrawableModule::Init();
   
   ofxJSONElement root;
   root.open(ofToDataPath("scales.json"));
   
   Json::Value& scales = root["scales"];
   mScales.resize(scales.size());
   for (int i=0; i<scales.size(); ++i)
   {
      Json::Value& scale = scales[i];
      
      mScales[i].mName = scale.begin().key().asString();
      Json::Value& pitches = scale[mScales[i].mName];
      for (int j=0; j<pitches.size(); ++j)
      {
         int pitch = pitches[j].asInt();
         mScales[i].mPitches.push_back(pitch);
      }
      
      mScaleSelector->AddLabel(mScales[i].mName.c_str(), i);
      
      if (mScales[i].mPitches.size() == 7)
      {
         ++mNumSeptatonicScales;
         assert(mNumSeptatonicScales == i+1);   //make sure septatonic scales come first
      }
   }
   
   if (mNumSeptatonicScales == 0)
   {
      mNumSeptatonicScales = 1;
      mScaleSelector->AddLabel("ionian", 0);
      mScales.resize(1);
      mScales[0].mName = "ionian";
      mScales[0].mPitches = vector<int>{0,2,4,5,7,9,11};
   }
   
   SetRoot(rand()%TheScale->GetTet());
   SetRandomSeptatonicScale();
}

float Scale::PitchToFreq(float pitch)
{
   switch (mIntonation)
   {
      case kIntonation_Equal:
         return Pow2((pitch-mReferencePitch)/mTet) * mReferenceFreq;
      /*case kIntonation_Rational:
      {
         int referencePitch = ScaleRoot();
         do
         {
            referencePitch += mTet;
         }while (referencePitch < mReferencePitch && abs(referencePitch - mReferencePitch) > mTet);
         float referenceFreq = Pow2((referencePitch-mReferencePitch)/mTet)*mReferenceFreq;
         
         int intPitch = (int)pitch;
         float remainder = pitch - intPitch;
         float ratio1 = RationalizeNumber(Pow2(float(intPitch-referencePitch)/mTet));
         float ratio2 = RationalizeNumber(Pow2(float((intPitch+1)-referencePitch)/mTet));
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
            referencePitch += mTet;
         }while (referencePitch < mReferencePitch && abs(referencePitch - mReferencePitch) > mTet);
         float referenceFreq = Pow2((referencePitch-mReferencePitch)/mTet) * mReferenceFreq;
         
         int intPitch = (int)pitch;
         float remainder = pitch - intPitch;
         float ratio1 = GetTuningTableRatio(intPitch - referencePitch);
         float ratio2 = GetTuningTableRatio((intPitch+1) - referencePitch);
         return ofLerp(ratio1,ratio2,remainder)*referenceFreq;
         
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
         return mReferencePitch + mTet*log2(freq/mReferenceFreq);
   //   default:
   //      assert(false);
   //}
   //assert(false);
   //return 0;
}

int Scale::MakeDiatonic(int pitch)
{
   assert(mScale.mScaleRoot >= 0 && mScale.mScaleRoot < mTet);
   assert(mScale.mScalePitches.size());
   
   int pitchOut = (pitch - mScale.mScaleRoot) % mTet; //transform into 0-12 scale space
   
   for (int i=(int)mScale.mScalePitches.size() - 1; i >= 0; --i)
   {
      if (mScale.GetScalePitch(i) <= pitchOut)
      {
         pitchOut = mScale.GetScalePitch(i);
         break;
      }
   }
   
   pitchOut += mTet * ((pitch - mScale.mScaleRoot)/mTet) + mScale.mScaleRoot; //transform back
   
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

void Scale::SetScale(int root, string type)
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

void Scale::SetScaleType(string type, bool force)
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
   mScaleIndex = rand()%mNumSeptatonicScales;
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
   for (list<IScaleListener*>::iterator i = mListeners.begin(); i != mListeners.end(); ++i)
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

   mRootSelector->Draw();
   mScaleSelector->Draw();
   mTetEntry->Draw();
   mReferenceFreqEntry->Draw();
   mReferencePitchEntry->Draw();
   mIntonationSelector->Draw();
}

vector<int> Scale::GetPitchesForScale(string type)
{
   for (int i=0; i<mScales.size(); ++i)
   {
      if (mScales[i].mName == type)
         return mScales[i].mPitches;
   }
   assert(false);
   return vector<int>();
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
   if (mIntonation == kIntonation_Equal)
   {
      //no table
   }
   if (mIntonation == kIntonation_Rational)
   {
      for (int i=0; i<256; ++i)
         mTuningTable[i] = RationalizeNumber(Pow2(float(i-128)/mTet));
   }
   if (mIntonation == kIntonation_Pythagorean ||
       mIntonation == kIntonation_Just ||
       mIntonation == kIntonation_Meantone)
   {
      if (mTet != 12)
      {
         mTet = 12;  //only 12-tet supported for these
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
         tunings[6] = 1024.0f/729.0f;
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
      SetScaleType(mScales[mScaleIndex].mName, true);
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
   if (entry == mTetEntry)
   {
      UpdateTuningTable();
      NotifyListeners();
   }
}

void ScalePitches::SetRoot(int root)
{
   assert(root >= 0);
   mScaleRoot = root % TheScale->GetTet();
}

void ScalePitches::SetScaleType(string type)
{
   mScaleType = type;
   mScalePitches = TheScale->GetPitchesForScale(type);
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
      int chordPitch = (chordForm[i]+pitch-mScaleRoot+TheScale->GetTet())%TheScale->GetTet();
      if (!VectorContains(chordPitch, mScalePitches))
      {
         if (type == kChord_Maj || type == kChord_Aug)
         {
            if (VectorContains(chordPitch-1, mScalePitches))
               accidentals.push_back(Accidental(chordPitch-1, 1)); //sharpen
            else if (VectorContains(chordPitch+1, mScalePitches))
               accidentals.push_back(Accidental(chordPitch+1, -1)); //flatten
         }
         else if (type == kChord_Min || type == kChord_Dim)
         {
            if (VectorContains(chordPitch+1, mScalePitches))
               accidentals.push_back(Accidental(chordPitch+1, -1)); //flatten
            else if (VectorContains(chordPitch-1, mScalePitches))
               accidentals.push_back(Accidental(chordPitch-1, 1)); //sharpen
         }
         else
         {
            assert(false);
         }
      }
   }
}

int ScalePitches::GetScalePitch(int index) const
{
   int pitch = mScalePitches[index];
   
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
   pitch += TheScale->GetTet();
   assert(pitch >= 0);
   pitch %= TheScale->GetTet();
   
   return pitch == GetScalePitch(0);
}

bool ScalePitches::IsInPentatonic(int pitch) const
{
   if (!IsInScale(pitch))
      return false;
   
   pitch -= mScaleRoot;
   pitch += TheScale->GetTet();
   assert(pitch >= 0);
   pitch %= TheScale->GetTet();
   
   bool isMinor = IsInScale(mScaleRoot+3);
   
   if (isMinor)
      return pitch == 0 || pitch == 3 || pitch == 5 || pitch == 7 || pitch == 10;
   else
      return pitch == 0 || pitch == 2 || pitch == 4 || pitch == 7 || pitch == 9;
}

bool ScalePitches::IsInScale(int pitch) const
{
   pitch -= mScaleRoot;
   pitch += TheScale->GetTet();
   if (pitch < 0)
      return false;
   pitch %= TheScale->GetTet();
   
   for (int i=0; i<mScalePitches.size(); ++i)
   {
      if (pitch == GetScalePitch(i))
         return true;
   }
   
   return false;
}

int ScalePitches::GetPitchFromTone(int n) const
{
   int numTones = (int)mScalePitches.size();
   assert(numTones > 0);
   int octave = n/numTones;
   while (n<0)
   {
      n+=numTones;
      --octave;
   }
   int degree = n%numTones;
   
   return GetScalePitch(degree) + TheScale->GetTet()*octave + mScaleRoot;
}

int ScalePitches::GetToneFromPitch(int pitch) const
{
   assert(mScaleRoot >= 0 && mScaleRoot < TheScale->GetTet());
   assert(mScalePitches.size());
   
   int numTones = (int)mScalePitches.size();
   int rootRel = pitch - mScaleRoot;
   while (rootRel < 0)
      rootRel += TheScale->GetTet();
   int tone = 0;
   
   for (int i=0; i<999; ++i)
   {
      tone = i;
      
      int octave = i/numTones;
      if (GetScalePitch(i%numTones) + octave*TheScale->GetTet() >= rootRel)
         break;
   }
   
   return tone;
}


