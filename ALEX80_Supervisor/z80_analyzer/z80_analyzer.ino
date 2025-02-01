/*******************************************************************
  Sketch per Arduino MEGA per l'utilizzo della scheda ALEX80 per
  il pilotaggio della scheda dal software Alex80 supervisor

  Informazioni su https://alexa-academy.it e su YouTube:
  https://www.youtube.com/watch?v=qdNUe48BPS8&list=PLm0mPRQw3XGpIOirMFhjB3-4Jtun4GYhv

  Versione 18
  Tolto pilotaggio INT per lasciarlo al PIO
********************************************************************/

#include "Arduino.h"

#define Z80_D0  22
#define Z80_D1  24
#define Z80_D2  26
#define Z80_D3  28
#define Z80_D4  30
#define Z80_D5  32
#define Z80_D6  34
#define Z80_D7  36

#define Z80_A0  23
#define Z80_A1  25
#define Z80_A2  27
#define Z80_A3  29
#define Z80_A4  31
#define Z80_A5  33
#define Z80_A6  35
#define Z80_A7  37
#define Z80_A8  39
#define Z80_A9  41
#define Z80_A10  43
#define Z80_A11  45
#define Z80_A12  47
#define Z80_A13  49
#define Z80_A14  51
#define Z80_A15  53

#define Z80_M1  52
#define Z80_WR  50
#define Z80_RD  48
#define Z80_IORQ 46
#define Z80_MRQ 44
#define Z80_RFSH 42

#define Z80_BREQ 38
#define Z80_BACK 40
#define Z80_INT  A15
#define Z80_NMI  A14

#define Z80_CLK 21
#define Z80_RST 20

#define STOP_CLOCK     B00000000
#define PRESCALER_1    B00000001
#define PRESCALER_8    B00000010
#define PRESCALER_64   B00000011
#define PRESCALER_256  B00000100
#define PRESCALER_1024 B00000101

#define REQUEST_BUS_TIMEOUT_MS 1000  // 1 secondo
#define INTERRUPT_WAIT_TIMEOUT_MS 10000  // 10 secondi

void loopSerial();
void startClock(unsigned int f);
void stopClock();

volatile bool performReset = false;

volatile unsigned int tick = 0;
volatile unsigned int cycle = 0;

unsigned int freq = 50;

void restart(unsigned int f) {
  stopClock();
  doReset();
  cycle = 0;
  tick = 0;
  startClock(f);
}

void enableLog() {
  attachInterrupt(digitalPinToInterrupt(Z80_CLK), ClockTrigger, RISING);
}

void disableLog() {
  detachInterrupt(digitalPinToInterrupt(Z80_CLK));
}

void calcOCR(unsigned int freq, unsigned int* ocr, byte* prescalerMask) {
  long desiredFreq = freq;
  long prescaler;

  if (freq < 2) {
    prescaler = 256;
    *prescalerMask = PRESCALER_256;
  } else if (freq < 20) {
    prescaler = 64;
    *prescalerMask = PRESCALER_64;
  } else if (freq < 150) {
    prescaler = 8;
    *prescalerMask = PRESCALER_8;
  } else if (freq < 30000) {
    prescaler = 1;
    *prescalerMask = PRESCALER_1;
  } else {
    desiredFreq = 30000;
    prescaler = 1;
    *prescalerMask = PRESCALER_1;
  }
  
  *ocr = 16000000L / (prescaler * desiredFreq * 2) + 1;
}

bool requestBus() {
  digitalWrite(Z80_BREQ, LOW);

  unsigned long curTime = millis();

  do {
    bool bus_ack = digitalRead(Z80_BACK);
    if (!bus_ack) { // E' attivo basso
      return true;
    } 
    delay(1);
    //delayMicroseconds(1);
  } while (millis() - curTime < REQUEST_BUS_TIMEOUT_MS);

  return false;
}

void releaseBus() {
  digitalWrite(Z80_BREQ, HIGH);
}

