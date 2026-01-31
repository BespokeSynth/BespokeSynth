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

#pragma once

#include "AbletonMoveLCD.h"

class MidiDevice;

enum class AbletonDeviceType
{
   Push2,
   Move
};

//https://raw.githubusercontent.com/Ableton/push-interface/master/doc/MidiMapping.png

#include "leathers/push"
#include "leathers/unused-variable"
namespace AbletonDevice
{
   //push 2
   const int kMainEncoderTouchSection = 0;
   const int kMainEncoderSection = 71 + 128;
   const int kPadsSection = 36;
   const int kTapTempoButton = 3 + 128;
   const int kMetronomeButton = 9 + 128;
   const int kBelowScreenButtonRow = 20 + 128;
   const int kMasterButton = 28 + 128;
   const int kStopClipButton = 29 + 128;
   const int kSetupButton = 30 + 128;
   const int kLayoutButton = 31 + 128;
   const int kConvertButton = 35 + 128;
   const int kQuantizeButtonSection = 36 + 128;
   const int kLeftButton = 44 + 128;
   const int kRightButton = 45 + 128;
   const int kUpButton = 46 + 128;
   const int kDownButton = 47 + 128;
   const int kSelectButton = 48 + 128;
   const int kShiftButton = 49 + 128;
   const int kNoteButton = 50 + 128;
   const int kSessionButton = 51 + 128;
   const int kAddDeviceButton = 52 + 128;
   const int kAddTrackButton = 53 + 128;
   const int kOctaveDownButton = 54 + 128;
   const int kOctaveUpButton = 55 + 128;
   const int kRepeatButton = 56 + 128;
   const int kAccentButton = 57 + 128;
   const int kScaleButton = 58 + 128;
   const int kUserButton = 59 + 128;
   const int kMuteButton = 60 + 128;
   const int kSoloButton = 61 + 128;
   const int kPageLeftButton = 62 + 128;
   const int kPageRightButton = 63 + 128;
   const int kCornerKnob = 79 + 128;
   const int kPlayButton = 85 + 128;
   const int kCircleButton = 86 + 128;
   const int kNewButton = 87 + 128;
   const int kDuplicateButton = 88 + 128;
   const int kAutomateButton = 89 + 128;
   const int kFixedLengthButton = 90 + 128;
   const int kAboveScreenButtonRow = 102 + 128;
   const int kDeviceButton = 110 + 128;
   const int kBrowseButton = 111 + 128;
   const int kMixButton = 112 + 128;
   const int kClipButton = 113 + 128;
   const int kQuantizeButton = 116 + 128;
   const int kDoubleLoopButton = 117 + 128;
   const int kDeleteButton = 118 + 128;
   const int kUndoButton = 119 + 128;
   const int kClickyEncoderTurn = 14 + 128;
   const int kClickyEncoderTouch = 9;

   const int kNumQuantizeButtons = 8;
   const int kNumMainEncoders = 8;
   const int kNumPads = 64;
   const int kPitchBendIndex = 128 + 128;
   const int kChannelPressureIndex = kPitchBendIndex + 1; //reserve next 16 indices for channels
   const int kNumChannelPressureIndices = 16;

   //move
   const int kTrackButtonSection = 40 + 128;
   const int kStepButtonSection = 16;
   const int kMovePadsSection = 68;
   const int kVolumeEncoderTouch = 8;
   const int kVolumeEncoderTurn = 79 + 128;
   const int kHamburgerButton = 50 + 128;
   const int kBackButton = 51 + 128;
   const int kCaptureButton = 52 + 128;
   const int kLoopButton = 58 + 128;
   const int kMoveDeleteButton = 119 + 128;
   const int kMoveUndoButton = 56 + 128;
   const int kDotButton = 118 + 128;
   const int kMoveMuteButton = 88 + 128;
   const int kCopyButton = 60 + 128;
   const int kClickyEncoderButton = 3 + 128;

