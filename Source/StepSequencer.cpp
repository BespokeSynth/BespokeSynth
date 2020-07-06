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

namespace
{
   const int kMetaStepLoop = 8;
}

StepSequencer::StepSequencer()
: mGrid(nullptr)
, mStrength(1)
, mStrengthSlider(nullptr)
, mStochasticMode(false)
, mStochasticCheckbox(nullptr)
, mGridController(nullptr)
, mPreset(0)
, mPresetDropdown(nullptr)
, mColorOffset(3)
, mLpYOff(0)
, mLpYOffDropdown(nullptr)
, mAdjustOffsets(false)
, mAdjustOffsetsCheckbox(nullptr)
, mRepeatRate(kInterval_None)
, mRepeatRateDropdown(nullptr)
, mStepInterval(kInterval_16n)
, mStepIntervalDropdown(nullptr)
, mUseStrengthSliderCheckbox(nullptr)
, mUseStrengthSlider(false)
, mCurrentColumn(0)
, mCurrentColumnSlider(nullptr)
, mFlusher(this)
, mShiftLeftButton(nullptr)
, mShiftRightButton(nullptr)
{
   TheTransport->AddListener(this, mStepInterval, OffsetInfo(0, true), true);
   mFlusher.SetInterval(mStepInterval);
   
   mMetaStepMasks = new uint32[META_STEP_MAX * NUM_STEPSEQ_ROWS];
   for (int i=0; i<META_STEP_MAX * NUM_STEPSEQ_ROWS; ++i)
      mMetaStepMasks[i] = 0xff;
}

void StepSequencer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mGrid = new UIGrid(40,45,180,150,16,NUM_STEPSEQ_ROWS, this);
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
   mGridController = new GridController(this,"grid",240,4);
   mVelocityGridController = new GridController(this,"velocity",240,16);
   mMetaStepGridController = new GridController(this,"metastep",240,28);
   
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
   
   delete[] mMetaStepMasks;
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

namespace
{
   const float kMidwayVelocity = .75f;
}

void StepSequencer::UpdateLights()
{
   if (mGridController == nullptr)
      return;
   
   for (int x=0; x<mGridController->NumCols(); ++x)
   {
      for (int y=0; y<mGridController->NumRows(); ++y)
      {
         if (mGridController->IsMultisliderGrid())
         {
            Vec2i gridPos = ControllerToGrid(Vec2i(x,y));
            mGridController->SetLightDirect(x, y, (int)(mGrid->GetVal(gridPos.x,gridPos.y)*127));
         }
         else
         {
            GridColor color = GetGridColor(x, y);
            
            mGridController->SetLight(x, y, color);
         }
      }
   }
}

GridColor StepSequencer::GetGridColor(int x, int y)
{
   Vec2i gridPos = ControllerToGrid(Vec2i(x,y));
   bool cellOn = mGrid->GetVal(gridPos.x,gridPos.y) > 0;
   bool cellBright = mGrid->GetVal(gridPos.x,gridPos.y) > kMidwayVelocity;
   bool colOn = (mGrid->GetHighlightCol() == gridPos.x) && mEnabled;
   
   GridColor color;
   if (colOn)
   {
      if (cellBright)
         color = kGridColor3Bright;
      else if (cellOn)
         color = kGridColor3Dim;
      else
         color = kGridColor2Dim;
   }
   else
   {
      if (cellBright)
         color = kGridColor1Bright;
      else if (cellOn)
         color = kGridColor1Dim;
      else
         color = kGridColorOff;
   }
   
   return color;
}

void StepSequencer::UpdateVelocityLights()
{
   if (mVelocityGridController == nullptr)
      return;
   
   float stepVelocity = 0;
   if (mHeldButtons.size() > 0)
      stepVelocity = mGrid->GetVal(mHeldButtons.begin()->mCol, mHeldButtons.begin()->mRow);
   
   for (int x=0; x<mVelocityGridController->NumCols(); ++x)
   {
      for (int y=0; y<mVelocityGridController->NumRows(); ++y)
      {
         GridColor color;
         if (stepVelocity >= (8 - y) / 8.0f)
            color = kGridColor2Bright;
         else
            color = kGridColorOff;
         
         mVelocityGridController->SetLight(x, y, color);
      }
   }
}

void StepSequencer::UpdateMetaLights()
{
   if (mMetaStepGridController == nullptr)
      return;
   
   bool hasHeldButtons = mHeldButtons.size() > 0;
   uint32 metaStepMask = 0;
   if (hasHeldButtons)
      metaStepMask = mMetaStepMasks[GetMetaStepMaskIndex(mHeldButtons.begin()->mCol, mHeldButtons.begin()->mRow)];
   
   for (int x=0; x<mMetaStepGridController->NumCols(); ++x)
   {
      for (int y=0; y<mMetaStepGridController->NumRows(); ++y)
      {
         GridColor color;
         if (hasHeldButtons && (metaStepMask & (1 << x)))
            color = kGridColor1Bright;
         else if (!hasHeldButtons && x == GetMetaStep(gTime))
            color = kGridColor3Bright;
         else
            color = kGridColorOff;
         
         mMetaStepGridController->SetLight(x, y, color);
      }
   }
}

