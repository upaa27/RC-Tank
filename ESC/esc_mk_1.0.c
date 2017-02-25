/******************************************************
  * Electronic Speed Controller                       *
  * Revision 1.0     January 15, 2017                 *
  * Phoenix Kang, Jordan Grelling, William Fehrnstrom *
  *****************************************************
*/

#define F_CPU           16000000UL
#define __DELAY_BACKWARD_COMPATIBLE__
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>

void Setup_IO(void);
void Setup_Interrupts(void);
void Setup_PWM(void);

volatile bool Command;
volatile uint8_t Pulse_Width;
volatile uint16_t Start_Time;

int main(void) {
  Setup_IO();
  Setup_PWM();
  Setup_Interrupts();
  for(;;) {}
  return 0;
}

void Setup_IO() {
  DDRD = (1 << 3); //DDRD pin 3 output
  DDRB = (1 << 3); //DDRB pin 3 output
}

void Setup_Interrupts() {
  cli();
  //External interrupt setup
  EICRA = (1 << ISC01) | (1 << ISC00); //Rising edge triggers ISR
  EIMSK = (1 << INT0); //External interrupt on INT0 enable
  //Timer 1 input capture
  TCCR1B = (1 << ICNC1) | (1 << ICES1); //Noise canceler and input capture interrupt on rising
  TIMSK1 = (1 << ICIE1); //Input capture interrupt enable
  // PCMSK2 = (1 << PCINT21); //PD5
  // PCICR = (1 << PCIE2); //Pin change interrupt 2 enable
  sei(); //Global interrupt enable
}

void Setup_PWM() {
  TCCR2A = (1 << COM2A1) | (1 << COM2B1); //OC2A and OC2B center alligned
  TCCR2B = (1 << WGM20) | (1 << CS21); //8 prescaler
}

ISR(TIMER1_CAPT_vect) {
  if(TCCR1B & (1 << ICES1)) { //If the ISR triggers on rising
    Start_Time = ICR1;
    TCCR1B &= ~(1 << ICES1);
  }
  else {
    if(Command)
      OCR2A = Pulse_Width/256;
    else
      OCR2B = Pulse_Width/256;
  }
}

ISR(INT0_vect) {
  Command = Command ? false : true;
}