   const int kNumTrackButtons = 4;
   const int kNumStepButtons = 16;
   const int kNumMovePads = 32;

   const int kButtonNew = 16;
   const int kButtonSettings = 17;
   const int kButtonBranch = 18;
   const int kButtonDowel = 19;
   const int kButtonTempo = 20;
   const int kButtonMetronome = 21;
   const int kButtonGroove = 22;
   const int kButtonLayout = 23;
   const int kButtonScale = 24;
   const int kButtonFullVelocity = 25;
   const int kButtonRepeat = 26;
   const int kButtonChord = 27;
   const int kButtonStar = 28;
   const int kButtonAdd = 29;
   const int kButtonDuplicate = 30;
   const int kButtonQuantize = 31;

   const int kStepButtonLedSection = 16 + 128;
   const int kLedNew = 16 + 128;
   const int kLedSettings = 17 + 128;
   const int kLedBranch = 18 + 128;
   const int kLedDowel = 19 + 128;
   const int kLedTempo = 20 + 128;
   const int kLedMetronome = 21 + 128;
   const int kLedGroove = 22 + 128;
   const int kLedLayout = 23 + 128;
   const int kLedScale = 24 + 128;
   const int kLedFullVelocity = 25 + 128;
   const int kLedRepeat = 26 + 128;
   const int kLedChord = 27 + 128;
   const int kLedStar = 28 + 128;
   const int kLedAdd = 29 + 128;
   const int kLedDuplicate = 30 + 128;
   const int kLedQuantize = 31 + 128;

