/**
    bespoke synth - TransformViz
**/

#include "TransformViz.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "juce_opengl/juce_opengl.h"
#include "PatchCable.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include <cmath>

using namespace juce::gl;

namespace
{
   const char* kFragSrc =
   "#version 150\n"
   "in vec2 vUv;\n"
   "out vec4 fragColor;\n"
   "uniform sampler2D uTex;\n"
   "uniform vec2 uTrans;\n"
   "uniform vec2 uScale;\n"
   "uniform float uRotate;\n"
   "uniform vec2 uRes;\n"
   "uniform vec2 uTexRes;\n"
   "uniform float uFill;\n"
   "void main(){\n"
   "  vec2 uv = vUv - 0.5;\n"
   "  float ratio = uRes.x / max(1.0, uRes.y) / max(0.0001, uTexRes.x / max(1.0, uTexRes.y));\n"
   "  if (uFill > 0.5) {\n"
   "     if (ratio > 1.0) uv.y /= ratio;\n"
   "     else uv.x *= ratio;\n"
   "  } else {\n"
   "     if (ratio > 1.0) uv.x *= ratio;\n"
   "     else uv.y /= ratio;\n"
   "  }\n"
   "  uv /= uScale;\n"
   "  float c = cos(-uRotate);\n"
   "  float s = sin(-uRotate);\n"
   "  uv = vec2(uv.x * c - uv.y * s, uv.x * s + uv.y * c);\n"
   "  uv += 0.5;\n"
   "  uv -= uTrans;\n"
   "  if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {\n"
   "     fragColor = vec4(0.0);\n"
   "  } else {\n"
   "     fragColor = texture(uTex, uv);\n"
   "  }\n"
   "}\n";
}

TransformViz::TransformViz()
{
}

TransformViz::~TransformViz()
{
   VizGL::DestroyFbo(mOut);
}


void TransformViz::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mTransXSlider = new FloatSlider(this, "trans x", 4, 18, 160, 14, &mTransX, -2.0f, 2.0f);
   mTransYSlider = new FloatSlider(this, "trans y", 4, 36, 160, 14, &mTransY, -2.0f, 2.0f);
   mScaleXSlider = new FloatSlider(this, "scale x", 4, 54, 160, 14, &mScaleX, 0.01f, 5.0f);
   mScaleYSlider = new FloatSlider(this, "scale y", 4, 72, 160, 14, &mScaleY, 0.01f, 5.0f);
   mRotateSlider = new FloatSlider(this, "rotate", 4, 90, 160, 14, &mRotate, -3.14159f * 2, 3.14159f * 2);

   mAspectSelector = new DropdownList(this, "ratio", 4, 108, &mAspect, 90);
   mAspectSelector->AddLabel("source", 0);
   mAspectSelector->AddLabel("16:9", 1);
   mAspectSelector->AddLabel("1:1", 2);
   mAspectSelector->AddLabel("9:16", 3);

   mFillCheckbox = new Checkbox(this, "fill", 98, 108, &mFill);

   mTargetCable = new PatchCableSource(this, kConnectionType_Special);
   mTargetCable->SetManualPosition(mWidth - 12, 12);
   AddPatchCableSource(mTargetCable);
}

void TransformViz::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   // Optional: validate or cache connected visual node
}

void TransformViz::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

bool TransformViz::EnsureShader()
{
   if (mShaderTried)
      return mProgram != 0;
   mShaderTried = true;

   mProgram = VizGL::CompileProgram(kFragSrc);
   if (mProgram != 0)
   {
      mLocTrans = glGetUniformLocation(mProgram, "uTrans");
      mLocScale = glGetUniformLocation(mProgram, "uScale");
      mLocRotate = glGetUniformLocation(mProgram, "uRotate");
      mLocRes = glGetUniformLocation(mProgram, "uRes");
      mLocTexRes = glGetUniformLocation(mProgram, "uTexRes");
      mLocFill = glGetUniformLocation(mProgram, "uFill");
   }
   return mProgram != 0;
}

void TransformViz::CookIfNeeded(int frameId)
{
   if (mLastCookFrame == frameId)
      return;
   mLastCookFrame = frameId;
   Cook();
}

void TransformViz::Cook()
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

   int fw, fh;
   if (mAspect == 1) // 16:9
   {
      fw = 1920;
      fh = 1080;
   }
   else if (mAspect == 2) // 1:1
   {
      fw = 1080;
      fh = 1080;
   }
   else if (mAspect == 3) // 9:16
   {
      fw = 1080;
      fh = 1920;
   }
   else
   {
      fw = inW;
      fh = inH;
   }

   mResW = MAX(1080, (fw & ~1));
   mResH = MAX(1080, (fh & ~1));

   if (mResW <= 0 || mResH <= 0)
      return;

   // make sure the output FBO is the same size as input
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
                           glUniform1f(mLocFill, mFill ? 1.0f : 0.0f);
                           glUniform2f(mLocTrans, mTransX, mTransY);
                           glUniform2f(mLocScale, mScaleX, mScaleY);
                           glUniform1f(mLocRotate, mRotate);

                           glActiveTexture(GL_TEXTURE0);
                           glBindTexture(GL_TEXTURE_2D, inTex);
                           glUniform1i(glGetUniformLocation(mProgram, "uTex"), 0);
                        });
}

void TransformViz::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mTransXSlider->Draw();
   mTransYSlider->Draw();
   mScaleXSlider->Draw();
   mScaleYSlider->Draw();
   mRotateSlider->Draw();
   mAspectSelector->Draw();
   mFillCheckbox->Draw();

   float px = 4;
   float py = 126;
   float pw = mWidth - 8;
   float ph = mHeight - py - 4;
   if (ph < 20 || pw < 20)
      return;

   ofPushStyle();
   ofFill();
   ofSetColor(10, 10, 10);
   ofRect(px, py, pw, ph);
   ofPopStyle();

   // Draw the output texture as a preview
   Cook(); // draw module is called once per frame typically, so we can ensure cook here
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
