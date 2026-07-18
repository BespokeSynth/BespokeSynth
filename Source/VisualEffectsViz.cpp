#include "VisualEffectsViz.h"
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

   "uniform float uNoiseAmount;\n"
   "uniform int uNoiseColor;\n"
   "uniform float uLensDistortX;\n"
   "uniform float uLensDistortY;\n"
   "uniform int uSymmetry;\n"
   "uniform float uPixelation;\n"
   "uniform float uBlur;\n"
   "uniform float uOil;\n"
   "uniform float uHalftone;\n"
   "uniform float uGlow;\n"
   "\n"
   "float rand(vec2 co) {\n"
   "   return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);\n"
   "}\n"
   "\n"
   "vec3 getOil(sampler2D tex, vec2 uv, vec2 texel) {\n"
   "  float radius = uOil * 4.0;\n"
   "  vec3 m[4]; vec3 s[4];\n"
   "  for (int k = 0; k < 4; ++k) { m[k] = vec3(0.0); s[k] = vec3(0.0); }\n"
   "  int rad = int(ceil(radius));\n"
   "  vec2 offsets[4] = vec2[](vec2(-1,-1), vec2(1,-1), vec2(-1,1), vec2(1,1));\n"
   "  for(int i = 0; i < 4; ++i) {\n"
   "    for(int y = 0; y <= rad; ++y) {\n"
   "      for(int x = 0; x <= rad; ++x) {\n"
   "        vec2 offset = vec2(x, y) * offsets[i];\n"
   "        vec3 c = texture(tex, uv + offset * texel).rgb;\n"
   "        m[i] += c;\n"
   "        s[i] += c * c;\n"
   "      }\n"
   "    }\n"
   "  }\n"
   "  float min_sigma2 = 1e+2;\n"
   "  vec3 outColor = texture(tex, uv).rgb;\n"
   "  float n = float((rad + 1) * (rad + 1));\n"
   "  for(int i = 0; i < 4; ++i) {\n"
   "    m[i] /= n;\n"
   "    s[i] = abs(s[i] / n - m[i] * m[i]);\n"
   "    float sigma2 = s[i].r + s[i].g + s[i].b;\n"
   "    if (sigma2 < min_sigma2) {\n"
   "      min_sigma2 = sigma2;\n"
   "      outColor = m[i];\n"
   "    }\n"
   "  }\n"
   "  return outColor;\n"
   "}\n"
   "\n"
   "vec3 getBlur(sampler2D tex, vec2 uv, vec2 texel, float amount) {\n"
   "  vec3 color = vec3(0.0);\n"
   "  float total = 0.0;\n"
   "  float r = amount * 10.0;\n"
   "  for(float x = -2.0; x <= 2.0; x += 1.0) {\n"
   "    for(float y = -2.0; y <= 2.0; y += 1.0) {\n"
   "      float weight = 1.0 - (length(vec2(x,y)) / 3.0);\n"
   "      if (weight > 0.0) {\n"
   "         color += texture(tex, uv + vec2(x, y) * texel * r).rgb * weight;\n"
   "         total += weight;\n"
   "      }\n"
   "    }\n"
   "  }\n"
   "  return color / total;\n"
   "}\n"
   "\n"
   "void main(){\n"
   "  vec2 uv = vUv;\n"
   "\n"
   "  if (abs(uLensDistortX) > 0.001 || abs(uLensDistortY) > 0.001) {\n"
   "    vec2 cc = uv - 0.5;\n"
   "    float dist = dot(cc, cc);\n"
   "    uv.x = 0.5 + cc.x * (1.0 + uLensDistortX * dist);\n"
   "    uv.y = 0.5 + cc.y * (1.0 + uLensDistortY * dist);\n"
   "  }\n"
   "\n"
   "  if (uSymmetry > 1) {\n"
   "    vec2 cc = uv - 0.5;\n"
   "    float angle = atan(cc.y, cc.x);\n"
   "    float radius = length(cc);\n"
   "    float slice = 6.28318530718 / float(uSymmetry);\n"
   "    angle = mod(angle, slice);\n"
   "    if (angle > slice / 2.0) angle = slice - angle;\n"
   "    uv = 0.5 + vec2(cos(angle), sin(angle)) * radius;\n"
   "  }\n"
   "\n"
   "  if (uPixelation > 1.0) {\n"
   "    float pSize = floor(uPixelation);\n"
   "    vec2 res = uRes / pSize;\n"
   "    uv = floor(uv * res) / res;\n"
   "  }\n"
   "\n"
   "  if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {\n"
   "    fragColor = vec4(0.0);\n"
   "    return;\n"
   "  }\n"
   "\n"
   "  vec2 texel = 1.0 / uRes;\n"
   "  vec3 col = texture(uTex, uv).rgb;\n"
   "\n"
   "  if (uOil > 0.01) {\n"
   "     col = mix(col, getOil(uTex, uv, texel), uOil);\n"
   "  }\n"
   "\n"
   "  if (uBlur > 0.01) {\n"
   "     col = mix(col, getBlur(uTex, uv, texel, uBlur), uBlur);\n"
   "  }\n"
   "\n"
   "  if (uGlow > 0.01) {\n"
   "     vec3 blurCol = getBlur(uTex, uv, texel, uGlow * 2.0);\n"
   "     col += blurCol * uGlow;\n"
   "  }\n"
   "\n"
   "  if (uHalftone > 0.01) {\n"
   "     float luma = dot(col, vec3(0.299, 0.587, 0.114));\n"
   "     float s = sin(uv.x * uRes.x * 3.14159 * uHalftone) * sin(uv.y * uRes.y * 3.14159 * uHalftone);\n"
   "     float threshold = luma * 1.5 - 0.2;\n"
   "     float dotCol = (s < threshold) ? 1.0 : 0.0;\n"
   "     col = mix(col, col * dotCol, uHalftone);\n"
   "  }\n"
   "\n"
   "  if (uNoiseAmount > 0.001) {\n"
   "     if (uNoiseColor == 1) {\n"
   "       vec3 n;\n"
   "       n.r = rand(uv + uTime * 1.0);\n"
   "       n.g = rand(uv + uTime * 1.1);\n"
   "       n.b = rand(uv + uTime * 1.2);\n"
   "       col = mix(col, n, uNoiseAmount);\n"
   "     } else {\n"
   "       float n = rand(uv + uTime);\n"
   "       col = mix(col, vec3(n), uNoiseAmount);\n"
   "     }\n"
   "  }\n"
   "\n"
   "  fragColor = vec4(col, 1.0);\n"
   "}\n";
}

