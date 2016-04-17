
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

// Modified by Charles Rymal, April, 2016
// 
// A linked list structure is used to control an arbitrary number of triacs while
// using only timer 1 of the Atmega chip.
// 
// Instructions:
// To leave the triac off, set the dimmer to 0
// To turn the triac to fully on, set the dimmer to a low value, like 10. A value below
// 10 but greater than 0 will pulse the triac too early in the cycle and the triac will not
// stay on, resulting in flicker
// 
// Bugs:
// If two channels are set to a very close level, they will both be brighter than if only
// one was on.

#define DIMMER_TIMER (1)
#define DIMMER_WIRE (1)
#define DIMMER_ZCI (1)

#if DIMMER_WIRE
#include <Wire.h>
#endif


//#include <avr/io.h>
//#include <avr/interrupt.h>

#define __ARDUINO
//#define __LINUX



#if DIMMER_TIMER
#include "List.h"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

#if DIMMER_TIMER
Item *	item_gate_h[2];
Item *	item_gate_l[2];
Item *  list;
Item *  list_ptr0;
Item *  list_ptr;

bool changed = true;
#endif


#define DETECT  2  //pin for zero cross detect
#define PULSE   4  //trigger pulse width (counts)
#define COMPARE 1000



#define COUNT_LIMIT (10000)


int half_cycles = 0;

#define CODE_A_SET 0
#define CODE_B_SET 1
#define CODE_A_GET 2
#define CODE_B_GET 3

int gate[2];
int count_write[2];
int count_comp = 0;

//int index = 0;

int code = -1;

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

#if DIMMER_TIMER
void  set_dimmer(int dimmer, unsigned short count);
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void  digWrite(uint8_t dimmer, uint8_t value)
{
  digitalWrite(gate[dimmer], value);
  ++count_write[dimmer];
}

#if DIMMER_WIRE
void hndRequest()
{
  //int code = Wire.read();

  Serial.print("hndRequest code=");
  Serial.println(code);

  if (code == CODE_A_GET) {
    Wire.write((char *) & (item_gate_h[0]->_M_count), 2);
  } else if (code == CODE_B_GET) {
    Wire.write((char *) & (item_gate_h[1]->_M_count), 2);
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
  while (Wire.available()) {
    buf[i] = Wire.read();
    Serial.print(buf[i]);
    Serial.print(" ");
    ++i;
  }
  Serial.println();

  code = buf[0];

  unsigned short value;

  if ((code == CODE_A_SET) || (code == CODE_B_SET)) {
    value = *((short *)(buf + 1));

    Serial.print("value ");
    Serial.println(value);

    if (code == CODE_A_SET) {
      set_dimmer(0, value);
    } else if (code == CODE_B_SET) {
      set_dimmer(1, value);
    }
  }

}
void setup_wire()
{
  //Serial.println("setup wire");
  Wire.begin(8);
  Wire.onReceive(hndReceive);
  Wire.onRequest(hndRequest);
}
#endif



#if DIMMER_TIMER
void  set_dimmer(int dimmer, unsigned short count)
{

  item_gate_h[dimmer]->_M_count = count;

  if (count == 0) {
    item_gate_l[dimmer]->_M_count = 0;
  } else {
    item_gate_l[dimmer]->_M_count = count + PULSE;
  }

  changed = true;
}
void  update_list()
{
  list = list->update(COMPARE, 0);

  list_ptr0 = list;

  while (true) {
    if (list_ptr0 == 0) break;
    if (list_ptr0->_M_count > 0) break;
    list_ptr0 = list_ptr0->_M_next;
  }

  if(list_ptr0) {
    list_ptr0->print();
  }
}
void  setup_list()
{
  // list
  item_gate_h[0] = new Item(0, &digWrite, 0, HIGH);
  item_gate_h[1] = new Item(0, &digWrite, 1, HIGH);
  item_gate_l[0] = new Item(0, &digWrite, 0, LOW);
  item_gate_l[1] = new Item(0, &digWrite, 1, LOW);

  list = item_gate_h[0];
  list->insert(item_gate_h[1]);
  list->insert(item_gate_l[0]);
  list->insert(item_gate_l[1]);
}
#endif

//Interrupt Service Routines

#if DIMMER_ZCI
void zeroCrossingInterrupt() { //zero cross detect

#if DIMMER_TIMER

  if (changed) {
    update_list();
    changed = false;
  }

  list_ptr = list_ptr0;
  
  if(list_ptr) {
    TCCR1B = 0x04; //start timer with divide by 256 input
    TCNT1 = list_ptr->_M_tcnt;   //reset timer - count from zero
  }
#endif

  // debugging
  half_cycles++;

}
#endif

#if DIMMER_TIMER
ISR(TIMER1_COMPA_vect) { //comparator match for OCR1A

  ++count_comp;

  while(true) {
    list_ptr->call();
    list_ptr = list_ptr->_M_next;
    
    if(list_ptr == 0) break;

    if(list_ptr->_M_count == list_ptr->_M_prev->_M_count) continue;

    break;
  }
  
  if (list_ptr == 0) {
    // stop the timer
    TCCR1B = 0x0;
    return;
  }

  if (list_ptr->_M_count >= COUNT_LIMIT) {
    // stop the timer
    TCCR1B = 0x0;
    return;
  }

  TCNT1 = list_ptr->_M_tcnt;

}

void setup_timer()
{
  // set up Timer1
  //(see ATMEGA 328 data sheet pg 134 for more details)
  //Serial.println("setup timer");

  OCR1A = 100;

  TIMSK1 = 0x02;    //enable comparator A
  TCCR1A = 0x00;    //timer control registers set for
  TCCR1B = 0x00;    //normal operation, timer disabled

  OCR1A = COMPARE;
}
#endif


void setup() {

  count_write[0] = 0;
  count_write[1] = 0;

  // debugging
  Serial.begin(9600);

  //Serial.println("setup");


#if DIMMER_WIRE
  setup_wire();
#endif

  gate[0] = 9;
  gate[1] = 10;

  // set up pins

  pinMode(gate[0], OUTPUT);      //triac gate control
  pinMode(gate[1], OUTPUT);      //triac gate control

  
#if DIMMER_TIMER
  setup_list();
  setup_timer();
#endif

#if DIMMER_ZCI

  pinMode(DETECT, INPUT);     //zero cross detect
  digitalWrite(DETECT, HIGH); //enable pull-up resistor

  // set up zero crossing interrupt
  attachInterrupt(0, zeroCrossingInterrupt, RISING);

  //IRQ0 is pin 2. Call zeroCrossingInterrupt
  //on rising signal
#endif


  set_dimmer(0, 100);

}




void loop() { // sample code to exercise the circuit

  delay(15);

  //Serial.println("loop");

  int interval = 60*5;
  
  if (half_cycles > interval) {

    char buf[256];
    sprintf(buf, "%i half cycles  gate 0: %i comp: %i", interval, count_write[0], count_comp);
    Serial.println(buf);
    
    half_cycles -= interval;

    //list->print();
  }

}



