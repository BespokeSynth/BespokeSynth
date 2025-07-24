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
/*
  ==============================================================================

    MathUtils.h
    Created: 12 Nov 2017 8:24:59pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "OpenFrameworksPort.h"

#define CUBE(x) ((x) * (x) * (x))
#define SQUARE(x) ((x) * (x))

namespace MathUtils
{
   double Bezier(double t, double p0, double p1, double p2, double p3);
   ofVec2d Bezier(double t, ofVec2d p0, ofVec2d p1, ofVec2d p2, ofVec2d p3);
   double BezierDerivative(double t, double p0, double p1, double p2, double p3);
   ofVec2d BezierPerpendicular(double t, ofVec2d p0, ofVec2d p1, ofVec2d p2, ofVec2d p3);
   ofVec2d ScaleVec(ofVec2d a, ofVec2d b);
   ofVec2d Normal(ofVec2d v);
   float Curve(float t, float curve);
   double Curve(double t, double curve);
   int HighestPow2(int n);
};
