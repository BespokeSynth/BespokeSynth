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
/*
  ==============================================================================

    AbletonDeviceShared.h
    Created: 20 April 2025
    Author:  Ryan Challinor

  ==============================================================================
*/

//https://raw.githubusercontent.com/Ableton/push-interface/master/doc/MidiMapping.png

#include "leathers/push"
#include "leathers/unused-variable"
namespace AbletonDevice
{
   const int kTapTempoButton = 3;
   const int kMetronomeButton = 9;
   const int kBelowScreenButtonRow = 20;
   const int kMasterButton = 28;
   const int kStopClipButton = 29;
   const int kSetupButton = 30;
   const int kLayoutButton = 31;
   const int kConvertButton = 35;
   const int kQuantizeButtonSection = 36;
   const int kLeftButton = 44;
   const int kRightButton = 45;
   const int kUpButton = 46;
   const int kDownButton = 47;
   const int kSelectButton = 48;
   const int kShiftButton = 49;
   const int kNoteButton = 50;
   const int kSessionButton = 51;
   const int kAddDeviceButton = 52;
   const int kAddTrackButton = 53;
   const int kOctaveDownButton = 54;
   const int kOctaveUpButton = 55;
   const int kRepeatButton = 56;
   const int kAccentButton = 57;
   const int kScaleButton = 58;
   const int kUserButton = 59;
   const int kMuteButton = 60;
   const int kSoloButton = 61;
   const int kPageLeftButton = 62;
   const int kPageRightButton = 63;
   const int kCornerKnob = 79;
   const int kPlayButton = 85;
   const int kCircleButton = 86;
   const int kNewButton = 87;
   const int kDuplicateButton = 88;
   const int kAutomateButton = 89;
   const int kFixedLengthButton = 90;
   const int kAboveScreenButtonRow = 102;
   const int kDeviceButton = 110;
   const int kBrowseButton = 111;
   const int kMixButton = 112;
   const int kClipButton = 113;
   const int kQuantizeButton = 116;
   const int kDoubleLoopButton = 117;
   const int kDeleteButton = 118;
   const int kUndoButton = 119;

   const int kNumQuantizeButtons = 8;
}
#include "leathers/pop"
