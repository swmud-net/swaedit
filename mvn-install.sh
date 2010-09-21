#!/bin/sh
exit 0

#####################################################################
#       USE WITH CAUTION - IT MAY REMOVE SOME IMPORTANT FILES       #
#####################################################################


mvn=/usr/local/apache-maven-2.2.0/bin/mvn
jar=/opt/sun-jdk-1.6.0.11/bin/jar

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
		cp {lib,bin}/*QtCore4.{so,dll}* artifact/lib/
		cp {lib,bin}/*QtGui4.{so,dll}* artifact/lib/
		cp {lib,bin}/*qtjambi.{so,dll} artifact/lib/
		cp {lib,bin}/*com_trolltech_qt_core.{so,dll}* artifact/lib/
		cp {lib,bin}/*com_trolltech_qt_gui.{so,dll}* artifact/lib/
		mv artifact $aid
		$jar cMf $aid.jar $aid
		$mvn install:install-file -Dfile=$aid.jar -DgroupId=$groupId -DartifactId=$aid -Dversion=$version -Dpackaging=jar -DgeneratePom=true -DcreateChecksum=true -DupdateReleaseInfo=true
		rm -rf $aid $aid.jar
		qtjambijar=$i/qtjambi-$version.jar
		cd $parent
	fi
done

$mvn install:install-file -Dfile=$qtjambijar -DgroupId=$groupId -DartifactId=qtjambi -Dversion=$version -Dpackaging=jar -DgeneratePom=true -DcreateChecksum=true -DupdateReleaseInfo=true


groupId=com.jogamp.jogl
jars_installed=0
for i in `ls -d jogl*$version*`
do
	if [ -d $i ]; then
		cd $i

		# jars
		if [ $jars_installed -ne 1 ]; then
			rm -rf ./jars
			mkdir -p jars
			cd lib
			cp -f newt.all-noawt.jar nativewindow.all-noawt.jar jogl.all-noawt.jar gluegen-rt.jar ../jars/
			cd ../jars
			for j in `ls -d *.jar`
			do
				aid=`echo $j |sed 's/\.jar//'`
				$mvn install:install-file -Dfile=$j -DgroupId=$groupId -DartifactId=$aid -Dversion=$version -Dpackaging=jar -DgeneratePom=true -DcreateChecksum=true -DupdateReleaseInfo=true
			done
			cd ..
			jars_installed=1
		fi

		# libs
		aid=`basename \`pwd\` |sed "s/-$version//"`
		rm -rf ./artifact
		mkdir -p artifact/lib
		cd lib
		cp -f *gluegen-rt.{so,dll} *jogl_desktop.{so,dll} *newt.{so,dll} libnativewindow_x11.so ../artifact/lib/
		cd ..
		rm -rf ./$aid*
		mv artifact $aid
		$jar cMf $aid.jar $aid
		$mvn install:install-file -Dfile=$aid.jar -DgroupId=$groupId -DartifactId=$aid -Dversion=$version -Dpackaging=jar -DgeneratePom=true -DcreateChecksum=true -DupdateReleaseInfo=true
		cd $parent
	fi
done


