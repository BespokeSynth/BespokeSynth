#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <assert.h>
#include <map>
#include <vector>
#include <list>

using namespace std;

class NVGcontext;

extern NVGcontext* gNanoVG;
extern NVGcontext* gFontBoundsNanoVG;

struct ofColor
{
   ofColor() : ofColor(1,1,1) {}
   ofColor(int _r, int _g, int _b) : r(_r), g(_g), b(_b), a(255) {}
   ofColor(int _r, int _g, int _b, int _a) : r(_r), g(_g), b(_b), a(_a) {}
   void set(int _r, int _g, int _b, int _a = 255) { r=_r; g=_g; b=_b; a=_a; }
   void setBrightness(int _bright);
   int getBrightness() const;
   void setSaturation(int _sat);
   int getSaturation() const;
   void getHsb(float& h, float& s, float& b) const;
   void setHsb(int h, int s, int b);
   ofColor operator*(const ofColor& other);
   ofColor operator*(float f);
   ofColor operator+(const ofColor& other);
   int r;
   int g;
   int b;
   int a;
   
   static ofColor black,white,grey,red,green,yellow,blue,orange,purple,lime;
};

struct ofVec2f
{
   ofVec2f() : x(0), y(0) {}
   ofVec2f(float _x, float _y) : x(_x), y(_y) {}
   void set(float _x, float _y) { x = _x; y = _y; }
   ofVec2f operator-(const ofVec2f& other)
   {
      return ofVec2f(x - other.x, y-other.y);
   }
   ofVec2f operator+(const ofVec2f& other)
   {
      return ofVec2f(x + other.x, y+other.y);
   }
   float lengthSquared() const
   {
      return x*x + y*y;
   }
   float distanceSquared() const
   {
      return x*x + y*y;
   }
   float distanceSquared(ofVec2f other) const
   {
      return (x-other.x)*(x-other.x) + (y-other.y)*(y-other.y);
   }
   float dot( const ofVec2f& vec ) const
   {
	   return x*vec.x + y*vec.y;
   }
   ofVec2f operator*(float f)
   {
      return ofVec2f(x*f, y*f);
   }
   ofVec2f operator/(float f)
   {
      return ofVec2f(x/f, y/f);
   }
   ofVec2f& operator-=(const ofVec2f& other)
   {
      x -= other.x;
      y -= other.y;
      return *this;
   }
   ofVec2f& operator+=(const ofVec2f& other)
   {
      x += other.x;
      y += other.y;
      return *this;
   }
   float x;
   float y;
};

