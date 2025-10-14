#define CMD_READ  "r"
#define CMD_READ_BIN  "rb"
#define CMD_WRITE "w"
#define CMD_WRITE_BIN "wb"
#define CMD_WRITE_BIN_FAST "wbf"
#define CMD_SET_ECHO "echo"
#define CMD_SET_COMPUTER_CONNECTED "cc"
#define CMD_SET_HUMAN_CONNECTED "hc"
#define CMD_SET_CLOCK_FREQ "sclk"
#define CMD_CLOCK "clk"
#define CMD_LOG "log"
#define CMD_DEBUG "debug"
#define CMD_STEP "s"
#define CMD_RESET "reset"
#define CMD_DUMP "d"
#define CMD_GET_INFO "info"
#define CMD_NMI "nmi"
#define CMD_MONITOR "monitor"
#define CMD_HELP "?"

#define COMMANDSIZE 16
char cmdbuf[COMMANDSIZE];

#define LINEBUF_SIZE   100
char linebuf[LINEBUF_SIZE];
uint8_t linelen = 0;

// Legge il comando da seriale
void readCommand() {
  memset(cmdbuf, 0, COMMANDSIZE);  // azzera tutto il buffer
  int idx = 0;

  while (idx < (COMMANDSIZE - 1)) { // lascia spazio per '\0'
    while (!Serial.available());   // aspetta dati
    char c = Serial.read();
    
    if (isEchoOn) Serial.print(c);

    if (c == '\n' || c == '\r') {
      break;
    }

    cmdbuf[idx++] = c;
  }

  cmdbuf[idx] = '\0';  // chiusura stringa
  Serial.println();
}

