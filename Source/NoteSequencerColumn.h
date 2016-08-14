//
//  NoteSequencerColumn.h
//  Bespoke
//
//  Created by Ryan Challinor on 4/12/16.
//
//

#ifndef __Bespoke__NoteSequencerColumn__
#define __Bespoke__NoteSequencerColumn__

#include "IDrawableModule.h"
#include "Slider.h"

class PatchCableSource;
class Grid;
class NoteStepSequencer;

class NoteSequencerColumn : public IDrawableModule, public IIntSliderListener
{
public:
   NoteSequencerColumn();
   ~NoteSequencerColumn();
   static IDrawableModule* Create() { return new NoteSequencerColumn(); }
   
   string GetTitleLabel() override { return "grid column"; }
   void CreateUIControls() override;
   
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
   //IPatchable
   void PostRepatch(PatchCableSource* cableSource) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(int& w, int& h) override;
   
   void SyncWithCable();
   
   IntSlider* mColumnSlider;
   int mColumn;
   IntSlider* mRowSlider;
   int mRow;
   
   NoteStepSequencer* mSequencer;
   Grid* mGrid;
   PatchCableSource* mGridCable;
};


#endif /* defined(__Bespoke__NoteSequencerColumn__) */
