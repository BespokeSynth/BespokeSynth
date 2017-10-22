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
#include "INoteReceiver.h"
#include "GridController.h"
#include "RollingBuffer.h"
#include "TextEntry.h"
#include "PatchCable.h"
#include "PatchCableSource.h"
#include "ChannelBuffer.h"

#ifdef JUCE_MAC
#import <execinfo.h>
#endif

int gBufferSize = 64;
int gSampleRate = 44100;
float gDefaultTempo = 105;
double gTwoPiOverSampleRate = TWO_PI / gSampleRate;
double gSampleRateMs = gSampleRate / 1000.0;
double gInvSampleRateMs = 1000.0 / gSampleRate;
float gNyquistLimit = gSampleRate / 2.0f;
bool gPrintMidiInput = false;
double gTime = 1; //using a double here, so I'm going to lose nanosecond accuracy
                  //if I run for 4 months straight
                  //this means I'll lose 44100 hz sample accuracy in 7100 years of
                  //continuous uptime
float gVizFreq = 440;
IUIControl* gBindToUIControl = nullptr;
RetinaTrueTypeFont gFont;
RetinaTrueTypeFont gFontBold;
float gModuleDrawAlpha = 255;
float gNullBuffer[kWorkBufferSize];
float gZeroBuffer[kWorkBufferSize];
float gWorkBuffer[kWorkBufferSize];
ChannelBuffer gWorkChannelBuffer(kWorkBufferSize);
IUIControl* gHoveredUIControl = nullptr;
IUIControl* gHotBindUIControl[10];
float gControlTactileFeedback = 0;
bool gIsRetina = false;
float gDrawScale = 1;
bool gShowDevModules = false;
float gCornerRoundness = 1;

void SynthInit()
{
   srand(time(nullptr));
   
   gDefaultTempo = rand() % 80 + 75;
   
   Clear(gZeroBuffer, kWorkBufferSize);
   
   for (int i=0; i<10; ++i)
      gHotBindUIControl[i] = nullptr;
   
   TheSynth->GetGlobalManagers()->mAudioFormatManager.registerBasicFormats();
   
   //assert(kNumVoices <= 16);  //assumption that we don't have more voices than midi channels
}

void LoadGlobalResources()
{
   gFont.LoadFont(ofToDataPath("frabk.ttf"));
   gFontBold.LoadFont(ofToDataPath("frabk_m.ttf"));
   //gModuleShader.load(ofToDataPath("shaders/module.vert"), ofToDataPath("shaders/module.frag"));
}

void SetGlobalBufferSize(int size)
{
   assert(size <= kWorkBufferSize);
   gBufferSize = size;
}

void SetGlobalSampleRate(int rate)
{
   gSampleRate = rate;
   gTwoPiOverSampleRate = TWO_PI / gSampleRate;
   gSampleRateMs = gSampleRate / 1000.0;
   gInvSampleRateMs = 1000.0 / gSampleRate;
   gNyquistLimit = gSampleRate / 2.0f;
}

void DrawAudioBuffer(float width, float height, ChannelBuffer* buffer, float start, float end, float pos, float vol /*=1*/, ofColor color /*=ofColor::black*/)
{
   int numChannels = buffer->NumActiveChannels();
   for (int i=0; i<numChannels; ++i)
   {
      DrawAudioBuffer(width, height/numChannels, buffer->GetChannel(i), start, end, pos, vol, color);
      ofTranslate(0, height/numChannels);
   }
}

void DrawAudioBuffer(float width, float height, const float* buffer, float start, float end, float pos, float vol /*=1*/, ofColor color /*=ofColor::black*/)
{
   vol = MAX(.1f,vol); //make sure we at least draw something if there is waveform data
   
   ofPushStyle();
   
   ofSetLineWidth(.5f);
   ofFill();
   ofSetColor(255,255,255,50);
   ofRect(0, 0, width, height);
   
   if (buffer && end - start > 0)
   {
      float step = .5f/gDrawScale;
      float samplesPerStep = (end-start) / width * step;
      
      for (float i = 0; i < width; i+=step)
      {
         float mag = 0;
         int position =  ofMap(i, 0, width, start, end-1, true);
         //rms
         int j;
         int inc = 1+samplesPerStep / 100;
         for (j=0; j<samplesPerStep && position+j < end-1; j+=inc)
            mag += buffer[position+j] * buffer[position+j];
         mag /= j/inc;
         mag = sqrt(mag);
         mag = sqrt(mag);
         mag *= height/2 * vol;
         if (mag > height/2)
         {
            ofSetColor(255,0,0);
            mag = height/2;
         }
         else
         {
            ofSetColor(color);
         }
         if (mag == 0)
            mag = .1f;
         ofLine(i, height/2-mag, i, height/2+mag);
      }
      
      if (pos != -1)
      {
         ofSetColor(0,255,0);
         int position =  ofMap(pos, start, end, 0, width, true);
         ofLine(position,0,position,height);
      }
   }

   ofPopStyle();
}

