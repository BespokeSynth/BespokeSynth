/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
/*
  ==============================================================================

    ScriptModule_PythonInterface.i
    Created: 21 May 2020 11:14:51pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "ScriptModule.h"
#include "ModularSynth.h"
#include "IDrawableModule.h"
#include "NoteStepSequencer.h"
#include "StepSequencer.h"
#include "NoteCanvas.h"
#include "SamplePlayer.h"
#include "GridModule.h"
#include "MidiController.h"
#include "LinnstrumentControl.h"
#include "OscController.h"
#include "OSCOutput.h"
#include "EnvelopeModulator.h"
#include "DrumPlayer.h"
#include "VSTPlugin.h"

#include "leathers/push"
#include "leathers/unused-value"
#include "leathers/range-loop-analysis"
   #include "pybind11/embed.h"
   #include "pybind11/stl.h"
#include "leathers/pop"

namespace py = pybind11;
using namespace pybind11::literals;

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
   m.def("reset_transport", []()
   {
      TheTransport->Reset();
   });
   m.def("get_step", [](int subdivision)
   {
      float subdivide = subdivision * ScriptModule::GetTimeSigRatio();
      return int(ScriptModule::GetScriptMeasureTime() * subdivide);
   });
   m.def("count_per_measure", [](int subdivision)
   {
      float subdivide = subdivision * ScriptModule::GetTimeSigRatio();
      return int(subdivide);
   });
   m.def("time_until_subdivision", [](int subdivision)
   {
      float subdivide = subdivision * ScriptModule::GetTimeSigRatio();
      float measureTime = ScriptModule::GetScriptMeasureTime();
      return ceil(measureTime * subdivide + .0001f) / subdivide - measureTime;
   });
   ///example: me.schedule_call(bespoke.time_until_subdivision(1), "on_downbeat()")
   m.def("get_time_sig_ratio", []()
   {
      return ScriptModule::GetTimeSigRatio();
   });
   m.def("get_root", []()
   {
      return TheScale->ScaleRoot();
   });
   m.def("get_scale", []()
   {
      auto scalePitches = TheScale->GetScalePitches().GetPitches();
      if (scalePitches.size() == 0)
      {
         for (int i=0; i<TheScale->GetPitchesPerOctave(); ++i)
            scalePitches.push_back(i);
      }
      return scalePitches;
   });
   m.def("get_scale_range", [](int octave, int count)
   {
      int root = TheScale->ScaleRoot();
      auto scalePitches = TheScale->GetScalePitches().GetPitches();
      if (scalePitches.size() == 0)
      {
         for (int i=0; i<TheScale->GetPitchesPerOctave(); ++i)
            scalePitches.push_back(i);
      }
      size_t numPitches = scalePitches.size();
      std::vector<int> ret(count);
      for (int i=0; i<count; ++i)
         ret[i] = scalePitches[i % numPitches] + TheScale->GetPitchesPerOctave() * (octave + i / numPitches) + root;
      return ret;
   });
   m.def("tone_to_pitch", [](int index)
   {
      return TheScale->GetPitchFromTone(index);
   });
   m.def("name_to_pitch", [](std::string noteName)
   {
      return PitchFromNoteName(noteName);
   });
   m.def("pitch_to_name", [](int pitch)
   {
      return NoteName(pitch);
   });
   m.def("pitch_to_freq", [](float pitch)
   {
      return TheScale->PitchToFreq(pitch);
   });
   m.def("get_tempo", []()
   {
      return TheTransport->GetTempo();
   });
   m.def("set_background_text", [](std::string str, float size, float xPos, float yPos, float red, float green, float blue)
   {
      ScriptModule::sBackgroundTextString = str;
      ScriptModule::sBackgroundTextSize = size;
      ScriptModule::sBackgroundTextPos.set(xPos, yPos);
      ScriptModule::sBackgroundTextColor.set(red * 255, green * 255, blue * 255);
   }, "str"_a, "size"_a=50, "xPos"_a = 150, "yPos"_a = 250, "red"_a = 1, "green"_a = 1, "blue"_a = 1);
   m.def("random", [](int seed, int index)
   {
      return DeterministicRandom(seed, index);
   });
   m.def("get_modules", []()
   {
      std::vector<IDrawableModule*> modules;
      TheSynth->GetAllModules(modules);
      std::vector<std::string> paths;
      for (auto* module : modules)
      {
         if (module)
            paths.push_back(module->Path());
      }
      return paths;
   });
   m.def("location_recall", [](char location)
   {
      TheSynth->GetLocationZoomer()->MoveToLocation(location);
   });
   m.def("location_store", [](char location)
   {
      TheSynth->GetLocationZoomer()->WriteCurrentLocation(location);
   });
}

PYBIND11_EMBEDDED_MODULE(scriptmodule, m)
{
   m.def("get_me", [](int scriptModuleIndex)
   {
      return ScriptModule::sScriptModules[scriptModuleIndex];
   }, py::return_value_policy::reference);
   py::class_<ScriptModule, IDrawableModule>(m, "scriptmodule")
      .def("play_note", [](ScriptModule& module, float pitch, float velocity, float length, float pan, int output_index)
      {
         module.PlayNoteFromScript(pitch, velocity, pan, output_index);
         module.PlayNoteFromScriptAfterDelay(pitch, 0, length, 0, output_index);
      }, "pitch"_a, "velocity"_a, "length"_a=1.0/16.0, "pan"_a = 0, "output_index"_a = 0)
      ///example: me.play_note(60, 127, 1.0/8)
      .def("schedule_note", [](ScriptModule& module, float delay, float pitch, float velocity, float length, float pan, int output_index)
      {
         module.PlayNoteFromScriptAfterDelay(pitch, velocity, delay, pan, output_index);
         module.PlayNoteFromScriptAfterDelay(pitch, 0, delay + length, 0, output_index);
      }, "delay"_a, "pitch"_a, "velocity"_a, "length"_a=1.0/16.0, "pan"_a = 0, "output_index"_a = 0)
      ///example: me.schedule_note(1.0/4, 60, 127, 1.0/8)
      .def("schedule_note_msg", [](ScriptModule& module, float delay, float pitch, float velocity, float pan, int output_index)
      {
         module.PlayNoteFromScriptAfterDelay(pitch, velocity, delay, pan, output_index);
      }, "delay"_a, "pitch"_a, "velocity"_a, "pan"_a = 0, "output_index"_a = 0)
      .def("schedule_call", [](ScriptModule& module, float delay, std::string method)
      {
         module.ScheduleMethod(method, delay);
      })
      ///example: me.schedule_call(1.0/4, "dotask()")
      .def("note_msg", [](ScriptModule& module, float pitch, float velocity, float pan, int output_index)
      {
         module.PlayNoteFromScript(pitch, velocity, pan, output_index);
      }, "pitch"_a, "velocity"_a, "pan"_a = 0, "output_index"_a = 0)
      .def("set", [](ScriptModule& module, std::string path, float value)
      {
         IUIControl* control = module.GetUIControl(path);
         if (control != nullptr)
            module.ScheduleUIControlValue(control, value, 0);
      })
      ///example: me.set("oscillator~pw", .2)
      .def("schedule_set", [](ScriptModule& module, float delay, std::string path, float value)
      {
         IUIControl* control = module.GetUIControl(path);
         if (control != nullptr)
            module.ScheduleUIControlValue(control, value, delay);
      })
      .def("get", [](ScriptModule& module, std::string path)
      {
         IUIControl* control = module.GetUIControl(path);
         if (control != nullptr)
            return control->GetValue();
         return 0.0f;
      })
      ///example: pulsewidth = me.get("oscillator~pulsewidth")
      .def("get_path_prefix", [](ScriptModule& module)
      {
         std::string path = module.Path();
         if (ofIsStringInString(path, "~"))
         {
            return path.substr(0, path.rfind('~') + 1);
         }
         else 
         {
            return std::string("");
         }
      })
      .def("adjust", [](ScriptModule& module, std::string path, float amount)
      {
         IUIControl* control = module.GetUIControl(path);
         if (control != nullptr)
         {
            float min, max;
            control->GetRange(min, max);
            float value = ofClamp(control->GetValue() + amount, min, max);
            module.ScheduleUIControlValue(control, value, 0);
         }
      })
      .def("highlight_line", [](ScriptModule& module, int lineNum, int scriptModuleIndex)
      {
         module.HighlightLine(lineNum, scriptModuleIndex);
      })
      .def("output", [](ScriptModule& module, py::object obj)
      {
         module.PrintText(py::str(obj));
      })
      ///example: me.output("hello world!")
      .def("me", [](ScriptModule& module)
      {
         return &module;
      })
      .def("stop", [](ScriptModule& module)
      {
         return module.Stop();
      })
      .def("get_caller", [](ScriptModule& module)
      ///example: me.get_caller().play_note(60,127)
      {
         return ScriptModule::sPriorExecutedModule;
      })
      .def("set_num_note_outputs", [](ScriptModule& module, int num)
      {
         module.SetNumNoteOutputs(num);
      })
      .def("connect_osc_input", [](ScriptModule& module, int port)
      {
         module.ConnectOscInput(port);
      })
      .def("send_cc", [](ScriptModule& module, int control, int value, int output_index)
      {
         module.SendCCFromScript(control, value, output_index);
      }, "control"_a, "value"_a, "output_index"_a = 0);
}

PYBIND11_EMBEDDED_MODULE(notesequencer, m)
{
   m.def("get", [](std::string path)
   {
      ScriptModule::sMostRecentLineExecutedModule->SetContext();
      auto* ret = dynamic_cast<NoteStepSequencer*>(TheSynth->FindModule(path));
      ScriptModule::sMostRecentLineExecutedModule->OnModuleReferenceBound(ret);
      ScriptModule::sMostRecentLineExecutedModule->ClearContext();
      return ret;
   }, py::return_value_policy::reference);
   py::class_<NoteStepSequencer, IDrawableModule>(m, "notesequencer")
      .def("set_step", [](NoteStepSequencer& seq, int step, int row, int velocity, float length)
      {
         seq.SetStep(step, row, velocity, length);
      }, "step"_a, "row"_a, "velocity"_a = 127, "length"_a = 1.0)
      .def("set_pitch", [](NoteStepSequencer& seq, int step, int pitch, int velocity, float length)
      {
         seq.SetPitch(step, pitch, velocity, length);
      }, "step"_a, "pitch"_a, "velocity"_a = 127, "length"_a = 1.0);
}

PYBIND11_EMBEDDED_MODULE(drumsequencer, m)
{
   m.def("get", [](std::string path)
   {
      ScriptModule::sMostRecentLineExecutedModule->SetContext();
      auto* ret = dynamic_cast<StepSequencer*>(TheSynth->FindModule(path));
      ScriptModule::sMostRecentLineExecutedModule->OnModuleReferenceBound(ret);
      ScriptModule::sMostRecentLineExecutedModule->ClearContext();
      return ret;
   }, py::return_value_policy::reference);
   py::class_<StepSequencer, IDrawableModule>(m, "drumsequencer")
      .def("set", [](StepSequencer& seq, int step, int pitch, int velocity)
      {
         seq.SetStep(step, pitch, velocity);
      })
      .def("get", [](StepSequencer& seq, int step, int pitch)
      {
         return seq.GetStep(step, pitch);
      });
}

PYBIND11_EMBEDDED_MODULE(grid, m)
{
   m.def("get", [](std::string path)
   {
      ScriptModule::sMostRecentLineExecutedModule->SetContext();
      auto* ret = dynamic_cast<GridModule*>(TheSynth->FindModule(path));
      ScriptModule::sMostRecentLineExecutedModule->OnModuleReferenceBound(ret);
      ScriptModule::sMostRecentLineExecutedModule->ClearContext();
      return ret;
   }, py::return_value_policy::reference);
   ///example: g = grid.get("grid")  #assuming there's a grid called "grid" somewhere in the layout
   py::class_<GridModule, IDrawableModule>(m, "grid")
      .def("set", [](GridModule& grid, int col, int row, float value)
      {
         grid.Set(col, row, value);
      })
      .def("get", [](GridModule& grid, int col, int row)
      {
         return grid.Get(col, row);
      })
      .def("set_grid", [](GridModule& grid, int cols, int rows)
      {
         grid.SetGrid(cols, rows);
      })
      .def("set_label", [](GridModule& grid, int row, std::string label)
      {
         grid.SetLabel(row, label);
      })
      .def("set_color", [](GridModule& grid, int colorIndex, float r, float g, float b)
      {
         grid.SetColor(colorIndex, ofColor(r*255,g*255,b*255));
      })
      .def("highlight_cell", [](GridModule& grid, int col, int row, double delay, double duration, int colorIndex)
      {
         double startTime = ScriptModule::sMostRecentLineExecutedModule->GetScheduledTime(delay);
         double endTime = ScriptModule::sMostRecentLineExecutedModule->GetScheduledTime(delay + duration);
         grid.HighlightCell(col, row, startTime, endTime - startTime, colorIndex);
      }, "col"_a, "row"_a, "delay"_a, "duration"_a, "colorIndex"_a=1)
      .def("set_division", [](GridModule& grid, int division)
      {
         grid.SetDivision(division);
      })
      .def("set_momentary", [](GridModule& grid, bool momentary)
      {
         grid.SetMomentary(momentary);
      })
      .def("set_cell_color", [](GridModule& grid, int col, int row, int colorIndex)
      {
         grid.SetCellColor(col, row, colorIndex);
      })
      .def("get_cell_color", [](GridModule& grid, int col, int row)
      {
         return grid.GetCellColor(col, row);
      })
      .def("add_listener", [](GridModule& grid, ScriptModule* script)
      {
         grid.AddListener(script);
      })
      .def("clear", [](GridModule& grid)
      {
         grid.Clear();
      });
}

PYBIND11_EMBEDDED_MODULE(notecanvas, m)
{
   m.def("get", [](std::string path)
   {
      ScriptModule::sMostRecentLineExecutedModule->SetContext();
      auto* ret = dynamic_cast<NoteCanvas*>(TheSynth->FindModule(path));
      ScriptModule::sMostRecentLineExecutedModule->OnModuleReferenceBound(ret);
      ScriptModule::sMostRecentLineExecutedModule->ClearContext();
      return ret;
   }, py::return_value_policy::reference);
   py::class_<NoteCanvas, IDrawableModule>(m, "notecanvas")
      .def("add_note", [](NoteCanvas& canvas, double measurePos, int pitch, int velocity, double length)
      {
         canvas.AddNote(measurePos, pitch, velocity, length);
      })
      .def("clear", [](NoteCanvas& canvas)
      {
         canvas.Clear(NextBufferTime(false));
      })
      .def("fit", [](NoteCanvas& canvas)
      {
         canvas.FitNotes();
      });
}

PYBIND11_EMBEDDED_MODULE(sampleplayer, m)
{
   m.def("get", [](std::string path)
   {
      ScriptModule::sMostRecentLineExecutedModule->SetContext();
      auto* ret = dynamic_cast<SamplePlayer*>(TheSynth->FindModule(path));
      ScriptModule::sMostRecentLineExecutedModule->OnModuleReferenceBound(ret);
      ScriptModule::sMostRecentLineExecutedModule->ClearContext();
      return ret;
   }, py::return_value_policy::reference);
   py::class_<SamplePlayer, IDrawableModule>(m, "sampleplayer")
      .def("set_cue_point", [](SamplePlayer& player, int pitch, float startSeconds, float lengthSeconds, float speed)
      {
         player.SetCuePoint(pitch, startSeconds, lengthSeconds, speed);
      })
      .def("fill", [](SamplePlayer& player, std::vector<float> data)
      {
         player.FillData(data);
      })
      .def("play_cue", [](SamplePlayer& player, int cue, float speedMult, float startOffsetSeconds)
      {
         ScriptModule* scriptModule = ScriptModule::sMostRecentLineExecutedModule;
         double time = scriptModule->GetScheduledTime(0);
         ModulationParameters modulation;
         modulation.pitchBend = scriptModule->GetPitchBend(cue);
         modulation.pitchBend->SetValue(log2(speedMult));
         modulation.modWheel = scriptModule->GetModWheel(cue);
         modulation.modWheel->SetValue(startOffsetSeconds);
         player.PlayNote(NoteMessage(time, cue, 127, -1, modulation));
      }, "cue"_a, "speedMult"_a = 1, "startOffsetSeconds"_a = 0)
      .def("get_length_seconds", [](SamplePlayer& player)
      {
         return player.GetLengthInSeconds();
      });
}

PYBIND11_EMBEDDED_MODULE(midicontroller, m)
{
   m.def("get", [](std::string path)
   {
      ScriptModule::sMostRecentLineExecutedModule->SetContext();
      auto* ret = dynamic_cast<MidiController*>(TheSynth->FindModule(path));
      ScriptModule::sMostRecentLineExecutedModule->OnModuleReferenceBound(ret);
      ScriptModule::sMostRecentLineExecutedModule->ClearContext();
      return ret;
   }, py::return_value_policy::reference);
   py::class_<MidiController, IDrawableModule> midiControllerClass(m, "midicontroller");
   py::enum_<MidiMessageType>(midiControllerClass, "MidiMessageType")
      .value("Note", MidiMessageType::kMidiMessage_Note)
      .value("Control", MidiMessageType::kMidiMessage_Control)
      .value("Program", MidiMessageType::kMidiMessage_Program)
      .value("PitchBend", MidiMessageType::kMidiMessage_PitchBend)
      .export_values();
   py::enum_<ControlType>(midiControllerClass, "ControlType")
      .value("Slider", ControlType::kControlType_Slider)
      .value("SetValue", ControlType::kControlType_SetValue)
      .value("Toggle", ControlType::kControlType_Toggle)
      .value("Direct", ControlType::kControlType_Direct)
      .value("SetValueOnRelease", ControlType::kControlType_SetValueOnRelease)
      .value("Default", ControlType::kControlType_Default)
      .export_values();
   midiControllerClass
      .def("set_connection", [](MidiController& midicontroller, MidiMessageType messageType, int control, std::string controlPath, ControlType controlType, int value, int channel, int page, int midi_off, int midi_on, bool scale, bool blink, float increment, bool twoway, int feedbackControl, bool isPageless)
      {
         ScriptModule::sMostRecentLineExecutedModule->SetContext();
         IUIControl* uicontrol = TheSynth->FindUIControl(controlPath.c_str());
         if (uicontrol != nullptr)
         {
            auto* connection = midicontroller.AddControlConnection(messageType, control, channel, uicontrol, page);
            if (controlType != kControlType_Default)
               connection->mType = controlType;
            connection->mValue = value;
            connection->mMidiOffValue = midi_off;
            connection->mMidiOnValue = midi_on;
            connection->mScaleOutput = scale;
            connection->mBlink = blink;
            connection->mIncrementAmount = increment;
            connection->mTwoWay = twoway;
            connection->mFeedbackControl = feedbackControl;
            connection->mPageless = isPageless;
         }
         ScriptModule::sMostRecentLineExecutedModule->ClearContext();
      }, "messageType"_a, "control"_a, "controlPath"_a, "controlType"_a = kControlType_Default, "value"_a = 0, "channel"_a = -1, "page"_a=0, "midi_off"_a=0, "midi_on"_a = 127, "scale"_a = false, "blink"_a = false, "increment"_a = 0, "twoway"_a = true, "feedbackControl"_a = -1, "isPageless"_a = false)
      ///example: m.set_connection(m.Control, 32, "oscillator~pw"), or m.set_connection(m.Note, 10, "oscillator~osc", m.SetValue, 2)
      .def("send_note", [](MidiController& midicontroller, int pitch, int velocity, bool forceNoteOn, int channel, int page)
      {
         midicontroller.SendNote(page, pitch, velocity, forceNoteOn, channel);
      }, "pitch"_a, "velocity"_a, "forceNoteOn"_a = false, "channel"_a = -1, "page"_a = 0)
      .def("send_cc", [](MidiController& midicontroller, int ctl, int value, int channel, int page)
      {
         midicontroller.SendCC(page, ctl, value, channel);
      }, "ctl"_a, "value"_a, "channel"_a = -1, "page"_a = 0)
      .def("send_program_change", [](MidiController& midicontroller, int program, int channel, int page)
      {
         midicontroller.SendProgramChange(page, program, channel);
      }, "program"_a, "channel"_a = -1, "page"_a = 0)
      .def("send_pitchbend", [](MidiController& midicontroller, int bend, int channel, int page)
      {
         midicontroller.SendPitchBend(page, bend, channel);
      }, "bend"_a, "channel"_a = -1, "page"_a = 0)
      .def("send_data", [](MidiController& midicontroller, int a, int b, int c, int page)
      {
         midicontroller.SendData(page, a, b, c);
      }, "a"_a, "b"_a, "c"_a, "page"_a = 0)
      .def("send_sysex", [](MidiController& midicontroller, std::string data, int page)
      {
         midicontroller.SendSysEx(page, data);
      }, "data"_a, "page"_a = 0)
      ///description: Sends a system exclusive message. The given data will be wrapped with header and tail bytes of 0xf0 and 0xf7. The example enables Programmer-Mode on a Launchpad X. 
      ///example: m.send_sysex(bytes([0, 32, 41, 2, 12, 14, 1]) 
      .def("add_script_listener", [](MidiController& midicontroller, ScriptModule* script)
      {
         midicontroller.AddScriptListener(script);
      })
      .def("resync_controller_state", [](MidiController& midicontroller)
      {
         midicontroller.ResyncControllerState();
      });
}

PYBIND11_EMBEDDED_MODULE(linnstrument, m)
{
   m.def("get", [](std::string path)
   {
      ScriptModule::sMostRecentLineExecutedModule->SetContext();
      auto* ret = dynamic_cast<LinnstrumentControl*>(TheSynth->FindModule(path));
      ScriptModule::sMostRecentLineExecutedModule->OnModuleReferenceBound(ret);
      ScriptModule::sMostRecentLineExecutedModule->ClearContext();
      return ret;
   }, py::return_value_policy::reference);
   py::class_<LinnstrumentControl, IDrawableModule> linnClass(m, "linnstrumentcontrol");

   linnClass.def("set_color", [](LinnstrumentControl& linnstrument, int x, int y, LinnstrumentControl::LinnstrumentColor color)
      {
         linnstrument.SetGridColor(x, y, color);
      });
   py::enum_<LinnstrumentControl::LinnstrumentColor>(linnClass, "LinnstrumentColor")
      .value("Off", LinnstrumentControl::LinnstrumentColor::kLinnColor_Off)
      .value("Red", LinnstrumentControl::LinnstrumentColor::kLinnColor_Red)
      .value("Yellow", LinnstrumentControl::LinnstrumentColor::kLinnColor_Yellow)
      .value("Green", LinnstrumentControl::LinnstrumentColor::kLinnColor_Green)
      .value("Cyan", LinnstrumentControl::LinnstrumentColor::kLinnColor_Cyan)
      .value("Blue", LinnstrumentControl::LinnstrumentColor::kLinnColor_Blue)
      .value("Magenta", LinnstrumentControl::LinnstrumentColor::kLinnColor_Magenta)
      .value("Black", LinnstrumentControl::LinnstrumentColor::kLinnColor_Black)
      .value("White", LinnstrumentControl::LinnstrumentColor::kLinnColor_White)
      .value("Orange", LinnstrumentControl::LinnstrumentColor::kLinnColor_Orange)
      .value("Lime", LinnstrumentControl::LinnstrumentColor::kLinnColor_Lime)
      .value("Pink", LinnstrumentControl::LinnstrumentColor::kLinnColor_Pink)
      .export_values();
}

PYBIND11_EMBEDDED_MODULE(osccontroller, m)
{
   m.def("get", [](std::string path)
   {
      ScriptModule::sMostRecentLineExecutedModule->SetContext();
      MidiController* midicontroller = dynamic_cast<MidiController*>(TheSynth->FindModule(path));
      if (midicontroller)
      {
         auto* ret = dynamic_cast<OscController*>(midicontroller->GetNonstandardController());
         if (ret != nullptr)
            ScriptModule::sMostRecentLineExecutedModule->OnModuleReferenceBound(midicontroller);
         ScriptModule::sMostRecentLineExecutedModule->ClearContext();
         return ret;
      }
      else
      {
         ScriptModule::sMostRecentLineExecutedModule->ClearContext();
         return (OscController*)nullptr;
      }
   }, py::return_value_policy::reference);
   py::class_<OscController>(m, "osccontroller")
      .def("add_control", [](OscController& osccontroller, std::string address, bool isFloat)
      {
         osccontroller.AddControl(address, isFloat);
      });
}

PYBIND11_EMBEDDED_MODULE(oscoutput, m)
{
   m.def("get", [](std::string path)
   {
      ScriptModule::sMostRecentLineExecutedModule->SetContext();
      auto* ret = dynamic_cast<OSCOutput*>(TheSynth->FindModule(path));
      ScriptModule::sMostRecentLineExecutedModule->OnModuleReferenceBound(ret);
      ScriptModule::sMostRecentLineExecutedModule->ClearContext();
      return ret;
   }, py::return_value_policy::reference);
   py::class_<OSCOutput, IDrawableModule>(m, "oscoutput")
      .def("send_float", [](OSCOutput& oscoutput, std::string address, float val)
      {
         oscoutput.SendFloat(address, val);
      })
      .def("send_int", [](OSCOutput& oscoutput, std::string address, int val)
      {
         oscoutput.SendInt(address, val);
      })
      .def("send_string", [](OSCOutput& oscoutput, std::string address, std::string val)
      {
         oscoutput.SendString(address, val);
      });
}

namespace
{
   void StartEnvelope(EnvelopeModulator& envelope, double time, const std::vector< std::tuple<float,float> >& stages)
   {
      ::ADSR adsr;
      adsr.SetNumStages((int)stages.size());
      adsr.GetHasSustainStage() = false;
      for (int i=0; i<adsr.GetNumStages() && i<(int)stages.size(); ++i)
      {
         adsr.GetStageData(i).time = std::get<0>(stages[i]);
         adsr.GetStageData(i).target = std::get<1>(stages[i]);
      }
      envelope.Start(time, adsr);
   }
}

PYBIND11_EMBEDDED_MODULE(envelope, m)
{
   m.def("get", [](std::string path)
   {
      ScriptModule::sMostRecentLineExecutedModule->SetContext();
      auto* ret = dynamic_cast<EnvelopeModulator*>(TheSynth->FindModule(path));
      ScriptModule::sMostRecentLineExecutedModule->OnModuleReferenceBound(ret);
      ScriptModule::sMostRecentLineExecutedModule->ClearContext();
      return ret;
   }, py::return_value_policy::reference);
   py::class_<EnvelopeModulator, IDrawableModule>(m, "envelope")
      .def("start", [](EnvelopeModulator& envelope, std::vector< std::tuple<float,float> > stages)
      {
         double time = ScriptModule::sMostRecentLineExecutedModule->GetScheduledTime(0);
         StartEnvelope(envelope, time, stages);
      })
      .def("schedule", [](EnvelopeModulator& envelope, float delay, std::vector< std::tuple<float,float> > stages)
      {
         double time = ScriptModule::sMostRecentLineExecutedModule->GetScheduledTime(delay);
         StartEnvelope(envelope, time, stages);
      });
}

PYBIND11_EMBEDDED_MODULE(drumplayer, m)
{
   m.def("get", [](std::string path)
   {
      ScriptModule::sMostRecentLineExecutedModule->SetContext();
      auto* ret = dynamic_cast<DrumPlayer*>(TheSynth->FindModule(path));
      ScriptModule::sMostRecentLineExecutedModule->OnModuleReferenceBound(ret);
      ScriptModule::sMostRecentLineExecutedModule->ClearContext();
      return ret;
   }, py::return_value_policy::reference);
   py::class_<DrumPlayer, IDrawableModule>(m, "drumplayer")
      .def("import_sampleplayer_cue", [](DrumPlayer& drumPlayer, SamplePlayer* samplePlayer, int srcCueIndex, int destHitIndex)
      {
         drumPlayer.ImportSampleCuePoint(samplePlayer, srcCueIndex, destHitIndex);
      });
}

PYBIND11_EMBEDDED_MODULE(vstplugin, m)
{
   m.def("get", [](std::string path)
   {
      ScriptModule::sMostRecentLineExecutedModule->SetContext();
      auto* ret = dynamic_cast<VSTPlugin*>(TheSynth->FindModule(path));
      ScriptModule::sMostRecentLineExecutedModule->OnModuleReferenceBound(ret);
      ScriptModule::sMostRecentLineExecutedModule->ClearContext();
      return ret;
   }, py::return_value_policy::reference);
   py::class_<VSTPlugin, IDrawableModule> vstpluginClass(m, "vstplugin");
   vstpluginClass
      .def("send_cc", [](VSTPlugin& vstplugin, int ctl, int value, int channel)
   {
      vstplugin.SendMidi(juce::MidiMessage::controllerEvent(std::clamp(channel, 1, 16), ctl, value));
   })
      .def("send_program_change", [](VSTPlugin& vstplugin, int program, int channel)
   {
      vstplugin.SendMidi(juce::MidiMessage::programChange(std::clamp(channel, 1, 16), program));
   })
      .def("send_data", [](VSTPlugin& vstplugin, int a, int b, int c)
   {
      vstplugin.SendMidi(juce::MidiMessage(a, b, c));
   });
}

PYBIND11_EMBEDDED_MODULE(module, m)
{
   m.def("get", [](std::string path)
   {
      ScriptModule::sMostRecentLineExecutedModule->SetContext();
      auto* ret = TheSynth->FindModule(path);
      ScriptModule::sMostRecentLineExecutedModule->OnModuleReferenceBound(ret);
      ScriptModule::sMostRecentLineExecutedModule->ClearContext();
      return ret;
   }, py::return_value_policy::reference);
   m.def("create", [](std::string moduleType, float x, float y)
   {
      ModuleFactory::Spawnable spawnable;
      spawnable.mLabel = moduleType;
      return TheSynth->SpawnModuleOnTheFly(spawnable, x, y);
   }, py::return_value_policy::reference);
   py::class_<IDrawableModule>(m, "module")
      .def("set_position", [](IDrawableModule& module, float x, float y)
      {
         module.SetPosition(x,y);
      })
      .def("get_position_x", [](IDrawableModule& module)
      {
         return module.GetPosition().x;
      })
      .def("get_position_y", [](IDrawableModule& module)
      {
         return module.GetPosition().y;
      })
      .def("get_width", [](IDrawableModule& module)
      {
         float w, h;
         module.GetDimensions(w, h);
         return w;
      })
      .def("get_height", [](IDrawableModule& module)
      {
         float w, h;
         module.GetDimensions(w, h);
         return h;
      })
      .def("set_target", [](IDrawableModule& module, IDrawableModule* target)
      {
         module.SetTarget(target);
      })
      .def("set_target", [](IDrawableModule& module, std::string targetPath)
      {
         IClickable* target = TheSynth->FindModule(targetPath);
         if (target == nullptr)
            target = TheSynth->FindUIControl(targetPath);
         module.SetTarget(target);
      })
      .def("set_target", [](IDrawableModule& module, int cableSourceIndex, std::string targetPath)
      {
         IClickable* target = TheSynth->FindModule(targetPath);
         if (target == nullptr)
            target = TheSynth->FindUIControl(targetPath);
         const auto cableSource = module.GetPatchCableSource(cableSourceIndex);
         if (cableSource)
            cableSource->SetTarget(target);
      })
      .def("get_target", [](IDrawableModule& module)
      {
         auto* cable = module.GetPatchCableSource();
         if (cable && cable->GetTarget())
            return cable->GetTarget()->Path();
         return std::string();
      })
      .def("get_targets", [](IDrawableModule& module)
      {
         std::vector<std::string> ret;
         for (auto* source : module.GetPatchCableSources())
         {
            if (source == nullptr)
               continue;

            for (auto* cable : source->GetPatchCables())
            {
               if (cable != nullptr && cable->GetTarget() != nullptr)
                  ret.push_back(cable->GetTarget()->Path());
            }
         }
         return ret;
      })
      .def("set_name", [](IDrawableModule& module, std::string name)
      {
         module.SetName(name.c_str());
      })
      .def("get_name", [](IDrawableModule& module)
      {
         return module.Name();
      })
      .def("delete", [](IDrawableModule& module)
      {
         module.GetOwningContainer()->DeleteModule(&module, !K(fail));
      })
      .def("set", [](IDrawableModule& module, std::string path, float value)
      {
         ScriptModule::sMostRecentLineExecutedModule->SetContext();
         IUIControl* control = module.FindUIControl(path.c_str(), false);
         ScriptModule::sMostRecentLineExecutedModule->ClearContext();
         if (control != nullptr)
         {
            control->SetValue(value, ScriptModule::sMostRecentRunTime);
         }
      })
      .def("get", [](IDrawableModule& module, std::string path)
      {
         ScriptModule::sMostRecentLineExecutedModule->SetContext();
         IUIControl* control = module.FindUIControl(path.c_str(), false);
         ScriptModule::sMostRecentLineExecutedModule->ClearContext();
         if (control != nullptr)
            return control->GetValue();
         return 0.0f;
      })
      .def("adjust", [](IDrawableModule& module, std::string path, float amount)
      {
         ScriptModule::sMostRecentLineExecutedModule->SetContext();
         IUIControl* control = module.FindUIControl(path.c_str(), false);
         ScriptModule::sMostRecentLineExecutedModule->ClearContext();
         if (control != nullptr)
         {
            float min, max;
            control->GetRange(min, max);
            float value = ofClamp(control->GetValue() + amount, min, max);
            control->SetValue(value, ScriptModule::sMostRecentRunTime);
         }
      })
      .def("set_focus", [](IDrawableModule& module, float zoom)
      {
         ofRectangle module_rect = module.GetRect();
         if (zoom >= 0.1)
            gDrawScale = ofClamp(zoom, 0.1, 8);
         else if (fabs(zoom) < 0.1) // Close to 0
         {
            //calculate zoom to view the entire module
            float margin = 60;
            float w_ratio = ofGetWidth() / (module_rect.width + margin);
            float h_ratio = ofGetHeight() / (module_rect.height + margin);
            float ratio = (w_ratio < h_ratio) ? w_ratio : h_ratio;
            gDrawScale = ofClamp(ratio, 0.1, 8);
         }
         // Move viewport to centered on the module
         float w, h;
         TheTitleBar->GetDimensions(w, h);
         TheSynth->SetDrawOffset(ofVec2f(-module_rect.x + ofGetWidth() / gDrawScale / 2 - module_rect.width / 2, -module_rect.y + ofGetHeight() / gDrawScale / 2 - (module_rect.height - h / 2) / 2));
      });
}

