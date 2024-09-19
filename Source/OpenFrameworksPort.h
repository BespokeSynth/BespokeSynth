#pragma once

#include <iostream>
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

template <typename T>
struct ofVec2
{
   ofVec2()
   {}
   ofVec2(T _x, T _y)
   : x(_x)
   , y(_y)
   {}
   void set(T _x, T _y)
   {
      x = _x;
      y = _y;
   }
   ofVec2 operator-(const ofVec2& other)
   {
      return ofVec2(x - other.x, y - other.y);
   }
   ofVec2 operator+(const ofVec2& other)
   {
      return ofVec2(x + other.x, y + other.y);
   }
   T lengthSquared() const
   {
      return x * x + y * y;
   }
   T distanceSquared() const
   {
      return x * x + y * y;
   }
   T distanceSquared(ofVec2 other) const
   {
      return (x - other.x) * (x - other.x) + (y - other.y) * (y - other.y);
   }
   T dot(const ofVec2& vec) const
   {
      return x * vec.x + y * vec.y;
   }
   ofVec2 operator*(T f)
   {
      return ofVec2(x * f, y * f);
   }
   ofVec2 operator/(T f)
   {
      return ofVec2(x / f, y / f);
   }
   //template <typename TCast>
   //operator ofVec2<TCast>()
   //{
   //   return ofVec2<TCast>(x , y );
   //}
   ofVec2& operator-=(const ofVec2& other)
   {
      x -= other.x;
      y -= other.y;
      return *this;
   }
   ofVec2& operator+=(const ofVec2& other)
   {
      x += other.x;
      y += other.y;
      return *this;
   }
   T x{ 0 };
   T y{ 0 };
};

typedef ofVec2<float> ofVec2f;
typedef ofVec2<double> ofVec2d;

template <class T>
struct ofVec3
{
   ofVec3()
   {}
   ofVec3(T _x, T _y, T _z)
   : x(_x)
   , y(_y)
   , z(_z)
   {}
   T length() const
   {
      return sqrt(x * x + y * y + z * z);
   }
   T x{ 0 };
   T y{ 0 };
   T z{ 0 };
};

typedef ofVec3<float> ofVec3f;
typedef ofVec3<double> ofVec3d;

template <class T>
struct ofRectangle_t
{
   ofRectangle_t()
   {}
   ofRectangle_t(T _x, T _y, T _w, T _h)
   : x(_x)
   , y(_y)
   , width(_w)
   , height(_h)
   {
   }
   ofRectangle_t(ofVec2<T> p1, ofVec2<T> p2)
   {
      set(p1.x, p1.y, p2.x - p1.x, p2.y - p1.y);
   }
   void set(T _x, T _y, T _w, T _h)
   {
      x = _x;
      y = _y;
      width = _w;
      height = _h;
   }
   bool intersects(const ofRectangle_t& other) const
   {
      return (getMinX() < other.getMaxX() && getMaxX() > other.getMinX() &&
              getMinY() < other.getMaxY() && getMaxY() > other.getMinY());
   }
   bool contains(T testX, T testY) const
   {
      return testX > getMinX() && testY > getMinY() &&
             testX < getMaxX() && testY < getMaxY();
   }
   ofRectangle_t& grow(T amount)
   {
      x -= amount;
      y -= amount;
      width += amount * 2;
      height += amount * 2;
      return *this;
   }
   T getMinX() const
   {
      return std::min(x, x + width); // - width
   }
   T getMaxX() const
   {
      return std::max(x, x + width); // - width
   }
   T getMinY() const
   {
      return std::min(y, y + height); // - height
   }
   T getMaxY() const
   {
      return std::max(y, y + height); // - height
   }
   ofVec2<T> getCenter() const { return ofVec2<T>(x + width * .5, y + height * .5); }
   T x{ 0 };
   T y{ 0 };
   T width{ 100 };
   T height{ 100 };
};

typedef ofRectangle_t<float> ofRectangle;
typedef ofRectangle_t<double> ofRectangle_d;

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

template <class T>
inline bool ofAlmostEquel(const T& a, const T& b, T epsilon = std::numeric_limits<T>::quiet_NaN())
{
   if (std::is_same_v<T, float> && std::isnan<T>(epsilon))
      epsilon = 0.0001f;
   else if (std::is_same_v<T, double> && std::isnan<T>(epsilon))
      epsilon = 0.0000000000001;
   else if (std::is_same_v<T, long double> && std::isnan<T>(epsilon))
      epsilon = 0.00000000000000000000000001L;
   return std::fabs(a - b) < epsilon;
}

//inline bool ofAlmostEquel(const double& a, const double& b, const double epsilon = 0.000000001)
//{
//   return fabs(a - b) < epsilon;
//}

#define PI 3.14159265358979323846264338327
#define TWO_PI 6.28318530717958647692528676654

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
double ofClamp(double val, double a, double b);
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
double ofMap(double val, double fromStart, double fromEnd, double toStart, double toEnd, bool clamp = false);
double ofRandom(double max);
double ofRandom(double x, double y);
void ofSetCircleResolution(float res);
unsigned long long ofGetSystemTimeNanos();
float ofGetWidth();
float ofGetHeight();
float ofGetFrameRate();
double ofLerp(double start, double stop, double amt);
float ofDistSquared(float x1, float y1, float x2, float y2);
std::vector<std::string> ofSplitString(std::string str, std::string splitter, bool ignoreEmpty = false, bool trim = false);
bool ofIsStringInString(const std::string& haystack, const std::string& needle);
void ofScale(float x, float y, float z);
void ofExit();
void ofToggleFullscreen();
void ofStringReplace(std::string& str, std::string from, std::string to, bool firstOnly = false);
std::string ofGetTimestampString(std::string in);
void ofTriangle(float x1, float y1, float x2, float y2, float x3, float y3);
