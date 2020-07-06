/*
  ==============================================================================

    QuickSpawnMenu.h
    Created: 22 Oct 2017 7:49:16pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"

class QuickSpawnMenu : public IDrawableModule
{
public:
   QuickSpawnMenu();
   virtual ~QuickSpawnMenu();
   
   void Init() override;
   void DrawModule() override;
   void SetDimensions(int w, int h) { mWidth = w; mHeight = h; }
   bool HasTitleBar() const override { return false; }
   string GetTitleLabel() override { return ""; }
   bool IsSaveable() override { return false; }
   
   void KeyPressed(int key, bool isRepeat) override;
   void KeyReleased(int key) override;
   void MouseReleased() override;
   
   bool IsSingleton() const override { return true; }
   
private:
   void OnClicked(int x, int y, bool right) override;
   void GetDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   int mWidth;
   int mHeight;
   std::vector<string> mElements;
   char mCurrentMenuChar;
};

extern QuickSpawnMenu* TheQuickSpawnMenu;
