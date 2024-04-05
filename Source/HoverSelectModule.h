#pragma once
#include "IDrawableModule.h"

class HoverSelectModule : public IDrawableModule
{
public:
   void DrawHoverSelect();
   bool hovered;
   virtual void OnHoverEnter() = 0;
   virtual void OnHoverExit() = 0;
   virtual void OnSelect() = 0;
   virtual void OnDeselect() = 0;
protected:
   bool MouseMoved(float x, float y) override;
};
