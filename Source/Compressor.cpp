//
//  Compressor.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 2/11/13.
//
//

#include "Compressor.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "Profiler.h"

static double lin2dB( double lin )
{
   static const double LOG_2_DB = 8.6858896380650365530225783783321;	// 20 / ln( 10 )
   return log( lin ) * LOG_2_DB;
}

static double dB2lin( double dB )
{
   static const double DB_2_LOG = 0.11512925464970228420089957273422;	// ln( 10 ) / 20
   return exp( dB * DB_2_LOG );
}

Compressor::Compressor()
: mThreshold(.5f)
, mRatio(.5f)
, mLookahead(3)
, mWindow(1)
, mAttack(.1f)
, mRelease(300)
, mThresholdSlider(NULL)
, mRatioSlider(NULL)
, mAttackSlider(NULL)
, mReleaseSlider(NULL)
{
   for (int i=0; i<ChannelBuffer::kMaxNumChannels; ++i)
      envdB_[i] = DC_OFFSET;
}

void Compressor::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mThresholdSlider = new FloatSlider(this,"threshold",5,8,110,15,&mThreshold,-40,0);
   mRatioSlider = new FloatSlider(this,"ratio",5,24,110,15,&mRatio,-1,1);
   mAttackSlider = new FloatSlider(this,"attack",5,40,110,15,&mAttack,.1f,50);
   mReleaseSlider = new FloatSlider(this,"release",5,56,110,15,&mRelease,.1f,500);
}

void Compressor::ProcessAudio(double time, ChannelBuffer* buffer)
{
   Profiler profiler("Compressor");

   if (!mEnabled)
      return;
   
   float bufferSize = buffer->BufferSize();

   ComputeSliders(0);

   for (int ch=0; ch<buffer->NumActiveChannels(); ++ch)
   {
      for (int i=0; i<bufferSize; ++i)
      {
         // create sidechain

         double rect1 = fabs( buffer->GetChannel(ch)[i] );	// rectify input
         double rect2 = fabs( buffer->GetChannel(ch)[i] );

         /* if desired, one could use another EnvelopeDetector to smooth
          * the rectified signal.
          */

         double link = std::max( rect1, rect2 );	// link channels with greater

         double keyLinked = link;

         // convert key to dB
         keyLinked += DC_OFFSET;				// add DC offset to avoid log( 0 )
         double keydB = lin2dB( keyLinked );	// convert linear -> dB

         // threshold
         double overdB = keydB - mThreshold;	// delta over threshold
         if ( overdB < 0.0 )
            overdB = 0.0;

         // attack/release

         overdB += DC_OFFSET;					// add DC offset to avoid denormal
         mEnv.run( overdB, envdB_[ch] );	// run attack/release envelope
         overdB = envdB_[ch] - DC_OFFSET;			// subtract DC offset

         /* REGARDING THE DC OFFSET: In this case, since the offset is added before
          * the attack/release processes, the envelope will never fall below the offset,
          * thereby avoiding denormals. However, to prevent the offset from causing
          * constant gain reduction, we must subtract it from the envelope, yielding
          * a minimum value of 0dB.
          */

         // transfer function
         double gr = overdB * ( mRatio - 1.0 );	// gain reduction (dB)
         gr = dB2lin( gr );						// convert dB -> linear

         // output gain
         buffer->GetChannel(ch)[i] *= gr;	// apply gain reduction to input
      }
   }
}

void Compressor::DrawModule()
{
   
   mThresholdSlider->Draw();
   mRatioSlider->Draw();
   mAttackSlider->Draw();
   mReleaseSlider->Draw();
}

void Compressor::CheckboxUpdated(Checkbox *checkbox)
{
}

void Compressor::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mAttackSlider)
      mEnv.setAttack(mAttack);
   if (slider == mReleaseSlider)
      mEnv.setRelease(mRelease);
}

//-------------------------------------------------------------
// envelope detector
//-------------------------------------------------------------
EnvelopeDetector::EnvelopeDetector( double ms )
{
   assert( ms > 0.0 );
   ms_ = ms;
   setCoef();
}

//-------------------------------------------------------------
void EnvelopeDetector::setTc( double ms )
{
   assert( ms > 0.0 );
   ms_ = ms;
   setCoef();
}

//-------------------------------------------------------------
void EnvelopeDetector::setCoef( void )
{
   coef_ = exp( -1000.0 / ( ms_ * gSampleRate ) );
}

//-------------------------------------------------------------
// attack/release envelope
//-------------------------------------------------------------
AttRelEnvelope::AttRelEnvelope( double att_ms, double rel_ms )
: att_( att_ms )
, rel_( rel_ms )
{
}

//-------------------------------------------------------------
void AttRelEnvelope::setAttack( double ms )
{
   att_.setTc( ms );
}

//-------------------------------------------------------------
void AttRelEnvelope::setRelease( double ms )
{
   rel_.setTc( ms );
}

