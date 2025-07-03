/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2025 Ryan Challinor (contact: awwbees@gmail.com)

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
/*
  ==============================================================================

    LatencyCalculator.h
    Created: 25 Jun 2025
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "ClickButton.h"

class LatencyCalculatorReceiver;

class LatencyCalculatorSender : public IAudioSource, public IDrawableModule, public IButtonListener
{
public:
   LatencyCalculatorSender();
   virtual ~LatencyCalculatorSender();
   static IDrawableModule* Create() { return new LatencyCalculatorSender(); }

   void CreateUIControls() override;

   void ButtonClicked(ClickButton* button, double time) override;

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return true; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override
   {
      w = 100;
      h = 25;
   }

   ClickButton* mTestButton{ nullptr };

   float mPhase{ 0 };
   bool mTestRequested{ false };
   double mTestRequestTime{ 0.0 };
   double mTestStartTime{ 0.0 };

   LatencyCalculatorReceiver* mReceiver{ nullptr };
};

class LatencyCalculatorReceiver : public IAudioProcessor, public IDrawableModule
{
public:
   LatencyCalculatorReceiver();
   virtual ~LatencyCalculatorReceiver();
   static IDrawableModule* Create() { return new LatencyCalculatorReceiver(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void OnTestRequested();
   void OnTestStarted(double time, int samplesIn);

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return true; }

   enum class State
   {
      Init,
      TestRequested,
      Testing,
      DisplayResult,
      NoResult
   };

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override
   {
      w = 400;
      h = 35;
   }

   State mState{ State::Init };
   int mTestSamplesElapsed{ 0 };
   double mTestStartTime{ 0.0 };
   double mTestEndTime{ 0.0 };
};
