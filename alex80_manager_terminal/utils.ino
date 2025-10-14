void printHex2(unsigned int val) {
  if (val < 0x10) Serial.print(F("0"));
  Serial.print(val, HEX);
}

void printHex4(unsigned int val) {
  if (val < 0x1000) Serial.print(F("0"));
  if (val < 0x100) Serial.print(F("0"));
  if (val < 0x10) Serial.print(F("0"));
  Serial.print(val, HEX);
}

void printFlags(byte f) {
  Serial.print(F("Flags: "));
  
  Serial.print(F("S="));
  Serial.print((f & 0x80) ? "1 " : "0 ");

  Serial.print(F("Z="));
  Serial.print((f & 0x40) ? "1 " : "0 ");

  Serial.print(F("H="));
  Serial.print((f & 0x10) ? "1 " : "0 ");

  Serial.print(F("P/V="));
  Serial.print((f & 0x04) ? "1 " : "0 ");

  Serial.print(F("N="));
  Serial.print((f & 0x02) ? "1 " : "0 ");

  Serial.print(F("C="));
  Serial.print((f & 0x01) ? "1" : "0");

  Serial.println();
}