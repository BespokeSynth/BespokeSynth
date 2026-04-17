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
//  SynthGlobals.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 11/22/12.
//
//


#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "IAudioSource.h"
#include "INoteSource.h"
#include "IAudioReceiver.h"
#include "GridController.h"
#include "RollingBuffer.h"
#include "TextEntry.h"
#include "PatchCable.h"
#include "PatchCableSource.h"
#include "ChannelBuffer.h"
#include "IPulseReceiver.h"
#include "exprtk.hpp"
#include "UserPrefs.h"

#include "juce_audio_formats/juce_audio_formats.h"
#include "juce_gui_basics/juce_gui_basics.h"

#ifdef JUCE_MAC
#import <execinfo.h>
#endif

using namespace juce;

int gBufferSize = -999; //values set in SetGlobalSampleRateAndBufferSize(), setting them to bad values here to highlight any bugs
int gSampleRate = -999;
double gTwoPiOverSampleRate = -999;
double gSampleRateMs = -999;
double gInvSampleRateMs = -999;
double gBufferSizeMs = -999;
double gNyquistLimit = -999;
bool gPrintMidiInput = false;
double gTime = 1; //using a double here, so I'm going to lose nanosecond accuracy
//if I run for 4 months straight
//this means I'll lose 44100 hz sample accuracy in 7100 years of
//continuous uptime
IUIControl* gBindToUIControl = nullptr;
RetinaTrueTypeFont gFont;
RetinaTrueTypeFont gFontBold;
RetinaTrueTypeFont gFontFixedWidth;
float gModuleDrawAlpha = 255;
float gZeroBuffer[kWorkBufferSize];
float gWorkBuffer[kWorkBufferSize];
ChannelBuffer gWorkChannelBuffer(kWorkBufferSize);
IDrawableModule* gHoveredModule = nullptr;
IUIControl* gHoveredUIControl = nullptr;
IUIControl* gHotBindUIControl[10];
float gControlTactileFeedback = 0;
float gDrawScale = 1;
bool gShowDevModules = false;
float gCornerRoundness = 1;
std::array<float, (int)StepVelocityType::NumVelocityLevels> gStepVelocityLevels{};

std::array<NVGcontext*, (int)NanoVGRenderContext::Num> gNanoVGRenderContexts{};
NVGcontext* gNanoVG = nullptr;

std::random_device gRandomDevice;
bespoke::core::Xoshiro256ss gRandom(gRandomDevice);
std::uniform_real_distribution<float> gRandom01(0.0f, 1.f);
std::uniform_real_distribution<float> gRandomBipolarDist(-1.f, 1.f);

void SynthInit()
{
   std::locale::global(std::locale::classic());

   Clear(gZeroBuffer, kWorkBufferSize);

   for (int i = 0; i < 10; ++i)
      gHotBindUIControl[i] = nullptr;

   TheSynth->GetAudioFormatManager().registerBasicFormats();

   assert(kNumVoices <= 16); //assumption that we don't have more voices than midi channels

   gStepVelocityLevels[(int)StepVelocityType::Off] = kVelocityOff;
   gStepVelocityLevels[(int)StepVelocityType::Ghost] = kVelocityGhost;
   gStepVelocityLevels[(int)StepVelocityType::Normal] = kVelocityNormal;
   gStepVelocityLevels[(int)StepVelocityType::Accent] = kVelocityAccent;
}

void LoadGlobalResources()
{
   gFont.LoadFont(ofToResourcePath("frabk.ttf"));
   gFontBold.LoadFont(ofToResourcePath("frabk_m.ttf"));
   gFontFixedWidth.LoadFont(ofToResourcePath("iosevka-type-light.ttf"));
   //gModuleShader.load(ofToResourcePath("shaders/module.vert"), ofToResourcePath("shaders/module.frag"));
}

