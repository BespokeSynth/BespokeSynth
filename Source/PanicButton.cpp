//
//  PanicButton.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 3/31/14.
//
//

#include "PanicButton.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"

PanicButton::PanicButton()
{
}

PanicButton::~PanicButton()
{
}

void PanicButton::OnClicked(int x, int y, bool right)
{
   TheSynth->LoadLayout(ofToDataPath("daftpunk.json"));
}

void PanicButton::DrawModule()
{
   gFont.DrawString("If anything goes horribly awry, click this\nand the party will be restored",15,5,12);
   
   ofPushStyle();
   ofFill();
   ofSetColor(70,0,0);
   ofRect(10,40,270,80);
   ofSetColor(255,0,0);
   gFont.DrawString("PANIC",108,15,115);
   ofPopStyle();
}