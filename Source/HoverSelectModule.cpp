#include "HoverSelectModule.h"


#include "QuickSpawnMenu.h"

void HoverSelectModule::DrawHoverSelect()
{
   float mWidth;
   float mHeight;
   GetModuleDimensions(mWidth, mHeight);

   float rO = 1.5f; //radiusOffset
   if (hovered)
   {
      if (TheQuickSpawnMenu->IsShowing())
      {
         hovered = false;
         OnHoverExit();
         return;
      }

      ofPushStyle();
      ofNoFill();
      ofClipWindow(-rO, -rO - mTitleBarHeight, mWidth + rO * 2, mHeight + mTitleBarHeight + rO * 2, false);
      ofSetColor(240, 240, 240);
      ofSetLineWidth(1.0f);
      ofRect(-rO, -rO - mTitleBarHeight, mWidth + rO * 2, mHeight + mTitleBarHeight + rO * 2, rO * 4.05f);
      ofPopStyle();
   }
}

bool HoverSelectModule::MouseMoved(float x, float y)
{
   float mWidth;
   float mHeight;
   GetModuleDimensions(mWidth, mHeight);
   if (x > 0 && y > -+mTitleBarHeight && x < mWidth && y < mHeight + mTitleBarHeight)
   {
      if (!hovered && !TheQuickSpawnMenu->IsShowing())
      {
         if (!IsEnabled())
            return IDrawableModule::MouseMoved(x, y);
         hovered = true;
         OnHoverEnter();
      }
   }
   else
   {
      if (hovered)
      {
         hovered = false;
         OnHoverExit();
      }
   }
   return IDrawableModule::MouseMoved(x, y);
}