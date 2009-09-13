#!/bin/sh

# full path to your java 1.6+ binary
JAVA_EXE=java

$JAVA_EXE -Djava.library.path=lib -cp lib/qtjambi.jar:bin pl.swmud.ns.swaedit.gui.SWAEdit &

