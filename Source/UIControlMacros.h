/*
  ==============================================================================

    UIControlMacros.h
    Created: 12 Feb 2020 11:28:22pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

// variable-argument macro technique from https://stackoverflow.com/questions/3046889/optional-parameters-with-c-macros
#define UIBLOCK_0() UIBLOCK_3(3,3,100)
#define UIBLOCK_1(A) UIBLOCK_3(3,3,A)
#define UIBLOCK_2(A,B) UIBLOCK_3(A,B,100)
#define UIBLOCK_3(A,B,C) { float xPos = A; float yPos = B; float originalY = B; float sliderWidth = C; float originalSliderWidth = C; IUIControl* lastUIControl = nullptr; float xMax = 0; float yMax = 0; float xOffset = 0; (void)originalY; (void)originalSliderWidth;
#define UIBLOCK_4(A,B,C,D) error
#define UIBLOCK_X(x,A,B,C,D,FUNC, ...)  FUNC
#define UIBLOCK(...)                    UIBLOCK_X(,##__VA_ARGS__,\
                                          UIBLOCK_4(__VA_ARGS__),\
                                          UIBLOCK_3(__VA_ARGS__),\
                                          UIBLOCK_2(__VA_ARGS__),\
                                          UIBLOCK_1(__VA_ARGS__),\
                                          UIBLOCK_0(__VA_ARGS__)\
                                         )

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

#define ENDUIBLOCK_0() }
#define ENDUIBLOCK_1(A) A = UIBLOCKHEIGHT(); }
#define ENDUIBLOCK_2(A,B) A = UIBLOCKWIDTH(); B = UIBLOCKHEIGHT(); }
#define ENDUIBLOCK_X(x,A,B,FUNC, ...)  FUNC
#define ENDUIBLOCK(...)                    ENDUIBLOCK_X(,##__VA_ARGS__,\
                                          ENDUIBLOCK_2(__VA_ARGS__),\
                                          ENDUIBLOCK_1(__VA_ARGS__),\
                                          ENDUIBLOCK_0(__VA_ARGS__)\
                                         )
