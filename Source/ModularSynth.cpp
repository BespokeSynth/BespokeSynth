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
#include "VinylTempoControl.h"
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

ModularSynth* TheSynth = nullptr;

#define RECORDING_LENGTH (gSampleRate*60*30) //30 minutes of recording

void AtExit()
{
   TheSynth->Exit();
}

ModularSynth::ModularSynth()
: mMoveModule(nullptr)
, mOutputBuffer(RECORDING_LENGTH)
, mAudioPaused(false)
, mIsLoadingState(false)
, mClickStartX(INT_MAX)
, mClickStartY(INT_MAX)
, mHeldSample(nullptr)
, mConsoleListener(nullptr)
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
, mIsLoadingModule(false)
{
   mConsoleText[0] = 0;
   assert(TheSynth == nullptr);
   TheSynth = this;
   
   mSaveOutputBuffer[0] = new float[RECORDING_LENGTH];
   mSaveOutputBuffer[1] = new float[RECORDING_LENGTH];
   
   mOutputBuffer.SetNumChannels(2);
}

ModularSynth::~ModularSynth()
{
   DeleteAllModules();
   
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
   
   bool loaded = mUserPrefs.open(GetUserPrefsPath());
   if (loaded)
   {
      SetGlobalBufferSize(mUserPrefs["buffersize"].asInt());
      mIOBufferSize = gBufferSize;
      gSampleRate = mUserPrefs["samplerate"].asInt();

      juce::File(ofToDataPath("savestate")).createDirectory();
      juce::File(ofToDataPath("recordings")).createDirectory();
      juce::File(ofToDataPath("internal")).createDirectory();
      juce::File(ofToDataPath("scripts")).createDirectory();
   }
   else
   {
      mFatalError = "couldn't find or load data/userprefs.json";
#if BESPOKE_MAC
      if (!juce::File(GetUserPrefsPath()).existsAsFile())
         mFatalError += "\nplease launch via run_bespoke.command";
#endif
      LogEvent("couldn't find or load userprefs.json", kLogEventType_Error);
      SetGlobalBufferSize(256);
      mIOBufferSize = gBufferSize;
      gSampleRate = 44100;
   }
   
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
}

void ModularSynth::LoadResources(void* nanoVG, void* fontBoundsNanoVG)
{
   gNanoVG = (NVGcontext*)nanoVG;
   gFontBoundsNanoVG = (NVGcontext*)fontBoundsNanoVG;
   LoadGlobalResources();
}

//static
string ModularSynth::GetUserPrefsPath()
{
   #if BESPOKE_WINDOWS
   string path = ofToDataPath("userprefs_win.json");
   if (juce::File(path).existsAsFile())
      return path;
   #endif

   #if BESPOKE_LINUX
   string path = ofToDataPath("userprefs_linux.json");
   if (juce::File(path).existsAsFile())
      return path;
   #endif
   
   return ofToDataPath("userprefs.json");
}