void serialLoop() {
  if (isWriting) {
    readCommand();

    // Fine scrittura
    if (strcmp(cmdbuf, "") == 0) {
      isWriting = false;

      if (isComputerConnected) Serial.println(F("!OK;")); else Serial.println(F("OK"));

      setAddCtrlBusOutput(false);

      releaseBus();
      busGranted = false;
    } else {
      byte data_wr = strtol(cmdbuf, NULL, 16);

      writeRAM(currentAddress++, data_wr);
    } 
  } /*else if (isWritingBinary) {
    if (Serial.available()) {
      byte c = Serial.read();

      Serial.print(F("Add: "));
      Serial.print(currentAddress, HEX);
      Serial.print(F("   Data: "));
      Serial.println(c, HEX);

      writeRAM(currentAddress++, c);

      --bytesRemaining;

      if (isComputerConnected && currentAddress % 60 == 0) {
        Serial.println(F("!STATUS5;"));
      }
      if (!isComputerConnected && currentAddress % 200 == 0) {
        Serial.print(F("."));
      }

      // Fine scrittura
      if (bytesRemaining == 0) { 
        isWritingBinary = false;

        if (isComputerConnected) {
          Serial.println(F("!STATUS4;")); 
        } else {
          Serial.println(F(""));
          Serial.println(F("OK"));
        }

        setAddCtrlBusOutput(false);

        releaseBus();
        busGranted = false;
      }
    }
  }*/ else if (isWritingBinaryFast) {
    if (Serial.available()) {
      char c = Serial.read();

      if (c == '\n' || c == '\r') {
        if (linelen > 0) {
          linebuf[linelen] = '\0';

          processFastWriteBase64(linebuf);
         
          linelen = 0;
        }
      } else if (linelen < LINEBUF_SIZE - 1) {
        linebuf[linelen++] = c;
      }
    }
  } else {
    readCommand();

    byte index = 0;
    char *strings[6];
    char *ptr = NULL;
    ptr = strtok(cmdbuf, " ");
    while (ptr != NULL) {
      strings[index++] = ptr;
      ptr = strtok(NULL, " ");
    }

    if (index > 0) {
      if (strcmp(strings[0], CMD_READ) == 0) {
        unsigned int startAdd = 0;
        unsigned int len = 16;
        if (index > 1) startAdd = strtol(strings[1], NULL, 16);
        if (index > 2) len = strtol(strings[2], NULL, 10);

        read_mem(startAdd, len);
      }  else if (strcmp(strings[0], CMD_READ_BIN) == 0) {
        unsigned int startAdd = 0;
        unsigned int len = 16;
        if (index > 1) startAdd = strtol(strings[1], NULL, 16);
        if (index > 2) len = strtol(strings[2], NULL, 10);

        read_mem_binary(startAdd, len);
      } else if (strcmp(strings[0], CMD_WRITE) == 0) {
        unsigned int startAdd = 0;
        if (index > 1) startAdd = strtol(strings[1], NULL, 16);
        currentAddress = startAdd;

        busGranted = requestBus();
        if (busGranted) {
          setAddCtrlBusOutput(true);

          isWriting = true;
        } else {
          if (isComputerConnected) Serial.println(F("!ERROR:BUS NOT GRANTED, CANNOT READ;")); else Serial.print(F("BUS NOT GRANTED, CANNOT READ"));
        }
      } else if (strcmp(strings[0], CMD_WRITE_BIN) == 0) {
        unsigned int startAdd = 0;
        if (index > 1) startAdd = strtol(strings[1], NULL, 16);
        currentAddress = startAdd;
        busGranted = requestBus();
        if (busGranted) {
          setAddCtrlBusOutput(true);

          if (isComputerConnected) Serial.println(F("!STATUS3;")); else Serial.println(F("SEND FILE (XMODEM PROTOCOL)"));
          bool ret = xmodem.receive();
          delay(100);
          if (ret) {
            if (isComputerConnected) {
              Serial.println(F("!STATUS4;")); 
            } else {
              Serial.println(F("OK"));
            }
          } else {
            if (isComputerConnected) Serial.println(F("!ERROR:AN ERROR OCCURRED DURING FILE TRANSFER;")); else Serial.println(F("AN ERROR OCCURRED DURING FILE TRANSFER"));
          }

          setAddCtrlBusOutput(false);

          releaseBus();
          busGranted = false;
        } else {
          if (isComputerConnected) Serial.println(F("!ERROR:BUS NOT GRANTED, CANNOT READ;")); else Serial.println(F("BUS NOT GRANTED, CANNOT READ"));
        }
        
        /*unsigned int startAdd = 0;
        int len = -1;
        if (index > 1) startAdd = strtol(strings[1], NULL, 16);
        if (index > 2) len = strtol(strings[2], NULL, 10);
        currentAddress = startAdd;
        bytesRemaining = len;

        if (len <= 0) {
          if (isComputerConnected) Serial.println(F("!ERROR:MISSING LEN;")); else Serial.print(F("MISSING LEN"));
        } else {
          busGranted = requestBus();
          if (busGranted) {
            setAddCtrlBusOutput(true);

            isWritingBinary = true;
            if (isComputerConnected) Serial.println(F("!STATUS3;")); else Serial.println(F("SEND FILE"));
          } else {
            if (isComputerConnected) Serial.println(F("!ERROR:BUS NOT GRANTED, CANNOT READ;")); else Serial.print(F("BUS NOT GRANTED, CANNOT READ"));
          }
        }*/
      } else if (strcmp(strings[0], CMD_WRITE_BIN_FAST) == 0) {
        unsigned int startAdd = 0;
        int len = -1;
        if (index > 1) startAdd = strtol(strings[1], NULL, 16);
        if (index > 2) len = strtol(strings[2], NULL, 10);
        startAddress = startAdd;
        currentAddress = startAdd;
        bytesRemaining = len;

        if (len == -1) {
          if (isComputerConnected) Serial.println(F("!ERROR:MISSING LEN;")); else Serial.print(F("MISSING LEN"));
        } else {
          busGranted = requestBus();
          if (busGranted) {
            setAddCtrlBusOutput(true);

            isWritingBinaryFast = true;
            if (isComputerConnected) Serial.println(F("!STATUS3;")); else Serial.println(F("SEND FILE"));
          } else {
            if (isComputerConnected) Serial.println(F("!ERROR:BUS NOT GRANTED, CANNOT READ;")); else Serial.print(F("BUS NOT GRANTED, CANNOT READ"));
          }
        }
      } else if (strcmp(strings[0], CMD_SET_ECHO) == 0) {
        if (index == 2) {
          if (strcmp(strings[1], "on") == 0) {
            isEchoOn = true;
            if (isComputerConnected) Serial.println(F("!OK;")); else Serial.println(F("OK"));
          } else if (strcmp(strings[1], "off") == 0) {
            isEchoOn = false;
            if (isComputerConnected) Serial.println(F("!OK;")); else Serial.println(F("OK"));
          } else {
            if (isComputerConnected) Serial.println(F("!ERROR:WRONG PARAMETER;")); else Serial.println(F("WRONG PARAMETER"));
          }
        } else {
          if (isComputerConnected) Serial.println(F("!ERROR:MISSING PARAMETERS;")); else Serial.println(F("MISSING PARAMETERS"));
        }
      } else if (strcmp(strings[0], CMD_SET_COMPUTER_CONNECTED) == 0) {
        isEchoOn = false;
        isComputerConnected = true;
      } else if (strcmp(strings[0], CMD_SET_HUMAN_CONNECTED) == 0) {
        isEchoOn = true;
        isComputerConnected = false;
      } else if (strcmp(strings[0], CMD_SET_CLOCK_FREQ) == 0) {  
        if (index == 2) {
          char* endPtr;
          unsigned int freq_val = (unsigned int)strtoul(strings[1], &endPtr, 10);  // base 10
          bool clockwasEnabled = clockEnabled;
          if (clockwasEnabled) stopClock();
          setClockFrequency(freq_val);
          if (clockwasEnabled) startClock();
          if (isComputerConnected) Serial.println(F("!OK;")); else {
            Serial.print(F("OK frequency set to "));
            Serial.print(freq);
            Serial.println(F(" Hz"));
          }
        } else {
          if (isComputerConnected) Serial.println(F("!ERROR:MISSING PARAMETERS;")); else Serial.println(F("MISSING PARAMETERS"));
        }
      } else if (strcmp(strings[0], CMD_CLOCK) == 0) { 
        if (index == 2) {
          if (strcmp(strings[1], "start") == 0) {
            if (clockFromArduino) {
              startClock();
              if (isComputerConnected) Serial.println("!OK;"); else Serial.println("OK");
            } else {
              if (isComputerConnected) Serial.println(F("!ERROR:CLOK EXTERNAL;")); else Serial.println(F("CLOCK IS SET TO EXTERNAL"));
            }
          } else if (strcmp(strings[1], "stop") == 0) {
            if (clockFromArduino) {
              stopClock();
              if (isComputerConnected) Serial.println("!OK;"); else Serial.println("OK");
            } else {
              if (isComputerConnected) Serial.println(F("!ERROR:CLOK EXTERNAL;")); else Serial.println(F("CLOCK IS SET TO EXTERNAL"));
            }
          } else if (strcmp(strings[1], "arduino") == 0) {
            setClockFromArduino();
            if (isComputerConnected) Serial.println(F("!OK;")); else Serial.println(F("OK"));
          } else if (strcmp(strings[1], "external") == 0) {
            setClockExternal();
            if (isComputerConnected) Serial.println(F("!OK;")); else Serial.println(F("OK"));
          } else {
            if (isComputerConnected) Serial.println(F("!ERROR:WRONG PARAMETER;")); else Serial.println(F("WRONG PARAMETER"));
          }
        } else {
          if (isComputerConnected) Serial.println(F("!ERROR:MISSING PARAMETERS;")); else Serial.println(F("MISSING PARAMETERS"));
        }
      } else if (strcmp(strings[0], CMD_LOG) == 0) {
        if (index == 2) {
          if (strcmp(strings[1], "on") == 0) {
            enableLog();
            if (isComputerConnected) Serial.println(F("!OK;")); else Serial.println(F("OK"));
          } else if (strcmp(strings[1], "off") == 0) {
            disableLog();
            if (isComputerConnected) Serial.println(F("!OK;")); else Serial.println(F("OK"));
          } else {
            if (isComputerConnected) Serial.println(F("!ERROR:WRONG PARAMETER;")); else Serial.println(F("WRONG PARAMETER"));
          }
        } else {
          if (isComputerConnected) Serial.println(F("!ERROR:MISSING PARAMETERS;")); else Serial.println(F("MISSING PARAMETERS"));
        }
      } else if (strcmp(strings[0], CMD_DEBUG) == 0) {
        if (index == 2) {
          if (strcmp(strings[1], "on") == 0) {
            debug_mode = true;

           /* debugRequested=true;
            in_wait = false;
            BREQ_LOW; */

            if (isComputerConnected) Serial.println(F("!OK;")); else Serial.println(F("OK"));
          } else if (strcmp(strings[1], "off") == 0) {
            debug_mode = false;
            in_wait = false;
            first_wait_logged = false;
            setWAIT(false);
            if (isComputerConnected) Serial.println(F("!OK;")); else Serial.println(F("OK"));
          } else {
            if (isComputerConnected) Serial.println(F("!ERROR:WRONG PARAMETER;")); else Serial.println(F("WRONG PARAMETER"));
          }
        } else {
          if (isComputerConnected) Serial.println(F("!ERROR:MISSING PARAMETERS;")); else Serial.println(F("MISSING PARAMETERS"));
        }
      } else if (strcmp(strings[0], CMD_STEP) == 0) {
        in_wait = false;
        first_wait_logged = false;
        setWAIT(false);

       /* in_wait = false;
        stepState1 = true;
        stepState2 = false;
        BREQ_HIGH; */
      } else if (strcmp(strings[0], CMD_RESET) == 0) {
        bool clockwasEnabled = clockEnabled;
        if (clockwasEnabled) stopClock();
        doReset();
        if (isComputerConnected) Serial.println(F("!OK;")); else Serial.println(F("OK"));
        if (clockwasEnabled) startClock();
      } else if (strcmp(strings[0], CMD_DUMP) == 0) {
        if (monitorEnabled) {
          if (debug_mode) {
            // Devo fare uno step rilasciando il WAIT altrimenti non viene sentito l'interrupt
            in_wait = false;
            first_wait_logged = false;
            dumpRequested = true;
            dumpRunning = false;
            intRequested = false;
            setWAIT(false);
          } else {
            endMonitorDetected = false;
            busRequested = false;
            interruptCycleDetected = false;
            SET_INT_OUTPUT;
            INT_LOW;
          }
        } else {
          if (isComputerConnected) Serial.println(F("!ERROR:MONITOR NON ACTIVE;")); else Serial.println(F("MONITOR SHOULD BE ACTIVE FOR REGISTERS DUMP"));
        }
       /* if (debug_mode) {
          if (inject_mode) {
            if (isComputerConnected) Serial.println(F("!ERROR:ALREADY DUMPING;")); else Serial.println(F("DUMP ALREADY IN PROGRESS"));
          } else {
            word pc = z80_add;  // PC visibile durante M1
            //Serial.print("Iniezione dump registri da PC = 0x");
            //Serial.println(pc, HEX);
            inject_address = pc;
            prepareInjectCode(pc);
            setWAIT(false);  // rilascia WAIT esegue il codice iniettato
          }
        } else {
          if (isComputerConnected) Serial.println(F("!ERROR:DEBUG NON ACTIVE;")); else Serial.println(F("DEBUG MODE SHOULD BE ACTIVE FOR REGISTERS DUMP"));
        }*/
      } else if (strcmp(strings[0], CMD_NMI) == 0) {
        //digitalWrite(Z80_NMI, LOW);
        NMI_LOW;
        delay(500);
        //digitalWrite(Z80_NMI, HIGH);
        NMI_HIGH;
 
        if (isComputerConnected) Serial.println(F("!OK;")); else Serial.println(F("OK"));
      } else if (strcmp(strings[0], CMD_GET_INFO) == 0) {
        if (isComputerConnected) {
          Serial.print(F("!INFO:CARD:ALEX80,"));
          if (clockFromArduino) {
            Serial.print(F("SCLK:A,FCLK:"));
            Serial.print(freq);
            if (clockEnabled) Serial.print(F(",ECLK:1,")); else Serial.print(F(",ECLK:0,"));
          } else {
            Serial.print(F("SCLK:E,"));
          }
          Serial.print(F("SMEM:E"));
          Serial.println(";");
        } else {
          Serial.println(F("Settings"));
          Serial.println(F("  Card: ALEX80"));
          if (clockFromArduino) {
            Serial.println(F("  Clock: Arduino"));
            Serial.print(F("  Clock freq: "));
            Serial.print(freq);
            Serial.println(F(" Hz"));
            if (clockEnabled) Serial.println(F("  Clock: On")); else Serial.println(F("  Clock: Off"));
          } else {
            Serial.println(F("  Clock: External"));
          }
          Serial.println(F("  Memory: External"));
        }
      } else if (strcmp(strings[0], CMD_MONITOR) == 0) {
        if (index == 2) {
          if (strcmp(strings[1], "on") == 0) {
            setClockFrequency(500);
            setClockFromArduino();
            startClock();

            monitorEnabled = true;

            // Carica il programma demo in memoria (Funziona se c'è il clock attivo altrimenti non riceve il bus grant)
            busGranted = requestBus();
            if (busGranted) {
              setAddCtrlBusOutput(true);
              for (int i=0; i<sizeof(demo_prog); ++i) writeRAM(0x8000+i, pgm_read_byte(&demo_prog[i]));
              for (int i=0; i<sizeof(monitor_prog); ++i) writeRAM(0xF010+i, pgm_read_byte(&monitor_prog[i]));
              writeRAM(0xF000, 0x10);
              writeRAM(0xF001, 0xF0);
              setAddCtrlBusOutput(false);
              releaseBus();
              busGranted = false;
            }
            activateClockInterrupt();

            if (isComputerConnected) Serial.println(F("!OK;")); else Serial.println(F("ARDUINO CLOCK IS ACTIVE!"));
          } else if (strcmp(strings[1], "off") == 0) {
            deactivateClockInterrupt();
            stopClock();
            setClockExternal();

            monitorEnabled = false;
            
            if (isComputerConnected) Serial.println(F("!OK;")); else Serial.println(F("ARDUINO CLOCK OFF. IT IS SAFE TO USE AN EXTERNAL CLOCK."));
          } else {
            if (isComputerConnected) Serial.println(F("!ERROR:WRONG PARAMETER;")); else Serial.println(F("WRONG PARAMETER"));
          }
        } else {
          if (isComputerConnected) Serial.println(F("!ERROR:MISSING PARAMETERS;")); else Serial.println(F("MISSING PARAMETERS"));
        }
      } else if (strcmp(strings[0], CMD_HELP) == 0) {
        Serial.println(F("Usage:"));
        Serial.println(F(""));
        Serial.println(F("r start length                      read exadecimal"));
        Serial.println(F("rb start length                     read binary"));
        Serial.println(F("w start                             write exadecimal, new line to end"));
        Serial.println(F("wb start                            write binary"));
        Serial.println(F("sclk freq                           set clock frequency"));
        Serial.println(F("clk start/stop/arduino/external     start/stop clock"));
        Serial.println(F("cc                                  set computer connected"));
        Serial.println(F("hc                                  set human conneted"));
        Serial.println(F("echo on/off                         set echo on/off"));
        Serial.println(F("log on/off                          set log on/off"));
        Serial.println(F("nmi                                 send nmi"));
        Serial.println(F("monitor on/off                      activate/deactivate monitor"));
        Serial.println(F("info                                get config infos"));
        Serial.println(F("debug on/off                        start/stop debug"));
        Serial.println(F("monitor on/off                      start/stop monitor"));
        Serial.println(F("?                                   this help"));
      } else {
        if (isComputerConnected) Serial.println(F("!ERROR:WRONG COMMAND;")); else Serial.println(F("WRONG COMMAND"));
      }
    } else {
      if (isComputerConnected) Serial.println(F("!ERROR:WRONG COMMAND;")); else Serial.println(F("WRONG COMMAND"));
    }
  }
}
