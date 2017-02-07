/******************************************************
  * Electronic Speed Controller                       *
  * Revision 1.0     January 15, 2017                 *
  * Phoenix Kang, Jordan Grelling, William Fehrnstrom *
  *****************************************************
*/

#define F_CPU           16000000UL
#define PULSE_LENGTH    5000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

void Setup_IO(void);
void Setup_Interrupts(void);

volatile bool Direction, Clear;

int main(void) {
  Setup_IO();
  Setup_Interrupts();
  for(;;) {}
  return 0;
}

void Setup_IO() {
  DDRD = (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7); //DDRD pin 3 to 7 output
}

void Setup_Interrupts() {
  cli();
  //External interrupt setup
  EICRA = (1 << ISC01) | (1 << ISC00); //Rising edge triggers ISR
  EIMSK = (1 << INT0); //External interrupt on INT0 enable

  PCMSK1 = (1 << PCINT1) | (1 << PCINT2);

  //Timer 1 input capture
  TCCR1B = (1 << WGM12) | (1 << CS10); //(1 << ICNC1) | (1 << ICES1); //Noise canceler and input capture interrupt on rising
  OCR1A = PULSE_LENGTH;
  PCICR |= (1 << PCIE1); //Pin change interrupt 1 enable
  TIMSK1 &= ~(1 << OCIE1A); //Input capture interrupt enable
  sei(); //Global interrupt enable
}

ISR(TIMER1_COMPA_vect) {
  //Choose phasing
  if(Direction) {
    PORTD >>= 1;
    if(PORTD == 0)
      PORTD |= (1 << 7) | (1 << 6);
    if(PORTD & (1 << 2))
      PORTD |= (1 << 7);
  }
  else {
    PORTD <<= 1;
    if(PORTD & (1 << 7))
      PORTD |= (1 << 2);
    if(PORTD == 0 || PORTD == 0x04)
      PORTD |= (1 << 2) | (1 << 3);
}

ISR(PCINT1_vect) {
  //Generate PWM
  if((Prev >> 2) != (PINB >> 2)) //PCINT2 triggered the interrupt
    TIMSK1 ^= (1 << OCIE1A);
  //Swap phasing
  else if((Prev >> 1) != (PINB >> 1)) //PCINT1 triggered the interrupt
    Direction = Direction ? false : true;
  Prev = PINB;
}

// ISR(TIMER1_CAPT_vect) {
//   if(TCCR1B & (1 << ICES1)) { //If the ISR triggers on rising
//     Start_Time = ICR1;
//     TCCR1B &= ~(1 << ICES1);
//   }
//   else {
//     if(Command)
//       OCR2A = Pulse_Width/256;
//     else
//       OCR2B = Pulse_Width/256;
//   }
// }
