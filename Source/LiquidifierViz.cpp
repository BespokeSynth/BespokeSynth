/**
    bespoke synth - LiquidifierViz
**/

#include "LiquidifierViz.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "juce_opengl/juce_opengl.h"
#include "PatchCable.h"
#include <cmath>

using namespace juce::gl;

namespace
{
   const char* kFragSrc =
   "#version 150\n"
   "in vec2 vUv;\n"
   "out vec4 fragColor;\n"
   "uniform sampler2D uTex;\n"
   "uniform float uTime;\n"
   "uniform vec2 uRes;\n"
   "uniform vec2 uTexRes;\n"
   "uniform float uTwisting;\n"
   "uniform float uDistortion;\n"
   "uniform float uSpeed;\n"
   "uniform int uPalette;\n"
   "\n"
   "vec3 getPalette(float t, int p) {\n"
   "  t = clamp(t, 0.0, 1.0);\n"
   "  if(p == 1) {\n" // thermal: black -> purple -> orange -> yellow
   "    if(t < 0.33) return mix(vec3(0.0), vec3(0.5, 0.0, 0.5), t * 3.0);\n"
   "    if(t < 0.66) return mix(vec3(0.5, 0.0, 0.5), vec3(1.0, 0.4, 0.0), (t-0.33)*3.0);\n"
   "    return mix(vec3(1.0, 0.4, 0.0), vec3(1.0, 1.0, 0.6), (t-0.66)*3.0);\n"
   "  }\n"
   "  if(p == 2) {\n" // oceanic: black -> deep blue -> cyan -> white
   "    if(t < 0.5) return mix(vec3(0.0, 0.0, 0.15), vec3(0.0, 0.5, 0.9), t * 2.0);\n"
   "    return mix(vec3(0.0, 0.5, 0.9), vec3(0.8, 1.0, 1.0), (t-0.5)*2.0);\n"
   "  }\n"
   "  if(p == 3) {\n" // psychedelic: cycling hue
   "    vec3 a = vec3(0.5, 0.5, 0.5);\n"
   "    vec3 b = vec3(0.5, 0.5, 0.5);\n"
   "    vec3 cc = vec3(1.0, 1.0, 1.0);\n"
   "    vec3 d = vec3(0.0, 0.33, 0.67);\n"
   "    return a + b * cos(6.28318*(cc*t+d));\n"
   "  }\n"
   "  return vec3(t);\n"
   "}\n"
   "\n"
   "void main(){\n"
   "  vec2 uv = vUv;\n"
   "  vec2 center = vec2(0.5, 0.5);\n"
   "  vec2 offset = uv - center;\n"
   "  float dist = length(offset);\n"
   "  float angle = atan(offset.y, offset.x);\n"
   "\n"
   "  float t = uTime * uSpeed;\n"
   "  angle += uTwisting * dist * 10.0 * sin(t + dist * 5.0);\n"
   "  dist += uDistortion * sin(angle * 4.0 - t * 2.0);\n"
   "\n"
   "  vec2 distortedUv = center + vec2(cos(angle), sin(angle)) * dist;\n"
   "\n"
   "  vec4 texColor = texture(uTex, clamp(distortedUv, 0.001, 0.999));\n"
   "  if (uPalette == 0) {\n"
   "     fragColor = texColor;\n"
   "  } else {\n"
   "     float lum = dot(texColor.rgb, vec3(0.299, 0.587, 0.114));\n"
   "     fragColor = vec4(getPalette(lum, uPalette), texColor.a);\n"
   "  }\n"
   "}\n";
}

LiquidifierViz::LiquidifierViz()
{
}

LiquidifierViz::~LiquidifierViz()
{
}

void LiquidifierViz::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mTwistSlider = new FloatSlider(this, "twisting", 4, 18, 160, 14, &mTwisting, -1.0f, 1.0f);
   mDistSlider = new FloatSlider(this, "distortion", 4, 36, 160, 14, &mDistortion, 0.0f, 1.0f);
   mSpeedSlider = new FloatSlider(this, "speed", 4, 54, 160, 14, &mSpeed, 0.0f, 5.0f);

   mPaletteSelector = new DropdownList(this, "palette", 4, 72, &mPalette, 90);
   mPaletteSelector->AddLabel("original", 0);
   mPaletteSelector->AddLabel("thermal", 1);
   mPaletteSelector->AddLabel("oceanic", 2);
   mPaletteSelector->AddLabel("psyched", 3);

   mTargetCable = new PatchCableSource(this, kConnectionType_Special);
   mTargetCable->SetManualPosition(mWidth - 12, 12);
   AddPatchCableSource(mTargetCable);
}

