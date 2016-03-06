
### automatic way : eclipse integration

install eclipse
install ADT plugin
install CDT plugin
install subclipse plugin

from eclipse :

add SVN repository http://jeyries-code.svn.beanstalkapp.com/quake2android

checkout trunk as a project in workspace

that'all

### manual way :

### setup path

SRCDIR=/media/data/julien/projets/quake2android

NDKDIR=/media/data/outils/android-ndk-1.6_r1

SVNURL=http://jeyries-code.svn.beanstalkapp.com/quake2android/trunk

### download source code

svn checkout $SVNURL $SRCDIR


### setup NDK links


ln -s $SRCDIR $NDKDIR/apps/quake2


#### build libquake2.so

cd $NDKDIR
make APP=quake2

#### build APK

start eclipse
-> New Android Project
-> create project from existing sources
-> browse to $SRCDIR/application


that's it !


