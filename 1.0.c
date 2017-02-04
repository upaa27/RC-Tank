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
//void Setup_PWM(void);

volatile bool Direction, Clear;//Command;
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
  DDRD = (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7); //DDRD pin 3 to 7 output
  DDRB = (1 << 0); //DDRB pin 0 output
}

void Setup_Interrupts() {
  cli();
  //External interrupt setup
  EICRA = (1 << ISC01) | (1 << ISC00); //Rising edge triggers ISR
  EIMSK = (1 << INT0); //External interrupt on INT0 enable

  PCMSK2 = (1 << PCINT17);

  //Timer 1 input capture
  TCCR1B = (1 << WGM12) | (1 << CS10); //(1 << ICNC1) | (1 << ICES1); //Noise canceler and input capture interrupt on rising
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
  if(Clear) {
    PORTD = 0;
    PORTB = 0;
    Clear = false;
  }
  //Choose phasing
  if(Direction) {
    //2 ouptuts high at a time 60 degrees apart
    if(Phase % 2 == 0) {
      if(Phase == 8)
        PORTB |= (1 << 0);
      else
        PORTD |= (1 << Phase);
      Phase = Phase == 8 ? 3 : Phase + 1;
    }

    else {
      //Prevent more than 2 outputs being high
      if(Phase > 4)
        PORTD &= ~(1 << Phase - 2);
      PORTD |= (1 << Phase);
      Phase = Phase == 8 ? 3 : Phase + 1;
    }
  }
  else {
    if(Phase % 2 == 0) {
      if(Phase < 7)
        PORTD &= ~(1 << Phase + 2);
      if(Phase == 8)
        PORTB = (1 << 0);
      else
        PORTD = (1 << Phase);
      Phase = Phase == 3 ? 8 : Phase - 1;
    }
    else {
      if(Phase == 7)
        PORTD &= ~(1 << 3);
      PORTD |= (1 << Phase);
      Phase = Phase == 3 ? 8 : Phase - 1;
    }
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

//Selects rotation
ISR(INT0_vect) {
  Clear = true;
  Direction = EICRA & (1 << ISC00) ? true : false;
  EICRA ^= (1 << ISC00); //Toggle edge selection
  //Command = Command ? false : true;
}
