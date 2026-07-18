/**
    bespoke synth - ShaderViz (implementation)
**/

#include "ShaderViz.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "juce_opengl/juce_opengl.h"
#include <cmath>

using namespace juce::gl;

namespace
{
   const float kCtrlTop = 4;
   const float kPreviewTop = 82;

   const char* kFragSrc =
   "#version 150\n"
   "in vec2 vUv;\n"
   "out vec4 fragColor;\n"
   "uniform float uTime;\n"
   "uniform float uReact;\n"
   "uniform float uHue;\n"
   "uniform float uSpeed;\n"
   "uniform vec2 uRes;\n"
   "vec3 hsv2rgb(vec3 c){\n"
   "  vec4 K = vec4(1.0, 2.0/3.0, 1.0/3.0, 3.0);\n"
   "  vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);\n"
   "  return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);\n"
   "}\n"
   "void main(){\n"
   "  vec2 uv = (vUv - 0.5) * vec2(uRes.x / max(1.0, uRes.y), 1.0);\n"
   "  float t = uTime * 0.001 * uSpeed;\n"
   "  float r = length(uv);\n"
   "  float a = atan(uv.y, uv.x);\n"
   "  float w = sin(r*10.0 - t*2.0 + uReact*8.0) + sin(a*6.0 + t) + sin((uv.x+uv.y)*8.0 - t*1.5);\n"
   "  w += uReact * 2.0 * sin(r*20.0 - t*4.0);\n"
   "  float hue = fract(uHue + 0.5 + 0.15*w + t*0.05);\n"
   "  float val = clamp(0.5 + 0.5*sin(w*3.14159) + uReact*0.4, 0.0, 1.0);\n"
   "  vec3 col = hsv2rgb(vec3(hue, 0.85, val));\n"
   "  fragColor = vec4(col, 1.0);\n"
   "}\n";
}

ShaderViz::ShaderViz()
: IAudioProcessor(gBufferSize)
{
}

ShaderViz::~ShaderViz()
{
   VizGL::DestroyFbo(mOut);
}

void ShaderViz::Process(double time)
{
   if (!mEnabled)
      return;

   SyncBuffers();
   const int bufferSize = GetBuffer()->BufferSize();
   IAudioReceiver* target = GetTarget();

   float sum = 0;
   const int nch = GetBuffer()->NumActiveChannels();
   for (int ch = 0; ch < nch; ++ch)
   {
      const float* data = GetBuffer()->GetChannel(ch);
      for (int i = 0; i < bufferSize; ++i)
         sum += data[i] * data[i];
      if (target != nullptr)
         Add(target->GetBuffer()->GetChannel(ch), data, bufferSize);
   }
   const float rms = (nch * bufferSize > 0) ? sqrtf(sum / (float)(nch * bufferSize)) : 0.0f;
   mAmplitude = mAmplitude * 0.8f + rms * 0.2f; //light smoothing for nicer motion

   GetBuffer()->Reset();
}

void ShaderViz::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mReactSlider = new FloatSlider(this, "react", 4, 22, 160, 14, &mReact, 0.0f, 1.0f);
   mHueSlider = new FloatSlider(this, "hue", 4, 40, 160, 14, &mHue, 0.0f, 1.0f);
   mSpeedSlider = new FloatSlider(this, "speed", 4, 58, 160, 14, &mSpeed, 0.0f, 3.0f);
   mFpsSelector = new DropdownList(this, "max fps", 172, 22, &mMaxFps, 56);
   mFpsSelector->AddLabel("60", 60);
   mFpsSelector->AddLabel("30", 30);
   mFpsSelector->AddLabel("20", 20);
   mFpsSelector->AddLabel("15", 15);
}

bool ShaderViz::EnsureShader()
{
   if (mShaderTried)
      return mProgram != 0;
   mShaderTried = true;

   mProgram = VizGL::CompileProgram(kFragSrc);
   if (mProgram == 0)
      return false;

   mLocTime = glGetUniformLocation(mProgram, "uTime");
   mLocReact = glGetUniformLocation(mProgram, "uReact");
   mLocHue = glGetUniformLocation(mProgram, "uHue");
   mLocSpeed = glGetUniformLocation(mProgram, "uSpeed");
   mLocRes = glGetUniformLocation(mProgram, "uRes");
   return true;
}

void ShaderViz::Cook()
{
   if (!EnsureShader())
      return;

   double interval = 1000.0 / (double)MAX(1, mMaxFps);
   if (gTime - mLastCookMs < interval)
      return;

   if (!VizGL::EnsureFbo(mOut, mResW, mResH))
      return;

   float t = (float)gTime;
   //audio drives the intensity; the "react" slider is the sensitivity. a small idle keeps it moving
   //even with no audio connected
   float react = ofClamp(0.08f + mAmplitude * mReact * 8.0f, 0.0f, 1.0f);
   float hue = mHue;
   float speed = mSpeed;
   float rw = (float)mOut.w;
   float rh = (float)mOut.h;
   int locT = mLocTime, locR = mLocReact, locH = mLocHue, locS = mLocSpeed, locRes = mLocRes;

   VizGL::RunShaderPass(mOut, mProgram, [=]()
                        {
                           if (locT >= 0)
                              glUniform1f(locT, t);
                           if (locR >= 0)
                              glUniform1f(locR, react);
                           if (locH >= 0)
                              glUniform1f(locH, hue);
                           if (locS >= 0)
                              glUniform1f(locS, speed);
                           if (locRes >= 0)
                              glUniform2f(locRes, rw, rh);
                        });

   mLastCookMs = gTime;
}

void ShaderViz::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mReactSlider->Draw();
   mHueSlider->Draw();
   mSpeedSlider->Draw();
   mFpsSelector->Draw();

   float px = 4;
   float py = kPreviewTop;
   float pw = mWidth - 8;
   float ph = mHeight - kPreviewTop - 4;
   if (pw < 16 || ph < 16)
      return;

   //internal render resolution (stretched to the preview), capped for speed
   int rw = MAX(1080, ((int)pw & ~1));
   int rh = MAX(1080, ((int)ph & ~1));

   mResW = rw;
   mResH = rh;

   //backdrop
   ofPushStyle();
   ofFill();
   ofSetColor(0, 0, 0);
   ofRect(px, py, pw, ph);
   ofPopStyle();

   if (!EnsureShader())
   {
      ofPushStyle();
      ofSetColor(240, 140, 120);
      DrawTextNormal("shader unavailable (see console)", (int)px + 6, (int)py + 18, 11);
      ofPopStyle();
      return;
   }

   Cook();
   VizGL::DrawTexture(GetOutputTexture(), px, py, pw, ph);

   ofPushStyle();
   ofNoFill();
   ofSetColor(120, 125, 145);
   ofRect(px, py, pw, ph);
   ofPopStyle();
}

void ShaderViz::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void ShaderViz::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void ShaderViz::SetUpFromSaveData()
{
}
