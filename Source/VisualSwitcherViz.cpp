#include "VisualSwitcherViz.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "VizGL.h"
#include "PatchCable.h"
#include "ModuleFactory.h"
#include "FileStream.h"
#include "juce_opengl/juce_opengl.h"

using namespace juce::gl;

VisualSwitcherViz::VisualSwitcherViz()
{
}

VisualSwitcherViz::~VisualSwitcherViz()
{
   TheTransport->RemoveListener(this);
}

const char* kCopyFragSrc =
"#version 150\n"
"in vec2 vUv;\n"
"out vec4 fragColor;\n"
"uniform sampler2D uTex;\n"
"void main(){\n"
"  fragColor = texture(uTex, vUv);\n"
"}\n";

bool VisualSwitcherViz::EnsureShader()
{
   if (mProgram != 0)
      return true;
   mProgram = VizGL::CompileProgram(kCopyFragSrc);
   return mProgram != 0;
}

void VisualSwitcherViz::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mIntervalSelector = new DropdownList(this, "interval", 4, 18, (int*)&mInterval, 90);
   mIntervalSelector->AddLabel("16", kInterval_16);
   mIntervalSelector->AddLabel("8", kInterval_8);
   mIntervalSelector->AddLabel("4", kInterval_4);
   mIntervalSelector->AddLabel("2", kInterval_2);
   mIntervalSelector->AddLabel("1", kInterval_1n);
   mIntervalSelector->AddLabel("1n", kInterval_1n);
   mIntervalSelector->AddLabel("2n", kInterval_2n);
   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("4nt", kInterval_4nt);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("8nt", kInterval_8nt);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("16nt", kInterval_16nt);
   mIntervalSelector->AddLabel("32n", kInterval_32n);
   mIntervalSelector->AddLabel("64n", kInterval_64n);

   mTargetCable = new PatchCableSource(this, kConnectionType_Special);
   mTargetCable->SetDefaultPatchBehavior(kDefaultPatchBehavior_Add);
   mTargetCable->SetManualPosition(mWidth - 12, 12);
   AddPatchCableSource(mTargetCable);

   UpdateTransportListener();
}

void VisualSwitcherViz::UpdateTransportListener()
{
   TransportListenerInfo* info = TheTransport->GetListenerInfo(this);
   if (info != nullptr)
      info->mInterval = mInterval;
   else
      TheTransport->AddListener(this, mInterval, OffsetInfo(0, false), false);
}

void VisualSwitcherViz::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mIntervalSelector)
      UpdateTransportListener();
}

void VisualSwitcherViz::OnTimeEvent(double time)
{
   auto cables = mTargetCable->GetPatchCables();
   int numTargets = 0;
   for (auto* cable : cables)
   {
      if (dynamic_cast<IVisualNode*>(cable->GetTarget()) != nullptr)
         numTargets++;
   }

   if (numTargets > 0)
   {
      mActiveIndex = (mActiveIndex + 1) % numTargets;
   }
   else
   {
      mActiveIndex = 0;
   }
}

void VisualSwitcherViz::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   // Clamp mActiveIndex just in case a cable was removed
   auto cables = mTargetCable->GetPatchCables();
   int numTargets = 0;
   for (auto* cable : cables)
   {
      if (dynamic_cast<IVisualNode*>(cable->GetTarget()) != nullptr)
         numTargets++;
   }

   if (numTargets > 0)
      mActiveIndex = mActiveIndex % numTargets;
   else
      mActiveIndex = 0;
}

void VisualSwitcherViz::Cook()
{
   mCurrentOutputTexture = 0;
   mCurrentOutputWidth = 0;
   mCurrentOutputHeight = 0;

   auto cables = mTargetCable->GetPatchCables();
   std::vector<IVisualNode*> targets;
   for (auto* cable : cables)
   {
      IVisualNode* target = dynamic_cast<IVisualNode*>(cable->GetTarget());
      if (target != nullptr)
         targets.push_back(target);
   }

   if (!targets.empty())
   {
      mActiveIndex = mActiveIndex % targets.size();
      IVisualNode* activeNode = targets[mActiveIndex];
      if (activeNode != nullptr)
      {
         activeNode->CookIfNeeded(mLastCookFrame);
         mCurrentOutputTexture = activeNode->GetOutputTexture();
         mCurrentOutputWidth = activeNode->GetOutputWidth();
         mCurrentOutputHeight = activeNode->GetOutputHeight();
      }
   }

   if (mCurrentOutputTexture != 0 && EnsureShader())
   {
      if (VizGL::EnsureFbo(mOut, mCurrentOutputWidth, mCurrentOutputHeight))
      {
         VizGL::RunShaderPass(mOut, mProgram, [&]()
                              {
                                 glActiveTexture(GL_TEXTURE0);
                                 glBindTexture(GL_TEXTURE_2D, mCurrentOutputTexture);
                                 glUniform1i(glGetUniformLocation(mProgram, "uTex"), 0);
                              });
      }
   }
}

void VisualSwitcherViz::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mIntervalSelector->Draw();

   float px = 4;
   float py = 36;
   float pw = mWidth - 8;
   float ph = mHeight - py - 4;
   if (ph < 20 || pw < 20)
      return;

   Cook();
   if (VizGL::FboTexture(mOut) != 0)
   {
      float frameAsp = (float)mCurrentOutputWidth / (float)MAX(1, mCurrentOutputHeight);
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

void VisualSwitcherViz::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();
   IDrawableModule::SaveState(out);
}

void VisualSwitcherViz::LoadState(FileStreamIn& in, int rev)
{
   int moduleRev;
   in >> moduleRev;
   IDrawableModule::LoadState(in, rev);
}
