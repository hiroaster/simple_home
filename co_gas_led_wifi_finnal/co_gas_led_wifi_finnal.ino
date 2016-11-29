#include <RF24.h>
#include <RF24_config.h>
#include <SPI.h>

const int AOUTpin=0;//the AOUT pin of the methane sensor goes into analog pin A0 of the arduino
const int DOUTpin=8;//the DOUT pin of the methane sensor goes into digital pin D8 of the arduino
const int AOUT2pin=1;//the AOUT pin of the methane sensor goes into analog pin A0 of the arduino
const int DOUT2pin=7;//the DOUT pin of the methane sensor goes into digital pin D8 of the arduino
//const int alertPin=3;//the anode of the LED connects to digital pin D13 of the arduino
//const int workPin=4;//the anode of the LED connects to digital pin D13 of the arduino

int g,c;
int limit,c_limit;
char buffers[10];
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL,0xF0F0F0F0D2LL };

RF24 radio(9,10);
//nRF24 set the pin 9 to CE and 10 to CSN/SS
// Cables are:
//     SS       -> 10
//     MOSI     -> 11
//     MISO     -> 12
//     SCK      -> 13


void setup() {
Serial.begin(9600);//sets the baud rate
pinMode(DOUTpin, INPUT);//sets the pin as an input to the arduino
pinMode(DOUT2pin, INPUT);//sets the pin as an input to the arduino
pinMode(3, OUTPUT);//sets the pin as an output of the arduino WORK
pinMode(4, OUTPUT);//sets the pin as an output of the arduino ALERT

  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(0x4c);
  // open pipe for writing,mind use diff write pipe
  radio.openWritingPipe(0xF0F0F0F0C3LL);
  radio.openReadingPipe(1,pipes[0]);
  radio.setDataRate(RF24_250KBPS);
  //radio.enableDynamicPayloads();
  radio.setPayloadSize(32);
  radio.setAutoAck(true);
  radio.powerUp();
  Serial.println("...Sending");
}

void loop()
{
 char SendPayload[32]="";
 digitalWrite(3, HIGH);
g = analogRead(AOUTpin);//reads the analaog value from the methane sensor's AOUT pin
c = analogRead(AOUT2pin);//reads the analaog value from the methane sensor's AOUT pin
limit= digitalRead(DOUTpin);//reads the digital value from the methane sensor's DOUT pin
c_limit= digitalRead(DOUT2pin);//reads the digital value from the methane sensor's DOUT pin

strcat(SendPayload, "G");
dtostrf(g,2,0,buffers);
strcat(SendPayload,buffers);
strcat(SendPayload, "G");
strcat(SendPayload, "C");
dtostrf(c,2,0,buffers);
strcat(SendPayload,buffers);
strcat(SendPayload, "C");


   //RADIO send a heartbeat
radio.stopListening();
radio.write(&SendPayload,sizeof(SendPayload));
radio.startListening(); 


if ((limit == LOW)||(c_limit == LOW) ){
digitalWrite(4, HIGH);//if limit has been reached, LED turns on as status indicator

}
else{
digitalWrite(4, LOW);//if threshold not reached, LED remains off
}


delay(1000);
digitalWrite(3, LOW);
delay(4000);
}
