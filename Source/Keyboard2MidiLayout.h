#pragma once

enum class Keyboard2MidiLayoutType
{
   Ableton,
   Fruity,
   Ignore
};

struct Keyboard2MidiResponse
{
public:
   int pitch;
   int newOctave;
};

class Keyboard2MidiLayout
{
public:
   static Keyboard2MidiResponse GetPitchForComputerKey(int key, int octave);
};
