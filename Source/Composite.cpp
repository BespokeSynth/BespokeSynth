/**
    bespoke synth - Composite (implementation) - shader compositor
**/

#include "Composite.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "ModuleFactory.h"
#include "PatchCableSource.h"
#include "PatchCable.h"
#include "IUIControl.h"
#include "IVisualNode.h"
#include "juce_opengl/juce_opengl.h"
#include <algorithm>
#include <cstdio>

using namespace juce::gl;

#include "nanovg/nanovg.h"
#define NANOVG_GLES2_IMPLEMENTATION
#include "nanovg/nanovg_gl.h"
#include "nanovg/nanovg_gl_utils.h"

namespace
{
   const float kRowH = 16;
   const float kListTop = 44;
   const float kLabelX = 4;
   const int kMaxInternalDim = 512;

   const char* kVertSrc =
   "#version 150\n"
   "in vec2 aPos;\n"
   "in vec2 aUv;\n"
   "out vec2 vUv;\n"
   "void main(){ vUv = aUv; gl_Position = vec4(aPos, 0.0, 1.0); }\n";

   //core-profile GLSL. sampler arrays must be indexed with constant expressions here, so the layer
   //blends are unrolled (constant indices) rather than looped with a dynamic index.
   const char* kFragSrc =
   "#version 150\n"
   "in vec2 vUv;\n"
   "out vec4 fragColor;\n"
   "uniform sampler2D uLayers[6];\n"
   "uniform int uCount;\n"
   "uniform float uOpacity[6];\n"
   "uniform int uLayerMode[6];\n"
   "uniform float uExposure;\n"
   "uniform float uContrast;\n"
   "uniform float uBlack;\n"
   "uniform float uSaturation;\n"
   "uniform float uFrameAspect;\n"
   "uniform float uLayerAspect[6];\n"
   "uniform float uFill;\n"
   "vec3 sampLayer(sampler2D tex, vec2 uv, float layerAsp){\n"
   "  float ratio = uFrameAspect / max(0.0001, layerAsp);\n"
   "  vec2 su = uv;\n"
   "  if(uFill > 0.5){\n"
   "    if(ratio > 1.0) su.y = (uv.y-0.5)/ratio + 0.5;\n"
   "    else su.x = (uv.x-0.5)*ratio + 0.5;\n"
   "    return texture(tex, clamp(su, 0.0, 1.0)).rgb;\n"
   "  }\n"
   "  if(ratio > 1.0) su.x = (uv.x-0.5)*ratio + 0.5;\n"
   "  else su.y = (uv.y-0.5)/ratio + 0.5;\n"
   "  if(su.x < 0.0 || su.x > 1.0 || su.y < 0.0 || su.y > 1.0) return vec3(0.0);\n"
   "  return texture(tex, su).rgb;\n"
   "}\n"
   "vec3 blendPair(vec3 b, vec3 s, int m){\n"
   "  if(m==1) return b+s;\n"
   "  if(m==2) return b+s-b*s;\n"
   "  if(m==3) return b*s;\n"
   "  if(m==4) return max(b,s);\n"
   "  if(m==5) return min(b,s);\n"
   "  if(m==6) return abs(b-s);\n"
   "  if(m==7){ vec3 lo=2.0*b*s; vec3 hi=1.0-2.0*(1.0-b)*(1.0-s);\n"
   "    return vec3(b.r<0.5?lo.r:hi.r, b.g<0.5?lo.g:hi.g, b.b<0.5?lo.b:hi.b); }\n"
   "  if(m==8) return (1.0-2.0*s)*b*b + 2.0*s*b;\n"
   "  if(m==9){ vec3 lo=2.0*s*b; vec3 hi=1.0-2.0*(1.0-s)*(1.0-b);\n"
   "    return vec3(s.r<0.5?lo.r:hi.r, s.g<0.5?lo.g:hi.g, s.b<0.5?lo.b:hi.b); }\n"
   "  return s;\n"
   "}\n"
   "vec3 comp(vec3 acc, vec3 s, int m, float op){\n"
   "  vec3 blended = (m==0) ? s : blendPair(acc, s, m);\n"
   "  return mix(acc, blended, op);\n"
   "}\n"
   "void main(){\n"
   "  vec3 acc = mix(vec3(0.0), sampLayer(uLayers[0], vUv, uLayerAspect[0]), uOpacity[0]);\n"
   "  if(uCount>1) acc = comp(acc, sampLayer(uLayers[1], vUv, uLayerAspect[1]), uLayerMode[1], uOpacity[1]);\n"
   "  if(uCount>2) acc = comp(acc, sampLayer(uLayers[2], vUv, uLayerAspect[2]), uLayerMode[2], uOpacity[2]);\n"
   "  if(uCount>3) acc = comp(acc, sampLayer(uLayers[3], vUv, uLayerAspect[3]), uLayerMode[3], uOpacity[3]);\n"
   "  if(uCount>4) acc = comp(acc, sampLayer(uLayers[4], vUv, uLayerAspect[4]), uLayerMode[4], uOpacity[4]);\n"
   "  if(uCount>5) acc = comp(acc, sampLayer(uLayers[5], vUv, uLayerAspect[5]), uLayerMode[5], uOpacity[5]);\n"
   "  vec3 c = acc * uExposure;\n"
   "  c = (c - uBlack) / max(1e-4, 1.0 - uBlack);\n"
   "  c = (c - 0.5) * uContrast + 0.5;\n"
   "  float l = dot(c, vec3(0.2126, 0.7152, 0.0722));\n"
   "  c = mix(vec3(l), c, uSaturation);\n"
   "  fragColor = vec4(clamp(c, 0.0, 1.0), 1.0);\n"
   "}\n";