void SetGlobalSampleRateAndBufferSize(int rate, int size)
{
   assert(size <= kWorkBufferSize);
   gBufferSize = size * UserPrefs.oversampling.Get();

   gSampleRate = rate * UserPrefs.oversampling.Get();
   gTwoPiOverSampleRate = TWO_PI / gSampleRate;
   gSampleRateMs = gSampleRate / 1000.0;
   gInvSampleRateMs = 1000.0 / gSampleRate;
   gBufferSizeMs = gBufferSize / gSampleRateMs;
   gNyquistLimit = gSampleRate / 2.0f;
}

std::string GetBuildInfoString()
{
   return
#if DEBUG
   "DEBUG BUILD " +
#endif
#if BESPOKE_NIGHTLY && !BESPOKE_SUPPRESS_NIGHTLY_LABEL
   "NIGHTLY " +
#endif
   juce::JUCEApplication::getInstance()->getApplicationVersion().toStdString() + " (" + std::string(__DATE__) + " " + std::string(__TIME__) + ")";
}

void DrawAudioBuffer(float width, float height, ChannelBuffer* buffer, float start, float end, float pos, float vol /*=1*/, ofColor color /*=ofColor::black*/, int wraparoundFrom /*= -1*/, int wraparoundTo /*= 0*/)
{
   ofPushMatrix();
   if (buffer != nullptr)
   {
      int numChannels = buffer->NumActiveChannels();
      for (int i = 0; i < numChannels; ++i)
      {
         DrawAudioBuffer(width, height / numChannels, buffer->GetChannel(i), start, MIN(end, buffer->BufferSize()), pos, vol, color, wraparoundFrom, wraparoundTo, buffer->BufferSize());
         ofTranslate(0, height / numChannels);
      }
   }
   ofPopMatrix();
}

void DrawAudioBuffer(float width, float height, const float* buffer, float start, float end, float pos, float vol /*=1*/, ofColor color /*=ofColor::black*/, int wraparoundFrom /*= -1*/, int wraparoundTo /*= 0*/, int bufferSize /*=-1*/)
{
   static std::array<float, 10000> sAudioBufferMinValues;
   static std::array<float, 10000> sAudioBufferMaxValues;

   vol = MAX(.1f, vol); //make sure we at least draw something if there is waveform data

   ofPushStyle();

   ofSetLineWidth(1);
   ofFill();
   ofSetColor(255, 255, 255, 50);
   if (width > 0)
      ofRect(0, 0, width, height);
   else
      ofRect(width, 0, -width, height);

   ofSetColor(color, 17);
   ofLine(0, height / 2, width, height / 2);

   float length = end - 1 - start;
   if (length < 0)
      length = length + wraparoundFrom - wraparoundTo;
   if (length < 0)
      length += bufferSize;

   if (length > 0)
   {
      const float kStepSize = 3;
      float samplesPerStep = length / abs(width) * kStepSize;
      start = start - (int(start) % MAX(1, int(samplesPerStep)));

      if (buffer && length > 0)
      {
         float step = width > 0 ? kStepSize : -kStepSize;
         samplesPerStep = length / width * step;

         ofSetColor(color);

         float highestMagnitude = 0;
         for (float i = 0; abs(i) < abs(width); i += step)
         {
            float max = -999;
            float min = 999;
            int position = i / width * length + start;

            int j;
            int inc = 1 + samplesPerStep / 100;
            for (j = 0; j < samplesPerStep; j += inc)
            {
               int sampleIdx = position + j;
               if (wraparoundFrom != -1 && sampleIdx > wraparoundFrom)
                  sampleIdx = sampleIdx - wraparoundFrom + wraparoundTo;
               if (bufferSize > 0)
                  sampleIdx %= bufferSize;
               max = std::max(max, buffer[sampleIdx]);
               min = std::min(min, buffer[sampleIdx]);
            }

            if (max > highestMagnitude)
               highestMagnitude = max;
            if (min < -highestMagnitude)
               highestMagnitude = -min;

            if (i < (int)sAudioBufferMinValues.size())
            {
               sAudioBufferMaxValues[i] = max;
               sAudioBufferMinValues[i] = min;
            }
         }

         float rescale = 1.0f;
         if (highestMagnitude != 0.0)
            rescale = std::clamp(1.0f / highestMagnitude, 1.0f, 10.0f);
         for (float i = 0; abs(i) < abs(width); i += step)
         {
            float max = sAudioBufferMaxValues[i];
            float min = sAudioBufferMinValues[i];

            min *= height / 2 * vol * rescale;
            max *= height / 2 * vol * rescale;
            if (max > height / 2 || min < -(height / 2))
            {
               //ofSetColor(255,0,0);
               max = std::min(max, height / 2);
               min = std::max(min, -height / 2);
            }
            else
            {
               //ofSetColor(color);
            }

            if (fabsf(max - min) < .1f) //always draw something even if max == min
            {
               max += .05f;
               min -= .05f;
            }

            ofLine(i, height / 2 - max, i, height / 2 - min);
         }

         if (pos != -1)
         {
            ofSetColor(0, 255, 0);
            int position = ofMap(pos, start, end, 0, width, true);
            ofLine(position, 0, position, height);
         }
      }
   }

   ofPopStyle();
}

