/**
    bespoke synth - VizGL (implementation)
**/

#include "VizGL.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "IDrawableModule.h"
#include "IClickable.h"
#include "IUIControl.h"
#include "juce_opengl/juce_opengl.h"
#include <cstdio>
#include <map>
#include <vector>

using namespace juce::gl;

#include "nanovg/nanovg.h"
#define NANOVG_GLES2_IMPLEMENTATION
#include "nanovg/nanovg_gl.h"
#include "nanovg/nanovg_gl_utils.h"

namespace
{
   //shared fullscreen quad + VAO, created once
   GLuint sVao = 0;
   GLuint sVbo = 0;
   bool sQuadReady = false;

   const char* kVertSrc =
   "#version 150\n"
   "in vec2 aPos;\n"
   "in vec2 aUv;\n"
   "out vec2 vUv;\n"
   "void main(){ vUv = aUv; gl_Position = vec4(aPos, 0.0, 1.0); }\n";

   GLuint CompileStage(GLenum type, const char* src)
   {
      GLuint sh = glCreateShader(type);
      glShaderSource(sh, 1, &src, nullptr);
      glCompileShader(sh);
      GLint ok = 0;
      glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
      if (!ok)
      {
         char log[2048];
         GLsizei n = 0;
         glGetShaderInfoLog(sh, sizeof(log), &n, log);
         printf("[VizGL] shader compile error: %s\n", log);
         fflush(stdout);
         glDeleteShader(sh);
         return 0;
      }
      return sh;
   }

   void EnsureQuad()
   {
      if (sQuadReady)
         return;
      const float quad[] = {
         -1, -1, 0, 0,
         1, -1, 1, 0,
         -1, 1, 0, 1,
         1, 1, 1, 1
      };
      GLint prevVao = 0, prevBuf = 0;
      glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prevVao);
      glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &prevBuf);
      glGenVertexArrays(1, &sVao);
      glBindVertexArray(sVao);
      glGenBuffers(1, &sVbo);
      glBindBuffer(GL_ARRAY_BUFFER, sVbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
      //attribute locations are fixed (0 = aPos, 1 = aUv) and bound the same in every program
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
      glBindVertexArray((GLuint)prevVao);
      glBindBuffer(GL_ARRAY_BUFFER, (GLuint)prevBuf);
      sQuadReady = true;
   }

   //cache of nanovg images wrapping GL textures for on-screen drawing
   struct ImgEntry
   {
      int image{ -1 };
      int w{ 0 };
      int h{ 0 };
   };
   std::map<unsigned int, ImgEntry> sImageCache;
}

namespace VizGL
{
   bool EnsureFbo(Fbo& f, int w, int h)
   {
      w = MAX(16, w);
      h = MAX(16, h);
      if (f.fb != nullptr && f.w == w && f.h == h)
         return true;

      NVGcontext* recVG = gNanoVGRenderContexts[(int)NanoVGRenderContext::Screenshot];
      if (recVG == nullptr)
         return false;

      if (f.fb != nullptr)
         nvgluDeleteFramebuffer(f.fb);
      f.fb = nvgluCreateFramebuffer(recVG, w, h, 0);
      f.w = w;
      f.h = h;
      return f.fb != nullptr;
   }

   void DestroyFbo(Fbo& f)
   {
      if (f.fb != nullptr)
      {
         nvgluDeleteFramebuffer(f.fb);
         f.fb = nullptr;
      }
      f.w = 0;
      f.h = 0;
   }

   unsigned int FboTexture(const Fbo& f)
   {
      return f.fb != nullptr ? f.fb->texture : 0;
   }

   void BindFbo(const Fbo& f)
   {
      if (f.fb)
         nvgluBindFramebuffer(f.fb);
   }

   void UnbindFbo()
   {
      nvgluBindFramebuffer(nullptr);
   }