   GLuint CompileShader(GLenum type, const char* src)
   {
      GLuint sh = glCreateShader(type);
      glShaderSource(sh, 1, &src, nullptr);
      glCompileShader(sh);
      GLint ok = 0;
      glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
      if (!ok)
      {
         char log[1024];
         GLsizei n = 0;
         glGetShaderInfoLog(sh, sizeof(log), &n, log);
         printf("[Composite] shader compile error: %s\n", log);
         fflush(stdout);
         glDeleteShader(sh);
         return 0;
      }
      return sh;
   }
}

Composite::Composite()
{
}

Composite::~Composite()
{
}

void Composite::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mMoveUpButton = new ClickButton(this, "up", 4, 4);
   mMoveDownButton = new ClickButton(this, "down", 40, 4);

   for (int i = 0; i < kMaxLayers; ++i)
   {
      mLayerData[i].opacitySlider = new FloatSlider(this, ("op" + ofToString(i)).c_str(), 0, 0, 72, 14, &mLayerData[i].opacity, 0.0f, 1.0f);
      mLayerData[i].blendDropdown = new DropdownList(this, ("blend" + ofToString(i)).c_str(), 0, 0, &mLayerData[i].blendMode, 80);
      mLayerData[i].blendDropdown->AddLabel("normal", 0);
      mLayerData[i].blendDropdown->AddLabel("add", 1);
      mLayerData[i].blendDropdown->AddLabel("screen", 2);
      mLayerData[i].blendDropdown->AddLabel("multiply", 3);
      mLayerData[i].blendDropdown->AddLabel("lighten", 4);
      mLayerData[i].blendDropdown->AddLabel("darken", 5);
      mLayerData[i].blendDropdown->AddLabel("difference", 6);
      mLayerData[i].blendDropdown->AddLabel("overlay", 7);
      mLayerData[i].blendDropdown->AddLabel("soft light", 8);
      mLayerData[i].blendDropdown->AddLabel("hard light", 9);
      mLayerData[i].opacitySlider->SetShowing(false);
      mLayerData[i].blendDropdown->SetShowing(false);
      //note: FloatSlider/DropdownList constructors already register the control with this module,
      //so we must NOT call AddUIControl again (double-registering asserts as a duplicate name)
   }

   mFpsSelector = new DropdownList(this, "max fps", 200, 4, &mMaxFps, 44);
   mFpsSelector->AddLabel("60", 60);
   mFpsSelector->AddLabel("30", 30);
   mFpsSelector->AddLabel("20", 20);
   mFpsSelector->AddLabel("15", 15);
   mFpsSelector->AddLabel("10", 10);

   mAspectSelector = new DropdownList(this, "ratio", 248, 4, &mAspect, 58);
   mAspectSelector->AddLabel("source", 0);
   mAspectSelector->AddLabel("16:9", 1);
   mAspectSelector->AddLabel("1:1", 2);
   mAspectSelector->AddLabel("9:16", 3);
   mFillCheckbox = new Checkbox(this, "fill", 312, 4, &mFill);

   mExposureSlider = new FloatSlider(this, "exp", 4, 24, 88, 14, &mExposure, 0.0f, 3.0f);
   mContrastSlider = new FloatSlider(this, "con", 96, 24, 88, 14, &mContrast, 0.0f, 3.0f);
   mBlackSlider = new FloatSlider(this, "blk", 188, 24, 88, 14, &mBlack, 0.0f, 0.5f);
   mSaturationSlider = new FloatSlider(this, "sat", 280, 24, 88, 14, &mSaturation, 0.0f, 3.0f);

   mCableSource = new PatchCableSource(this, kConnectionType_Special);
   mCableSource->SetAllowMultipleTargets(true);
   mCableSource->SetDefaultPatchBehavior(kDefaultPatchBehavior_Add);
   mCableSource->SetManualPosition(mWidth - 12, 12);
   AddPatchCableSource(mCableSource);
}

