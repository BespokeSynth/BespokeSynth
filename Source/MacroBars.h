#pragma once

#include "IDrawableModule.h"
#include "IModulator.h"
#include "Slider.h"
#include <vector>
#include <string>

class PatchCableSource;

class MacroBars : public IDrawableModule, public IIntSliderListener
{
public:
   MacroBars();
   virtual ~MacroBars();
   static IDrawableModule* Create() { return new MacroBars(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void DrawModule() override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override {}

   void OnClicked(float x, float y, bool right) override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;

   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   void GetModuleDimensions(float& width, float& height) override;

private:
   static const int kMaxFaders = 8;
   static const int kFaderWidth = 20;
   static const int kFaderHeight = 120;
   static const int kFaderSpacing = 15;
   static const int kMarginX = 15;
   static const int kMarginY = 30;

   struct Fader : public IModulator
   {
   public:
      Fader(MacroBars* owner, int index);
      ~Fader();

      void UpdateControl();

      // IModulator
      float Value(int samplesIn = 0) override;
      void Poll() override;
      bool Active() const override { return true; }
      bool CanAdjustRange() const override { return false; }

      MacroBars* mOwner{ nullptr };
      int mIndex{ 0 };
      float mValue{ 0.0f }; // 0 to 1
      PatchCableSource* mTargetCableSource{ nullptr };

      PatchCableSource* GetCableSource() const { return mTargetCableSource; }
   };

   IntSlider* mNumFadersSlider{ nullptr };
   int mNumFaders{ 4 };

   std::vector<Fader*> mFaders;

   int mDraggingFader{ -1 };
   int mHoveredFader{ -1 };
   float mDragStartY{ 0 };
   float mDragStartValue{ 0 };
};
