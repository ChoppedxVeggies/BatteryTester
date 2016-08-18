
#include <SPI.h> //include the SPI library tool


/*
   This program can be used to check the functionality of the DAC. The DAC needs a 16 bit word to work, 
   the first 4 bits are control bits and the remaining bits contain the value we want to send. 
   With 12 bits available we will have a range of values from 0-4095, which correspond to 0-5V.
 
 */

const int cs = 10;

void setup() {
pinMode(cs, OUTPUT);
digitalWrite(cs, HIGH);
Serial.begin(9600);
SPI.begin();                   //start the chip that's selected
SPI.setBitOrder(MSBFIRST);
//SPI.setDataMode(SPI_MODE0);      //select the 1,1 mode of the DAC
}

void loop() {
  for (int i=2149; i <= 4095; i++){
  setDac(i,0);               //MOSFET opens around 2152
  Serial.println(i);
  
  }
}

// Function to set the DAC
void setDac(int value, int channel) {
  byte dacRegister = 0b00110000;      //First four bits are mode selections for DAC
  int dacSecondaryByteMask = 0b0000000011111111;
  byte dacPrimaryByte = (value >> 8) | dacRegister; //Shifts value 8 bits to the right
  byte dacSecondaryByte = value & dacSecondaryByteMask;
  noInterrupts();
  digitalWrite(cs, LOW);
  SPI.transfer(dacPrimaryByte);
  SPI.transfer(dacSecondaryByte);
  delay(1000);                        //Check delay
  digitalWrite(cs, HIGH);
  interrupts();
}