   const int kColorOff = 0;
   const int kColorLightCoral = 1;
   const int kColorDeepRed = 2;
   const int kColorOrange = 3;
   const int kColorBurntOrange = 4;
   const int kColorLightAmber = 5;
   const int kColorGoldenOrange = 6;
   const int kColorLightHoney = 7;
   const int kColorYellowGold = 8;
   const int kColorLemonYellow = 9;
   const int kColorLightGreen = 10;
   const int kColorWintergreen = 11;
   const int kColorOliveGreen = 12;
   const int kColorMintGreen = 13;
   const int kColorTeal = 14;
   const int kColorAquaBlue = 15;
   const int kColorSkyBlue = 16;
   const int kColorLightBlue = 17;
   const int kColorRichBlue = 18;
   const int kColorPurpleBlue = 19;
   const int kColorBrightBlue = 20;
   const int kColorPaleIndigo = 21;
   const int kColorPurple = 22;
   const int kColorViolet = 23;
   const int kColorPink = 24;
   const int kColorBrightPink = 25;
   const int kColorBubblegum = 26;
   const int kColorDustyRed = 27;
   const int kColorSoftPeach = 28;
   const int kColorMustard = 29;
   const int kColorPaleGold = 30;
   const int kColorOlive = 31;
   const int kColorBrightGreen = 32;
   const int kColorJeanBlue = 33;
   const int kColorLightPurple = 34;
   const int kColorPinkishPurple = 35;
   const int kColorBrightPeach = 36;
   const int kColorPeachPink = 37;
   const int kColorLightPeach = 38;
   const int kColorBlushPink = 39;
   const int kColorLightCream = 40;
   const int kColorSoftRose = 41;
   const int kColorLightBeige = 42;
   const int kColorIcyBlue = 43;
   const int kColorPaleBlue = 44;
   const int kColorLavenderBlue = 45;
   const int kColorPowderBlue = 46;
   const int kColorGlacierBlue = 47;
   const int kColorPeriwinkle = 48;
   const int kColorPaleLavender = 49;
   const int kColorLavenderMist = 50;
   const int kColorLilac = 51;
   const int kColorLightViolet = 52;
   const int kColorDustyPink = 53;
   const int kColorMauve = 54;
   const int kColorFreshWhite = 55;
   const int kColorIvory = 56;
   const int kColorLilacBlue = 57;
   const int kColorSoftPurple = 58;
   const int kColorPeachyPink = 59;
   const int kColorDustyLavender = 60;
   const int kColorPinkRose = 61;
   const int kColorRosePink = 62;
   const int kColorPalePink = 63;
   const int kColorLightLilac = 64;
   const int kColorSoftPink = 65;
   const int kColorDarkRed = 66;
   const int kColorSoftRed = 67;
   const int kColorDimRed = 68;
   const int kColorPumpkinOrange = 69;
   const int kColorBurntSienna = 70;
   const int kColorCitrusOrange = 71;
   const int kColorCopperBrown = 72;
   const int kColorWoodBrown = 73;
   const int kColorCaramel = 74;
   const int kColorRustyBrown = 75;
   const int kColorDarkBrown = 76;
   const int kColorGoldenPeach = 77;
   const int kColorDimYellow = 78;
   const int kColorSunsetYellow = 79;
   const int kColorGoldenAmber = 80;
   const int kColorLimeGreen = 81;
   const int kColorPickleGreen = 82;
   const int kColorGreenOlive = 83;
   const int kColorFreshGreen = 84;
   const int kColorLime = 85;
   const int kColorMossGreen = 86;
   const int kColorYellowGreen = 87;
   const int kColorYellowOlive = 88;
   const int kColorLightAqua = 89;
   const int kColorDarkGreen = 90;
   const int kColorIceBlue = 91;
   const int kColorDarkCyan = 92;
   const int kColorElectricSky = 93;
   const int kColorSeaBlue = 94;
   const int kColorCoolBlue = 95;
   const int kColorFadedBlue = 96;
   const int kColorBlueEyes = 97;
   const int kColorStormyBlue = 98;
   const int kColorElectricBlue = 99;
   const int kColorDarkNavy = 100;
   const int kColorMidnightBlue = 101;
   const int kColorDarkestBlue = 102;
   const int kColorCalmBlue = 103;
   const int kColorDuskyBlue = 104;
   const int kColorBrightPurple = 105;
   const int kColorVividBlue = 106;
   const int kColorNeonPurple = 107;
   const int kColorPurpleViolet = 108;
   const int kColorElectricViolet = 109;
   const int kColorDimPurple = 110;
   const int kColorDeepPink = 111;
   const int kColorDimPink = 112;
   const int kColorNeonPink = 113;
   const int kColorPinkRed = 114;
   const int kColorBrightMagenta = 115;
   const int kColorPaleMagenta = 116;
   const int kColorBlack = 117;
   const int kColorPaleWhite = 118;
   const int kColorGrey = 119;
   const int kColorWhite = 120;
   const int kColorLightGrey = 121;
   const int kColorSnowWhite = 122;
   const int kColorAshWhite = 123;
   const int kColorDarkGrey = 124;
   const int kColorBlue = 125;
   const int kColorGreen = 126;
   const int kColorRed = 127;

   struct AbletonColor
   {
      AbletonColor(int _index, std::string _name, ofColor _color)
      : index(_index)
      , name(_name)
      , color(_color)
      {
      }
      int index{ 0 };
      std::string name{};
      ofColor color{};
   };

