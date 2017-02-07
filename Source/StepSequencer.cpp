//
//  StepSequencer.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/12/12.
//
//

#include "StepSequencer.h"
#include "IAudioSource.h"
#include "SynthGlobals.h"
#include "DrumPlayer.h"
#include "ModularSynth.h"
#include "MidiController.h"

StepSequencer::StepSequencer()
: mGrid(NULL)
, mStrength(1)
, mStrengthSlider(NULL)
, mStochasticMode(false)
, mStochasticCheckbox(NULL)
, mGridController(NULL)
, mPreset(0)
, mPresetDropdown(NULL)
, mColorOffset(3)
, mLpYOff(0)
, mLpYOffDropdown(NULL)
, mAdjustOffsets(false)
, mAdjustOffsetsCheckbox(NULL)
, mRepeatRate(kInterval_None)
, mRepeatRateDropdown(NULL)
, mHeldRow(-1)
, mHeldCol(-1)
, mStepInterval(kInterval_16n)
, mStepIntervalDropdown(NULL)
, mUseStrengthSliderCheckbox(NULL)
, mUseStrengthSlider(false)
, mCurrentColumn(0)
, mCurrentColumnSlider(NULL)
, mFlusher(this)
, mShiftLeftButton(NULL)
, mShiftRightButton(NULL)
{
   SetEnabled(false);
   
   TheTransport->AddListener(this, mStepInterval);
   mFlusher.SetInterval(mStepInterval);
}

void StepSequencer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mGrid = new Grid(40,45,180,150,16,NUM_STEPSEQ_ROWS);
   mStrengthSlider = new FloatSlider(this,"str",75,22,50,15,&mStrength,0,1,2);
   mUseStrengthSliderCheckbox = new Checkbox(this,"use str",128,22,&mUseStrengthSlider);
   mStochasticCheckbox = new Checkbox(this,"stch",145,22,&mStochasticMode);
   mPresetDropdown = new DropdownList(this,"preset",5,4,&mPreset);
   mLpYOffDropdown = new DropdownList(this,"yoff",190,22,&mLpYOff);
   mAdjustOffsetsCheckbox = new Checkbox(this,"offsets",175,4,&mAdjustOffsets);
   mRepeatRateDropdown = new DropdownList(this,"repeat",5,22,(int*)(&mRepeatRate));
   mStepIntervalDropdown = new DropdownList(this,"step",133,4,(int*)(&mStepInterval));
   mCurrentColumnSlider = new IntSlider(this,"column",HIDDEN_UICONTROL,HIDDEN_UICONTROL,100,15,&mCurrentColumn,0,15);
   mShiftLeftButton = new ClickButton(this,"<",80,4);
   mShiftRightButton = new ClickButton(this,">",100,4);
   
   mGrid->SetMajorColSize(4);
   mGrid->SetFlip(true);
   
   mPresetDropdown->AddLabel("16s", 0);
   mPresetDropdown->AddLabel("8s", 1);
   mPresetDropdown->AddLabel("kicksnare", 2);
   mPresetDropdown->AddLabel("amen", 3);
   mPresetDropdown->AddLabel("boogaloo", 4);
   mPresetDropdown->AddLabel("dubstep", 5);
   mPresetDropdown->AddLabel("trades", 6);
   mPresetDropdown->AddLabel("clear", -1);
   
   mLpYOffDropdown->AddLabel("0", 0);
   mLpYOffDropdown->AddLabel("1", 1);
   mLpYOffDropdown->AddLabel("2", 2);
   mLpYOffDropdown->AddLabel("3", 3);
   
   for (int i=0; i<NUM_STEPSEQ_ROWS; ++i)
   {
      mRows[i] = new StepSequencerRow(this, mGrid, i);
      mOffsets[i] = 0;
      mOffsetSlider[i] = new FloatSlider(this,("offset"+ofToString(i)).c_str(),230,185-i*9.4f,90,9,&mOffsets[i],-1,1);
      mNoteRepeats[i] = new NoteRepeat(this, i);
   }
   
   mRepeatRateDropdown->AddLabel("no repeat", kInterval_None);
   mRepeatRateDropdown->AddLabel("4n", kInterval_4n);
   mRepeatRateDropdown->AddLabel("4nt", kInterval_4nt);
   mRepeatRateDropdown->AddLabel("8n", kInterval_8n);
   mRepeatRateDropdown->AddLabel("8nt", kInterval_8nt);
   mRepeatRateDropdown->AddLabel("16n", kInterval_16n);
   mRepeatRateDropdown->AddLabel("16nt", kInterval_16nt);
   mRepeatRateDropdown->AddLabel("32n", kInterval_32n);
   mRepeatRateDropdown->AddLabel("32nt", kInterval_32nt);
   mRepeatRateDropdown->AddLabel("64n", kInterval_64n);
   
   mStepIntervalDropdown->AddLabel("4n", kInterval_4n);
   mStepIntervalDropdown->AddLabel("4nt", kInterval_4nt);
   mStepIntervalDropdown->AddLabel("8n", kInterval_8n);
   mStepIntervalDropdown->AddLabel("8nt", kInterval_8nt);
   mStepIntervalDropdown->AddLabel("16n", kInterval_16n);
   mStepIntervalDropdown->AddLabel("16nt", kInterval_16nt);
   mStepIntervalDropdown->AddLabel("32n", kInterval_32n);
   mStepIntervalDropdown->AddLabel("32nt", kInterval_32nt);
   mStepIntervalDropdown->AddLabel("64n", kInterval_64n);
   
   mUseStrengthSliderCheckbox->SetDisplayText(false);
}

