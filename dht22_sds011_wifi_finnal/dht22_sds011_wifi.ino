#include <DHT22.h>
#include <RF24.h>
#include <RF24_config.h>
#include <SPI.h>
#define DHT22_PIN 2
DHT22 dht(DHT22_PIN);

// ce,csn pins
RF24 radio(9, 10);

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
unsigned int Pm25 = 0;
unsigned int Pm10 = 0;
float pm25 = 0;
float pm10 = 0;
float t = 0;
float h = 0;
char buffers[10];
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
//sign a count
//sign a count
void setup(void)
{
  Serial.begin(9600);
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
}
void loop()
{
  delay(10000);
  char SendPayload[32] = "";
  // do temperatures
  DHT22_ERROR_t errorCode;
  errorCode = dht.readData();
  t = dht.getTemperatureC();
  Serial.println(t);

  strcat(SendPayload, "T");
  dtostrf(t, 2, 2, buffers);
  // move buffer to payload
  strcat(SendPayload, buffers);
  //add temperature flag
  strcat(SendPayload, "T");

  //do humidity
   h = dht.getHumidity();
  Serial.println(h);
  strcat(SendPayload, "H");
  // cut and load to buffer
  dtostrf(h, 2, 2, buffers);
  // move buffer to payload
  strcat(SendPayload, buffers);
  //add temperature flag
  strcat(SendPayload, "H");

//do pm2.5
ProcessSerialData();
pm25 = float(Pm25) / 10;
pm10 = float(Pm10) / 10;

strcat(SendPayload, "S");
// cut and load to buff
dtostrf(pm25, 2, 2, buffers);
// move buffer to payload
strcat(SendPayload, buffers);
//add temperature flag
strcat(SendPayload, "S");

strcat(SendPayload, "B");
// cut and load to buffer
dtostrf(pm10, 2, 2, buffers);
// move buffer to payload
strcat(SendPayload, buffers);
//add temperature flag
strcat(SendPayload, "B");

//RADIO send a heartbeat
radio.stopListening();
radio.write(&SendPayload, sizeof(SendPayload));
radio.startListening();

}



void ProcessSerialData()
{
  uint8_t mData = 0;
  uint8_t i = 0;
  uint8_t mPkt[10] = {0};
  uint8_t mCheck = 0;
  while (Serial.available() > 0)
  {
    // packet format: AA C0 PM25_Low PM25_High PM10_Low PM10_High 0 0 CRC AB
    mData = Serial.read();     delay(2);//wait until packet is received
    if (mData == 0xAA) //head1 ok
    {
      mPkt[0] =  mData;
      mData = Serial.read();
      if (mData == 0xc0) //head2 ok
      {
        mPkt[1] =  mData;
        mCheck = 0;
        for (i = 0; i < 6; i++) //data recv and crc calc
        {
          mPkt[i + 2] = Serial.read();
          delay(2);
          mCheck += mPkt[i + 2];
        }
        mPkt[8] = Serial.read();
        delay(2);
        mPkt[9] = Serial.read();
        if (mCheck == mPkt[8]) //crc ok
        {


          Pm25 = (uint16_t)mPkt[2] | (uint16_t)(mPkt[3] << 8);
          Pm10 = (uint16_t)mPkt[4] | (uint16_t)(mPkt[5] << 8);
          if (Pm25 > 9999)
            Pm25 = 9999;
          if (Pm10 > 9999)
            Pm10 = 9999;
          //get one good packet
          return ;
        }
      }
    }
  }

}