static int sFrameCount = 0;
void ModularSynth::Poll()
{
   if (!mInitialized && sFrameCount > 3) //let some frames render before blocking for a load
   {
      LoadLayoutFromFile(ofToDataPath(mUserPrefs["layout"].asString()));
      mInitialized = true;
   }
   
   mZoomer.Update();
   for (auto p : mExtraPollers)
      p->Poll();
   mModuleContainer.Poll();
   
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

void ModularSynth::ZoomView(float zoomAmount)
{
   float oldDrawScale = gDrawScale;
   gDrawScale *= 1 + zoomAmount;
   float minZoom = .1f;
   float maxZoom = 8;
   gDrawScale = ofClamp(gDrawScale,minZoom,maxZoom);
   zoomAmount = (gDrawScale - oldDrawScale) / oldDrawScale; //find actual adjusted amount
   ofVec2f zoomCenter = ofVec2f(GetMouseX(), GetMouseY()) + mDrawOffset;
   mDrawOffset -= zoomCenter * zoomAmount;
   mZoomer.CancelMovement();
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
      return;
   }
   
   DrawLissajous(&mOutputBuffer, 0, 0, ofGetWidth(), ofGetHeight(), .7f, 0, 0);
   
   if (gTime == 1)
   {
      string loading("Bespoke is initializing audio...");
      DrawTextNormal(loading,ofGetWidth()/2-GetStringWidth(loading,30)/2,ofGetHeight()/2-6, 30);
      return;
   }
   
   if (!mInitialized)
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
   
   ofPopMatrix();
   
   Profiler::Draw();
   
   DrawConsole();
   
   ofPopMatrix();
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
      consoleY += 15;
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
   
   int outputLines = (int)mEvents.size();
   if (IKeyboardFocusListener::GetActiveKeyboardFocus() == mConsoleEntry)
      outputLines += mErrors.size();
   if (outputLines > 0)
   {
      ofPushStyle();
      ofSetColor(0,0,0,150);
      ofFill();
      ofRect(10,consoleY-15,400,outputLines*15+3);
      ofPopStyle();
   }
   
   for (auto it=mEvents.begin(); it != mEvents.end();)
   {
      if (it->second + 3000 > gTime)
      {
         ofPushStyle();
         ofSetColor(255,255,0);
         DrawTextNormal(it->first, 10, consoleY);
         ofPopStyle();
         consoleY += 15;
         ++it;
      }
      else
      {
         it = mEvents.erase(it);
      }
   }
   
   if (IKeyboardFocusListener::GetActiveKeyboardFocus() == mConsoleEntry)
   {
      if (!mErrors.empty())
      {
         ofPushStyle();
         ofSetColor(255,0,0);
         for (int i=0;i<mErrors.size();++i)
         {
            DrawTextNormal(mErrors[i], 10, consoleY);
            consoleY += 15;
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
      else if (key != OF_KEY_BACKSPACE && key != ' ' && key != OF_KEY_TAB)
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
      ADSRDisplay::ToggleDisplayMode();

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

void ModularSynth::MouseMoved(int intX, int intY )
{
   mMousePos.x = intX;
   mMousePos.y = intY;
   
   if (IsKeyHeld(' '))
   {
      mDrawOffset += (ofVec2f(intX,intY) - mLastMoveMouseScreenPos) / gDrawScale;
      mZoomer.CancelMovement();
   }
   
   mLastMoveMouseScreenPos = ofVec2f(intX,intY);
   
   float x = GetMouseX();
   float y = GetMouseY();

   for (auto* modal : mModalFocusItemStack)
      modal->NotifyMouseMoved(x,y);

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

   mModuleContainer.MouseMoved(x, y);
   
   if (gHoveredUIControl)
   {
      float uiX,uiY;
      gHoveredUIControl->GetPosition(uiX, uiY);
      float w, h;
      gHoveredUIControl->GetDimensions(w, h);
      
      if (x < uiX - 10 || y < uiY - 10 || x > uiX + w + 10 || y > uiY + h + 10)
         gHoveredUIControl = nullptr;
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
   mMousePos.x = intX;
   mMousePos.y = intY;
   
   float x = GetMouseX();
   float y = GetMouseY();
   
   mLastMouseDragPos = ofVec2f(x,y);
   mGroupSelectContext = nullptr;
   
   bool rightButton = button == 2;

   IKeyboardFocusListener::ClearActiveKeyboardFocus(K(acceptEntry));

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
      mGroupSelectContext = &mModuleContainer;
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
   else
      TheSaveDataPanel->SetModule(nullptr);
}

void ModularSynth::MouseScrolled(float x, float y)
{
   if (IsKeyHeld(' '))
   {
      ZoomView(y/100);
   }
   else if (gHoveredUIControl)
   {
#if JUCE_WINDOWS
      y *= -1;
      y -= x / 3; //taking advantage of logitech horizontal scroll wheel
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
         
      float change = -y/100 * movementScale;
         
      if (floatSlider && floatSlider->GetLFO() && floatSlider->GetLFO()->Enabled())
      {
         FloatSliderLFOControl* lfo = floatSlider->GetLFO();
         float min = floatSlider->GetMin();
         float max = floatSlider->GetMax();
         float lfoMin = ofMap(lfo->GetMin(),min,max,0,1);
         float lfoMax = ofMap(lfo->GetMax(),min,max,0,1);
            
         float changeX = x/100 * movementScale;
            
         lfo->GetMin() = ofMap(lfoMin + change,0,1,min,max,K(clamp));
         lfo->GetMax() = ofMap(lfoMax + changeX,0,1,min,max,K(clamp));
            
         return;
      }
         
      if (gHoveredUIControl->InvertScrollDirection())
         val -= change;
      else
         val += change;
      val = ofClamp(val, 0, 1);
      gHoveredUIControl->SetFromMidiCC(val);
   }
   else
   {
#if JUCE_WINDOWS
      y *= -1;
      y -= x / 3; //taking advantage of logitech horizontal scroll wheel
#endif

      IDrawableModule* module = GetModuleAt(GetMouseX(), GetMouseY());
      if (module)
         module->NotifyMouseScrolled(GetMouseX(), GetMouseY(), x, y);
   }
}

void ModularSynth::MouseMagnify(int intX, int intY, float scaleFactor)
{
   mMousePos.x = intX;
   mMousePos.y = intY;
   ZoomView(scaleFactor - 1);
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
   if (module == TheVinylTempoControl)
      TheVinylTempoControl = nullptr;
   
   for (int i=0; i<MAX_INPUT_CHANNELS; ++i)
   {
      if (module == mInput[i])
         mInput[i] = nullptr;
   }
   
   for (int i=0; i<MAX_OUTPUT_CHANNELS; ++i)
   {
      if (module == mOutput[i])
         mOutput[i] = nullptr;
   }
   
   mAudioThreadMutex.Unlock();
}

void ModularSynth::MouseReleased(int intX, int intY, int button)
{
   mMousePos.x = intX;
   mMousePos.y = intY;
   
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
      mGroupSelectContext->GetModulesWithinRect(ofRectangle(ofPoint(mClickStartX,mClickStartY),ofPoint(x,y)), mGroupSelectedModules);
      if (mGroupSelectedModules.size() > 0)
      {
         for (int i=(int)mGroupSelectedModules.size()-1; i>=0; --i) //do this backwards to preserve existing order
            MoveToFront(mGroupSelectedModules[i]);
      }
      mGroupSelectContext = nullptr;
   }
   
   mClickStartX = INT_MAX;
   mClickStartY = INT_MAX;
}

void ModularSynth::AudioOut(float** output, int bufferSize, int nChannels)
{
   PROFILER(audioOut_total);
   
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
   
   assert(nChannels <= MAX_OUTPUT_CHANNELS);
   
   /////////// AUDIO PROCESSING STARTS HERE /////////////
   float* outBuffer[MAX_OUTPUT_CHANNELS];
   assert(bufferSize == mIOBufferSize);
   assert(mIOBufferSize == gBufferSize);  //need to be the same for now
                                          //if we want these different, need to fix outBuffer here, and also fix audioIn()
   for (int ioOffset = 0; ioOffset < mIOBufferSize; ioOffset += gBufferSize)
   {
      if (TheVinylTempoControl &&
          mInput[TheVinylTempoControl->GetLeftChannel()-1] &&
          mInput[TheVinylTempoControl->GetRightChannel()-1])
      {
         TheVinylTempoControl->SetVinylControlInput(
               mInput[TheVinylTempoControl->GetLeftChannel()-1]->GetBuffer()->GetChannel(0),
               mInput[TheVinylTempoControl->GetRightChannel()-1]->GetBuffer()->GetChannel(0), gBufferSize);
      }

      for (int i=0; i<nChannels; ++i)
      {
         if (mOutput[i])
            mOutput[i]->ClearBuffer();
      }
      
      //get audio from sources
      for (int i=0; i<mSources.size(); ++i)
         mSources[i]->Process(gTime);
      
      //put it into speakers
      for (int i=0; i<MAX_OUTPUT_CHANNELS; ++i)
         outBuffer[i] = gZeroBuffer;
      for (int i=0; i<nChannels; ++i)
      {
         if (mOutput[i])
         {
            mOutput[i]->Process();
            outBuffer[i] = mOutput[i]->GetBuffer()->GetChannel(0);
         }
      }
      
      if (TheMultitrackRecorder)
         TheMultitrackRecorder->Process(gTime, outBuffer[0], outBuffer[1], gBufferSize);
      
      for (int ch=0; ch<nChannels; ++ch)
         BufferCopy(output[ch]+ioOffset, outBuffer[ch]+ioOffset, gBufferSize);
      
      double elapsed = gInvSampleRateMs * gBufferSize;
      gTime += elapsed;
      TheTransport->Advance(elapsed);
   }
   /////////// AUDIO PROCESSING ENDS HERE /////////////
   
   mOutputBuffer.WriteChunk(outBuffer[0], bufferSize, 0);
   mOutputBuffer.WriteChunk(outBuffer[1], bufferSize, 1);
   mRecordingLength += bufferSize;
   mRecordingLength = MIN(mRecordingLength, RECORDING_LENGTH);
   
   Profiler::PrintCounters();
}

void ModularSynth::AudioIn(const float** input, int bufferSize, int nChannels)
{
   if (mAudioPaused)
      return;
   
   ScopedMutex mutex(&mAudioThreadMutex, "audioIn()");

   assert(bufferSize == mIOBufferSize);
   assert(nChannels <= MAX_INPUT_CHANNELS);
   
   for (int i=0; i<nChannels; ++i)
   {
      if (mInput[i])
         BufferCopy(mInput[i]->GetBuffer()->GetChannel(0), input[i], bufferSize);
   }
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
   
   for (int i=0; i<MAX_INPUT_CHANNELS; ++i)
      mInput[i] = nullptr;
   for (int i=0; i<MAX_OUTPUT_CHANNELS; ++i)
      mOutput[i] = nullptr;

   mDeletedModules.clear();
   mSources.clear();
   mLissajousDrawers.clear();
   mMoveModule = nullptr;
   LFOPool::Shutdown();
   IKeyboardFocusListener::ClearActiveKeyboardFocus(!K(acceptEntry));
   
   mEvents.clear();
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
   
   mDrawOffset.set(0,0);
   mZoomer.Init();
}

bool ModularSynth::SetInputChannel(int channel, InputChannel* input)
{
   assert(channel > 0 && channel <= MAX_INPUT_CHANNELS);
   
   //ScopedMutex mutex(&mAudioThreadMutex, "SetInputChannel()");
   
   for (int i=0; i<MAX_INPUT_CHANNELS; ++i)  //remove if we're changing an already assigned channel
   {
      if (mInput[channel-1] == input)
         mInput[channel-1] = nullptr;
   }
   
   if (mInput[channel-1] == nullptr)
   {
      mInput[channel-1] = input;
      return true;
   }
   
   return false;
}

bool ModularSynth::SetOutputChannel(int channel, OutputChannel* output)
{
   assert(channel > 0 && channel <= MAX_OUTPUT_CHANNELS);
   
   //ScopedMutex mutex(&mAudioThreadMutex, "SetOutputChannel()");
   if (mOutput[channel-1] == nullptr)
   {
      mOutput[channel-1] = output;
      return true;
   }
   
   return false;
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
   if (type == kLogEventType_Normal)
   {
      ofLog() << "event: " << event;
      
      mEvents.push_back(std::pair<string,double>(event,gTime));
   }
   else if (type == kLogEventType_Error)
   {
      ofLog() << "error: " << event;
      
      mErrors.push_back(event);
   }
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
   FileChooser chooser("Save current layout as...", File(ofToDataPath("layouts/newlayout.json")));
   if (chooser.browseForFileToSave(true))
      SaveLayout(chooser.getResult().getRelativePathFrom(File(ofToDataPath(""))).toStdString());
}

void ModularSynth::SaveStatePopup()
{
   FileChooser chooser("Save current state as...", File(ofToDataPath(ofGetTimestampString("savestate/%Y-%m-%d_%H-%M.bsk"))));
   if (chooser.browseForFileToSave(true))
      SaveState(chooser.getResult().getRelativePathFrom(File(ofToDataPath(""))).toStdString());
}

void ModularSynth::LoadStatePopup()
{
   mShowLoadStatePopup = true;
}

void ModularSynth::LoadStatePopupImp()
{
   FileChooser chooser("Load state", File(ofToDataPath("savestate")), "*.bsk");
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

IDrawableModule* ModularSynth::SpawnModuleOnTheFly(string moduleName, float x, float y, bool addToContainer)
{
   vector<string> tokens = ofSplitString(moduleName," ");
   if (tokens.size() == 0)
      return nullptr;
   
   if (tokens[0] == "siggen")
      tokens[0] = "signalgenerator";

   ofxJSONElement dummy;
   dummy["type"] = tokens[0];
   vector<IDrawableModule*> allModules;
   mModuleContainer.GetAllModules(allModules);
   dummy["name"] = GetUniqueName(tokens[0], allModules);
   dummy["onthefly"] = true;

   if (tokens[0] == "effectchain")
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
      }
   }
   catch (LoadingJSONException& e)
   {
      LogEvent("Error spawning \""+moduleName+"\" on the fly", kLogEventType_Normal);
   }
   catch (UnknownModuleException& e)
   {
      LogEvent("Error spawning \""+moduleName+"\" on the fly, couldn't find \""+e.mSearchName+"\"", kLogEventType_Normal);
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

   assert(mRecordingLength <= RECORDING_LENGTH);
   
   for (int i=0; i<mRecordingLength; ++i)
   {
      mSaveOutputBuffer[0][i] = mOutputBuffer.GetSample((int)mRecordingLength-i-1, 0);
      mSaveOutputBuffer[1][i] = mOutputBuffer.GetSample((int)mRecordingLength-i-1, 1);
   }

   Sample::WriteDataToFile(filename.c_str(), mSaveOutputBuffer, (int)mRecordingLength, 2);
   
   //mOutputBufferMeasurePos.ReadChunk(mSaveOutputBuffer, mRecordingLength);
   //Sample::WriteDataToFile(filenamePos.c_str(), mSaveOutputBuffer, mRecordingLength, 1);
   
   mOutputBuffer.ClearBuffer();
   mRecordingLength = 0;
}

void ConsoleListener::TextEntryActivated(TextEntry* entry)
{
   TheSynth->ClearConsoleInput();
}

void ConsoleListener::TextEntryComplete(TextEntry* entry)
{
   TheSynth->OnConsoleInput();
}