bool LiquidifierViz::EnsureShader()
{
   if (mProgram != 0)
      return true;

   mProgram = VizGL::CompileProgram(kFragSrc);
   if (mProgram != 0)
   {
      mLocTwisting = glGetUniformLocation(mProgram, "uTwisting");
      mLocDistortion = glGetUniformLocation(mProgram, "uDistortion");
      mLocSpeed = glGetUniformLocation(mProgram, "uSpeed");
      mLocPalette = glGetUniformLocation(mProgram, "uPalette");
      mLocTime = glGetUniformLocation(mProgram, "uTime");
      mLocRes = glGetUniformLocation(mProgram, "uRes");
      mLocTexRes = glGetUniformLocation(mProgram, "uTexRes");
   }
   return mProgram != 0;
}

void LiquidifierViz::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
}

void LiquidifierViz::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void LiquidifierViz::Cook()
{
   if (!EnsureShader())
      return;

   IVisualNode* inputNode = nullptr;
   for (auto* cable : mTargetCable->GetPatchCables())
   {
      IDrawableModule* target = dynamic_cast<IDrawableModule*>(cable->GetTarget());
      if (target != nullptr)
      {
         inputNode = dynamic_cast<IVisualNode*>(target);
         if (inputNode != nullptr)
            break;
      }
   }

   int inW = 1080;
   int inH = 1080;
   unsigned int inTex = 0;
   if (inputNode != nullptr)
   {
      inputNode->CookIfNeeded(mLastCookFrame);
      inTex = inputNode->GetOutputTexture();
      inW = inputNode->GetOutputWidth();
      inH = inputNode->GetOutputHeight();
   }

   mResW = inW;
   mResH = inH;

   if (mResW <= 0 || mResH <= 0)
      return;

   if (!VizGL::EnsureFbo(mOut, mResW, mResH))
      return;

   if (inTex == 0)
   {
      VizGL::BindFbo(mOut);
      glClearColor(0, 0, 0, 0);
      glClear(GL_COLOR_BUFFER_BIT);
      VizGL::UnbindFbo();
      return;
   }

   VizGL::RunShaderPass(mOut, mProgram, [&]()
                        {
                           glUniform2f(mLocRes, (float)mResW, (float)mResH);
                           glUniform2f(mLocTexRes, (float)inW, (float)inH);
                           glUniform1f(mLocTwisting, mTwisting);
                           glUniform1f(mLocDistortion, mDistortion);
                           glUniform1f(mLocSpeed, mSpeed);
                           glUniform1i(mLocPalette, mPalette);
                           glUniform1f(mLocTime, (float)gTime);

                           glActiveTexture(GL_TEXTURE0);
                           glBindTexture(GL_TEXTURE_2D, inTex);
                           glUniform1i(glGetUniformLocation(mProgram, "uTex"), 0);
                        });
}

void LiquidifierViz::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mTwistSlider->Draw();
   mDistSlider->Draw();
   mSpeedSlider->Draw();
   mPaletteSelector->Draw();

   float px = 4;
   float py = 90;
   float pw = mWidth - 8;
   float ph = mHeight - py - 4;
   if (ph < 20 || pw < 20)
      return;

   Cook();
   if (VizGL::FboTexture(mOut) != 0)
   {
      float frameAsp = (float)mResW / (float)MAX(1, mResH);
      float rectAsp = pw / MAX(1.0f, ph);
      float dw = pw, dh = ph, dx = px, dy = py;
      if (frameAsp > rectAsp)
      {
         dh = pw / frameAsp;
         dy = py + (ph - dh) * 0.5f;
      }
      else
      {
         dw = ph * frameAsp;
         dx = px + (pw - dw) * 0.5f;
      }
      VizGL::DrawTexture(VizGL::FboTexture(mOut), dx, dy, dw, dh);
   }
}

void LiquidifierViz::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();
   IDrawableModule::SaveState(out);
}

void LiquidifierViz::LoadState(FileStreamIn& in, int rev)
{
   int moduleRev;
   in >> moduleRev;
   IDrawableModule::LoadState(in, rev);
}
