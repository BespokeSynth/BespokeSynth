#pragma once

#include <sstream>
#include <iomanip>
#include <assert.h>
#include <map>
#include <vector>
#include <list>
#include <cmath>
#include <mutex>

class NVGcontext;

extern NVGcontext* gNanoVG;
extern NVGcontext* gFontBoundsNanoVG;

struct ofColor
{
   ofColor()
   : ofColor(255, 255, 255)
   {}
   ofColor(int _r, int _g, int _b)
   : r(_r)
   , g(_g)
   , b(_b)
   {}
   ofColor(int _r, int _g, int _b, int _a)
   : r(_r)
   , g(_g)
   , b(_b)
   , a(_a)
   {}
   void set(int _r, int _g, int _b, int _a = 255)
   {
      r = _r;
      g = _g;
      b = _b;
      a = _a;
   }
   void setBrightness(int _bright);
   int getBrightness() const;
   void setSaturation(int _sat);
   int getSaturation() const;
   void getHsb(float& h, float& s, float& b) const;
   void setHsb(int h, int s, int b);
   ofColor operator*(const ofColor& other);
   ofColor operator*(float f);
   ofColor operator+(const ofColor& other);
   int r{ 0 };
   int g{ 0 };
   int b{ 0 };
   int a{ 255 };

   static ofColor black, white, grey, red, green, yellow, blue, orange, purple, lime, magenta, cyan, clear;

   static ofColor lerp(ofColor a, ofColor b, float t)
   {
      return ofColor(int(a.r * (1 - t) + b.r * t), int(a.g * (1 - t) + b.g * t), int(a.b * (1 - t) + b.b * t), int(a.a * (1 - t) + b.a * t));
   }
};