StepSequencer::~StepSequencer()
{
   TheTransport->RemoveListener(this);
   
   for (int i=0; i<NUM_STEPSEQ_ROWS; ++i)
   {
      delete mRows[i];
      delete mNoteRepeats[i];
   }
}

void StepSequencer::Init()
{
   IDrawableModule::Init();
   
   SetPreset(rand() % 7);
}

void StepSequencer::Poll()
{
   IDrawableModule::Poll();
   
   ComputeSliders(0);
   
   if (mGridController)
   {
      int numChunks = GetNumControllerChunks();
      if (numChunks != mLpYOffDropdown->GetNumValues())
      {
         mLpYOffDropdown->Clear();
         for (int i=0; i<numChunks; ++i)
            mLpYOffDropdown->AddLabel(ofToString(i).c_str(), i);
      }
   }
}

void StepSequencer::UpdateLights()
{
   if (mGridController == NULL)
      return;
   
   for (int x=0; x<mGridController->NumCols(); ++x)
   {
      for (int y=0; y<mGridController->NumRows(); ++y)
      {
         Vec2i gridPos = ControllerToGrid(Vec2i(x,y));
         
         GridColor color = kGridColorOff;
         bool cellOn = mGrid->GetVal(gridPos.x,gridPos.y) > 0;
         bool colOn = (mGrid->GetHighlightCol() == gridPos.x) && mEnabled;
         if (mGridController->IsMultisliderGrid())
         {
            mGridController->SetLightDirect(x, y, (int)(mGrid->GetVal(gridPos.x,gridPos.y)*127));
         }
         else
         {
            if (cellOn)
               color = kGridColor1Bright;
            if (colOn)
               color = kGridColor2Dim;
            if (colOn && cellOn)
               color = kGridColor3Bright;
            
            mGridController->SetLight(x, y, color);
         }
      }
   }
}

void StepSequencer::ConnectGridController(IGridController* grid)
{
   if (mGridController == grid)
      return;
   
   if (mGridController)
      mGridController->SetTarget(NULL);
   
   assert(grid);
   mGridController = grid;
   mGridController->ResetLights();
}

void StepSequencer::OnGridButton(int x, int y, float velocity, IGridController* grid)
{
   bool press = velocity > 0;
   if (x>=0 && y>=0)
   {
      Vec2i gridPos = ControllerToGrid(Vec2i(x,y));
      
      if (mGridController &&
          mGridController->IsMultisliderGrid())
      {
         mGrid->SetVal(gridPos.x, gridPos.y, velocity);
      }
      else
      {
         if (press)
         {
            if (mUseStrengthSlider)
            {
               mGrid->SetVal(gridPos.x,gridPos.y,mStrength);
            }
            else
            {
               bool wasOn = mGrid->GetVal(gridPos.x,gridPos.y) > 0;
               if (!wasOn)
                  mGrid->SetVal(gridPos.x,gridPos.y,.5f);
               else if (wasOn)
                  mGrid->SetVal(gridPos.x,gridPos.y,0);
            }
         }
      }
      
      if (press)
      {
         mHeldCol = gridPos.x;
         mHeldRow = gridPos.y;
      }
      else
      {
         mHeldCol = -1;
         mHeldRow = -1;
      }
      
      UpdateLights();
   }
}

