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
   m.def("reset_transport", [](float rewind_amount)
   {
      TheTransport->Reset();
   }, "rewind_amount"_a=.001f);
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
      return TheScale->GetScalePitches().mScalePitches;
   });
   m.def("get_scale_range", [](int octave, int count)
   {
      int root = TheScale->ScaleRoot();
      auto scalePitches = TheScale->GetScalePitches().mScalePitches;
      size_t numPitches = scalePitches.size();
      vector<int> ret(count);
      for (int i=0; i<count; ++i)
         ret[i] = scalePitches[i % numPitches] + TheScale->GetTet() * (octave + i / numPitches) + root;
      return ret;
   });
   m.def("tone_to_pitch", [](int index)
   {
      return TheScale->GetPitchFromTone(index);
   });
   m.def("name_to_pitch", [](string noteName)
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
   m.def("set_background_text", [](string text, float size, float red, float green, float blue)
   {
      ScriptModule::sBackgroundTextString = text;
      ScriptModule::sBackgroundTextSize = size;
      ScriptModule::sBackgroundTextColor.set(red * 255, green * 255, blue * 255);
   }, "text"_a, "size"_a=50, "red"_a = 1, "green"_a = 1, "blue"_a = 1);
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
      .def("schedule_call", [](ScriptModule& module, float delay, string method)
      {
         module.ScheduleMethod(method, delay);
      })
      ///example: me.schedule_call(1.0/4, "dotask()")
      .def("note_msg", [](ScriptModule& module, float pitch, float velocity, float pan, int output_index)
      {
         module.PlayNoteFromScript(pitch, velocity, pan, output_index);
      }, "pitch"_a, "velocity"_a, "pan"_a = 0, "output_index"_a = 0)
      .def("set", [](ScriptModule& module, string path, float value)
      {
         IUIControl* control = module.GetUIControl(path);
         if (control != nullptr)
            module.ScheduleUIControlValue(control, value, 0);
      })
      ///example: me.set("oscillator~pw", .2)
      .def("schedule_set", [](ScriptModule& module, float delay, string path, float value)
      {
         IUIControl* control = module.GetUIControl(path);
         if (control != nullptr)
            module.ScheduleUIControlValue(control, value, delay);
      })
      .def("get", [](ScriptModule& module, string path)
      {
         IUIControl* control = module.GetUIControl(path);
         if (control != nullptr)
            return control->GetValue();
         return 0.0f;
      })
      ///example: pulsewidth = me.get("oscillator~pulsewidth")
      .def("adjust", [](ScriptModule& module, string path, float amount)
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
      });
}

PYBIND11_EMBEDDED_MODULE(notesequencer, m)
{
   m.def("get", [](string path)
   {
      auto* ret = dynamic_cast<NoteStepSequencer*>(TheSynth->FindModule(path));
      ScriptModule::sMostRecentLineExecutedModule->OnModuleReferenceBound(ret);
      return ret;
   }, py::return_value_policy::reference);
   py::class_<NoteStepSequencer, IDrawableModule>(m, "notesequencer")
      .def("set_step", [](NoteStepSequencer& seq, int step, int pitch, int velocity, float length)
      {
         seq.SetStep(step, pitch, velocity, length);
      });
}

