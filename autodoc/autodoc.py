import json

before = '''<!DOCTYPE html>
<html lang="en">

<head>

    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta name="description" content="">
    <meta name="author" content="">

    <title>Bespoke Synth</title>

    <!-- Bootstrap Core CSS -->
    <link href="../css/bootstrap.min.css" rel="stylesheet">

    <!-- Custom CSS -->
    <link href="../css/grayscale.css" rel="stylesheet">

    <link href="../css/table.css" rel="stylesheet">

    <!-- Custom Fonts -->
    <link href="../font-awesome/css/font-awesome.min.css" rel="stylesheet" type="text/css">
    <link href="http://fonts.googleapis.com/css?family=Lora:400,700,400italic,700italic" rel="stylesheet" type="text/css">
    <link href="http://fonts.googleapis.com/css?family=Montserrat:400,700" rel="stylesheet" type="text/css">

    <!-- HTML5 Shim and Respond.js IE8 support of HTML5 elements and media queries -->
    <!-- WARNING: Respond.js doesn't work if you view the page via file:// -->
    <!--[if lt IE 9]>
        <script src="https://oss.maxcdn.com/libs/html5shiv/3.7.0/html5shiv.js"></script>
        <script src="https://oss.maxcdn.com/libs/respond.js/1.4.2/respond.min.js"></script>
    <![endif]-->

</head>

<body id="page-top" data-spy="scroll" data-target=".navbar-fixed-top">

    
    <a name="top"></a>
    <!-- Intro Header -->
    <header class="intro">
        <div class="intro-body">
            <div class="container">
                <div class="row">
                    <div>
                        <h1 class="brand-heading">Bespoke</h1>
                        <h5 class="intro-text">Reference Guide</h5>'''

after = '''
    

    <!-- jQuery -->
    <script src="js/jquery.js"></script>

    <!-- Bootstrap Core JavaScript -->
    <script src="js/bootstrap.min.js"></script>

    <!-- Plugin JavaScript -->
    <script src="js/jquery.easing.min.js"></script>

    <!-- Custom Theme JavaScript -->
    <script src="js/grayscale.js"></script>

    <!-- Google analytics -->    
    <script>
      (function(i,s,o,g,r,a,m){i['GoogleAnalyticsObject']=r;i[r]=i[r]||function(){
      (i[r].q=i[r].q||[]).push(arguments)},i[r].l=1*new Date();a=s.createElement(o),
      m=s.getElementsByTagName(o)[0];a.async=1;a.src=g;m.parentNode.insertBefore(a,m)
      })(window,document,'script','//www.google-analytics.com/analytics.js','ga');

      ga('create', 'UA-59239115-1', 'auto');
      ga('send', 'pageview');

    </script>

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


html = open("bespokesynth.com/docs/index.html", "w")
html.write(before)
html.write('''<div class="divTable blueTable">
                           <div class="divTableHeading">
                           <div class="divTableRow">''')
for moduleType in moduleTypes:
   html.write('''<div class="divTableHead">'''+moduleType+'''</div>''')
html.write('''</div></div><div class="divTableBody">''')
for i in range(longestColumn):
   html.write('''<div class="divTableRow">''')
   for moduleType in moduleTypes:
      entry = ""
      if i < len(modulesByType[moduleType]):
         entry = modulesByType[moduleType][i]
      html.write('''<div class="divTableCell"><a href=#'''+entry+'''>'''+entry+'''</a></div>''')
   html.write('''</div>''')

html.write('''      </div>
                </div>
            </div>
        </div>
</header>
<section id="modules" class="container content-section">''')

html.write('''<div class="divTable blueTable">''')
for moduleType in moduleTypes:
   for module in modulesByType[moduleType]:
      html.write('''<div class="divTableRow">
      <div class="divTableCellImage">
      <a name='''+module+'''>
      <img src="screenshots/'''+module+'''.png">
      </a>
      </div>
      <div class="divTableCellWide"><b>'''+module+'''</b><br/>''')
      moduleInfo = documentation[module]
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
      html.write('''<br/><a href=#top>back to top</a><br/><br/>
      </div>
      </div>''')
html.write('''</div>''')

html.write("</section>")

html.write(after)