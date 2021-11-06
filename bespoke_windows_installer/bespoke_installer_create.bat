"C:\Program Files (x86)\WiX Toolset v3.11\bin\heat" dir ..\ignore\build\Source\BespokeSynth_artefacts\Release\resource -o HarvestedResourceDir.wxs -scom -frag -srd -sreg -gg -cg BespokeResourceDir -dr RESOURCE_DIR_REF
"C:\Program Files (x86)\WiX Toolset v3.11\bin\heat" dir ..\ignore\build\Source\BespokeSynth_artefacts\Release\python -o HarvestedPythonDir.wxs -scom -frag -srd -sreg -gg -cg BespokePythonDir -dr PYTHON_DIR_REF
"C:\Program Files (x86)\WiX Toolset v3.11\bin\candle" BespokeSynth.wxs HarvestedResourceDir.wxs HarvestedPythonDir.wxs -arch x64
"C:\Program Files (x86)\WiX Toolset v3.11\bin\light" BespokeSynth.wixobj HarvestedResourceDir.wixobj HarvestedPythonDir.wixobj -b ..\ignore\build\Source\BespokeSynth_artefacts\Release\resource -b ..\ignore\build\Source\BespokeSynth_artefacts\Release\python -out Bespoke-Windows.msi -ext WixUIExtension

@pause