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
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   void MouseReleased() override;
   void KeyPressed(int key, bool isRepeat) override;
   void KeyReleased(int key) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   bool Enabled() const override { return mEnabled; }
   void OnClicked(int x, int y, bool right) override;
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override { mWidth = w; mHeight = h; }
   
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
   bool mLatch;
   float mLastPlayedTime[128];
};

#endif /* defined(__Bespoke__KeyboardDisplay__) */
