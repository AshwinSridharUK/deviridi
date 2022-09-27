

// © 2022 by Ashwin Sridhar hackster.io/cyborgash122. All rights reserved. Attribution 4.0 International (CC BY 4.0)
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "Seeed_Arduino_GroveAI.h"
#include <Wire.h>
TFT_eSPI tft; //Initializing TFT LCD
TFT_eSprite spr = TFT_eSprite(&tft); //Initializing buffer
#define TFT_OLIVE 0xB674 //Olive Colour
#include "sgp30.h"
#include <SensirionI2CSht4x.h>
int vocPPB ;
SensirionI2CSht4x sht4x;
GroveAI ai(Wire);
uint8_t state = 0;
char *stateFood[] = {"Fresh Apple", "Fresh Banana", "Rotten Apple", "Rotten Banana"}; //List of Different Classes for Image Classification
String foodStatus ;

enum : byte {IDLE, TEMP, GAS, CAMERA, WARNING, DATA} currentState = IDLE;//State Control

void setup() {
  Serial.begin(9600);
  pinMode(WIO_LIGHT, INPUT);
  Wire.begin();
 
    uint16_t error;
    char errorMessage[256];
 
    sht4x.begin(Wire);
 
    uint32_t serialNumber;
    error = sht4x.serialNumber(serialNumber);
    if (error) {
        Serial.print("Error trying to execute serialNumber(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    } else {
        Serial.print("Serial Number: ");
        Serial.println(serialNumber);
    }
  tft.begin();
  tft.setRotation(3);
  spr.createSprite(TFT_HEIGHT, TFT_WIDTH);
  if (ai.begin(ALGO_OBJECT_DETECTION, MODEL_EXT_INDEX_1)) // Object Detection with Model 1 (Change model no. accordingly based on file name)
  {
    Serial.print("Version: ");
    Serial.println(ai.version());
    Serial.print("ID: ");
    Serial.println( ai.id());
    Serial.print("Algo: ");
    Serial.println( ai.algo());
    Serial.print("Model: ");
    Serial.println(ai.model());
    Serial.print("Confidence: ");
    Serial.println(ai.confidence());
    state = 1;

  }
  s16 err;
  u32 ah = 0;
  u16 scaled_ethanol_signal, scaled_h2_signal;
  Serial.begin(115200);
  Serial.println("serial start!!");

  
  while (sgp_probe() != STATUS_OK) {
    Serial.println("SGP failed");
    while (1);
  }
  /*Read H2 and Ethanol signal in the way of blocking*/
  err = sgp_measure_signals_blocking_read(&scaled_ethanol_signal,
                                          &scaled_h2_signal);
  if (err == STATUS_OK) {
    Serial.println("R");
  } else {
    Serial.println("Signal Error");
  }

  
  sgp_set_absolute_humidity(12750);
  err = sgp_iaq_init();
  
}

void loop() {
  switch (currentState) {
    case IDLE: {
        currentState = TEMP;
        break;


      }
     case TEMP: {
        uint16_t error;
    float temperature, humidity;
    int int_temp, int_humi;
    
    error = sht4x.measureHighPrecision(temperature, humidity);
    Serial.print("Temperature: "); Serial.print(temperature);
    Serial.print(" Humidity: "); Serial.println(humidity);
  
    int_temp = temperature*100;
    int_humi = humidity*100;
      
      
      currentState = GAS;
        break;
      
      }
    case GAS: {
        s16 err = 0;
        u16 tvoc_ppb, co2_eq_ppm;
        err = sgp_measure_iaq_blocking_read(&tvoc_ppb, &co2_eq_ppm);
        if (err == STATUS_OK) {
          Serial.print("tVOC  Concentration:");
          Serial.print(tvoc_ppb);
          Serial.println("ppb");
          vocPPB = tvoc_ppb ;

          Serial.print("CO2eq Concentration:");
          Serial.print(co2_eq_ppm);
          Serial.println("ppm");
        } else {
          Serial.println("error reading IAQ values\n");
        }
        delay(1000);
      }
      currentState = CAMERA;
      break;
    case CAMERA: {
        if (state == 1)
        {
          uint32_t tick = millis();
          if (ai.invoke()) 
          {
            while (1) // wait for invoking finished
            {
              CMD_STATE_T ret = ai.state();
              if (ret == CMD_STATE_IDLE)
              {
                break;
              }
              delay(20);
            }

            uint8_t len = ai.get_result_len(); 
            if (len)
            {
              int time1 = millis() - tick;
              Serial.print("Time consuming: ");
              Serial.println(time1);
              Serial.print("Number of people: ");
              Serial.println(len);
              object_detection_t data;       

              for (int i = 0; i < len; i++)
              {
                Serial.println("result:detected");
                Serial.print("Detecting and calculating: ");
                Serial.println(i + 1);
                ai.get_result(i, (uint8_t*)&data, sizeof(object_detection_t)); //get result

                Serial.print("confidence:");
                Serial.print(data.confidence);
                Serial.println();
                
                foodStatus = stateFood[data.target];//Compare Classification to List
         
                Serial.println();
              }
            }
            else
            {
              Serial.println("No identification");
            }
          }
          else
          {
            delay(1000);
            Serial.println("Invoke Failed.");
          }
        }
        else
        {
          state == 0;
        }

        currentState = DATA;
        break;
      }
    case DATA: {
        int lightSensorValue = analogRead(WIO_LIGHT);
        spr.fillSprite(TFT_WHITE);
        spr.fillRect(0, 0, 320, 30, TFT_OLIVE);
        spr.setTextColor(TFT_WHITE);
        spr.setTextSize(3);
        spr.drawString("Food Sensor", 68, 6);
        spr.drawFastVLine(150, 30, 210, TFT_DARKGREEN); 
        spr.drawFastHLine(0, 140, 320, TFT_DARKGREEN); 

        //Freshness from Camera
        spr.setTextColor(TFT_BLACK);
        spr.setTextSize(2);
        spr.drawString(" Freshness ", 12, 45);
        spr.setTextSize(2);
        spr.drawString(foodStatus, 10, 75); 


        spr.setTextSize(2);
        spr.drawString("Humidity", 25, 160);
        spr.setTextSize(3);

       //VOC Data
        spr.setTextSize(2);
        spr.drawString("Gas Concent", 160, 45);
        spr.setTextSize(3);
        spr.drawNumber(vocPPB, 200, 75); //Display humidity values
        spr.drawString("PPB", 240, 75);
        //Light Sensor Data
        spr.setTextSize(2);
        spr.drawString("Light", 200, 160);
        spr.setTextSize(3);
        lightSensorValue = map(lightSensorValue, 0, 1023, 0, 100); //Convert sensor values from ADC to percentage 
        spr.drawNumber(lightSensorValue, 205, 190); 
        spr.drawString("%", 245, 190);
        spr.pushSprite(0, 0); //Update Screen
        delay(50);
        currentState = IDLE;
        break;



      }
  }

}

