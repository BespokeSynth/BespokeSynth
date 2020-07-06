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

SampleCanvas::SampleCanvas()
: mCanvas(nullptr)
, mCanvasControls(nullptr)
{
}

void SampleCanvas::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mCanvas = new Canvas(this, 5, 15, 800, 100, L(length,1), L(rows,3), L(cols,4), &(SampleCanvasElement::Create));
   AddUIControl(mCanvas);
   mCanvasControls = new CanvasControls();
   mCanvasControls->SetCanvas(mCanvas);
   mCanvasControls->CreateUIControls();
   AddChild(mCanvasControls);
   
   mCanvas->SetListener(this);
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
   
   int numBars = mCanvas->GetNumCols();
   
   float canvasPos = ((TheTransport->GetMeasure(time) % numBars) + TheTransport->GetMeasurePos(time)) / numBars;
   
   mCanvas->SetCursorPos(canvasPos);
   
   int bufferSize = GetTarget()->GetBuffer()->BufferSize();
   float* out = GetTarget()->GetBuffer()->GetChannel(0);
   assert(bufferSize == gBufferSize);
   
   Clear(gWorkBuffer, bufferSize);
   
   const vector<CanvasElement*>& elements = mCanvas->GetElements();
   for (int elemIdx = 0; elemIdx < elements.size(); ++elemIdx)
   {
      SampleCanvasElement* element = static_cast<SampleCanvasElement*>(elements[elemIdx]);
      Sample* clip = element->GetSample();
      int numLoops = element->GetNumLoops();
      float vol = element->GetVolume();
      bool measureSync = element->ShouldMeasureSync();
      if (clip == nullptr)
         continue;
      
      for (int i=0; i<bufferSize; ++i)
      {
         float sample = 0;
         
         float pos = canvasPos + i / (TheTransport->MsPerBar()*gSampleRateMs*numBars);
         
         if (measureSync)
         {
            sample = ofMap(pos, element->GetStart(), element->GetEnd(), 0, clip->LengthInSamples() * numLoops);
         }
         else
         {
            sample = 0; //TODO(Ryan)
         }
         
         sample *= vol;
         
         //TODO(Ryan) multichannel
         if (sample >= 0 && sample < clip->LengthInSamples() * numLoops)
            gWorkBuffer[i] += GetInterpolatedSample(sample, clip->Data()->GetChannel(0), clip->LengthInSamples());
      }
   }
   
   Add(out, gWorkBuffer, bufferSize);
   GetVizBuffer()->WriteChunk(gWorkBuffer, bufferSize, 0);
}

void SampleCanvas::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);
   
   float canvasX,canvasY;
   mCanvas->GetPosition(canvasX, canvasY, true);
   if (y >= 0 && y < canvasY)
   {
      float pos = float(x - canvasX)/mCanvas->GetWidth() * mCanvas->GetNumCols();
      int measure = int(pos);
      TheTransport->SetMeasure(measure);
      TheTransport->SetMeasurePos(pos - measure);
   }
}

bool SampleCanvas::MouseScrolled(int x, int y, float scrollX, float scrollY)
{
   float canvasX,canvasY;
   mCanvas->GetPosition(canvasX, canvasY, true);
   ofVec2f canvasPos = ofVec2f(ofMap(x, canvasX, canvasX+mCanvas->GetWidth(), 0, 1),
                               ofMap(y, canvasY, canvasY+mCanvas->GetHeight(), 0, 1));
   if (IsInUnitBox(canvasPos))
   {
      float zoomCenter = ofLerp(mCanvas->mStart, mCanvas->mEnd, canvasPos.x);
      float distFromStart = zoomCenter - mCanvas->mStart;
      float distFromEnd = zoomCenter - mCanvas->mEnd;
      
      distFromStart *= 1 + scrollY/100;
      distFromEnd *= 1 + scrollY/100;
      
      float slideX = (mCanvas->mEnd - mCanvas->mStart) * -scrollX/300;
      
      mCanvas->mStart = ofClamp(zoomCenter - distFromStart + slideX, 0, 1);
      mCanvas->mEnd = ofClamp(zoomCenter - distFromEnd + slideX, 0, 1);
      return true;
   }
   return false;
}

void SampleCanvas::CanvasUpdated(Canvas* canvas)
{
   if (canvas == mCanvas)
   {
      
   }
}

void SampleCanvas::FilesDropped(vector<string> files, int x, int y)
{
   Sample sample;
   sample.Read(files[0].c_str());
   sample.SetNumBars(MAX(1, int(sample.LengthInSamples() / gSampleRate / 2 + .5f)));
   SampleDropped(x,y,&sample);
}

void SampleCanvas::SampleDropped(int x, int y, Sample* sample)
{
   CanvasCoord coord = mCanvas->GetCoordAt(x,y);
   coord.col = MAX(0,coord.col);
   coord.row = MAX(0,coord.row);
   SampleCanvasElement* element = static_cast<SampleCanvasElement*>(mCanvas->CreateElement(coord.col,coord.row));
   Sample* newSamp = new Sample();
   newSamp->Create(sample->Data());
   newSamp->SetNumBars(sample->GetNumBars());
   element->SetSample(newSamp);
   mCanvas->AddElement(element);
   mCanvas->SelectElement(element);
}

void SampleCanvas::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
   mCanvas->Draw();
   mCanvasControls->Draw();
}

void SampleCanvas::CheckboxUpdated(Checkbox* checkbox)
{
}

void SampleCanvas::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void SampleCanvas::IntSliderUpdated(IntSlider* slider, int oldVal)
{
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
