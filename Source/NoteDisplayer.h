//
//  NoteDisplayer.h
//  Bespoke
//
//  Created by Ryan Challinor on 6/17/15.
//
//

#ifndef __Bespoke__NoteDisplayer__
#define __Bespoke__NoteDisplayer__

#include "IDrawableModule.h"
#include "NoteEffectBase.h"

class NoteDisplayer : public NoteEffectBase, public IDrawableModule
{
public:
   NoteDisplayer();
   static IDrawableModule* Create() { return new NoteDisplayer(); }
   
   string GetTitleLabel() override { return "notedisplayer"; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationChain* pitchBend = nullptr, ModulationChain* modWheel = nullptr, ModulationChain* pressure = nullptr) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& width, int& height) override { width = 110; height = 60; }
   bool Enabled() const override { return true; }
   
   void DrawNoteName(int pitch, float y) const;
};

#endif /* defined(__Bespoke__NoteDisplayer__) */