void Add(float* dst, const float* src, int bufferSize)
{
#ifdef USE_VECTOR_OPS
   FloatVectorOperations::add(dst, src, bufferSize);
#else
   for (int i = 0; i < bufferSize; ++i)
   {
      dst[i] += src[i];
   }
#endif
}

void Subtract(float* dst, const float* src, int bufferSize)
{
#ifdef USE_VECTOR_OPS
   FloatVectorOperations::subtract(dst, src, bufferSize);
#else
   for (int i = 0; i < bufferSize; ++i)
   {
      dst[i] -= src[i];
   }
#endif
}

void Mult(float* buff, float val, int bufferSize)
{
#ifdef USE_VECTOR_OPS
   FloatVectorOperations::multiply(buff, val, bufferSize);
#else
   for (int i = 0; i < bufferSize; ++i)
   {
      buff[i] *= val;
   }
#endif
}

void Mult(float* dst, const float* src, int bufferSize)
{
#ifdef USE_VECTOR_OPS
   FloatVectorOperations::multiply(dst, src, bufferSize);
#else
   for (int i = 0; i < bufferSize; ++i)
   {
      dst[i] *= src[i];
   }
#endif
}

void Clear(float* buffer, int bufferSize)
{
#ifdef USE_VECTOR_OPS
   FloatVectorOperations::clear(buffer, bufferSize);
#else
   bzero(buffer, bufferSize * sizeof(float));
#endif
}

void BufferCopy(float* dst, const float* src, int bufferSize)
{
#ifdef USE_VECTOR_OPS
   FloatVectorOperations::copy(dst, src, bufferSize);
#else
   memcpy(dst, src, bufferSize * sizeof(float));
#endif
}

std::string NoteName(int pitch, bool flat, bool includeOctave)
{
   int octave = pitch / 12;
   pitch %= 12;
   std::string ret = "x";
   switch (pitch)
   {
      default:
      case 0:
         ret = "C";
         break;
      case 1:
         //ret = "C#/Db";
         ret = flat ? "Db" : "C#";
         break;
      case 2:
         ret = "D";
         break;
      case 3:
         //ret = "D#/Eb";
         ret = flat ? "Eb" : "D#";
         break;
      case 4:
         ret = "E";
         break;
      case 5:
         ret = "F";
         break;
      case 6:
         //ret = "F#/Gb";
         ret = flat ? "Gb" : "F#";
         break;
      case 7:
         ret = "G";
         break;
      case 8:
         //ret = "G#/Ab";
         ret = flat ? "Ab" : "G#";
         break;
      case 9:
         ret = "A";
         break;
      case 10:
         //ret = "A#/Bb";
         ret = flat ? "Bb" : "A#";
         break;
      case 11:
         ret = "B";
         break;
   }

   if (includeOctave)
      ret += ofToString(octave - 2);

   return ret;
}

