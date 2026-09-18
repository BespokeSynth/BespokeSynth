#pragma once

#include "IDrawableModule.h"
#include "IModulator.h"
#include <vector>

class PatchCableSource;

class MacroXY : public IDrawableModule
{
public:
   MacroXY();
   virtual ~MacroXY();
   static IDrawableModule* Create() { return new MacroXY(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void DrawModule() override;

   void OnClicked(float x, float y, bool right) override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;

   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   void GetModuleDimensions(float& width, float& height) override;

private:
   static const int kPadSize = 150;
   static const int kMarginX = 15;
   static const int kMarginY = 25;

   struct Axis : public IModulator
   {
   public:
      Axis(MacroXY* owner, int index, const std::string& label);
      ~Axis();

      void UpdateControl();

      // IModulator
      float Value(int samplesIn = 0) override;
      void Poll() override;
      bool Active() const override { return true; }
      bool CanAdjustRange() const override { return false; }

      MacroXY* mOwner{ nullptr };
      int mIndex{ 0 };
      float mValue{ 0.5f }; // 0 to 1
      std::string mLabel;
      PatchCableSource* mTargetCableSource{ nullptr };

      PatchCableSource* GetCableSource() const { return mTargetCableSource; }
   };

   Axis* mAxisX{ nullptr };
   Axis* mAxisY{ nullptr };

   bool mDragging{ false };
};