   const std::array<AbletonColor, 128> kColors = {
      AbletonColor(0, "off", ofColor(0, 0, 0)),
      AbletonColor(1, "light coral", ofColor(255, 100, 100)),
      AbletonColor(2, "deep red", ofColor(225, 25, 6)),
      AbletonColor(3, "orange", ofColor(255, 120, 25)),
      AbletonColor(4, "burnt orange", ofColor(255, 75, 25)),
      AbletonColor(5, "light amber", ofColor(255, 150, 75)),
      AbletonColor(6, "golden orange", ofColor(221, 100, 25)),
      AbletonColor(7, "light honey", ofColor(255, 200, 111)),
      AbletonColor(8, "yellow gold", ofColor(255, 200, 64)),
      AbletonColor(9, "lemon yellow", ofColor(225, 237, 59)),
      AbletonColor(10, "light green", ofColor(175, 241, 75)),
      AbletonColor(11, "wintergreen", ofColor(100, 241, 75)),
      AbletonColor(12, "olive green", ofColor(175, 219, 50)),
      AbletonColor(13, "mint green", ofColor(150, 255, 150)),
      AbletonColor(14, "teal", ofColor(100, 200, 176)),
      AbletonColor(15, "aqua blue", ofColor(75, 220, 220)),
      AbletonColor(16, "sky blue", ofColor(50, 150, 255)),
      AbletonColor(17, "light blue", ofColor(65, 130, 255)),
      AbletonColor(18, "rich blue", ofColor(70, 80, 255)),
      AbletonColor(19, "purple blue", ofColor(80, 0, 255)),
      AbletonColor(20, "bright blue", ofColor(20, 88, 255)),
      AbletonColor(21, "pale indigo", ofColor(105, 81, 255)),
      AbletonColor(22, "purple", ofColor(126, 25, 255)),
      AbletonColor(23, "violet", ofColor(177, 25, 255)),
      AbletonColor(24, "pink", ofColor(225, 50, 225)),
      AbletonColor(25, "bright pink", ofColor(255, 50, 102)),
      AbletonColor(26, "bubblegum", ofColor(255, 90, 230)),
      AbletonColor(27, "dusty red", ofColor(255, 100, 100)),
      AbletonColor(28, "soft peach", ofColor(225, 138, 110)),
      AbletonColor(29, "mustard", ofColor(225, 175, 25)),
      AbletonColor(30, "pale gold", ofColor(225, 195, 98)),
      AbletonColor(31, "olive", ofColor(172, 200, 25)),
      AbletonColor(32, "bright green", ofColor(50, 200, 75)),
      AbletonColor(33, "jean blue", ofColor(25, 105, 255)),
      AbletonColor(34, "light purple", ofColor(190, 115, 248)),
      AbletonColor(35, "pinkish purple", ofColor(204, 100, 200)),
      AbletonColor(36, "bright peach", ofColor(255, 197, 245)),
      AbletonColor(37, "peach pink", ofColor(255, 200, 190)),
      AbletonColor(38, "light peach", ofColor(255, 200, 150)),
      AbletonColor(39, "blush pink", ofColor(255, 200, 178)),
      AbletonColor(40, "light cream", ofColor(255, 217, 197)),
      AbletonColor(41, "soft rose", ofColor(235, 200, 196)),
      AbletonColor(42, "light beige", ofColor(225, 200, 200)),
      AbletonColor(43, "icy blue", ofColor(216, 225, 225)),
      AbletonColor(44, "pale blue", ofColor(175, 217, 225)),
      AbletonColor(45, "lavender blue", ofColor(225, 218, 225)),
      AbletonColor(46, "powder blue", ofColor(175, 224, 255)),
      AbletonColor(47, "glacier blue", ofColor(121, 175, 255)),
      AbletonColor(48, "periwinkle", ofColor(125, 175, 255)),
      AbletonColor(49, "pale lavender", ofColor(175, 150, 255)),
      AbletonColor(50, "lavender mist", ofColor(201, 161, 255)),
      AbletonColor(51, "lilac", ofColor(225, 175, 255)),
      AbletonColor(52, "light violet", ofColor(255, 150, 225)),
      AbletonColor(53, "dusty pink", ofColor(211, 200, 228)),
      AbletonColor(54, "mauve", ofColor(200, 175, 206)),
      AbletonColor(55, "fresh white", ofColor(205, 205, 225)),
      AbletonColor(56, "ivory", ofColor(225, 225, 235)),
      AbletonColor(57, "lilac blue", ofColor(200, 175, 249)),
      AbletonColor(58, "soft purple", ofColor(200, 150, 225)),
      AbletonColor(59, "peachy pink", ofColor(210, 155, 229)),
      AbletonColor(60, "dusty lavender", ofColor(199, 150, 212)),
      AbletonColor(61, "pink rose", ofColor(215, 187, 200)),
      AbletonColor(62, "rose pink", ofColor(200, 170, 197)),
      AbletonColor(63, "pale pink", ofColor(225, 151, 221)),
      AbletonColor(64, "light lilac", ofColor(255, 225, 255)),
      AbletonColor(65, "soft pink", ofColor(255, 100, 125)),
      AbletonColor(66, "dark red", ofColor(178, 50, 67)),
      AbletonColor(67, "soft red", ofColor(255, 50, 25)),
      AbletonColor(68, "dim red", ofColor(225, 35, 0)),
      AbletonColor(69, "pumpkin orange", ofColor(255, 100, 25)),
      AbletonColor(70, "burnt sienna", ofColor(175, 75, 24)),
      AbletonColor(71, "citrus orange", ofColor(229, 73, 24)),
      AbletonColor(72, "copper brown", ofColor(174, 50, 2)),
      AbletonColor(73, "wood brown", ofColor(200, 150, 125)),
      AbletonColor(74, "caramel", ofColor(150, 100, 89)),
      AbletonColor(75, "rusty brown", ofColor(200, 75, 50)),
      AbletonColor(76, "dark brown", ofColor(125, 50, 0)),
      AbletonColor(77, "golden peach", ofColor(225, 200, 150)),
      AbletonColor(78, "dim yellow", ofColor(145, 135, 75)),
      AbletonColor(79, "sunset yellow", ofColor(255, 200, 75)),
      AbletonColor(80, "golden amber", ofColor(150, 100, 25)),
      AbletonColor(81, "lime green", ofColor(200, 225, 76)),
      AbletonColor(82, "pickle green", ofColor(134, 148, 31)),
      AbletonColor(83, "green olive", ofColor(175, 225, 125)),
      AbletonColor(84, "fresh green", ofColor(105, 138, 75)),
      AbletonColor(85, "lime", ofColor(100, 200, 100)),
      AbletonColor(86, "moss green", ofColor(75, 125, 25)),
      AbletonColor(87, "yellow green", ofColor(150, 150, 28)),
      AbletonColor(88, "yellow olive", ofColor(100, 93, 25)),
      AbletonColor(89, "light aqua", ofColor(125, 225, 200)),
      AbletonColor(90, "dark green", ofColor(75, 125, 100)),
      AbletonColor(91, "ice blue", ofColor(88, 152, 225)),
      AbletonColor(92, "dark cyan", ofColor(75, 100, 125)),
      AbletonColor(93, "electric sky", ofColor(50, 235, 255)),
      AbletonColor(94, "sea blue", ofColor(25, 100, 101)),
      AbletonColor(95, "cool blue", ofColor(50, 150, 255)),
      AbletonColor(96, "faded blue", ofColor(25, 75, 200)),
      AbletonColor(97, "blue eyes", ofColor(50, 100, 255)),
      AbletonColor(98, "stormy blue", ofColor(25, 50, 200)),
      AbletonColor(99, "electric blue", ofColor(15, 50, 255)),
      AbletonColor(100, "dark navy", ofColor(25, 25, 225)),
      AbletonColor(101, "midnight blue", ofColor(50, 0, 255)),
      AbletonColor(102, "darkest blue", ofColor(49, 0, 190)),
      AbletonColor(103, "calm blue", ofColor(34, 82, 255)),
      AbletonColor(104, "dusky blue", ofColor(25, 50, 200)),
      AbletonColor(105, "bright purple", ofColor(75, 60, 255)),
      AbletonColor(106, "vivid blue", ofColor(50, 25, 223)),
      AbletonColor(107, "neon purple", ofColor(136, 25, 255)),
      AbletonColor(108, "purple violet", ofColor(75, 25, 200)),
      AbletonColor(109, "electric violet", ofColor(200, 51, 255)),
      AbletonColor(110, "dim purple", ofColor(125, 25, 204)),
      AbletonColor(111, "deep pink", ofColor(200, 63, 244)),
      AbletonColor(112, "dim pink", ofColor(125, 25, 150)),
      AbletonColor(113, "neon pink", ofColor(255, 50, 145)),
      AbletonColor(114, "pink red", ofColor(200, 25, 75)),
      AbletonColor(115, "bright magenta", ofColor(255, 50, 255)),
      AbletonColor(116, "pale magenta", ofColor(175, 25, 175)),
      AbletonColor(117, "black", ofColor(0, 0, 0)),
      AbletonColor(118, "pale white", ofColor(240, 240, 240)),
      AbletonColor(119, "grey", ofColor(100, 100, 100)),
      AbletonColor(120, "white", ofColor(255, 255, 255)),
      AbletonColor(121, "light grey", ofColor(150, 150, 150)),
      AbletonColor(122, "snow white", ofColor(240, 240, 240)),
      AbletonColor(123, "ash white", ofColor(160, 160, 160)),
      AbletonColor(124, "dark grey", ofColor(100, 100, 100)),
      AbletonColor(125, "blue", ofColor(0, 0, 255)),
      AbletonColor(126, "green", ofColor(0, 225, 0)),
      AbletonColor(127, "red", ofColor(255, 0, 0))
   };

