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
/*
 ==============================================================================

 OpenFrameworksPort.cpp
 Created: 30 May 2016 9:13:01pm
 Author:  Ryan Challinor

 ==============================================================================
 */

#ifdef BESPOKE_WINDOWS
#include <windows.h>
#endif

#include "juce_opengl/juce_opengl.h"
using namespace juce::gl;
using namespace juce;
#include <VersionInfo.h>

//#include <chrono>
#include <time.h>

#include "OpenFrameworksPort.h"
#include "nanovg/nanovg.h"
#define NANOVG_GLES2_IMPLEMENTATION
#include "nanovg/nanovg_gl.h"
#include "ModularSynth.h"
#include "Push2Control.h"
#include "UserData.h"

ofColor ofColor::black(0, 0, 0);
ofColor ofColor::white(255, 255, 255);
ofColor ofColor::grey(128, 128, 128);
ofColor ofColor::red(255, 0, 0);
ofColor ofColor::green(0, 200, 0);
ofColor ofColor::yellow(255, 255, 0);
ofColor ofColor::orange(255, 165, 0);
ofColor ofColor::blue(0, 0, 255);
ofColor ofColor::purple(148, 0, 211);
ofColor ofColor::lime(0, 255, 0);
ofColor ofColor::magenta(255, 0, 255);
ofColor ofColor::cyan(0, 255, 255);
ofColor ofColor::clear(0, 0, 0, 0);

NVGcontext* gNanoVG = nullptr;
NVGcontext* gFontBoundsNanoVG = nullptr;

std::string ofToDataPath(const std::string& path)
{
   if (!path.empty() && (path[0] == '.' || juce::File::isAbsolutePath(path)))
      return path;

   static const auto sDataDir = []
   {
      auto defaultDataDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("BespokeSynth").getFullPathName().toStdString();
      auto dataDir = juce::SystemStats::getEnvironmentVariable("BESPOKE_DATA_DIR", defaultDataDir).toStdString();
#if BESPOKE_WINDOWS
      std::replace(begin(dataDir), end(dataDir), '\\', '/');
#endif
      UpdateUserData(dataDir);
      dataDir += '/';
      return dataDir;
   }();

   return sDataDir + path;
}

std::string ofToFactoryPath(const std::string& subdir)
{
   std::string result;
#if BESPOKE_MAC
   auto resDir = juce::File::getSpecialLocation(juce::File::SpecialLocationType::currentApplicationFile).getChildFile("Contents/Resources").getChildFile(subdir);
#else
   auto resDir = juce::File::getSpecialLocation(juce::File::SpecialLocationType::currentExecutableFile).getSiblingFile(subdir);
#if BESPOKE_LINUX
   if (!resDir.isDirectory())
   {
      resDir = juce::File::getSpecialLocation(juce::File::SpecialLocationType::currentApplicationFile).getChildFile("../../share/BespokeSynth").getChildFile(subdir);
      if (!resDir.isDirectory())
         resDir = juce::File{ juce::CharPointer_UTF8{ Bespoke::CMAKE_INSTALL_PREFIX } }.getChildFile("share/BespokeSynth").getChildFile(subdir);
   }
#endif
#endif
   if (!resDir.isDirectory())
      throw std::runtime_error{ "Application directory not found. Please reinstall Bespoke." };

   result = resDir.getFullPathName().toStdString();
#if BESPOKE_WINDOWS
   std::replace(begin(result), end(result), '\\', '/');
#endif
   return result;
}

std::string ofToResourcePath(const std::string& path)
{
   if (!path.empty() && (path[0] == '.' || juce::File::isAbsolutePath(path)))
      return path;

   static const auto sResourceDir = ofToFactoryPath("resource") + '/';
   return sResourceDir + path;
}

struct StyleStack
{
   struct Style
   {
      Style()
      : fill(false)
      , color(255, 255, 255)
      , lineWidth(1)
      {}
      bool fill;
      ofColor color;
      float lineWidth;
   };

