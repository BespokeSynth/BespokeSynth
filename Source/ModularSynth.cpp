#include "ModularSynth.h"
#include "IAudioSource.h"
#include "IAudioEffect.h"
#include "SynthGlobals.h"
#include "Scale.h"
#include "Transport.h"
#include "TextEntry.h"
#include "InputChannel.h"
#include "OutputChannel.h"
#include "TitleBar.h"
#include "LFOController.h"
#include "MidiController.h"
#include "ChaosEngine.h"
#include "ModuleSaveDataPanel.h"
#include "Profiler.h"
#include "MultitrackRecorder.h"
#include "Sample.h"
#include "FloatSliderLFOControl.h"
//#include <CoreServices/CoreServices.h>
#include "fenv.h"
#include <stdlib.h>
#include "GridController.h"
#include "PerformanceTimer.h"
#include "FileStream.h"
#include "PatchCable.h"
#include "ADSRDisplay.h"
#include "../JuceLibraryCode/JuceHeader.h"
#include "QuickSpawnMenu.h"
#include "AudioToCV.h"
#include "ScriptModule.h"
#include "DrumPlayer.h"
#include "VSTPlugin.h"
#include "Prefab.h"
#include "HelpDisplay.h"
#include "nanovg/nanovg.h"
#include "UserPrefsEditor.h"
#include "Canvas.h"
#include "EffectChain.h"

ModularSynth* TheSynth = nullptr;

//static
bool ModularSynth::sShouldAutosave = true;
float ModularSynth::sBackgroundLissajousR = 0.408f;
float ModularSynth::sBackgroundLissajousG = 0.245f;
float ModularSynth::sBackgroundLissajousB = 0.418f;

void AtExit()
{
   TheSynth->Exit();
}

ModularSynth::ModularSynth()
: mMoveModule(nullptr)
, mIsMousePanning(false)
, mGlobalRecordBuffer(nullptr)
, mAudioPaused(false)
, mIsLoadingState(false)
, mClickStartX(INT_MAX)
, mClickStartY(INT_MAX)
, mWantReloadInitialLayout(false)
, mHeldSample(nullptr)
, mConsoleListener(nullptr)
, mUserPrefsEditor(nullptr)
, mLastClickedModule(nullptr)
, mInitialized(false)
, mRecordingLength(0)
, mGroupSelectContext(nullptr)
, mResizeModule(nullptr)
, mShowLoadStatePopup(false)
, mHasDuplicatedDuringDrag(false)
, mFrameRate(0)
, mQuickSpawn(nullptr)
, mScheduledEnvelopeEditorSpawnDisplay(nullptr)
, mFrameCount(0)
, mIsLoadingModule(false)
, mLastClapboardTime(-9999)
, mScrollMultiplierHorizontal(1)
, mScrollMultiplierVertical(1)
, mPixelRatio(1)
{
   mConsoleText[0] = 0;
   assert(TheSynth == nullptr);
   TheSynth = this;
}

ModularSynth::~ModularSynth()
{
   DeleteAllModules();
   
   delete mGlobalRecordBuffer;
   delete[] mSaveOutputBuffer[0];
   delete[] mSaveOutputBuffer[1];

   SetMemoryTrackingEnabled(false); //avoid crashes when the tracking lists themselves are deleted
   
   assert(TheSynth == this);
   TheSynth = nullptr;
   
   ScriptModule::UninitializePython();
}

bool ModularSynth::IsReady()
{
   return gTime > 100;
}

void ModularSynth::Setup(GlobalManagers* globalManagers, juce::Component* mainComponent)
{
   mGlobalManagers = globalManagers;
   mMainComponent = mainComponent;
   
   bool loaded = mUserPrefs.open(GetUserPrefsPath(false));
   if (loaded)
   {
      sShouldAutosave = mUserPrefs["autosave"].isNull() ? false : (mUserPrefs["autosave"].asInt() > 0);

      if (!mUserPrefs["scroll_multiplier_horizontal"].isNull())
         mScrollMultiplierHorizontal = mUserPrefs["scroll_multiplier_horizontal"].asDouble();
      if (!mUserPrefs["scroll_multiplier_vertical"].isNull())
         mScrollMultiplierVertical = mUserPrefs["scroll_multiplier_vertical"].asDouble();

      int recordBufferLengthMinutes = 30;
      if (!mUserPrefs["record_buffer_length_minutes"].isNull())
         recordBufferLengthMinutes = mUserPrefs["record_buffer_length_minutes"].asDouble();
      mGlobalRecordBuffer = new RollingBuffer(recordBufferLengthMinutes * 60 * gSampleRate);
      mGlobalRecordBuffer->SetNumChannels(2);
      mSaveOutputBuffer[0] = new float[mGlobalRecordBuffer->Size()];
      mSaveOutputBuffer[1] = new float[mGlobalRecordBuffer->Size()];

      juce::File(ofToDataPath("savestate")).createDirectory();
      juce::File(ofToDataPath("savestate/autosave")).createDirectory();
      juce::File(ofToDataPath("recordings")).createDirectory();
      juce::File(ofToDataPath("internal")).createDirectory();
      juce::File(ofToDataPath("samples")).createDirectory();
      juce::File(ofToDataPath("scripts")).createDirectory();
   }
   else
   {
      mFatalError = "couldn't find or load data/"+GetUserPrefsPath(true);
#if BESPOKE_MAC
      if (!juce::File(GetUserPrefsPath(false)).existsAsFile())
         mFatalError += "\nplease install to /Applications/BespokeSynth or launch via run_bespoke.command";
#endif
      LogEvent("couldn't find or load userprefs.json", kLogEventType_Error);
   }

   mIOBufferSize = gBufferSize;
   
   SynthInit();

   new Transport();
   new Scale();
   TheScale->CreateUIControls();
   TheTransport->CreateUIControls();

   TheScale->Init();
   TheTransport->Init();
   
   DrumPlayer::SetUpHitDirectories();
   
   ResetLayout();
   
   mConsoleListener = new ConsoleListener();
   mConsoleEntry = new TextEntry(mConsoleListener,"console",0,20,50,mConsoleText);
   mConsoleEntry->SetRequireEnter(true);
}

void ModularSynth::LoadResources(void* nanoVG, void* fontBoundsNanoVG)
{
   gNanoVG = (NVGcontext*)nanoVG;
   gFontBoundsNanoVG = (NVGcontext*)fontBoundsNanoVG;
   LoadGlobalResources();

   if (!gFont.IsLoaded())
      mFatalError = "couldn't load font from " + gFont.GetFontPath();
}

void ModularSynth::InitIOBuffers(int inputChannelCount, int outputChannelCount)
{
   for (int i = 0; i < inputChannelCount; ++i)
      mInputBuffers.push_back(new float[gBufferSize]);
   for (int i = 0; i < outputChannelCount; ++i)
      mOutputBuffers.push_back(new float[gBufferSize]);
}

//static
string ModularSynth::GetUserPrefsPath(bool relative)
{
   if (JUCEApplication::getCommandLineParameterArray().size() > 0)
   {
      string path = ofToDataPath(JUCEApplication::getCommandLineParameterArray()[0].toStdString());
      if (juce::File(path).existsAsFile())
      {
         if (relative)
            return JUCEApplication::getCommandLineParameterArray()[0].toStdString();
         return path;
      }
   }
   
   if (relative)
      return "userprefs.json";
   return ofToDataPath("userprefs.json");
}

static int sFrameCount = 0;
void ModularSynth::Poll()
{
   if (mFatalError == "")
   {
      if (!mInitialized && sFrameCount > 3) //let some frames render before blocking for a load
      {
         LoadLayoutFromFile(ofToDataPath(mUserPrefs["layout"].asString()));
         mInitialized = true;
      }

      if (mWantReloadInitialLayout)
      {
         LoadLayoutFromFile(ofToDataPath(TheSynth->GetUserPrefs()["layout"].asString()));
         mWantReloadInitialLayout = false;
      }
   }
   
   mZoomer.Update();
   
   if (!mIsLoadingState)
   {
      for (auto p : mExtraPollers)
         p->Poll();
      mModuleContainer.Poll();
   }
   
   if (mShowLoadStatePopup)
   {
      mShowLoadStatePopup = false;
      LoadStatePopupImp();
   }
   
   if (mScheduledEnvelopeEditorSpawnDisplay != nullptr)
   {
      mScheduledEnvelopeEditorSpawnDisplay->SpawnEnvelopeEditor();
      mScheduledEnvelopeEditorSpawnDisplay = nullptr;
   }

   {
      static MouseCursor sCurrentCursor = MouseCursor::NormalCursor;
      MouseCursor desiredCursor;

      if (mIsLoadingState)
      {
         desiredCursor = MouseCursor::WaitCursor;
      }
      else if (gHoveredUIControl != nullptr && gHoveredUIControl->IsMouseDown())
      {
         if (GetKeyModifiers() == kModifier_Shift)
            desiredCursor = MouseCursor::CrosshairCursor;
         else
            desiredCursor = MouseCursor::LeftRightResizeCursor;
      }
      else if (gHoveredUIControl != nullptr && gHoveredUIControl->IsTextEntry())
      {
         desiredCursor = MouseCursor::IBeamCursor;
      }
      else if (gHoveredUIControl != nullptr && dynamic_cast<Canvas*>(gHoveredUIControl) != nullptr)
      {
         desiredCursor = dynamic_cast<Canvas*>(gHoveredUIControl)->GetMouseCursorType();
      }
      else if (mIsMousePanning)
      {
         desiredCursor = MouseCursor::DraggingHandCursor;
      }
      else if (GetKeyModifiers() == kModifier_Shift)
      {
         desiredCursor = MouseCursor::PointingHandCursor;
      }
      else
      {
         desiredCursor = MouseCursor::NormalCursor;
      }

      if (desiredCursor != sCurrentCursor)
      {
         sCurrentCursor = desiredCursor;
         mMainComponent->setMouseCursor(desiredCursor);
      }
   }
   
   ++sFrameCount;
}

