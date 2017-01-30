/******************************************************
  * Electronic Speed Controller                       *
  * Revision 1.0     January 15, 2017                 *
  * Phoenix Kang, Jordan Grelling, William Fehrnstrom *
  *****************************************************
*/

#define F_CPU           16000000UL
#define PULSE_LENGTH    50000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

void Setup_IO(void);
void Setup_Interrupts(void);
//void Setup_PWM(void);

volatile bool Direction;//Command;
volatile uint8_t Phase = 3;//, Pulse_Width;
//volatile uint16_t Start_Time;

int main(void) {
  Setup_IO();
//  Setup_PWM();
  Setup_Interrupts();
  for(;;) {}
  return 0;
}

void Setup_IO() {
  DDRD = (1 << 3) | (1 << 4) | (1 << 5); //DDRD pin 3 output
  //DDRB = (1 << 3); //DDRB pin 3 output
}

void Setup_Interrupts() {
  cli();
  //External interrupt setup
  EICRA = (1 << ISC01) | (1 << ISC00); //Rising edge triggers ISR
  EIMSK = (1 << INT0); //External interrupt on INT0 enable

  PCMSK2 = (1 << PCINT16);

  //Timer 1 input capture
  TCCR1B = (1 << WGM12); //(1 << ICNC1) | (1 << ICES1); //Noise canceler and input capture interrupt on rising
  OCR1A = PULSE_LENGTH;
  PCICR |= (1 << PCIE2);
  TIMSK1 &= ~(1 << OCIE1A); //Input capture interrupt enable
  // PCMSK2 = (1 << PCINT21); //PD5
  // PCICR = (1 << PCIE2); //Pin change interrupt 2 enable
  sei(); //Global interrupt enable
}

// void Setup_PWM() {
//   TCCR2A = (1 << COM2A1) | (1 << COM2B1) | (1 << WGM20); //OC2A and OC2B center alligned
//   TCCR2B = (1 << CS21); //8 prescaler
// }

ISR(TIMER1_COMPA_vect) {
  if(Direction) {
    PORTD = (1 << Phase);
    Phase = Phase == 5 ? 3 : Phase + 1;
  }
  else {
    PORTD = (1 << Phase);
    Phase = Phase == 3 ? 5 : Phase - 1;
  }
}

ISR(PCINT2_vect) {
  TIMSK1 ^= (1 << OCIE1A);
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

ISR(INT0_vect) {
  Direction = EICRA & (1 << ISC00) ? true : false;
  EICRA ^= (1 << ISC00);
  //Command = Command ? false : true;
}
