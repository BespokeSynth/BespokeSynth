/*
  ==============================================================================

    NoteQuantizer.h
    Created: 6 Dec 2020 7:36:30pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "NoteEffectBase.h"
#include "DropdownList.h"
#include "Transport.h"
#include "IPulseReceiver.h"

class NoteQuantizer : public NoteEffectBase, public IDrawableModule, public IDropdownListener, public ITimeListener, public IPulseReceiver
{
public:
   NoteQuantizer();
   virtual ~NoteQuantizer();
   static IDrawableModule* Create() { return new NoteQuantizer(); }
   void CreateUIControls() override;

   string GetTitleLabel() override { return "quantizer"; }

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;

   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void OnTimeEvent(double time) override;
   void OnPulse(double time, float velocity, int flags) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 80; height = 40; }
   bool Enabled() const override { return true; }
   void OnEvent(double time, float strength);

   struct InputInfo
   {
      InputInfo() : velocity(0), voiceIdx(-1), held(false), hasPlayedYet(false) {}
      int velocity;
      int voiceIdx;
      bool held;
      bool hasPlayedYet;
      ModulationParameters modulation;
   };

   bool mNoteRepeat;
   Checkbox* mNoteRepeatCheckbox;
   NoteInterval mQuantizeInterval;
   DropdownList* mQuantizeIntervalSelector;
   std::array<InputInfo, 128> mInputInfos{};
   std::array<bool, 128> mScheduledOffs{};
   std::array<bool, 128> mPreScheduledOffs{};
   bool mHasReceivedPulse;
};