void doReset() {
  digitalWrite(Z80_RST, HIGH);
  performReset = true;
}

void writeData(byte b) {
  digitalWrite(Z80_D0, bitRead(b, 0));
  digitalWrite(Z80_D1, bitRead(b, 1));
  digitalWrite(Z80_D2, bitRead(b, 2));
  digitalWrite(Z80_D3, bitRead(b, 3));
  digitalWrite(Z80_D4, bitRead(b, 4));
  digitalWrite(Z80_D5, bitRead(b, 5));
  digitalWrite(Z80_D6, bitRead(b, 6));
  digitalWrite(Z80_D7, bitRead(b, 7));
}

byte readData() {
  bool d7 = digitalRead(Z80_D7);
  bool d6 = digitalRead(Z80_D6);
  bool d5 = digitalRead(Z80_D5);
  bool d4 = digitalRead(Z80_D4);
  bool d3 = digitalRead(Z80_D3);
  bool d2 = digitalRead(Z80_D2);
  bool d1 = digitalRead(Z80_D1);
  bool d0 = digitalRead(Z80_D0);

  byte data_bus = d7<<7 | d6<<6 | d5<<5 | d4<<4 | d3<<3 | d2<<2 | d1<<1 | d0;

  return data_bus;
}

void writeAddress(word w) {
  digitalWrite(Z80_A0, bitRead(w, 0));
  digitalWrite(Z80_A1, bitRead(w, 1));
  digitalWrite(Z80_A2, bitRead(w, 2));
  digitalWrite(Z80_A3, bitRead(w, 3));
  digitalWrite(Z80_A4, bitRead(w, 4));
  digitalWrite(Z80_A5, bitRead(w, 5));
  digitalWrite(Z80_A6, bitRead(w, 6));
  digitalWrite(Z80_A7, bitRead(w, 7));
  digitalWrite(Z80_A8, bitRead(w, 8));
  digitalWrite(Z80_A9, bitRead(w, 9));
  digitalWrite(Z80_A10, bitRead(w, 10));
  digitalWrite(Z80_A11, bitRead(w, 11));
  digitalWrite(Z80_A12, bitRead(w, 12));
  digitalWrite(Z80_A13, bitRead(w, 13));
  digitalWrite(Z80_A14, bitRead(w, 14));
  digitalWrite(Z80_A15, bitRead(w, 15));
}

void setDataBusOutput(bool isOutput) {
  pinMode(Z80_D0, isOutput?OUTPUT:INPUT);
  pinMode(Z80_D1, isOutput?OUTPUT:INPUT);
  pinMode(Z80_D2, isOutput?OUTPUT:INPUT);
  pinMode(Z80_D3, isOutput?OUTPUT:INPUT);
  pinMode(Z80_D4, isOutput?OUTPUT:INPUT);
  pinMode(Z80_D5, isOutput?OUTPUT:INPUT);
  pinMode(Z80_D6, isOutput?OUTPUT:INPUT);
  pinMode(Z80_D7, isOutput?OUTPUT:INPUT);
}

void setAddCtrlBusOutput(bool isOutput) {
  pinMode(Z80_A0, isOutput?OUTPUT:INPUT);
  pinMode(Z80_A1, isOutput?OUTPUT:INPUT);
  pinMode(Z80_A2, isOutput?OUTPUT:INPUT);
  pinMode(Z80_A3, isOutput?OUTPUT:INPUT);
  pinMode(Z80_A4, isOutput?OUTPUT:INPUT);
  pinMode(Z80_A5, isOutput?OUTPUT:INPUT);
  pinMode(Z80_A6, isOutput?OUTPUT:INPUT);
  pinMode(Z80_A7, isOutput?OUTPUT:INPUT);
  pinMode(Z80_A8, isOutput?OUTPUT:INPUT);
  pinMode(Z80_A9, isOutput?OUTPUT:INPUT);
  pinMode(Z80_A10, isOutput?OUTPUT:INPUT);
  pinMode(Z80_A11, isOutput?OUTPUT:INPUT);
  pinMode(Z80_A12, isOutput?OUTPUT:INPUT);
  pinMode(Z80_A13, isOutput?OUTPUT:INPUT);
  pinMode(Z80_A14, isOutput?OUTPUT:INPUT);
  pinMode(Z80_A15, isOutput?OUTPUT:INPUT);

  pinMode(Z80_MRQ, isOutput?OUTPUT:INPUT);
  pinMode(Z80_RD, isOutput?OUTPUT:INPUT);
  pinMode(Z80_WR, isOutput?OUTPUT:INPUT);
}

