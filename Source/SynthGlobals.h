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
//  SynthGlobals.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/22/12.
//
//

#pragma once

#if __clang__
#pragma clang diagnostic ignored "-Wreorder"
#endif

#include "Xoshiro256ss.h"
#include "OpenFrameworksPort.h"
#include <map>
#include <list>
#include <vector>
#include <algorithm>
#include <array>
#include <cctype>
#include <random>
#include <float.h>

//#define BESPOKE_DEBUG_ALLOCATIONS

#ifdef BESPOKE_DEBUG_ALLOCATIONS
void* operator new(std::size_t size, const char* file, int line) throw(std::bad_alloc);
void* operator new[](std::size_t size, const char* file, int line) throw(std::bad_alloc);
#define DEBUG_NEW new (__FILE__, __LINE__)
#else
#define DEBUG_NEW new
#endif
#define new DEBUG_NEW

#define BESPOKE_SUPPRESS_NIGHTLY_LABEL 0

#if !defined(__PRETTY_FUNCTION__) && !defined(__GNUC__)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#define MAX_BUFFER_SIZE 30 * gSampleRate
#define MAX_TEXTENTRY_LENGTH 1024

#ifndef M_PI
#define M_PI PI
#endif

#define FPI 3.14159265358979323846f
#define FTWO_PI 6.28318530717958647693f

#define USE_VECTOR_OPS

//bool labeling technique that I stole from Ableton
#define K(x) true
#define L(x, y) y

//avoid "unused variable" warnings
#define UNUSED(x) ((void)x)

class IUIControl;
class IDrawableModule;
class RollingBuffer;
class ChannelBuffer;

typedef std::map<std::string, int> EnumMap;

const int kWorkBufferSize = 8192 * 16 * 2; //larger than the audio buffer size would ever be (even oversampled). Noxy: This needs to be twice as large as the largest possible buffersize (Largest I've seen on my system is 8192) multiplied by the largest possible oversampling times two. Why two? Well effectchains use this buffer twice consecutive for the drywet mixing. Obviously this should become a smart buffer so we can dynamically increase the size when it is needed but that is for later. For now this increase should fix it ... mostly.

const int kNumVoices = 16;

extern int gSampleRate;
extern int gBufferSize;
extern double gTwoPiOverSampleRate;
extern double gSampleRateMs;
extern double gInvSampleRateMs;
extern double gBufferSizeMs;
extern double gNyquistLimit;
extern bool gPrintMidiInput;
extern double gTime;
extern IUIControl* gBindToUIControl;
extern RetinaTrueTypeFont gFont;
extern RetinaTrueTypeFont gFontBold;
extern RetinaTrueTypeFont gFontFixedWidth;
extern float gModuleDrawAlpha;
extern float gNullBuffer[kWorkBufferSize];
extern float gZeroBuffer[kWorkBufferSize];
extern float gWorkBuffer[kWorkBufferSize]; //scratch buffer for doing work in
extern ChannelBuffer gWorkChannelBuffer;
extern IDrawableModule* gHoveredModule;
extern IUIControl* gHoveredUIControl;
extern IUIControl* gHotBindUIControl[10];
extern float gControlTactileFeedback;
extern float gDrawScale;
extern bool gShowDevModules;
extern float gCornerRoundness;

extern std::random_device gRandomDevice;

extern bespoke::core::Xoshiro256ss gRandom;
extern std::uniform_real_distribution<float> gRandom01;
extern std::uniform_real_distribution<float> gRandomBipolarDist;

enum OscillatorType
{
   kOsc_Sin,
   kOsc_Square,
   kOsc_Tri,
   kOsc_Saw,
   kOsc_NegSaw,
   kOsc_Random,
   kOsc_Drunk,
   kOsc_Perlin
};

enum KeyModifiers
{
   kModifier_None = 0,
   kModifier_Shift = 1,
   kModifier_Alt = 2,
   kModifier_Control = 4,
   kModifier_Command = 8
};

enum class StepVelocityType
{
   Off = 0,
   Ghost = 1,
   Normal = 2,
   Accent = 3,
   NumVelocityLevels = 4
};
constexpr float kVelocityOff = 0.0f;
constexpr float kVelocityGhost = 0.4f;
constexpr float kVelocityNormal = 0.75f;
constexpr float kVelocityAccent = 1.0f;
extern std::array<float, (int)StepVelocityType::NumVelocityLevels> gStepVelocityLevels;

class LoadingJSONException : public std::exception
{
};
class UnknownModuleException : public std::exception
{
public:
   UnknownModuleException(std::string searchName)
   : mSearchName(searchName)
   {}
   ~UnknownModuleException() throw() {}
   std::string mSearchName;
};
class UnknownEffectTypeException : public std::exception
{
};
class BadUIControlPathException : public std::exception
{
};
class UnknownUIControlException : public std::exception
{
};
class WrongModuleTypeException : public std::exception
{
};
class LoadStateException : public std::exception
{
};

void SynthInit();
void LoadGlobalResources();