void Add(float* buff1, const float* buff2, int bufferSize)
{
#ifdef USE_VECTOR_OPS
   FloatVectorOperations::add(buff1, buff2, bufferSize);
#else
   for (int i=0; i<bufferSize; ++i)
   {
      buff1[i] += buff2[i];
   }
#endif
}

void Mult(float* buff, float val, int bufferSize)
{
#ifdef USE_VECTOR_OPS
   FloatVectorOperations::multiply(buff, val, bufferSize);
#else
   for (int i=0; i<bufferSize; ++i)
   {
      buff[i] *= val;
   }
#endif
}

void Mult(float* buff1, const float* buff2, int bufferSize)
{
#ifdef USE_VECTOR_OPS
   FloatVectorOperations::multiply(buff1, buff2, bufferSize);
#else
   for (int i=0; i<bufferSize; ++i)
   {
      buff1[i] *= buff2[i];
   }
#endif
}

void Clear(float* buffer, int bufferSize)
{
#ifdef USE_VECTOR_OPS
   FloatVectorOperations::clear(buffer, bufferSize);
#else
   bzero(buffer, bufferSize*sizeof(float));
#endif
}

void BufferCopy(float* dst, const float* src, int bufferSize)
{
#ifdef USE_VECTOR_OPS
   FloatVectorOperations::copy(dst, src, bufferSize);
#else
   memcpy(dst, src, bufferSize*sizeof(float));
#endif
}

string NoteName(int pitch, bool flat)
{
   pitch %= 12;
   switch (pitch)
   {
      case 0:
         return "C";
      case 1:
         //return "C#/Db";
         return flat ? "Db" : "C#";
      case 2:
         return "D";
      case 3:
         //return "D#/Eb";
         return flat ? "Eb" : "D#";
      case 4:
         return "E";
      case 5:
         return "F";
      case 6:
         //return "F#/Gb";
         return flat ? "Gb" : "F#";
      case 7:
         return "G";
      case 8:
         //return "G#/Ab";
         return flat ? "Ab" : "G#";
      case 9:
         return "A";
      case 10:
         //return "A#/Bb";
         return flat ? "Bb" : "A#";
      case 11:
         return "B";
   }
   return "x";
}

int PitchFromNoteName(string noteName)
{
   int pitch;
   for (pitch=0; pitch<12; ++pitch)
   {
      if (noteName == NoteName(pitch, false) || noteName == NoteName(pitch, true))
         break;
   }
   assert(pitch != 12);
   return pitch;
}

float Interp(float a, float start, float end)
{
   return a * (end-start) + start;
}

double GetPhaseInc(float freq)
{
   return freq * gTwoPiOverSampleRate;
}

void FloatWrap(float& num, float space)
{
   if (space == 0)
      num = 0;
   num -= space * floor(num/space);
}

void DrawText(string text, int x, int y, float size)
{
   gFont.DrawString(text, size, x, y);
}

void DrawTextLeftJustify(string text, int x, int y, float size)
{
   gFont.DrawString(text, size, x - GetStringWidth(text,size), y);
}

void DrawTextBold(string text, int x, int y, float size)
{
   gFontBold.DrawString(text, size, x, y);
}

float GetStringWidth(string text, float size)
{
   return gFont.GetStringWidth(text, size);
}

void AssertIfDenormal(float input)
{
   assert(input == 0 || fabsf(input) > numeric_limits<float>::min());
}

float GetInterpolatedSample(float offset, const float* buffer, int bufferSize)
{
   FloatWrap(offset, bufferSize);
   int pos = int(offset);
   int posNext = int(offset+1) % bufferSize;
   
   float sample = buffer[pos];
   float nextSample = buffer[posNext];
   float a = offset - pos;
   float output = (1-a)*sample + a*nextSample; //interpolate
   
   return output;
}

