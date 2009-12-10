#!/bin/bash

SELENIUM_VERSION=1.0.1

echo "Installing Selenium $SELENIUM_VERSION and dependencies in a bash environnement"

function die () {
    echo "$1" >&2
    exit 1
}

MYUID=$(id -u) || die "What? No UID?"
[ "$MYUID" == 0 ] || die "You need to be root"


[ -x /usr/bin/unzip ] || die "No unzip"

XDIR=$(mktemp -d)
mkdir -p /opt/selenium-rc || die "Cannot mkdir"
wget -c http://release.seleniumhq.org/selenium-remote-control/${SELENIUM_VERSION}/selenium-remote-control-${SELENIUM_VERSION}-dist.zip
unzip -d $XDIR selenium-remote-control-${SELENIUM_VERSION}-dist.zip *selenium-server.jar *selenium.py || die "Problem unzipping"
mv $XDIR/*/*/*.{py,jar} /opt/selenium-rc || die "No needed files found"
rm -r $XDIR || die "We dont care"

# Add selenium to PYTHONPATH
PYPATH=/opt/selenium-rc/
if [ "x$PYTHONPATH" != "x" ] && [ echo $PYTHONPATH | grep -q $PYTPATH ]
then
    echo "PYTHONPATH already set"
else
    echo "export PYTHONPATH=$PYPATH:$PYTHONPATH" >> ~/.bashrc
fi

aptitude show openjdk-6-jre | grep -q 'State: installed' 
if [ $? == 0 ]
then
    echo "openjdk-6-jre already installed"
else
    aptitude -R -y install openjdk-6-jre
fi