void SetGlobalSampleRateAndBufferSize(int rate, int size);
std::string GetBuildInfoString();
void DrawAudioBuffer(float width, float height, ChannelBuffer* buffer, float start, float end, float pos, float vol = 1, ofColor color = ofColor::black, int wraparoundFrom = -1, int wraparoundTo = 0);
void DrawAudioBuffer(float width, float height, const float* buffer, float start, float end, float pos, float vol = 1, ofColor color = ofColor::black, int wraparoundFrom = -1, int wraparoundTo = 0, int bufferSize = -1);
void Add(float* buff1, const float* buff2, int bufferSize);
void Subtract(float* buff1, const float* buff2, int bufferSize);
void Mult(float* buff, float val, int bufferSize);
void Mult(float* buff1, const float* buff2, int bufferSize);
void Clear(float* buffer, int bufferSize);
void BufferCopy(float* dst, const float* src, int bufferSize);
std::string NoteName(int pitch, bool flat = false, bool includeOctave = false);
int PitchFromNoteName(std::string noteName);
float Interp(float a, float start, float end);
double GetPhaseInc(float freq);
float FloatWrap(float num, float space);
double DoubleWrap(double num, float space);
void DrawTextNormal(std::string text, int x, int y, float size = 13);
void DrawTextRightJustify(std::string text, int x, int y, float size = 13);
void DrawTextBold(std::string text, int x, int y, float size = 13);
float GetStringWidth(std::string text, float size = 13);
void AssertIfDenormal(float input);
float GetInterpolatedSample(double offset, const float* buffer, int bufferSize);
float GetInterpolatedSample(double offset, ChannelBuffer* buffer, int bufferSize, float channelBlend);
void WriteInterpolatedSample(double offset, float* buffer, int bufferSize, float sample);
std::string GetRomanNumeralForDegree(int degree);
void UpdateTarget(IDrawableModule* module);
void DrawLissajous(RollingBuffer* buffer, float x, float y, float w, float h, float r = .2f, float g = .7f, float b = .2f, bool autocorrelationMode = true);
void StringCopy(char* dest, const char* source, int destLength);
int GetKeyModifiers();
bool IsKeyHeld(int key, int modifiers = kModifier_None);
int KeyToLower(int key);
float EaseIn(float start, float end, float a);
float EaseOut(float start, float end, float a);
float Bias(float value, float bias);
float Pow2(float in);
void PrintCallstack();
bool IsInUnitBox(ofVec2f pos);
std::string GetUniqueName(std::string name, std::vector<IDrawableModule*> existing);
std::string GetUniqueName(std::string name, std::vector<std::string> existing);
void SetMemoryTrackingEnabled(bool enabled);
void DumpUnfreedMemory();
float DistSqToLine(ofVec2f point, ofVec2f a, ofVec2f b);
uint32_t JenkinsHash(const char* key);
void LoadStateValidate(bool assertion);
float GetLeftPanGain(float pan);
float GetRightPanGain(float pan);
void DrawFallbackText(const char* text, float posX, float posY);
bool EvaluateExpression(std::string expression, float currentValue, float& output);
double NextBufferTime(bool includeLookahead);
bool IsAudioThread();

inline static float RandomSample()
{
   return gRandomBipolarDist(gRandom);
}

inline static int DeterministicRandom(int seed, int index)
{
   uint64_t x = seed + ((uint64_t)index << 32);
   x = (x ^ (x >> 30)) * (0xbf58476d1ce4e5b9);
   x = (x ^ (x >> 27)) * (0x94d049bb133111eb);
   x = x ^ (x >> 31);
   return (int)x;
}

inline static std::string GetPathSeparator()
{
#if BESPOKE_WINDOWS
   return "\\";
#else
   return "/";
#endif
}

#ifndef assert
#define assert Assert

inline static void Assert(bool condition)
{
   if (condition == false)
   {
      ofLog() << "assertion failed";
      throw new exception();
   }
}
#endif

template <class T>
void RemoveFromVector(T element, std::vector<T>& vec, bool fail = false)
{
   auto toRemove = std::find(vec.begin(), vec.end(), element);
   if (fail && toRemove == vec.end())
      assert(false);
   if (toRemove != vec.end())
      vec.erase(toRemove);
}

template <class T>
bool VectorContains(T element, const std::vector<T>& vec)
{
   return std::find(vec.begin(), vec.end(), element) != vec.end();
}

template <class T>
bool ListContains(T element, const std::list<T>& lis)
{
   return std::find(lis.begin(), lis.end(), element) != lis.end();
}

struct Vec2i
{
   Vec2i()
   : x(0)
   , y(0)
   {}
   Vec2i(int _x, int _y)
   : x(_x)
   , y(_y)
   {}
   int x;
   int y;
};

class ofLog
{
public:
   ofLog()
   : mSendToBespokeConsole(true)
   {}
   ~ofLog();

   template <class T>
   ofLog& operator<<(const T& value)
   {
      mMessage += ofToString(value);
      return *this;
   }

   ofLog& operator!()
   {
      mSendToBespokeConsole = false;
      return *this;
   }

private:
   std::string mMessage;
   bool mSendToBespokeConsole;
};
