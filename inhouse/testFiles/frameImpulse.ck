// frameImpulse.ck
// ChucK file that generates a 900 frame wavfile (where 1 30fps frame = 1470 samples), 
// consisting of a repeated pattern:
// 44 frames of 0's, 1 frame of 1's, 44 frames of 0's, 1 frame of 1's...
//
// ChucK is available at http://chuck.cs.princeton.edu/ 
//
// Run this file with the command:
// chuck frameImpulse.ck
//
// or for the silent version, take the dac out of the signal chain below and run with:
// chuck --silent frameImpulse.ck

Step s => WvOut w => blackhole;

0.0 => s.next;
30 => int fps;
44 => int blackFrames;
45 => int endFrames;
900 => int numFrames;

second / samp => float sampleRate;

sampleRate / fps => float samplesPerFrame;

<<<"samples per frame: " + samplesPerFrame >>>;

"impulse.wav" => w.wavFilename;
<<<"writing to file:", "'" + w.filename() + "'">>>;

for (0 => int i; i < numFrames; i++)
{
    // write 44 frames of 0's the first time, then 45 every other time
    
    (blackFrames * samplesPerFrame)::samp => now;

    // write 1 frame of 1's
    1.0 => s.next;
    samplesPerFrame::samp => now;

//    45 => blackFrames; 
    0.0 => s.next;
}


// write 44 frames of 0's to the file.
//(startFrames * samplesPerFrame)::samp => now;

// write 1 frame of 1's to the file
//1.0 => s.next;
//samplesPerFrame::samp => now;

// write 45 frames of 0's to the file
//0.0 => s.next;
//(endFrames * samplesPerFrame)::samp => now;

// close file
w.closeFile(w.filename());

// copy
//Std.system("cp impulse.wav ../data/");
