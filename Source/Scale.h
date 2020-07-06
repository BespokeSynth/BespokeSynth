//
//  Scale.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/24/12.
//
//

#ifndef __modularSynth__Scale__
#define __modularSynth__Scale__

#include "IDrawableModule.h"
#include "DropdownList.h"
#include "LFO.h"
#include "Ramp.h"
#include "Checkbox.h"
#include "Chord.h"
#include "TextEntry.h"
#include "ChordDatabase.h"

class IScaleListener
{
public:
   virtual ~IScaleListener() {}
   virtual void OnScaleChanged() = 0;
};

struct Accidental
{
   Accidental(int pitch, int direction) : mPitch(pitch), mDirection(direction) {}
   int mPitch;
   int mDirection;
};

inline bool operator==(const Accidental& lhs, const Accidental& rhs)
{
   return lhs.mPitch == rhs.mPitch && lhs.mDirection == rhs.mDirection;
}

struct ScalePitches
{
   int mScaleRoot;
   string mScaleType;
   vector<int> mScalePitches;
   std::vector<Accidental> mAccidentals;
   
   void SetRoot(int root);
   void SetScaleType(string type);
   void SetAccidentals(const std::vector<Accidental>& accidentals);
   
   int ScaleRoot() const { return mScaleRoot; }
   string GetType() const { return mScaleType; }
   void GetChordDegreeAndAccidentals(const Chord& chord, int& degree, std::vector<Accidental>& accidentals) const;
   int GetScalePitch(int index) const;
   
   int MakeDiatonic(int pitch) const;
   bool IsRoot(int pitch) const;
   bool IsInPentatonic(int pitch) const;
   bool IsInScale(int pitch) const;
   int GetPitchFromTone(int n) const;
   int GetToneFromPitch(int pitch) const;
   int NumPitchesInScale() const { return (int)mScalePitches.size(); }
};

class Scale : public IDrawableModule, public IDropdownListener, public IFloatSliderListener, public IIntSliderListener, public ITextEntryListener
{
public:
   Scale();
   void Init() override;
   
   string GetTitleLabel() override { return "scale"; }
   void CreateUIControls() override;
   
   bool IsSingleton() const override { return true; }
   
   int MakeDiatonic(int pitch);
   bool IsRoot(int pitch);
   bool IsInPentatonic(int pitch);
   bool IsInScale(int pitch);
   int GetPitchFromTone(int n);
   int GetToneFromPitch(int pitch);
   void SetScale(int root, string type);
   int ScaleRoot() { return mScale.mScaleRoot; }
   string GetType() { return mScale.mScaleType; }
   void SetRoot(int root, bool force = true);
   void SetScaleType(string type, bool force = true);
   void AddListener(IScaleListener* listener);
   void RemoveListener(IScaleListener* listener);
   void Poll() override;
   void SetScaleDegree(int degree);
   int GetScaleDegree() { return mScaleDegree; }
   void SetAccidentals(const std::vector<Accidental>& accidentals);
   void GetChordDegreeAndAccidentals(const Chord& chord, int& degree, std::vector<Accidental>& accidentals);
   ScalePitches& GetScalePitches() { return mScale; }
   vector<int> GetPitchesForScale(string type);
   void SetRandomSeptatonicScale();
   int GetNumScaleTypes() { return (int)mScales.size(); }
   string GetScaleName(int index) { return mScales[index].mName; }
   int NumPitchesInScale() const { return mScale.NumPitchesInScale(); }
   int GetTet() const { return mTet; }
   
   float PitchToFreq(float pitch);
   float FreqToPitch(float freq);
   
   const ChordDatabase& GetChordDatabase() const { return mChordDatabase; }

   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   void TextEntryComplete(TextEntry* entry) override;

private:
   struct ScaleInfo
   {
      string mName;
      vector<int> mPitches;
   };
   
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 164; height = 62; }
   bool Enabled() const override { return true; }
   
   void NotifyListeners();
   
   float RationalizeNumber(float input);
   void UpdateTuningTable();
   float GetTuningTableRatio(int semitonesFromCenter);
   
   enum IntonationMode
   {
      kIntonation_Equal,
      kIntonation_Just,
      kIntonation_Pythagorean,
      kIntonation_Meantone,
      kIntonation_Rational
   };
   
   ScalePitches mScale;
   list<IScaleListener*> mListeners;
   DropdownList* mRootSelector;
   DropdownList* mScaleSelector;
   IntSlider* mScaleDegreeSlider;
   int mScaleDegree;
   
   vector<ScaleInfo> mScales;
   int mNumSeptatonicScales;
   int mScaleIndex;
   
   int mTet;
   float mReferenceFreq;
   float mReferencePitch;
   TextEntry* mTetEntry;
   TextEntry* mReferenceFreqEntry;
   TextEntry* mReferencePitchEntry;
   IntonationMode mIntonation;
   DropdownList* mIntonationSelector;
   
   float mTuningTable[256];
   
   ChordDatabase mChordDatabase;
};

extern Scale* TheScale;

#endif /* defined(__modularSynth__Scale__) */

