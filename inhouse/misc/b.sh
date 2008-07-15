#!/bin/sh
URL=$1
shift
echo TITLE: $@ > /tmp/draft.txt
echo AUTHOR: koya >> /tmp/draft.txt
echo DATE:  >> /tmp/draft.txt
echo DESC: >> /tmp/draft.txt
echo FORMAT: raw >> /tmp/draft.txt
echo ----- >> /tmp/draft.txt
echo BODY: >> /tmp/draft.txt
echo \<a href="$URL"\>$@\</a\> >> /tmp/draft.txt
echo END----- >> /tmp/draft.txt
cat /tmp/draft.txt
nb --file /tmp/draft.txt -a
rm /tmp/draft.txt
