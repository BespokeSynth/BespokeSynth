#pragma once

#include "juce_opengl/juce_opengl.h"

struct Event{
    static inline const juce::KeyPress& keycode(const void* data, size_t size){return *(const juce::KeyPress*)data;}
};