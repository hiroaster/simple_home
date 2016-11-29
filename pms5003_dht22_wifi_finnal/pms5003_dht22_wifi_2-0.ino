#include <DHT22.h>
#include <RF24.h>
#include <RF24_config.h>
#include <SPI.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#define DHT22_PIN 2
DHT22 dht(DHT22_PIN);
#include <SoftwareSerial.h>
SoftwareSerial mySerial(5, 6);//rx,tx
// ce,csn pins
RF24 radio(9, 10);
LiquidCrystal_I2C lcd(0x27,16,2); // set the LCD address to 0x27 for a 16 chars and 2 line display

//nRF24 set the pin 9 to CE and 10 to CSN/SS
// Cables are:
//     SS       -> 10
//     MOSI     -> 11
//     MISO     -> 12
//     SCK      -> 13

/*
  SDS011 采集端

  引脚接法：
  SDS011   Arduino UNO
  CTL <-> 悬空
  1um <-> 悬空
  5v <-> 5V  PIN#D8
  25um <-> 悬空
  RX0 <-> rx
  TX0 <-> tx
*/
 int Pm25 = 0;
 int Pm10 = 0;
float t = 0;
float h = 0;
char buffers[10];
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
//sign a count
//sign a count
void setup(void)
{
  Serial.begin(9600);
  mySerial.begin(9600);
  //SDS init
  Pm25 = 0;
  Pm10 = 0;
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(0x4c);
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1, pipes[0]);
  // radio.openReadingPipe(1,0xF0F0F0F0C4LL);
  radio.setDataRate(RF24_250KBPS);
  radio.setPayloadSize(32);
  radio.setAutoAck(true);
  radio.powerUp();
  
lcd.init(); // initialize the lcd 
lcd.backlight(); //Open the backlight

}
void loop()
{
  delay(10000);
  char SendPayload[32] = "";
  // do temperatures
  DHT22_ERROR_t errorCode;
  errorCode = dht.readData();
  t = dht.getTemperatureC();
  strcat(SendPayload, "T");
  dtostrf(t, 2, 1, buffers);
  // move buffer to payload
  strcat(SendPayload, buffers);
  //add temperature flag
  strcat(SendPayload, "T");

  //do humidity
   h = dht.getHumidity();
  strcat(SendPayload, "H");
  // cut and load to buffer
  dtostrf(h, 2, 1, buffers);
  // move buffer to payload
  strcat(SendPayload, buffers);
  //add temperature flag
  strcat(SendPayload, "H");

//do pm2.5
ProcessSerialData();

strcat(SendPayload, "S");
// cut and load to buff
dtostrf(Pm25, 2, 0, buffers);
// move buffer to payload
strcat(SendPayload, buffers);
//add temperature flag
strcat(SendPayload, "S");

strcat(SendPayload, "B");
// cut and load to buffer
dtostrf(Pm10, 2, 0, buffers);
// move buffer to payload
strcat(SendPayload, buffers);
//add temperature flag
strcat(SendPayload, "B");

//RADIO send a heartbeat
radio.stopListening();
radio.write(&SendPayload, sizeof(SendPayload));
radio.startListening();


lcd.setCursor(0,0);
lcd.print("T:");// Print a message to the LCD
lcd.print(t,1);// Print a message to the LCD
lcd.print("C");// Print a message to the LCD
lcd.print(" H:");// Print a message to the LCD
lcd.print(h,1);// Print a message to the LCD
lcd.print("%");// Print a message to the LCD


delay(5000);

}



void ProcessSerialData()//读取PMS5003的数据
{
  uint8_t mData = 0;
  uint8_t i = 0;
  uint8_t mPkt[32] = {0};
  int mCheck = 0;
while (mySerial.available() > 0) 
  {  
    //Basing on the protocol of Plantower PMS1003
    mData = mySerial.read();     
    delay(2);//wait until packet is received
    if(mData == 0x42)//head1 ok
     {
        mPkt[0] =  mData;
        mData = mySerial.read();
        if(mData ==0x4d)//head2 ok
        {
          mPkt[1] =  mData;
          mCheck = 143;
          for(int i=2;i < 30;i++)//data recv and crc calc
          {
             mPkt[i] = mySerial.read();
             delay(2);
             mCheck += mPkt[i];
          }
          mPkt[30] = mySerial.read();
          delay(1);
          mPkt[31] = mySerial.read();
          Serial.println();
          Serial.print(mCheck);
          Serial.print("  ");
          Serial.println(mPkt[30]*256+mPkt[31]);
          if(mCheck == mPkt[30]*256+mPkt[31])//crc ok
          {
            Serial.println("done");
            Serial.flush();
            Pm25 = mPkt[12]*256+mPkt[13];
            Pm10 = mPkt[14]*256+mPkt[15];
            lcd.clear();
            lcd.setCursor(0,1); //newline
            lcd.print("PM2.5: ");// Print a message to the LCD
            lcd.print(Pm25);// Print a message to the LCD
            return;
          }
        }      
     }
   } 
}