   StyleStack() { stack.push_front(Style()); }
   void Push() { stack.push_front(GetStyle()); }
   void Pop() { stack.pop_front(); }
   Style& GetStyle() { return *stack.begin(); }

   std::list<Style> stack;
};

static StyleStack sStyleStack;

void ofPushStyle()
{
   sStyleStack.Push();
}

void ofPopStyle()
{
   sStyleStack.Pop();

   ofSetColor(sStyleStack.GetStyle().color);
   ofSetLineWidth(sStyleStack.GetStyle().lineWidth);
}

void ofPushMatrix()
{
   nvgSave(gNanoVG);
}

void ofPopMatrix()
{
   nvgRestore(gNanoVG);
}

void ofTranslate(float x, float y, float z)
{
   nvgTranslate(gNanoVG, x, y);
}

void ofRotate(float radians)
{
   nvgRotate(gNanoVG, radians);
}

void ofClipWindow(float x, float y, float width, float height, bool intersectWithExisting)
{
   if (intersectWithExisting)
      nvgIntersectScissor(gNanoVG, x, y, width, height);
   else
      nvgScissor(gNanoVG, x, y, width, height);
}

void ofResetClipWindow()
{
   nvgResetScissor(gNanoVG);
}

void ofSetColor(float r, float g, float b, float a)
{
   sStyleStack.GetStyle().color = ofColor(r, g, b, a);
   if (Push2Control::sDrawingPush2Display)
   {
      nvgStrokeColor(gNanoVG, nvgRGBA(r, g, b, a));
      nvgFillColor(gNanoVG, nvgRGBA(r, g, b, a));
      return;
   }
   static int sImage = -1;
   if (sImage == -1)
      sImage = nvgCreateImage(gNanoVG, ofToResourcePath("noise.jpg").c_str(), NVG_IMAGE_REPEATX | NVG_IMAGE_REPEATY);
   NVGpaint pattern = nvgImagePattern(gNanoVG, ofRandom(0, 10), ofRandom(0, 10), 300 / gDrawScale / TheSynth->GetPixelRatio(), 300 / gDrawScale / TheSynth->GetPixelRatio(), ofRandom(0, 10), sImage, a);
   pattern.innerColor = nvgRGBA(r, g, b, a);
   pattern.outerColor = nvgRGBA(r, g, b, a);
   nvgStrokePaint(gNanoVG, pattern);
   nvgFillPaint(gNanoVG, pattern);
}

void ofSetColorGradient(const ofColor& colorA, const ofColor& colorB, ofVec2f gradientStart, ofVec2f gradientEnd)
{
   sStyleStack.GetStyle().color = colorA;
   if (Push2Control::sDrawingPush2Display)
   {
      nvgStrokeColor(gNanoVG, nvgRGBA(colorA.r, colorA.g, colorA.b, colorA.a));
      nvgFillColor(gNanoVG, nvgRGBA(colorA.r, colorA.g, colorA.b, colorA.a));
      return;
   }
   NVGpaint pattern = nvgLinearGradient(gNanoVG, gradientStart.x, gradientStart.y, gradientEnd.x, gradientEnd.y, nvgRGBA(colorA.r, colorA.g, colorA.b, colorA.a), nvgRGBA(colorB.r, colorB.g, colorB.b, colorB.a));
   nvgStrokePaint(gNanoVG, pattern);
   nvgFillPaint(gNanoVG, pattern);
}

void ofSetColor(float grey)
{
   ofSetColor(grey, grey, grey);
}

void ofSetColor(const ofColor& color)
{
   ofSetColor(color.r, color.g, color.b, color.a);
}

void ofSetColor(const ofColor& color, float a)
{
   ofSetColor(color.r, color.g, color.b, a);
}

void ofFill()
{
   sStyleStack.GetStyle().fill = true;
}

void ofNoFill()
{
   sStyleStack.GetStyle().fill = false;
}

void ofCircle(float x, float y, float radius)
{
   nvgBeginPath(gNanoVG);
   nvgCircle(gNanoVG, x, y, radius);
   if (sStyleStack.GetStyle().fill)
      nvgFill(gNanoVG);
   else
      nvgStroke(gNanoVG);
}