   bool EnsureFbo3D(Fbo3D& f, int w, int h)
   {
      w = MAX(16, w);
      h = MAX(16, h);
      if (f.fbo != 0 && f.w == w && f.h == h)
         return true;
      DestroyFbo3D(f);

      GLint prevFbo = 0, prevTex = 0, prevRbo = 0;
      glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);
      glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevTex);
      glGetIntegerv(GL_RENDERBUFFER_BINDING, &prevRbo);

      glGenFramebuffers(1, &f.fbo);
      glBindFramebuffer(GL_FRAMEBUFFER, f.fbo);

      glGenTextures(1, &f.colorTex);
      glBindTexture(GL_TEXTURE_2D, f.colorTex);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, f.colorTex, 0);

      glGenRenderbuffers(1, &f.depthRbo);
      glBindRenderbuffer(GL_RENDERBUFFER, f.depthRbo);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, f.depthRbo);

      bool ok = (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

      glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)prevFbo);
      glBindTexture(GL_TEXTURE_2D, (GLuint)prevTex);
      glBindRenderbuffer(GL_RENDERBUFFER, (GLuint)prevRbo);

      f.w = w;
      f.h = h;
      return ok;
   }

   void DestroyFbo3D(Fbo3D& f)
   {
      if (f.fbo != 0)
         glDeleteFramebuffers(1, &f.fbo);
      if (f.colorTex != 0)
         glDeleteTextures(1, &f.colorTex);
      if (f.depthRbo != 0)
         glDeleteRenderbuffers(1, &f.depthRbo);
      f.fbo = 0;
      f.colorTex = 0;
      f.depthRbo = 0;
      f.w = 0;
      f.h = 0;
   }

   unsigned int Fbo3DTexture(const Fbo3D& f)
   {
      return f.colorTex;
   }

   unsigned int CompileProgram(const char* fragSrc)
   {
      GLuint vs = CompileStage(GL_VERTEX_SHADER, kVertSrc);
      GLuint fs = CompileStage(GL_FRAGMENT_SHADER, fragSrc);
      if (vs == 0 || fs == 0)
         return 0;

      GLuint prog = glCreateProgram();
      glAttachShader(prog, vs);
      glAttachShader(prog, fs);
      glBindAttribLocation(prog, 0, "aPos");
      glBindAttribLocation(prog, 1, "aUv");
      glLinkProgram(prog);
      GLint ok = 0;
      glGetProgramiv(prog, GL_LINK_STATUS, &ok);
      glDeleteShader(vs);
      glDeleteShader(fs);
      if (!ok)
      {
         char log[2048];
         GLsizei n = 0;
         glGetProgramInfoLog(prog, sizeof(log), &n, log);
         printf("[VizGL] program link error: %s\n", log);
         fflush(stdout);
         glDeleteProgram(prog);
         return 0;
      }
      return prog;
   }

   void RunShaderPass(const Fbo& out, unsigned int program, const std::function<void()>& setup)
   {
      if (out.fb == nullptr || program == 0)
         return;
      EnsureQuad();

      GLint prevFBO = 0, prevVp[4] = { 0, 0, 0, 0 }, prevProg = 0, prevVao = 0;
      glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO);
      glGetIntegerv(GL_VIEWPORT, prevVp);
      glGetIntegerv(GL_CURRENT_PROGRAM, &prevProg);
      glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prevVao);
      GLboolean prevBlend = glIsEnabled(GL_BLEND);
      GLboolean prevScissor = glIsEnabled(GL_SCISSOR_TEST);

      glBindFramebuffer(GL_FRAMEBUFFER, out.fb->fbo);
      glViewport(0, 0, out.w, out.h);
      glDisable(GL_BLEND);
      glDisable(GL_SCISSOR_TEST);
      glClearColor(0, 0, 0, 1);
      glClear(GL_COLOR_BUFFER_BIT);

      glUseProgram(program);
      if (setup)
         setup();

      glBindVertexArray(sVao);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

      glBindVertexArray((GLuint)prevVao);
      glUseProgram((GLuint)prevProg);
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

   void RenderModuleToFbo(IDrawableModule* module, const Fbo& out)
   {
      NVGcontext* recVG = gNanoVGRenderContexts[(int)NanoVGRenderContext::Screenshot];
      if (recVG == nullptr || out.fb == nullptr || module == nullptr)
         return;

      GLint prevFBO = 0;
      GLint prevVp[4] = { 0, 0, 0, 0 };
      glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO);
      glGetIntegerv(GL_VIEWPORT, prevVp);
      NVGcontext* mainVG = gNanoVG;

      gNanoVG = recVG;
      nvgluBindFramebuffer(out.fb);
      glViewport(0, 0, out.w, out.h);
      glClearColor(0, 0, 0, 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
      nvgBeginFrame(recVG, out.w, out.h, 1);

      float tw = 1, th = 1;
      module->GetDimensions(tw, th);
      if (tw < 1)
         tw = 1;
      if (th < 1)
         th = 1;
      float tx = 0, ty = 0;
      module->GetPosition(tx, ty);
      float cover = MAX((float)out.w / tw, (float)out.h / th);
      float offX = (out.w - tw * cover) * 0.5f;
      float offY = (out.h - th * cover) * 0.5f;

      std::vector<IUIControl*> controls = module->GetUIControls();
      std::vector<bool> wasShowing(controls.size());
      for (size_t c = 0; c < controls.size(); ++c)
      {
         wasShowing[c] = controls[c]->IsShowing();
         controls[c]->SetShowing(false);
      }

      ofPushMatrix();
      ofTranslate(offX, offY, 0);
      ofScale(cover, cover, 1);
      ofTranslate(-tx, -ty, 0);
      module->Render();
      ofPopMatrix();

      for (size_t c = 0; c < controls.size(); ++c)
         controls[c]->SetShowing(wasShowing[c]);

      nvgEndFrame(recVG);
      nvgluBindFramebuffer(nullptr);
      glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)prevFBO);
      glViewport(prevVp[0], prevVp[1], prevVp[2], prevVp[3]);
      gNanoVG = mainVG;
   }

   void DrawTexture(unsigned int tex, float x, float y, float w, float h)
   {
      if (tex == 0)
         return;
      ImgEntry& e = sImageCache[tex];
      if (e.image < 0 || e.w != (int)w || e.h != (int)h)
      {
         if (e.image >= 0)
            nvgDeleteImage(gNanoVG, e.image);
         e.image = nvglCreateImageFromHandleGLES2(gNanoVG, (GLuint)tex, (int)w, (int)h, NVG_IMAGE_FLIPY | NVG_IMAGE_NODELETE);
         e.w = (int)w;
         e.h = (int)h;
      }
      if (e.image < 0)
         return;
      NVGpaint paint = nvgImagePattern(gNanoVG, x, y, w, h, 0.0f, e.image, 1.0f);
      nvgBeginPath(gNanoVG);
      nvgRect(gNanoVG, x, y, w, h);
      nvgFillPaint(gNanoVG, paint);
      nvgFill(gNanoVG);
   }
}
