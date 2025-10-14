/*******************************************************************
  Sketch per Arduino MEGA per l'utilizzo della scheda ALEX80 per
  il pilotaggio della scheda dal software Alex80 supervisor
  in fase di sviluppo
  v 0.1

  Informazioni su https://alexa-academy.it e su YouTube:
  https://www.youtube.com/watch?v=qdNUe48BPS8&list=PLm0mPRQw3XGpIOirMFhjB3-4Jtun4GYhv

  Paolo Godino
  Alexa Academy
  2025

  ALEX80 scheda v 1.1
********************************************************************/

#include "Arduino.h"
#include <XModem.h>

XModem xmodem;
bool xmodem_process_block(void *blk_id, size_t idSize, byte *data, size_t dataSize);  // Trasferimento di file con protocollo XMODEM

const char b64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

bool isEchoOn = true;
bool isComputerConnected = false;

bool isWriting = false;
bool isWritingBinary = false;
bool isWritingBinaryFast = false;

word startAddress = 0;     // Indirizzo inziale di scrittura (serve nella write inary fast)
word currentAddress = 0;  // Utilizzato nella scrittura in memoria
int bytesRemaining = 0;   // Numero di byte ancora da scrivere

bool busGranted=false; // Indica se si ha ricevuto l'accesso ai bus dello Z80

bool in_wait = false;
bool first_wait_logged = false;   // To log the T1 cycle when WAIT is active (LOW)
bool debug_mode = false;


/*
//Port ** Arduino Pin Number ** pin designation
PA 0 ** 22 ** D22	
PA 1 ** 23 ** D23	
PA 2 ** 24 ** D24	
PA 3 ** 25 ** D25	
PA 4 ** 26 ** D26	
PA 5 ** 27 ** D27	
PA 6 ** 28 ** D28	
PA 7 ** 29 ** D29	

PB 0 ** 53 ** SPI_SS	
PB 1 ** 52 ** SPI_SCK	
PB 2 ** 51 ** SPI_MOSI	
PB 3 ** 50 ** SPI_MISO	
PB 4 ** 10 ** PWM10	
PB 5 ** 11 ** PWM11	
PB 6 ** 12 ** PWM12	
PB 7 ** 13 ** PWM13	

PC 0 ** 37 ** D37	
PC 1 ** 36 ** D36	
PC 2 ** 35 ** D35	
PC 3 ** 34 ** D34	
PC 4 ** 33 ** D33	
PC 5 ** 32 ** D32	
PC 6 ** 31 ** D31	
PC 7 ** 30 ** D30	

PD 0 ** 21 ** I2C_SCL	
PD 1 ** 20 ** I2C_SDA	
PD 2 ** 19 ** USART1_RX	
PD 3 ** 18 ** USART1_TX	
PD 7 ** 38 ** D38	

PE 0 ** 0 ** USART0_RX	
PE 1 ** 1 ** USART0_TX	
PE 3 ** 5 ** PWM5	
PE 4 ** 2 ** PWM2	
PE 5 ** 3 ** PWM3	

PF 0 ** 54 ** A0	
PF 1 ** 55 ** A1	
PF 2 ** 56 ** A2	
PF 3 ** 57 ** A3	
PF 4 ** 58 ** A4	
PF 5 ** 59 ** A5	
PF 6 ** 60 ** A6	
PF 7 ** 61 ** A7	

PG 0 ** 41 ** D41	
PG 1 ** 40 ** D40	
PG 2 ** 39 ** D39	
PG 5 ** 4 ** PWM4	

PH 0 ** 17 ** USART2_RX	
PH 1 ** 16 ** USART2_TX	
PH 3 ** 6 ** PWM6	
PH 4 ** 7 ** PWM7	
PH 5 ** 8 ** PWM8	
PH 6 ** 9 ** PWM9	

PJ 0 ** 15 ** USART3_RX	
PJ 1 ** 14 ** USART3_TX	

PK 0 ** 62 ** A8	
PK 1 ** 63 ** A9	
PK 2 ** 64 ** A10	
PK 3 ** 65 ** A11	
PK 4 ** 66 ** A12	
PK 5 ** 67 ** A13	
PK 6 ** 68 ** A14	
PK 7 ** 69 ** A15	

PL 0 ** 49 ** D49	
PL 1 ** 48 ** D48	
PL 2 ** 47 ** D47	
PL 3 ** 46 ** D46	
PL 4 ** 45 ** D45	
PL 5 ** 44 ** D44	
PL 6 ** 43 ** D43	
PL 7 ** 42 ** D42	*/


//#define Z80_D0  22    PA0
//#define Z80_D1  24    PA2
//#define Z80_D2  26    PA4
//#define Z80_D3  28    PA6
//#define Z80_D4  30    PC7
//#define Z80_D5  32    PC5
//#define Z80_D6  34    PC3
//#define Z80_D7  36    PC1