Vec2i StepSequencer::ControllerToGrid(const Vec2i& controller)
{
   if (mGridController == NULL)
      return Vec2i(0,0);
   
   int numChunks = GetNumControllerChunks();
   int chunkSize = mGrid->GetRows() / numChunks;
   int col = controller.x + (controller.y/chunkSize)*mGridController->NumCols();
   int row = (chunkSize-1)-(controller.y%chunkSize)+mLpYOff*chunkSize;
   return Vec2i(col,row);
}

int StepSequencer::GetNumControllerChunks()
{
   if (mGridController == NULL)
      return 1;
   
   int numBreaks = int((mGrid->GetCols() / MAX(1.0f,mGridController->NumCols())) + .5f);
   int numChunks = int(mGrid->GetRows() / MAX(1.0f,(mGridController->NumRows() / MAX(1,numBreaks)))+.5f);
   return numChunks;
}

void StepSequencer::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
   
   mGrid->Draw();
   mStrengthSlider->Draw();
   mUseStrengthSliderCheckbox->Draw();
   mStochasticCheckbox->Draw();
   mPresetDropdown->Draw();
   mAdjustOffsetsCheckbox->Draw();
   mRepeatRateDropdown->Draw();
   mLpYOffDropdown->Draw();
   mStepIntervalDropdown->Draw();
   mShiftLeftButton->Draw();
   mShiftRightButton->Draw();
   
   int gridX, gridY;
   mGrid->GetPosition(gridX, gridY);
   for (int i=0; i<NUM_STEPSEQ_ROWS; ++i)
   {
      if (i < mNumRows)
      {
         float y = gridY + mGrid->GetHeight() - (i+1) * (mGrid->GetHeight() / float(mNumRows));
         
         DrawTextLeftJustify(DrumPlayer::GetDrumHitName(i).c_str(), gridX - 7, y + 9);
         
         if (mAdjustOffsets)
         {
            mOffsetSlider[i]->SetShowing(true);
            mOffsetSlider[i]->SetPosition(gridX + mGrid->GetWidth() + 5, y);
            mOffsetSlider[i]->Draw();
         }
         else
         {
            mOffsetSlider[i]->SetShowing(false);
         }
      }
      else
      {
         mOffsetSlider[i]->SetShowing(false);
      }
   }

   if (mGridController)
   {
      ofPushStyle();
      ofNoFill();
      ofSetLineWidth(4);
      ofSetColor(255,0,0,50);
      float squareh = float(mGrid->GetHeight())/mNumRows;
      float squarew = float(mGrid->GetWidth())/GetNumSteps(mStepInterval);
      int chunkSize = mGrid->GetRows() / GetNumControllerChunks();
      float width = MIN(mGrid->GetWidth(), squarew * mGridController->NumCols() * GetNumControllerChunks());
      ofRect(gridX,gridY+squareh*(mNumRows-chunkSize)-squareh*mLpYOff*chunkSize,width,squareh*chunkSize);
      ofPopStyle();
   }
}

void StepSequencer::DrawRowLabel(const char* label, int row, int x, int y)
{
   DrawTextLeftJustify(label, x, y+row*9.4f);
}

void StepSequencer::GetModuleDimensions(int &width, int &height)
{
   width = mGrid->GetWidth() + 45;
   if (mAdjustOffsets)
      width += 100;
   height = mGrid->GetHeight() + 50;
}

void StepSequencer::Resize(float w, float h)
{
   float extraW = 45;
   float extraH = 50;
   if (mAdjustOffsets)
      extraW += 100;
   mGrid->SetDimensions(MAX(w - extraW, 185), MAX(h - extraH, 150));
}

void StepSequencer::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x,y,right);
   if (mGrid->TestClick(x,y,right))
      UpdateLights();
}