void ModularSynth::DeleteAllModules()
{
   mModuleContainer.Clear();
   
   for (int i=0; i<mDeletedModules.size(); ++i)
      delete mDeletedModules[i];
   mDeletedModules.clear();
   
   delete TheScale;
   TheScale = nullptr;
   delete TheTransport;
   TheTransport = nullptr;
   delete mConsoleListener;
   mConsoleListener = nullptr;
}

bool SortPointsByY(ofVec2f a, ofVec2f b)
{
   return a.y < b.y;
}

void ModularSynth::ZoomView(float zoomAmount, bool fromMouse)
{
   float oldDrawScale = gDrawScale;
   gDrawScale *= 1 + zoomAmount;
   float minZoom = .1f;
   float maxZoom = 8;
   gDrawScale = ofClamp(gDrawScale,minZoom,maxZoom);
   zoomAmount = (gDrawScale - oldDrawScale) / oldDrawScale; //find actual adjusted amount
   ofVec2f zoomCenter;
   if (fromMouse)
      zoomCenter = ofVec2f(GetMouseX(), GetMouseY()) + mDrawOffset;
   else
      zoomCenter = ofVec2f(ofGetWidth() / gDrawScale * .5f, ofGetHeight() / gDrawScale * .5f);
   mDrawOffset -= zoomCenter * zoomAmount;
   mZoomer.CancelMovement();
}

void ModularSynth::PanView(float x, float y)
{
   mDrawOffset += ofVec2f(x, y) / gDrawScale;
}

void ModularSynth::Draw(void* vg)
{
   gNanoVG = (NVGcontext*)vg;
   
   ofNoFill();
   
   //DrawTextNormal("fps: "+ofToString(ofGetFrameRate(),4)+" "+ofToString(ofGetWidth()*ofGetHeight()), 100, 100,50);
   //return;
   
   mDrawRect.set(-mDrawOffset.x, -mDrawOffset.y, ofGetWidth() / gDrawScale, ofGetHeight() / gDrawScale);
   
   if (mFatalError != "")
   {
      ofSetColor(255, 255, 255, 255);
      if (gFont.IsLoaded())
         DrawTextNormal(mFatalError,100,100, 20);
      else
         DrawFallbackText(mFatalError.c_str(), 100, 100);
   }
   
   DrawLissajous(mGlobalRecordBuffer, 0, 0, ofGetWidth(), ofGetHeight(), sBackgroundLissajousR, sBackgroundLissajousG, sBackgroundLissajousB);

   if (ScriptModule::sBackgroundTextString != "")
   {
      ofPushStyle();
      ofSetColor(ScriptModule::sBackgroundTextColor);
      DrawTextBold(ScriptModule::sBackgroundTextString, 150, 200 + ScriptModule::sBackgroundTextSize, ScriptModule::sBackgroundTextSize);
      ofPopStyle();
   }
   
   if (gTime == 1 && mFatalError == "")
   {
      string loading("Bespoke is initializing audio...");
      DrawTextNormal(loading,ofGetWidth()/2-GetStringWidth(loading,30)/2,ofGetHeight()/2-6, 30);
      return;
   }
   
   if (!mInitialized && mFatalError == "")
   {
      string loading("Bespoke is loading...");
      DrawTextNormal(loading,ofGetWidth()/2-GetStringWidth(loading,30)/2,ofGetHeight()/2-6, 30);
      return;
   }
   
   ofPushMatrix();
   
   ofScale(gDrawScale,gDrawScale,gDrawScale);
   
   ofPushMatrix();
   
   ofTranslate(mDrawOffset.x, mDrawOffset.y);

	ofNoFill();
   
   TheTitleBar->SetPosition(-TheSynth->GetDrawOffset().x, -TheSynth->GetDrawOffset().y);
   TheSaveDataPanel->SetShowing(TheSaveDataPanel->GetModule());
   TheSaveDataPanel->UpdatePosition();
   
   mModuleContainer.Draw();
   mModuleContainer.DrawPatchCables();
   mModuleContainer.DrawUnclipped();
   
   for (auto* modal : mModalFocusItemStack)
      modal->Draw();
   
   for (int i=0; i<mLissajousDrawers.size(); ++i)
   {
      float moduleX, moduleY;
      mLissajousDrawers[i]->GetPosition(moduleX, moduleY);
      IAudioSource* source = dynamic_cast<IAudioSource*>(mLissajousDrawers[i]);
      DrawLissajous(source->GetVizBuffer(), moduleX, moduleY-240, 240, 240);
   }
   
   if (mGroupSelectContext != nullptr)
   {
      ofPushStyle();
      ofSetColor(255,255,255);
      ofRect(mClickStartX, mClickStartY, GetMouseX()-mClickStartX, GetMouseY()-mClickStartY);
      ofPopStyle();
   }

   const bool kDrawCursorDot = false;
   if (kDrawCursorDot)
   {
      ofPushStyle();
      ofSetColor(255, 255, 255);
      ofCircle(GetMouseX(), GetMouseY(), 1);
      ofPopStyle();
   }   
   
   /*TODO_PORT(Ryan)
   const int starvationDisplayTicks = 500;
   if (mSoundStream.getTickCount() - mSoundStream.GetLastStarvationTick() < starvationDisplayTicks)
   {
      ofPushStyle();
      ofSetColor(255,255,255, (1.0f-(float(mSoundStream.getTickCount() - mSoundStream.GetLastStarvationTick())/starvationDisplayTicks))*255);
      DrawTextNormal("X", 5, 15);
      ofPopStyle();
   }*/

   if (mHeldSample)
   {
      ofPushMatrix();
      ofTranslate(GetMouseX(), GetMouseY());
      DrawAudioBuffer(100, 70, mHeldSample->Data(), 0, mHeldSample->LengthInSamples(), -1);
      ofPopMatrix();
   }
   
   /*ofPushStyle();
   ofNoFill();
   ofSetLineWidth(3);
   ofSetColor(0,255,0,100);
   ofSetCircleResolution(100);
   ofCircle(GetMouseX(), GetMouseY(), 30 + (TheTransport->GetMeasurePos() * 20));
   ofPopStyle();*/

   if (HelpDisplay::sShowTooltips)
   {
      string tooltip = "";
      HelpDisplay* helpDisplay = TheTitleBar->GetHelpDisplay();

      if (gHoveredUIControl && string(gHoveredUIControl->Name()) != "enabled")
      {
         tooltip = helpDisplay->GetUIControlTooltip(gHoveredUIControl);
      }
      else if (gHoveredModule)
      {
         if (gHoveredModule == mQuickSpawn)
         {
            string name = mQuickSpawn->GetHoveredModuleTypeName();
            ofStringReplace(name, " " + string(ModuleFactory::kEffectChainSuffix), "");   //strip this suffix if it's there
            tooltip = helpDisplay->GetModuleTooltipFromName(name);
         }
         else if (gHoveredModule == GetTopModalFocusItem() && dynamic_cast<DropdownListModal*>(gHoveredModule))
         {
            DropdownListModal* list = dynamic_cast<DropdownListModal*>(gHoveredModule);
            if (list->GetOwner()->GetModuleParent() == TheTitleBar)
            {
               string moduleTypeName = dynamic_cast<DropdownListModal*>(gHoveredModule)->GetHoveredLabel();
               ofStringReplace(moduleTypeName, " (exp.)", "");
               tooltip = helpDisplay->GetModuleTooltipFromName(moduleTypeName);
            }
            else if (dynamic_cast<EffectChain*>(list->GetOwner()->GetParent()) != nullptr)
            {
               string effectName = dynamic_cast<DropdownListModal*>(gHoveredModule)->GetHoveredLabel();
               tooltip = helpDisplay->GetModuleTooltipFromName(effectName);
            }
         }
         else
         {
            tooltip = helpDisplay->GetModuleTooltip(gHoveredModule);
         }
      }

      if (tooltip != "")
      {
         float x = GetMouseX() + 25;
         float y = GetMouseY() + 7;
         float maxWidth = 300;

         float fontSize = 15;
         nvgFontFaceId(gNanoVG, gFont.GetFontHandle());
         nvgFontSize(gNanoVG, fontSize);
         float bounds[4];
         nvgTextBoxBounds(gNanoVG, x, y, maxWidth, tooltip.c_str(), nullptr, bounds);
         float padding = 3;
         ofRectangle rect(bounds[0]-padding, bounds[1] - padding, bounds[2] - bounds[0] + padding*2, bounds[3] - bounds[1] + padding*2);

         ofFill();
         ofSetColor(50, 50, 50);
         ofRect(rect.x, rect.y, rect.width, rect.height);

         ofNoFill();
         ofSetColor(255, 255, 255);
         ofRect(rect.x, rect.y, rect.width, rect.height);

         ofSetColor(255, 255, 255);
         //DrawTextNormal(tooltip, x + 5, y + 12);
         gFont.DrawStringWrap(tooltip, fontSize, x, y, maxWidth);
      }
   }
   
   ofPopMatrix();
   
   Profiler::Draw();
   
   DrawConsole();
   
   if (gTime - mLastClapboardTime < 100)
   {
      ofSetColor(255,255,255,(1 - (gTime - mLastClapboardTime) / 100) * 255);
      ofFill();
      ofRect(0, 0, ofGetWidth(), ofGetHeight());
   }
   
   ofPopMatrix();

   ofPushStyle();
   ofNoFill();
   float centerX = ofGetWidth() * .5f;
   float centerY = ofGetHeight() * .5f;
   if (mSpaceMouseInfo.mTwist != 0)
   {
      if (mSpaceMouseInfo.mUsingTwist)
         ofSetColor(0, 255, 255, 100);
      else
         ofSetColor(0, 100, 255, 100);
      ofSetLineWidth(1.5f);
      ofCircle(centerX + sin(mSpaceMouseInfo.mTwist * M_PI) * 20, centerY + cos(mSpaceMouseInfo.mTwist * M_PI) * 20, 3);
      ofCircle(centerX + sin(mSpaceMouseInfo.mTwist * M_PI + M_PI) * 20, centerY + cos(mSpaceMouseInfo.mTwist * M_PI + M_PI) * 20, 3);
   }
   ofSetLineWidth(3);
   if (mSpaceMouseInfo.mZoom != 0)
   {
      if (mSpaceMouseInfo.mUsingZoom)
         ofSetColor(0, 255, 255, 100);
      else
         ofSetColor(0, 100, 255, 100);
      ofCircle(centerX, centerY, 15 - (mSpaceMouseInfo.mZoom * 15));
   }
   if (mSpaceMouseInfo.mPan.x != 0 || mSpaceMouseInfo.mPan.y != 0)
   {
      if (mSpaceMouseInfo.mUsingPan)
         ofSetColor(0, 255, 255, 100);
      else
         ofSetColor(0, 100, 255, 100);
      ofLine(centerX, centerY, centerX + mSpaceMouseInfo.mPan.x * 40, centerY + mSpaceMouseInfo.mPan.y * 40);
   }
   ofPopStyle();

   ++mFrameCount;
}

