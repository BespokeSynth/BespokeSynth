#pragma once

#include "juce_opengl/juce_opengl.h"

struct Signal{
    public:
        enum class type_t{NONE,KEYCODE,UI_SIGNAL};
        enum class ui_signal_t{};
    private:
        type_t type;
        union{
            const juce::KeyPress* _keycode;
            const ui_signal_t _ui_signal;
        };
    public:

    Signal():type(type_t::NONE){}
    Signal(const juce::KeyPress& k):_keycode(&k),type(type_t::KEYCODE){};
    Signal(const ui_signal_t& k):_ui_signal(k),type(type_t::UI_SIGNAL){};

    inline const juce::KeyPress& keycode() const{if(type!=type_t::KEYCODE)throw "WrongSignalTypeException";return *_keycode;}
    inline const ui_signal_t& ui_signal() const{if(type!=type_t::UI_SIGNAL)throw "WrongSignalTypeException";return _ui_signal;}
};