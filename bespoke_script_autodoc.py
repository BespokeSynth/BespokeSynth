import re

def extractParamString(group, skipFirst):
   params = group.split(' ')[1:][::2]
   if skipFirst:
      params = params[1:]
   paramString = " ".join(params)
   return paramString

sourceFile = open("Source/ScriptModule_PythonInterface.i")
lines = sourceFile.readlines()

print"the script module takes python code as input. any python code and imports should work fine. this document describes the specific calls you get from bespoke and can make into bespoke.\n")
print("script module inputs:")
print("   on_pulse(): called by pulse input")
print("   on_note(pitch, velocity): called by note input")
print("   on_grid_button(col, row, velocity): called by grid input")

currentModule = ""

for line in lines:
   if line.startswith("   py::class_<") and not isBootstrapModule:
      print("   instance:")
   
   m = re.match(".*///(.*)",line)
   if m:
      print(tab+"   "+m.group(1))

   m = re.match("PYBIND11_EMBEDDED_MODULE\((.*),", line)
   if m:
      currentModule = m.group(1)
      isBootstrapModule = currentModule == "bespoke" or currentModule == "scriptmodule"
      if currentModule == "scriptmodule":
         currentModule = "this"
      print("\n")
      if isBootstrapModule:
         tab = "   "
         if currentModule == "bespoke":
            print("globals:")
         if currentModule == "this":
            print("script-relative:")
      else:
         tab = "      "
         print("import "+currentModule)
         print("   static:")
   else:
      m = re.match(".*m.def\(\"(.*)\".*\[\]\((.*)\)", line)
      if m:
         paramString = extractParamString(m.group(2), False)
         if (m.group(1) != "get_this"):
            print(tab+currentModule+"."+m.group(1)+"("+paramString+")")
      else:
         m = re.match(".*.def\(\"(.*)\".*\[\]\((.*)\)", line)
         if m:
            paramString = extractParamString(m.group(2), True)
            if isBootstrapModule:
               instanceName = currentModule
            else:
               instanceName = currentModule[0]
            print(tab+instanceName+"."+m.group(1)+"("+paramString+")")
            #print(tab+example: "+currentModule+"."+m.group(1)+"("+paramString+")")
         else:
            m = re.match(" *}, (\".*)\)", line)
            if m:
               defaults = m.group(1).split(',')
               defaultList = []
               for default in defaults:
                  m = re.match(".*\"(.*)\"_a(.*)", default)
                  if m and m.group(2) != "":
                     defaultList.append(m.group(1)+m.group(2))
               if defaultList != []:
                  print (tab+"   optional: "+", ".join(defaultList))
