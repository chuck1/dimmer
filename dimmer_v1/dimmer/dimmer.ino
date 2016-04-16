
// AC Control V1.1
//
// This arduino sketch is for use with the heater 
// control circuit board which includes a zero 
// crossing detect fucntion and an opto-isolated triac.
//
// AC Phase control is accomplished using the internal 
// hardware timer1 in the arduino
//
// Timing Sequence
// * timer is set up but disabled
// * zero crossing detected on pin 2
// * timer starts counting from zero
// * comparator set to "delay to on" value
// * counter reaches comparator value
// * comparator ISR turns on triac gate
// * counter set to overflow - pulse width
// * counter reaches overflow
// * overflow ISR truns off triac gate
// * triac stops conducting at next zero cross


// The hardware timer runs at 16MHz. Using a
// divide by 256 on the counter each count is 
// 16 microseconds.  1/2 wave of a 60Hz AC signal
// is about 520 counts (8,333 microseconds).

#include <Wire.h>
#include <avr/io.h>
#include <avr/interrupt.h>


#define DETECT  2  //pin for zero cross detect
#define GATE_A  9  //pin for triac gate
#define GATE_B 10  //pin for triac gate
#define PULSE   4  //trigger pulse width (counts)

struct Item {
	Item	insert(Item * i)
	{
		if(_M_count >= i->_M_count) {
			
		}
	}
};

unsigned short dimmer_A = 0;
unsigned short dimmer_B = 200;

int half_cycles = 0;


#define CODE_A_SET 0
#define CODE_B_SET 1
#define CODE_A_GET 2
#define CODE_B_GET 3

int index = 0;

int code = -1;

void hndRequest()
{
  

  
  //int code = Wire.read();

  Serial.print("hndRequest code=");
  Serial.println(code);
  
  if(code == CODE_A_GET) {
    Wire.write((char *)&dimmer_A, 2);
  } else if(code == CODE_B_GET) {
    Wire.write((char *)&dimmer_B, 2);
  } else {
    Serial.println("sendData code error");
  }
  return;
}
void hndReceive(int c)
{ 
  Serial.print("receive ");
  Serial.println(c);

  int i = 0;
  unsigned char buf[16];
  while(Wire.available()) {
    buf[i] = Wire.read();
    Serial.print(buf[i]);
    Serial.print(" ");
    ++i;
  }
  Serial.println();

  code = buf[0];

  unsigned short value;

  if((code == CODE_A_SET) || (code == CODE_B_SET)) {
    value = *((short *)(buf+1));
    
    Serial.print("value ");
    Serial.println(value);
    
    if(code == CODE_A_SET) {
      dimmer_A = value;
    } else if(code == CODE_B_SET) {
      dimmer_B = value;
    }
  }
  
}

void setup_wire()
{
  Wire.begin(8);
  Wire.onReceive(hndReceive);
  Wire.onRequest(hndRequest);
}
void setup(){

  setup_wire();

  // debugging
  Serial.begin(9600);
  
  // set up pins
  pinMode(DETECT, INPUT);     //zero cross detect
  digitalWrite(DETECT, HIGH); //enable pull-up resistor
  pinMode(GATE_A, OUTPUT);      //triac gate control
  pinMode(GATE_B, OUTPUT);      //triac gate control

  // set up Timer1 
  //(see ATMEGA 328 data sheet pg 134 for more details)
  OCR1A = 100;      //initialize the comparator
  OCR1B = 100;      //initialize the comparator
  TIMSK1 = 0x07;    //enable comparator A, comparator B, and overflow interrupts
  TCCR1A = 0x00;    //timer control registers set for
  TCCR1B = 0x00;    //normal operation, timer disabled



  TIMSK2 = 0x01;    // overflow interrupts
  TCCR2A = 0x00;
  TCCR2B = 0x00;
  
  // set up zero crossing interrupt
  attachInterrupt(0, zeroCrossingInterrupt, RISING);
  
  //IRQ0 is pin 2. Call zeroCrossingInterrupt 
  //on rising signal
}

//Interrupt Service Routines
int step[2];
void zeroCrossingInterrupt(){ //zero cross detect   
  TCCR1B = 0x04; //start timer with divide by 256 input
  TCNT1 = 0;   //reset timer - count from zero

  // debugging
  half_cycles++;

  step[0] = 0;
  step[1] = 0;

  OCR1A = dimmer_A;
  OCR1B = dimmer_B;
}



ISR(TIMER1_COMPA_vect){ //comparator match for OCR1A

  if(step[0] == 0) {
    digitalWrite(GATE_A,HIGH);  //set triac gate to high
    OCR1A = dimmer_A + PULSE;
    step[0] = 1;
  } else {
    digitalWrite(GATE_A,LOW);  //set triac gate to high
    OCR1A = dimmer_A;
    step[0] = 0;
  }

}

ISR(TIMER1_COMPB_vect){ //comparator match for OCR1B
  
  if(step[1] == 0) {
    digitalWrite(GATE_B,HIGH);  //set triac gate to high
    OCR1B = dimmer_B + PULSE;
    step[1] = 1;
  } else {
    digitalWrite(GATE_B,LOW);  //set triac gate to high
    OCR1B = dimmer_B;
    step[1] = 0;
  }

}



void loop(){ // sample code to exercise the circuit

  
  // change dimmer value
 
       //set the compare register brightness desired.
  OCR1B = dimmer_B;

  OCR2A = dimmer_A + PULSE;     //set the compare register brightness desired.
  OCR2B = dimmer_B + PULSE;
  
  delay(15);                             
  
  
  if(half_cycles > 120) {
    Serial.print("60 cycles\n");
    half_cycles -= 120;
  }

}


