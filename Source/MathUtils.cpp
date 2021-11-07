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

    MathUtils.cpp
    Created: 12 Nov 2017 8:24:59pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "MathUtils.h"

namespace MathUtils
{
   float Bezier(float t, float p0, float p1, float p2, float p3)
   {
      return CUBE(1-t) * p0 + 3 * SQUARE(1-t) * t * p1 + 3 * (1-t) * SQUARE(t) * p2 + CUBE(t) * p3;
   }
   
   ofVec2f Bezier(float t, ofVec2f p0, ofVec2f p1, ofVec2f p2, ofVec2f p3)
   {
      return ofVec2f(Bezier(t, p0.x, p1.x, p2.x, p3.x), Bezier(t, p0.y, p1.y, p2.y, p3.y));
      
      //below comments help visualize bezier
      /*if (t < .333f)
       return ofVec2f(ofLerp(p0.x,p1.x, t*3), ofLerp(p0.y,p1.y, t*3));
       else if (t < .666f)
       return ofVec2f(ofLerp(p1.x,p2.x,(t-.333f)*3), ofLerp(p1.y,p2.y,(t-.333f)*3));
       else
       return ofVec2f(ofLerp(p2.x,p3.x, (t-.666f)*3), ofLerp(p2.y,p3.y, (t-.666f)*3));*/
   }
   
   float BezierDerivative(float t, float p0, float p1, float p2, float p3)
   {
      return 3 * SQUARE(1-t) * (p1-p0) + 6 * (1-t) * t * (p2-p1) + 3 * t * t * (p3-p2);
   }
   
   ofVec2f BezierPerpendicular(float t, ofVec2f p0, ofVec2f p1, ofVec2f p2, ofVec2f p3)
   {
      ofVec2f perp(-BezierDerivative(t, p0.y, p1.y, p2.y, p3.y), BezierDerivative(t, p0.x, p1.x, p2.x, p3.x));
      return perp / sqrt(perp.lengthSquared());
   }
   
   ofVec2f ScaleVec(ofVec2f a, ofVec2f b)
   {
      return ofVec2f(a.x * b.x, a.y * b.y);
   }
   
   ofVec2f Normal(ofVec2f v)
   {
      return v / sqrtf(v.lengthSquared());
   }
   
   float Curve(float t, float curve)
   {
      /*float a = ofLerp(0, .5f+curve*.5f, t);
      float b = ofLerp(.5f+curve*.5f, 1, t);
      float c = ofLerp(a, b, t);
      return c;*/
      /*if (curve > 0)
       return ofLerp(c, b, t);
       else
       return ofLerp(a, c, t);*/
      return powf(t, powf(5,-curve));
   }

   int HighestPow2(int n)
   {
      int res = 0;
      for (int i = n; i >= 1; i--)
      {
         // If i is a power of 2
         if ((i & (i - 1)) == 0)
         {
            res = i;
            break;
         }
      }
      return res;
   }
};
