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
   double Bezier(double t, double p0, double p1, double p2, double p3)
   {
      return CUBE(1 - t) * p0 + 3 * SQUARE(1 - t) * t * p1 + 3 * (1 - t) * SQUARE(t) * p2 + CUBE(t) * p3;
   }

   ofVec2d Bezier(double t, ofVec2d p0, ofVec2d p1, ofVec2d p2, ofVec2d p3)
   {
      return { Bezier(t, p0.x, p1.x, p2.x, p3.x), Bezier(t, p0.y, p1.y, p2.y, p3.y) };

      //below comments help visualize bezier
      /*
      if (t < .333)
         return { ofLerp(p0.x, p1.x, t * 3), ofLerp(p0.y, p1.y, t * 3) };
      else if (t < .666)
         return { ofLerp(p1.x, p2.x, (t - .333) * 3), ofLerp(p1.y, p2.y, (t - .333) * 3) };
      else
         return { ofLerp(p2.x, p3.x, (t - .666) * 3), ofLerp(p2.y, p3.y, (t - .666) * 3) };
      */
   }

   double BezierDerivative(double t, double p0, double p1, double p2, double p3)
   {
      return 3 * SQUARE(1 - t) * (p1 - p0) + 6 * (1 - t) * t * (p2 - p1) + 3 * t * t * (p3 - p2);
   }

   ofVec2d BezierPerpendicular(double t, ofVec2d p0, ofVec2d p1, ofVec2d p2, ofVec2d p3)
   {
      ofVec2d perp(-BezierDerivative(t, p0.y, p1.y, p2.y, p3.y), BezierDerivative(t, p0.x, p1.x, p2.x, p3.x));
      return perp / sqrt(perp.lengthSquared());
   }

   ofVec2d ScaleVec(ofVec2d a, ofVec2d b)
   {
      return { a.x * b.x, a.y * b.y };
   }

   ofVec2d Normal(ofVec2d v)
   {
      return v / sqrt(v.lengthSquared());
   }

   float Curve(float t, float curve)
   {
      return powf(t, expf(-2 * curve));
   }

   double Curve(double t, double curve)
   {
      return pow(t, exp(-2 * curve));
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
