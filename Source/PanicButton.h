//
//  PanicButton.h
//  Bespoke
//
//  Created by Ryan Challinor on 3/31/14.
//
//

#ifndef __Bespoke__PanicButton__
#define __Bespoke__PanicButton__

#include <iostream>
#include "IDrawableModule.h"
#include "OpenFrameworksPort.h"

class PanicButton : public IDrawableModule
{
public:
   PanicButton();
   ~PanicButton();
   static IDrawableModule* Create() { return new PanicButton(); }
   
private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& width, float& height) override { width=300; height=150; }
   void OnClicked(int x, int y, bool right) override;
};

#endif /* defined(__Bespoke__PanicButton__) */