void StepSequencer::OnControllerPageSelected()
{
   UpdateLights();
}

void StepSequencer::OnGridButton(int x, int y, float velocity, IGridController* grid)
{
   if (grid == mGridController)
   {
      bool press = velocity > 0;
      if (x>=0 && y>=0)
      {
         Vec2i gridPos = ControllerToGrid(Vec2i(x,y));
         
         if (grid->IsMultisliderGrid())
         {
            mGrid->SetVal(gridPos.x, gridPos.y, velocity);
         }
         else
         {
            if (press)
            {
               mHeldButtons.push_back(HeldButton(gridPos.x, gridPos.y));
               float val = mGrid->GetVal(gridPos.x,gridPos.y);
               if (val == 0)
               {
                  float strength = 1;
                  if (mUseStrengthSlider)
                     strength = mStrength;
                  mGrid->SetVal(gridPos.x, gridPos.y, strength);
                  mHeldButtons.rbegin()->mTime = 0;
               }
            }
            else
            {
               double holdTime = 0;
               for (auto iter = mHeldButtons.begin(); iter != mHeldButtons.end(); ++iter)
               {
                  if (iter->mCol == gridPos.x && iter->mRow == gridPos.y)
                  {
                     holdTime = gTime - iter->mTime;
                     mHeldButtons.erase(iter);
                     break;
                  }
               }
               
               if (holdTime < 500)
               {
                  if (mUseStrengthSlider)
                  {
                     mGrid->SetVal(gridPos.x,gridPos.y,mStrength);
                  }
                  else
                  {
                     float val = mGrid->GetVal(gridPos.x,gridPos.y);
                     if (val > kMidwayVelocity)
                        mGrid->SetVal(gridPos.x,gridPos.y,kMidwayVelocity);
                     else if (val > 0)
                        mGrid->SetVal(gridPos.x,gridPos.y,0);
                     else
                        mGrid->SetVal(gridPos.x,gridPos.y,1);
                     //mGrid->SetVal(gridPos.x, gridPos.y, val > 0 ? 0 : 1);
                  }
               }
            }
         }
         
         UpdateLights();
         UpdateVelocityLights();
         UpdateMetaLights();
      }
   }
   
   if (grid == mVelocityGridController)
   {
      if (velocity > 0)
      {
         for (auto iter : mHeldButtons)
         {
            mGrid->SetVal(iter.mCol, iter.mRow, (8 - y) / 8.0f);
            iter.mTime = 0;
         }
         UpdateVelocityLights();
      }
   }
   
   if (grid == mMetaStepGridController)
   {
      if (velocity > 0)
      {
         for (auto iter : mHeldButtons)
         {
            mMetaStepMasks[GetMetaStepMaskIndex(iter.mCol,iter.mRow)] ^= 1 << x;
            iter.mTime = 0;
         }
         UpdateMetaLights();
      }
   }
}

int StepSequencer::GetStep(int step, int pitch)
{
   return ofClamp(mGrid->GetVal(step, pitch), 0, 1) * 127;
}

void StepSequencer::SetStep(int step, int pitch, int velocity)
{
   mGrid->SetVal(step, pitch, ofClamp(velocity / 127.0f, 0, 1));
   UpdateLights();
}

Vec2i StepSequencer::ControllerToGrid(const Vec2i& controller)
{
   if (mGridController == nullptr)
      return Vec2i(0,0);
   
   int numChunks = GetNumControllerChunks();
   int chunkSize = mGrid->GetRows() / numChunks;
   int col = controller.x + (controller.y/chunkSize)*mGridController->NumCols();
   int row = (chunkSize-1)-(controller.y%chunkSize)+mLpYOff*chunkSize;
   return Vec2i(col,row);
}