int PitchFromNoteName(std::string noteName)
{
   int octave = -2;
   if (isdigit(noteName[noteName.length() - 1]))
   {
      octave = noteName[noteName.length() - 1] - '0';
      if (noteName[noteName.length() - 2] == '-')
         octave *= -1;
      if (octave < 0)
         noteName = noteName.substr(0, noteName.length() - 2);
      else
         noteName = noteName.substr(0, noteName.length() - 1);
   }

   int pitch;
   for (pitch = 0; pitch < 12; ++pitch)
   {
      if (noteName == NoteName(pitch, false) || noteName == NoteName(pitch, true))
         break;
   }
   if (pitch == 12)
      TheSynth->LogEvent("error finding pitch for note " + noteName, kLogEventType_Error);
   return pitch + (octave + 2) * 12;
}

float Interp(float a, float start, float end)
{
   return a * (end - start) + start;
}

double GetPhaseInc(float freq)
{
   return freq * gTwoPiOverSampleRate;
}

float FloatWrap(float num, float space)
{
   if (space == 0)
      num = 0;
   num -= space * floor(num / space);
   return num;
}

double DoubleWrap(double num, float space)
{
   if (space == 0)
      num = 0;
   num -= space * floor(num / space);
   return num;
}

void DrawTextNormal(std::string text, int x, int y, float size)
{
   gFont.DrawString(text, size, x, y);
}

void DrawTextRightJustify(std::string text, int x, int y, float size)
{
   gFont.DrawString(text, size, x - gFont.GetStringWidth(text, size), y);
}

void DrawTextBold(std::string text, int x, int y, float size)
{
   gFontBold.DrawString(text, size, x, y);
}

float GetStringWidth(std::string text, float size)
{
   return gFont.GetStringWidth(text, size);
}

void AssertIfDenormal(float input)
{
   assert(input == 0 || input != input || fabsf(input) > std::numeric_limits<float>::min());
}

float GetInterpolatedSample(double offset, const float* buffer, int bufferSize)
{
   offset = DoubleWrap(offset, bufferSize);
   int pos = int(offset);
   int posNext = int(offset + 1) % bufferSize;

   float sample = buffer[pos];
   float nextSample = buffer[posNext];
   float a = offset - pos;
   float output = (1 - a) * sample + a * nextSample; //interpolate

   return output;
}

float GetInterpolatedSample(double offset, ChannelBuffer* buffer, int bufferSize, float channelBlend)
{
   assert(channelBlend <= buffer->NumActiveChannels());
   assert(channelBlend >= 0);

   if (buffer->NumActiveChannels() == 1)
      return GetInterpolatedSample(offset, buffer->GetChannel(0), bufferSize);

   int channelA = floor(channelBlend);
   if (channelA == buffer->NumActiveChannels() - 1)
      channelA -= 1;
   int channelB = channelA + 1;

   return (1 - (channelBlend - channelA)) * GetInterpolatedSample(offset, buffer->GetChannel(channelA), bufferSize) +
          (channelBlend - channelA) * GetInterpolatedSample(offset, buffer->GetChannel(channelB), bufferSize);
}

void WriteInterpolatedSample(double offset, float* buffer, int bufferSize, float sample)
{
   offset = DoubleWrap(offset, bufferSize);
   int pos = int(offset);
   int posNext = int(offset + 1) % bufferSize;

   float a = offset - pos;
   buffer[pos] += (1 - a) * sample;
   buffer[posNext] += a * sample;
}

std::string GetRomanNumeralForDegree(int degree)
{
   std::string roman;
   switch ((degree + 700) % 7)
   {
      default:
      case 0: roman = "I"; break;
      case 1: roman = "II"; break;
      case 2: roman = "III"; break;
      case 3: roman = "IV"; break;
      case 4: roman = "V"; break;
      case 5: roman = "VI"; break;
      case 6: roman = "VII"; break;
   }
   return roman;
}