PYBIND11_EMBEDDED_MODULE(drumsequencer, m)
{
   m.def("get", [](string path)
   {
      auto* ret = dynamic_cast<StepSequencer*>(TheSynth->FindModule(path));
      ScriptModule::sMostRecentLineExecutedModule->OnModuleReferenceBound(ret);
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
   m.def("get", [](string path)
   {
      auto* ret = dynamic_cast<GridModule*>(TheSynth->FindModule(path));
      ScriptModule::sMostRecentLineExecutedModule->OnModuleReferenceBound(ret);
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
      .def("set_label", [](GridModule& grid, int row, string label)
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
   m.def("get", [](string path)
   {
      auto* ret = dynamic_cast<NoteCanvas*>(TheSynth->FindModule(path));
      ScriptModule::sMostRecentLineExecutedModule->OnModuleReferenceBound(ret);
      return ret;
   }, py::return_value_policy::reference);
   py::class_<NoteCanvas, IDrawableModule>(m, "notecanvas")
      .def("add_note", [](NoteCanvas& canvas, double measurePos, int pitch, int velocity, double length)
      {
         canvas.AddNote(measurePos, pitch, velocity, length);
      })
      .def("clear", [](NoteCanvas& canvas)
      {
         canvas.Clear();
      });
}

PYBIND11_EMBEDDED_MODULE(sampleplayer, m)
{
   m.def("get", [](string path)
   {
      auto* ret = dynamic_cast<SamplePlayer*>(TheSynth->FindModule(path));
      ScriptModule::sMostRecentLineExecutedModule->OnModuleReferenceBound(ret);
      return ret;
   }, py::return_value_policy::reference);
   py::class_<SamplePlayer, IDrawableModule>(m, "sampleplayer")
      .def("set_cue_point", [](SamplePlayer& player, int pitch, float startSeconds, float lengthSeconds, float speed)
      {
         player.SetCuePoint(pitch, startSeconds, lengthSeconds, speed);
      })
      .def("fill", [](SamplePlayer& player, vector<float> data)
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
         player.PlayNote(time, cue, 127, -1, modulation);
      }, "cue"_a, "speedMult"_a = 1, "startOffsetSeconds"_a = 0);
}

PYBIND11_EMBEDDED_MODULE(midicontroller, m)
{
   m.def("get", [](string path)
   {
      auto* ret = dynamic_cast<MidiController*>(TheSynth->FindModule(path));
      ScriptModule::sMostRecentLineExecutedModule->OnModuleReferenceBound(ret);
      return ret;
   }, py::return_value_policy::reference);
   py::class_<MidiController, IDrawableModule> midiControllerClass(m, "midicontroller");
   midiControllerClass
      .def("set_connection", [](MidiController& midicontroller, MidiMessageType messageType, int control, int channel, string controlPath)
      {
         IUIControl* uicontrol = TheSynth->FindUIControl(controlPath.c_str());
         if (uicontrol != nullptr)
            midicontroller.AddControlConnection(messageType, control, channel, uicontrol);
      })
      .def("send_note", [](MidiController& midicontroller, int pitch, int velocity, bool forceNoteOn, int channel, int page)
      {
         midicontroller.SendNote(page, pitch, velocity, forceNoteOn, channel);
      }, "pitch"_a, "velocity"_a, "forceNoteOn"_a = false, "channel"_a = -1, "page"_a = 0)
      .def("send_cc", [](MidiController& midicontroller, int ctl, int value, int channel, int page)
      {
         midicontroller.SendCC(page, ctl, value, channel);
      }, "ctl"_a, "value"_a, "channel"_a = -1, "page"_a = 0)
      .def("send_pitchbend", [](MidiController& midicontroller, int bend, int channel, int page)
      {
         midicontroller.SendPitchBend(page, bend, channel);
      }, "bend"_a, "channel"_a = -1, "page"_a = 0)
      .def("send_data", [](MidiController& midicontroller, int a, int b, int c, int page)
      {
         midicontroller.SendData(page, a, b, c);
      }, "a"_a, "b"_a, "c"_a, "page"_a = 0)
      .def("add_script_listener", [](MidiController& midicontroller, ScriptModule* script)
      {
         midicontroller.AddScriptListener(script);
      });
   py::enum_<MidiMessageType>(midiControllerClass, "MidiMessageType")
      .value("Note", MidiMessageType::kMidiMessage_Note)
      .value("Control", MidiMessageType::kMidiMessage_Control)
      .value("Program", MidiMessageType::kMidiMessage_Program)
      .value("PitchBend", MidiMessageType::kMidiMessage_PitchBend)
      .export_values();
}

PYBIND11_EMBEDDED_MODULE(linnstrument, m)
{
   m.def("get", [](string path)
   {
      auto* ret = dynamic_cast<LinnstrumentControl*>(TheSynth->FindModule(path));
      ScriptModule::sMostRecentLineExecutedModule->OnModuleReferenceBound(ret);
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
   m.def("get", [](string path)
   {
      MidiController* midicontroller = dynamic_cast<MidiController*>(TheSynth->FindModule(path));
      if (midicontroller)
      {
         auto* ret = dynamic_cast<OscController*>(midicontroller->GetNonstandardController());
         if (ret != nullptr)
            ScriptModule::sMostRecentLineExecutedModule->OnModuleReferenceBound(midicontroller);
         return ret;
      }
      else
      {
         return (OscController*)nullptr;
      }
   }, py::return_value_policy::reference);
   py::class_<OscController>(m, "osccontroller")
      .def("add_control", [](OscController& osccontroller, string address, bool isFloat)
      {
         osccontroller.AddControl(address, isFloat);
      });
}

PYBIND11_EMBEDDED_MODULE(oscoutput, m)
{
   m.def("get", [](string path)
   {
      auto* ret = dynamic_cast<OSCOutput*>(TheSynth->FindModule(path));
      ScriptModule::sMostRecentLineExecutedModule->OnModuleReferenceBound(ret);
      return ret;
   }, py::return_value_policy::reference);
   py::class_<OSCOutput, IDrawableModule>(m, "oscoutput")
      .def("send_float", [](OSCOutput& oscoutput, string address, float val)
      {
         oscoutput.SendFloat(address, val);
      })
      .def("send_int", [](OSCOutput& oscoutput, string address, int val)
      {
         oscoutput.SendInt(address, val);
      })
      .def("send_string", [](OSCOutput& oscoutput, string address, string val)
      {
         oscoutput.SendString(address, val);
      });
}

namespace
{
   void StartEnvelope(EnvelopeModulator& envelope, double time, const vector< tuple<float,float> >& stages)
   {
      ::ADSR adsr;
      adsr.SetNumStages((int)stages.size());
      adsr.GetHasSustainStage() = false;
      for (size_t i=0; i<stages.size(); ++i)
      {
         adsr.GetStageData((int)i).time = get<0>(stages[i]);
         adsr.GetStageData((int)i).target = get<1>(stages[i]);
      }
      envelope.Start(time, adsr);
   }
}

PYBIND11_EMBEDDED_MODULE(envelope, m)
{
   m.def("get", [](string path)
   {
      auto* ret = dynamic_cast<EnvelopeModulator*>(TheSynth->FindModule(path));
      ScriptModule::sMostRecentLineExecutedModule->OnModuleReferenceBound(ret);
      return ret;
   }, py::return_value_policy::reference);
   py::class_<EnvelopeModulator, IDrawableModule>(m, "envelope")
      .def("start", [](EnvelopeModulator& envelope, vector< tuple<float,float> > stages)
      {
         double time = ScriptModule::sMostRecentLineExecutedModule->GetScheduledTime(0);
         StartEnvelope(envelope, time, stages);
      })
      .def("schedule", [](EnvelopeModulator& envelope, float delay, vector< tuple<float,float> > stages)
      {
         double time = ScriptModule::sMostRecentLineExecutedModule->GetScheduledTime(delay);
         StartEnvelope(envelope, time, stages);
      });
}

PYBIND11_EMBEDDED_MODULE(drumplayer, m)
{
   m.def("get", [](string path)
   {
      auto* ret = dynamic_cast<DrumPlayer*>(TheSynth->FindModule(path));
      ScriptModule::sMostRecentLineExecutedModule->OnModuleReferenceBound(ret);
      return ret;
   }, py::return_value_policy::reference);
   py::class_<DrumPlayer, IDrawableModule>(m, "drumplayer")
      .def("import_sampleplayer_cue", [](DrumPlayer& drumPlayer, SamplePlayer* samplePlayer, int srcCueIndex, int destHitIndex)
      {
         drumPlayer.ImportSampleCuePoint(samplePlayer, srcCueIndex, destHitIndex);
      });
}

PYBIND11_EMBEDDED_MODULE(module, m)
{
   m.def("get", [](string path)
   {
      auto* ret = TheSynth->FindModule(path);
      ScriptModule::sMostRecentLineExecutedModule->OnModuleReferenceBound(ret);
      return ret;
   }, py::return_value_policy::reference);
   m.def("create", [](string moduleType, int x, int y)
   {
      return TheSynth->SpawnModuleOnTheFly(moduleType, x, y);
   }, py::return_value_policy::reference);
   py::class_<IDrawableModule>(m, "module")
      .def("set_position", [](IDrawableModule& module, int x, int y)
      {
         module.SetPosition(x,y);
      })
      .def("set_target", [](IDrawableModule& module, IDrawableModule* target)
      {
         module.SetTarget(target);
      })
      .def("delete", [](IDrawableModule& module)
      {
         module.GetOwningContainer()->DeleteModule(&module);
      })
      .def("set", [](IDrawableModule& module, string path, float value)
      {
         IUIControl* control = module.FindUIControl(path.c_str(), false);
         if (control != nullptr)
         {
            control->SetValue(value);
         }
      })
      .def("get", [](IDrawableModule& module, string path)
      {
         IUIControl* control = module.FindUIControl(path.c_str(), false);
         if (control != nullptr)
            return control->GetValue();
         return 0.0f;
      })
      .def("adjust", [](IDrawableModule& module, string path, float amount)
      {
         IUIControl* control = module.FindUIControl(path.c_str(), false);
         if (control != nullptr)
         {
            float min, max;
            control->GetRange(min, max);
            float value = ofClamp(control->GetValue() + amount, min, max);
            control->SetValue(value);
         }
      });
}

