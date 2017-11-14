/*
  ==============================================================================

    MathUtils.h
    Created: 12 Nov 2017 8:24:59pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "OpenFrameworksPort.h"

#define CUBE(x) ((x)*(x)*(x))
#define SQUARE(x) ((x)*(x))

namespace MathUtils
{
   float Bezier(float t, float p0, float p1, float p2, float p3);
   ofVec2f Bezier(float t, ofVec2f p0, ofVec2f p1, ofVec2f p2, ofVec2f p3);
   float BezierDerivative(float t, float p0, float p1, float p2, float p3);
   ofVec2f BezierPerpendicular(float t, ofVec2f p0, ofVec2f p1, ofVec2f p2, ofVec2f p3);
   ofVec2f ScaleVec(ofVec2f a, ofVec2f b);
   ofVec2f Normal(ofVec2f v);
   float Curve(float t, float curve);
};