void Composite::SyncLayers()
{
   std::vector<IDrawableModule*> targets;
   if (mCableSource != nullptr)
   {
      for (auto* cable : mCableSource->GetPatchCables())
      {
         IDrawableModule* m = dynamic_cast<IDrawableModule*>(cable->GetTarget());
         if (m == nullptr)
            continue;
         if (TheSynth->GetModuleFactory()->GetModuleCategory(m->GetTypeName()) != kModuleCategory_Visualizer)
            continue;
         targets.push_back(m);
      }
   }

   std::vector<IDrawableModule*> next;
   for (int i = 0; i < kMaxLayers; ++i)
   {
      if (mLayerData[i].module != nullptr)
      {
         if (std::find(targets.begin(), targets.end(), mLayerData[i].module) != targets.end())
            next.push_back(mLayerData[i].module);
      }
   }
   for (auto* t : targets)
   {
      if (std::find(next.begin(), next.end(), t) == next.end())
         next.push_back(t);
   }

   for (int i = 0; i < kMaxLayers; ++i)
   {
      if (i < (int)next.size())
         mLayerData[i].module = next[i];
      else
         mLayerData[i].module = nullptr;

      bool show = (mLayerData[i].module != nullptr);
      mLayerData[i].opacitySlider->SetShowing(show);
      mLayerData[i].blendDropdown->SetShowing(show);
   }

   int count = (int)next.size();
   if (mSelectedLayer >= count)
      mSelectedLayer = count - 1;
   if (mSelectedLayer < 0)
      mSelectedLayer = 0;
}

