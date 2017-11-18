/*
  ==============================================================================

    IModulator.h
    Created: 16 Nov 2017 9:59:15pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

class IModulator
{
public:
   IModulator() {}
   virtual ~IModulator() {}
   virtual float Value(int samplesIn = 0) = 0;
   virtual bool Active() const = 0;
   virtual float& GetMin() = 0;
   virtual float& GetMax() = 0;
};
