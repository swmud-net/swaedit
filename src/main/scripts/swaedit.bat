REM full path to your javaw 1.6+ binary
set JAVA_EXE=javaw

start /B %JAVA_EXE% -Djava.library.path=lib -cp lib/fontbox.jar;lib/gluegen-rt.jar;lib/jogl.all-noawt.jar;lib/nativewindow.all-noawt.jar;lib/newt.all-noawt.jar;lib/qtjambi.jar;bin pl.swmud.ns.swaedit.gui.SWAEdit

