/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
//
//  FilterViz.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/24/14.
//
//

#include "FilterViz.h"
#include "ModularSynth.h"
#include "SynthGlobals.h"
#include "FFT.h"
#include "EffectFactory.h"

FilterViz::FilterViz()
{
   mImpulseBuffer = new float[FILTER_VIZ_BINS];
   mFFTOutReal = new float[FILTER_VIZ_BINS];
   mFFTOutImag = new float[FILTER_VIZ_BINS];
   Clear(mFFTOutReal, FILTER_VIZ_BINS);
   Clear(mFFTOutImag, FILTER_VIZ_BINS);

   for (int i = 0; i < 1; ++i)
   {
      IAudioEffect* filter = TheSynth->GetEffectFactory()->MakeEffect("eq");
      AddChild(filter);
      filter->SetPosition(4 + 100 * i, 20);
      filter->SetName(("filter " + ofToString(i)).c_str());
      mFilters.push_back(filter);
   }
}

void FilterViz::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   for (int i = 0; i < mFilters.size(); ++i)
      mFilters[i]->CreateUIControls();
}

FilterViz::~FilterViz()
{
   for (int i = 0; i < mFilters.size(); ++i)
      delete mFilters[i];
   delete mImpulseBuffer;
   delete mFFTOutReal;
   delete mFFTOutImag;
}

void FilterViz::Poll()
{
   mNeedUpdate = true;
   if (mNeedUpdate)
   {
      GraphFilter();
      mNeedUpdate = false;
   }
}

void FilterViz::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   for (int i = 0; i < mFilters.size(); ++i)
      mFilters[i]->Draw();

   float w, h;
   GetDimensions(w, h);

   ofPushStyle();
   {
      ofSetColor(255, 255, 255);
      ofPoint lastPoint(0, 0);
      for (int i = 0; i < FILTER_VIZ_BINS / 2 - 1; ++i)
      {
         float a = float(i) / (FILTER_VIZ_BINS / 2 - 1);
         a = sqrtf(a);
         ofPoint point(a * w, h - (mFFTOutReal[i] / 12 * h));
         if (i != 0)
            ofLine(lastPoint, point);
         lastPoint = point;
      }
   }

   {
      ofSetColor(255, 0, 255);
      ofPoint lastPoint(0, 0);
      for (int i = 0; i < FILTER_VIZ_BINS / 2 - 1; ++i)
      {
         float a = float(i) / (FILTER_VIZ_BINS / 2 - 1);
         a = sqrtf(a);
         ofPoint point(a * w, h / 2 - (mFFTOutImag[i] / (PI * 2) * h));
         if (i != 0)
            ofLine(lastPoint, point);
         lastPoint = point;
      }
   }

   ofPopStyle();
}

void FilterViz::GraphFilter()
{
   Clear(mImpulseBuffer, FILTER_VIZ_BINS);
   mImpulseBuffer[0] = 1;
   ChannelBuffer temp(mImpulseBuffer, FILTER_VIZ_BINS);
   for (int i = 0; i < mFilters.size(); ++i)
      mFilters[i]->ProcessAudio(gTime, &temp);
   ::FFT fft(FILTER_VIZ_BINS);
   fft.Forward(mImpulseBuffer, mFFTOutReal, mFFTOutImag);

   for (int j = 0; j < FILTER_VIZ_BINS / 2 + 1; ++j)
   {
      float real = mFFTOutReal[j];
      float imag = mFFTOutImag[j];

      //cartesian to polar
      float amp = 2. * sqrtf(real * real + imag * imag);
      float phase = atan2(imag, real);

      mFFTOutReal[j] = amp;
      mFFTOutImag[j] = phase;
   }
}

void FilterViz::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
}

void FilterViz::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void FilterViz::ButtonClicked(ClickButton* button, double time)
{
}