//#define Z80_A0  23    PA1
//#define Z80_A1  25    PA3
//#define Z80_A2  27    PA5
//#define Z80_A3  29    PA7
//#define Z80_A4  31    PC6
//#define Z80_A5  33    PC4
//#define Z80_A6  35    PC2
//#define Z80_A7  37    PC0
//#define Z80_A8  39    PG2
//#define Z80_A9  41    PG0
//#define Z80_A10  43   PL6
//#define Z80_A11  45   PL4
//#define Z80_A12  47   PL2
//#define Z80_A13  49   PL0
//#define Z80_A14  51   PB2
//#define Z80_A15  53   PB0


//#define Z80_M1  52      // PB1
#define M1_READ   PINB & 0b00000010
#define SET_M1_INPUT    DDRB &= 0b11111101; PORTB &= 0b11111101

//#define Z80_WR  50    // PB3
#define WR_HIGH     (PORTB |= 0b00001000)
#define WR_LOW      (PORTB &= 0b11110111)
#define WR_READ     PINB & 0b00001000

//#define Z80_RD  48    // PL1
#define RD_HIGH     (PORTL |= 0b00000010)
#define RD_LOW      (PORTL &= 0b11111101)
#define RD_READ     PINL & 0b00000010

//#define Z80_IORQ 46     // PL3
#define IORQ_READ   PINL & 0b00001000
#define SET_IORQ_INPUT    DDRL &= 0b11110111; PORTL &= 0b11110111

//#define Z80_MRQ 44    // PL5
#define MREQ_HIGH   (PORTL |= 0b00100000)
#define MREQ_LOW    (PORTL &= 0b11011111)
#define MREQ_READ   PINL & 0b00100000

//#define Z80_RFSH 42     // PL7
#define RFSH_READ   PINL & 0b10000000
#define SET_RFSH_INPUT    DDRL &= 0b01111111; PORTL &= 0b01111111

//#define Z80_BREQ 38     // PD7
#define BREQ_HIGH   (PORTD |= 0b10000000)
#define BREQ_LOW    (PORTD &= 0b01111111)
#define SET_BREQ_OUTPUT   DDRD |= 0b10000000

//#define Z80_BACK 40     // PG1
#define SET_BACK_INPUT    DDRG &= 0b11111101; PORTG &= 0b11111101
#define BACK_READ         PING & 0b00000010

//#define Z80_INT  A15    // PK7
#define SET_INT_INPUT    DDRK &= 0b01111111; PORTK &= 0b01111111
#define SET_INT_OUTPUT   DDRK |= 0b10000000
#define INT_LOW          (PORTK &= 0b01111111)

//#define Z80_NMI  A14    // PK6
#define NMI_HIGH   (PORTK |= 0b01000000)
#define NMI_LOW    (PORTK &= 0b10111111)
#define SET_NMI_OUTPUT   DDRK |= 0b01000000

//#define Z80_WAIT A2   // PF2
#define WAIT_HIGH   (PORTF |= 0b00000100)
#define WAIT_LOW    (PORTF &= 0b11111011)
#define SET_WAIT_OUTPUT DDRF |= 0b00000100

#define Z80_CLK_INT 21  // PD0
#define CLK_HIGH   (PORTD |= 0b00000001)
#define CLK_LOW    (PORTD &= 0b11111110)
#define SET_CLK_OUTPUT   DDRD |= 0b00000001
#define SET_CLK_INPUT    DDRD &= 0b11111110; PORTD &= 0b11111110

//#define Z80_RST 20      // PD1
#define RST_HIGH   (PORTD |= 0b00000010)
#define RST_LOW    (PORTD &= 0b11111101)
#define SET_RST_OUTPUT    DDRD |= 0b00000010
//#define SET_RST_INPUT     DDRD &= 0b11111101

#define STOP_CLOCK     B00000000
#define PRESCALER_1    B00000001
#define PRESCALER_8    B00000010
#define PRESCALER_64   B00000011
#define PRESCALER_256  B00000100
#define PRESCALER_1024 B00000101

#define REQUEST_BUS_TIMEOUT_MS 1000  // 1 secondo
#define INTERRUPT_WAIT_TIMEOUT_MS 10000  // 10 secondi

#define READ_CHUNK_SIZE 300

volatile bool z80_busak;
volatile bool z80_rfsh;
volatile bool z80_m1;
volatile bool z80_wr;
volatile bool z80_rd;
volatile bool z80_ioreq;
volatile bool z80_mreq;
volatile bool z80_halt;
volatile uint16_t z80_add;
volatile uint8_t z80_data;

volatile bool performReset = false;

volatile unsigned int tick = 0;
volatile unsigned int cycle = 0;

unsigned int freq = 50;

bool logEnabled = false;
bool clockEnabled = false;
bool clockFromArduino = true;


volatile bool interruptCycleDetected = false;
volatile bool endMonitorDetected = false;
volatile bool busRequested = false;
uint8_t sp_for_read[2];
uint8_t reg_for_read[14];

/*bool stepState1=false;
bool stepState2=false;
bool debugRequested=false;*/

bool monitorEnabled=false;

bool dumpRequested=false;
bool intRequested=false;
bool dumpRunning=false;
uint16_t returnPC;

