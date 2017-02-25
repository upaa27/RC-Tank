/******************************************************
  * RC Railgun Tank Controller                        *
  * Revision 1.0     January 12, 2017                 *
  * Phoenix Kang, Jordan Grelling, William Fehrnstrom *
  *****************************************************
*/

#define Kp  1
#define Ki  0.1

void Setup_Outputs();
void Setup_Timers();
void Setup_Serial();
void Start_SMPS();
void Motor_Control();
void FIRE();

bool Connected = false;

uint8_t Ready[] = {0xFC, 0xAD, 0xDC, 0xA8, 0xAF, 0x78};
uint8_t Fire[] = {0xF1, 0x43, 0xDA, 0x33};

volatile uint8_t CHARGE, Movement_Op, Vol_Data_H, Vol_Data_L, r, data;
volatile uint16_t Cap_Voltage;
volatile bool ARMED = false;

void setup() {
  Setup_Outputs();
  Setup_Serial();
  cli();
  Setup_Timers();
  sei();
}

void Setup_Outputs() {
  DDRB = (1 << 3);
  DDRD = (1 << 2) | (1 << 3) | (1 << 4);
}

void Setup_Timers() {
  //Timer 1 CTC
  TCCR1A = 0;
  TCCR1B = (1 << CS11) | (1 << CS10) | (1 << WGM12);
  OCR1A = 1;
  //Timer 2 Center alligned on both channels
  TCCR2A = (1 << COM2B1) | (1 << COM2A1) | (1 << WGM20);
  TCCR2B = (1 << CS21);
  //Right motor
  OCR2A = 0;
  //Left motor
  OCR2B = 0;
//Enabe Timer 1 output compare interrupt
  TIMSK1 |= (1 << OCIE1A);
}

void Setup_Serial() {
  Serial.begin(9600);
  Serial.write("a");
  while(Connected == false) {
      if(Serial.available() && Serial.read() == 0xFE)
        Connected = true;
  }
  //Tell the transmitter that the robot is ready
  for(int i = 0; i < 6; i++)
    Serial.write(Ready[i]);
  Serial.print("done");
}

void Start_SMPS() {
  PORTD |= (1 << 4);
}

//Motor Control
void Motor_Control() {
  if(Serial.available())
  Movement_Op = Serial.read();
    //Move Forward
    if(Movement_Op == 0xFA && Serial.available()) {
      OCR2A = Serial.read();
      OCR2B = OCR2A;
    }
    //Turn Right
    else if(Movement_Op == 0xBA && Serial.available())
      OCR2B = Serial.read();
    //Turn left
    else if(Serial.available()) 
      OCR2A = Serial.read();
}

//Fire railgun
void FIRE() {
  r = 0;
  data = 0;
      for(int i = 0; i < 4; i++) {
        if(Serial.available())
          data = Serial.read();
        r = data == Fire[i] ? r+1 : r;
      }
      if(r == 4) {
        PORTD |= (1 << 2);
        //Fire command recieved
        Serial.write(0x69);
      }
}

ISR(TIMER1_COMPA_vect) {
  //Checks to see if communication is available and if the command is a power adjust command
//if(Serial.available() && Serial.read() == 0x01) {
//  if(Serial.read() == 0xFE && Serial.available()) {
//    if(Serial.read() != 0xFE) {
//      Setpoint = Serial.read();
//      Start_SMPS();
//    }
//  }
//}

//Send the capacitor bank's voltage in 2 bytes
Cap_Voltage = analogRead(0);
Vol_Data_H = (Cap_Voltage >> 8);
Vol_Data_L = Cap_Voltage;

Serial.write(Vol_Data_H);
Serial.write(Vol_Data_L);

  //Checks to see if communication is available and if the command is a movement command
if(Serial.available() && Serial.read() == 0x03) {
  if(Serial.read() == 0xFD && Serial.available()) {
//    if(Serial.read() != 0xFD)
      Motor_Control();
  }
}
//Physical firing of railgun
  //If the charge button is determined to be pressed, start the SMPS
//  if(CHARGE == 255 && Cap_Voltage > Setpoint) 
//    TCCR1A |= (1 << COM1A1);
  //Railgun is ready to be fired
//  else if(Setpoint - 5 > Cap_Voltage && Cap_Voltage - 5 > Setpoint)
//    ARMED = true;
  //Determines if the charge button is being pressed
//  else if((_BV(PD3) & PIND) && CHARGE < 255)
//    CHARGE++;
}

void loop() {}
