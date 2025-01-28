enum SmState {
  STATE_WAIT_AA,
  STATE_WAIT_COMMAND,
  STATE_WAIT_LENGTH_HIGH,
  STATE_WAIT_LENGTH_LOW,
  STATE_WAIT_BYTE1,
  STATE_WAIT_BYTE2,
  STATE_WAIT_DATA
};

#define CMD_LOG         0x00
#define CMD_READ_MEM    0x40
#define CMD_WRITE_MEM   0x41
#define CMD_RESTART     0x42
#define CMD_STOP        0x43
#define CMD_EN_LOG      0x44
#define CMD_INT         0x45
#define CMD_NMI         0x46

#define READ_CHUNK_SIZE 300

byte state = STATE_WAIT_AA; // Stato corrente della macchina a stati

byte command; // Comando ricevuto

byte smTemp = 0;  // Serve per memorizzare il primo byte di una word

word dataLenght = 0;  // Lunghezza dei dati da leggere o scrivere
word dataStart = 0;  // Posizione in memoria dei dati da leggere o scrivere

word writeAddressIdx = 0;  // Indice del byte scritto in RAM
bool busGranted=false; // Indica se si ha ricevuto l'accesso ai bus dello Z80

void sendResponse40();
void sendResponse41(bool success);

void sendResponse40() {
  byte message[READ_CHUNK_SIZE + 10];

  busGranted = requestBus();

  if (busGranted && dataLenght > 0) {
    setAddCtrlBusOutput(true);

    message[0] = 0xAA;
    message[1] = CMD_READ_MEM;
    message[2] = (dataLenght & 0xFF00)>>8;
    message[3] = dataLenght & 0x00FF;
    message[4] = (dataStart & 0xFF00)>>8;
    message[5] = dataStart & 0x00FF;

    int msgStart=6;

    if (dataLenght < READ_CHUNK_SIZE) {
      continuousReadCycle(dataStart, message + msgStart, dataLenght);

      Serial.write(message, msgStart + dataLenght);
      Serial.flush();
    } else {
      unsigned int numReadCycle = dataLenght / READ_CHUNK_SIZE + 1;  // Si fanno letture di gruppi di 300 byte e si inviano sulla seriale
      
      for (int i=0; i < numReadCycle; ++i) {
        continuousReadCycle(dataStart + i * READ_CHUNK_SIZE, message + msgStart, READ_CHUNK_SIZE);

        Serial.write(message, msgStart + READ_CHUNK_SIZE);
        Serial.flush();

        msgStart=0;
      }
    }

    setAddCtrlBusOutput(false);
  } else {
    message[0] = 0xAA;
    message[1] = 0x40;
    message[2] = 0x00;
    message[3] = 0x00;
    message[4] = 0x00;
    message[5] = 0x00;

    Serial.write(message, sizeof(message));
	  Serial.flush();
  }

  releaseBus();
  busGranted = false;
}

void sendResponse41(bool success) {
  byte message[] = {0xAA, 0x41, success?((dataLenght & 0xFF00)>>8):0, success?(dataLenght & 0x00FF):0, (dataStart & 0xFF00)>>8, dataStart & 0x00FF};

	Serial.write(message, sizeof(message));
	Serial.flush();
}

void execRestart(unsigned int f) {
  restart(f);

  byte message[] = {0xAA, CMD_RESTART};

	Serial.write(message, sizeof(message));
	Serial.flush();
}

void loopSerial() {
	if (Serial.available() > 0) {
		byte byteRead = Serial.read();

    switch (state) {
      case STATE_WAIT_AA:
        if (byteRead == 0xAA) {
          state = STATE_WAIT_COMMAND;
        }
        break;

      case STATE_WAIT_COMMAND:
        command = byteRead;
        if (byteRead == CMD_STOP) {
          stopClock();
          state = STATE_WAIT_AA;
        } else if (byteRead == CMD_EN_LOG || byteRead == CMD_RESTART) {
          state = STATE_WAIT_BYTE1;
        } else if (byteRead == CMD_NMI) {
          digitalWrite(Z80_NMI, LOW);
          delay(500);
          digitalWrite(Z80_NMI, HIGH);
          state = STATE_WAIT_AA;
        } else {
          state = STATE_WAIT_LENGTH_HIGH;
        }
        break;

      case STATE_WAIT_LENGTH_HIGH:
        state = STATE_WAIT_LENGTH_LOW;
        smTemp = byteRead;
        break;

      case STATE_WAIT_LENGTH_LOW:
        dataLenght = (smTemp<<8) + byteRead;
        state = STATE_WAIT_BYTE1;
        break;

      case STATE_WAIT_BYTE1:
        if (command == CMD_EN_LOG) {
          if (byteRead == 0x00) {
            disableLog();
          } else {
            enableLog();
          }
          state = STATE_WAIT_AA;
        } else {
          state = STATE_WAIT_BYTE2;
          smTemp = byteRead;
        }
        break;

      case STATE_WAIT_BYTE2:
        if (command == CMD_READ_MEM) {
          dataStart = (smTemp<<8) + byteRead;
          sendResponse40();
          state = STATE_WAIT_AA;
        } else if (command == CMD_WRITE_MEM) {
        //  Serial3.println("Richiesta wcrittura");
          dataStart = (smTemp<<8) + byteRead;
          busGranted = requestBus();
          if (busGranted && dataLenght > 0) {
          //  Serial3.println("Bus ottenuto");
          //  Serial3.print("dataLenght: ");
          //  Serial3.print(dataLenght);
          //  Serial3.print("  dataStart: ");
          //  Serial3.println(dataStart);
            setAddCtrlBusOutput(true);
            writeAddressIdx = dataStart;
            state = STATE_WAIT_DATA;
          } else {
            sendResponse41(false);
            state = STATE_WAIT_AA;
          }
        } else if (command == CMD_RESTART) {
          int freq = (smTemp<<8) + byteRead;
          execRestart(freq);
          state = STATE_WAIT_AA;
        }
        break;

      case STATE_WAIT_DATA:
        if (command == CMD_WRITE_MEM) {
          if (busGranted) {
           /* Serial3.print(writeAddressIdx ,HEX);
            Serial3.print("  ");
            Serial3.print(byteRead ,HEX);
            Serial3.print(".  writeAddressIdx - dataStart ");
            Serial3.println(writeAddressIdx - dataStart);*/
            writeRAM(byteRead, writeAddressIdx++); 

            if (writeAddressIdx - dataStart == dataLenght) {
              sendResponse41(true);
              state = STATE_WAIT_AA;
              setAddCtrlBusOutput(false);
              releaseBus();
              busGranted = false;
            }
          } else {
            sendResponse41(false);
            state = STATE_WAIT_AA;
          }
        } else {
          state = STATE_WAIT_AA;
        }
        break;

      default:
        state = STATE_WAIT_AA;
        break;
    }
	}
}