const byte demo_prog[] PROGMEM = { 0x3e, 0xf0, 0xed, 0x47, 0xfb, 0x3e, 0x0f, 0xd3, 0x03, 0x01, 0xea, 0x34, 0x11, 0x12, 0xda, 0x21, 0x98, 0xf3, 0xdd, 0x21, 0x54, 0xab, 0xfd, 0x21, 0xfa, 0xe2, 0xaf, 0xd3, 0x02, 0x3c, 0xc3, 0x1b, 0x80 };
const byte monitor_prog[] PROGMEM = { 0xf3, 0xed, 0x73, 0x50, 0xf0, 0xf5, 0xc5, 0xd5, 0xe5, 0xdd, 0xe5, 0xfd, 0xe5, 0xed, 0x7b, 0x50, 0xf0, 0xd3, 0x1f, 0xfb, 0xed, 0x4d };

void activateClockInterrupt() {
  attachInterrupt(digitalPinToInterrupt(Z80_CLK_INT), ClockTrigger, FALLING);
}

void deactivateClockInterrupt() {
  detachInterrupt(digitalPinToInterrupt(Z80_CLK_INT));
}

void enableLog() {
  logEnabled = true;
  if (!debug_mode) {
    activateClockInterrupt();
  }
}

void disableLog() {
  logEnabled = false;
  if (!debug_mode) {
     detachInterrupt(digitalPinToInterrupt(Z80_CLK_INT));
  }
}

inline void setWAIT(bool active) {
  //digitalWrite(Z80_WAIT, !active);

  if (active) {
    //PORTF &= ~(1 << PF2);  // LOW = attiva WAIT
    WAIT_LOW;
  } else {
   // PORTF |=  (1 << PF2);  // HIGH = disattiva WAIT  
    WAIT_HIGH;
  } 
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
  //digitalWrite(Z80_BREQ, LOW);
  BREQ_LOW;

  unsigned long curTime = millis();

  do {
    bool bus_ack = BACK_READ;  // digitalRead(Z80_BACK);
    if (!bus_ack) { // E' attivo basso
      return true;
    } 
    delay(1);
    //delayMicroseconds(1);
  } while (millis() - curTime < REQUEST_BUS_TIMEOUT_MS);

  return false;
}

void releaseBus() {
  //digitalWrite(Z80_BREQ, HIGH);
  BREQ_HIGH;
}

void doReset() {
  //digitalWrite(Z80_RST, HIGH);
  RST_HIGH;
  performReset = true;
}

void setDataBusOutput(bool isOutput) {
  if (isOutput) {
    DDRA |= 0b01010101;     // Z80_D0 - PA0, Z80_D1 - PA2, Z80_D2 - PA4, Z80_D0 - PA6 (pin 22–24-26-28)   -  OUTPUT bit a 1
    DDRC |= 0b10101010;     // Z80_D4 - PC7, Z80_D5 - PC5, Z80_D6 - PC3, Z80_D7 - PC1 (pin 30–32-34-36)   -  OUTPUT bit a 1
  } else {
    DDRA  &= 0b10101010;    // Z80_D0 - PA0, Z80_D1 - PA2, Z80_D2 - PA4, Z80_D0 - PA6 (pin 22–24-26-28)   -  INPUT bit a 0
    PORTA &= 0b10101010;    // Disattiva pull-up: imposta PA0, PA2, PA4, PA6 a 0
    DDRC  &= 0b01010101;    // Z80_D4 - PC7, Z80_D5 - PC5, Z80_D6 - PC3, Z80_D7 - PC1 (pin 30–32-34-36)  -  INPUT bit a 0
    PORTC &= 0b01010101;    // Disattiva pull-up: imposta PC1, PC3, PC5, PC7 a 0
  }
}

uint8_t readDataBus() {
  uint8_t data = 0;

  if (PINA & 0b00000001) data |= 0x01;  // PA0 → Z80_D0
  if (PINA & 0b00000100) data |= 0x02;  // PA2 → Z80_D1
  if (PINA & 0b00010000) data |= 0x04;  // PA4 → Z80_D2
  if (PINA & 0b01000000) data |= 0x08;  // PA6 → Z80_D3
  if (PINC & 0b10000000) data |= 0x10;  // PC7 → Z80_D4
  if (PINC & 0b00100000) data |= 0x20;  // PC5 → Z80_D5
  if (PINC & 0b00001000) data |= 0x40;  // PC3 → Z80_D6
  if (PINC & 0b00000010) data |= 0x80;  // PC1 → Z80_D7

  return data; 
}

void writeDataBus(uint8_t data) {
  PORTA = (PORTA & 0b10101010) |
          ((data & 0x01) ? 0b00000001 : 0) |  // Z80_D0 → PA0
          ((data & 0x02) ? 0b00000100 : 0) |  // Z80_D1 → PA2
          ((data & 0x04) ? 0b00010000 : 0) |  // Z80_D2 → PA4
          ((data & 0x08) ? 0b01000000 : 0);   // Z80_D3 → PA6
  PORTC = (PORTC & 0b01010101) |
          ((data & 0x10) ? 0b10000000 : 0) |  // Z80_D4 → PC7
          ((data & 0x20) ? 0b00100000 : 0) |  // Z80_D5 → PC5
          ((data & 0x40) ? 0b00001000 : 0) |  // Z80_D6 → PC3
          ((data & 0x80) ? 0b00000010 : 0);   // Z80_D7 → PC1
}