void StepSequencer::MouseReleased()
{
   IDrawableModule::MouseReleased();
   mGrid->MouseReleased();
   UpdateLights();
}

bool StepSequencer::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x,y);
   if (mGrid->NotifyMouseMoved(x,y))
      UpdateLights();
   return false;
}

int StepSequencer::GetNumSteps(NoteInterval interval) const
{
   return TheTransport->CountInStandardMeasure(interval) * TheTransport->GetTimeSigTop()/TheTransport->GetTimeSigBottom() * mNumMeasures;
}

int StepSequencer::GetStep(float offsetMs)
{
   int measure = TheTransport->GetMeasure() % mNumMeasures;
   return TheTransport->GetQuantized(offsetMs, mStepInterval) + measure * GetNumSteps(mStepInterval) / mNumMeasures;
}

void StepSequencer::OnTimeEvent(int samplesTo)
{
   mGrid->SetGrid(GetNumSteps(mStepInterval),mNumRows);

   if (!mEnabled)
   {
      UpdateLights();
      return;
   }
 
   mCurrentColumn = GetStep(0);
   mGrid->SetHighlightCol(mCurrentColumn);
   UpdateLights();
}

void StepSequencer::PlayNote(int note, float val)
{
   if (mStochasticMode)
   {
      if (val > ofRandom(1))
         mNoteOutput.PlayNote(gTime, note, val * 127);
   }
   else
   {
      mNoteOutput.PlayNote(gTime, note, val * 127);
   }
}

void StepSequencer::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= NULL*/, ModulationChain* modWheel /*= NULL*/, ModulationChain* pressure /*= NULL*/)
{
   if (mRepeatRate == kInterval_None)
      PlayNoteOutput(time, pitch, velocity, voiceIdx, pitchBend, modWheel, pressure);
}

void StepSequencer::SendPressure(int pitch, int pressure)
{
   mPadPressures[pitch] = pressure;
}

void StepSequencer::Exit()
{
   IDrawableModule::Exit();
   if (mGridController)
      mGridController->ResetLights();
}

void StepSequencer::SetPreset(int preset)
{
   mPreset = preset;
   
   mGrid->Clear();
   
   switch (preset)
   {
      case 0:
         for (int i=0;i<16;++i)
            mGrid->SetValRefactor(2,i,.5f);
         mGrid->SetValRefactor(0,0,.5f);
         mGrid->SetValRefactor(1,4,.5f);
         mGrid->SetValRefactor(0,8,.5f);
         mGrid->SetValRefactor(1,12,.5f);
         break;
      case 1:
         for (int i=0;i<16;i+=2)
            mGrid->SetValRefactor(2,i,.5f);
         mGrid->SetValRefactor(0,0,.5f);
         mGrid->SetValRefactor(1,4,.5f);
         mGrid->SetValRefactor(0,8,.5f);
         mGrid->SetValRefactor(1,12,.5f);
         break;
      case 2:
         mGrid->SetValRefactor(0,0,.5f);
         mGrid->SetValRefactor(0,4,.5f);
         mGrid->SetValRefactor(1,4,.5f);
         mGrid->SetValRefactor(0,8,.5f);
         mGrid->SetValRefactor(0,12,.5f);
         mGrid->SetValRefactor(1,12,.5f);
         break;
      case 3:  //amen
         mGrid->SetValRefactor(0,0,.5f);
         mGrid->SetValRefactor(6,0,.1f);
         mGrid->SetValRefactor(0,2,.5f);
         mGrid->SetValRefactor(6,2,.1f);
         mGrid->SetValRefactor(1,4,.5f);
         mGrid->SetValRefactor(2,6,.5f);
         mGrid->SetValRefactor(6,6,.1f);
         mGrid->SetValRefactor(4,7,.25f);
         mGrid->SetValRefactor(2,8,.5f);
         mGrid->SetValRefactor(6,8,.1f);
         mGrid->SetValRefactor(4,9,.25f);
         mGrid->SetValRefactor(0,10,.5f);
         mGrid->SetValRefactor(6,10,.1f);
         mGrid->SetValRefactor(1,12,.5f);
         mGrid->SetValRefactor(2,14,.5f);
         mGrid->SetValRefactor(6,15,.1f);
         break;
      case 4:
         for (int i=0;i<16;i+=2)
            mGrid->SetValRefactor(2,i,.5f);
         mGrid->SetValRefactor(0,0,.5f);
         mGrid->SetValRefactor(1,4,.5f);
         mGrid->SetValRefactor(1,7,.5f);
         mGrid->SetValRefactor(1,9,.5f);
         mGrid->SetValRefactor(0,10,.5f);
         mGrid->SetValRefactor(1,12,.5f);
         break;
      case 5:
         mGrid->SetValRefactor(0,0,.5f);
         mGrid->SetValRefactor(1,8,.5f);
         break;
      case 6:
         mGrid->SetValRefactor(0,0,.5f);
         mGrid->SetValRefactor(2,2,.5f);
         mGrid->SetValRefactor(0,4,.5f);
         mGrid->SetValRefactor(1,4,.5f);
         mGrid->SetValRefactor(2,6,.5f);
         mGrid->SetValRefactor(0,8,.5f);
         mGrid->SetValRefactor(2,10,.5f);
         mGrid->SetValRefactor(0,12,.5f);
         mGrid->SetValRefactor(1,12,.5f);
         mGrid->SetValRefactor(2,14,.5f);
         break;
   }
}

