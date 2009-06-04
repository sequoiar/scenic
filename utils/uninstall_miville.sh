#!/bin/bash
while [ "x$answer" = "x" ]
do
    echo -n "Are you sure you want to anihilate the miville package? (y/n) "
    read answer
    case "$answer" in
        N|n)
            echo "Quitting..."
            exit
        ;;
        Y|y)
            sudo rm -rf /usr/local/lib/python2.5/site-packages/miville-0.1.3_a-py2.5.egg
            sudo rm -f /usr/local/bin/miville
            sudo sed -i "/miville/d"  /usr/local/lib/python2.5/site-packages/easy-install.pth
            echo "Purged miville from /usr/local/"
            exit
        ;;
        *)
            answer=""
        ;;
    esac
done
