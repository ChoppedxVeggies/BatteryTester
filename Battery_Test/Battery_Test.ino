/*
***************************************************
*                                                 *
*                Battery Tester                   *
*              Occidental College                 *
*           S. Renner and J. Drumright            *
*                     Ψ-III                       *
*                                                 *
*                                                 *
*                                                 *
***************************************************

This is the code for a simple batery tester.
It uses the following I/O:

Analog-to-digital (A/D) channel A0 for the value of the voltage
A/D channel A1 for the value of the reference voltage

It will output the values to the serial terminal where they
can be cut and pasted into Excel.

In-line comments are provided throughout for explainations
*/

#include <SPI.h>                                           // include the SPI library
const int cs                          = 10;                  // chip Select pin on Arduino
const int voltage_numReadings         = 60;                  // size of the voltage readings array
const int voltage2_numReadings        = 60;                  // size of the second voltage readings array
const int current_numReadings          = 5;


// Variables used throughout program are first initialized here for future refence

int voltage_readings[voltage_numReadings];                 // the readings from the analog input
int voltage_readIndex = 0;                                 // the index of the current reading
int voltage_total = 0;                                     // the running total
int voltage_average = 0;                                   // the average
int inputPin1 = A1;                                        // the data input pin on the Arduino
int voltage2_readings[voltage2_numReadings];           // see above
int voltage2_readIndex = 0;
int voltage2_total = 0;
int voltage2_average = 0;
int inputPin = A0;
int current_readings[current_numReadings];             // see above
int current_readIndex = 0;
int current_total = 0;
int current_average = 0;
float true_voltage = 0;
float true_voltage2 = 0;
int DACinput              = 2300;          // starts the DAC output at around 2.8V for
int VoltageValue          = 0;             // some variables
int datapoints            = 0;             // loop counter
int timer                 = 1;             // battery test timer
float voltage             = 0;
float secondvoltage       = 0;
float LoadResistor        = 1.00;           // the resistor that is used to draw a load in the circuit
float calibrationfactor   = 1.024;          // a scale factor for the A/D input (may need further adjusting)  
float currentLevel        = 200;            // user setable current value for battery test - in milivolts
String readString1;

void setup()                                // the setup code is placed here, the code is completed once:
{

  Serial.begin(9600);                                    // start the serial monitor
  pinMode(cs, OUTPUT);
  digitalWrite(cs, HIGH);                                // sets DAC input high
  SPI.begin();                                           // starts the chip select
  SPI.setBitOrder(MSBFIRST);                             // Most Significant Bit First

  /* This portion takes the analog input data, places a datapoint
     into an array slot, then moves the array input to the next empty slot
     for the insert of the next data point */

  for (int A1_Reading = 0; A1_Reading < voltage_numReadings; A1_Reading++)
  {
    voltage_readings[A1_Reading] = 0;
  }
  
  for (int A0_Reading = 0; A0_Reading < voltage2_numReadings; A0_Reading++)
  {
    voltage2_readings[A0_Reading] = 0;
  }

  for (int current_Reading = 0; current_Reading < current_numReadings; current_Reading++)
  {
    current_readings[current_Reading] = 0;
  }
}