void StepSequencer::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush();
}

void StepSequencer::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mStrengthSlider)
   {
      mGrid->SetStrength(mStrength);
      if (mHeldRow != -1)
      {
         mGrid->SetValRefactor(mHeldRow, mHeldCol, mStrength);
         UpdateLights();
      }
   }
   for (int i=0; i<NUM_STEPSEQ_ROWS; ++i)
   {
      if (slider == mOffsetSlider[i])
      {
         float offset = -mOffsets[i]/32; //up to 1/32nd late or early
         mRows[i]->SetOffset(offset);
         mNoteRepeats[i]->SetOffset(offset);
         mGrid->SetDrawOffset(i, mOffsets[i]/2);
      }
   }
}

void StepSequencer::RadioButtonUpdated(RadioButton* radio, int oldVal)
{
}

void StepSequencer::IntSliderUpdated(IntSlider* slider, int oldVal)
{
}

void StepSequencer::ButtonClicked(ClickButton* button)
{
   if (button == mShiftLeftButton || button == mShiftRightButton)
   {
      int shift = (button == mShiftRightButton) ? 1 : -1;
      
      for (int row=0; row<mGrid->GetRows(); ++row)
      {
         int start = (shift == 1) ? mGrid->GetCols()-1 : 0;
         int end = (shift == 1) ? 0 : mGrid->GetCols()-1;
         float startVal = mGrid->GetVal(start, row);
         for (int col=start; col != end; col-=shift)
            mGrid->SetVal(col, row, mGrid->GetVal(col-shift,row));
         mGrid->SetVal(end, row, startVal);
      }
   }
}

void StepSequencer::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mPresetDropdown)
      SetPreset(mPreset);
   if (list == mRepeatRateDropdown)
   {
      for (int i=0; i<NUM_STEPSEQ_ROWS; ++i)
         mNoteRepeats[i]->SetInterval(mRepeatRate);
   }
   if (list == mStepIntervalDropdown)
   {
      Grid* oldGrid = new Grid(*mGrid);
      int oldNumSteps = GetNumSteps((NoteInterval)oldVal);
      int newNumSteps = GetNumSteps(mStepInterval);
      for (int i=0; i<mGrid->GetRows(); ++i)
      {
         for (int j=0; j<newNumSteps; ++j)
         {
            float div = j * ((float)oldNumSteps / newNumSteps);
            int col = (int)div;
            if (div == col)
               mGrid->SetValRefactor(i, j, oldGrid->GetValRefactor(i,col));
            else
               mGrid->SetValRefactor(i, j, 0);
         }
      }
      oldGrid->Delete();
      TheTransport->UpdateListener(this, mStepInterval);
      mFlusher.SetInterval(mStepInterval);
      mGrid->SetMajorColSize(TheTransport->CountInStandardMeasure(mStepInterval) / 4);
      for (int i=0; i<NUM_STEPSEQ_ROWS; ++i)
         mRows[i]->UpdateTimeListener();
   }
}