bool Composite::EnsureShader()
{
   if (mShaderTried)
      return mShaderOk;
   mShaderTried = true;

   GLuint vs = CompileShader(GL_VERTEX_SHADER, kVertSrc);
   GLuint fs = CompileShader(GL_FRAGMENT_SHADER, kFragSrc);
   if (vs == 0 || fs == 0)
      return false;

   GLuint prog = glCreateProgram();
   glAttachShader(prog, vs);
   glAttachShader(prog, fs);
   glLinkProgram(prog);
   GLint ok = 0;
   glGetProgramiv(prog, GL_LINK_STATUS, &ok);
   glDeleteShader(vs);
   glDeleteShader(fs);
   if (!ok)
   {
      char log[1024];
      GLsizei n = 0;
      glGetProgramInfoLog(prog, sizeof(log), &n, log);
      printf("[Composite] shader link error: %s\n", log);
      fflush(stdout);
      glDeleteProgram(prog);
      return false;
   }

   mProgram = prog;
   mLocLayers = glGetUniformLocation(prog, "uLayers[0]");
   mLocCount = glGetUniformLocation(prog, "uCount");
   mLocOpacity = glGetUniformLocation(prog, "uOpacity[0]");
   mLocLayerMode = glGetUniformLocation(prog, "uLayerMode[0]");
   mLocPos = glGetAttribLocation(prog, "aPos");
   mLocUv = glGetAttribLocation(prog, "aUv");
   mLocExposure = glGetUniformLocation(prog, "uExposure");
   mLocContrast = glGetUniformLocation(prog, "uContrast");
   mLocBlack = glGetUniformLocation(prog, "uBlack");
   mLocSaturation = glGetUniformLocation(prog, "uSaturation");
   mLocFrameAspect = glGetUniformLocation(prog, "uFrameAspect");
   mLocLayerAspect = glGetUniformLocation(prog, "uLayerAspect[0]");
   mLocFill = glGetUniformLocation(prog, "uFill");

   const float quad[] = {
      -1, -1, 0, 0,
      1, -1, 1, 0,
      -1, 1, 0, 1,
      1, 1, 1, 1
   };

   //core profile requires a VAO; bake the attrib layout into it once
   GLint prevVao = 0;
   glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prevVao);
   GLuint vao = 0, vbo = 0;
   glGenVertexArrays(1, &vao);
   glBindVertexArray(vao);
   glGenBuffers(1, &vbo);
   glBindBuffer(GL_ARRAY_BUFFER, vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
   if (mLocPos >= 0)
   {
      glEnableVertexAttribArray((GLuint)mLocPos);
      glVertexAttribPointer((GLuint)mLocPos, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
   }
   if (mLocUv >= 0)
   {
      glEnableVertexAttribArray((GLuint)mLocUv);
      glVertexAttribPointer((GLuint)mLocUv, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
   }
   glBindVertexArray((GLuint)prevVao);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   mVAO = vao;
   mVBO = vbo;

   mShaderOk = true;
   return true;
}

void Composite::EnsureResources(int w, int h)
{
   if (mFBW == w && mFBH == h && mLayerFB[0] != nullptr && mResultFB != nullptr)
      return;

   NVGcontext* recVG = gNanoVGRenderContexts[(int)NanoVGRenderContext::Screenshot];
   if (recVG == nullptr)
      return;

   if (mResultImage >= 0)
   {
      nvgDeleteImage(gNanoVG, mResultImage);
      mResultImage = -1;
   }
   for (int i = 0; i < kMaxLayers; ++i)
   {
      if (mLayerFB[i] != nullptr)
      {
         nvgluDeleteFramebuffer(mLayerFB[i]);
         mLayerFB[i] = nullptr;
      }
   }
   if (mResultFB != nullptr)
   {
      nvgluDeleteFramebuffer(mResultFB);
      mResultFB = nullptr;
   }

   for (int i = 0; i < kMaxLayers; ++i)
      mLayerFB[i] = nvgluCreateFramebuffer(recVG, w, h, 0);
   mResultFB = nvgluCreateFramebuffer(recVG, w, h, 0);

   if (mResultFB != nullptr)
      mResultImage = nvglCreateImageFromHandleGLES2(gNanoVG, mResultFB->texture, w, h, NVG_IMAGE_FLIPY | NVG_IMAGE_NODELETE);

   mFBW = w;
   mFBH = h;
}

// Legacy RenderLayerToFbo removed. Phase 0B enforces IVisualNode requirement for Composite inputs.

void Composite::CompositeLayers(int count, int w, int h)
{
   if (mResultFB == nullptr || mProgram == 0)
      return;

   //save GL state we touch
   GLint prevFBO = 0, prevVp[4] = { 0, 0, 0, 0 }, prevProg = 0, prevActive = 0, prevArrayBuf = 0, prevVao = 0;
   glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO);
   glGetIntegerv(GL_VIEWPORT, prevVp);
   glGetIntegerv(GL_CURRENT_PROGRAM, &prevProg);
   glGetIntegerv(GL_ACTIVE_TEXTURE, &prevActive);
   glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &prevArrayBuf);
   glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prevVao);
   GLboolean prevBlend = glIsEnabled(GL_BLEND);
   GLboolean prevScissor = glIsEnabled(GL_SCISSOR_TEST);

   glBindFramebuffer(GL_FRAMEBUFFER, mResultFB->fbo);
   glViewport(0, 0, w, h);
   glDisable(GL_BLEND);
   glDisable(GL_SCISSOR_TEST);
   glClearColor(0, 0, 0, 1);
   glClear(GL_COLOR_BUFFER_BIT);

   glUseProgram(mProgram);

   //bind a valid texture to all 6 units (unused ones point at layer 0 so nothing is incomplete)
   GLint units[kMaxLayers] = { 0, 1, 2, 3, 4, 5 };
   float opacities[kMaxLayers]{};
   int modes[kMaxLayers]{};
   for (int i = 0; i < kMaxLayers; ++i)
   {
      glActiveTexture(GL_TEXTURE0 + i);
      GLuint tex = (i < count && mLayerTex[i] != 0) ? mLayerTex[i] : mLayerTex[0];
      glBindTexture(GL_TEXTURE_2D, tex);
      opacities[i] = mLayerData[i].opacity;
      modes[i] = mLayerData[i].blendMode;
   }
   if (mLocLayers >= 0)
      glUniform1iv(mLocLayers, kMaxLayers, units);
   if (mLocCount >= 0)
      glUniform1i(mLocCount, count);
   if (mLocOpacity >= 0)
      glUniform1fv(mLocOpacity, kMaxLayers, opacities);
   if (mLocLayerMode >= 0)
      glUniform1iv(mLocLayerMode, kMaxLayers, modes);
   if (mLocExposure >= 0)
      glUniform1f(mLocExposure, mExposure);
   if (mLocContrast >= 0)
      glUniform1f(mLocContrast, mContrast);
   if (mLocBlack >= 0)
      glUniform1f(mLocBlack, mBlack);
   if (mLocSaturation >= 0)
      glUniform1f(mLocSaturation, mSaturation);
   if (mLocFrameAspect >= 0)
      glUniform1f(mLocFrameAspect, (float)w / (float)MAX(1, h));
   if (mLocLayerAspect >= 0)
      glUniform1fv(mLocLayerAspect, kMaxLayers, mLayerAspect);
   if (mLocFill >= 0)
      glUniform1f(mLocFill, mFill ? 1.0f : 0.0f);

   glBindVertexArray(mVAO);
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

   //restore
   glBindVertexArray((GLuint)prevVao);
   glBindBuffer(GL_ARRAY_BUFFER, (GLuint)prevArrayBuf);
   glUseProgram((GLuint)prevProg);
   glActiveTexture((GLenum)prevActive);
   glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)prevFBO);
   glViewport(prevVp[0], prevVp[1], prevVp[2], prevVp[3]);
   if (prevBlend)
      glEnable(GL_BLEND);
   else
      glDisable(GL_BLEND);
   if (prevScissor)
      glEnable(GL_SCISSOR_TEST);
   else
      glDisable(GL_SCISSOR_TEST);
}

