#include <AD5933.h> // https://github.com/WuMRC/drive/wiki
#include <Wire.h> // I²C Bus


#define PI 3.14159

String Rescal, messnum; 
double gainfactor, sysphase, temp = 0;
double impmag = 0, phase = 0, real = 0, imag = 0;
double impt = 0, phaset = 0, impavg, phaseavg, realavg, imagavg;
double rad = PI /180;
String mode = "rest";
byte freq;

void setup() {
    
    Wire.begin(); // I²C Bus
    Serial.begin(9600); // Arduino serial zu kommunizieren
    AD5933.setExtClock(false); // Externe Clocksignale (true/false)
    AD5933.resetAD5933();    
    AD5933.setStartFreq(100000); // 100 KHz (Maximale Frequenz von AD5933)
    AD5933.setSettlingCycles(15, 1);       
    AD5933.setStepSize(50000);
    AD5933.setNumofIncrement(1);
    AD5933.setPGA(GAIN_1); // PGA x1
    AD5933.setRange(RANGE_4); // Range 4 200 mV
    AD5933.tempUpdate();
    AD5933.setCtrMode(STAND_BY);
    //Kalibiration

      long value;
      value = 200000; // wert von string zu integer wandlern
    AD5933.getGainFactor (value, 4, gainfactor, sysphase); // Mit den Kalibrierungswiderstand kann das Systems die Verstäerkung und Phase des Systems berechnen
      
}

void loop() {

      anzahl:
      messnum =1; // Kalibrationswiderstand lesen

      AD5933.tempUpdate();      
      AD5933.setCtrMode(STAND_BY);
      AD5933.setCtrMode(INIT_START_FREQ);
      AD5933.setCtrMode(START_FREQ_SWEEP);
      
      AD5933.getComplex(gainfactor, sysphase, impmag, phase); // Magnitude und Phase von unbekannte Impedanz berechnen. Wichtig: Das System wartet bis die Messung beendet ist
                       
      real = impmag * cos (phase * rad); // Reale Teil berechnen
      imag = impmag * sin (phase * rad); // Imaginär Teil berechnen

      AD5933.setCtrMode(POWER_DOWN);                 
    
      Serial.print("Impedance: ");
      Serial.print(real);
      Serial.print("\n");
  
 }