void setAddCtrlBusOutput(bool isOutput) {
  if (isOutput) {
    DDRA |= 0b10101010;  // Z80_A0 - PA1, Z80_A1 - PA3, Z80_A2 - PA5, Z80_A3 - PA7 (pin 23-25-27-29)    -  OUTPUT bit a 1
    DDRC |= 0b01010101;  // Z80_A4 - PC6, Z80_A5 - PC4, Z80_A6 - PC2, Z80_A7 - PC0 (pin 31-33-35-37)    -  OUTPUT bit a 1
    DDRG |= 0b00000101;  // Z80_A8 - PG2, Z80_A9 - PG0 - (pin 39-41)      -  OUTPUT bit a 1
    DDRL |= 0b01110111;  // Z80_A10 - PL6, Z80_A11 - PL4, Z80_A12 - PL2, Z80_A13 - PL0       Z80_MRQ - PL5,  Z80_RD - PL1     -  OUTPUT bit a 1
    DDRB |= 0b00001101;  // Z80_A14 - PB2, Z80_A15 - PB0      Z80_WR - PB3      -  OUTPUT bit a 1
  } else {
    DDRA &= 0b01010101;  // Z80_A0 - PA1, Z80_A1 - PA3, Z80_A2 - PA5, Z80_A3 - PA7 (pin 23-25-27-29)    -  OUTPUT bit a 0
    DDRC &= 0b10101010;  // Z80_A4 - PC6, Z80_A5 - PC4, Z80_A6 - PC2, Z80_A7 - PC0 (pin 31-33-35-37)    -  OUTPUT bit a 0
    DDRG &= 0b11111010;  // Z80_A8 - PG2, Z80_A9 - PG0 - (pin 39-41)      -  OUTPUT bit a 0
    DDRL &= 0b10001000;  // Z80_A10 - PL6, Z80_A11 - PL4, Z80_A12 - PL2, Z80_A13 - PL0       Z80_MRQ - PL5,  Z80_RD - PL1     -  OUTPUT bit a 0
    DDRB &= 0b11110010;  // Z80_A14 - PB2, Z80_A15 - PB0      Z80_WR - PB3      -  OUTPUT bit a 0
  } 
}

uint16_t readAddressBus() {
  uint16_t addr = 0;

  if (PINA & 0b00000010) addr |= 0x0001;  // PA1 → Z80_A0
  if (PINA & 0b00001000) addr |= 0x0002;  // PA3 → Z80_A1
  if (PINA & 0b00100000) addr |= 0x0004;  // PA5 → Z80_A2
  if (PINA & 0b10000000) addr |= 0x0008;  // PA7 → Z80_A3

  if (PINC & 0b01000000) addr |= 0x0010;  // PC6 → Z80_A4
  if (PINC & 0b00010000) addr |= 0x0020;  // PC4 → Z80_A5
  if (PINC & 0b00000100) addr |= 0x0040;  // PC2 → Z80_A6
  if (PINC & 0b00000001) addr |= 0x0080;  // PC0 → Z80_A7

  if (PING & 0b00000100) addr |= 0x0100;  // PG2 → Z80_A8
  if (PING & 0b00000001) addr |= 0x0200;  // PG0 → Z80_A9

  if (PINL & 0b01000000) addr |= 0x0400;  // PL6 → Z80_A10
  if (PINL & 0b00010000) addr |= 0x0800;  // PL4 → Z80_A11
  if (PINL & 0b00000100) addr |= 0x1000;  // PL2 → Z80_A12
  if (PINL & 0b00000001) addr |= 0x2000;  // PL0 → Z80_A13

  if (PINB & 0b00000100) addr |= 0x4000;  // PB2 → Z80_A14
  if (PINB & 0b00000001) addr |= 0x8000;  // PB0 → Z80_A15

  return addr;
}

void writeAddressBus(uint16_t addr) {
  PORTA = (PORTA & 0b01010101) |
          ((addr & 0x0001) ? 0b00000010 : 0) |  // Z80_A0 → PA1
          ((addr & 0x0002) ? 0b00001000 : 0) |  // Z80_A1 → PA3
          ((addr & 0x0004) ? 0b00100000 : 0) |  // Z80_A2 → PA5
          ((addr & 0x0008) ? 0b10000000 : 0);   // Z80_A3 → PA7
  PORTC = (PORTC & 0b10101010) |
          ((addr & 0x0010) ? 0b01000000 : 0) |  // Z80_A4 → PC6
          ((addr & 0x0020) ? 0b00010000 : 0) |  // Z80_A5 → PC4
          ((addr & 0x0040) ? 0b00000100 : 0) |  // Z80_A6 → PC2
          ((addr & 0x0080) ? 0b00000001 : 0);   // Z80_A7 → PC0
  PORTG = (PORTG & 0b11111010) |
          ((addr & 0x0100) ? 0b00000100 : 0) |  // Z80_A8 → PG2
          ((addr & 0x0200) ? 0b00000001 : 0);   // Z80_A9 → PG0
  PORTL = (PORTL & 0b10101010) |
          ((addr & 0x0400) ? 0b01000000 : 0) |  // Z80_A10 → PL6
          ((addr & 0x0800) ? 0b00010000 : 0) |  // Z80_A11 → PL4
          ((addr & 0x1000) ? 0b00000100 : 0) |  // Z80_A12 → PL2
          ((addr & 0x2000) ? 0b00000001 : 0);   // Z80_A13 → PL0
  PORTB = (PORTB & 0b11111010) |
          ((addr & 0x4000) ? 0b00000100 : 0) |  // Z80_A14 → PB2
          ((addr & 0x8000) ? 0b00000001 : 0);   // Z80_A15 → PB0
}