void ModularSynth::PostRender()
{
   mModuleContainer.PostRender();
}

void ModularSynth::DrawConsole()
{
   if (!mErrors.empty())
   {
      ofPushStyle();
      ofFill();
      ofSetColor(255,0,0,128);
      ofBeginShape();
      ofVertex(0,0);
      ofVertex(20,0);
      ofVertex(0,20);
      ofEndShape();
      ofPopStyle();
   }
   
   float consoleY = 51;
   
   if (IKeyboardFocusListener::GetActiveKeyboardFocus() == mConsoleEntry)
   {
      mConsoleEntry->SetPosition(0, consoleY-15);
      mConsoleEntry->Draw();
      consoleY += 17;
   }
   else
   {
      if (gHoveredUIControl != nullptr)
      {
         ofPushStyle();
         ofSetColor(0, 255, 255);
         DrawTextNormal(gHoveredUIControl->Path(), 0, consoleY-4);
         ofPopStyle();
      }
   }
   
   if (IKeyboardFocusListener::GetActiveKeyboardFocus() == mConsoleEntry)
   {
      int outputLines = (int)mEvents.size();
      if (IKeyboardFocusListener::GetActiveKeyboardFocus() == mConsoleEntry)
         outputLines += mErrors.size();
      if (outputLines > 0)
      {
         ofPushStyle();
         ofSetColor(0, 0, 0, 150);
         ofFill();
         float titleBarW, titleBarH;
         TheTitleBar->GetDimensions(titleBarW, titleBarH);
         ofRect(0, consoleY - 16, titleBarW, outputLines * 15 + 3);
         ofPopStyle();
      }

      for (auto it = mEvents.begin(); it != mEvents.end(); ++it)
      {
         ofPushStyle();
         if (it->type == kLogEventType_Error)
            ofSetColor(255, 0, 0);
         else if (it->type == kLogEventType_Warning)
            ofSetColor(255, 255, 0);
         else
            ofSetColor(200, 200, 200);
         DrawTextNormal(it->text, 10, consoleY);
         vector<string> lines = ofSplitString(it->text, "\n");
         ofPopStyle();
         consoleY += 15 * lines.size();
      }

      if (!mErrors.empty())
      {
         consoleY = 0;
         ofPushStyle();
         ofSetColor(255,0,0);
         for (auto it = mErrors.begin(); it != mErrors.end(); ++it)
         {
            DrawTextNormal(*it, 600, consoleY);
            vector<string> lines = ofSplitString(*it, "\n");
            consoleY += 15 * lines.size();
         }
         ofPopStyle();
      }
   }
}

void ModularSynth::Exit()
{
   mAudioThreadMutex.Lock("exiting");
   mAudioPaused = true;
   mAudioThreadMutex.Unlock();
   mSoundStream.stop();
   mModuleContainer.Exit();
   DeleteAllModules();
   ofExit();
}

IDrawableModule* ModularSynth::GetLastClickedModule() const
{
   return mLastClickedModule;
}

void ModularSynth::KeyPressed(int key, bool isRepeat)
{
   if (gHoveredUIControl &&
       IKeyboardFocusListener::GetActiveKeyboardFocus() == nullptr &&
       !isRepeat)
   {
      if (key == OF_KEY_DOWN || key == OF_KEY_UP)
      {
         float inc;
         if ((key == OF_KEY_DOWN && gHoveredUIControl->InvertScrollDirection() == false) ||
             (key == OF_KEY_UP   && gHoveredUIControl->InvertScrollDirection() == true))
            inc = -1;
         else
            inc = 1;
         if (GetKeyModifiers() & kModifier_Shift)
            inc *= .01f;
         gHoveredUIControl->Increment(inc);
      }
      else if (key == '[')
      {
         gHoveredUIControl->Halve();
      }
      else if (key == ']')
      {
         gHoveredUIControl->Double();
      }
      else if (key == '\\')
      {
         gHoveredUIControl->ResetToOriginal();
      }
      else if (key != ' ' && key != OF_KEY_TAB && key != '`' && juce::CharacterFunctions::isPrintable((char)key))
      {
         gHoveredUIControl->AttemptTextInput();
      }
   }
   
   if (IKeyboardFocusListener::GetActiveKeyboardFocus())  //active text entry captures all input
   {
      IKeyboardFocusListener::GetActiveKeyboardFocus()->OnKeyPressed(key, isRepeat);
      return;
   }
   
   key = KeyToLower(key);  //now convert to lowercase because everything else just cares about keys as buttons (unmodified by shift)
   
   if (key == OF_KEY_BACKSPACE && !isRepeat)
   {
      for (auto module : mGroupSelectedModules)
         module->GetOwningContainer()->DeleteModule(module);
      mGroupSelectedModules.clear();
   }
   
   if (key == '`' && !isRepeat)
   {
      if (GetKeyModifiers() == kModifier_Shift)
         TriggerClapboard();
      else
         ADSRDisplay::ToggleDisplayMode();
   }

   if (key == OF_KEY_TAB && !isRepeat)
   {
      bzero(mConsoleText, MAX_TEXTENTRY_LENGTH);
      mConsoleEntry->MakeActiveTextEntry(true);
   }
   
   mZoomer.OnKeyPressed(key);
   
   if (CharacterFunctions::isDigit((char)key) && (GetKeyModifiers() == kModifier_Alt))
   {
      int num = key - '0';
      assert(num >= 0 && num <= 9);
      gHotBindUIControl[num] = gHoveredUIControl;
   }
   
   mModuleContainer.KeyPressed(key, isRepeat);

   if (key == '/' && !isRepeat)
      ofToggleFullscreen();
   
   if (key == 'p' && GetKeyModifiers() == kModifier_Shift && !isRepeat)
      mAudioPaused = !mAudioPaused;
   
   //if (key == 'c' && !isRepeat)
   //   mousePressed(GetMouseX(), GetMouseY(), 0);
   
   //if (key == '=' && !isRepeat)
   //   ZoomView(.1f);
   //if (key == '-' && !isRepeat)
   //   ZoomView(-.1f);
}

void ModularSynth::KeyReleased(int key)
{
   key = KeyToLower(key);
   
   //if (key == 'c')
   //   mouseReleased(GetMouseX(), GetMouseY(), 0);
   
   mModuleContainer.KeyReleased(key);
}

float ModularSynth::GetMouseX(float rawX /*= FLT_MAX*/)
{
   return (rawX == FLT_MAX ? mMousePos.x : rawX) / gDrawScale - mDrawOffset.x;
}

float ModularSynth::GetMouseY(float rawY /*= FLT_MAX*/)
{
#if BESPOKE_MAC
   const float kYOffset = -4;
#else
   const float kYOffset = 0;
#endif
   return ((rawY == FLT_MAX ? mMousePos.y : rawY) + kYOffset) / gDrawScale - mDrawOffset.y;
}

bool ModularSynth::IsMouseButtonHeld(int button)
{
   if (button >= 0 && button < (int)mIsMouseButtonHeld.size())
      return mIsMouseButtonHeld[button];

   return false;
}

void ModularSynth::MouseMoved(int intX, int intY )
{
   bool changed = (mMousePos.x != intX || mMousePos.y != intY);

   mMousePos.x = intX;
   mMousePos.y = intY;
   
   if (IsKeyHeld(' ') || mIsMousePanning)
   {
      mDrawOffset += (ofVec2f(intX,intY) - mLastMoveMouseScreenPos) / gDrawScale;
      mZoomer.CancelMovement();
   }
   
   mLastMoveMouseScreenPos = ofVec2f(intX,intY);
   
   float x = GetMouseX();
   float y = GetMouseY();

   if (changed)
   {
      for (auto* modal : mModalFocusItemStack)
         modal->NotifyMouseMoved(x, y);
   }

   if (mMoveModule)
   {
      float oldX, oldY;
      mMoveModule->GetPosition(oldX, oldY);
      mMoveModule->Move(x + mMoveModuleOffsetX - oldX, y + mMoveModuleOffsetY - oldY);
      
      if (GetKeyModifiers() == kModifier_Shift)
      {
         for (auto* module : mModuleContainer.GetModules())
         {
            if (module == mMoveModule)
               continue;
            for (auto* patchCableSource : module->GetPatchCableSources())
            {
               if (patchCableSource->TestHover(x, y))
               {
                  patchCableSource->FindValidTargets();
                  if (!patchCableSource->IsValidTarget(mMoveModule))
                     continue;
                  
                  if (patchCableSource->GetPatchCables().size() == 0)
                  {
                     PatchCableSource::sAllowInsert = false;
                     patchCableSource->SetTarget(mMoveModule);
                     PatchCableSource::sAllowInsert = true;
                     break;
                  }
                  else if (patchCableSource->GetPatchCables().size() == 1 && patchCableSource->GetTarget() != mMoveModule &&
                      mMoveModule->GetPatchCableSource()) //insert
                  {
                     mMoveModule->GetPatchCableSource()->FindValidTargets();
                     if (mMoveModule->GetPatchCableSource()->IsValidTarget(patchCableSource->GetTarget()))
                     {
                        PatchCableSource::sAllowInsert = false;
                        mMoveModule->SetTarget(patchCableSource->GetTarget());
                        patchCableSource->SetTarget(mMoveModule);
                        PatchCableSource::sAllowInsert = true;
                     }
                     break;
                  }
               }
            }
            
            if (mMoveModule->GetPatchCableSource() && mMoveModule->GetPatchCableSource()->GetTarget() == nullptr && module->HasTitleBar())
            {
               ofRectangle titleBarRect(module->GetPosition().x, module->GetPosition().y - IDrawableModule::TitleBarHeight(), module->IClickable::GetDimensions().x, IDrawableModule::TitleBarHeight());
               if (titleBarRect.contains(mMoveModule->GetPatchCableSource()->GetPosition().x, mMoveModule->GetPatchCableSource()->GetPosition().y))
               {
                  mMoveModule->GetPatchCableSource()->FindValidTargets();
                  if (mMoveModule->GetPatchCableSource()->IsValidTarget(module))
                  {
                     PatchCableSource::sAllowInsert = false;
                     mMoveModule->SetTarget(module);
                     PatchCableSource::sAllowInsert = true;
                     break;
                  }
               }
            }
         }
      }
      
      return;
   }

   if (changed)
      mModuleContainer.MouseMoved(x, y);
   
   if (gHoveredUIControl)
   {  
      if (!gHoveredUIControl->IsMouseDown())
      {
         float uiX, uiY;
         gHoveredUIControl->GetPosition(uiX, uiY);
         float w, h;
         gHoveredUIControl->GetDimensions(w, h);
         if (x < uiX - 10 || y < uiY - 10 || x > uiX + w + 10 || y > uiY + h + 10)
            gHoveredUIControl = nullptr;
      }
   }
   
   gHoveredModule = GetModuleAt(GetMouseX(), GetMouseY());
}

