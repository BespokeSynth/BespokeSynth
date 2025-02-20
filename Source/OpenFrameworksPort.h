#pragma once

#include <sstream>
#include <iomanip>
#include <assert.h>
#include <map>
#include <vector>
#include <list>
#include <cmath>
#include <mutex>
#include <random>

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
   void getHsb(double& h, double& s, double& b) const;
   void setHsb(int h, int s, int b);
   ofColor operator*(const ofColor& other);
   ofColor operator*(double f);
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
   template <typename TCast>
   operator ofVec2<TCast>()
   {
      return ofVec2<TCast>(x, y);
   }
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

#ifdef min // #Blame minwindef.h
#undef min
#endif
#ifdef max // #Blame minwindef.h
#undef max
#endif

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

typedef ofRectangle_t<float> ofRectangle_f;
typedef ofRectangle_t<double> ofRectangle;

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
bool ofAlmostEquel(const T& a, const T& b, T epsilon = std::numeric_limits<T>::quiet_NaN())
{
   if (std::is_integral_v<T>) // In case someone uses this to compare integral values
      return a == b;
   // Chosen a different value for epsilon compared to std::numeric_limits<T>::epsilon() so that floating point values around 100000 still work correctly since the "vanilla" epsilon is based around a value of 1.
   if (std::is_same_v<T, float> && std::isnan(epsilon))
      epsilon = 0.001f;
   else if (std::is_same_v<T, double> && std::isnan(epsilon))
      epsilon = 0.00000000001;
   else if (std::is_same_v<T, long double> && std::isnan(epsilon))
      epsilon = 0.00000000000000000000000001L;
   if (epsilon == std::numeric_limits<T>::quiet_NaN()) // float128 or some such? std::float128_t is C++23 and we haven't switched to c++20 yet. But instead of returning something definitely incorrect we use the types epsilon instead.
      epsilon = std::numeric_limits<T>::epsilon();
   return std::abs(a - b) < epsilon;
}

template <class T, class U>
bool ofAlmostEquel(const T& a, const U& b, T epsilon = std::numeric_limits<T>::quiet_NaN())
{
   return ofAlmostEquel(a, static_cast<T>(b), epsilon);
}

#define PI 3.14159265358979323846264338327
#define TWO_PI 6.28318530717958647692528676654

class RetinaTrueTypeFont
{
public:
   RetinaTrueTypeFont() {}
   void LoadFont(std::string path);
   void DrawString(std::string str, double size, double x, double y);
   ofRectangle DrawStringWrap(std::string str, double size, double x, double y, double width);
   double GetStringWidth(std::string str, double size);
   double GetStringHeight(std::string str, double size);
   bool IsLoaded() { return mLoaded; }
   int GetFontHandle() const { return mFontHandle; }
   std::string GetFontPath() const { return mFontPath; }

private:
   int mFontHandle{};
   int mFontBoundsHandle{};
   bool mLoaded{ false };
   std::string mFontPath;
};

typedef ofVec2d ofPoint;

std::string ofToSamplePath(const std::string& path);
std::string ofToDataPath(const std::string& path);
std::string ofToFactoryPath(const std::string& path);
std::string ofToResourcePath(const std::string& path);
void ofPushStyle();
void ofPopStyle();
void ofPushMatrix();
void ofPopMatrix();
void ofTranslate(double x, double y, double z = 0);
void ofRotate(double radians);
void ofClipWindow(double x, double y, double width, double height, bool intersectWithExisting);
void ofResetClipWindow();
void ofSetColor(double r, double g, double b, double a = 255);
void ofSetColor(double grey);
void ofSetColor(const ofColor& color);
void ofSetColor(const ofColor& color, double a);
void ofSetColorGradient(const ofColor& colorA, const ofColor& colorB, ofVec2d gradientStart, ofVec2d gradientEnd);
void ofFill();
void ofNoFill();
void ofCircle(double x, double y, double radius);
void ofRect(double x, double y, double width, double height, double cornerRadius = 3);
void ofRect(const ofRectangle& rect, double cornerRadius = 3);

template <class T, class U, class V>
T ofClamp(const T val, const U a, const V b)
{
   if (val < a)
      return static_cast<T>(a);
   if (val > b)
      return static_cast<T>(b);
   return val;
}

ofVec2d ofPolToCar(double pos, double radius);
ofVec2d ofCarToPol(double x, double y);
double ofGetLastFrameTime();
int ofToInt(const std::string& intString);
float ofToFloat(const std::string& floatString);
double ofToDouble(const std::string& doubleString);
int ofHexToInt(const std::string& hexString);
void ofLine(double x1, double y1, double x2, double y2);
void ofLine(ofVec2f v1, ofVec2f v2);
void ofSetLineWidth(double width);
void ofBeginShape();
void ofEndShape(bool close = false);
void ofVertex(double x, double y, double z = 0);
void ofVertex(ofVec2d point);

template <class T>
T ofMap(T val, T fromStart, T fromEnd, T toStart, T toEnd, bool clamp = false)
{
   T ret;
   if (fromEnd - fromStart != 0)
      ret = ((val - fromStart) / (fromEnd - fromStart)) * (toEnd - toStart) + toStart;
   else
      ret = toEnd;
   if (clamp)
      ret = ofClamp(ret, MIN(toStart, toEnd), MAX(toStart, toEnd));
   return ret;
}

template <class T, class T1, class T2, class T3, class T4>
T ofMap(T val, T1 fromStart, T2 fromEnd, T3 toStart, T4 toEnd, bool clamp = false)
{
   return ofMap(val, static_cast<T>(fromStart), static_cast<T>(fromEnd), static_cast<T>(toStart), static_cast<T>(toEnd), clamp);
}

double ofRandom(double max);
double ofRandom(double x, double y);
void ofSetCircleResolution(double res);
unsigned long long ofGetSystemTimeNanos();
double ofGetWidth();
double ofGetHeight();
double ofGetFrameRate();

template <class T>
T ofLerp(T start, T stop, T amt)
{
   return start + (stop - start) * amt;
}
template <class T, class T1, class T2>
T ofLerp(T start, T1 stop, T2 amt)
{
   return ofLerp(start, static_cast<T>(stop), static_cast<T>(amt));
}

template <class T>
T ofDistSquared(T x1, T y1, T x2, T y2)
{
   return ((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}
template <class T, class T1, class T2, class T3>
T ofDistSquared(T x1, T1 y1, T2 x2, T3 y2)
{
   return ofDistSquared(x1, static_cast<T>(y1), static_cast<T>(x2), static_cast<T>(y2));
}

std::vector<std::string> ofSplitString(std::string str, std::string splitter, bool ignoreEmpty = false, bool trim = false);
bool ofIsStringInString(const std::string& haystack, const std::string& needle);
void ofScale(double x, double y, double z);
void ofExit();
void ofToggleFullscreen();
void ofStringReplace(std::string& str, std::string from, std::string to, bool firstOnly = false);
std::string ofGetTimestampString(std::string in);
void ofTriangle(double x1, double y1, double x2, double y2, double x3, double y3);
