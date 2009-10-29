#!/bin/bash


function die () {
    echo "$1" >&2
    exit 1
}

MYUID=$(id -u) || die "What? No UID?"
[ "$MYUID" == 0 ] || die "You need to be root"


[ -x /usr/bin/unzip ] || die "No unzip"

# TODO: Check for openjdk-6-jre
XDIR=$(mktemp -d)
mkdir -p /opt/selenium-rc || die "Cannot mkdir"
unzip -d $XDIR selenium-remote-control-1.0.1-dist.zip *selenium-server.jar *selenium.py || die "Problem unzipping"
mv $XDIR/*/*/*.{py,jar} /opt/selenium-rc || die "No needed files found"
rm -r $XDIR || die "We dont care"