void Composite::DrawPreview(float px, float py, float pw, float ph, int count)
{
   if (mResultImage < 0)
      return;
   NVGpaint paint = nvgImagePattern(gNanoVG, px, py, pw, ph, 0.0f, mResultImage, 1.0f);
   nvgBeginPath(gNanoVG);
   nvgRect(gNanoVG, px, py, pw, ph);
   nvgFillPaint(gNanoVG, paint);
   nvgFill(gNanoVG);
}

void Composite::OnClicked(float x, float y, bool right)
{
   if (!right)
   {
      int count = 0;
      for (int i = 0; i < kMaxLayers; ++i)
      {
         if (mLayerData[i].module != nullptr)
            count++;
      }
      for (int i = 0; i < count; ++i)
      {
         float ry = kListTop + i * kRowH;
         if (x >= kLabelX && x <= mWidth - 8 && y >= ry && y <= ry + kRowH)
         {
            mSelectedLayer = i;
            return;
         }
      }
   }
   IDrawableModule::OnClicked(x, y, right);
}

void Composite::Poll()
{
   SyncLayers();
}

void Composite::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   SyncLayers();
}

void Composite::ButtonClicked(ClickButton* button, double time)
{
   int count = 0;
   for (int i = 0; i < kMaxLayers; ++i)
   {
      if (mLayerData[i].module != nullptr)
         count++;
   }

   if (button == mMoveUpButton)
   {
      if (mSelectedLayer > 0 && mSelectedLayer < count)
      {
         std::swap(mLayerData[mSelectedLayer].module, mLayerData[mSelectedLayer - 1].module);
         std::swap(mLayerData[mSelectedLayer].opacity, mLayerData[mSelectedLayer - 1].opacity);
         std::swap(mLayerData[mSelectedLayer].blendMode, mLayerData[mSelectedLayer - 1].blendMode);
         mSelectedLayer--;
      }
   }
   else if (button == mMoveDownButton)
   {
      if (mSelectedLayer >= 0 && mSelectedLayer < count - 1)
      {
         std::swap(mLayerData[mSelectedLayer].module, mLayerData[mSelectedLayer + 1].module);
         std::swap(mLayerData[mSelectedLayer].opacity, mLayerData[mSelectedLayer + 1].opacity);
         std::swap(mLayerData[mSelectedLayer].blendMode, mLayerData[mSelectedLayer + 1].blendMode);
         mSelectedLayer++;
      }
   }
}

