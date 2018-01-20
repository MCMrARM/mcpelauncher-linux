#!/bin/bash

function print_help {
  echo "$(basename $0) [-h] [-c] -v -a -e

  optional arguments:

    -h, --help      print this help
    -c, --cmake     path to cmake ( default: cmake )
    -t, --tempdir   path to store temporary files (default: /tmp )
    --notroot       Must add this to run the script as non root ( causes bad ownership of files in .deb )

  mandatory arguments:
    -e, --email     email address for package maintainer
    -a, --author    Name of package maintainer
    -v, --version   Version of package


  Before running this script please make sure that \"make\" can run succesfully.
    "
    exit
}

unparsed_args=()
while [[ $# -gt 0 ]]; do
  case $1 in
      -h|--help)
        print_help
      ;;
      -e|--email)
      MAINTAINER_EMAIL="$2"
      shift 2;;
      -a|--author)
      MAINTAINER_NAME="$2"
      shift 2;;
      -v|--version)
      VERSION="$2"
      shift 2;;
      -c|--cmake)
      CMAKE="$2"
      shift 2;;
      -t|--tempdir)
      TEMPDIR="$2"
      shift 2;;
      --tempdir)
      NOTROOT="user override"
      shift;;
      *)
      unparsed_args+=("$1")
      shift;;
  esac
done
set -- "${unparsed_args[@]}"

if [[ $EUID -ne 0 ]] && [ -z "${NOTROOT}" ]; then
  echo You are Not root
  echo
  print_help
fi


if [ -z "$CMAKE" ]; then
  CMAKE=cmake
fi

if [ -z "$TEMPDIR" ]; then
  TEMPDIR=/tmp
fi

if [ -z "${MAINTAINER_NAME}" ] || [ -z "${MAINTAINER_EMAIL}" ] || [ -z "$VERSION" ]; then
  print_help
fi;

echo Author   = "${MAINTAINER_NAME}"
echo Email    = "${MAINTAINER_EMAIL}"
echo version  = "${VERSION}"
echo cmake    = "${CMAKE}"
echo tempdir  = "${TEMPDIR}"

if [[ -n $@ ]]; then
  echo The folloing arguments could not be understood or parsed
  echo \( did you forget to enclose the author name in quotes ? \)
  echo
  echo unparsed = \"$@\"
  echo
  print_help
fi;


BUILDDIR="${TEMPDIR}/mcpelauncher-linux/"
INSTALLDIR="${BUILDDIR}usr/local/";
DEBDIR="${BUILDDIR}DEBIAN/";

mkdir -p "${INSTALLDIR}"
mkdir -p "${DEBDIR}"
${CMAKE} -DCMAKE_INSTALL_PREFIX="${INSTALLDIR}"

if [ $? -ne 0 ]; then
  echo Something went wrong with cmake, you should fix that.
  exit;
fi

LAUNCHDIR="$PWD"

#do we really need to run make again?
make

make install

#Copy extra files needed
#Menu launcher:
MENULAUNCERDIR="${BUILDDIR}usr/share/applications/"
mkdir -p "${MENULAUNCERDIR}"
cp mcpelauncher.desktop "${MENULAUNCERDIR}"

#License - to avoid being marked as tainted
DOCDIR="${BUILDDIR}usr/share/doc/mcpelauncher-linux/"
mkdir -p "${DOCDIR}"
cp LICENSE "${DOCDIR}/copyright"


#fix the symlink in usr/local/bin to be rellative
rm "${INSTALLDIR}bin/mcpelauncher"
ln -s "..share/mcpelauncher/bin/mcpelauncher" "${INSTALLDIR}bin/mcpelauncher"

#Create md5sums of all files rellative to install root:
cd "${BUILDDIR}"

find usr -type f -print0 | xargs -0 md5sum > "${DEBDIR}/md5sums"

#Get installed size:

INSTALLSIZE=$(du -s usr/ | awk '{print $1}')

echo "Package: mcpelauncher-linux
Version: ${VERSION}
Installed-Size: ${INSTALLSIZE}
Maintainer: ${MAINTAINER_NAME} <${MAINTAINER_EMAIL}>
Section: unknown
Source: mcpelauncher-linux
Homepage: https://github.com/MCMrARM/mcpelauncher-linux
Architecture: i386
Priority: optional
Depends: libegl1-mesa, libx11-6, libuuid1, libpng12-0, libcurl3, libzip-dev, libprotobuf-dev, libglib2.0-0, libnss3, libfontconfig1, libgtk2.0-0, libgtkglext1, libasound2, libnss3, libxss1, libgconf2-4, libxtst6, libudev1
Description: Linux launcher for android version of Minecraft
 Launch Minecraft [Bedrock engine] on Linux!
  please be aware that you must have purchased Minecraft via
  https://play.google.com/store
   in order to use the launcher.
  Last verified minecraft version to work with this launcher: ${VERSION}
" > "${DEBDIR}/control"

#back to where we were launched;
cd "${LAUNCHDIR}";

dpkg-deb -b "${BUILDDIR}" "mcpelauncher-linux_${VERSION}_i386.deb"