void UpdateTarget(IDrawableModule* module)
{
   IAudioSource* audioSource = dynamic_cast<IAudioSource*>(module);
   INoteSource* noteSource = dynamic_cast<INoteSource*>(module);
   IGridController* grid = dynamic_cast<IGridController*>(module);
   IPulseSource* pulseSource = dynamic_cast<IPulseSource*>(module);
   std::string targetName = "";
   if (audioSource)
   {
      for (int i = 0; i < audioSource->GetNumTargets(); ++i)
      {
         IDrawableModule* target = dynamic_cast<IDrawableModule*>(audioSource->GetTarget(i));
         if (target)
            targetName = target->Path();
         else
            targetName = "";
         module->GetSaveData().SetString("target" + (i == 0 ? "" : ofToString(i + 1)), targetName);
      }
   }
   if (noteSource || grid || pulseSource)
   {
      if (module->GetPatchCableSource())
      {
         const std::vector<PatchCable*>& cables = module->GetPatchCableSource()->GetPatchCables();
         for (int i = 0; i < cables.size(); ++i)
         {
            PatchCable* cable = cables[i];
            IClickable* target = cable->GetTarget();
            if (target)
            {
               targetName += target->Path();
               if (i < cables.size() - 1)
                  targetName += ",";
            }
         }
      }
      module->GetSaveData().SetString("target", targetName);
   }
}

void DrawLissajous(RollingBuffer* buffer, float x, float y, float w, float h, float r, float g, float b, bool autocorrelationMode /* = true */)
{
   ofPushStyle();
   ofSetLineWidth(1.5f);

   int secondChannel = 1;
   if (buffer->NumChannels() == 1)
      secondChannel = 0;

   ofSetColor(r * 255, g * 255, b * 255, 70);
   ofBeginShape();
   const int delaySamps = 90;
   int numPoints = MIN(buffer->Size() - delaySamps - 1, .02f * gSampleRate);
   for (int i = 100; i < numPoints; ++i)
   {
      float vx = x + w / 2 + buffer->GetSample(i, 0) * .8f * MIN(w, h);
      float vy;
      if (autocorrelationMode)
         vy = y + h / 2 + buffer->GetSample(i + delaySamps, secondChannel) * .8f * MIN(w, h);
      else
         vy = y + h / 2 + buffer->GetSample(i, secondChannel) * .8f * MIN(w, h);
      //float alpha = 1 - (i/float(numPoints));
      //ofSetColor(r*255,g*255,b*255,alpha*alpha*255);
      ofVertex(vx, vy);
   }
   ofEndShape();

   ofPopStyle();
}

void StringCopy(char* dest, const char* source, int destLength)
{
   if (dest == source)
      return;
   strncpy(dest, source, destLength);
   dest[destLength - 1] = 0;
}

int GetKeyModifiers()
{
   int ret = 0;
   if (ModifierKeys::currentModifiers.isShiftDown())
      ret |= kModifier_Shift;
   if (ModifierKeys::currentModifiers.isAltDown())
      ret |= kModifier_Alt;
#if BESPOKE_MAC
   if (ModifierKeys::currentModifiers.isCtrlDown())
      ret |= kModifier_Control; //control and command interfere with each other on non-mac keyboards
#endif
   if (ModifierKeys::currentModifiers.isCommandDown())
      ret |= kModifier_Command;
   return ret;
}

bool IsKeyHeld(int key, int modifiers)
{
   return IKeyboardFocusListener::GetActiveKeyboardFocus() == nullptr &&
          KeyPress::isKeyCurrentlyDown(key) &&
          GetKeyModifiers() == modifiers &&
          TheSynth->GetMainComponent()->hasKeyboardFocus(true);
}

