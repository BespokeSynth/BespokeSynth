/*
  ==============================================================================

    ScriptModule_PythonInterface.i
    Created: 21 May 2020 11:14:51pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "ScriptModule.h"

#include "pybind11/embed.h"
#include "pybind11/stl.h"

namespace py = pybind11;

PYBIND11_EMBEDDED_MODULE(bespoke, m) {
   // `m` is a `py::module` which is used to bind functions and classes
   m.def("get_measure_time", []()
   {
      return ScriptModule::GetScriptMeasureTime();
   });
   m.def("get_measure", []()
   {
      return (int)ScriptModule::GetScriptMeasureTime();
   });
   m.def("get_step", [](int subdivision)
   {
      float subdivide = subdivision * (float(TheTransport->GetTimeSigTop()) / TheTransport->GetTimeSigBottom());
      return int(ScriptModule::GetScriptMeasureTime() * subdivide);
   });
   m.def("count_per_measure", [](int subdivision)
   {
      float subdivide = subdivision * (float(TheTransport->GetTimeSigTop()) / TheTransport->GetTimeSigBottom());
      return int(subdivide);
   });
   m.def("time_until_subdivision", [](int subdivision)
   {
      float subdivide = subdivision * (float(TheTransport->GetTimeSigTop()) / TheTransport->GetTimeSigBottom());
      float measureTime = ScriptModule::GetScriptMeasureTime();
      return ceil(measureTime * subdivide + .0001f) / subdivide - measureTime;
   });
   m.def("get_root", []()
   {
      return TheScale->ScaleRoot();
   });
   m.def("get_scale", []()
   {
      return TheScale->GetScalePitches().mScalePitches;
   });
   m.def("tone_to_pitch", [](int index)
   {
      return TheScale->GetPitchFromTone(index);
   });
}

PYBIND11_EMBEDDED_MODULE(scriptmodule, m)
{
   m.def("get_this", [](int scriptModuleIndex)
   {
      return ScriptModule::sScriptModules[scriptModuleIndex];
   }, py::return_value_policy::reference);
   py::class_<ScriptModule>(m, "scriptmodule")
      .def("play_note", [](ScriptModule &module, int pitch, int velocity, float length)
      {
         module.PlayNoteFromScript(pitch, velocity, 0);
         module.PlayNoteFromScriptAfterDelay(pitch, 0, length - .01f, 0);
      })
      .def("play_note_pan", [](ScriptModule &module, int pitch, int velocity, float length, float pan)
      {
         module.PlayNoteFromScript(pitch, velocity, pan);
         module.PlayNoteFromScriptAfterDelay(pitch, 0, length - .01f, 0);
      })
      .def("schedule_note", [](ScriptModule &module, float delay, int pitch, int velocity, float length)
      {
         module.PlayNoteFromScriptAfterDelay(pitch, velocity, delay, 0);
         module.PlayNoteFromScriptAfterDelay(pitch, 0, delay + length - .01f, 0);
      })
      .def("schedule_note_pan", [](ScriptModule &module, float delay, int pitch, int velocity, float length, float pan)
      {
         module.PlayNoteFromScriptAfterDelay(pitch, velocity, delay, pan);
         module.PlayNoteFromScriptAfterDelay(pitch, 0, delay + length - .01f, 0);
      })
      .def("schedule_note_on", [](ScriptModule &module, float delay, int pitch, int velocity)
      {
         module.PlayNoteFromScriptAfterDelay(pitch, velocity, delay, 0);
      })
      .def("schedule_call", [](ScriptModule &module, float delay, string method)
      {
         module.ScheduleMethod(method, delay);
      })
      .def("note_on", [](ScriptModule &module, int pitch, int velocity)
      {
         module.PlayNoteFromScript(pitch, velocity, 0);
      })
      .def("note_off", [](ScriptModule &module, int pitch)
      {
         module.PlayNoteFromScript(pitch, 0, 0);
      })
      .def("set", [](ScriptModule &module, string path, float value)
      {
         IUIControl* control = module.GetUIControl(path);
         if (control != nullptr)
         {
            control->SetValue(value);
            module.OnAdjustUIControl(control, value);
         }
      })
      .def("get", [](ScriptModule &module, string path)
      {
         IUIControl* control = module.GetUIControl(path);
         if (control != nullptr)
            return control->GetValue();
         return 0.0f;
      })
      .def("adjust", [](ScriptModule &module, string path, float amount)
      {
         IUIControl* control = module.GetUIControl(path);
         if (control != nullptr)
         {
            float min, max;
            control->GetRange(min, max);
            float value = ofClamp(control->GetValue() + amount, min, max);
            control->SetValue(value);
            module.OnAdjustUIControl(control, value);
         }
      })
      .def("highlight_line", [](ScriptModule& module, int lineNum)
      {
         module.HighlightLine(lineNum);
      })
      .def("output", [](ScriptModule& module, string text)
      {
         module.PrintText(text);
      });
}
