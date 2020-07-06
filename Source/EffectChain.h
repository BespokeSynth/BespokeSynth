//
//  EffectChain.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/25/12.
//
//

#ifndef __modularSynth__EffectChain__
#define __modularSynth__EffectChain__

#include <iostream>
#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "ClickButton.h"
#include "Slider.h"
#include "Checkbox.h"
#include "DropdownList.h"

#define MAX_EFFECTS_IN_CHAIN 100
#define MIN_EFFECT_WIDTH 80

class IAudioEffect;

class EffectChain : public IDrawableModule, public IAudioProcessor, public IButtonListener, public IFloatSliderListener, public IDropdownListener
{
public:
   EffectChain();
   virtual ~EffectChain();
   static IDrawableModule* Create() { return new EffectChain(); }
   
   string GetTitleLabel() override { return "effect chain"; }
   void CreateUIControls() override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void Init() override;
   void Poll() override;
   void AddEffect(string type, bool onTheFly = false);
   void SetWideCount(int count) { mNumFXWide = count; }
   
   //IAudioSource
   void Process(double time) override;
   
   void KeyPressed(int key, bool isRepeat) override;
   void KeyReleased(int key) override;

   void ButtonClicked(ClickButton* button) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   
   virtual void LoadBasics(const ofxJSONElement& moduleInfo, string typeName) override;
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   virtual void SaveLayout(ofxJSONElement& moduleInfo) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return mEnabled; }
   
   int GetRowHeight(int row);
   int NumRows() const;
   void DeleteLastEffect();
   
   vector<IAudioEffect*> mEffects;
   ChannelBuffer mDryBuffer;
   vector<ClickButton*> mMoveButtons;
   vector<FloatSlider*> mDryWetSliders;
   float mDryWetLevels[MAX_EFFECTS_IN_CHAIN];  //implicit max of 100 effects
   
   double mSwapTime;
   int mSwapFromIdx;
   int mSwapToIdx;
   ofVec2f mSwapFromPos;
   ofVec2f mSwapToPos;
   float mVolume;
   FloatSlider* mVolumeSlider;
   int mNumFXWide;
   bool mInitialized;
   bool mShowSpawnList;
   bool mWantDeleteLastEffect;
   
   std::vector<string> mEffectTypesToSpawn;
   int mSpawnIndex;
   DropdownList* mEffectSpawnList;
   ClickButton* mDeleteLastEffectButton;
   
   ofMutex mEffectMutex;
};

#endif /* defined(__modularSynth__EffectChain__) */