VisualEffectsViz::VisualEffectsViz()
{
   mModuleSaveData.SetString("moduleType", "VisualEffectsViz");
}

VisualEffectsViz::~VisualEffectsViz()
{
}

void VisualEffectsViz::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   int y = 25;
   int rowH = 18;

   mNoiseAmountSlider = new FloatSlider(this, "noise", 4, y, 160, 14, &mNoiseAmount, 0.0f, 1.0f);
   y += rowH;
   mNoiseColorCheckbox = new Checkbox(this, "noise color", 4, y, &mNoiseColor);
   y += rowH;

   mLensDistortXSlider = new FloatSlider(this, "lens x", 4, y, 160, 14, &mLensDistortX, -2.0f, 2.0f);
   y += rowH;
   mLensDistortYSlider = new FloatSlider(this, "lens y", 4, y, 160, 14, &mLensDistortY, -2.0f, 2.0f);
   y += rowH;

   mSymmetrySlider = new IntSlider(this, "symmetry", 4, y, 160, 14, &mSymmetry, 1, 12);
   y += rowH;

   mPixelationSlider = new FloatSlider(this, "pixelation", 4, y, 160, 14, &mPixelation, 1.0f, 50.0f);
   y += rowH;

   mBlurSlider = new FloatSlider(this, "blur", 4, y, 160, 14, &mBlur, 0.0f, 1.0f);
   y += rowH;

   mOilSlider = new FloatSlider(this, "oil effect", 4, y, 160, 14, &mOil, 0.0f, 1.0f);
   y += rowH;

   mHalftoneSlider = new FloatSlider(this, "halftone", 4, y, 160, 14, &mHalftone, 0.0f, 1.0f);
   y += rowH;

   mGlowSlider = new FloatSlider(this, "glow", 4, y, 160, 14, &mGlow, 0.0f, 1.0f);
   y += rowH;

   mTargetCable = new PatchCableSource(this, kConnectionType_Special);
   mTargetCable->SetManualPosition(mWidth - 12, 12);
   AddPatchCableSource(mTargetCable);
}

