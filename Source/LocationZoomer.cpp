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

LocationZoomer::LocationZoomer()
: mCurrentProgress(1)
{
}

void LocationZoomer::Init()
{
   mHome.mZoomLevel = 1;
   mHome.mOffset.set(0,0);
   mStart = mHome;
   mDestination = mHome;
   mCurrentProgress = 1;
}

void LocationZoomer::Update()
{
   if (mCurrentProgress < 1)
   {
      mCurrentProgress = ofClamp(mCurrentProgress + ofGetLastFrameTime() * 2, 0, 1);
      float easeOut = -1 * mCurrentProgress*(mCurrentProgress-2);
      gDrawScale = ofLerp(mStart.mZoomLevel, mDestination.mZoomLevel, easeOut);
      ofVec2f offset;
      offset.x = ofLerp(mStart.mOffset.x, mDestination.mOffset.x, easeOut);
      offset.y = ofLerp(mStart.mOffset.y, mDestination.mOffset.y, easeOut);
      TheSynth->SetDrawOffset(offset);
   }
}

void LocationZoomer::OnKeyPressed(char key)
{
   if (CharacterFunctions::isDigit((char)key) && key != '0') //0 is reserved
   {
      if (GetKeyModifiers() == kModifier_Control)
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

void LocationZoomer::MoveToLocation(char key)
{
   if (mLocations.count(key) > 0)
   {
      mStart.mZoomLevel = gDrawScale;
      mStart.mOffset = TheSynth->GetDrawOffset();
      mDestination = mLocations[key];
      mCurrentProgress = 0;
   }
}

void LocationZoomer::GoHome()
{
   mStart.mZoomLevel = gDrawScale;
   mStart.mOffset = TheSynth->GetDrawOffset();
   mDestination = mHome;
   mCurrentProgress = 0;
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
      int shortcut = saveData[i]["shortcut"].asInt();
      mLocations[shortcut].mZoomLevel = saveData[i]["zoomlevel"].asDouble();
      mLocations[shortcut].mOffset.set(saveData[i]["offset_x"].asDouble(),
                                       saveData[i]["offset_y"].asDouble());
   }
   MoveToLocation('1');
   mCurrentProgress = .999f;
}
