#include <AD5933.h> // https://github.com/WuMRC/drive/wiki
#include <Wire.h> // I²C Bus


#define PI 3.1415926535897932384626433832795

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
    
    
    Serial.print("\n");
    Serial.print("\Impedance Analyzer  AD5933");
    Serial.print("\n\n");
    Serial.print("The sensor is configured with maximum frequency 100 KHz (maximal) \n and out signal of  198 mV p-p");
    Serial.print("\n\n");

    temp = AD5933.getTemperature();
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.print("\n\n");

    //Kalibiration

    Serial.print("Calibrating");
      Serial.print("\n\n");
      delay(1000);
      
      Serial.print("Calibration Resistance(Ohms): "); 
      while(!Serial.available()){}
      Rescal = Serial.readString(); // Kalibrationswiderstand lesen
      long value;
      value = Rescal.toInt(); // wert von string zu integer wandlern
      Serial.print(value);
      Serial.print("\n\n");
      
      AD5933.tempUpdate();        
      AD5933.getGainFactor (value, 4, gainfactor, sysphase); // Mit den Kalibrierungswiderstand kann das Systems die Verstäerkung und Phase des Systems berechnen
                       
      delay(1000);
      Serial.print("Gain Factor: ");
      Serial.print(gainfactor);
      Serial.print("\n");

      delay(500);
      Serial.print("System Phase: ");
      Serial.print(sysphase);
      Serial.print("\n\n");

      delay(500);
      Serial.print("Completed");
      Serial.print("\n\n");
      
      mode = "rest";
      
      delay(2000);

}

void loop() {

      Serial.print("Measurement");
      Serial.print("\n\n");
      delay(1000);      

      anzahl:
      messnum =1; // Kalibrationswiderstand lesen
      long value2;
      value2 = messnum.toInt(); // wert von string zu integer wandlern
      
      if (value2 == 0){
        Serial.print("Not correct ");
        Serial.print("\n\n");
        goto anzahl;
      }
      
      Serial.print(value2);
      Serial.print(" Measuring");
      Serial.print("\n\n");


      Serial.print("Writing stopped, Measuring ended");
      Serial.print("\n\n");
      
      AD5933.tempUpdate();      
      AD5933.setCtrMode(STAND_BY);
      AD5933.setCtrMode(INIT_START_FREQ);
      AD5933.setCtrMode(START_FREQ_SWEEP);
      
           
      int i;
      for (i = 0; i < value2; i++){
                    
      AD5933.getComplex(gainfactor, sysphase, impmag, phase); // Magnitude und Phase von unbekannte Impedanz berechnen. Wichtig: Das System wartet bis die Messung beendet ist
                       
      real = impmag * cos (phase * rad); // Reale Teil berechnen
      imag = impmag * sin (phase * rad); // Imaginär Teil berechnen

      impt = impt + impmag;
      phaset = phaset + phase;
      impavg = impt / (i+1);
      phaseavg = phaset / (i+1);
      realavg = impavg * cos (phaseavg * rad); // Reale Teil berechnen
      imagavg = impavg * sin (phaseavg * rad); // Imaginär Teil berechnen

      
      if (Serial.readString() == "stop"){

        Serial.print("Measuring stopped");
        Serial.print("\n\n");
        break;
      }
      
      }

      AD5933.setCtrMode(POWER_DOWN);
                     
    
      Serial.print("Impedance: ");
      Serial.print(impavg);
      Serial.print("\n");

    
      Serial.print("Phase angle: ");
      Serial.print(phaseavg);
      Serial.print("\n");

      Serial.print("Real: ");
      Serial.print(realavg);
      Serial.print("\n");

      
      Serial.print("Imaginary: ");
      Serial.print(imagavg);
      Serial.print("\n\n");
      
     
      Serial.print("Complete");
      Serial.print("\n\n");

      impt = 0;
      phaset = 0;
      mode = "rest";

      

}
