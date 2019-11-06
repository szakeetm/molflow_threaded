echo off

echo Copying files ...
md ..\bin\win\desorption_yields\
md ..\bin\win\images\
md ..\bin\win\parameter_catalog\
copy .\7za.exe ..\bin\win\7za.exe
copy .\updater_config_default_win.xml ..\bin\win\updater_config.xml
copy .\desorption_yields\* ..\bin\win\desorption_yields\
copy .\images\* ..\bin\win\images\
copy .\parameter_catalog\* ..\bin\win\parameter_catalog\

echo Copying files for release build ...
md ..\bin\win\release\
#md ..\bin\win\release\desorption_yields\
#md ..\bin\win\release\images\
#md ..\bin\win\release\parameter_catalog\
#copy .\7za.exe ..\bin\win\release\7za.exe
#copy .\updater_config_default_win.xml ..\bin\win\release\updater_config_default_win.xml
#copy .\desorption_yields\* ..\bin\win\release\desorption_yields\
#copy .\images\* ..\bin\win\release\images\
#copy .\parameter_catalog\* ..\bin\win\release\parameter_catalog\
copy ..\lib_external\win\dll\*.dll ..\bin\win\release\

echo Copying files for debug build ...
md ..\bin\win\debug\
#md ..\bin\win\debug\desorption_yields\
#md ..\bin\win\debug\images\
#md ..\bin\win\debug\parameter_catalog\
#copy .\7za.exe ..\bin\win\debug\7za.exe
#copy .\updater_config_default_win.xml ..\bin\win\debug\updater_config_default_win.xml
#copy .\desorption_yields\* ..\bin\win\debug\desorption_yields\
#copy .\images\* ..\bin\win\debug\images\
#copy .\parameter_catalog\* ..\bin\win\debug\parameter_catalog\
copy ..\lib_external\win\dll\*.dll ..\bin\win\debug\
pause