void loop() // the main code is here and is completed repeatedly (indicated by the loop)
{

  setDac(0, 0); // set DAC input to 0 to remove load from previous test
  Serial.println();
  Serial.println("Occidental College Battery Tester");
  Serial.println();
  delay(2000); // add pause for dramatic effect
  Serial.print("Please Input current Level (in miliamps), no higher than 600: ");

  do {} while (Serial.available() == 0); // loop here to wait for user input

  while (Serial.available()) {           // read currrent limit in mills
    char c = Serial.read();              // gets a byte from serial buffer
    readString1 += c;                    // concatinate the string
    delay(2);                            // allow buffer to fill with next character
  }

  if (readString1.length() > 0) {
    Serial.println(readString1);         // so you can see the captured string
    currentLevel = readString1.toInt();  // convert readString into a number
    delay(1000);
  }

  Serial.println();                             // prints text to serial monitor for user's notfication
  Serial.println("Start Battery Testing ...");
  Serial.println("\r");
  delay(1000);
  Serial.print("Setting current level to ");
  Serial.print(currentLevel);
  Serial.println(" miliamps");
  Serial.println("\r");
  delay(1000);
  Serial.println("Averaging Data Points... ");
  Serial.println();
  delay(1000);
  Serial.println("Time - Voltage - current");
  Serial.println();

  while (DACinput != 4095) {             // run the the loop until the DAC reaches its maximum output of 4095

    setDac(DACinput, 0);                 // begin to adjust DACinput value to adjust load on battery

    /* This portion of the code repeatedly reads from  analog input A1 (voltage) or A0 (secondvoltage), calculates a running average
       and prints it to the computer. The specified nunber of readings are put into an array and
       continually averaged.
     */

    // Average of Analog A0 voltage
    voltage_total = voltage_total - voltage_readings[voltage_readIndex];           // subtract the last reading:
    voltage_readings[voltage_readIndex] = analogRead(A1);                          // read from the sensor:
    voltage_total = voltage_total + voltage_readings[voltage_readIndex];           // add the reading to the total:
    voltage_readIndex = voltage_readIndex + 1;                                     // advance to the next position in the array:

    if (voltage_readIndex >= voltage_numReadings) {                                // if we're at the end of the array...
      voltage_readIndex = 0;                                                       // ...wrap around to the beginning:
    }

    (float) voltage_numReadings;          //typecast the readings into a floating point
    float voltage_average   =  voltage_total / voltage_numReadings;                                             // calculate the average
    float voltage  =  voltage_average * (5.0 / 1023.0) * (33.0 + 47.0) / 47.0 * calibrationfactor * 1.0015;     // convert the reading (0 - 1023) to a voltage (0-5) with proper scale factor

    true_voltage = analogRead(A1) * (5.0 / 1023.0) * (33 + 47) / 47 * calibrationfactor;                        // unaveraged voltage reading used for calibrations

    // Average of Analog A1 secondvoltage
    voltage2_total = voltage2_total - voltage2_readings[voltage2_readIndex];                                    // see above
    voltage2_readings[voltage2_readIndex] = analogRead(A0);
    voltage2_total = voltage2_total + voltage2_readings[voltage2_readIndex];
    voltage2_readIndex = voltage2_readIndex + 1;

    true_voltage2 = analogRead(A0) * (5.0 / 1023.0) * (33 + 47) / 47 * calibrationfactor;

    if (voltage2_readIndex >= voltage2_numReadings) {
      voltage2_readIndex = 0;
    }

    (float) voltage2_numReadings;
    float voltage2_average  =  voltage2_total / voltage2_numReadings;
    float secondvoltage =  voltage2_average * (5.0 / 1023.0) * calibrationfactor * (33 + 47) / 47;

    float current = (true_voltage - true_voltage2) * 1000 / LoadResistor;   // finds voltage difference between the two readings and converts that value into a current reading

    current_total = current_total - current_readings[current_readIndex];    // see above
    current_readings[current_readIndex] = current;
    current_total = current_total + current_readings[current_readIndex];
    current_readIndex = current_readIndex + 1;
    current_average    =  current_total / current_numReadings;

    if (current_readIndex >= current_numReadings) {
      current_readIndex = 0;
    }

    // DAC adjustment code
    if (datapoints > 100 && current_average < currentLevel ) {
      DACinput = DACinput + 1;
      setDac(DACinput, 0);
      delay(50);
    }
    if (datapoints > 100 && current_average > currentLevel) {
      DACinput = DACinput - 1;
      setDac(DACinput, 0);
      delay(50);
    }
    if (datapoints > 100 && current_average == currentLevel) {
      setDac(DACinput, 0);
      delay(50);
    }

    if (datapoints > 600) {

      Serial.print(timer );                                                        // print the timer tick
      Serial.print("\t");
      Serial.print(voltage, 4);                                                    // print the voltage with 4 decimal places
      Serial.print("\t");
      //Serial.print(secondvoltage);                                              
      Serial.println(current_average);                                             // print the current
      //Serial.print("\t");
      //Serial.println(DACinput);                                                  // NOTE: Delete the slashes in front of code to display data in 
      timer ++;                                                                    // parenthesis. The data can be used for troublehsooting if values                       
      delay(1000);                                                                 // are not correct. Remember to move the println to the last
    }                                                                              // statement desired to printed to the serial monitor.
    datapoints ++;
  }
}

// function to set the DAC, accepts the value to be sent and the channel of the DAC to be used.
void setDac(int value, int channel) {
  byte dacRegister = 0b00110000;                                            // sets default DAC registers B00110000, 1st bit choses DAC, A=0 B=1, 2nd Bit bypasses input Buffer, 3rd bit sets output gain to 1x, 4th bit controls active low shutdown. LSB are insignifigant here.
  int dacSecondaryByteMask = 0b0000000011111111;                            // isolates the last 8 bits of the 12 bit value, B0000000011111111.
  byte dacPrimaryByte = (value >> 8) | dacRegister;                         // value is a maximum 12 Bit value, it is shifted to the right by 8 bytes to get the first 4 MSB out of the value for entry into th Primary Byte, then ORed with the dacRegister
  byte dacSecondaryByte = value & dacSecondaryByteMask;                     // compares the 12 bit value to isolate the 8 LSB and reduce it to a single byte
  noInterrupts();                                                           // disable interupts to prepare to send data to the DAC
  digitalWrite(cs, LOW);                                                    // take the Chip Select pin low to select the DAC:
  SPI.transfer(dacPrimaryByte);                                             // send in the Primary Byte:
  SPI.transfer(dacSecondaryByte);                                           // send in the Secondary Byte
  delay(1000);
  digitalWrite(cs, HIGH);                                                   // take the Chip Select pin high to de-select the DAC:
  interrupts();                                                             // enable interupts
}
