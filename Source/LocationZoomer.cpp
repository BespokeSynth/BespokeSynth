/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
//
//  LocationZoomer.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/6/14.
//
//

#include "LocationZoomer.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "UserPrefs.h"

#include "juce_core/juce_core.h"

LocationZoomer::LocationZoomer()
: mCurrentProgress(1)
, mSpeed(2)
, mInVanityPanningMode(false)
{
}

void LocationZoomer::Init()
{
   mHome.mZoomLevel = UserPrefs.zoom.Get();
   mHome.mOffset.set(0,0);
   mStart = mHome;
   mDestination = mHome;
   mCurrentProgress = 1;
   gDrawScale = mHome.mZoomLevel;
}

void LocationZoomer::Update()
{
   if (mCurrentProgress < 1)
   {
      mCurrentProgress = ofClamp(mCurrentProgress + ofGetLastFrameTime() * mSpeed, 0, 1);
      float ease;
      if (mInVanityPanningMode)  //ease in/out
         ease = mCurrentProgress < 0.5 ? 2 * mCurrentProgress * mCurrentProgress : 1 - pow(-2 * mCurrentProgress + 2, 2) / 2;
      else  //ease out
         ease = -1 * mCurrentProgress*(mCurrentProgress-2);
      gDrawScale = ofLerp(mStart.mZoomLevel, mDestination.mZoomLevel, ease);
      ofVec2f offset;
      offset.x = ofLerp(mStart.mOffset.x, mDestination.mOffset.x, ease);
      offset.y = ofLerp(mStart.mOffset.y, mDestination.mOffset.y, ease);
      TheSynth->SetDrawOffset(offset);
      
      if (mInVanityPanningMode && mCurrentProgress >= 1)
         PickNewVanityPanningDestination();
   }
}

void LocationZoomer::OnKeyPressed(char key)
{
   if (key < CHAR_MAX && juce::CharacterFunctions::isDigit(key) && key != '0') // 0 is reserved
   {
      if (GetKeyModifiers() == kModifier_Command)
         WriteCurrentLocation(key);
      else if (GetKeyModifiers() == kModifier_Shift)
         MoveToLocation(key);
   }
}

void LocationZoomer::WriteCurrentLocation(char key)
{
   mLocations[key].mZoomLevel = gDrawScale;
   mLocations[key].mOffset = TheSynth->GetDrawOffset();
}

bool LocationZoomer::HasLocation(char key)
{
   return mLocations.find(key) != mLocations.end();
}

void LocationZoomer::MoveToLocation(char key)
{
   if (mLocations.count(key) > 0)
   {
      mStart.mZoomLevel = gDrawScale;
      mStart.mOffset = TheSynth->GetDrawOffset();
      mDestination = mLocations[key];
      mCurrentProgress = 0;
      mSpeed = 2;
   }
}

void LocationZoomer::GoHome()
{
   mStart.mZoomLevel = gDrawScale;
   mStart.mOffset = TheSynth->GetDrawOffset();
   mDestination = mHome;
   mCurrentProgress = 0;
   mSpeed = 2;
}

void LocationZoomer::EnterVanityPanningMode()
{
   mInVanityPanningMode = true;
   PickNewVanityPanningDestination();
}

void LocationZoomer::ExitVanityPanningMode()
{
   mInVanityPanningMode = false;
   mCurrentProgress = 1;
}

void LocationZoomer::PickNewVanityPanningDestination()
{
   std::vector<IDrawableModule*> modules;
   TheSynth->GetAllModules(modules);
   
   ofVec2f allModulesCenter;
   for (int i=0; i<(int)modules.size(); ++i)
   {
      if (modules[i]->IsShowing() && !modules[i]->Minimized())
      {
         ofVec2f modulePos = modules[i]->GetRect().getCenter();
         allModulesCenter += modulePos / (int)modules.size();
      }
   }
   
   const int kRandomChoices = 3;
   ofVec2f randomModulesCenter;
   int attempts = 0;
   for (int i=0; i<kRandomChoices; ++i)
   {
      int choice = gRandom() % ((int)modules.size());
      if (modules[choice]->IsShowing() && !modules[choice]->Minimized())
      {
         ofVec2f modulePos = modules[choice]->GetRect().getCenter();
         randomModulesCenter += modulePos / kRandomChoices;
      }
      else
      {
         --i;  //try again
      }
      ++attempts;
      
      if (attempts > 100)  //avoid infinite loop if all modules are hidden/minimized
         break;
   }
   
   ofVec2f center = allModulesCenter * .5f + randomModulesCenter * .5f;
   
   float newScale = ofRandom(1, 1.5f) * UserPrefs.zoom.Get();
   
   mStart.mZoomLevel = gDrawScale;
   mStart.mOffset = TheSynth->GetDrawOffset();
   
   mDestination.mZoomLevel = newScale;
   mDestination.mOffset = (center * -1) + ofVec2f(ofGetWidth(), ofGetHeight()) / newScale * .5f;
   
   mCurrentProgress = 0;
   mSpeed = ofRandom(.03f, .1f);
}

ofxJSONElement LocationZoomer::GetSaveData()
{
   ofxJSONElement save;
   save.resize((unsigned int)mLocations.size());
   int i=0;
   for (auto iter = mLocations.begin(); iter != mLocations.end(); ++iter)
   {
      const Location& loc = iter->second;
      save[i]["shortcut"] = iter->first;
      save[i]["zoomlevel"] = loc.mZoomLevel;
      save[i]["offset_x"] = loc.mOffset.x;
      save[i]["offset_y"] = loc.mOffset.y;
      ++i;
   }
   return save;
}

void LocationZoomer::LoadFromSaveData(const ofxJSONElement& saveData)
{
   mLocations.clear();
   for (int i=0; i<saveData.size(); ++i)
   {
      try
      {
         int shortcut = saveData[i]["shortcut"].asInt();
         mLocations[shortcut].mZoomLevel = saveData[i]["zoomlevel"].asDouble();
         mLocations[shortcut].mOffset.set(saveData[i]["offset_x"].asDouble(),
            saveData[i]["offset_y"].asDouble());
      }
      catch (Json::LogicError& e)
      {
         TheSynth->LogEvent(__PRETTY_FUNCTION__ + std::string(" json error: ") + e.what(), kLogEventType_Error);
      }
   }
   MoveToLocation('1');
   mCurrentProgress = .999f;
}
