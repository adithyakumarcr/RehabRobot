#include<AD5933.h>
#include<Wire.h>

#define PI 3.1415926535897932384626433832795

String Rescal, messnum; 
double gainfactor, sysphase, temp = 0;
double impmag = 0, phase = 0, real = 0, imag = 0;
double impt = 0, phaset = 0, impavg, phaseavg, realavg, imagavg;
double rad = PI /180;
String mode = "rest";
byte freq;

void setup(){

  Wire.begin(); //To begin the I2C Bus
  Serial.begin(9600); //Communication with Ardiuno with Serial Monitor

  AD5933.setExtClock(false);
  AD5933.resetAD5933();

  AD5933.setStartFreq(100000); //Setting frequency to 100Khz (Maximum for AD5933) 
  AD5933.setSettlingCycles(15,1);
  AD5933.setStepSize(50000);

  AD5933.setNumofIncrement(1);
  AD5933.setPGA(GAIN_1); // PGA x1
  AD5933.setRange(RANGE_4); // Range 4 200 mV
  AD5933.tempUpdate();
  AD5933.setCtrMode(STAND_BY);

  Serial.print("\n\n Impedance Analyzer AD5933");
  Serial.print("\n The sensor is configured to run at 100Khz and out signal of 198mV ");
  Serial.print("\n\n");

  temp=AD5933.getTemperature();
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print("\n\n");

  //Calibration

  Serial.print("\n\n");
  Serial.print("Calibration Resistance(Ohms): "); 
  while(!Serial.available()){}
  Rescal = Serial.readString(); // Kalibrationswiderstand lesen
  long value;
  value = Rescal.toInt(); // wert von string zu integer wandlern
  Serial.print(value);
  Serial.print("\n\n");
    
  AD5933.tempUpdate();        
  AD5933.getGainFactor (value, 4, gainfactor, sysphase); // With the calibration resistor, we can calculate the gain factor

  Serial.print("Gain Factor: ");
  Serial.print(gainfactor);
  Serial.print("\n");

  Serial.print("System Phase: ");
  Serial.print(sysphase);
  Serial.print("\n\n");

  delay(500);
  Serial.print("Completed");
  Serial.print("\n\n");
  
  
}

void loop() {

  Serial.print("\n Measuring..");
  Serial.print("\n\n");

   AD5933.tempUpdate();      
   AD5933.setCtrMode(STAND_BY);
   AD5933.setCtrMode(INIT_START_FREQ);
   AD5933.setCtrMode(START_FREQ_SWEEP);

   AD5933.getComplex(gainfactor, sysphase, impmag, phase); //Gets the values of Magnitude and Phase
                       
   real = impmag * cos (phase * rad); // Real value (R)
   imag = impmag * sin (phase * rad); // Imaginary value (I)

   AD5933.setCtrMode(POWER_DOWN);

   Serial.print("Impdedance: ");
   Serial.print(impmag);
   delay(500);

}

     