void ModularSynth::MouseDragged(int intX, int intY, int button)
{
   mMousePos.x = intX;
   mMousePos.y = intY;
   
   float x = GetMouseX();
   float y = GetMouseY();
   
   ofVec2f drag = ofVec2f(x,y) - mLastMouseDragPos;
   mLastMouseDragPos = ofVec2f(x,y);

   if (GetMoveModule() && (abs(mClickStartX-x) >= 1 || abs(mClickStartY-y) >= 1))
   {
      mClickStartX = INT_MAX;  //moved enough from click spot to reset
      mClickStartY = INT_MAX;
   }
   
   for (auto* modal : mModalFocusItemStack)
      modal->NotifyMouseMoved(x,y);
   
   if (GetKeyModifiers() == kModifier_Alt && !mHasDuplicatedDuringDrag)
   {
      vector<IDrawableModule*> newGroupSelectedModules;
      map<IDrawableModule*, IDrawableModule*> oldToNewModuleMap;
      for (auto module : mGroupSelectedModules)
      {
         if (!module->IsSingleton())
         {
            IDrawableModule* newModule = DuplicateModule(module);
            newGroupSelectedModules.push_back(newModule);
            oldToNewModuleMap[module] = newModule;
         }
      }
      for (auto module : newGroupSelectedModules)
      {
         for (auto* cableSource : module->GetPatchCableSources())
         {
            for (auto* cable : cableSource->GetPatchCables())
            {
               if (VectorContains(dynamic_cast<IDrawableModule*>(cable->GetTarget()), mGroupSelectedModules))
               {
                  cableSource->SetPatchCableTarget(cable, oldToNewModuleMap[dynamic_cast<IDrawableModule*>(cable->GetTarget())], false);
               }
            }
         }
      }
      mGroupSelectedModules = newGroupSelectedModules;
      
      if (mMoveModule && !mMoveModule->IsSingleton())
         mMoveModule = DuplicateModule(mMoveModule);
      
      mHasDuplicatedDuringDrag = true;
   }
   
   for (auto module : mGroupSelectedModules)
      module->Move(drag.x, drag.y);

   if (mMoveModule)
   {
      mMoveModule->Move(drag.x, drag.y);
      return;
   }
   
   if (mResizeModule)
   {
      float moduleX, moduleY;
      mResizeModule->GetPosition(moduleX, moduleY);
      float newWidth = x - moduleX;
      float newHeight = y - moduleY;
      ofVec2f minimumDimensions = mResizeModule->GetMinimumDimensions();
      newWidth = MAX(newWidth, minimumDimensions.x);
      newHeight = MAX(newHeight, minimumDimensions.y);
      mResizeModule->Resize(newWidth, newHeight);
   }

   mModuleContainer.MouseMoved(x, y);
}

void ModularSynth::MousePressed(int intX, int intY, int button)
{
   if (PatchCable::sActivePatchCable != nullptr)
      return;
   
   mMousePos.x = intX;
   mMousePos.y = intY;

   if (button >= 0 && button < (int)mIsMouseButtonHeld.size())
      mIsMouseButtonHeld[button] = true;
   
   float x = GetMouseX();
   float y = GetMouseY();
   
   mLastMouseDragPos = ofVec2f(x,y);
   mGroupSelectContext = nullptr;
   
   bool rightButton = button == 2;

   IKeyboardFocusListener::ClearActiveKeyboardFocus(K(notifyListeners));

   if (GetTopModalFocusItem())
   {
      bool clicked = GetTopModalFocusItem()->TestClick(x,y,rightButton);
      if (!clicked)
      {
         FloatSliderLFOControl* lfo = dynamic_cast<FloatSliderLFOControl*>(GetTopModalFocusItem());
         if (lfo) //if it's an LFO, don't dismiss it if you're adjusting the slider
         {
            FloatSlider* slider = lfo->GetOwner();
            float uiX,uiY;
            slider->GetPosition(uiX, uiY);
            float w, h;
            slider->GetDimensions(w, h);
            
            if (x < uiX || y < uiY || x > uiX + w || y > uiY + h)
               PopModalFocusItem();
         }
         else  //otherwise, always dismiss if you click outside it
         {
            PopModalFocusItem();
         }
      }
      else
      {
         return;
      }
   }

   if (InMidiMapMode())
   {
      if (gBindToUIControl == gHoveredUIControl)   //if it's the same, clear it
         gBindToUIControl = nullptr;
      else
         gBindToUIControl = gHoveredUIControl;
      return;
   }
   
   IDrawableModule* clicked = GetModuleAt(x,y);
   
   for (auto cable : mPatchCables)
   {
      if (clicked &&
          (clicked == GetTopModalFocusItem() ||
           clicked->AlwaysOnTop() ||
           mModuleContainer.IsHigherThan(clicked, cable->GetOwningModule())))
         break;
      if (cable->TestClick(x,y,rightButton))
         return;
   }
   
   mClickStartX = x;
   mClickStartY = y;
   if (clicked == nullptr)
   {
      if (rightButton)
         mIsMousePanning = true;
      else
         SetGroupSelectContext(&mModuleContainer);
   }
   if (clicked != nullptr && clicked != TheTitleBar)
      mLastClickedModule = clicked;
   else
      mLastClickedModule = nullptr;
   mHasDuplicatedDuringDrag = false;
   
   if (mGroupSelectedModules.empty() == false)
   {
      if (!VectorContains(clicked, mGroupSelectedModules))
         mGroupSelectedModules.clear();
      return;
   }

   if (clicked)
      CheckClick(clicked, x, y, rightButton);
   else if (TheSaveDataPanel != nullptr)
      TheSaveDataPanel->SetModule(nullptr);
}

void ModularSynth::MouseScrolled(float x, float y, bool canZoomCanvas)
{
   x *= mScrollMultiplierHorizontal;
   y *= mScrollMultiplierVertical;

   if (IsKeyHeld(' ') || GetModuleAt(GetMouseX(), GetMouseY()) == nullptr)
   {
      if (canZoomCanvas)
         ZoomView(y/50, true);
   }
   else if (gHoveredUIControl)
   {
#if JUCE_WINDOWS
      y += x / 4; //taking advantage of logitech horizontal scroll wheel
#endif

      float val = gHoveredUIControl->GetMidiValue();
      float movementScale = 3;
      FloatSlider* floatSlider = dynamic_cast<FloatSlider*>(gHoveredUIControl);
      IntSlider* intSlider = dynamic_cast<IntSlider*>(gHoveredUIControl);
      if (floatSlider || intSlider)
      {
         float w,h;
         gHoveredUIControl->GetDimensions(w, h);
         movementScale = 200.0f / w;
            
         if (GetKeyModifiers() & kModifier_Shift)
            movementScale *= .01f;
      }
         
      float change = y/100 * movementScale;
         
      if (floatSlider && floatSlider->GetModulator() && floatSlider->GetModulator()->Active() && floatSlider->GetModulator()->CanAdjustRange())
      {
         IModulator* modulator = floatSlider->GetModulator();
         float min = floatSlider->GetMin();
         float max = floatSlider->GetMax();
         float modMin = ofMap(modulator->GetMin(),min,max,0,1);
         float modMax = ofMap(modulator->GetMax(),min,max,0,1);
            
         modulator->GetMin() = ofMap(modMin - change,0,1,min,max,K(clamp));
         modulator->GetMax() = ofMap(modMax + change,0,1,min,max,K(clamp));
            
         return;
      }
         
      if (gHoveredUIControl->InvertScrollDirection())
         val -= change;
      else
         val += change;
      val = ofClamp(val, 0, 1);
      gHoveredUIControl->SetFromMidiCC(val);

      gHoveredUIControl->NotifyMouseScrolled(GetMouseX(), GetMouseY(), x, y);
   }
   else
   {
      IDrawableModule* module = GetModuleAt(GetMouseX(), GetMouseY());
      if (module)
         module->NotifyMouseScrolled(GetMouseX(), GetMouseY(), x, y);
   }
}

void ModularSynth::MouseMagnify(int intX, int intY, float scaleFactor)
{
   mMousePos.x = intX;
   mMousePos.y = intY;
   ZoomView(scaleFactor - 1, true);
}

