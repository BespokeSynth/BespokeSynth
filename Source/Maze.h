#pragma once

#include "IDrawableModule.h"
#include "INoteSource.h"
#include "Transport.h"
#include "Checkbox.h"
#include "DropdownList.h"
#include "ClickButton.h"
#include "Slider.h"
#include "NoteOutputQueue.h"
#include <set>

class Maze : public IDrawableModule, public ITimeListener, public INoteSource, public IDropdownListener, public IButtonListener, public IFloatSliderListener
{
public:
   Maze();
   virtual ~Maze();

   static IDrawableModule* Create() { return new Maze(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }
   void Init() override;

   void CreateUIControls() override;
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(float x, float y, bool right) override;

   void OnTimeEvent(double time) override;
   void Poll() override;

   // Listeners
   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void DropdownUpdated(DropdownList* dropdown, int index, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 1; }

   void GenerateChords();

private:
   Checkbox* mScaledDegreeCheckbox{ nullptr };
   DropdownList* mNumChordsDropdown{ nullptr };
   ClickButton* mRandomiseButton{ nullptr };
   FloatSlider* mGrooveSlider{ nullptr };
   FloatSlider* mHumaniseVelocitySlider{ nullptr };
   FloatSlider* mUpperHarmonicsSlider{ nullptr };
   FloatSlider* mStrumSlider{ nullptr };
   FloatSlider* mHumaniseTimingSlider{ nullptr };

   bool mScaledDegree{ true };
   int mNumChordsIndex{ 3 };
   int mNumChords{ 4 };
   float mGroove{ 0.0f };
   float mHumaniseVelocity{ 0.0f };
   float mUpperHarmonics{ 0.0f };
   float mStrum{ 0.0f };
   float mHumaniseTiming{ 0.0f };

   std::set<int> mLockedNotes;
   TransportListenerInfo* mTransportListenerInfo{ nullptr };

   struct Chord
   {
      std::vector<int> pitches;
   };
   std::vector<Chord> mChords;

   int mCurrentMeasure{ -1 };
   int mCurrentChordIndex{ -1 };
   double mLastChordTime{ -1.0 };

   // State tracking for UI
   std::vector<int> mPlayingNotes;
};
