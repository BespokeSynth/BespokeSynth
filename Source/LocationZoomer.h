//
//  LocationZoomer.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/6/14.
//
//

#ifndef __Bespoke__LocationZoomer__
#define __Bespoke__LocationZoomer__

#include "OpenFrameworksPort.h"
#include "ofxJSONElement.h"

class LocationZoomer
{
public:
   LocationZoomer();
   void Init();
   void Update();
   void OnKeyPressed(char key);
   void CancelMovement() { mCurrentProgress = 1; }
   void GoHome();
   ofxJSONElement GetSaveData();
   void LoadFromSaveData(const ofxJSONElement& saveData);
private:
   void WriteCurrentLocation(char key);
   void MoveToLocation(char key);
   
   struct Location
   {
      float mZoomLevel;
      ofVec2f mOffset;
   };
   
   map<int, Location> mLocations;
   Location mStart;
   Location mDestination;
   float mCurrentProgress;
   Location mHome;
};

#endif /* defined(__Bespoke__LocationZoomer__) */
