// Gestisce la memorizzazione in EEPROM di alcuni parametri di funzionamento:
// - clock arduino o esterno
// - frequenza di clock (utilizzato solo con clock da Arduino)
// - clock attivo (utilizzato solo con clock da Arduino)

#include <EEPROM.h>

#define CLK_ARD_LOC         0 
#define CLK_FREQ_LOC        1 

void eeprom_init() {
  byte data;

  EEPROM.get(CLK_ARD_LOC, data);
  if (data==0xff || data==0) setClockExternal(false); else setClockFromArduino(false);
  EEPROM.get(CLK_FREQ_LOC, freq);
  if (freq == 0xffff || freq==0) setClockFrequency(50);
}

void eeprom_save_clk_ard() {
  EEPROM.write(CLK_ARD_LOC, clockFromArduino?1:0);
}

void eeprom_save_clk_freq() {
  EEPROM.put(CLK_FREQ_LOC, freq);
}