int KeyToLower(int key)
{
   if (key < CHAR_MAX && CharacterFunctions::isLetter((char)key))
      return tolower(key);
   if (key == '!')
      return '1';
   if (key == '@')
      return '2';
   if (key == '#')
      return '3';
   if (key == '$')
      return '4';
   if (key == '%')
      return '5';
   if (key == '^')
      return '6';
   if (key == '&')
      return '7';
   if (key == '*')
      return '8';
   if (key == '(')
      return '9';
   if (key == ')')
      return '0';
   if (key == '~')
      return '`';
   if (key == '_')
      return '-';
   if (key == '+')
      return '=';
   if (key == '<')
      return ',';
   if (key == '>')
      return '.';
   if (key == '?')
      return '/';
   if (key == ':')
      return ';';
   if (key == '"')
      return '\'';
   if (key == '{')
      return '[';
   if (key == '}')
      return ']';
   if (key == '|')
      return '\\';
   return key;
}

float EaseIn(float start, float end, float a)
{
   return (end - start) * a * a + start;
}

float EaseOut(float start, float end, float a)
{
   return -(end - start) * a * (a - 2) + start;
}

float Bias(float value, float bias)
{
   assert(bias >= 0 && bias <= 1);
   const float kLog25 = log(25);
   bias = .2f * expf(kLog25 * bias); //pow(25,bias)
   return pow(value, bias);
}

float Pow2(float in)
{
   const float kLog2 = log(2);
   return expf(kLog2 * in);
}

void PrintCallstack()
{
#ifdef JUCE_MAC
   void* callstack[128];
   int frameCount = backtrace(callstack, 128);
   char** frameStrings = backtrace_symbols(callstack, frameCount);

   if (frameStrings != nullptr)
   {
      // Start with frame 1 because frame 0 is PrintBacktrace()
      for (int i = 1; i < frameCount; i++)
      {
         printf("%s\n", frameStrings[i]);
      }
      free(frameStrings);
   }
#endif
}

bool IsInUnitBox(ofVec2f pos)
{
   return pos.x >= 0 && pos.x < 1 && pos.y >= 0 && pos.y < 1;
}

std::string GetUniqueName(std::string name, std::vector<IDrawableModule*> existing)
{
   std::string strippedName = name;
   while (strippedName.length() > 1 && CharacterFunctions::isDigit((char)strippedName[strippedName.length() - 1]))
      strippedName.resize(strippedName.length() - 1);
   int suffix = 1;
   std::string suffixString = name;
   ofStringReplace(suffixString, strippedName, "");
   if (!suffixString.empty())
      suffix = atoi(suffixString.c_str());
   while (true)
   {
      bool isNameUnique = true;
      for (int i = 0; i < existing.size(); ++i)
      {
         if (existing[i]->Name() == name)
         {
            ++suffix;
            name = strippedName + ofToString(suffix);
            isNameUnique = false;
            break;
         }
      }
      if (isNameUnique)
         break;
   }

   return name;
}

std::string GetUniqueName(std::string name, std::vector<std::string> existing)
{
   std::string origName = name;
   int suffix = 1;
   while (true)
   {
      bool isNameUnique = true;
      for (int i = 0; i < existing.size(); ++i)
      {
         if (existing[i] == name)
         {
            ++suffix;
            name = origName + ofToString(suffix);
            isNameUnique = false;
            break;
         }
      }
      if (isNameUnique)
         break;
   }

   return name;
}

float DistSqToLine(ofVec2f point, ofVec2f a, ofVec2f b)
{
   float l2 = a.distanceSquared(b);
   if (l2 == 0.0f)
      return point.distanceSquared(a);

   float t = ((point.x - b.x) * (a.x - b.x) + (point.y - b.y) * (a.y - b.y)) / l2;
   if (t < 0.0f)
      return point.distanceSquared(a);
   if (t > 1.0f)
      return point.distanceSquared(b);
   return point.distanceSquared(ofVec2f(b.x + t * (a.x - b.x), b.y + t * (a.y - b.y)));
}

