#!/bin/bash
set -e

bury_copy() { mkdir -p "`dirname $2`" && cp "$1" "$2"; }

if [ -n "$1" ] && [ -n "$2" ] && [ -n "$3" ] && [ -z "$4" ] && [ "$(basename `pwd`)" == "dist" ] ; then
		packagename=$3
        versionnumber=$2
        versionname=$1
elif [ -n "$1" ] && [ -n "$2" ] && [ -z "$3" ] && [ "$(basename `pwd`)" == "dist" ] ; then
		packagename=sonicawe
        versionnumber=$2
        versionname=$1
elif [ -n "$1" ] && [ -z "$2" ] && [ "$(basename `pwd`)" == "dist" ] ; then
		packagename=sonicawe
        versionnumber=$1
        versionname=$1
else
	echo "Creates a Sonic AWE package for Debian linux"
	echo
	echo "SYNOPSIS"
	echo "    package-debian.sh version_string"
	echo
	echo "DESCRIPTIION"
	echo "     'version_string' is on the form"
	echo "         0.2011.01.12"
	echo "         0.2011.01.12-snapshot"
	echo
	echo "     Or use 'def' as version_string to produce default format."
	echo
	echo "Run this script from the sonic/sonicawe/dist directory"
    exit
fi

echo $versionname

if [ $versionname == "def" ] ; then
	versionname="$(date +0.%Y.%m.%d-snapshot)"
fi

package=tmp/package-debian~
share=$package/opt/muchdifferent/sonicawe/.

pushd ..
rm -rf $package
mkdir -p tmp
cp -r dist/package-debian $package
if [ -n "${versionnumber}" ]; then
	sed -i "s/Version: .*$/Version: ${versionnumber}/g" $package/DEBIAN/control
fi


if [ "sonicawe" != "${packagename}" ]; then
  sed -i "s/Package: .*$/Package: ${packagename}/g" $package/DEBIAN/control
  mv $package/usr/lib/mime/packages/sonicawe $package/usr/lib/mime/packages/${packagename}
  mv $package/usr/share/applications/sonicawe.desktop $package/usr/share/applications/${packagename}.desktop
  mv $package/usr/share/menu/sonicawe $package/usr/share/menu/${packagename}
  mv $package/usr/share/mime/packages/sonicawe.xml $package/usr/share/mime/packages/${packagename}.xml
  mv $package/usr/share/mime-info/sonicawe.keys $package/usr/share/mime-info/${packagename}.keys
  mv $package/usr/share/mime-info/sonicawe.mime $package/usr/share/mime-info/${packagename}.mime
  mv $package/usr/share/pixmaps/sonicawe16.xpm $package/usr/share/pixmaps/${packagename}16.xpm
  mv $package/usr/share/pixmaps/sonicawe.xpm $package/usr/share/pixmaps/${packagename}.xpm

  sed -i "s/sonicawe /${packagename} /g" $package/usr/lib/mime/packages/${packagename}

  sed -i "s/Exec=sonicawe/Exec=${packagename}/g" $package/usr/share/applications/${packagename}.desktop

  prettyname=$(echo ${packagename} | sed "s/sonicawe-//" | sed "s/-/ /g")
  prettyname=$(echo $prettyname | nawk -F: '{ print toupper(substr ($1,1,1)) substr ($1,2) }')
  sed -i "s/Name=Sonic AWE/Name=Sonic AWE ${prettyname}/g" $package/usr/share/applications/${packagename}.desktop

  sed -i "s/Sonic AWE/Sonic AWE ${prettyname}/g" $package/usr/share/menu/${packagename}
  sed -i "s/sonicawe/${packagename}/g" $package/usr/share/menu/${packagename}
  sed -i "s/level=sonicawe/level=${packagename}/g" $package/usr/share/mime-info/${packagename}.keys
fi


mkdir -p $package/usr/bin
ln -s ../../opt/muchdifferent/sonicawe/${packagename} $package/usr/bin/${packagename}

mkdir -p $share
stat src/${packagename}-cuda >& /dev/null && CUDA=1
if [ $CUDA ]; then
    cp src/${packagename} $share/${packagename}-cpu
    cp src/${packagename}-cuda $share/${packagename}-cuda
    cp src/sonicawe-launcher.sh $share/${packagename}
else
    cp src/${packagename} $share/${packagename}
fi
sed -i "s/sonicawe/${packagename}/g" $share/${packagename}
cp -r lib/sonicawe-ubuntulib/lib/* $share/.

if [ "`uname -m`" = "x86_64" ]; then
    sed -i "s/Architecture: .*$/Architecture: amd64/g" $package/DEBIAN/control
    [ $CUDA ] && cp -r /usr/local/cuda/lib64/libcudart.so* $share/.
    [ $CUDA ] && cp -r /usr/local/cuda/lib64/libcufft.so* $share/.
else
    sed -i "s/Architecture: .*$/Architecture: i386/g" $package/DEBIAN/control
    [ $CUDA ] && cp -r /usr/local/cuda/lib/libcudart.so* $share/.
    [ $CUDA ] && cp -r /usr/local/cuda/lib/libcufft.so* $share/.
fi
bury_copy sonicawe.1 $package/usr/local/share/man/man1/${packagename}.1
mkdir -p $share/examples
cp matlab/sawe_compute_cwt.m $share
cp matlab/sawe_extract_cwt.m $share
cp matlab/sawe_extract_cwt_time.m $share
cp matlab/sawe_filewatcher.m $share
cp matlab/sawe_getdatainfo.m $share
cp matlab/sawe_datestr.m $share
cp plugins/exampleplugin.m $share/examples
cp plugins/examplepluginicon.png $share/examples
cp plugins/examplesource.m $share/examples
cp plugins/exportpeakfrequencies.m $share/examples
cp plugins/reversesignal.m $share/examples
cp matlab/examples/amplify.m $share/examples
cp matlab/examples/convolve.m $share/examples
cp matlab/examples/lowpass.m $share/examples
cp matlab/examples/plotwaveform.m $share/examples
cp matlab/sawe_loadstruct.m $share
cp matlab/sawe_savestruct.m $share
cp matlab/sawe_makestruct.m $share
cp matlab/sawe_discard.m $share
cp matlab/sawe_plot.m $share
cp matlab/sawe_plot2.m $share
cp -r license $share
pushd $package
gzip -f usr/local/share/man/man1/${packagename}.1
rm -f DEBIAN/md5sums
for i in `find -name *~`; do rm $i; done
for i in `find usr opt -type f`; do md5sum $i >> DEBIAN/md5sums; done
for i in `find usr -type l`; do md5sum $i >> DEBIAN/md5sums; done
popd
installedsize=`du -l | grep "[0-9]*.*\\.$" | sed "s/\t.*//g"`
sed -i "s/Installed-Size: .*$/Installed-Size: ${installedsize}/g" $package/DEBIAN/control

output_deb="${packagename}_"$versionname"_`uname -m`.deb"
#http://www.debian.org/doc/debian-policy/ch-controlfields.html
dpkg -b $package tmp/$output_deb
echo "OUTPUT"
echo "    `pwd`/tmp/$output_deb"
popd