struct ofVec3f
{
   ofVec3f() : x(0), y(0), z(0) {}
   ofVec3f(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
   float length() const
   {
      return sqrt(x*x + y*y + z*z);
   }
   float x;
   float y;
   float z;
};

struct ofRectangle
{
   ofRectangle() : x(0), y(0), width(100), height(100) {}
   ofRectangle(float _x, float _y, float _w, float _h) : x(_x), y(_y), width(_w), height(_h)
   {
   }
   ofRectangle(ofVec2f p1, ofVec2f p2)
   {
      set(p1.x,p1.y,p2.x-p1.x,p2.y-p1.y);
   }
   void set(float _x, float _y, float _w, float _h) { x = _x; y = _y; width = _w; height = _h; }
   bool intersects(const ofRectangle& other) const;
   bool contains(float x, float y) const;
   float getMinX() const;
   float getMaxX() const;
   float getMinY() const;
   float getMaxY() const;
   ofVec2f getCenter() const { return ofVec2f(x + width * .5f, y + height * .5f); }
   float x;
   float y;
   float width;
   float height;
};

class ofMutex
{
public:
   void lock()
   {
      mCritSec.enter();
   }
   void unlock()
   {
      mCritSec.exit();
   }
   CriticalSection mCritSec;
};

#define CLAMP(v,a,b) (v<a ? a : (v>b ? b : v))


#define MAX(a,b) (a>b?a:b)
#define MIN(a,b) (a<b?a:b)

#define OF_KEY_RETURN KeyPress::returnKey
#define OF_KEY_TAB KeyPress::tabKey
#define OF_KEY_BACKSPACE KeyPress::backspaceKey
#define OF_KEY_LEFT KeyPress::leftKey
#define OF_KEY_RIGHT KeyPress::rightKey
#define OF_KEY_ESC KeyPress::escapeKey
#define OF_KEY_UP KeyPress::upKey
#define OF_KEY_DOWN KeyPress::downKey

template <class T>
inline std::string ofToString(const T& value)
{
   std::ostringstream out;
   out << value;
   return out.str();
}

template <class T>
inline std::string ofToString(const T& value, int precision)
{
   std::ostringstream out;
   out << std::fixed << std::setprecision(precision) << value;
   return out.str();
}

namespace Poco
{
   namespace FastMutex
   {
      class ScopedLock
      {
      public:
         ScopedLock(ofMutex& mutex)
         : mMutex(&mutex)
         {
            mMutex->lock();
         }
         ~ScopedLock()
         {
            mMutex->unlock();
         }
      private:
         ofMutex* mMutex;
      };
   }
}



struct ofDragInfo
{
   vector<string> files;
};

struct ofMessage
{
   
};



class ofSoundStream
{
public:
   void setup(void*, int outChannels, int inChannels, int sampleRate, int bufferSize, int numBuffers) {}
   int getTickCount() { return 0; }
   int GetLastStarvationTick() { return 0; }
   void stop() {}
};

#define PI 3.14159265358979
#define TWO_PI PI*2

class RetinaTrueTypeFont
{
public:
   RetinaTrueTypeFont() : mLoaded(false) {}
   void LoadFont(string path);
   void DrawString(string str, float size, float x, float y);
   ofRectangle DrawStringWrap(string str, float size, float x, float y, float width);
   float GetStringWidth(string str, float size, bool isRenderThread = false);
   float GetStringHeight(string str, float size, bool isRenderThread = false);
   bool IsLoaded() { return mLoaded; }
private:
   int mFontHandle;
   int mFontBoundsHandle;
   bool mLoaded;
};

struct ofFileDialogResult
{
   bool bSuccess;
   string filePath;
};

struct ofEventArgs
{
};


typedef ofVec2f ofPoint;

string ofToDataPath(string path, bool makeAboslute = false);
void ofPushStyle();
void ofPopStyle();
void ofPushMatrix();
void ofPopMatrix();
void ofTranslate(float x, float y, float z=0);
void ofClipWindow(float x, float y, float width, float height);
void ofSetColor(float r, float g, float b, float a=255);
void ofSetColor(float grey);
void ofSetColor(const ofColor& color);
void ofSetColor(const ofColor& color, float a);
void ofFill();
void ofNoFill();
void ofCircle(float x, float y, float radius);
void ofRect(float x, float y, float width, float height, float cornerRadius = 3);
void ofRect(const ofRectangle& rect, float cornerRadius = 3);
float ofClamp(float val, float a, float b);
float ofGetLastFrameTime();
int ofToInt(const string& intString);
float ofToFloat(const string& floatString);
int ofHexToInt(const string& hexString);
float ofGetMouseX();
float ofGetMouseY();
void ofLine(float x1, float y1, float x2, float y2);
void ofLine(ofVec2f v1, ofVec2f v2);
void ofSetLineWidth(float width);
void ofBeginShape();
void ofEndShape(bool close = false);
void ofVertex(float x, float y, float z=0);
float ofMap(float val, float fromStart, float fromEnd, float toStart, float toEnd, bool clamp = false);
float ofRandom(float max);
float ofRandom(float x, float y);
void ofSetCircleResolution(float res);
unsigned long long ofGetSystemTimeNanos();
float ofGetWidth();
float ofGetHeight();
float ofGetFrameRate();
float ofLerp(float start, float stop, float amt);
float ofDistSquared(float x1, float y1, float x2, float y2);
vector<string> ofSplitString(string str, string splitter, bool ignoreEmpty = false, bool trim = false);
bool ofIsStringInString(const string& haystack, const string& needle);
void ofScale(float x, float y, float z);
void ofExit();
void ofToggleFullscreen();
void ofStringReplace(string& str, string from, string to, bool firstOnly = false);
string ofGetTimestampString(string in);
void ofTriangle(float x1, float y1, float x2, float y2, float x3, float y3);


