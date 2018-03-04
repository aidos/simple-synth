
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>


#define PWM_PIN       3
#define PWM_VALUE     OCR2B
#define PWM_INTERRUPT TIMER2_OVF_vect


// sine wave represented as 256 discrete samples
const  char  waveTable[] PROGMEM = {
  1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31,
  33, 35, 37, 39, 41, 43, 45, 47, 49, 51, 53, 55, 57, 59, 61, 63, 65,
  67, 69, 71, 73, 75, 77, 79, 81, 83, 85, 87, 89, 91, 93, 95, 97, 99,
  101, 103, 105, 107, 109, 111, 113, 115, 117, 119, 121, 123, 125, 127, 129, 131, 133,
  135, 137, 139, 141, 143, 145, 147, 149, 151, 153, 155, 157, 159, 161, 163, 165, 167,
  169, 171, 173, 175, 177, 179, 181, 183, 185, 187, 189, 191, 193, 195, 197, 199, 201,
  203, 205, 207, 209, 211, 213, 215, 217, 219, 221, 223, 225, 227, 229, 231, 233, 235,
  237, 239, 241, 243, 245, 247, 249, 251, 253, 255, 254, 252, 250, 248, 246, 244, 242,
  240, 238, 236, 234, 232, 230, 228, 226, 224, 222, 220, 218, 216, 214, 212, 210, 208,
  206, 204, 202, 200, 198, 196, 194, 192, 190, 188, 186, 184, 182, 180, 178, 176, 174,
  172, 170, 168, 166, 164, 162, 160, 158, 156, 154, 152, 150, 148, 146, 144, 142,
  140, 138, 136, 134, 132, 130, 128, 126, 124, 122, 120, 118, 116, 114, 112, 110, 108,
  106, 104, 102, 100, 98, 96, 94, 92, 90, 88, 86, 84, 82, 80, 78, 76, 74, 72, 70, 68, 66,
  64, 62, 60, 58, 56, 54, 52, 50, 48, 46, 44, 42, 40, 38, 36, 34, 32, 30, 28, 26, 24, 22,
  20, 18, 16, 14, 12, 10, 8, 6, 4, 2, 0
};



// Effectively we can represent each note on the keyboard as the number of wavetable
// entries we need to jump per sample.

// The wavetable itself takes 256 samples to run through
// So if we just did incremented 1 position for each sample it would be 31250/256 = 122 Hz

// For an A at 440 Hz, we want to run through the wavetable 440 times per second
// IE we need move to the next wavetable entry every 0.277432528 samples
// IE we want to increment 3.60448 wavetable entries per sample for a note at 440 Hz

// The best way to respresent that is to work in 16 bit using some multiple of
// 2 and then shift that down so we don't lose too much resolution due to rounding.
// So, we can represent 440 Hz as 3691 = (2^10) * 256 * 440 / 31250

// more generally:
// notes = [261.6, 277.2, 293.7, 311.1, 329.6, 349.2, 370.0, 392.0, 415.3, 440.0, 466.2, 493.9]
// increments = [round(n * 256 / 31250 * (2**10)) for n in notes]
// increments = [2194, 2325, 2464, 2610, 2765, 2929, 3104, 3288, 3484, 3691, 3911, 4143]

// Note that we only need an octave worth and then can shift up / down to get the other octaves


volatile uint16_t freq = 0;

void audioOn() {
  // Set up PWM to 31.25kHz, phase accurate
  TCCR2A = _BV(COM2B1) | _BV(WGM20);
  TCCR2B = _BV(CS20);
  TIMSK2 = _BV(TOIE2);
}


// -----------------------------------------------------------------------------

MIDI_CREATE_DEFAULT_INSTANCE();

void setup() {
  pinMode(PWM_PIN, OUTPUT);
  audioOn();
}


void loop() {
  // no-op (for the moment)
  // we'll handle the midi notes / updating variables that start/stop oscilators etc here
}


ISR(PWM_INTERRUPT)
{
  // We have a timer which interrupts us 31250 times per second. Each time that happens we calculate the
  // height of the current wave and set that as the PWM duty cycle value.

  uint8_t output;

  PWM_VALUE = output;
}

