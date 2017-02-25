#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL
#define BAUD 9600
#define BAUDRATE ((F_CPU)/(BAUD * 16UL) - 1)

void setup_uart(void);

volatile unsigned char inbuf[128];
volatile unsigned char next_available_index_in = 0;
unsigned char outbuf[128];
unsigned char next_available_index_out = 0;

void setup(){
  cli(); //Disable interrupts
  setup_uart(); //Configure the Universal Asynchronous Rx/Tx
  sei(); //Enable interrupts
}

void setup_uart(void){
  UBRR0H = (BAUDRATE >> 8); //Shift Half of the Baud Rate to fit on the left side of the two byte register UBRRH.
  UBRR0L = BAUDRATE; //Fit the other half on the right side of the two byte register UBRRH.
  UCSR0B = (1 << TXEN0)|(1 << RXEN0); //Enable Transmission and Reception.
  UCSR0B = (1 << RXCIE0);//|(1 << TXCIE); //Enable interrupts
  //First, select that we want to set UART Control and Status Register C.
  //Second, configure the number of data bits in a serial frame to be 8.
  UCSR0C = (1 << UMSEL)|(1 << UCSZ0)|(1 << UCSZ1);
}

void transmit_data(unsigned char data){
  while(!(UCSR0A) & (1 << UDRE0)); //Wait for the UDRE Flag bit to indicate that the UART Data Register is empty
  UDR0 = data; //Then write the data into the UART Data Register
}

unsigned char receive_data(void){
  while(!(UCSR0A) & (1 << RXC0)); //Wait for the RXC flag bit to indicate that the entire byte has been received
  return UDR0; //Take the byte from the UDR
}

unsigned char* receive_message(){
  unsigned char* msg_in; //Construct a char pointing to inbuf's address
  memcpy(msg_in, inbuf, sizeof(inbuf)); //Set the messsage's value to inbuf
  memset(&inbuf[0], 0, sizeof(inbuf)); //Clear inbuf
  return msg_in; //return the pointer to the received message
}

void transmit_message(unsigned char* msg, unsigned char msg_length){
  if(msg_length < sizeof(outbuf)){ //If the message is less long than the size of the outbuffer
    memcpy(outbuf, msg, msg_length); //Copy the message into the outbuffer
  }
  unsigned char i = 0; //construct a counter variable
  while(i < sizeof(outbuf)){ //Iterate through the array and transmit each piece
    transmit_data(outbuf[i]); //Place each byte in UDR
    i++; //Incrememnt i
  }
  memset(&outbuf[0], 0, sizeof(outbuf)); //Set outbuf's memory location to 0
}

ISR(USART_RX_vect){
  inbuf[next_available_index_in] = receive_data(); //Place the incoming byte in the longer term incoming buffer
  next_available_index_in++; //increment the variable storing the first available position to put a byte
}

void loop(){}
