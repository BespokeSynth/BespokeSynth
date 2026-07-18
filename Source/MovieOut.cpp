/**
    bespoke synth - MovieOut (implementation)
**/

#include "MovieOut.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "ModuleFactory.h"
#include "PatchCableSource.h"
#include "IUIControl.h"
#include "IVisualNode.h"
#include <vector>

MovieOut::MovieOut()
{
}

MovieOut::~MovieOut()
{
}

void MovieOut::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mAspectSelector = new DropdownList(this, "ratio", 6, 40, &mAspect, 90);
   mScaleSlider = new IntSlider(this, "scale", 6, 58, 120, 14, &mScale, 1, 4);
   mRecordButton = new ClickButton(this, "record", 6, 78);

   mAspectSelector->AddLabel("source", 0);
   mAspectSelector->AddLabel("16:9", 1);
   mAspectSelector->AddLabel("1:1", 2);
   mAspectSelector->AddLabel("9:16", 3);

   //a cable you drag onto the visualizer you want to capture
   mTargetCable = new PatchCableSource(this, kConnectionType_Special);
   mTargetCable->SetManualPosition(mWidth - 12, 20);
   AddPatchCableSource(mTargetCable);
}

void MovieOut::ComputeFrameSize(float tw, float th, int& fw, int& fh) const
{
   float sc = (float)mScale;
   switch (mAspect)
   {
      case 1: //16:9, keyed to module width (landscape crop)
         fw = (int)(tw * sc);
         fh = (int)(fw * 9.0f / 16.0f);
         break;
      case 2: //1:1, keyed to the longer side
      {
         int s = (int)(MAX(tw, th) * sc);
         fw = s;
         fh = s;
         break;
      }
      case 3: //9:16, keyed to module width
         fw = (int)(tw * sc);
         fh = (int)(fw * 16.0f / 9.0f);
         break;
      default: //source: exactly the module box * scale
         fw = (int)(tw * sc);
         fh = (int)(th * sc);
         break;
   }
}

IDrawableModule* MovieOut::GetTarget() const
{
   if (mTargetCable == nullptr)
      return nullptr;
   return dynamic_cast<IDrawableModule*>(mTargetCable->GetTarget());
}

void MovieOut::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   //only allow cabling to visualizer modules - reject anything else and clear the cable
   IDrawableModule* target = GetTarget();
   if (target != nullptr)
   {
      ModuleCategory cat = TheSynth->GetModuleFactory()->GetModuleCategory(target->GetTypeName());
      if (cat != kModuleCategory_Visualizer)
      {
         mTargetCable->SetTarget(nullptr);
         mNote = "visualizers only";
      }
      else
      {
         mNote = "";
      }
   }
}

void MovieOut::ButtonClicked(ClickButton* button, double time)
{
   if (button == mRecordButton)
   {
      if (mRecorder.IsRecording())
      {
         mRecorder.Toggle(0, 0); //stop
      }
      else
      {
         IDrawableModule* target = GetTarget();
         if (target == nullptr)
            return;
         float tw = 0, th = 0;
         target->GetDimensions(tw, th);
         ComputeFrameSize(tw, th, mFrameW, mFrameH);
         mRecorder.Toggle(mFrameW, mFrameH);
      }
   }
}

void MovieOut::Poll()
{
   IDrawableModule* target = GetTarget();
   if (mRecorder.IsRecording() && target == nullptr)
      mRecorder.Toggle(0, 0); //target disappeared; stop cleanly
}

void MovieOut::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mAspectSelector->Draw();
   mScaleSlider->Draw();
   mRecordButton->Draw();

   IDrawableModule* target = GetTarget();

   ofPushStyle();
   //target label
   ofSetColor(200, 205, 215);
   std::string tname = target ? target->Name() : "(cable a visualizer)";
   DrawTextNormal(tname, 6, 20, 11);

   //status line
   if (mRecorder.IsRecording())
   {
      ofSetColor(255, 80, 80);
      ofFill();
      ofCircle(mWidth - 16, 82, 4); //blinking-ish REC dot
      ofSetColor(255, 120, 120);
      DrawTextNormal("REC", mWidth - 46, 86, 11);
   }
   else if (!mNote.empty())
   {
      ofSetColor(240, 180, 90);
      DrawTextNormal(mNote, 6, 108, 9);
   }
   else if (!mRecorder.Status().empty())
   {
      ofSetColor(150, 210, 160);
      DrawTextNormal(mRecorder.Status(), 6, 108, 9);
   }
   ofPopStyle();

   //do the actual capture AFTER drawing our own UI, re-rendering the target offscreen,
   //centered and cover-fit into the chosen aspect-ratio frame
   if (mRecorder.IsRecording() && target != nullptr)
   {
      //fast path: IVisualNode targets expose a GPU texture — read pixels directly without
      //re-rendering through NanoVG. This avoids context collisions and extra copies.
      IVisualNode* vn = dynamic_cast<IVisualNode*>(target);
      if (vn != nullptr)
      {
         mRecorder.CaptureFrameFromTexture(vn, mFrameW, mFrameH);
      }
      else
      {
         //legacy path: re-render the module into an offscreen NanoVG framebuffer
         float tx = 0, ty = 0;
         target->GetPosition(tx, ty);
         float fw = (float)mFrameW;
         float fh = (float)mFrameH;
         mRecorder.CaptureFrame([target, tx, ty, fw, fh]()
                                {
                                   //hide the target's controls for the recorded frame so we capture
                                   //just the visualization (the live module on screen is untouched)
                                   std::vector<IUIControl*> controls = target->GetUIControls();
                                   std::vector<bool> wasShowing(controls.size());
                                   for (size_t i = 0; i < controls.size(); ++i)
                                   {
                                      wasShowing[i] = controls[i]->IsShowing();
                                      controls[i]->SetShowing(false);
                                   }

                                   float tw = 1, th = 1;
                                   target->GetDimensions(tw, th);
                                   if (tw < 1)
                                      tw = 1;
                                   if (th < 1)
                                      th = 1;
                                   //cover-fit: scale so the module fills the frame, then center it
                                   //(overflow is clipped by the framebuffer viewport)
                                   float cover = MAX(fw / tw, fh / th);
                                   float offX = (fw - tw * cover) * 0.5f;
                                   float offY = (fh - th * cover) * 0.5f;

                                   ofPushMatrix();
                                   ofTranslate(offX, offY, 0);
                                   ofScale(cover, cover, 1);
                                   ofTranslate(-tx, -ty, 0);
                                   target->Render();
                                   ofPopMatrix();

                                   for (size_t i = 0; i < controls.size(); ++i)
                                      controls[i]->SetShowing(wasShowing[i]);
                                });
      }
   }
}

void MovieOut::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void MovieOut::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void MovieOut::SetUpFromSaveData()
{
}
