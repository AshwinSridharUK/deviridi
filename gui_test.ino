#include <TFT_eSPI.h>

TFT_eSPI tft; //Initializing TFT LCD
TFT_eSprite spr = TFT_eSprite(&tft); //Initializing buffer
#define TFT_OLIVE 0xB674



void setup() {
  Serial.begin(9600);
  pinMode(WIO_LIGHT, INPUT);
  tft.begin();
  tft.setRotation(3); //Set TFT LCD rotation
  spr.createSprite(TFT_HEIGHT,TFT_WIDTH); //Create buffer

}
void loop() {
  int lightSensorValue = analogRead(WIO_LIGHT);
  spr.fillSprite(TFT_WHITE);
  spr.fillRect(0,0,320,30,TFT_OLIVE);
  spr.setTextColor(TFT_WHITE);
  spr.setTextSize(3);
  spr.drawString("Food Sensor",68,6);
   spr.drawFastVLine(150,30,210,TFT_DARKGREEN); //Drawing verticle line
  spr.drawFastHLine(0,140,320,TFT_DARKGREEN); //Drawing horizontal line

  //Setting temperature
  spr.setTextColor(TFT_BLACK);
  spr.setTextSize(2);
  spr.drawString("Temperature",10,45);
  spr.setTextSize(3);

   spr.setTextSize(2);
  spr.drawString("Humidity",25,160);
  spr.setTextSize(3);


   spr.setTextSize(2);
  spr.drawString("Gas Concent",160,45);
  spr.setTextSize(3);

  spr.setTextSize(2);
  spr.drawString("Light",200,160);
  spr.setTextSize(3);
  lightSensorValue = map(lightSensorValue,0,1023,0,100); //Map sensor values 
  spr.drawNumber(lightSensorValue,205,190); //Display sensor values as percentage  
  spr.drawString("%",245,190);
  spr.pushSprite(0,0);
  delay(50);
  
}
  // put your main code here, to run repeatedly:
