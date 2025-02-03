# Codice per computer ALEX80 che permette il pilotaggio di LED NeoPixel con un po' di hardware esterno
  
Vedere video YouTube https://youtube.com/live/eqKhF6nzU44  
  
Compilato con z88dk https://github.com/z88dk/z88dk  
  
Per generare il file binario a.out  
zcc +z80 -clib=classic main.c breakout.c neopixel.c sound.c time.c letters.c hardware_new.asm  -create-app -m -lm  
-lm aggiunge le librerie matematiche per usare i float  
  
Per ottenere il listing del codice macchina  
z88dk-dis -o 0x8000 -x a.map a.rom > a.out  
