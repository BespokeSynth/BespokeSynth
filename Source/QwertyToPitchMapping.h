#pragma once

enum class QwertyToPitchMappingMode
{
   Ableton,
   Fruity
};

struct QwertyToPitchResponse
{
public:
   int mPitch{ -1 };
   int mOctaveShift{ 0 };
};

class QwertyToPitchMapping
{
public:
   static QwertyToPitchResponse GetPitchForComputerKey(int key);
};
