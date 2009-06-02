#!/bin/bash
sudo rm -rf /usr/local/lib/python2.5/site-packages/miville-0.1.3_a-py2.5.egg
sudo rm -f /usr/local/bin/miville
sudo sed -i "/miville/d"  /usr/local/lib/python2.5/site-packages/easy-install.pth
echo purged miville from /usr/local/
