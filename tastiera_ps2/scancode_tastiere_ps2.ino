/***************************************************************
 Sketch Arduino per lettura scan code tastiera PS/2

 Utilizzato per verificare il funzionamento del protocollo
 PS/2 per poi collegare la tastiera alla scheda ALEX80

 Video YouTube: https://youtu.be/ZvvwGuTVjT0
*****************************************************************/

#define DATA_PIN  8   // Pin dati della tastiera PS/2
#define CLOCK_PIN 9   // Pin clock della tastiera PS/2

void setup() {
    Serial.begin(9600);
    pinMode(CLOCK_PIN, INPUT_PULLUP);
    pinMode(DATA_PIN, INPUT_PULLUP);
    
    delay(5000);
    Serial.println("Accende num lock e caps lock.");
    setKeyboardLEDs(true, true, false);
    delay(5000);
    Serial.println("Spegne tutto.");
    setKeyboardLEDs(false, false, false);
}

unsigned char readScanCode() {
    while (digitalRead(CLOCK_PIN) == HIGH); // Attesa del bit di start
    
    unsigned char scanCode = 0;
    int onesCount = 0; // Contatore per il numero di bit a 1
    
    for (int i = 0; i < 8; i++) {
        while (digitalRead(CLOCK_PIN) == LOW);
        while (digitalRead(CLOCK_PIN) == HIGH);
        
        if (digitalRead(DATA_PIN)) {
            scanCode |= (1 << i);
            onesCount++; // Incrementa se il bit è 1
        }
    }
    
    while (digitalRead(CLOCK_PIN) == LOW);
    while (digitalRead(CLOCK_PIN) == HIGH);
    bool parityBit = digitalRead(DATA_PIN); // Lettura del bit di parità
    
    while (digitalRead(CLOCK_PIN) == LOW);
    while (digitalRead(CLOCK_PIN) == HIGH); // Attesa del bit di stop
    
    // Controllo della parità dispari: il numero di bit a 1 deve essere dispari
    bool calculatedParity = (onesCount % 2 == 0); // Se pari, il bit di parità deve essere 1
    
    if (calculatedParity != parityBit) {
        Serial.println("Errore di parità!");
        return 0; // Restituisce 0 in caso di errore di parità
    }
    
    return scanCode;
}

bool sendCommand(unsigned char command) {
    pinMode(CLOCK_PIN, OUTPUT);
    digitalWrite(CLOCK_PIN, LOW);
    delayMicroseconds(100);
    pinMode(DATA_PIN, OUTPUT);
    digitalWrite(DATA_PIN, LOW);
    delayMicroseconds(100);
    pinMode(CLOCK_PIN, INPUT_PULLUP); // Rilascia il clock per farlo generare alla tastiera
    
    int onesCount = 0;
    for (int i = 0; i < 8; i++) {
        while (digitalRead(CLOCK_PIN) == HIGH);
        digitalWrite(DATA_PIN, (command >> i) & 1);
        if ((command >> i) & 1) onesCount++;
        while (digitalRead(CLOCK_PIN) == LOW);
    }
    
    // Invia il bit di parità (parità dispari)
    while (digitalRead(CLOCK_PIN) == HIGH);
    digitalWrite(DATA_PIN, !(onesCount % 2));
    while (digitalRead(CLOCK_PIN) == LOW);
    
    // Invia il bit di stop
    while (digitalRead(CLOCK_PIN) == HIGH);
    digitalWrite(DATA_PIN, HIGH);
    while (digitalRead(CLOCK_PIN) == LOW);
    
    pinMode(DATA_PIN, INPUT_PULLUP);
    
    // Attendere l'ACK dalla tastiera
    while (digitalRead(CLOCK_PIN) == HIGH);
    if (digitalRead(DATA_PIN) == LOW) {
        while (digitalRead(CLOCK_PIN) == LOW);
        unsigned char ack = readScanCode();
        if (ack == 0xFA) {
            Serial.println("ACK ricevuto correttamente");
            return true;
        } else {
            Serial.print("Errore: ricevuto ");
            Serial.println(ack, HEX);
            return false;
        }
    }
    return false;
}

void setKeyboardLEDs(bool numLock, bool capsLock, bool scrollLock) {
    sendCommand(0xED); // Comando per impostare i LED
    delay(10);
    unsigned char ledStatus = (scrollLock << 0) | (numLock << 1) | (capsLock << 2);
    sendCommand(ledStatus);
}

void loop() {
    if (digitalRead(CLOCK_PIN) == LOW) {
        unsigned char scanCode = readScanCode();
        if (scanCode != 0) { // Verifica che non ci sia stato errore di parità
            Serial.print("Scan Code: ");
            Serial.println(scanCode, HEX);
        }
    }
}