//Jenkins one-at-a-time hash
uint32_t JenkinsHash(const char* key)
{
   uint32_t hash, i;
   for (hash = i = 0; key[i] != 0; ++i)
   {
      hash += key[i];
      hash += (hash << 10);
      hash ^= (hash >> 6);
   }
   hash += (hash << 3);
   hash ^= (hash >> 11);
   hash += (hash << 15);
   return hash;
}

void LoadStateValidate(bool assertion)
{
   if (!assertion)
      throw LoadStateException();
}

double NextBufferTime(bool includeLookahead)
{
   double time = gTime + gBufferSizeMs;
   if (includeLookahead)
      time += TheTransport->GetEventLookaheadMs();
   return time;
}

bool IsMainThread()
{
   return std::this_thread::get_id() == ModularSynth::GetMainThreadID();
}

bool IsAudioThread()
{
   return std::this_thread::get_id() == ModularSynth::GetAudioThreadID();
}

bool IsRenderThread()
{
   return std::this_thread::get_id() == ModularSynth::GetRenderThreadID();
}

float GetLeftPanGain(float pan)
{
   return 1 - ofClamp(pan, -1, 1);
}

float GetRightPanGain(float pan)
{
   return ofClamp(pan, -1, 1) + 1;
}

bool EvaluateExpression(std::string expressionStr, float currentValue, float& output)
{
   exprtk::symbol_table<float> symbolTable;
   exprtk::expression<float> expression;
   symbolTable.add_variable("x", currentValue);
   symbolTable.add_constants();
   expression.register_symbol_table(symbolTable);

   juce::String input = expressionStr;
   if (input.startsWith("+="))
      input = input.replace("+=", "x+");
   if (input.startsWith("*="))
      input = input.replace("*=", "x*");
   if (input.startsWith("/="))
      input = input.replace("/=", "x/");
   if (input.startsWith("-="))
      input = input.replace("-=", "x-");

   exprtk::parser<float> parser;
   bool expressionValid = parser.compile(input.toStdString(), expression);
   if (expressionValid)
   {
      output = expression.value();
      return true;
   }
   return false;
}

ofLog::~ofLog()
{
   std::string output = ofToString(gTime / 1000, 8) + ": " + mMessage;
   DBG(output);
   if (mSendToBespokeConsole)
      TheSynth->LogEvent(output, kLogEventType_Verbose);
}

#ifdef BESPOKE_DEBUG_ALLOCATIONS
FILE* logAllocationsFile;

namespace
{
   const int maxFilenameLen = 90;
   typedef struct
   {
      uint32 address;
      uint32 size;
      char file[maxFilenameLen];
      uint32 line;
      bool allocated;
   } AllocInfo;

   bool enableTracking = false;

   template <class T>
   class my_allocator
   {
   public:
      typedef size_t size_type;
      typedef ptrdiff_t difference_type;
      typedef T* pointer;
      typedef const T* const_pointer;
      typedef T& reference;
      typedef const T& const_reference;
      typedef T value_type;

      my_allocator() {}
      my_allocator(const my_allocator&) {}

      pointer allocate(size_type n, const void* = 0)
      {
         T* t = (T*)malloc(n * sizeof(T));
         return t;
      }

      void deallocate(void* p, size_type)
      {
         if (p)
            free(p);
      }

      pointer address(reference x) const { return &x; }
      const_pointer address(const_reference x) const { return &x; }
      my_allocator<T>& operator=(const my_allocator&) { return *this; }
#undef new
      void construct(pointer p, const T& val)
      {
         new ((T*)p) T(val);
      }
#define new DEBUG_NEW
      void destroy(pointer p)
      {
         p->~T();
      }

      size_type max_size() const { return size_t(-1); }

      template <class U>
      struct rebind
      {
         typedef my_allocator<U> other;
      };

      template <class U>
      my_allocator(const my_allocator<U>&) {}

      template <class U>
      my_allocator& operator=(const my_allocator<U>&) { return *this; }
   };