void printSerialLog() {
  Serial.print(F("Clock #"));
  Serial.print(cycle);

  Serial.print("  AB: ");
  printHex4(z80_add);

  Serial.print(F("  DB: "));
  for (int i = 7; i >= 0; --i) {
    Serial.print((z80_data >> i) & 1);
  }

  Serial.print(F(" ("));
  if (z80_data < 0x10) Serial.print(F("0"));
  Serial.print(z80_data, HEX);
  Serial.print(F(")  "));  

  Serial.print(F("   RD:"));
  Serial.print(z80_rd);

  Serial.print(F("   WR:"));
  Serial.print(z80_wr);

  Serial.print(F("   MREQ:"));
  Serial.print(z80_mreq);

  Serial.print(F("   IOREQ:"));
  Serial.print(z80_ioreq);

  Serial.print(F("   RFSH:"));
  Serial.print(z80_rfsh);

  Serial.print(F("   M1:"));
  Serial.print(z80_m1);

  Serial.println(F(""));
}

void ClockTrigger() {
  z80_add = readAddressBus();
  z80_data = readDataBus(); 

  // Leggi MREQ su pin 44 → PL5
  z80_mreq = MREQ_READ;
  //PINL & 0b00100000;  
  // Leggi RD su pin 48 → PL3
  z80_rd  = RD_READ;
  //PINL & 0b00001000;  
  // Leggi WR su pin 50 → PB3
  z80_wr = WR_READ;
  //PINB & 0b00001000; 

 // z80_rd = digitalRead(Z80_RD);   
 // z80_wr = digitalRead(Z80_WR);   
  
  z80_ioreq = IORQ_READ;
  //z80_ioreq = digitalRead(Z80_IORQ); 
  
  //z80_mreq = digitalRead(Z80_MRQ); 
  z80_rfsh = RFSH_READ;
  //digitalRead(Z80_RFSH);   
  z80_m1 = M1_READ;
  //z80_m1 = digitalRead(Z80_M1);

  static bool lastM1 = HIGH;

  if (busRequested) {
    bool bus_ack = BACK_READ; 
    if (!bus_ack) {  // Attivo basso
      setAddCtrlBusOutput(true);

      continuousReadCycle(0xf050, sp_for_read, 2);

      uint16_t sp_val = (uint16_t)(sp_for_read[1])<<8 | sp_for_read[0];

      continuousReadCycle(sp_val-12, reg_for_read, 14);

      Serial.println();
      Serial.print(F("A: "));
      printHex2(reg_for_read[11]);
      Serial.print(F("  B: "));
      printHex2(reg_for_read[9]);
      Serial.print(F("  C: "));
      printHex2(reg_for_read[8]);
      Serial.print(F("  D: "));
      printHex2(reg_for_read[7]);
      Serial.print(F("  E: "));
      printHex2(reg_for_read[6]);
      Serial.print(F("  H: "));
      printHex2(reg_for_read[5]);
      Serial.print(F("  L: "));
      printHex2(reg_for_read[4]);
      uint16_t ix = ((uint16_t)reg_for_read[3] << 8) | reg_for_read[2];
      Serial.print(F("  IX: "));
      printHex4(ix);
      uint16_t iy = ((uint16_t)reg_for_read[1] << 8) | reg_for_read[0];
      Serial.print(F("  IY: "));
      printHex4(iy);

      Serial.print(F("  SP: "));
      printHex4(sp_val+2);

      Serial.print(F("  PC: "));
      returnPC = ((uint16_t)reg_for_read[13] << 8) | reg_for_read[12];
      printHex4(returnPC);
      Serial.println();

      printFlags(reg_for_read[10]);

      setAddCtrlBusOutput(false);
      BREQ_HIGH;
    }
  }

  bool currentM1 = z80_m1;

  /*if (debugRequested) {
    bool bus_ack = BACK_READ; 
    if (!bus_ack) {  // Attivo basso
      debugRequested = false;
      in_wait = true;
    }
  }

   if (stepState1) {
    if (debug_mode && !in_wait && lastM1 == HIGH && currentM1 == LOW) {
      BREQ_LOW;
      stepState1 = false;
      stepState2 = true;
    }
  } */

  if (dumpRunning) {
    if (lastM1 == HIGH && currentM1 == LOW && z80_add == returnPC) {
      if (debug_mode) {
         setWAIT(true);
        in_wait = true;
      } else {
        setWAIT(false);
        in_wait = false;
      }
      dumpRunning = false;
    }
  } else {
    if (debug_mode && !dumpRequested && !intRequested && !in_wait && lastM1 == HIGH && currentM1 == LOW) {
      setWAIT(true);
      in_wait = true;
    }
  }

  if (dumpRequested) {
    dumpRequested = false;
    intRequested = true;
    
    endMonitorDetected = false;
    busRequested = false;
    interruptCycleDetected = false;
    SET_INT_OUTPUT;
    INT_LOW;
  }


  if (!dumpRunning && logEnabled && lastM1 == HIGH && currentM1 == LOW) {
    if (!isComputerConnected) Serial.println();
  }

  lastM1 = currentM1;

  if (z80_m1 == LOW && z80_ioreq == LOW) {
    if (!interruptCycleDetected) {
      interruptCycleDetected = true;
      intRequested = false;
      SET_INT_INPUT;

      setDataBusOutput(true);
      writeDataBus(0);

      dumpRunning = true;
    }
  } else {
    interruptCycleDetected = false;
    setDataBusOutput(false);
  }

  if (!endMonitorDetected && !busRequested && z80_ioreq == LOW && z80_wr == LOW)  {
    uint8_t half_add = z80_add&0xFF;
    if (half_add == 0x1f) {
      endMonitorDetected = true;
      busRequested = true;

      BREQ_LOW;
    }
  }

  if (!dumpRunning && logEnabled && (!in_wait || !first_wait_logged)) {
  //if (logEnabled && !in_wait) {
    if (in_wait) first_wait_logged = true;
    
    if (isComputerConnected) {
      // Serial.print(F("!LOG"));
      // byte control = z80_rd<<5 | z80_wr<<4 | z80_ioreq<<3 | z80_mreq<<2 | z80_rfsh<<1 | z80_m1;
      // encodeAndStreamLogBase64(highByte(z80_add), lowByte(z80_add), readZ80DataBus(), control);
      // Serial.print(F(";")); 
    } else {
      printSerialLog();
    }
  }

/*
  if (stepState2) {
    bool bus_ack = BACK_READ; 
    if (!bus_ack) {  // Attivo basso
      stepState2 = false;
      in_wait = true;
    }
  }  */
}