   static int GetPadColorForType(ModuleCategory type, bool enabled)
   {
      int color;
      switch (type)
      {
         case kModuleCategory_Instrument:
            color = enabled ? 26 : 116;
            break;
         case kModuleCategory_Note:
            color = enabled ? 8 : 80;
            break;
         case kModuleCategory_Synth:
            color = enabled ? 11 : 86;
            break;
         case kModuleCategory_Audio:
            color = enabled ? 18 : 96;
            break;
         case kModuleCategory_Modulator:
            color = enabled ? 22 : 110;
            break;
         case kModuleCategory_Pulse:
            color = enabled ? 9 : 82;
            break;
         default:
            color = enabled ? 118 : 119;
            break;
      }
      return color;
   }

   static float GetEncoderIncrement(float midiInputValue)
   {
      float increment = midiInputValue < 64 ? midiInputValue : midiInputValue - 128;
      return increment * .005f;
   }
}
#include "leathers/pop"

class IAbletonGridDevice
{
public:
   virtual void SetLed(int index, int color, int flashColor = -1) = 0;
   virtual bool GetButtonState(int index) const = 0;
   virtual int GetGridControllerOption1Control() const = 0;
   virtual int GetGridControllerOption2Control() const = 0;
   virtual MidiDevice* GetDevice() = 0;
   virtual void SetDisplayModule(IDrawableModule* module, bool addToHistory = true) = 0;
   virtual IDrawableModule* GetDisplayModule() const = 0;
   virtual AbletonDeviceType GetAbletonDeviceType() const = 0;
   virtual int GetGridStartIndex() const = 0;
   virtual int GetGridNumPads() const = 0;
   virtual int GetGridNumCols() const = 0;
   virtual int GetGridNumRows() const = 0;
   virtual void DisplayScreenMessage(std::string message, float durationMs = 500) = 0;
};

//https://raw.githubusercontent.com/Ableton/push-interface/master/doc/MidiMapping.png
class IAbletonGridController
{
public:
   virtual ~IAbletonGridController() {}
   virtual void OnAbletonGridConnect(IAbletonGridDevice* abletonGrid) {}
   virtual bool OnAbletonGridControl(IAbletonGridDevice* abletonGrid, int controlIndex, float midiValue) = 0;
   virtual void UpdateAbletonGridLeds(IAbletonGridDevice* abletonGrid) = 0;
   virtual bool UpdateAbletonMoveScreen(IAbletonGridDevice* abletonGrid, AbletonMoveLCD* lcd) { return false; }
   virtual bool HasHighPriorityAbletonMoveScreenUpdate(IAbletonGridDevice* abletonGrid) { return false; }
};