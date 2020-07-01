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

#include "../JuceLibraryCode/JuceHeader.h"
#include "OpenFrameworksPort.h"
#include <map>
#include <list>
#include <vector>
#include <array>
#include <math.h>
#include <cctype>
#include <random>
#include <float.h>

//#define BESPOKE_DEBUG_ALLOCATIONS

#ifdef BESPOKE_DEBUG_ALLOCATIONS
void* operator new(std::size_t size, const char *file, int line) throw(std::bad_alloc);
void* operator new[](std::size_t size, const char *file, int line) throw(std::bad_alloc);
#define DEBUG_NEW new(__FILE__, __LINE__)
#else
#define DEBUG_NEW new
#endif
#define new DEBUG_NEW

#define MAX_BUFFER_SIZE 30*gSampleRate
#define MAX_TEXTENTRY_LENGTH 128

#ifndef M_PI
#define M_PI PI
#endif

#define FPI       3.14159265358979323846f
#define FTWO_PI   6.28318530717958647693f

#define USE_VECTOR_OPS

#if JUCE_WINDOWS
#define popen _popen
#define pclose _pclose
#endif

//bool labeling technique that I stole from Ableton
#define K(x) true
#define L(x,y) y

//avoid "unused variable" warnings
#define UNUSED(x) ((void) x)

using namespace std;

class IUIControl;
class IDrawableModule;
class RollingBuffer;
class ChannelBuffer;

typedef map<string,int> EnumMap;

const int kWorkBufferSize = 4096; //larger than the audio buffer size would ever be

const int kNumVoices = 18;

extern int gSampleRate;
extern int gBufferSize;
extern float gDefaultTempo;
extern double gTwoPiOverSampleRate;
extern double gSampleRateMs;
extern double gInvSampleRateMs;
extern float gNyquistLimit;
extern bool gPrintMidiInput;
extern double gTime;
extern float gVizFreq;
extern IUIControl* gBindToUIControl;
extern RetinaTrueTypeFont gFont;
extern RetinaTrueTypeFont gFontBold;
extern RetinaTrueTypeFont gFontFixedWidth;
extern float gModuleDrawAlpha;
extern float gNullBuffer[4096];
extern float gZeroBuffer[4096];
extern float gWorkBuffer[4096];  //scratch buffer for doing work in
extern ChannelBuffer gWorkChannelBuffer;
extern IDrawableModule* gHoveredModule;
extern IUIControl* gHoveredUIControl;
extern IUIControl* gHotBindUIControl[10];
extern float gControlTactileFeedback;
extern float gDrawScale;
extern bool gShowDevModules;
extern float gCornerRoundness;

struct GlobalManagers
{
   AudioDeviceManager mDeviceManager;
   AudioFormatManager mAudioFormatManager;
};

enum OscillatorType
{
   kOsc_Sin,
   kOsc_Square,
   kOsc_Tri,
   kOsc_Saw,
   kOsc_NegSaw,
   kOsc_Random,
   kOsc_Drunk
};

enum KeyModifiers
{
   kModifier_None = 0,
   kModifier_Shift = 1,
   kModifier_Alt = 2,
   kModifier_Control = 4,
   kModifier_Command = 8
};

class LoadingJSONException : public exception {};
class UnknownModuleException : public exception
{
public:
   UnknownModuleException(string searchName)
   : mSearchName(searchName) {}
   ~UnknownModuleException() throw() {}
   string mSearchName;
};
class UnknownEffectTypeException : public exception {};
class BadUIControlPathException : public exception {};
class UnknownUIControlException : public exception {};
class WrongModuleTypeException : public exception {};
class LoadStateException : public exception {};

void SynthInit();
void LoadGlobalResources();

void SetGlobalBufferSize(int size);
void SetGlobalSampleRate(int rate);
void DrawAudioBuffer(float width, float height, ChannelBuffer* buffer, float start, float end, float pos, float vol=1, ofColor color=ofColor::black);
void DrawAudioBuffer(float width, float height, const float* buffer, float start, float end, float pos, float vol=1, ofColor color=ofColor::black);
void Add(float* buff1, const float* buff2, int bufferSize);
void Mult(float* buff, float val, int bufferSize);
void Mult(float* buff1, const float* buff2, int bufferSize);
void Clear(float* buffer, int bufferSize);
void BufferCopy(float* dst, const float* src, int bufferSize);
string NoteName(int pitch, bool flat=false, bool includeOctave = false);
int PitchFromNoteName(string noteName);
float Interp(float a, float start, float end);
double GetPhaseInc(float freq);
void FloatWrap(float& num, float space);
void FloatWrap(double& num, float space);
void DrawTextNormal(string text, int x, int y, float size = 15);
void DrawTextLeftJustify(string text, int x, int y, float size = 15);
void DrawTextBold(string text, int x, int y, float size = 15);
float GetStringWidth(string text, float size = 15);
void AssertIfDenormal(float input);
float GetInterpolatedSample(double offset, const float* buffer, int bufferSize);
float GetInterpolatedSample(double offset, ChannelBuffer* buffer, int bufferSize, float channelBlend);
void WriteInterpolatedSample(double offset, float* buffer, int bufferSize, float sample);
string GetRomanNumeralForDegree(int degree);
void UpdateTarget(IDrawableModule* module);
void DrawLissajous(RollingBuffer* buffer, float x, float y, float w, float h, float r = .2f, float g = .7f, float b = .2f);
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
string GetUniqueName(string name, vector<IDrawableModule*> existing);
string GetUniqueName(string name, vector<string> existing);
void SetMemoryTrackingEnabled(bool enabled);
void DumpUnfreedMemory();
float DistSqToLine(ofVec2f point, ofVec2f a, ofVec2f b);
uint32_t JenkinsHash(const char* key);
void LoadStateValidate(bool assertion);
float GetLeftPanGain(float pan);
float GetRightPanGain(float pan);
void DrawFallbackText(const char* text, float posX, float posY);
bool EvaluateExpression(string expression, float currentValue, float& output);

inline static float RandomSample()
{
   return (float(rand())/RAND_MAX) * 2.0f - 1.0f;
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
void RemoveFromVector(T element, std::vector<T>& vec, bool fail=false)
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
   Vec2i() : x(0), y(0) {}
   Vec2i(int _x, int _y) : x(_x), y(_y) {}
   int x;
   int y;
};

#ifdef JUCE_WINDOWS
inline void bzero(void* mem, size_t size)
{
   memset(mem, 0, size);
}
#endif

#define FIX_DENORMAL(p) if(fabsf(p)<1e-9) p = 0

class ofLog
{
public:
   ofLog() {}
   ~ofLog() { cout << gTime << ": " << mMessage << endl; }
   
   template <class T>
   ofLog& operator<<(const T& value)
   {
      mMessage += ofToString(value);
      return *this;
   }
private:
   string mMessage;
};