word decodeAddress() {
  word add = 0;

  bitWrite(add, 0, digitalRead(Z80_A0)); 
  bitWrite(add, 1, digitalRead(Z80_A1)); 
  bitWrite(add, 2, digitalRead(Z80_A2)); 
  bitWrite(add, 3, digitalRead(Z80_A3)); 
  bitWrite(add, 4, digitalRead(Z80_A4)); 
  bitWrite(add, 5, digitalRead(Z80_A5)); 
  bitWrite(add, 6, digitalRead(Z80_A6)); 
  bitWrite(add, 7, digitalRead(Z80_A7)); 
  bitWrite(add, 8, digitalRead(Z80_A8)); 
  bitWrite(add, 9, digitalRead(Z80_A9)); 
  bitWrite(add, 10, digitalRead(Z80_A10)); 
  bitWrite(add, 11, digitalRead(Z80_A11)); 
  bitWrite(add, 12, digitalRead(Z80_A12)); 
  bitWrite(add, 13, digitalRead(Z80_A13)); 
  bitWrite(add, 14, digitalRead(Z80_A14)); 
  bitWrite(add, 15, digitalRead(Z80_A15)); 

  return add;
}

byte decodeData() {
  bool d7 = digitalRead(Z80_D7);
  bool d6 = digitalRead(Z80_D6);
  bool d5 = digitalRead(Z80_D5);
  bool d4 = digitalRead(Z80_D4);
  bool d3 = digitalRead(Z80_D3);
  bool d2 = digitalRead(Z80_D2);
  bool d1 = digitalRead(Z80_D1);
  bool d0 = digitalRead(Z80_D0);

  byte data_bus = d7<<7 | d6<<6 | d5<<5 | d4<<4 | d3<<3 | d2<<2 | d1<<1 | d0; 

  return data_bus;
}

void ClockTrigger() {
  word add = decodeAddress();
  byte data = decodeData();

  bool z80_rd = digitalRead(Z80_RD);   
  bool z80_wr = digitalRead(Z80_WR);   
  bool z80_iorq = digitalRead(Z80_IORQ); 
  bool z80_mreq = digitalRead(Z80_MRQ); 
  bool z80_rfsh = digitalRead(Z80_RFSH);   
  bool z80_m1 = digitalRead(Z80_M1);
  byte control = z80_rd<<5 | z80_wr<<4 | z80_iorq<<3 | z80_mreq<<2 | z80_rfsh<<1 | z80_m1; 

  byte message[] = {0xAA, 0x00, 0x00, 0x4, highByte(add), lowByte(add), data, control};

  Serial.write(message, sizeof(message));
  Serial.flush();
}

// Clock cycle
ISR(TIMER1_COMPA_vect) {
  tick++;
  if (tick%2 == 0) {
    digitalWrite(Z80_CLK, LOW);
    cycle++;
  } else {
    digitalWrite(Z80_CLK, HIGH);
  }

  if (performReset && cycle > 7) {
    performReset = false;
    digitalWrite(Z80_RST, LOW);
  }
}

void startClock(unsigned int f) {
  pinMode(Z80_CLK, OUTPUT);
  digitalWrite(Z80_CLK, HIGH);
  digitalWrite(Z80_RST, LOW);

  unsigned int ocr;
  byte prescalerMask;

  freq = f;

  calcOCR(f, &ocr, &prescalerMask);

  cli();

  TCCR1A = 0;
  TCCR1B = TCCR1B & B11100000 | B00001000 | prescalerMask;
  TCNT1 = 0;  // Inizializza il counter
  OCR1A = ocr;
  TIMSK1 = 0b00000010; // Abilita interrupt su conteggio OCR1A

  sei();
}

