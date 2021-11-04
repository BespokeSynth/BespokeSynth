import re
import os

def extractParamString(group, skipFirst):
   params = group.split(' ')[1:][::2]
   if skipFirst:
      params = params[1:]
   params = [x for x in params if "<" not in x]
   paramString = " ".join(params)
   return paramString

sourceFile = open("Source/ScriptModule_PythonInterface.i")
lines = sourceFile.readlines()
docOutput = []
stubOutput = {}

def docLine(line):
   docOutput.append(line+"\n")

def stubLine(module, line):
   if not module in stubOutput:
      stubOutput[module] = []
   stubOutput[module].append(line+"\n")

def replaceInStubLine(module, lineIndex, fromStr, toStr):
   stubOutput[module][lineIndex] = stubOutput[module][lineIndex].replace(fromStr, toStr)

docLine("the script module takes python code as input. any python code and imports should work fine.\nthis document describes the specific calls you get from bespoke and can make into bespoke.\n")
docLine("if you have jedi installed, python autocomplete is supported.\nthe typical way to install jedi on your system is to enter 'pip install jedi' into your system's terminal.\n")
docLine("script module inputs:")
docLine("   on_pulse(): called by pulse input")
docLine("   on_note(pitch, velocity): called by note input")
docLine("   on_grid_button(col, row, velocity): called by grid input")
docLine("   on_osc(message): called by external osc source, after registering with me.connect_osc_input(port)")
docLine("   on_midi(messageType, control, value, channel): called by midicontroller, after registering with add_script_listener(me.me())")

currentModule = ""
inStaticMethods = True

for line in lines:
   if line.startswith("   py::class_<") and not isBootstrapModule:
      docLine("   instance:")
      inStaticMethods = False
      stubLine(currentModule, "class "+currentModule+":")
   
   m = re.match(".*///(.*)",line)
   if m:
      docLine(tab+"   "+m.group(1))

   m = re.match("PYBIND11_EMBEDDED_MODULE\((.*),", line)
   if m:
      currentModule = m.group(1)
      isBootstrapModule = currentModule == "bespoke" or currentModule == "scriptmodule"
      if currentModule == "scriptmodule":
         currentModule = "me"
      docLine("\n")
      if isBootstrapModule:
         tab = "   "
         if currentModule == "bespoke":
            docLine("globals:")
         if currentModule == "me":
            docLine("script-relative:")
      else:
         tab = "      "
         docLine("import "+currentModule)
         docLine("   static:")
      stubLine(currentModule, "from __future__ import annotations\n")
      inStaticMethods = True
   else:
      m = re.match(".*m.def\(\"(.*)\".*\[\]\((.*)\)", line)
      if m:
         paramString = extractParamString(m.group(2), False)
         if m.group(1) != "get_me":
            docLine(tab+currentModule+"."+m.group(1)+"("+paramString+")")
            if m.group(1) == "get":
               stubLine(currentModule, "def "+m.group(1)+"("+paramString+") -> "+currentModule+":")
            else:
               stubLine(currentModule, "def "+m.group(1)+"("+paramString+"):")
            stubLine(currentModule, "   pass\n")
      else:
         m = re.match(".*.def\(\"(.*)\".*\[\]\((.*)\)", line)
         if m:
            paramString = extractParamString(m.group(2), True)
            if isBootstrapModule:
               instanceName = currentModule
            else:
               instanceName = currentModule[0]
            docLine(tab+instanceName+"."+m.group(1)+"("+paramString+")")
            #docLine(tab+example: "+currentModule+"."+m.group(1)+"("+paramString+")")
            thisPrefix = "this, "
            if isBootstrapModule:
               thisPrefix = ""
            stubLine(currentModule, "   def "+m.group(1)+"("+thisPrefix+paramString+"):")
            stubLine(currentModule, "      pass\n")
         else:
            m = re.match(" *}, (\".*)\)", line)
            if m:
               defaults = m.group(1).split(',')
               defaultList = []
               for default in defaults:
                  m = re.match(".*\"(.*)\"_a(.*)", default)
                  if m and m.group(2) != "":
                     defaultValue = m.group(2)
                     if "_" in defaultValue:
                        defaultValue = re.sub('^ = .*?_', ' = ', defaultValue)
                     defaultList.append(m.group(1)+defaultValue)
                     replaceInStubLine(currentModule, -2, m.group(1), m.group(1)+defaultValue)
               if defaultList != []:
                  docLine(tab+"   optional: "+", ".join(defaultList))
            else:
               m = re.match(".*\.value\(\"(.*)\"", line)
               if m:
                  stubLine(currentModule, "   "+m.group(1)+": ...")
                  #   Note: ...

docFile = open("resource/scripting_reference.txt", "w+")
docFile.writelines(docOutput)

for key in stubOutput.keys():
   dir = "resource/python_stubs/"+key+"/"
   if not os.path.exists(dir):
      os.mkdir(dir)
   stubFile = open(dir+"__init__.pyi", "w+")
   stubFile.writelines(stubOutput[key])