bool ModularSynth::InMidiMapMode()
{
   return IsKeyHeld('m', kModifier_Shift);
}

bool ModularSynth::ShouldAccentuateActiveModules() const
{
   return IsKeyHeld('s', kModifier_Shift);
}

void ModularSynth::RegisterPatchCable(PatchCable* cable)
{
   mPatchCables.push_back(cable);
}

void ModularSynth::UnregisterPatchCable(PatchCable* cable)
{
   RemoveFromVector(cable, mPatchCables);
}

void ModularSynth::PushModalFocusItem(IDrawableModule* item)
{
   mModalFocusItemStack.push_back(item);
}

void ModularSynth::PopModalFocusItem()
{
   if (mModalFocusItemStack.empty() == false)
      mModalFocusItemStack.pop_back();
}

IDrawableModule* ModularSynth::GetTopModalFocusItem() const
{
   if (mModalFocusItemStack.empty())
      return nullptr;
   return mModalFocusItemStack.back();
}

bool ModularSynth::IsModalFocusItem(IDrawableModule* item) const
{
   return std::find(mModalFocusItemStack.begin(), mModalFocusItemStack.end(), item) == mModalFocusItemStack.end();
}

IDrawableModule* ModularSynth::GetModuleAt(int x, int y)
{
   if (GetTopModalFocusItem() && GetTopModalFocusItem()->TestClick(x, y, false, true))
      return GetTopModalFocusItem();
   return mModuleContainer.GetModuleAt(x, y);
}

void ModularSynth::CheckClick(IDrawableModule* clickedModule, int x, int y, bool rightButton)
{
   if (clickedModule != TheTitleBar)
      MoveToFront(clickedModule);
   
   //check to see if we clicked in the move area
   float moduleX, moduleY;
   clickedModule->GetPosition(moduleX, moduleY);
   int modulePosY = y - moduleY;
   
   if (modulePosY < 0)
   {
      mMoveModule = clickedModule;
      mMoveModuleOffsetX = moduleX - x;
      mMoveModuleOffsetY = moduleY - y;
   }
   
   float parentX = 0;
   float parentY = 0;
   if (clickedModule->GetParent())
      clickedModule->GetParent()->GetPosition(parentX, parentY);
   
   //do the regular click
   clickedModule->TestClick(x - parentX,y - parentY,rightButton);
}

void ModularSynth::MoveToFront(IDrawableModule* module)
{
   if (module->GetOwningContainer())
      module->GetOwningContainer()->MoveToFront(module);
}

void ModularSynth::OnModuleDeleted(IDrawableModule* module)
{
   if (module->IsSingleton())
      return;
   
   mDeletedModules.push_back(module);
   
   mAudioThreadMutex.Lock("delete");
   
   list<PatchCable*> cablesToRemove;
   for (auto* cable : mPatchCables)
   {
      if (cable->GetOwningModule() == module)
         cablesToRemove.push_back(cable);
   }
   for (auto* cable : cablesToRemove)
      RemoveFromVector(cable, mPatchCables);
   
   RemoveFromVector(dynamic_cast<IAudioSource*>(module),mSources);
   RemoveFromVector(module,mLissajousDrawers);
   TheTransport->RemoveAudioPoller(dynamic_cast<IAudioPoller*>(module));
   //delete module; TODO(Ryan) deleting is hard... need to clear out everything with a reference to this, or switch to smart pointers
   
   if (module == TheChaosEngine)
      TheChaosEngine = nullptr;
   if (module == TheLFOController)
      TheLFOController = nullptr;
   
   mAudioThreadMutex.Unlock();
}

void ModularSynth::MouseReleased(int intX, int intY, int button)
{
   mMousePos.x = intX;
   mMousePos.y = intY;

   if (button >= 0 && button < (int)mIsMouseButtonHeld.size())
      mIsMouseButtonHeld[button] = false;
   
   float x = GetMouseX();
   float y = GetMouseY();
   
   if (GetTopModalFocusItem())
   {
      GetTopModalFocusItem()->MouseReleased();
   }

   if (mMoveModule)
   {
      float moduleX, moduleY;
      mMoveModule->GetPosition(moduleX, moduleY);
      mMoveModule = nullptr;
   }
   
   if (mResizeModule)
      mResizeModule = nullptr;

   mModuleContainer.MouseReleased();
   
   if (mHeldSample)
   {
      IDrawableModule* module = GetModuleAt(x, y);
      if (module)
      {
         float moduleX, moduleY;
         module->GetPosition(moduleX, moduleY);
         module->SampleDropped(x-moduleX, y-moduleY, GetHeldSample());
      }
      ClearHeldSample();
   }
   
   if (mGroupSelectContext != nullptr)
   {
      ofRectangle rect = ofRectangle(ofPoint(MIN(mClickStartX, x), MIN(mClickStartY, y)), ofPoint(MAX(mClickStartX, x), MAX(mClickStartY, y)));
      if (rect.width > 10 || rect.height > 10)
      {
         mGroupSelectContext->GetModulesWithinRect(rect, mGroupSelectedModules);
         if (mGroupSelectedModules.size() > 0)
         {
            for (int i = (int)mGroupSelectedModules.size() - 1; i >= 0; --i) //do this backwards to preserve existing order
               MoveToFront(mGroupSelectedModules[i]);
         }
      }
      else
      {
         mGroupSelectedModules.clear();
      }
      mGroupSelectContext = nullptr;
   }
   
   mClickStartX = INT_MAX;
   mClickStartY = INT_MAX;

   mIsMousePanning = false;
}

void ModularSynth::AudioOut(float** output, int bufferSize, int nChannels)
{
   PROFILER(audioOut_total);
   
   static bool sFirst = true;
   if (sFirst)
   {
      FloatVectorOperations::disableDenormalisedNumberSupport();
      sFirst = false;
   }
   
   if (mAudioPaused)
   {
      for (int ch=0; ch<nChannels; ++ch)
      {
         for (int i=0; i<bufferSize; ++i)
            output[ch][i] = 0;
      }
      return;
   }
   
   ScopedMutex mutex(&mAudioThreadMutex, "audioOut()");
   
   /////////// AUDIO PROCESSING STARTS HERE /////////////
   assert(bufferSize == mIOBufferSize);
   assert(nChannels == (int)mOutputBuffers.size());
   assert(mIOBufferSize == gBufferSize);  //need to be the same for now
                                          //if we want these different, need to fix outBuffer here, and also fix audioIn()
                                          //by now, many assumptions will have to be fixed to support mIOBufferSize and gBufferSize diverging
   for (int ioOffset = 0; ioOffset < mIOBufferSize; ioOffset += gBufferSize)
   {
      for (size_t i = 0; i < mOutputBuffers.size(); ++i)
         Clear(mOutputBuffers[i], mIOBufferSize);

      double elapsed = gInvSampleRateMs * mIOBufferSize;
      gTime += elapsed;
      TheTransport->Advance(elapsed);
      
      //process all audio
      for (int i=0; i<mSources.size(); ++i)
         mSources[i]->Process(gTime);

      //put it into speakers
      for (int i = 0; i < nChannels; ++i)
         BufferCopy(output[i], mOutputBuffers[i], gBufferSize);
      
      if (TheMultitrackRecorder && mOutputBuffers.size() >= 2)
         TheMultitrackRecorder->Process(gTime, mOutputBuffers[0], mOutputBuffers[1], gBufferSize);
   }
   
   if (gTime - mLastClapboardTime < 100)
   {
      for (int ch=0; ch<nChannels; ++ch)
      {
         for (int i=0; i<bufferSize; ++i)
         {
            float sample = sin(GetPhaseInc(440) * i) * (1 - ((gTime - mLastClapboardTime) / 100));
            output[ch][i] = sample;
         }
      }
   }
   /////////// AUDIO PROCESSING ENDS HERE /////////////
   if (nChannels >= 1)
      mGlobalRecordBuffer->WriteChunk(output[0], bufferSize, 0);
   if (nChannels >= 2)
      mGlobalRecordBuffer->WriteChunk(output[1], bufferSize, 1);
   mRecordingLength += bufferSize;
   mRecordingLength = MIN(mRecordingLength, mGlobalRecordBuffer->Size());
   
   Profiler::PrintCounters();
}

void ModularSynth::AudioIn(const float** input, int bufferSize, int nChannels)
{
   if (mAudioPaused)
      return;
   
   ScopedMutex mutex(&mAudioThreadMutex, "audioIn()");

   assert(bufferSize == mIOBufferSize);
   assert(nChannels == (int)mInputBuffers.size());
   
   for (int i=0; i<nChannels; ++i)
   {
      BufferCopy(mInputBuffers[i], input[i], bufferSize);
   }
}

float* ModularSynth::GetInputBuffer(int channel)
{
   return mInputBuffers[channel];
}

float* ModularSynth::GetOutputBuffer(int channel)
{
   return mOutputBuffers[channel];
}

void ModularSynth::TriggerClapboard()
{
   mLastClapboardTime = gTime; //for synchronizing internally recorded audio and externally recorded video
}

void ModularSynth::FilesDropped(vector<string> files, int intX, int intY)
{
   if (files.size() > 0)
   {
      float x = GetMouseX(intX);
      float y = GetMouseY(intY);
      IDrawableModule* target = mModuleContainer.GetModuleAt(x, y);

      if (target != nullptr)
      {
         float moduleX, moduleY;
         target->GetPosition(moduleX, moduleY);
         x -= moduleX;
         y -= moduleY;
         target->FilesDropped(files, x, y);
      }
   }
}

struct SourceDepInfo
{
   SourceDepInfo(IAudioSource* me) : mMe(me) {}
   IAudioSource* mMe;
   vector<IAudioSource*> mDeps;
};

