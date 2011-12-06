#!/bin/bash
set -e

if [ -z "$verifyRepos" ]; then
  . ./prereq.sh
fi

if [ -z "${version}" ]; then echo "Missing version, can't upload."; exit 1; fi


if [ -z "${target}" ]; then 
  packagename=sonicawe
  versiontag="${version}${snapshot}"
  qmaketarget=
else
  packagename=sonicawe-${target}
  versiontag="${version}${snapshot}"
  qmaketarget="CONFIG+=TARGET_${target} DEFINES+=TARGET_${target}"
fi

qmaketarget="${qmaketarget} DEFINES+=SONICAWE_VERSION=${versiontag} DEFINES+=SONICAWE_UNAME=`uname -s` CONFIG+=customtarget CUSTOMTARGET=$packagename TARGETNAME+=$target"

if [ "$(uname -s)" == "MINGW32_NT-6.1" ]; then
    platform=windows
elif [ "$(uname -s)" == "Linux" ]; then
    platform=debian
elif [ "$(uname -s)" == "Darwin" ]; then
    platform=macx
else
    echo "Don't know how to build Sonic AWE for this platform: $(uname -s).";
    platform=unknown
fi

. ./make-${platform}.sh
. ./upload.sh
