1.0 => dac.gain;
Step step => dac;
1.0 => float amp;
amp => step.next;

while (true)
{
    800::samp => now;
}