// Clock cycle
ISR(TIMER1_COMPA_vect) {
  tick++;
  if (tick%2 == 0) {
    //digitalWrite(Z80_CLK, LOW);
    //PORTD &= ~(1 << PD0);  // CLK LOW
    CLK_LOW;
    cycle++;
  } else {
    //digitalWrite(Z80_CLK, HIGH);
    //PORTD |= (1 << PD0);   // CLK HIGH
    CLK_HIGH;
  }

  if (performReset && cycle > 7) {
    performReset = false;
   // digitalWrite(Z80_RST, LOW);
    RST_LOW;
  }
}

void setClockFrequency(unsigned int f) {
  freq = f;
  eeprom_save_clk_freq();
}

void setClockFromArduino(bool save=true) {
  //pinMode(Z80_CLK, OUTPUT);
  //DDRD |= (1 << PD0);  // Z80_CLK D21 (PD0) come OUTPUT
  SET_CLK_OUTPUT;
  clockFromArduino=true;

  if (save) eeprom_save_clk_ard();
}

void setClockExternal(bool save=true) {
  //pinMode(Z80_CLK, INPUT);
 // DDRD &= ~(1 << PD0);     // Z80_CLK D21  (PD0) come input
 // PORTD &= ~(1 << PD0);    // disattiva la pull-up
  SET_CLK_INPUT;
  clockFromArduino=false;

  if (save) eeprom_save_clk_ard();
}

void startClock() {
  clockEnabled = true;

  CLK_HIGH;
  //PORTD |= (1 << PD0);   // CLK HIGH
  //digitalWrite(Z80_CLK, HIGH);
  //digitalWrite(Z80_RST, LOW);

  unsigned int ocr;
  byte prescalerMask;

  calcOCR(freq, &ocr, &prescalerMask);

  cli();

  TCCR1A = 0;
  TCCR1B = TCCR1B & B11100000 | B00001000 | prescalerMask;
  TCNT1 = 0;  // Inizializza il counter
  OCR1A = ocr;
  TIMSK1 = 0b00000010; // Abilita interrupt su conteggio OCR1A

  sei();
}

void stopClock() {
  clockEnabled = false;
 
  cli();

  TCCR1A = 0;
  TCCR1B = TCCR1B & B11100000 | B00001000 | STOP_CLOCK;
  TCNT1 = 0;  // Inizializza il counter
  TIMSK1 = 0b00000000; // Disabilita interrupt

  sei();
}

