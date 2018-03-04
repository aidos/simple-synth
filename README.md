# simple-synth

Simple synth for the Arduino Uno (ATMega328).

I have a full size keyboard midi controller (M-audio Keystation 88es) just sitting around that I want to turn into an electric keyboard. As a software guy, the obvious path was to use some sort of computer - USB input from the keyboard into an open source software synth. But a) I don't want to wait for it to boot each time I turn it on and b) it wouldn't be much of a challenge (or therefore much fun).

Then it came to me, there are millions of old keyboards that could make sound in the 80s, how hard could it be to make a simple synth using modern technology? Turns out there's a quite a bit you need to know and, while there's loads of information out there, there isn't a specific easy example to look at. Much of the code is opaque and poorly documented.

Software is my thing, but I have no real experience in C. I have never written code for an embedded system and I've never done a real electronics project.

The plan is document what I learn here while building a simple synth.


# Goals

- Read MIDI In to figure out which keys are being pressed
- Polyphonic, to some degree (maybe paraphonic)
- Probably some external controls for manipulating the sound
- Just line level output - use an external amplifier
- ATMega328 only - I (now) understand it's not suited to this type of work, but that probably makes it better for learning on


# Sound Synthesis

There are loads of ways of synthesizing sound but there you'll see many of the same things coming up again and again. It's worth reading through some introductions to get an idea of the different models.

# Round 1

I'm not really sure how much load I'll be able to put on an ATMega328 chip, but there aren't many clock cycles in which to get our work done. For that reason I'm going to start _really_ simple and then push things further until they break. At that point I'll probably look to at other chips designed for this type of work.

- midi in using midi library (slow?)
- simple oscilator using wavetable lookup
- monophonic?
- no filters / extra anything
- no envelope
- use timer + interrupt 30k per second for samples
- output using PWM

TIMERS - say what?! As far as I can tell, timers are a pretty huge part of all embedded systems. In our case, we're going to use a timer to fire an interrupt roughly 30k times per second. During that interrupt we will decide what the output voltage should be at that moment (height of the waveform). We then use a digital output of the chip to "fake" that output voltage as an analog value. This is done using pulse width modulation (PWM), which again is another timer.

# Resources

I've found a few specific resources handy, so I'll start fleshing them out in here again as I need them.