void stopClock() {
  pinMode(Z80_CLK, INPUT);

  cli();

  TCCR1A = 0;
  TCCR1B = TCCR1B & B11100000 | B00001000 | STOP_CLOCK;
  TCNT1 = 0;  // Inizializza il counter
  TIMSK1 = 0b00000000; // Disabilita interrupt

  sei();

}

void writeRAM(byte data, word address) {
  writeAddress(address);
  delayMicroseconds(3);
  digitalWrite(Z80_RD, 1);
  delayMicroseconds(3);
  digitalWrite(Z80_MRQ, 0);
  delayMicroseconds(3);
  setDataBusOutput(true);
  writeData(data);
  delayMicroseconds(3);
  digitalWrite(Z80_WR, 0);
  delayMicroseconds(1);
  digitalWrite(Z80_WR, 1);
  delayMicroseconds(3);
  setDataBusOutput(false);
  delayMicroseconds(3);
  digitalWrite(Z80_MRQ, 1);
  delayMicroseconds(3);
}

/*
byte readCycle(word address) {
  Serial3.println(address, HEX);
  digitalWrite(Z80_WR, 1);
  writeAddress(address);
  digitalWrite(Z80_MRQ, 0);
  delayMicroseconds(3);
  digitalWrite(Z80_RD, 0);
  delayMicroseconds(3);
  byte data = readData();
  digitalWrite(Z80_RD, 1);
  delayMicroseconds(3);
  digitalWrite(Z80_MRQ, 1);
  delayMicroseconds(3);

  Serial3.println(data, HEX);

  return data;
}
*/

void continuousReadCycle(word address, byte* data, unsigned int size) {
  digitalWrite(Z80_WR, 1);
  writeAddress(address);
  delayMicroseconds(1);

  digitalWrite(Z80_MRQ, 0);
  digitalWrite(Z80_RD, 0);

  for (unsigned int idx = 0; idx < size; ++idx) {
    writeAddress(address + idx);
    delayMicroseconds(1);
    data[idx] = readData();
  }

  digitalWrite(Z80_RD, 1);
  digitalWrite(Z80_MRQ, 1);
  delayMicroseconds(1);
}

/*
void readMemoryBanks(char* msgPtr, int startBank, int numBanks) {
  int sBank = startBank;
  if (sBank > 4096) {
    sBank = 0;
  }

  int tBanks = numBanks;
  if (numBanks > 4096) {
    tBanks = 4096;
  }

  for (int bank=0; bank < tBanks; ++bank) {
    for (int i=0; i<16; ++i) {
      msgPtr[bank*16 + i] = readCycle((sBank+bank)*16 + i);
    }
  } 
}
*/

void setup() {
  Serial.begin(115200);
 // Serial3.begin(9600);
//  Serial3.println("Start");

  setDataBusOutput(false);
  setAddCtrlBusOutput(false);

  pinMode(Z80_M1, INPUT);
  pinMode(Z80_WR, INPUT);
  pinMode(Z80_RD, INPUT);
  pinMode(Z80_IORQ, INPUT);
  pinMode(Z80_MRQ, INPUT);
  pinMode(Z80_RFSH, INPUT);

  pinMode(Z80_BREQ, OUTPUT);
  pinMode(Z80_BACK, INPUT);
  pinMode(Z80_INT, INPUT);
  pinMode(Z80_NMI, OUTPUT);

  digitalWrite(Z80_BREQ, HIGH);
  digitalWrite(Z80_NMI, HIGH);

  pinMode(Z80_CLK, INPUT);  // Eventualmente sarà messo in OUTPUT nella startClock
  pinMode(Z80_RST, OUTPUT);

  stopClock();
}

void loop() {
  loopSerial();
}
