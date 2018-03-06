

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>


// output on arduino pin 3
#define PWM_PIN       3
#define PWM_VALUE     OCR2B
#define PWM_INTERRUPT TIMER2_OVF_vect

#define NUM_OSCILATORS 5

// General model is that we have a wavetable (below) of 256 discrete samples representing a sine wave which we play back at the desired frequency

// To play at 440 Hz (middle A) we have 440*256 = 112,640 discrete samples to play through in one second
// Consider the speed at which the interrupts are happening - 31,250 times per second
// So for each interrupt we need to jump forward 112640/31250 = 3.60448 entries in the wavetable
// We'll call that the "increment"

// More generally, each musical note can be represented as an increment. The
// best way to respresent that is to work in 16 bit using some multiple of 2
// and then shift that down so we don't lose too much resolution due to rounding.

// So, we can represent 440 Hz as 3691 = 440 * 256 * (2^10) / 31250
// More generally:
// >>> notes = [261.6, 277.2, 293.7, 311.1, 329.6, 349.2, 370.0, 392.0, 415.3, 440.0, 466.2, 493.9]
// >>> increments = [round(n * 256 * (2**10) / 31250) for n in notes]
// >>> [2194, 2325, 2464, 2610, 2765, 2929, 3104, 3288, 3484, 3691, 3911, 4143]
// TODO should we be shifting by 8 bits instead?

// Note that we only need an octave worth and then can shift up / down to get the other octaves


// A sine wave represented as 256 discrete samples
// Note that we can, and will ultimately, represent different waves (square and saw) in the same way
// >>> inc = math.pi * 2 / 256
// >>> [int(256 - (128 * (1 + math.cos(inc*i)))) for i in range(256)]
const char wavetable[] PROGMEM = {
  0, 1, 1, 1, 1, 1, 2, 2, 3, 4, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14, 16, 17, 19, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40,
  43, 45, 47, 50, 52, 55, 57, 60, 63, 65, 68, 71, 74, 77, 80, 82, 85, 88, 91, 94, 97, 100, 104, 107, 110, 113, 116, 119,
  122, 125, 128, 132, 135, 138, 141, 144, 147, 150, 153, 157, 160, 163, 166, 169, 172, 175, 177, 180, 183, 186, 189, 192,
  194, 197, 200, 202, 205, 207, 210, 212, 214, 217, 219, 221, 223, 225, 227, 229, 231, 233, 235, 237, 238, 240, 241, 243,
  244, 246, 247, 248, 249, 250, 251, 252, 253, 253, 254, 255, 255, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 255, 255,
  254, 253, 253, 252, 251, 250, 249, 248, 247, 246, 244, 243, 241, 240, 238, 237, 235, 233, 231, 229, 227, 225, 223, 221, 219, 217,
  214, 212, 210, 207, 205, 202, 200, 197, 194, 192, 189, 186, 183, 180, 177, 175, 172, 169, 166, 163, 160, 157, 153, 150, 147, 144,
  141, 138, 135, 132, 129, 125, 122, 119, 116, 113, 110, 107, 104, 100, 97, 94, 91, 88, 85, 82, 80, 77, 74, 71, 68, 65, 63, 60,
  57, 55, 52, 50, 47, 45, 43, 40, 38, 36, 34, 32, 30, 28, 26, 24, 22, 20, 19, 17, 16, 14, 13, 11, 10, 9, 8, 7, 6,
  5, 4, 4, 3, 2, 2, 1, 1, 1, 1, 1
};




// oscilator tracks where we're currently at in the wavetable. On each interrupt we add the increment to that
// to give the new wavetable position.
// NOTE we use 16 bits for each of the below to maintain resolution and will then
// shift it down at the final step - TODO is there a way to just use the msb?
typedef struct
{
   uint16_t oscilator;
   uint16_t increment;
} Oscilator;

Oscilator volatile oscilators[NUM_OSCILATORS];


void audioOn() {
  // set up Timer 2 to handle our PWM
  // we use the interrupt to trigger the generation of samples
  // we use the pwm for outputs
  // Set up PWM to 31.25kHz, phase accurate
  // phase accurute = runs fom 0 -> 255 -> back to down 0 again

  // COM2B1 = Clear OC2B on Compare Match
  // WGM20  = PWM, Phase Correct
  // CS20   = No prescaling
  TCCR2A = _BV(COM2B1) | _BV(WGM20);
  TCCR2B = _BV(CS20);
  TIMSK2 = _BV(TOIE2);
}


// -----------------------------------------------------------------------------


void setup() {
  pinMode(PWM_PIN, OUTPUT);
  audioOn();
}


void loop() {
  // no-op (for the moment)
  // we'll handle the midi notes / updating variables that start/stop oscilators etc here

  oscilators[0].increment = 1097; // C3
  oscilators[1].increment = 1305; // E
  oscilators[2].increment = 1552; // G
  oscilators[3].increment = 2194; // C4
  delay(100);

  oscilators[0].increment = 0; // C3
  oscilators[1].increment = 0; // E
  oscilators[2].increment = 0; // G
  oscilators[3].increment = 0; // C4
  delay(500);
}


ISR(PWM_INTERRUPT)
{
  // We have a timer which interrupts us 31250 times per second. Each time that happens we calculate the
  // amplitude of the wave and set that as the PWM duty cycle value.
  // TODO does this need to be halfed along the way because of the phase accurate PWM setting?

  uint8_t output = 0;

  for (uint8_t i = 0; i < NUM_OSCILATORS; i++) {
    oscilators[i].oscilator += oscilators[i].increment;
    output += (pgm_read_byte(&wavetable[(oscilators[i].oscilator >> 10)])>>1);
  }

  PWM_VALUE = output;
}