bool VisualEffectsViz::EnsureShader()
{
   if (mProgram != 0)
      return true;

   mProgram = VizGL::CompileProgram(kFragSrc);
   if (mProgram != 0)
   {
      mLocTime = glGetUniformLocation(mProgram, "uTime");
      mLocRes = glGetUniformLocation(mProgram, "uRes");
      mLocTexRes = glGetUniformLocation(mProgram, "uTexRes");

      mLocNoiseAmount = glGetUniformLocation(mProgram, "uNoiseAmount");
      mLocNoiseColor = glGetUniformLocation(mProgram, "uNoiseColor");
      mLocLensDistortX = glGetUniformLocation(mProgram, "uLensDistortX");
      mLocLensDistortY = glGetUniformLocation(mProgram, "uLensDistortY");
      mLocSymmetry = glGetUniformLocation(mProgram, "uSymmetry");
      mLocPixelation = glGetUniformLocation(mProgram, "uPixelation");
      mLocBlur = glGetUniformLocation(mProgram, "uBlur");
      mLocOil = glGetUniformLocation(mProgram, "uOil");
      mLocHalftone = glGetUniformLocation(mProgram, "uHalftone");
      mLocGlow = glGetUniformLocation(mProgram, "uGlow");
   }
   return mProgram != 0;
}

void VisualEffectsViz::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
}

void VisualEffectsViz::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void VisualEffectsViz::Cook()
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
                           if (mLocTime != -1)
                              glUniform1f(mLocTime, gTime);
                           if (mLocRes != -1)
                              glUniform2f(mLocRes, mResW, mResH);
                           if (mLocTexRes != -1)
                              glUniform2f(mLocTexRes, mResW, mResH);

                           if (mLocNoiseAmount != -1)
                              glUniform1f(mLocNoiseAmount, mNoiseAmount);
                           if (mLocNoiseColor != -1)
                              glUniform1i(mLocNoiseColor, mNoiseColor ? 1 : 0);
                           if (mLocLensDistortX != -1)
                              glUniform1f(mLocLensDistortX, mLensDistortX);
                           if (mLocLensDistortY != -1)
                              glUniform1f(mLocLensDistortY, mLensDistortY);
                           if (mLocSymmetry != -1)
                              glUniform1i(mLocSymmetry, mSymmetry);
                           if (mLocPixelation != -1)
                              glUniform1f(mLocPixelation, mPixelation);
                           if (mLocBlur != -1)
                              glUniform1f(mLocBlur, mBlur);
                           if (mLocOil != -1)
                              glUniform1f(mLocOil, mOil);
                           if (mLocHalftone != -1)
                              glUniform1f(mLocHalftone, mHalftone);
                           if (mLocGlow != -1)
                              glUniform1f(mLocGlow, mGlow);

                           glActiveTexture(GL_TEXTURE0);
                           glBindTexture(GL_TEXTURE_2D, inTex);
                           glUniform1i(glGetUniformLocation(mProgram, "uTex"), 0);
                        });
}

void VisualEffectsViz::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   if (mHeight < 450)
      Resize(mWidth, 450);

   mNoiseAmountSlider->Draw();
   mNoiseColorCheckbox->Draw();
   mLensDistortXSlider->Draw();
   mLensDistortYSlider->Draw();
   mSymmetrySlider->Draw();

   mPixelationSlider->Draw();
   mBlurSlider->Draw();
   mOilSlider->Draw();
   mHalftoneSlider->Draw();
   mGlowSlider->Draw();

   float px = 4;
   float py = 210;
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
