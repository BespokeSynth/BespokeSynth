#pragma once

#include "IDrawableModule.h"
#include "IModulator.h"
#include "TextEntry.h"
#include <vector>

class PatchCableSource;

class MacroKnobs : public IDrawableModule, public ITextEntryListener
{
public:
   MacroKnobs();
   virtual ~MacroKnobs();
   static IDrawableModule* Create() { return new MacroKnobs(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void DrawModule() override;

   void OnClicked(float x, float y, bool right) override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;

   void TextEntryComplete(TextEntry* entry) override {}

   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   void GetModuleDimensions(float& width, float& height) override;

private:
   static const int kNumKnobs = 6;
   static const int kKnobRadius = 20;
   static const int kKnobSpacingX = 110;
   static const int kKnobSpacingY = 80;
   static const int kMarginX = 30;
   static const int kMarginY = 30;

   struct Knob : public IModulator
   {
   public:
      Knob(MacroKnobs* owner, int index);
      ~Knob();

      void CreateUIControls();
      void UpdateControl();

      // IModulator
      float Value(int samplesIn = 0) override;
      void Poll() override;
      bool Active() const override { return true; }
      bool CanAdjustRange() const override { return false; }

      MacroKnobs* mOwner{ nullptr };
      int mIndex{ 0 };
      float mValue{ 0.0f }; // 0 to 1
      std::string mLabel;

      PatchCableSource* GetCableSource() const { return mTargetCableSource; }
   };

   std::vector<Knob*> mKnobs;
   std::vector<TextEntry*> mLabels;

   int mHoveredKnob{ -1 };
   int mDraggingKnob{ -1 };
   float mDragStartY{ 0 };
   float mDragStartValue{ 0 };
};
