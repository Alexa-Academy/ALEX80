# ALEX80
Computer based on a Zilog Z80 which can be managed by an Arduino Mega  
Computer basato su uno Zilog Z80 che può essere gestito da un Arduino Mega  

## Caratteristiche
Microprocessore: Z80 6MHz  
Clock di sistema: Selezionabile tra 3.6864 MHz generato da quarzo locale alla scheda, clock generato da Arduino e clock generato manualmente con pulsante  
Reset: possibile con pulsante su scheda oppure da Arduino  
Memoria EEPROM: 32K - 28C256 (tramite adattatore 27C256 o 27C64)  
Memoria RAM: 32K - 62256  
Z80 PIO (opzionale): Accesso a Porta A e B con Ready e Strobe  
Z80 CTC (opzionale se non si usa il SIO): Accesso ai segnali CLK/TRG e ZC/TC  
Z80 SIO (opzionale): Accesso alla connessione seriale per l'utilizzo di un modulo FTDI  
IO aggiuntivo: Dupe porte da 8 bit in ingresso e due porte da 8 bit in uscita  
Due bus di espansione da 40 PIN  
Supporto display LCD  
Connettori per l'inserimento di un Arduino Mega (opzionale)  
Alimentazione da Arduino o da fonte esterna DC a 9V  

## Mappa memoria e IO
0x0000-0x7FFF      ROM  
0x8000-0xFFFF      RAM  

0x00        PIO Data A  
0x01        PIO Control A  
0x02        PIO Data B  
0x03        PIO Control B  
0x04        CTC C0  
0x05        CTC C1  
0x06        CTC C2  
0x07        CTC C3  
0x08        SIO Data A  
0x09        SIO Control A  
0x0A        SIO Data B  
0x0B        SIO Control B  
0x0C-0x0F   IN-OUT A con 74HCT273 e 74HCT244  
0x10-0x13   IN-OUT B con 74HCT273 e 74HCT244  
  
## Realizzazione
La realizzazione della scheda è descritta in questa serie di video YouTube  
1 - Z80 e EEPROM: https://youtu.be/4KqvvQpHY0I  
2 - Aggiunta RAM ed uso di Arduino TIMER: https://youtu.be/xPTAFL8nSBg  
3 - Aggiunta del decoder per la gestione degli I/O e blink di un LED: https://youtu.be/YlRGMI4QHdw  
4 - Lettura memoria da Arduino con DMA: https://youtu.be/-bkBDCRCxac  
5 - Scrittura RAM ed esecuzione del codice comandato da Arduino: https://youtu.be/zcbHsDlLe6I  
6 - Aggiunta del clock su scheda con NE555 e refactoring codice: https://youtu.be/Sk1F04g4o2U  
7 - Aggiunta dello Z80 PIO - Parallel Input Output: https://youtu.be/_GM3fBWz_60  
8 - Colleghiamo un display 1602 ad una porta del PIO: https://youtu.be/cb-bPxwCO_M  
9 - Aggiunta dello Z80 CTC - Counter Timer: https://youtu.be/WaMA73MsbgE  
10 - Quarzo da 6MHz su millefori: https://youtu.be/UldfmqZj3Rk  
11 - Aggiunta dello Z80 SIO per la comunicazione seriale: https://youtu.be/H8DIwzcfdJA  
12 - Utilizzo del clock con quarzo per la temporizzazione del SIO: https://youtu.be/jvysisM3B0E  
13 - Facciamo funzionare il Microsoft BASIC: https://youtu.be/tE3tAP2hR2E  
14 - Aggiungiamo dei moduli di IO e realizziamo il PCB: https://youtu.be/wMeW58kEyi4  
15 - Inizio saldatura su PCB: https://youtu.be/g2DVUZfGPAw  
16 - Continua la saldatura su PCB: https://youtu.be/GuwJ82NjnYw  
17 - Terminiamo la saldatura su PCB e facciamo i primi test: https://youtu.be/Bij9ebOSaQI  
18 - Prova delle varie funzionalità della scheda finita: https://youtu.be/arjoyaLRYoo  
19 - Collegamento di una tastiera PS2 tramite il SIO: https://youtu.be/ZvvwGuTVjT0  
20 - Collegamento di una matrice NeoPixel: https://youtube.com/live/eqKhF6nzU44  
24 - ALEX80 e ALEX80µ in kit - Facciamo il punto! Vediamo gli sketch per Arduino in funzione: https://youtu.be/FrGPm7ekkkc  
27 - ALEX80µ e ALEX80 vediamo le funzionalità degli ultimi sketch per Arduino: https://youtu.be/qI7mw8zR-SI  



## Versioni della scheda su PCB
Versione 1.0: Come abbiamo visto nei video 15, 16 e 17 questa prima versione ha qualche problema:  
* Come si può vedere nello schema Schematic_Z80-PCB_v1.0.png alcune connessioni di alimentazione si chiamano ancora Vcc e non risultano connesse nel PCB. Vanno rinominate in +5V
* Altri problemi illustrati nel video riguardano il PCB.

Versione 1.1: Risolti tutti i problemi che aveva la versione 1.0. Ridotte notevolmente le dimensioni della scheda mantenendo le stesse funzionalità.