int StepSequencer::GetNumControllerChunks()
{
   if (mGridController == nullptr)
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
   mGridController->Draw();
   mVelocityGridController->Draw();
   mMetaStepGridController->Draw();
   
   float gridX, gridY;
   mGrid->GetPosition(gridX, gridY, true);
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
   
   ofPushStyle();
   ofFill();
   for (int col = 0; col < mGrid->GetCols(); ++col)
   {
      for (int row = 0; row < mGrid->GetRows(); ++row)
      {
         uint32 mask = mMetaStepMasks[GetMetaStepMaskIndex(col, row)];
         ofVec2f pos = mGrid->GetCellPosition(col, row) + mGrid->GetPosition(true);
         float cellWidth = (float)mGrid->GetWidth() / mGrid->GetCols();
         float cellHeight = (float)mGrid->GetHeight() / mGrid->GetRows();
         for (int i=0; i<kMetaStepLoop; ++i)
         {
            if (mask != 0xff)
            {
               float x = pos.x + ((i % 4) + 1.5f) * (cellWidth / 6);
               float y = pos.y + ((i / 4 + 1.5f) * (cellHeight / 4)) - cellHeight;
               float radius = cellHeight * .08f;
               
               if (i == GetMetaStep(gTime))
               {
                  ofSetColor(255, 220, 0);
                  ofCircle(x, y, radius * 1.5f);
               }
               
               if ((mask & (1 << i)) == 0)
                  ofSetColor(0, 0, 0);
               else
                  ofSetColor(255, 0, 0);
               
               ofCircle(x, y, radius);
            }
         }
      }
   }
   ofPopStyle();
}

void StepSequencer::DrawRowLabel(const char* label, int row, int x, int y)
{
   DrawTextLeftJustify(label, x, y+row*9.4f);
}

void StepSequencer::GetModuleDimensions(float& width, float& height)
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

bool StepSequencer::OnPush2Control(MidiMessageType type, int controlIndex, float midiValue)
{
   if (type == kMidiMessage_Note)
   {
      if (controlIndex >= 36 && controlIndex <= 99)
      {
         int gridIndex = controlIndex - 36;
         int gridX = gridIndex % 8;
         int gridY = 7 - gridIndex / 8;
         OnGridButton(gridX, gridY, midiValue/127, mGridController);
         return true;
      }
      if (controlIndex == 12) //touching pitchbend
      {
         mUseStrengthSlider = midiValue > 0;
      }
   }
   
   if (type == kMidiMessage_PitchBend)
   {
      if (midiValue != 8192)  //default value, happens on pitch bend release
      {
         float val = midiValue / 16320.0f;
         float oldStrength = mStrength;
         mStrength = val;
         FloatSliderUpdated(mStrengthSlider, oldStrength);
      }
      return true;
   }
   
   return false;
}

void StepSequencer::UpdatePush2Leds(Push2Control* push2)
{
   for (int x=0; x<8; ++x)
   {
      for (int y=0; y<8; ++y)
      {
         GridColor color = GetGridColor(x, y);
         int pushColor = 0;
         switch (color)
         {
            case kGridColorOff:  //off
               pushColor = 0; break;
            case kGridColor1Dim: //
               pushColor = 86; break;
            case kGridColor1Bright: //pressed
               pushColor = 32; break;
            case kGridColor2Dim:
               pushColor = 114; break;
            case kGridColor2Bright: //root
               pushColor = 25; break;
            case kGridColor3Dim: //not in pentatonic
               pushColor = 116; break;
            case kGridColor3Bright: //in pentatonic
               pushColor = 115; break;
         }
         push2->SetLed(kMidiMessage_Note, x + (7-y)*8 + 36, pushColor);
      }
   }
}

int StepSequencer::GetNumSteps(NoteInterval interval) const
{
   return TheTransport->CountInStandardMeasure(interval) * TheTransport->GetTimeSigTop()/TheTransport->GetTimeSigBottom() * mNumMeasures;
}

int StepSequencer::GetStepNum(double time)
{
   int measure = TheTransport->GetMeasure(time) % mNumMeasures;
   return TheTransport->GetQuantized(time, mStepInterval) + measure * GetNumSteps(mStepInterval) / mNumMeasures;
}

void StepSequencer::OnTimeEvent(double time)
{
   mGrid->SetGrid(GetNumSteps(mStepInterval),mNumRows);

   if (!mEnabled)
   {
      UpdateLights();
      return;
   }
 
   mCurrentColumn = GetStepNum(time);
   mGrid->SetHighlightCol(mCurrentColumn);
   UpdateLights();
   UpdateMetaLights();
}

void StepSequencer::PlayStepNote(double time, int note, float val)
{
   if (mStochasticMode)
   {
      if (val > ofRandom(1))
         mNoteOutput.PlayNote(time, note, val * 127);
   }
   else
   {
      mNoteOutput.PlayNote(time, note, val * 127);
   }
}

void StepSequencer::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mRepeatRate == kInterval_None)
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
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

int StepSequencer::GetMetaStep(double time)
{
   return TheTransport->GetMeasure(time) % kMetaStepLoop;
}

bool StepSequencer::IsMetaStepActive(double time, int col, int row)
{
   return mMetaStepMasks[GetMetaStepMaskIndex(col, row)] & (1 << GetMetaStep(time));
}

void StepSequencer::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(gTime);
}

