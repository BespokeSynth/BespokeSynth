//
//  SampleCanvas.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 6/24/15.
//
//

#include "SampleCanvas.h"
#include "IAudioSource.h"
#include "SynthGlobals.h"
#include "DrumPlayer.h"
#include "ModularSynth.h"
#include "CanvasControls.h"
#include "CanvasElement.h"
#include "Profiler.h"
#include "Sample.h"
#include "CanvasTimeline.h"
#include "CanvasScrollbar.h"

SampleCanvas::SampleCanvas()
: mCanvas(nullptr)
, mCanvasControls(nullptr)
, mNumMeasures(4)
, mInterval(kInterval_1n)
{
}

void SampleCanvas::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mNumMeasuresSlider = new IntSlider(this, "measures", 5, 5, 100, 15, &mNumMeasures, 1, 16);
   mIntervalSelector = new DropdownList(this, "interval", 110, 5, (int*)(&mInterval));

   mIntervalSelector->AddLabel("1n", kInterval_1n);
   mIntervalSelector->AddLabel("2n", kInterval_2n);
   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("4nt", kInterval_4nt);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("8nt", kInterval_8nt);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("16nt", kInterval_16nt);
   mIntervalSelector->AddLabel("32n", kInterval_32n);
   mIntervalSelector->AddLabel("64n", kInterval_64n);

   mCanvas = new Canvas(this, 5, 35, 790, 100, mNumMeasures, L(rows,4), L(cols,4), &(SampleCanvasElement::Create));
   AddUIControl(mCanvas);
   mCanvasControls = new CanvasControls();
   mCanvasControls->SetCanvas(mCanvas);
   mCanvasControls->CreateUIControls();
   AddChild(mCanvasControls);
   UpdateNumColumns();
   
   mCanvas->SetListener(this);

   mCanvasTimeline = new CanvasTimeline(mCanvas, "timeline");
   AddUIControl(mCanvasTimeline);

   mCanvasScrollbarHorizontal = new CanvasScrollbar(mCanvas, "scrollh", CanvasScrollbar::Style::kHorizontal);
   AddUIControl(mCanvasScrollbarHorizontal);

   mCanvasScrollbarVertical = new CanvasScrollbar(mCanvas, "scrollv", CanvasScrollbar::Style::kVertical);
   AddUIControl(mCanvasScrollbarVertical);
}

SampleCanvas::~SampleCanvas()
{
   mCanvas->SetListener(nullptr);
}

void SampleCanvas::Process(double time)
{
   PROFILER(SampleCanvas);
   
   if (!mEnabled || GetTarget() == nullptr)
      return;
   
   float canvasPos = GetCurPos(time);
   
   mCanvas->SetCursorPos(canvasPos);
   
   int bufferSize = GetTarget()->GetBuffer()->BufferSize();
   assert(bufferSize == gBufferSize);
   
   gWorkChannelBuffer.Clear();
   
   const vector<CanvasElement*>& elements = mCanvas->GetElements();
   for (int elemIdx = 0; elemIdx < elements.size(); ++elemIdx)
   {
      SampleCanvasElement* element = static_cast<SampleCanvasElement*>(elements[elemIdx]);
      Sample* clip = element->GetSample();
      float vol = element->GetVolume();
      if (clip == nullptr || element->IsMuted())
         continue;
      
      for (int i=0; i<bufferSize; ++i)
      {
         float sample = 0;
         
         float pos = GetCurPos(time + i * gInvSampleRateMs);
         
         sample = ofMap(pos, element->GetStart(), element->GetEnd(), 0, clip->LengthInSamples());
         
         sample *= vol;
         
         if (sample >= 0 && sample < clip->LengthInSamples())
         {
            for (int ch = 0; ch < GetTarget()->GetBuffer()->NumActiveChannels(); ++ch)
            {
               int sampleChannel = MAX(ch, clip->NumChannels() - 1);
               gWorkChannelBuffer.GetChannel(ch)[i] += GetInterpolatedSample(sample, clip->Data()->GetChannel(sampleChannel), clip->LengthInSamples());
            }
         }
      }
   }
   
   for (int ch = 0; ch < GetTarget()->GetBuffer()->NumActiveChannels(); ++ch)
   {
      ChannelBuffer* out = GetTarget(0)->GetBuffer();
      Add(out->GetChannel(ch), gWorkChannelBuffer.GetChannel(ch), gBufferSize);
      GetVizBuffer()->WriteChunk(gWorkChannelBuffer.GetChannel(ch), gBufferSize, ch);
   }
}