void ModularSynth::ArrangeAudioSourceDependencies()
{
   //ofLog() << "Calculating audio source dependencies:";
   
   vector<SourceDepInfo> deps;
   for (int i=0; i<mSources.size(); ++i)
      deps.push_back(SourceDepInfo(mSources[i]));
      
   for (int i=0; i<mSources.size(); ++i)
   {
      for (int j=0; j<mSources.size(); ++j)
      {
         for (int k=0; k<mSources[i]->GetNumTargets(); ++k)
         {
            if (mSources[i]->GetTarget(k) != nullptr &&
                mSources[i]->GetTarget(k) == dynamic_cast<IAudioReceiver*>(mSources[j]))
            {
               deps[j].mDeps.push_back(mSources[i]);
            }
         }
      }
   }
   
   /*for (int i=0; i<deps.size(); ++i)
   {
      string depStr;
      for (int j=0;j<deps[i].mDeps.size();++j)
      {
         depStr += dynamic_cast<IDrawableModule*>(deps[i].mDeps[j])->Name();
         if (j<deps[i].mDeps.size()-1)
            depStr += ", ";
      }
      ofLog() << dynamic_cast<IDrawableModule*>(deps[i].mMe)->Name() << "depends on:" << depStr;
   }*/
   
   //TODO(Ryan) detect circular dependencies
   
   mSources.clear();
   int loopCount = 0;
   while (deps.size() > 0 && loopCount < 1000) //stupid circular dependency detection, make better
   {
      for (int i=0; i<deps.size(); ++i)
      {
         bool hasDeps = false;
         for (int j=0; j<deps[i].mDeps.size(); ++j)
         {
            bool found = false;
            for (int k=0; k<mSources.size(); ++k)
            {
               if (deps[i].mDeps[j] == mSources[k])
                  found = true;
            }
            if (!found) //has a dep that hasn't been added yet
               hasDeps = true;
         }
         if (!hasDeps)
         {
            mSources.push_back(deps[i].mMe);
            deps.erase(deps.begin() + i);
            i-=1;
         }
      }
      ++loopCount;
   }
   
   if (loopCount == 1000)  //circular dependency, don't lose the rest of the sources
   {
      ofLog() << "circular dependency detected";
      for (int i=0; i<deps.size(); ++i)
         mSources.push_back(deps[i].mMe);
   }
   
   /*ofLog() << "new ordering:";
   for (int i=0; i<mSources.size(); ++i)
      ofLog() << dynamic_cast<IDrawableModule*>(mSources[i])->Name();*/
}

void ModularSynth::ResetLayout()
{
   mModuleContainer.Clear();
   
   for (int i=0; i<mDeletedModules.size(); ++i)
      delete mDeletedModules[i];

   mDeletedModules.clear();
   mSources.clear();
   mLissajousDrawers.clear();
   mMoveModule = nullptr;
   LFOPool::Shutdown();
   IKeyboardFocusListener::ClearActiveKeyboardFocus(!K(notifyListeners));
   
   mErrors.clear();
   
   vector<PatchCable*> cablesToDelete = mPatchCables;
   for (auto cable : cablesToDelete)
      delete cable;
   assert(mPatchCables.size() == 0); //everything should have been cleared out by that
   
   gBindToUIControl = nullptr;
   mModalFocusItemStack.clear();
   gHoveredUIControl = nullptr;
   mLastClickedModule = nullptr;

   LFOPool::Init();
   
   delete TheTitleBar;
   delete TheSaveDataPanel;
   delete mQuickSpawn;
   
   TitleBar* titleBar = new TitleBar();
   titleBar->SetPosition(0,0);
   titleBar->SetName("titlebar");
   titleBar->SetTypeName("titlebar");
   titleBar->CreateUIControls();
   titleBar->SetModuleFactory(&mModuleFactory);
   titleBar->Init();
   mModuleContainer.AddModule(titleBar);
   
   ModuleSaveDataPanel* saveDataPanel = new ModuleSaveDataPanel();
   saveDataPanel->SetPosition(-200, 50);
   saveDataPanel->SetName("savepanel");
   saveDataPanel->CreateUIControls();
   saveDataPanel->Init();
   mModuleContainer.AddModule(saveDataPanel);
   
   mQuickSpawn = new QuickSpawnMenu();
   mQuickSpawn->SetName("quickspawn");
   mQuickSpawn->CreateUIControls();
   mQuickSpawn->Init();
   mModuleContainer.AddModule(mQuickSpawn);

   mUserPrefsEditor = new UserPrefsEditor();
   mUserPrefsEditor->SetName("userprefseditor");
   mUserPrefsEditor->CreateUIControls();
   mUserPrefsEditor->Init();
   mUserPrefsEditor->SetPosition(300, 300);
   mUserPrefsEditor->SetShowing(false);
   mModuleContainer.AddModule(mUserPrefsEditor);
   if (mFatalError != "")
   {
      mUserPrefsEditor->Show();
      TheTitleBar->SetShowing(false);
   }
   
   mDrawOffset.set(0,0);
   mZoomer.Init();
}

bool ModularSynth::LoadLayoutFromFile(string jsonFile, bool makeDefaultLayout /*= true*/)
{
   ofLog() << "Loading layout: " << jsonFile;
   mLoadedLayoutPath = String(jsonFile).replace(ofToDataPath("").c_str(), "").toStdString();
   
   ofxJSONElement root;
   bool loaded = root.open(jsonFile);
   
   if (!loaded)
   {
      LogEvent("Couldn't load, error parsing "+jsonFile, kLogEventType_Error);
      LogEvent("Try loading it up in a json validator", kLogEventType_Error);
      return false;
   }
   
   LoadLayout(root);
   
   if (juce::String(jsonFile).endsWith("blank.json"))
   {
      IDrawableModule* gain = FindModule("gain");
      IDrawableModule* splitter = FindModule("splitter");
      IDrawableModule* output1 = FindModule("output 1");
      IDrawableModule* output2 = FindModule("output 2");
      if (output1 != nullptr && output1->GetPosition().y > ofGetHeight() - 20)
      {
         float offset = ofGetHeight() - output1->GetPosition().y - 20;
         if (gain != nullptr)
            gain->SetPosition(gain->GetPosition().x, gain->GetPosition().y + offset);
         if (splitter != nullptr)
            splitter->SetPosition(splitter->GetPosition().x, splitter->GetPosition().y + offset);
         if (output1 != nullptr)
            output1->SetPosition(output1->GetPosition().x, output1->GetPosition().y + offset);
         if (output2 != nullptr)
            output2->SetPosition(output2->GetPosition().x, output2->GetPosition().y + offset);
      }
   }
   
   if (makeDefaultLayout)
      UpdateUserPrefsLayout();
   
   return true;
}

bool ModularSynth::LoadLayoutFromString(string jsonString)
{
   ofxJSONElement root;
   bool loaded = root.parse(jsonString);
   
   if (!loaded)
   {
      LogEvent("Couldn't load, error parsing json string", kLogEventType_Error);
      ofLog() << jsonString;
      return false;
   }
   
   LoadLayout(root);
   return true;
}

void ModularSynth::LoadLayout(ofxJSONElement json)
{
   ScriptModule::UninitializePython();
   Transport::sDoEventLookahead = false;
   
   //ofLoadURLAsync("http://bespoke.com/telemetry/"+jsonFile);
   
   ScopedMutex mutex(&mAudioThreadMutex, "LoadLayout()");
   ScopedLock renderLock(mRenderLock);
   
   ResetLayout();
   
   mModuleContainer.LoadModules(json["modules"]);
   
   //timer.PrintCosts();
   
   mZoomer.LoadFromSaveData(json["zoomlocations"]);
   ArrangeAudioSourceDependencies();
}

void ModularSynth::UpdateUserPrefsLayout()
{
   //mUserPrefs["layout"] = mLoadedLayoutPath;
   //mUserPrefs.save(GetUserPrefsPath(), true);
}

void ModularSynth::AddExtraPoller(IPollable* poller)
{
   if (!ListContains(poller, mExtraPollers))
      mExtraPollers.push_front(poller);
}

void ModularSynth::RemoveExtraPoller(IPollable* poller)
{
   if (ListContains(poller, mExtraPollers))
      mExtraPollers.remove(poller);
}

IDrawableModule* ModularSynth::CreateModule(const ofxJSONElement& moduleInfo)
{
   IDrawableModule* module = nullptr;

   if (moduleInfo["comment_out"].asBool()) //hack since json doesn't allow comments
      return nullptr;

   string type = moduleInfo["type"].asString();
   type = ModuleFactory::FixUpTypeName(type);
   
   try
   {
      if (type == "transport")
         module = TheTransport;
      else if (type == "scale")
         module = TheScale;
      else
         module = mModuleFactory.MakeModule(type);
   
      if (module == nullptr)
      {
         LogEvent("Couldn't create unknown module type \""+type+"\"", kLogEventType_Error);
         return nullptr;
      }
      
      if (module->IsSingleton() == false)
         module->CreateUIControls();
      module->LoadBasics(moduleInfo, type);
      assert(strlen(module->Name()) > 0);
   }
   catch (UnknownModuleException& e)
   {
      LogEvent("Couldn't find referenced module \""+e.mSearchName+"\" when loading \""+moduleInfo["name"].asString()+"\"", kLogEventType_Error);
   }
   
   return module;
}

void ModularSynth::SetUpModule(IDrawableModule* module, const ofxJSONElement& moduleInfo)
{
   assert(module != nullptr);

   try
   {
      mIsLoadingModule = true;
      module->LoadLayout(moduleInfo);
      mIsLoadingModule = false;
   }
   catch (UnknownModuleException& e)
   {
      LogEvent("Couldn't find referenced module \""+e.mSearchName+"\" when setting up \""+moduleInfo["name"].asString()+"\"", kLogEventType_Error);
   }
}

void ModularSynth::OnModuleAdded(IDrawableModule* module)
{
   IAudioSource* source = dynamic_cast<IAudioSource*>(module);
   if (source)
      mSources.push_back(source);
}

void ModularSynth::AddDynamicModule(IDrawableModule* module)
{
   mModuleContainer.AddModule(module);
}

void ModularSynth::ScheduleEnvelopeEditorSpawn(ADSRDisplay* adsrDisplay)
{
   mScheduledEnvelopeEditorSpawnDisplay = adsrDisplay;
}

