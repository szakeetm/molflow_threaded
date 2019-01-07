echo off
echo Cleaning...
del ..\bin\win\images\*.png /q
del ..\bin\linux_debian\images\*.png /q
del ..\bin\linux_fedora\images\*.png /q
del ..\bin\mac\images\*.png /q
echo Copying...
copy *.png ..\bin\win\images\
copy *.png ..\bin\linux_debian\images\
copy *.png ..\bin\linux_fedora\images\
copy *.png ..\bin\mac\images\
pause