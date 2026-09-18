/**
    bespoke synth - VizGL

    Shared GPU helper for the visual-node system. Centralizes all the fragile OpenGL/nanovg plumbing
    (Core-profile shaders, VAOs, offscreen framebuffers, GL state save/restore) so a visual node only
    has to supply a fragment shader and a uniform-setup callback.

    Everything here runs on the GL/main thread. macOS Core-profile OpenGL.
**/

#pragma once

#include <functional>

struct NVGLUframebuffer;
class IDrawableModule;

namespace VizGL
{
   //an offscreen render target (color texture). Reused across frames; only reallocated on resize.
   struct Fbo
   {
      NVGLUframebuffer* fb{ nullptr };
      int w{ 0 };
      int h{ 0 };
   };

   //(re)allocate the fbo to w x h if needed. returns true if usable.
   bool EnsureFbo(Fbo& f, int w, int h);
   void DestroyFbo(Fbo& f);
   unsigned int FboTexture(const Fbo& f); //GL texture id (0 if none)

   void BindFbo(const Fbo& f);
   void UnbindFbo();

   //a render target WITH a real depth buffer, for 3D mesh rendering (the nanovg FBOs are
   //color+stencil only). color is an RGBA8 texture you can display via DrawTexture.
   struct Fbo3D
   {
      unsigned int fbo{ 0 };
      unsigned int colorTex{ 0 };
      unsigned int depthRbo{ 0 };
      int w{ 0 };
      int h{ 0 };
   };
   bool EnsureFbo3D(Fbo3D& f, int w, int h);
   void DestroyFbo3D(Fbo3D& f);
   unsigned int Fbo3DTexture(const Fbo3D& f);

   //compile a fragment shader into a program using the shared fullscreen-quad vertex shader.
   //the fragment shader gets: in vec2 vUv; out vec4 fragColor; (GLSL 150 core). returns 0 on failure
   //(and prints the compile/link log to stdout). caller owns the returned program.
   unsigned int CompileProgram(const char* fragSrc);

   //run a fullscreen shader pass into `out`. `program` is glUseProgram'd, then `setup` is called to
   //set uniforms / bind input textures, then a fullscreen quad is drawn. GL state is saved/restored
   //so the surrounding nanovg frame is undisturbed.
   void RunShaderPass(const Fbo& out, unsigned int program, const std::function<void()>& setup);

   //render an existing drawable module (e.g. a legacy CPU visualizer) into `out`, cover-fit, with its
   //own UI controls hidden. lets legacy visualizers act as texture sources in the node graph.
   void RenderModuleToFbo(IDrawableModule* module, const Fbo& out);

   //draw a GL texture into the current nanovg frame at the given rect (wraps it as a nanovg image,
   //cached per texture id + size). use this for a node's on-screen preview.
   void DrawTexture(unsigned int tex, float x, float y, float w, float h);
}