struct ofVec2f
{
   ofVec2f()
   {}
   ofVec2f(float _x, float _y)
   : x(_x)
   , y(_y)
   {}
   void set(float _x, float _y)
   {
      x = _x;
      y = _y;
   }
   ofVec2f operator-(const ofVec2f& other)
   {
      return ofVec2f(x - other.x, y - other.y);
   }
   ofVec2f operator+(const ofVec2f& other)
   {
      return ofVec2f(x + other.x, y + other.y);
   }
   float lengthSquared() const
   {
      return x * x + y * y;
   }
   float distanceSquared() const
   {
      return x * x + y * y;
   }
   float distanceSquared(ofVec2f other) const
   {
      return (x - other.x) * (x - other.x) + (y - other.y) * (y - other.y);
   }
   float dot(const ofVec2f& vec) const
   {
      return x * vec.x + y * vec.y;
   }
   ofVec2f operator*(float f)
   {
      return ofVec2f(x * f, y * f);
   }
   ofVec2f operator/(float f)
   {
      return ofVec2f(x / f, y / f);
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
   float x{ 0 };
   float y{ 0 };
};

struct ofVec3f
{
   ofVec3f()
   {}
   ofVec3f(float _x, float _y, float _z)
   : x(_x)
   , y(_y)
   , z(_z)
   {}
   float length() const
   {
      return sqrt(x * x + y * y + z * z);
   }
   float x{ 0 };
   float y{ 0 };
   float z{ 0 };
};

struct ofRectangle
{
   ofRectangle()
   {}
   ofRectangle(float _x, float _y, float _w, float _h)
   : x(_x)
   , y(_y)
   , width(_w)
   , height(_h)
   {
   }
   ofRectangle(ofVec2f p1, ofVec2f p2)
   {
      set(p1.x, p1.y, p2.x - p1.x, p2.y - p1.y);
   }
   void set(float _x, float _y, float _w, float _h)
   {
      x = _x;
      y = _y;
      width = _w;
      height = _h;
   }
   bool intersects(const ofRectangle& other) const;
   bool contains(float x, float y) const;
   ofRectangle& grow(float amount)
   {
      x -= amount;
      y -= amount;
      width += amount * 2;
      height += amount * 2;
      return *this;
   }
   static ofRectangle include(const ofRectangle& a, const ofRectangle& b);
   float getMinX() const;
   float getMaxX() const;
   float getMinY() const;
   float getMaxY() const;
   ofVec2f getCenter() const { return ofVec2f(x + width * .5f, y + height * .5f); }
   float x{ 0 };
   float y{ 0 };
   float width{ 100 };
   float height{ 100 };
};

using ofMutex = std::recursive_mutex;

#define CLAMP(v, a, b) (v < a ? a : (v > b ? b : v))


#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)

#define OF_KEY_RETURN juce::KeyPress::returnKey
#define OF_KEY_TAB juce::KeyPress::tabKey
#define OF_KEY_LEFT juce::KeyPress::leftKey
#define OF_KEY_RIGHT juce::KeyPress::rightKey
#define OF_KEY_ESC juce::KeyPress::escapeKey
#define OF_KEY_UP juce::KeyPress::upKey
#define OF_KEY_DOWN juce::KeyPress::downKey

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

#define PI 3.14159265358979323846
#define TWO_PI 6.28318530717958647693

class RetinaTrueTypeFont
{
public:
   RetinaTrueTypeFont() {}
   void LoadFont(std::string path);
   void DrawString(std::string str, float size, float x, float y);
   ofRectangle DrawStringWrap(std::string str, float size, float x, float y, float width);
   float GetStringWidth(std::string str, float size);
   float GetStringHeight(std::string str, float size);
   bool IsLoaded() { return mLoaded; }
   int GetFontHandle() const { return mFontHandle; }
   std::string GetFontPath() const { return mFontPath; }

private:
   int mFontHandle{};
   int mFontBoundsHandle{};
   bool mLoaded{ false };
   std::string mFontPath;
};

typedef ofVec2f ofPoint;

std::string ofToSamplePath(const std::string& path);
std::string ofToDataPath(const std::string& path);
std::string ofToFactoryPath(const std::string& path);
std::string ofToResourcePath(const std::string& path);
void ofPushStyle();
void ofPopStyle();
void ofPushMatrix();
void ofPopMatrix();
void ofTranslate(float x, float y, float z = 0);
void ofRotate(float radians);
void ofClipWindow(float x, float y, float width, float height, bool intersectWithExisting);
void ofResetClipWindow();
void ofSetColor(float r, float g, float b, float a = 255);
void ofSetColor(float grey);
void ofSetColor(const ofColor& color);
void ofSetColor(const ofColor& color, float a);
void ofSetColorGradient(const ofColor& colorA, const ofColor& colorB, ofVec2f gradientStart, ofVec2f gradientEnd);
void ofFill();
void ofNoFill();
void ofCircle(float x, float y, float radius);
void ofRect(float x, float y, float width, float height, float cornerRadius = 3);
void ofRect(const ofRectangle& rect, float cornerRadius = 3);
float ofClamp(float val, float a, float b);
float ofGetLastFrameTime();
int ofToInt(const std::string& intString);
float ofToFloat(const std::string& floatString);
double ofToDouble(const std::string& doubleString);
int ofHexToInt(const std::string& hexString);
void ofLine(float x1, float y1, float x2, float y2);
void ofLine(ofVec2f v1, ofVec2f v2);
void ofSetLineWidth(float width);
void ofBeginShape();
void ofEndShape(bool close = false);
void ofVertex(float x, float y, float z = 0);
void ofVertex(ofVec2f point);
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
std::vector<std::string> ofSplitString(std::string str, std::string splitter, bool ignoreEmpty = false, bool trim = false);
bool ofIsStringInString(const std::string& haystack, const std::string& needle);
void ofScale(float x, float y, float z);
void ofExit();
void ofToggleFullscreen();
void ofStringReplace(std::string& str, std::string from, std::string to, bool firstOnly = false);
std::string ofGetTimestampString(std::string in);
void ofTriangle(float x1, float y1, float x2, float y2, float x3, float y3);
