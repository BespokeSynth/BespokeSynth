/*
  ==============================================================================

    UIControlMacros.h
    Created: 12 Feb 2020 11:28:22pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

// variable-argument macro technique from https://stackoverflow.com/questions/9183993/msvc-variadic-macro-expansion
#define GLUE(x, y) x y
#define RETURN_ARG_COUNT(_1_, _2_, _3_, _4_, _5_, count, ...) count
#define EXPAND_ARGS(args) RETURN_ARG_COUNT args
#define COUNT_ARGS_MAX5(...) EXPAND_ARGS((__VA_ARGS__, 5, 4, 3, 2, 1, 0))
#define OVERLOAD_MACRO2(name, count) name##count
#define OVERLOAD_MACRO1(name, count) OVERLOAD_MACRO2(name, count)
#define OVERLOAD_MACRO(name, count) OVERLOAD_MACRO1(name, count)
#define CALL_OVERLOAD(name, ...) GLUE(OVERLOAD_MACRO(name, COUNT_ARGS_MAX5(__VA_ARGS__)), (__VA_ARGS__))

#define UIBLOCK0() UIBLOCK3(3,3,100)
#define UIBLOCK1(A) UIBLOCK3(3,3,A)
#define UIBLOCK2(A,B) UIBLOCK3(A,B,100)
#define UIBLOCK3(A,B,C) { float xPos = A; float yPos = B; float originalY = B; float sliderWidth = C; float originalSliderWidth = C; IUIControl* lastUIControl = nullptr; float xMax = 0; float yMax = 0; float xOffset = 0; (void)originalY; (void)originalSliderWidth;
#define UIBLOCK(...) CALL_OVERLOAD(UIBLOCK, __VA_ARGS__)

#define UIBLOCK_SHIFTDOWN() yPos += lastUIControl->GetDimensions().y + 2;
#define UIBLOCK_SHIFTRIGHT() xOffset = lastUIControl->GetPosition(true).x + lastUIControl->GetDimensions().x + 2; yPos = lastUIControl->GetPosition(true).y;
#define UIBLOCK_SHIFTX(amount) xOffset += amount; yPos = lastUIControl->GetPosition(true).y;
#define UIBLOCK_SHIFTY(amount) yPos += amount;
#define UIBLOCK_NEWLINE() xOffset = 0;
#define UIBLOCK_NEWCOLUMN() xPos += sliderWidth + 3; yPos = originalY;
#define UIBLOCK_PUSHSLIDERWIDTH(w) sliderWidth = w;
#define UIBLOCK_POPSLIDERWIDTH() sliderWidth = originalSliderWidth;

#define UIBLOCK_UPDATEEXTENTS() xMax = MAX(xMax, lastUIControl->GetPosition(true).x + lastUIControl->GetDimensions().x); yMax = MAX(yMax, lastUIControl->GetPosition(true).y + lastUIControl->GetDimensions().y);

#define UICONTROLBASICS(name) this,name,xPos+xOffset,yPos

#define FLOATSLIDER(slider,name,var,min,max) slider = new FloatSlider(UICONTROLBASICS(name),sliderWidth,15,var,min,max); lastUIControl = slider; UIBLOCK_SHIFTDOWN(); UIBLOCK_UPDATEEXTENTS();
#define FLOATSLIDER_DIGITS(slider,name,var,min,max,digits) slider = new FloatSlider(UICONTROLBASICS(name),sliderWidth,15,var,min,max,digits); lastUIControl = slider; UIBLOCK_SHIFTDOWN(); UIBLOCK_UPDATEEXTENTS();

#define CHECKBOX(checkbox,name,var) checkbox = new Checkbox(UICONTROLBASICS(name),var); lastUIControl = checkbox; UIBLOCK_SHIFTDOWN(); UIBLOCK_UPDATEEXTENTS();

#define DROPDOWN(dropdown,name,var,width) dropdown = new DropdownList(UICONTROLBASICS(name),var,width); lastUIControl = dropdown; UIBLOCK_SHIFTDOWN(); UIBLOCK_UPDATEEXTENTS();

#define UICONTROLCUSTOM(init) lastUIControl = init; UIBLOCK_UPDATEEXTENTS();

#define UIBLOCKWIDTH() xMax + 3
#define UIBLOCKHEIGHT() yMax + 2

#define ENDUIBLOCK0() }
#define ENDUIBLOCK1(A) A = UIBLOCKHEIGHT(); }
#define ENDUIBLOCK2(A,B) A = UIBLOCKWIDTH(); B = UIBLOCKHEIGHT(); }
#define ENDUIBLOCK(...) CALL_OVERLOAD(ENDUIBLOCK, __VA_ARGS__)