IDrawableModule* ModularSynth::FindModule(string name, bool fail)
{
   if (name[0] == '$')
      return mModuleContainer.FindModule(name.substr(1,name.length()-1), fail);
   return mModuleContainer.FindModule(IClickable::sLoadContext+name, fail);
}

MidiController* ModularSynth::FindMidiController(string name, bool fail)
{
   if (name == "")
      return nullptr;
   
   MidiController* m = nullptr;
   
   try
   {
      m = dynamic_cast<MidiController*>(FindModule(name, fail));
      if (m == nullptr && fail)
         throw WrongModuleTypeException();
   }
   catch (UnknownModuleException& e)
   {
      LogEvent("Couldn't find referenced midi controller \""+name+"\"", kLogEventType_Error);
   }
   catch (WrongModuleTypeException& e)
   {
      LogEvent("\""+name+"\" is not a midicontroller", kLogEventType_Error);
   }
   
   return m;
}

IUIControl* ModularSynth::FindUIControl(string path)
{
   return mModuleContainer.FindUIControl(path);
}

void ModularSynth::GrabSample(ChannelBuffer* data, bool window, int numBars)
{
   delete mHeldSample;
   mHeldSample = new Sample();
   mHeldSample->Create(data);
   mHeldSample->SetNumBars(numBars);
   
   //window sample to avoid clicks
   if (window)
   {
      int length = data->BufferSize();
      const int fadeSamples = 15;
      if (length > fadeSamples * 2) //only window if there's enough space
      {
         for (int i=0; i<fadeSamples; ++i)
         {
            for (int ch=0; ch<mHeldSample->NumChannels(); ++ch)
            {
               float fade = float(i)/fadeSamples;
               mHeldSample->Data()->GetChannel(ch)[i] *= fade;
               mHeldSample->Data()->GetChannel(ch)[length-1-i] *= fade;
            }
         }
      }
   }
}

void ModularSynth::ClearHeldSample()
{
   delete mHeldSample;
   mHeldSample = nullptr;
}

void ModularSynth::LogEvent(string event, LogEventType type)
{
   if (type == kLogEventType_Warning)
   {
      !ofLog() << "warning: " << event;
   }
   else if (type == kLogEventType_Error)
   {
      !ofLog() << "error: " << event;
      mErrors.push_back(event);
   }

   mEvents.push_back(LogEventItem(gTime, event, type));
   if (mEvents.size() > 30)
      mEvents.pop_front();
}

IDrawableModule* ModularSynth::DuplicateModule(IDrawableModule* module)
{
   {
      FileStreamOut out(ofToDataPath("tmp").c_str());
      module->SaveState(out);
   }
   
   ofxJSONElement layoutData;
   module->SaveLayout(layoutData);
   vector<IDrawableModule*> allModules;
   mModuleContainer.GetAllModules(allModules);
   string newName = GetUniqueName(layoutData["name"].asString(), allModules);
   layoutData["name"] = newName;
   
   IDrawableModule* newModule = CreateModule(layoutData);
   mModuleContainer.AddModule(newModule);
   SetUpModule(newModule, layoutData);
   newModule->Init();
   
   assert(newModule);
   
   newModule->SetName(module->Name()); //temporarily rename to the same as what we duplicated, so we can load state properly
   
   {
      FileStreamIn in(ofToDataPath("tmp").c_str());
      mIsLoadingModule = true;
      newModule->LoadState(in);
      mIsLoadingModule = false;
   }
   
   newModule->SetName(newName.c_str());
   
   return newModule;
}

ofxJSONElement ModularSynth::GetLayout()
{
   ofxJSONElement root;
   
   root["modules"] = mModuleContainer.WriteModules();
   root["zoomlocations"] = mZoomer.GetSaveData();
   
   return root;
}

void ModularSynth::SaveLayout(string jsonFile, bool makeDefaultLayout /*= true*/)
{
   if (jsonFile.empty())
      jsonFile = ofToDataPath(mLoadedLayoutPath);
   
   ofxJSONElement root = GetLayout();
   root.save(jsonFile, true);
   
   mLoadedLayoutPath = String(jsonFile).replace(ofToDataPath("").c_str(), "").toStdString();
   
   TheTitleBar->ListLayouts();
   if (makeDefaultLayout)
      UpdateUserPrefsLayout();
}

void ModularSynth::SaveLayoutAsPopup()
{
   FileChooser chooser("Save current layout as...", File(ofToDataPath("layouts/newlayout.json")), "*.json", true, false, mMainComponent->getTopLevelComponent());
   if (chooser.browseForFileToSave(true))
      SaveLayout(chooser.getResult().getRelativePathFrom(File(ofToDataPath(""))).toStdString());
}

void ModularSynth::SaveStatePopup()
{
   FileChooser chooser("Save current state as...", File(ofToDataPath(ofGetTimestampString("savestate/%Y-%m-%d_%H-%M.bsk"))), "*.bsk", true, false, mMainComponent->getTopLevelComponent());
   if (chooser.browseForFileToSave(true))
      SaveState(chooser.getResult().getRelativePathFrom(File(ofToDataPath(""))).toStdString());
}

void ModularSynth::LoadStatePopup()
{
   mShowLoadStatePopup = true;
}

void ModularSynth::LoadStatePopupImp()
{
   FileChooser chooser("Load state", File(ofToDataPath("savestate")), "*.bsk", true, false, mMainComponent->getTopLevelComponent());
   if (chooser.browseForFileToOpen())
      LoadState(chooser.getResult().getRelativePathFrom(File(ofToDataPath(""))).toStdString());
}

void ModularSynth::SaveState(string file)
{
   mAudioThreadMutex.Lock("SaveState()");
   
   FileStreamOut out(ofToDataPath(file).c_str());
   
   out << GetLayout().getRawString(true);
   mModuleContainer.SaveState(out);
   
   mAudioThreadMutex.Unlock();
}

void ModularSynth::LoadState(string file)
{
   ofLog() << "LoadState() " << ofToDataPath(file);

   if (!juce::File(ofToDataPath(file)).existsAsFile())
   {
      LogEvent("couldn't find file " + ofToDataPath(file), kLogEventType_Error);
      return;
   }

   if (mInitialized)
      TitleBar::sShowInitialHelpOverlay = false;  //don't show initial help popup
   
   mAudioThreadMutex.Lock("LoadState()");
   LockRender(true);
   mAudioPaused = true;
   mIsLoadingState = true;
   LockRender(false);
   mAudioThreadMutex.Unlock();
   
   FileStreamIn in(ofToDataPath(file).c_str());
   
   string jsonString;
   in >> jsonString;
   bool layoutLoaded = LoadLayoutFromString(jsonString);
   
   if (layoutLoaded)
   {
      mIsLoadingModule = true;
      mModuleContainer.LoadState(in);
      mIsLoadingModule = false;
      
      TheTransport->Reset();
   }
   
   
   mAudioThreadMutex.Lock("LoadState()");
   LockRender(true);
   mAudioPaused = false;
   mIsLoadingState = false;
   LockRender(false);
   mAudioThreadMutex.Unlock();
}

IAudioReceiver* ModularSynth::FindAudioReceiver(string name, bool fail)
{
   IAudioReceiver* a = nullptr;
   
   if (name == "")
      return nullptr;
   
   try
   {
      a = dynamic_cast<IAudioReceiver*>(FindModule(name,fail));
      if (a == nullptr)
         throw WrongModuleTypeException();
   }
   catch (UnknownModuleException& e)
   {
      LogEvent("Couldn't find referenced audio receiver \""+name+"\"", kLogEventType_Error);
   }
   catch (WrongModuleTypeException& e)
   {
      LogEvent("\""+name+"\" is not an audio receiver", kLogEventType_Error);
   }
   
   return a;
}

INoteReceiver* ModularSynth::FindNoteReceiver(string name, bool fail)
{
   INoteReceiver* n = nullptr;
   
   if (name == "")
      return nullptr;
   
   try
   {
      n = dynamic_cast<INoteReceiver*>(FindModule(name,fail));
      if (n == nullptr)
         throw WrongModuleTypeException();
   }
   catch (UnknownModuleException& e)
   {
      LogEvent("Couldn't find referenced note receiver \""+name+"\"", kLogEventType_Error);
   }
   catch (WrongModuleTypeException& e)
   {
      LogEvent("\""+name+"\" is not a note receiver", kLogEventType_Error);
   }
      
   return n;
}

