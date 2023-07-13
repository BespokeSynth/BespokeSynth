import json

before = '''<!DOCTYPE html>
<html lang="en">

   <head>
      <title>Bespoke Synth Reference</title>
      <link href="../css/accordion.css" rel="stylesheet">
   </head>

   <body id="page-top" data-spy="scroll" data-target=".navbar-fixed-top">
      <div class="container">'''

after = '''
      </div>
   </body>
</html>'''

def getAcceptsInputsString(moduleInfo):
   ret = ""
   if "canReceivePulses" in moduleInfo:
      if moduleInfo["canReceivePulses"]:
         ret += "<font color=yellow>pulses</font> "
   if "canReceiveNote" in moduleInfo:
      if moduleInfo["canReceiveNote"]:
         ret += "<font color=orange>notes</font> "
   if "canReceiveAudio" in moduleInfo:
      if moduleInfo["canReceiveAudio"]:
         ret += "<font color=cyan>audio</font> "   
   if ret != "":
      ret = "accepts: "+ret
   return ret

documentation = json.load(open("module_documentation.json","r"))

moduleTypes = ["instruments","note effects","synths","audio effects","modulators","pulse","effect chain","other"]
modulesByType = {}
longestColumn = 0
for moduleName in documentation.keys():
   module = documentation[moduleName]
   if not module["type"] in modulesByType:
      modulesByType[module["type"]] = []
   modulesByType[module["type"]].append(moduleName)
   longestColumn = max(longestColumn, len(modulesByType[module["type"]]))

for moduleType in moduleTypes:
   modulesByType[moduleType].sort()


html = open("bespokesynth.com/docs/index.html", "w")
html.write(before)
html.write('''
         <nav class="menu">
            
            <ul></br>Bespoke Synth Reference</br></br>
            <a href="#video">overview video</a>
            <a href="#basics">basics</a>''')
for moduleType in moduleTypes:
   html.write('''
               <input type="checkbox" name="menu" id="'''+moduleType.replace(" ","")+'''">
               <li>
                  <label for="'''+moduleType.replace(" ","")+'''" class="title">'''+moduleType+'''</label>''')
   for entry in modulesByType[moduleType]:
      if documentation[entry]["description"][0] == '[':
         continue
      html.write('''
                  <a href="#'''+entry+'''">'''+entry+'''</a>''')
   html.write('''
               </li>''')

html.write('''
            </ul>
         </nav>''')

html.write('''
      <main class="main">
         <a name=video></a></br>
         <h1>Bespoke Synth Reference</h1>
         <h2>overview video</h2>
         <span style="display:inline-block;margin-left:15px;">
         <p>Here is a video detailing basic usage of Bespoke, including a bunch of less-obvious shortcuts and workflow features.</p>
         <p>There are some things in this video that are slightly out of date (I should make a new video!), but there is still plenty of useful information in there.</p>
         <iframe width="1120" height="630" src="https://www.youtube-nocookie.com/embed/SYBc8X2IxqM?rel=0" frameborder="0" allowfullscreen></iframe>
         </span>
         <br/><br/>
         <a name=basics></a>
         <h2>basics</h2>
         <p>''')
html.write(open("../resource/help.txt","r").read())
html.write('''
         </p>''')
for moduleType in moduleTypes:
   html.write('''
         <h2>'''+moduleType+'''</h2>''')
   for module in modulesByType[moduleType]:
      moduleInfo = documentation[module]
      if moduleInfo["description"][0] == '[':
         continue
      html.write('''
         <a name='''+module+'''></a></br>
         <b>'''+module+'''</b></br>
         <img src="screenshots/'''+module+'''.png">
         </br>
         <span style="display:inline-block;margin-left:50px;">
         ''')
      if "description" in moduleInfo:
         html.write(moduleInfo["description"]+"<br/><br/>")
      if "controls" in moduleInfo:
         for control in moduleInfo["controls"].keys():
            tooltip = moduleInfo["controls"][control]
            if tooltip != "" and tooltip != "[todo]" and tooltip != "[none]":
               html.write("<b>"+control+"</b>: "+tooltip+"<br/>")
      acceptsInputs = getAcceptsInputsString(moduleInfo)
      if acceptsInputs != "":
         html.write("<br/>"+acceptsInputs+"<br/>")
      html.write('''</br></br></br></br>
      </span>''')
html.write('''</main>''')

html.write(after)