float GetInterpolatedSample(float offset, ChannelBuffer* buffer, int bufferSize, float channelBlend)
{
   assert(channelBlend <= buffer->NumActiveChannels());
   
   if (buffer->NumActiveChannels() == 1)
      return GetInterpolatedSample(offset, buffer->GetChannel(0), bufferSize);
   
   int channelA = floor(channelBlend);
   if (channelA == buffer->NumActiveChannels())
      channelA -= 1;
   int channelB = channelA + 1;
   
   return (1 - (channelBlend - channelA)) * GetInterpolatedSample(offset, buffer->GetChannel(channelA), bufferSize) +
          (channelBlend - channelA) * GetInterpolatedSample(offset, buffer->GetChannel(channelB), bufferSize);
}

void WriteInterpolatedSample(float offset, float* buffer, int bufferSize, float sample)
{
   FloatWrap(offset, bufferSize);
   int pos = int(offset);
   int posNext = int(offset+1) % bufferSize;

   float a = offset - pos;
   buffer[pos] += (1-a)*sample;
   buffer[posNext] += a*sample;
}

string GetRomanNumeralForDegree(int degree)
{
   string roman;
   switch ((degree + 700) % 7)
   {
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
   string targetName = "";
   if (audioSource)
   {
      for (int i=0; i<audioSource->GetNumTargets(); ++i)
      {
         IDrawableModule* target = dynamic_cast<IDrawableModule*>(audioSource->GetTarget(i));
         if (target)
            targetName = target->Path();
         module->GetSaveData().SetString("target"+(i==0 ? "" : ofToString(i+1)), targetName);
      }
   }
   if (noteSource || grid)
   {
      if (module->GetPatchCableSource())
      {
         const vector<PatchCable*>& cables = module->GetPatchCableSource()->GetPatchCables();
         for (int i=0; i<cables.size(); ++i)
         {
            PatchCable* cable = cables[i];
            IClickable* target = cable->GetTarget();
            if (target)
            {
               targetName += target->Path();
               if (i < cables.size()-1)
                  targetName += ",";
            }
         }
      }
      module->GetSaveData().SetString("target", targetName);
   }
}

void DrawLissajous(RollingBuffer* buffer, float x, float y, float w, float h, float r, float g, float b)
{
   ofPushStyle();
   ofSetLineWidth(1.5f);
   
   int secondChannel = 1;
   if (buffer->NumChannels() == 1)
      secondChannel = 0;
   
   ofSetColor(r*255,g*255,b*255, 70);
   ofBeginShape();
   const int delaySamps = 90;
   int numPoints = MIN(buffer->Size()-delaySamps-1, .02f*gSampleRate);
   for (int i=100; i < numPoints; ++i)
   {
      float vx = x + w/2 + buffer->GetSample(i, 0) * MAX(w,h);
      float vy = y + h/2 + buffer->GetSample(i+delaySamps, secondChannel) * MAX(w,h);
      //float alpha = 1 - (i/float(numPoints));
      //ofSetColor(r*255,g*255,b*255,alpha*alpha*255);
      ofVertex(vx,vy);
   }
   ofEndShape();
   
   ofPopStyle();
}

void StringCopy(char* dest, const char* source, int destLength)
{
   strncpy(dest, source, destLength);
   dest[destLength-1] = 0;
}

int GetKeyModifiers()
{
   ModifierKeys modifiers = ModifierKeys::getCurrentModifiers();
   int ret = 0;
   if (modifiers.isShiftDown())
      ret |= kModifier_Shift;
   if (modifiers.isAltDown())
      ret |= kModifier_Alt;
   if (modifiers.isCtrlDown())
      ret |= kModifier_Control;
   if (modifiers.isCommandDown())
      ret |= kModifier_Command;
   return ret;
}

bool IsKeyHeld(int key, int modifiers)
{
   return TextEntry::GetActiveTextEntry() == nullptr &&
          ofGetKeyPressed(key) &&
          GetKeyModifiers() == modifiers;
}

int KeyToLower(int key)
{
   if (CharacterFunctions::isLetter((char)key))
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
   return (end-start) * a * a + start;
}

float EaseOut(float start, float end, float a)
{
   return -(end-start) * a * (a-2) + start;
}

float Bias(float value, float bias)
{
   assert(bias >= 0 && bias <= 1);
   const float kLog25 = log(25);
   bias = .2f * expf(kLog25*bias);   //pow(25,bias)
   return pow(value, bias);
}

float Pow2(float in)
{
   const float kLog2 = log(2);
   return expf(kLog2*in);
}

void PrintCallstack()
{
#ifdef JUCE_MAC
   void *callstack[128];
   int frameCount = backtrace(callstack, 128);
   char **frameStrings = backtrace_symbols(callstack, frameCount);
   
   if ( frameStrings != nullptr ) {
      // Start with frame 1 because frame 0 is PrintBacktrace()
      for ( int i = 1; i < frameCount; i++ ) {
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

string GetUniqueName(string name, vector<IDrawableModule*> existing)
{
   string origName = name;
   while (origName.length() > 1 && CharacterFunctions::isDigit((char)origName[origName.length()-1]))
      origName.resize(origName.length()-1);
   int suffix = 1;
   while (true)
   {
      bool isNameUnique = true;
      for (int i=0; i<existing.size(); ++i)
      {
         if (existing[i]->Path() == name)
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

string GetUniqueName(string name, vector<string> existing)
{
   string origName = name;
   int suffix = 1;
   while (true)
   {
      bool isNameUnique = true;
      for (int i=0; i<existing.size(); ++i)
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
   for(hash = i = 0; key[i] != 0; ++i)
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

#ifdef BESPOKE_DEBUG_ALLOCATIONS
FILE* logAllocationsFile;

namespace
{
   const int maxFilenameLen = 90;
   typedef struct
   {
      uint32	address;
      uint32	size;
      char     file[maxFilenameLen];
      uint32	line;
      bool     allocated;
   } AllocInfo;
   
   bool enableTracking = false;
   
   template <class T>
   class my_allocator
   {
   public:
      typedef size_t    size_type;
      typedef ptrdiff_t difference_type;
      typedef T*        pointer;
      typedef const T*  const_pointer;
      typedef T&        reference;
      typedef const T&  const_reference;
      typedef T         value_type;
      
      my_allocator() {}
      my_allocator(const my_allocator&) {}
      
      pointer allocate(size_type n, const void * = 0)
      {
         T* t = (T*)malloc(n * sizeof(T));
         return t;
      }
      
      void deallocate(void* p, size_type)
      {
         if (p)
            free(p);
      }
      
      pointer           address(reference x) const { return &x; }
      const_pointer     address(const_reference x) const { return &x; }
      my_allocator<T>&  operator=(const my_allocator&) { return *this; }
#undef new
      void              construct(pointer p, const T& val) { new ((T*) p) T(val); }
#define new DEBUG_NEW
      void              destroy(pointer p) { p->~T(); }
      
      size_type         max_size() const { return size_t(-1); }
      
      template <class U>
      struct rebind { typedef my_allocator<U> other; };
      
      template <class U>
      my_allocator(const my_allocator<U>&) {}
      
      template <class U>
      my_allocator& operator=(const my_allocator<U>&) { return *this; }
   };
   
   map<uint32,AllocInfo*,less<int>,my_allocator<pair<uint32,AllocInfo*> > > allocList;
   
   void AddTrack(uint32 addr,  uint32 asize,  const char* fname, uint32 lnum)
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
   map<string,int> perFileTotal;
   
   for(auto element : allocList)
   {
      const AllocInfo* info = element.second;
      if (info && info->allocated)
      {
         sprintf(buf, "%-90s:  LINE %5d,  ADDRESS %08x  %8d unfreed",
                 info->file, info->line, info->address, info->size);
         ofLog() << buf;
         totalSize += info->size;
         
         perFileTotal[info->file] += info->size;
      }
   }
   ofLog() << "-----------------------------------------------------------";
   for(auto fileInfo : perFileTotal)
   {
      sprintf(buf, "%-90s:  %10d unfreed",
              fileInfo.first.c_str(), fileInfo.second);
      ofLog() << buf;
   }
   ofLog() << "-----------------------------------------------------------";
   sprintf(buf, "Total Unfreed: %d bytes", totalSize);
   ofLog() << buf;
};

#undef new
void* operator new(std::size_t size) throw(std::bad_alloc)
{
   void *ptr = (void*)malloc(size);
   //AddTrack((uint32)ptr, size, "<unknown>", 0);
   return(ptr);
}
void* operator new(std::size_t size, const char *file, int line) throw(std::bad_alloc)
{
   void *ptr = (void*)malloc(size);
   AddTrack((uint32)ptr, size, file, line);
   return(ptr);
}
void operator delete(void* p) throw()
{
   //RemoveTrack((uint32)p);
   free(p);
}
void* operator new[](std::size_t size) throw(std::bad_alloc)
{
   void *ptr = (void*)malloc(size);
   //AddTrack((uint32)ptr, size, "<unknown>", 0);
   return(ptr);
}
void* operator new[](std::size_t size, const char *file, int line) throw(std::bad_alloc)
{
   void* ptr = (void*)malloc(size);
   AddTrack((uint32)ptr, size, file, line);
   return(ptr);
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