void ofRect(float x, float y, float width, float height, float cornerRadius /*=3*/)
{
   nvgBeginPath(gNanoVG);
   nvgRoundedRect(gNanoVG, x, y, width, height, cornerRadius * gCornerRoundness);
   if (sStyleStack.GetStyle().fill)
      nvgFill(gNanoVG);
   else
      nvgStroke(gNanoVG);
}

void ofRect(const ofRectangle& rect, float cornerRadius /*=3*/)
{
   ofRect(rect.x, rect.y, rect.width, rect.height, cornerRadius);
}

float ofClamp(float val, float a, float b)
{
   if (val < a)
      return a;
   if (val > b)
      return b;
   return val;
}

float ofGetLastFrameTime()
{
   /*TODO_PORT(Ryan)*/
   return .01666f;
}

int ofToInt(const std::string& intString)
{
   String str(intString);
   return str.getIntValue();
}

float ofToFloat(const std::string& floatString)
{
   String str(floatString);
   return str.getFloatValue();
}

int ofHexToInt(const std::string& hexString)
{
   String str(hexString);
   return str.getHexValue32();
}

void ofLine(float x1, float y1, float x2, float y2)
{
   nvgBeginPath(gNanoVG);
   nvgMoveTo(gNanoVG, x1, y1);
   nvgLineTo(gNanoVG, x2, y2);
   nvgStroke(gNanoVG);
}

void ofLine(ofVec2f v1, ofVec2f v2)
{
   ofLine(v1.x, v1.y, v2.x, v2.y);
}

void ofSetLineWidth(float width)
{
   sStyleStack.GetStyle().lineWidth = width;

   const float kLineWidthAdjustAmount = 0.25f;
   const float kLineWidthAdjustFactor = 1.5f;
   nvgStrokeWidth(gNanoVG, width * kLineWidthAdjustFactor + kLineWidthAdjustAmount);
}

namespace
{
   std::vector<ofVec2f> gShapePoints;
}

void ofBeginShape()
{
   gShapePoints.clear();
   nvgBeginPath(gNanoVG);
}

void ofEndShape(bool close)
{
   if (sStyleStack.GetStyle().fill)
      nvgFill(gNanoVG);
   else
      nvgStroke(gNanoVG);
}

void ofVertex(float x, float y, float z)
{
   if (gShapePoints.empty())
      nvgMoveTo(gNanoVG, x, y);
   else
      nvgLineTo(gNanoVG, x, y);
   gShapePoints.push_back(ofVec2f(x, y));
}

void ofVertex(ofVec2f point)
{
   ofVertex(point.x, point.y);
}

float ofMap(float val, float fromStart, float fromEnd, float toStart, float toEnd, bool clamp)
{
   float ret;
   if (fromEnd - fromStart != 0)
      ret = ((val - fromStart) / (fromEnd - fromStart)) * (toEnd - toStart) + toStart;
   else
      ret = toEnd;
   if (clamp)
      ret = ofClamp(ret, MIN(toStart, toEnd), MAX(toStart, toEnd));
   return ret;
}

float ofRandom(float max)
{
   return max * gRandom01(gRandom);
}

float ofRandom(float x, float y)
{
   float high = 0;
   float low = 0;
   float randNum = 0;
   // if there is no range, return the value
   if (x == y)
      return x; // float == ?, wise? epsilon?
   high = MAX(x, y);
   low = MIN(x, y);
   randNum = low + ((high - low) * gRandom01(gRandom));
   return randNum;
}

void ofSetCircleResolution(float res)
{
}

#if BESPOKE_WINDOWS
namespace windowsport
{
   struct timespec
   {
      long tv_sec;
      long tv_nsec;
   }; //header part
   int clock_gettime(int, struct timespec* spec) //C-file part
   {
      __int64 wintime;
      GetSystemTimeAsFileTime((FILETIME*)&wintime);
      wintime -= 116444736000000000ll; //1jan1601 to 1jan1970
      spec->tv_sec = wintime / 10000000ll; //seconds
      spec->tv_nsec = wintime % 10000000ll * 100; //nano-seconds
      return 0;
   }
}