void writeRAM(uint16_t address, uint8_t data) {
  writeAddressBus(address);
  delayMicroseconds(3);
  //digitalWrite(Z80_RD, 1);
  RD_HIGH;
  delayMicroseconds(3);
  //digitalWrite(Z80_MRQ, 0);
  MREQ_LOW;
  delayMicroseconds(3);
  setDataBusOutput(true);
  writeDataBus(data);
  delayMicroseconds(3);
  //digitalWrite(Z80_WR, 0);
  WR_LOW;
  delayMicroseconds(1);
  //digitalWrite(Z80_WR, 1);
  WR_HIGH;
  delayMicroseconds(3);
  setDataBusOutput(false);
  delayMicroseconds(3);
  //digitalWrite(Z80_MRQ, 1);
  MREQ_HIGH;
  delayMicroseconds(3);
}

void continuousReadCycle(uint16_t address, byte* data, unsigned int size) {
  //digitalWrite(Z80_WR, 1);
  WR_HIGH;
  writeAddressBus(address);
  delayMicroseconds(1);

  //digitalWrite(Z80_MRQ, 0);
  MREQ_LOW;
  //digitalWrite(Z80_RD, 0);
  RD_LOW;

  for (unsigned int idx = 0; idx < size; ++idx) {
    writeAddressBus(address + idx);
    delayMicroseconds(1);
    data[idx] = readDataBus();
  }

  //digitalWrite(Z80_RD, 1);
  RD_HIGH;
  //digitalWrite(Z80_MRQ, 1);
  MREQ_HIGH;
  delayMicroseconds(1);
}

void setup() {
  Serial.begin(19200);

  xmodem.begin(Serial, XModem::ProtocolType::XMODEM);
  xmodem.setRecieveBlockHandler(xmodem_process_block);

  setDataBusOutput(false);
  setAddCtrlBusOutput(false);

  //pinMode(Z80_WR, INPUT);
  //pinMode(Z80_RD, INPUT);
  //pinMode(Z80_MRQ, INPUT);

  //pinMode(Z80_M1, INPUT);
  SET_M1_INPUT;
  //pinMode(Z80_IORQ, INPUT);
  SET_IORQ_INPUT;
  //pinMode(Z80_RFSH, INPUT);
  SET_RFSH_INPUT;
  //pinMode(Z80_BREQ, OUTPUT);
  SET_BREQ_OUTPUT;
  //pinMode(Z80_BACK, INPUT);
  SET_BACK_INPUT;
  //pinMode(Z80_INT, INPUT);
  SET_INT_INPUT;
  //pinMode(Z80_NMI, OUTPUT);
  SET_NMI_OUTPUT;

  //pinMode(Z80_WAIT, OUTPUT);
 // DDRF |= (1 << PF2);  // imposta Z80_WAIT (A2) come OUTPUT
  SET_WAIT_OUTPUT;
  WAIT_HIGH;
 // setWAIT(false);

  //digitalWrite(Z80_BREQ, HIGH);
  BREQ_HIGH;
  //digitalWrite(Z80_NMI, HIGH);
  NMI_HIGH;

  //pinMode(Z80_CLK, INPUT);  // Eventualmente sarà messo in OUTPUT nella startClock
  //DDRD &= ~(1 << PD0);     // imposta  Z80_CLK (PD0) come input
  //PORTD &= ~(1 << PD0);    // disattiva la pull-up
  SET_CLK_INPUT;
  //pinMode(Z80_RST, OUTPUT);
  SET_RST_OUTPUT;
  RST_LOW;
  //SET_RST_INPUT;

  //setClockFrequency(50);

  //doReset();

  eeprom_init();

  if (!isComputerConnected) Serial.println("OK");
}

void loop() {
  serialLoop();
}

void read_mem(word start, word len) {
  byte mem_data[16];

  busGranted = requestBus();

  if (busGranted && len > 0) {
    setAddCtrlBusOutput(true);

    unsigned long currentAddress = start;
    uint32_t endAddress = (uint32_t)start + (uint32_t)len;  // può "sforare" 0xFFFF!

    // Se il range sfora la memoria a 16 bit, limitiamo a 0x10000
    if (endAddress > 0x10000UL) {
      endAddress = 0x10000UL;
    }

    while (currentAddress < endAddress) {
      unsigned int chunkSize = min(16, endAddress - currentAddress);

      continuousReadCycle(currentAddress, mem_data, chunkSize);

      printHex4(currentAddress);  // stampa indirizzo allineato
      Serial.print(F(":  "));

      // HEX
      for (int i = 0; i < 16; ++i) {
        if (i < chunkSize) {
          byte val = mem_data[i];
          if (val < 0x10) Serial.print(F("0"));
          Serial.print(val, HEX);
        } else {
          Serial.print(F("  "));
        }

        Serial.print(F(" "));
        if (i == 7) Serial.print(F(" "));
      }

      Serial.print(F(" "));

      // ASCII
      for (int i = 0; i < 16; ++i) {
        if (i < chunkSize) {
          uint8_t val = mem_data[i];
          Serial.write(isprint(val) ? val : '.');
        } else {
          Serial.print(F(" "));
        }
      }

      Serial.println();
      currentAddress += chunkSize;

      if (currentAddress > 0xFFFF) break;  // non andare oltre lo spazio a 16 bit
    }

    setAddCtrlBusOutput(false);
  }

  releaseBus();
  busGranted = false;
}

