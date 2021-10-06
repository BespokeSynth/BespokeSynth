#pragma once

#include "juce_opengl/juce_opengl.h"


struct Signal;
struct SignalQueue;
struct SignalMatcher;

struct KeyCode{
    enum mode_t{RAISE,SUSTAIN,FALL};

    juce::KeyPress  base;
    mode_t          mode;
};

struct Signal{
    typedef KeyCode keycode_t;
    
    enum class ui_signal_t{};
    enum class type_t{NONE,UNHANDLED,KEYCODE,UI_SIGNAL};

    private:
        type_t _type;
        union{
            keycode_t _keycode;
            ui_signal_t _ui_signal;
        };
    public:

    Signal():_type(type_t::NONE){}
    Signal(const keycode_t& k):_keycode(k),_type(type_t::KEYCODE){};
    Signal(const ui_signal_t& k):_ui_signal(k),_type(type_t::UI_SIGNAL){};

    inline const keycode_t& keycode() const{if(_type!=type_t::KEYCODE)throw "WrongSignalTypeException";return _keycode;}
    inline const ui_signal_t& ui_signal() const{if(_type!=type_t::UI_SIGNAL)throw "WrongSignalTypeException";return _ui_signal;}
    inline auto type() const{return _type;}
};

struct SignalQueue{
    //Emit signal
    //Load matcher on a dependency layer
    //Suppress/Enable dependency layer
};