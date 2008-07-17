#! /usr/bin/octave
# require gnu octave, also
# requires octave avi plugins available at 
# http://mentat.za.net/octave-avifile-20070718.tar.bz2

# open the avi file

filename = "inputVid.avi";

m = avifile(filename, "codec", "mpeg4", "fps", 30);

# Generate and add frames
numFrames = 900;
width = 160;
height = 90;

I = zeros(height, width, numFrames);    # buffer of black frames


numWhiteFrames = numFrames / 45;
whiteFrames = 45 .* [1:numWhiteFrames]; # indices of white frames


for t = 1:numWhiteFrames
    I(:, :, whiteFrames(t)) = ones(height, width);
endfor


for i = 1:numFrames
    addframe(m, I(:, :, i))      # add normalized frame to file
endfor

printf("\n")

# close the file
clear m

#copyCommand = strcat("cp ", filename, " ../data/");

#if system(copyCommand) ~= 0
#    printf("Copy failed.")
#end


