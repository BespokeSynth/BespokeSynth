//
//  VSTPlayhead.h
//  Bespoke
//
//  Created by Ryan Challinor on 1/20/16.
//
//

#ifndef __Bespoke__VSTPlayhead__
#define __Bespoke__VSTPlayhead__

#include "../JuceLibraryCode/JuceHeader.h"

class VSTPlayhead : public juce::AudioPlayHead
{
public:
   bool getCurrentPosition (CurrentPositionInfo& result);
};

#endif /* defined(__Bespoke__VSTPlayhead__) */