void Composite::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mMoveUpButton->Draw();
   mMoveDownButton->Draw();
   mFpsSelector->Draw();
   mAspectSelector->Draw();
   mFillCheckbox->Draw();
   mExposureSlider->Draw();
   mContrastSlider->Draw();
   mBlackSlider->Draw();
   mSaturationSlider->Draw();

   int count = 0;
   for (int i = 0; i < kMaxLayers; ++i)
   {
      if (mLayerData[i].module != nullptr)
         count++;
   }

   if (count == 0)
   {
      ofPushStyle();
      ofSetColor(150, 155, 170);
      DrawTextNormal("cable visualizers into the nub →", 4, (int)kListTop + 6, 12);
      ofPopStyle();
      return;
   }

   ofPushStyle();
   for (int i = 0; i < count; ++i)
   {
      float ry = kListTop + i * kRowH;
      if (i == mSelectedLayer)
      {
         ofFill();
         ofSetColor(70, 90, 150, 120);
         ofRect(kLabelX, ry, mWidth - 12, kRowH - 2);
      }
      ofSetColor(215, 220, 230);
      std::string name = mLayerData[i].module ? mLayerData[i].module->Name() : "?";
      std::string tag = (i == 0) ? " (base)" : "";
      DrawTextNormal(name + tag, (int)kLabelX + 3, (int)ry + 12, 11);

      mLayerData[i].opacitySlider->SetPosition(110, ry + 1);
      mLayerData[i].blendDropdown->SetPosition(185, ry + 1);
      mLayerData[i].opacitySlider->Draw();
      mLayerData[i].blendDropdown->Draw();
   }
   ofPopStyle();

   float yTop = kListTop + count * kRowH + 6;
   float px = 4;
   float py = yTop;
   float pw = mWidth - 8;
   float ph = mHeight - yTop - 4;
   if (ph < 20 || pw < 20)
      return;

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

   //frame size for the chosen ratio (internal render resolution, capped for speed). "source" tracks
   //the preview box; the fixed ratios use a constant frame that layers are aspect-fit into
   int previewIw = MIN(kMaxInternalDim, MAX(16, (int)pw));
   int previewIh = MIN(kMaxInternalDim, MAX(16, (int)ph));
   int fw, fh;
   if (mAspect == 1) //16:9
   {
      fw = kMaxInternalDim;
      fh = (kMaxInternalDim * 9) / 16;
   }
   else if (mAspect == 2) //1:1
   {
      fw = kMaxInternalDim;
      fh = kMaxInternalDim;
   }
   else if (mAspect == 3) //9:16
   {
      fw = (kMaxInternalDim * 9) / 16;
      fh = kMaxInternalDim;
   }
   else //source
   {
      fw = previewIw;
      fh = previewIh;
   }
   fw &= ~1;
   fh &= ~1;

   EnsureResources(fw, fh);
   CookIfNeeded(0);

   //draw the result letterboxed inside the preview box so its true ratio shows, centered
   float frameAsp = (float)mFBW / (float)MAX(1, mFBH);
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
   DrawPreview(dx, dy, dw, dh, count);

   //preview frame (around the fitted image)
   ofPushStyle();
   ofNoFill();
   ofSetColor(120, 125, 145);
   ofRect(dx, dy, dw, dh);
   ofPopStyle();
}

void Composite::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void Composite::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void Composite::SetUpFromSaveData()
{
}

unsigned int Composite::GetOutputTexture()
{
   return mResultImage >= 0 ? mResultFB->texture : 0;
}

void Composite::CookIfNeeded(int frameId)
{
   if (mLastCookFrame == frameId && frameId != 0)
      return;
   mLastCookFrame = frameId;

   if (mResultFB == nullptr)
      return;

   //throttle the expensive re-render+composite to the chosen max fps
   double interval = 1000.0 / (double)MAX(1, mMaxFps);
   if (gTime - mLastCompositeMs >= interval)
   {
      SyncLayers();

      int count = 0;
      for (int i = 0; i < kMaxLayers; ++i)
      {
         if (mLayerData[i].module != nullptr)
            count++;
      }

      for (int i = 0; i < kMaxLayers; ++i)
         mLayerAspect[i] = 1.0f;

      for (int i = 0; i < count; ++i)
      {
         IVisualNode* vn = dynamic_cast<IVisualNode*>(mLayerData[i].module);
         if (vn != nullptr)
         {
            vn->CookIfNeeded(frameId);
            mLayerTex[i] = vn->GetOutputTexture();
            int lw = vn->GetOutputWidth(), lh = vn->GetOutputHeight();
            mLayerAspect[i] = (lh > 0) ? (float)lw / (float)lh : 1.0f;
         }
         else
         {
            mLayerTex[i] = 0;
         }
      }
      CompositeLayers(count, mFBW, mFBH);
      mLastCompositeMs = gTime;
   }
}