   map<uint32, AllocInfo*, less<int>, my_allocator<pair<uint32, AllocInfo*> > > allocList;

   void AddTrack(uint32 addr, uint32 asize, const char* fname, uint32 lnum)
   {
      if (enableTracking)
      {
         fprintf(logAllocationsFile, "%-90s:  LINE %5d,  ADDRESS %08x  %8d newed\n", fname, lnum, addr, asize);
         /*AllocInfo* info = (AllocInfo*)malloc(sizeof(AllocInfo));
         
         info->address = addr;
         strncpy(info->file, fname, maxFilenameLen-1);
         info->file[maxFilenameLen-1] = 0;
         info->line = lnum;
         info->size = asize;
         info->allocated = true;
         allocList[addr] = info;*/
      }
   };

   void RemoveTrack(uint32 addr)
   {
      if (enableTracking)
      {
         //fprintf(logAllocationsFile, "ADDRESS %08x  deleted\n", addr);
         /*auto info = allocList.find(addr);
         if (info == allocList.end())
         {
            //assert(false); //Trying to free a never allocated address!
            ofLog() << "Trying to free a never allocated address!";
         }
         else
         {
            if (info->second->allocated == false)
               assert(false); //Trying to free an already freed address!
            else
               info->second->allocated = false;
         }*/
      }
   };
}

void SetMemoryTrackingEnabled(bool enabled)
{
   if (enabled && logAllocationsFile == nullptr)
   {
      logAllocationsFile = fopen(ofToDataPath("allocationslog.txt").c_str(), "w");
   }

   enableTracking = enabled;
}

void DumpUnfreedMemory()
{
   uint32 totalSize = 0;
   char buf[1024];
   map<string, int> perFileTotal;

   for (auto element : allocList)
   {
      const AllocInfo* info = element.second;
      if (info && info->allocated)
      {
         snprintf(buf, sizeof(buf), "%-90s:  LINE %5d,  ADDRESS %08x  %8d unfreed",
                  info->file, info->line, info->address, info->size);
         ofLog() << buf;
         totalSize += info->size;

         perFileTotal[info->file] += info->size;
      }
   }
   ofLog() << "-----------------------------------------------------------";
   for (auto fileInfo : perFileTotal)
   {
      snprintf(buf, sizeof(buf), "%-90s:  %10d unfreed",
               fileInfo.first.c_str(), fileInfo.second);
      ofLog() << buf;
   }
   ofLog() << "-----------------------------------------------------------";
   snprintf(buf, sizeof(buf), "Total Unfreed: %d bytes", totalSize);
   ofLog() << buf;
};

#undef new
void* operator new(std::size_t size) throw(std::bad_alloc)
{
   void* ptr = (void*)malloc(size);
   //AddTrack((uint32)ptr, size, "<unknown>", 0);
   return (ptr);
}
void* operator new(std::size_t size, const char* file, int line) throw(std::bad_alloc)
{
   void* ptr = (void*)malloc(size);
   AddTrack((uint32)ptr, size, file, line);
   return (ptr);
}
void operator delete(void* p) throw()
{
   //RemoveTrack((uint32)p);
   free(p);
}
void* operator new[](std::size_t size) throw(std::bad_alloc)
{
   void* ptr = (void*)malloc(size);
   //AddTrack((uint32)ptr, size, "<unknown>", 0);
   return (ptr);
}
void* operator new[](std::size_t size, const char* file, int line) throw(std::bad_alloc)
{
   void* ptr = (void*)malloc(size);
   AddTrack((uint32)ptr, size, file, line);
   return (ptr);
}
void operator delete[](void* p) throw()
{
   //RemoveTrack((uint32)p);
   free(p);
}
#define new DEBUG_NEW
#else
void SetMemoryTrackingEnabled(bool enabled)
{
}
void DumpUnfreedMemory()
{
   ofLog() << "This only works with BESPOKE_DEBUG_ALLOCATIONS defined";
};
#endif