void StepSequencer::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   moduleInfo["gridwidth"] = mGrid->GetWidth();
   moduleInfo["gridheight"] = mGrid->GetHeight();
}

void StepSequencer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadInt("gridwidth", moduleInfo, 250, 150, 2000, true);
   mModuleSaveData.LoadInt("gridheight", moduleInfo, 150, 150, 2000, true);
   mModuleSaveData.LoadInt("gridrows", moduleInfo, 8, 1, NUM_STEPSEQ_ROWS);
   mModuleSaveData.LoadInt("gridmeasures", moduleInfo, 1, 1, 16);
   mModuleSaveData.LoadBool("multislider_mode", moduleInfo, true);

   SetUpFromSaveData();
}

void StepSequencer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mGrid->SetDimensions(mModuleSaveData.GetInt("gridwidth"), mModuleSaveData.GetInt("gridheight"));
   mNumRows = mModuleSaveData.GetInt("gridrows");
   mNumMeasures = mModuleSaveData.GetInt("gridmeasures");
   mGrid->SetGrid(mNumRows, GetNumSteps(mStepInterval));
   
   bool multisliderMode = mModuleSaveData.GetBool("multislider_mode");
   mGrid->SetGridMode(multisliderMode ? Grid::kMultislider : Grid::kNormal);
   mGrid->SetRestrictDragToRow(multisliderMode);
   mGrid->SetClickClearsToZero(!multisliderMode);
}

namespace
{
   const int kSaveStateRev = 0;
}

void StepSequencer::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
   mGrid->SaveState(out);
}

void StepSequencer::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);
   
   mGrid->LoadState(in);
}

StepSequencerRow::StepSequencerRow(StepSequencer* seq, Grid* grid, int row)
: mSeq(seq)
, mGrid(grid)
, mRow(row)
, mOffset(0)
{
   TheTransport->AddListener(this, mSeq->GetStepInterval());
}

StepSequencerRow::~StepSequencerRow()
{
   TheTransport->RemoveListener(this);
}

void StepSequencerRow::OnTimeEvent(int samplesTo)
{
   if (mSeq->Enabled() == false)
      return;
   
   float offsetMs = mOffset*TheTransport->MsPerBar();
   int step = mSeq->GetStep(offsetMs);
   float val = mGrid->GetValRefactor(mRow,step);
   if (val > 0)
      mSeq->PlayNote(mRow, val * val);
}

void StepSequencerRow::SetOffset(float offset)
{
   mOffset = offset;
   UpdateTimeListener();
}

void StepSequencerRow::UpdateTimeListener()
{
   TheTransport->UpdateListener(this, mSeq->GetStepInterval(), mOffset, false);
}

NoteRepeat::NoteRepeat(StepSequencer* seq, int row)
: mSeq(seq)
, mRow(row)
, mOffset(0)
, mInterval(kInterval_None)
{
   TheTransport->AddListener(this, mInterval);
}

NoteRepeat::~NoteRepeat()
{
   TheTransport->RemoveListener(this);
}

void NoteRepeat::OnTimeEvent(int samplesTo)
{
   int pressure = mSeq->GetPadPressure(mRow);
   if (pressure > 10)
      mSeq->PlayNote(mRow, pressure / 85.0f);
}

void NoteRepeat::SetInterval(NoteInterval interval)
{
   mInterval = interval;
   TheTransport->UpdateListener(this, mInterval, mOffset, false);
}

void NoteRepeat::SetOffset(float offset)
{
   mOffset = offset;
   TheTransport->UpdateListener(this, mInterval, mOffset, false);
}

StepSequencerNoteFlusher::StepSequencerNoteFlusher(StepSequencer* seq)
: mSeq(seq)
{
   TheTransport->AddListener(this, mSeq->GetStepInterval(), .01f, false);
}

StepSequencerNoteFlusher::~StepSequencerNoteFlusher()
{
   TheTransport->RemoveListener(this);
}

void StepSequencerNoteFlusher::SetInterval(NoteInterval interval)
{
   TheTransport->UpdateListener(this, interval, .01f, false);
}

void StepSequencerNoteFlusher::OnTimeEvent(int samplesTo)
{
   mSeq->Flush();
}

