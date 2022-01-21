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
//  Compressor.h
//  modularSynth
//
//  Created by Ryan Challinor on 2/11/13.
//
//

#ifndef __modularSynth__Compressor__
#define __modularSynth__Compressor__

#include <iostream>
#include "IAudioEffect.h"
#include "Slider.h"
#include "Checkbox.h"
#include "RollingBuffer.h"

//-------------------------------------------------------------
// DC offset (to prevent denormal)
//-------------------------------------------------------------

// USE:
// 1. init envelope state to DC_OFFSET before processing
// 2. add to input before envelope runtime function
static const double DC_OFFSET = 1.0E-25;

//-------------------------------------------------------------
// envelope detector
//-------------------------------------------------------------
class EnvelopeDetector
{
	public:
		EnvelopeDetector(
                       double ms = 1.0
                       );
		virtual ~EnvelopeDetector() {}

		// time constant
		virtual void   setTc( double ms );
		virtual double getTc( void ) const { return ms_; }

		// runtime function
		void run( double in, double &state ) {
			state = in + coef_ * ( state - in );
		}

	protected:

		double ms_;				// time constant in ms
		double coef_;			// runtime coefficient
		virtual void setCoef( void );	// coef calculation

	};	// end SimpleComp class

	//-------------------------------------------------------------
	// attack/release envelope
	//-------------------------------------------------------------
	class AttRelEnvelope
	{
	public:
		AttRelEnvelope(
                     double att_ms = 10.0
                     , double rel_ms = 100.0
                     );
		virtual ~AttRelEnvelope() {}

		// attack time constant
		virtual void   setAttack( double ms );
		virtual double getAttack( void ) const { return att_.getTc(); }

		// release time constant
		virtual void   setRelease( double ms );
		virtual double getRelease( void ) const { return rel_.getTc(); }

		// runtime function
		void run( double in, double &state ) {

			/* assumes that:
          * positive delta = attack
          * negative delta = release
          * good for linear & log values
          */

			if ( in > state )
				att_.run( in, state );	// attack
			else
				rel_.run( in, state );	// release
		}
      
	private:
      
		EnvelopeDetector att_;
		EnvelopeDetector rel_;
		
};	// end AttRelEnvelope class

class Compressor : public IAudioEffect, public IFloatSliderListener
{
public:
   Compressor();
   
   static IAudioEffect* Create() { return new Compressor(); }
   
   
   void CreateUIControls() override;

   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   std::string GetType() override { return "compressor"; }

   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width=mWidth; height=mHeight; }
   bool Enabled() const override { return mEnabled; }

   float mMix;
   float mDrive;
   float mThreshold;
   float mRatio;
   float mAttack;
   float mRelease;
   float mLookahead;
   float mOutputAdjust;
   FloatSlider* mMixSlider;
   FloatSlider* mDriveSlider;
   FloatSlider* mThresholdSlider;
   FloatSlider* mRatioSlider;
   FloatSlider* mAttackSlider;
   FloatSlider* mReleaseSlider;
   FloatSlider* mLookaheadSlider;
   FloatSlider* mOutputAdjustSlider;
   
   double mCurrentInputDb;
   double mOutputGain;
   float mWidth;
   float mHeight;

   // runtime variables
   double envdB_;			// over-threshold envelope (dB)

   AttRelEnvelope mEnv;
   
   RollingBuffer mDelayBuffer;
};

#endif /* defined(__modularSynth__Compressor__) */

