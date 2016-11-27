//
//  KeyboardDisplay.h
//  Bespoke
//
//  Created by Ryan Challinor on 5/12/16.
//
//

#ifndef __Bespoke__KeyboardDisplay__
#define __Bespoke__KeyboardDisplay__

#include "IDrawableModule.h"
#include "NoteEffectBase.h"

class KeyboardDisplay : public NoteEffectBase, public IDrawableModule
{
public:
   KeyboardDisplay();
   static IDrawableModule* Create() { return new KeyboardDisplay(); }
   
   string GetTitleLabel() override { return "keyboard"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationChain* pitchBend = NULL, ModulationChain* modWheel = NULL, ModulationChain* pressure = NULL) override;
   
   void MouseReleased() override;
   void KeyPressed(int key, bool isRepeat) override;
   void KeyReleased(int key) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& width, int& height) override { width = mWidth; height = mHeight; }
   bool Enabled() const override { return mEnabled; }
   void OnClicked(int x, int y, bool right) override;
   
   void DrawKeyboard(int x, int y, int w, int h);
   ofRectangle GetKeyboardKeyRect(int pitch, int w, int h, bool& isBlackKey) const;
   
   int RootKey() const;
   int NumKeys() const;
   int GetPitchForTypingKey(int key) const;
   
   int mWidth;
   int mHeight;
   int mRootOctave;
   int mNumOctaves;
   int mPlayingMousePitch;
   
   bool mTypingInput;
   Checkbox* mTypingInputCheckbox;
};

#endif /* defined(__Bespoke__KeyboardDisplay__) */
