#!/bin/sh

# full path to your java 1.6+ binary
JAVA_EXE=java

$JAVA_EXE -Djava.library.path=lib -cp lib/fontbox.jar:lib/gluegen-rt.jar:lib/jogl.all-noawt.jar:lib/nativewindow.all-noawt.jar:lib/newt.all-noawt.jar:lib/qtjambi.jar:bin pl.swmud.ns.swaedit.gui.SWAEdit &