void ModularSynth::OnConsoleInput()
{
   vector<string> tokens = ofSplitString(mConsoleText," ",true,true);
   if (tokens.size() > 0)
   {
      if (tokens[0] == "")
      {
      }
      else if (tokens[0] == "clearerrors")
      {
         mErrors.clear();
      }
      else if (tokens[0] == "clearall")
      {
         mAudioThreadMutex.Lock("clearall");
         ScopedLock renderLock(mRenderLock);
         ResetLayout();
         mAudioThreadMutex.Unlock();
      }
      else if (tokens[0] == "load")
      {
         LoadLayout(ofToDataPath(tokens[1]));
      }
      else if (tokens[0] == "save")
      {
         if (tokens.size() > 1)
            SaveLayout(ofToDataPath(tokens[1]));
         else
            SaveLayout();
      }
      else if (tokens[0] == "write")
      {
         SaveOutput();
      }
      else if (tokens[0] == "reconnect")
      {
         ReconnectMidiDevices();
      }
      else if (tokens[0] == "profiler")
      {
         Profiler::ToggleProfiler();
      }
      else if (tokens[0] == "clear")
      {
         mErrors.clear();
         mEvents.clear();
      }
      else if (tokens[0] == "minimizeall")
      {
         const vector<IDrawableModule*> modules = mModuleContainer.GetModules();
         for (auto iter = modules.begin(); iter != modules.end(); ++iter)
         {
            (*iter)->SetMinimized(true);
         }
      }
      else if (tokens[0] == "resettime")
      {
         gTime = 0;
      }
      else if (tokens[0] == "hightime")
      {
         gTime += 1000000;
      }
      else if (tokens[0] == "tempo")
      {
         if (tokens.size() >= 2)
         {
            float tempo = atof(tokens[1].c_str());
            if (tempo > 0)
               TheTransport->SetTempo(tempo);
         }
      }
      else if (tokens[0] == "home")
      {
         mZoomer.GoHome();
      }
      else if (tokens[0] == "saveas")
      {
         SaveLayoutAsPopup();
      }
      else if (tokens[0] == "dev")
      {
         gShowDevModules = true;
         TheTitleBar->SetModuleFactory(&mModuleFactory);
      }
      else if (tokens[0] == "dumpmem")
      {
         DumpUnfreedMemory();
      }
      else if (tokens[0] == "savestate")
      {
         if (tokens.size() >= 2)
            SaveState("savestate/"+tokens[1]);
      }
      else if (tokens[0] == "loadstate")
      {
         if (tokens.size() >= 2)
            LoadState("savestate/"+tokens[1]);
      }
      else if (tokens[0] == "s")
      {
         SaveState("savestate/quicksave.bsk");
      }
      else if (tokens[0] == "l")
      {
         LoadState("savestate/quicksave.bsk");
      }
      else if (tokens[0] == "getwindowinfo")
      {
         ofLog() << "pos:(" << mMainComponent->getTopLevelComponent()->getPosition().x << ", " << mMainComponent->getTopLevelComponent()->getPosition().y << ") size:(" << ofGetWidth() << ", " << ofGetHeight() << ")";
      }
      else
      {
         ofLog() << "Creating: " << mConsoleText;
         ofVec2f grabOffset(-40,20);
         IDrawableModule* module = SpawnModuleOnTheFly(mConsoleText, GetMouseX() + grabOffset.x, GetMouseY() + grabOffset.y);
         TheSynth->SetMoveModule(module, grabOffset.x, grabOffset.y);
      }
   }
}

void ModularSynth::ClearConsoleInput()
{
   mConsoleText[0] = 0;
   mConsoleEntry->UpdateDisplayString();
}

namespace
{
   class FileTimeComparator
   {
   public:
      int compareElements(juce::File first, juce::File second)
      {
         return (first.getCreationTime() < second.getCreationTime()) ? 1 : ((first.getCreationTime() == second.getCreationTime()) ? 0 : -1);
      }
   };
}

void ModularSynth::DoAutosave()
{
   const int kMaxAutosaveSlots = 10;

   juce::File parentDirectory(ofToDataPath("savestate/autosave"));
   Array<juce::File> autosaveFiles;
   parentDirectory.findChildFiles(autosaveFiles, juce::File::findFiles, false, "*.bsk");
   if (autosaveFiles.size() >= kMaxAutosaveSlots)
   {
      FileTimeComparator cmp;
      autosaveFiles.sort(cmp, false);
      for (int i = kMaxAutosaveSlots; i < autosaveFiles.size(); ++i) //delete oldest files beyond slot limit
         autosaveFiles[i].deleteFile();
   }

   SaveState(ofGetTimestampString("savestate/autosave/autosave_%Y-%m-%d_%H-%M-%S.bsk"));
}

IDrawableModule* ModularSynth::SpawnModuleOnTheFly(string moduleName, float x, float y, bool addToContainer)
{
   if (mInitialized)
      TitleBar::sShowInitialHelpOverlay = false;  //don't show initial help popup

   vector<string> tokens = ofSplitString(moduleName," ");
   if (tokens.size() == 0)
      return nullptr;

   if (sShouldAutosave)
      DoAutosave();

   string moduleType = tokens[0];
   
   moduleType = ModuleFactory::FixUpTypeName(moduleType);

   string vstToSetUp = "";
   if (tokens.size() > 1 && tokens[tokens.size() - 1] == ModuleFactory::kVSTSuffix)
   {
      moduleType = "vstplugin";
      for (size_t i = 0; i < tokens.size() - 1; ++i)
      {
         vstToSetUp += tokens[i];
         if (i != tokens.size() - 2)
            vstToSetUp += " ";
      }
   }

   string prefabToSetUp = "";
   if (tokens.size() > 1 && tokens[tokens.size() - 1] == ModuleFactory::kPrefabSuffix)
   {
      moduleType = "prefab";
      for (size_t i = 0; i < tokens.size() - 1; ++i)
      {
         prefabToSetUp += tokens[i];
         if (i != tokens.size() - 2)
            prefabToSetUp += " ";
      }
   }

   string midiControllerToSetUp = "";
   if (tokens.size() > 1 && tokens[tokens.size() - 1] == ModuleFactory::kMidiControllerSuffix)
   {
      moduleType = "midicontroller";
      for (size_t i = 0; i < tokens.size() - 1; ++i)
      {
         midiControllerToSetUp += tokens[i];
         if (i != tokens.size() - 2)
            midiControllerToSetUp += " ";
      }
   }

   string effectToSetUp = "";
   if (tokens.size() > 1 && tokens[tokens.size() - 1] == ModuleFactory::kEffectChainSuffix)
   {
      moduleType = "effectchain";
      for (size_t i = 0; i < tokens.size() - 1; ++i)
      {
         effectToSetUp += tokens[i];
         if (i != tokens.size() - 2)
            effectToSetUp += " ";
      }
   }

   ofxJSONElement dummy;
   dummy["type"] = moduleType;
   vector<IDrawableModule*> allModules;
   mModuleContainer.GetAllModules(allModules);
   dummy["name"] = GetUniqueName(moduleType, allModules);
   dummy["onthefly"] = true;

   if (moduleType == "effectchain")
   {
      for (int i=1; i<tokens.size(); ++i)
      {
         if (VectorContains(tokens[i],GetEffectFactory()->GetSpawnableEffects()))
         {
            ofxJSONElement effect;
            effect["type"] = tokens[i];
            dummy["effects"].append(effect);
         }
      }
   }

   dummy["position"][0u] = x;
   dummy["position"][1u] = y;

   IDrawableModule* module = nullptr;
   try
   {
      ScopedMutex mutex(&mAudioThreadMutex, "CreateModule");
      module = CreateModule(dummy);
      if (module != nullptr)
      {
         if (addToContainer)
            mModuleContainer.AddModule(module);
         SetUpModule(module, dummy);
         module->Init();

         if (vstToSetUp != "")
         {
            VSTPlugin* plugin = dynamic_cast<VSTPlugin*>(module);
            if (plugin != nullptr)
               plugin->SetVST(vstToSetUp);
         }
      }
   }
   catch (LoadingJSONException& e)
   {
      LogEvent("Error spawning \""+moduleName+"\" on the fly", kLogEventType_Warning);
   }
   catch (UnknownModuleException& e)
   {
      LogEvent("Error spawning \""+moduleName+"\" on the fly, couldn't find \""+e.mSearchName+"\"", kLogEventType_Warning);
   }

   if (prefabToSetUp != "")
   {
      Prefab* prefab = dynamic_cast<Prefab*>(module);
      if (prefab != nullptr)
         prefab->LoadPrefab("prefabs" + GetPathSeparator() + prefabToSetUp);
   }

   if (midiControllerToSetUp != "")
   {
      MidiController* controller = dynamic_cast<MidiController*>(module);
      if (controller != nullptr)
      {
         controller->GetSaveData().SetString("devicein", midiControllerToSetUp);
         controller->SetUpFromSaveData();
      }
   }

   if (effectToSetUp != "")
   {
      EffectChain* effectChain = dynamic_cast<EffectChain*>(module);
      if (effectChain != nullptr)
         effectChain->AddEffect(effectToSetUp, K(onTheFly));
   }

   return module;
}

void ModularSynth::SetMoveModule(IDrawableModule* module, float offsetX, float offsetY)
{
   mMoveModule = module;
   mMoveModuleOffsetX = offsetX;
   mMoveModuleOffsetY = offsetY;
}

void ModularSynth::AddMidiDevice(MidiDevice* device)
{
   if (!VectorContains(device, mMidiDevices))
      mMidiDevices.push_back(device);
}

void ModularSynth::ReconnectMidiDevices()
{
   for (int i=0; i<mMidiDevices.size(); ++i)
      mMidiDevices[i]->Reconnect();
}

void ModularSynth::SaveOutput()
{
   ScopedMutex mutex(&mAudioThreadMutex, "SaveOutput()");

   string filename = ofGetTimestampString("recordings/recording_%Y-%m-%d_%H-%M.wav");
   //string filenamePos = ofGetTimestampString("recordings/pos_%Y-%m-%d_%H-%M.wav");

   assert(mRecordingLength <= mGlobalRecordBuffer->Size());
   
   for (int i=0; i<mRecordingLength; ++i)
   {
      mSaveOutputBuffer[0][i] = mGlobalRecordBuffer->GetSample((int)mRecordingLength-i-1, 0);
      mSaveOutputBuffer[1][i] = mGlobalRecordBuffer->GetSample((int)mRecordingLength-i-1, 1);
   }

   Sample::WriteDataToFile(filename.c_str(), mSaveOutputBuffer, (int)mRecordingLength, 2);
   
   //mOutputBufferMeasurePos.ReadChunk(mSaveOutputBuffer, mRecordingLength);
   //Sample::WriteDataToFile(filenamePos.c_str(), mSaveOutputBuffer, mRecordingLength, 1);
   
   mGlobalRecordBuffer->ClearBuffer();
   mRecordingLength = 0;
}

void ModularSynth::SetFatalError(string error)
{
   if (mFatalError == "")
   {
      mFatalError = error;
      if (mUserPrefsEditor != nullptr)
         mUserPrefsEditor->Show();
      if (TheTitleBar != nullptr)
         TheTitleBar->SetShowing(false);
   }
}

void ConsoleListener::TextEntryActivated(TextEntry* entry)
{
   TheSynth->ClearConsoleInput();
}

void ConsoleListener::TextEntryComplete(TextEntry* entry)
{
   TheSynth->OnConsoleInput();
}
