echo off
echo Cleaning...
del ..\bin\win\parameter_catalog\*.csv /q
del ..\bin\linux_debian\parameter_catalog\*.csv /q
del ..\bin\linux_fedora\parameter_catalog\*.csv /q
del ..\bin\mac\parameter_catalog\*.csv /q
echo Copying...
copy *.csv ..\bin\win\parameter_catalog\
copy *.csv ..\bin\linux_debian\parameter_catalog\
copy *.csv ..\bin\linux_fedora\parameter_catalog\
copy *.csv ..\bin\mac\parameter_catalog\
pause