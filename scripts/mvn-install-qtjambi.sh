#!/bin/sh
exit 0

#####################################################################
#       USE WITH CAUTION - IT MAY REMOVE SOME IMPORTANT FILES       #
#####################################################################


mvn=/usr/local/apache-maven-2.2.1/bin/mvn
jar=/opt/sun-jdk-1.6.0.22/bin/jar

version='4.5.2_01'
echo $version
groupId=com.trolltech.qtjambi

parent=`pwd`
for i in `ls -d qtjambi-*$version*`
do
	if [ -d $i ]; then
		echo $i
		cd $i
		aid=`basename \`pwd\` |sed "s/-$version//"`
		rm -rf artifact
		mkdir -p artifact/lib

		cp {lib,bin}/libstdc++.{so,dll}* artifact/lib/
		cp {lib,bin}/*QtCore{,4}.{so,dll}* artifact/lib/
		cp {lib,bin}/*QtGui{,4}.{so,dll}* artifact/lib/
		cp {lib,bin}/*QtOpenGL{,4}.{so,dll}* artifact/lib/
		cp {lib,bin}/*qtjambi.{so,dll} artifact/lib/
		cp {lib,bin}/*com_trolltech_qt_core.{so,dll}* artifact/lib/
		cp {lib,bin}/*com_trolltech_qt_gui.{so,dll}* artifact/lib/
		cp {lib,bin}/*com_trolltech_qt_opengl.{so,dll}* artifact/lib/
		mv artifact $aid
		$jar cMf $aid.jar $aid
		$mvn install:install-file -Dfile=$aid.jar -DgroupId=$groupId -DartifactId=$aid -Dversion=$version -Dpackaging=jar -DgeneratePom=true -DcreateChecksum=true -DupdateReleaseInfo=true
		rm -rf $aid $aid.jar
		qtjambijar=$i/qtjambi-$version.jar
		cd $parent
	fi
done

$mvn install:install-file -Dfile=$qtjambijar -DgroupId=$groupId -DartifactId=qtjambi -Dversion=$version -Dpackaging=jar -DgeneratePom=true -DcreateChecksum=true -DupdateReleaseInfo=true