double SampleCanvas::GetCurPos(double time) const
{
   int loopMeasures = MAX(1, int(mCanvas->mLoopEnd - mCanvas->mLoopStart));
   return (((TheTransport->GetMeasure(time) % loopMeasures) + TheTransport->GetMeasurePos(time)) + mCanvas->mLoopStart) / mCanvas->GetLength();
}

void SampleCanvas::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);
   
   /*float canvasX,canvasY;
   mCanvas->GetPosition(canvasX, canvasY, true);
   if (y >= 0 && y < canvasY)
   {
      float pos = float(x - canvasX)/mCanvas->GetWidth() * mCanvas->GetNumCols();
      int measure = int(pos);
      TheTransport->SetMeasure(measure);
      TheTransport->SetMeasurePos(pos - measure);
   }*/
}

void SampleCanvas::CanvasUpdated(Canvas* canvas)
{
   if (canvas == mCanvas)
   {
      
   }
}

namespace
{
   const float extraW = 20;
   const float extraH = 163;
}

void SampleCanvas::Resize(float w, float h)
{
   w = MAX(w - extraW, 390);
   h = MAX(h - extraH, 40);
   mCanvas->SetDimensions(w, h);
}

void SampleCanvas::GetModuleDimensions(float& width, float& height)
{
   width = mCanvas->GetWidth() + extraW;
   height = mCanvas->GetHeight() + extraH;
}

void SampleCanvas::FilesDropped(vector<string> files, int x, int y)
{
   Sample sample;
   sample.Read(files[0].c_str());
   SampleDropped(x,y,&sample);
}

void SampleCanvas::SampleDropped(int x, int y, Sample* sample)
{
   CanvasCoord coord = mCanvas->GetCoordAt(x - mCanvas->GetPosition(true).x, y - mCanvas->GetPosition(true).y);
   coord.col = MAX(0,coord.col);
   coord.row = MAX(0,coord.row);
   SampleCanvasElement* element = static_cast<SampleCanvasElement*>(mCanvas->CreateElement(coord.col,coord.row));
   Sample* newSamp = new Sample();
   newSamp->CopyFrom(sample);
   element->SetSample(newSamp);
   double lengthMs = sample->LengthInSamples() / sample->GetSampleRateRatio() / gSampleRateMs;
   element->mLength = lengthMs / TheTransport->GetDuration(mInterval);
   mCanvas->AddElement(element);
   mCanvas->SelectElement(element);
}

void SampleCanvas::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mNumMeasuresSlider->Draw();
   mIntervalSelector->Draw();

   mCanvas->Draw();
   mCanvasTimeline->Draw();
   mCanvasScrollbarHorizontal->Draw();
   mCanvasScrollbarVertical->Draw();
   mCanvasControls->Draw();
}

void SampleCanvas::SetNumMeasures(int numMeasures)
{
   mNumMeasures = numMeasures;
   mCanvas->SetLength(mNumMeasures);
   mCanvas->SetNumCols(TheTransport->CountInStandardMeasure(mInterval) * mNumMeasures);
   if (mInterval < kInterval_8n)
      mCanvas->SetMajorColumnInterval(TheTransport->CountInStandardMeasure(mInterval));
   else
      mCanvas->SetMajorColumnInterval(TheTransport->CountInStandardMeasure(mInterval) / 4);
   mCanvas->mViewStart = 0;
   mCanvas->mViewEnd = mNumMeasures;
   mCanvas->mLoopStart = 0;
   mCanvas->mLoopEnd = mNumMeasures;
}

void SampleCanvas::UpdateNumColumns()
{
   mCanvas->RescaleNumCols(TheTransport->CountInStandardMeasure(mInterval) * mNumMeasures);
   if (mInterval < kInterval_8n)
      mCanvas->SetMajorColumnInterval(TheTransport->CountInStandardMeasure(mInterval));
   else
      mCanvas->SetMajorColumnInterval(TheTransport->CountInStandardMeasure(mInterval) / 4);
}

void SampleCanvas::CheckboxUpdated(Checkbox* checkbox)
{
}

void SampleCanvas::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void SampleCanvas::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mNumMeasuresSlider)
   {
      SetNumMeasures(mNumMeasures);
   }
}

void SampleCanvas::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mIntervalSelector)
   {
      UpdateNumColumns();
   }
}

void SampleCanvas::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void SampleCanvas::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}