void StepSequencer::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mStrengthSlider)
   {
      mGrid->SetStrength(mStrength);
      for (auto iter : mHeldButtons)
         mGrid->SetVal(iter.mCol, iter.mRow, mStrength);
      
      if (mHeldButtons.size() > 0)
         UpdateLights();
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
      UIGrid* oldGrid = new UIGrid(*mGrid);
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

void StepSequencer::KeyPressed(int key, bool isRepeat)
{
   IDrawableModule::KeyPressed(key, isRepeat);
   
   ofVec2f mousePos(TheSynth->GetMouseX(), TheSynth->GetMouseY());
   if (key >= '1' && key <= '8' && mGrid->GetRect().contains(mousePos.x, mousePos.y))
   {
      int metaStep = key - '1';
      auto cell = mGrid->GetGridCellAt(mousePos.x - mGrid->GetPosition().x, mousePos.y - mGrid->GetPosition().y);
      mMetaStepMasks[GetMetaStepMaskIndex(cell.mCol, cell.mRow)] ^= (1 << metaStep);
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
   mGrid->SetGridMode(multisliderMode ? UIGrid::kMultislider : UIGrid::kNormal);
   mGrid->SetRestrictDragToRow(multisliderMode);
   mGrid->SetClickClearsToZero(!multisliderMode);
}

namespace
{
   const int kSaveStateRev = 1;
}

void StepSequencer::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
   mGrid->SaveState(out);
   
   int numMetaStepMasks = META_STEP_MAX * NUM_STEPSEQ_ROWS;
   out << numMetaStepMasks;
   for (int i=0; i<numMetaStepMasks; ++i)
      out << mMetaStepMasks[i];
}

void StepSequencer::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);
   
   mGrid->LoadState(in);
   
   if (rev >= 1)
   {
      int numMetaStepMasks;
      in >> numMetaStepMasks;
      for (int i=0; i<numMetaStepMasks; ++i)
         in >> mMetaStepMasks[i];
   }
}

StepSequencerRow::StepSequencerRow(StepSequencer* seq, UIGrid* grid, int row)
: mSeq(seq)
, mGrid(grid)
, mRow(row)
, mOffset(0)
{
   TheTransport->AddListener(this, mSeq->GetStepInterval(), OffsetInfo(0, true), true);
}

StepSequencerRow::~StepSequencerRow()
{
   TheTransport->RemoveListener(this);
}

void StepSequencerRow::OnTimeEvent(double time)
{
   if (mSeq->Enabled() == false)
      return;
   
   float offsetMs = mOffset*TheTransport->MsPerBar();
   int step = mSeq->GetStepNum(time + offsetMs);
   float val = mGrid->GetVal(step,mRow);
   if (val > 0 && mSeq->IsMetaStepActive(time, step,mRow))
      mSeq->PlayStepNote(time, mRow, val * val);
}

void StepSequencerRow::SetOffset(float offset)
{
   mOffset = offset;
   UpdateTimeListener();
}

void StepSequencerRow::UpdateTimeListener()
{
   TheTransport->UpdateListener(this, mSeq->GetStepInterval(), OffsetInfo(mOffset, false));
}

NoteRepeat::NoteRepeat(StepSequencer* seq, int row)
: mSeq(seq)
, mRow(row)
, mOffset(0)
, mInterval(kInterval_None)
{
   TheTransport->AddListener(this, mInterval, OffsetInfo(0, true), true);
}

NoteRepeat::~NoteRepeat()
{
   TheTransport->RemoveListener(this);
}

void NoteRepeat::OnTimeEvent(double time)
{
   int pressure = mSeq->GetPadPressure(mRow);
   if (pressure > 10)
      mSeq->PlayStepNote(time, mRow, pressure / 85.0f);
}

void NoteRepeat::SetInterval(NoteInterval interval)
{
   mInterval = interval;
   TheTransport->UpdateListener(this, mInterval, OffsetInfo(mOffset, false));
}

void NoteRepeat::SetOffset(float offset)
{
   mOffset = offset;
   TheTransport->UpdateListener(this, mInterval, OffsetInfo(mOffset, false));
}

StepSequencerNoteFlusher::StepSequencerNoteFlusher(StepSequencer* seq)
: mSeq(seq)
{
   TheTransport->AddListener(this, mSeq->GetStepInterval(), OffsetInfo(.01f, false), true);
}

StepSequencerNoteFlusher::~StepSequencerNoteFlusher()
{
   TheTransport->RemoveListener(this);
}

void StepSequencerNoteFlusher::SetInterval(NoteInterval interval)
{
   TheTransport->UpdateListener(this, interval, OffsetInfo(.01f, false));
}

void StepSequencerNoteFlusher::OnTimeEvent(double time)
{
   mSeq->Flush(time);
}

