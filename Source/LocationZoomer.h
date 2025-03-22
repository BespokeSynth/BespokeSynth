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
//  LocationZoomer.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/6/14.
//
//

#pragma once

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
   void EnterVanityPanningMode();
   void ExitVanityPanningMode();
   void WriteCurrentLocation(char key);
   bool HasLocation(char key);
   void MoveToLocation(char key);

private:
   void PickNewVanityPanningDestination();

   struct Location
   {
      float mZoomLevel{ 1 };
      ofVec2f mOffset{ 0, 0 };
   };

   Location mLoadLocation{};
   std::map<int, Location> mLocations{};
   Location mStart{};
   Location mDestination{};
   float mCurrentProgress{ 1 };
   float mSpeed{ 2 };
   Location mHome{};

   bool mInVanityPanningMode{ false };
};