void encodeAndStreamBase64(byte *data, unsigned int length) {
  // 300 byte in base64 diventano al massimo 400 caratteri
  char output[404]; // margine extra per sicurezza o terminatore
  byte in[3];
  unsigned int outIndex = 0;

  for (unsigned int i = 0; i < length; i += 3) {
    in[0] = data[i];
    in[1] = (i + 1 < length) ? data[i + 1] : 0;
    in[2] = (i + 2 < length) ? data[i + 2] : 0;

    output[outIndex++] = b64_table[(in[0] & 0xFC) >> 2];
    output[outIndex++] = b64_table[((in[0] & 0x03) << 4) | ((in[1] & 0xF0) >> 4)];
    output[outIndex++] = (i + 1 < length) ? b64_table[((in[1] & 0x0F) << 2) | ((in[2] & 0xC0) >> 6)] : '=';
    output[outIndex++] = (i + 2 < length) ? b64_table[in[2] & 0x3F] : '=';
  }

  // Invia tutto in un colpo solo
  if (isComputerConnected) Serial.print(F("!DATA"));
  Serial.write((const uint8_t*)output, outIndex);
  if (isComputerConnected) Serial.print(F(";"));
}

void read_mem_binary(word start, word len) {
  byte mem_data[READ_CHUNK_SIZE];

  busGranted = requestBus();

  if (busGranted) {
    if (len > 0) {
      if (isComputerConnected) Serial.print(F("!STATUS1;"));

      setAddCtrlBusOutput(true);

      if (len < READ_CHUNK_SIZE) {
        continuousReadCycle(start, mem_data, len);

        encodeAndStreamBase64(mem_data, len);
      } else {
        unsigned int numReadCycle = len / READ_CHUNK_SIZE + 1;  // Si fanno letture di gruppi di 300 byte e si inviano sulla seriale
        
        for (int i=0; i < numReadCycle; ++i) {
          continuousReadCycle(start + i * READ_CHUNK_SIZE, mem_data, READ_CHUNK_SIZE);

          encodeAndStreamBase64(mem_data, READ_CHUNK_SIZE);
        }
      }

      setAddCtrlBusOutput(false);

      if (isComputerConnected) Serial.println(F("!STATUS2;")); else Serial.println(F("\nOK"));
    } else {
      if (isComputerConnected) Serial.println(F("!ERROR:LEN MUST BE GREATER THAN 0;")); else Serial.print(F("LEN MUST BE GREATER THAN 0"));
    }
  } else {
    if (isComputerConnected) Serial.println(F("!ERROR:BUS NOT GRANTED, CANNOT READ;")); else Serial.print(F("BUS NOT GRANTED, CANNOT READ"));
  }

  releaseBus();
  busGranted = false;
}

void processFastWriteBase64(const char* line) {
  while (*line && bytesRemaining > 0) {
    // Decodifica 4 caratteri base64 → 3 byte
    char c0 = *line++;
    char c1 = *line++;
    char c2 = *line++;
    char c3 = *line++;

    if (!c0 || !c1 || !c2 || !c3) break;

    uint8_t b[4];
    for (int i = 0; i < 4; i++) {
      char ch = (i == 0) ? c0 : (i == 1 ? c1 : (i == 2 ? c2 : c3));
      if (ch >= 'A' && ch <= 'Z') b[i] = ch - 'A';
      else if (ch >= 'a' && ch <= 'z') b[i] = ch - 'a' + 26;
      else if (ch >= '0' && ch <= '9') b[i] = ch - '0' + 52;
      else if (ch == '+') b[i] = 62;
      else if (ch == '/') b[i] = 63;
      else if (ch == '=') b[i] = 0;
    }

    if (bytesRemaining > 0) {
      writeRAM(currentAddress++, (b[0] << 2) | (b[1] >> 4));
      --bytesRemaining;
    }

    if (bytesRemaining > 0 && c2 != '=') {
      writeRAM(currentAddress++, (b[1] << 4) | (b[2] >> 2));
      --bytesRemaining;
    }

    if (bytesRemaining > 0 && c3 != '=') {
      writeRAM(currentAddress++, (b[2] << 6) | b[3]);
      --bytesRemaining;
    }
  }

  Serial.print(F("!B: ")); 
  Serial.print(bytesRemaining); 
  Serial.println(F(";")); 
  if (bytesRemaining == 0) {
    isWritingBinaryFast = false;
    Serial.println(F("!STATUS4;")); 

    setAddCtrlBusOutput(false);

    releaseBus();
    busGranted = false;
  } else {
    if ((currentAddress - startAddress) % 60 == 0)  Serial.println(F("!STATUS5;"));
  }
}

bool xmodem_process_block(void *blk_id, size_t idSize, byte *data, size_t dataSize) {
  for(int i = 0; i < dataSize; ++i) {
    writeRAM(currentAddress++, data[i]);
  }

  return true; 
}
