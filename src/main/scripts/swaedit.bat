REM full path to your javaw 1.6+ binary
set JAVA_EXE=javaw

start /B %JAVA_EXE% -Djava.library.path=lib -cp lib/qtjambi.jar;bin pl.swmud.ns.swaedit.gui.SWAEdit