unsigned long long ofGetSystemTimeNanos()
{
   //auto now = std::chrono::high_resolution_clock::now();
   //return std::chrono::duration_cast<std::chrono:nanoseconds>(now.time_since_epoch()).count();
   struct windowsport::timespec t;
   windowsport::clock_gettime(0, &t);
   return t.tv_sec * 1000000000 + t.tv_nsec;
}
#else
unsigned long long ofGetSystemTimeNanos()
{
   //auto now = std::chrono::high_resolution_clock::now();
   //return std::chrono::duration_cast<std::chrono:nanoseconds>(now.time_since_epoch()).count();
   struct timespec t;
   clock_gettime(CLOCK_MONOTONIC, &t);
   return t.tv_sec * 1000000000 + t.tv_nsec;
}
#endif

float ofGetWidth()
{
   return TheSynth->GetMainComponent()->getWidth();
}

float ofGetHeight()
{
   return TheSynth->GetMainComponent()->getHeight();
}

float ofGetFrameRate()
{
   return TheSynth->GetFrameRate();
}

float ofLerp(float start, float stop, float amt)
{
   return start + (stop - start) * amt;
}

float ofDistSquared(float x1, float y1, float x2, float y2)
{
   return ((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

std::vector<std::string> ofSplitString(std::string str, std::string splitter, bool ignoreEmpty, bool trim)
{
   StringArray tokens;

   tokens.addTokens(String(str), String(splitter), "");

   if (ignoreEmpty)
      tokens.removeEmptyStrings();

   if (trim)
      tokens.trim();

   std::vector<std::string> ret;
   for (auto s : tokens)
      ret.push_back(s.toStdString());

   return ret;
}

bool ofIsStringInString(const std::string& haystack, const std::string& needle)
{
   return (strstr(haystack.c_str(), needle.c_str()) != nullptr);
}

void ofScale(float x, float y, float z)
{
   nvgScale(gNanoVG, x, y);
}

void ofExit()
{
   //TODO_PORT(Ryan)
   assert(false);
}

void ofToggleFullscreen()
{
#if !BESPOKE_WINDOWS
   if (Desktop::getInstance().getKioskModeComponent() == nullptr)
      Desktop::getInstance().setKioskModeComponent(TheSynth->GetMainComponent()->getTopLevelComponent(), false);
   else
      Desktop::getInstance().setKioskModeComponent(nullptr, false);
#endif
}

void ofStringReplace(std::string& input, std::string searchStr, std::string replaceStr, bool firstOnly /*= false*/)
{
   size_t uPos = 0;
   size_t uFindLen = searchStr.length();
   size_t uReplaceLen = replaceStr.length();

   if (uFindLen == 0)
   {
      return;
   }

   for (; (uPos = input.find(searchStr, uPos)) != std::string::npos;)
   {
      input.replace(uPos, uFindLen, replaceStr);
      uPos += uReplaceLen;

      if (firstOnly)
         break;
   }
}

//%Y-%m-%d-%H-%M-%S-%i
std::string ofGetTimestampString(std::string in)
{
   Time time = Time::getCurrentTime();
   ofStringReplace(in, "%Y", ofToString(time.getYear()));
   char buff[16];
   snprintf(buff, sizeof(buff), "%02d", time.getMonth() + 1);
   ofStringReplace(in, "%m", buff);
   snprintf(buff, sizeof(buff), "%02d", time.getDayOfMonth());
   ofStringReplace(in, "%d", buff);
   snprintf(buff, sizeof(buff), "%02d", time.getHours());
   ofStringReplace(in, "%H", buff);
   snprintf(buff, sizeof(buff), "%02d", time.getMinutes());
   ofStringReplace(in, "%M", buff);
   snprintf(buff, sizeof(buff), "%02d", time.getSeconds());
   ofStringReplace(in, "%S", buff);
   snprintf(buff, sizeof(buff), "%03d", time.getMilliseconds());
   ofStringReplace(in, "%i", buff);
   return in;
}

void ofTriangle(float x1, float y1, float x2, float y2, float x3, float y3)
{
   ofBeginShape();
   ofVertex(x1, y1);
   ofVertex(x2, y2);
   ofVertex(x3, y3);
   ofVertex(x1, y1);
   ofEndShape();
}

float ofRectangle::getMinX() const
{
   return MIN(x, x + width); // - width
}

float ofRectangle::getMaxX() const
{
   return MAX(x, x + width); // - width
}

float ofRectangle::getMinY() const
{
   return MIN(y, y + height); // - height
}

float ofRectangle::getMaxY() const
{
   return MAX(y, y + height); // - height
}

bool ofRectangle::intersects(const ofRectangle& other) const
{
   return (getMinX() < other.getMaxX() && getMaxX() > other.getMinX() &&
           getMinY() < other.getMaxY() && getMaxY() > other.getMinY());
}

bool ofRectangle::contains(float testX, float testY) const
{
   return testX > getMinX() && testY > getMinY() &&
          testX < getMaxX() && testY < getMaxY();
}

void ofColor::setBrightness(int brightness)
{
   float hue, saturation, oldBrightness;
   getHsb(hue, saturation, oldBrightness);
   setHsb(hue, saturation, brightness);
}

int ofColor::getBrightness() const
{
   float max = r;
   if (g > max)
   {
      max = g;
   }
   if (b > max)
   {
      max = b;
   }
   return max;
}

void ofColor::setSaturation(int saturation)
{
   float hue, oldSaturation, brightness;
   getHsb(hue, oldSaturation, brightness);
   setHsb(hue, saturation, brightness);
}

int ofColor::getSaturation() const
{
   float max = getBrightness();

   float min = r;
   if (g < min)
      min = g;
   if (b < min)
      min = b;

   if (max == min) // grays
      return 0;

   return 255 * (max - min) / max;
}

void ofColor::getHsb(float& hue,
                     float& saturation,
                     float& brightness) const
{
   float max = getBrightness();

   float min = r;
   if (g < min)
   {
      min = g;
   }
   if (b < min)
   {
      min = b;
   }

   if (max == min)
   { // grays
      hue = 0.f;
      saturation = 0.f;
      brightness = max;
      return;
   }

   float hueSixth;
   if (r == max)
   {
      hueSixth = (g - b) / (max - min);
      if (hueSixth < 0.f)
         hueSixth += 6.f;
   }
   else if (g == max)
   {
      hueSixth = 2.f + (b - r) / (max - min);
   }
   else
   {
      hueSixth = 4.f + (r - g) / (max - min);
   }
   hue = 255 * hueSixth / 6.f;
   saturation = 255 * (max - min) / max;
   brightness = max;
}

void ofColor::setHsb(int hue, int saturation, int brightness)
{
   saturation = ofClamp(saturation, 0, 255);
   brightness = ofClamp(brightness, 0, 255);
   if (brightness == 0)
   { // black
      set(0, 0, 0);
   }
   else if (saturation == 0)
   { // grays
      set(brightness, brightness, brightness);
   }
   else
   {
      float hueSix = hue * 6.f / 255.0f;
      float saturationNorm = saturation / 255.0f;
      int hueSixCategory = (int)floorf(hueSix);
      float hueSixRemainder = hueSix - hueSixCategory;
      float pv = ((1.f - saturationNorm) * brightness);
      float qv = ((1.f - saturationNorm * hueSixRemainder) * brightness);
      float tv = ((1.f - saturationNorm * (1.f - hueSixRemainder)) * brightness);
      switch (hueSixCategory)
      {
         case 0:
         case 6: // r
            r = brightness;
            g = tv;
            b = pv;
            break;
         case 1: // g
            r = qv;
            g = brightness;
            b = pv;
            break;
         case 2:
            r = pv;
            g = brightness;
            b = tv;
            break;
         case 3: // b
            r = pv;
            g = qv;
            b = brightness;
            break;
         case 4:
            r = tv;
            g = pv;
            b = brightness;
            break;
         case 5: // back to r
            r = brightness;
            g = pv;
            b = qv;
            break;
      }
   }
}

ofColor ofColor::operator*(const ofColor& other)
{
   return ofColor(r * other.r / 255.0f, g * other.g / 255.0f, b * other.b / 255.0f, a * other.a / 255.0f);
}

ofColor ofColor::operator*(float f)
{
   return ofColor(r * f, g * f, b * f, a * f);
}

ofColor ofColor::operator+(const ofColor& other)
{
   return ofColor(r + other.r, g + other.g, b + other.b, a + other.a);
}

void RetinaTrueTypeFont::LoadFont(std::string path)
{
   mFontPath = ofToDataPath(path);
   File file(mFontPath.c_str());
   if (file.existsAsFile())
   {
      mFontHandle = nvgCreateFont(gNanoVG, path.c_str(), path.c_str());
      mFontBoundsHandle = nvgCreateFont(gFontBoundsNanoVG, path.c_str(), path.c_str());
      mLoaded = true;
   }
   else
   {
      mLoaded = false;
   }
}

void RetinaTrueTypeFont::DrawString(std::string str, float size, float x, float y)
{
   if (!mLoaded)
      return;

   nvgFontFaceId(gNanoVG, mFontHandle);
   nvgFontSize(gNanoVG, size);


   if (ofIsStringInString(str, "\n") == false)
   {
      nvgText(gNanoVG, x, y, str.c_str(), nullptr);
   }
   else
   {
      std::vector<std::string> lines = ofSplitString(str, "\n");
      float bounds[4];
      nvgTextBounds(gNanoVG, 0, 0, str.c_str(), nullptr, bounds);
      float lineHeight = bounds[3] - bounds[1];
      for (int i = 0; i < lines.size(); ++i)
      {
         nvgText(gNanoVG, x, y, lines[i].c_str(), nullptr);
         y += lineHeight;
      }
   }
}

ofRectangle RetinaTrueTypeFont::DrawStringWrap(std::string str, float size, float x, float y, float width)
{
   if (!mLoaded)
      return ofRectangle();

   TheSynth->LockRender(true);
   nvgFontFaceId(gNanoVG, mFontHandle);
   nvgFontSize(gNanoVG, size);
   nvgTextBox(gNanoVG, x, y, width, str.c_str(), nullptr);
   float bounds[4];
   nvgTextBoxBounds(gNanoVG, x, y, width, str.c_str(), nullptr, bounds);
   ofRectangle rect(bounds[0], bounds[1], bounds[2] - bounds[0], bounds[3] - bounds[1]);
   TheSynth->LockRender(false);
   return rect;
}

float RetinaTrueTypeFont::GetStringWidth(std::string str, float size)
{
   if (!mLoaded)
      return str.size() * 12;

   NVGcontext* vg;
   int handle;
   if (TheSynth->GetOpenGLContext()->getCurrentContext() != nullptr)
   {
      vg = gNanoVG;
      handle = mFontHandle;
   }
   else
   {
      vg = gFontBoundsNanoVG;
      handle = mFontBoundsHandle;
   }

   nvgFontFaceId(vg, handle);
   nvgFontSize(vg, size);
   float bounds[4];
   float width = nvgTextBounds(vg, 0, 0, str.c_str(), nullptr, bounds);

   return width;
}

float RetinaTrueTypeFont::GetStringHeight(std::string str, float size)
{
   if (!mLoaded)
      return str.size() * 12;

   NVGcontext* vg;
   int handle;
   if (TheSynth->GetOpenGLContext()->getCurrentContext() != nullptr)
   {
      vg = gNanoVG;
      handle = mFontHandle;
   }
   else
   {
      vg = gFontBoundsNanoVG;
      handle = mFontBoundsHandle;
   }

   nvgFontFaceId(vg, handle);
   nvgFontSize(vg, size);
   float bounds[4];
   nvgTextBounds(vg, 0, 0, str.c_str(), nullptr, bounds);

   float lineHeight = bounds[3] - bounds[1];
   return lineHeight;
}
