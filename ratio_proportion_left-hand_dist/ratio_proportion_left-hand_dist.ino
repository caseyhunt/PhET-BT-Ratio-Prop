/*This example demonstrates how to create a service with one characteristic 
 * that contains a float value (4-bytes), and which sends a notification every second. 
 */
#include <bluefruit.h>

#include <Wire.h>
#include "SparkFun_VL53L1X.h"


#define SHUTDOWN_PIN 2
#define INTERRUPT_PIN 3

SFEVL53L1X distanceSensor;
//Uncomment the following line to use the optional shutdown and interrupt pins.
//SFEVL53L1X distanceSensor(Wire, SHUTDOWN_PIN, INTERRUPT_PIN);

float value = 0.0; 
int lastTime = millis();
int leftHandVal;

//All included services are in: C:\Users\Ali\AppData\Local\Arduino15\packages\adafruit\hardware\nrf52\0.14.0\libraries\Bluefruit52Lib\src\services
BLEService floatNumberService;
BLECharacteristic floatNumberCharacteristic;

void setupServicesAndCharacteristics(){
  floatNumberService = BLEService(0xAE6F);
  floatNumberService.begin();

  floatNumberCharacteristic = BLECharacteristic(0x2947);
  floatNumberCharacteristic.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY); 
  floatNumberCharacteristic.setFixedLen(4);
  floatNumberCharacteristic.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
    //For the permission, the first parameter is the READ, second the WRITE permission
  floatNumberCharacteristic.begin();
}

void setup(){ 
    Wire.begin();
    
  Bluefruit.autoConnLed(true);   // Setup the BLE LED to be enabled on CONNECT
  //All config***() function must be called before begin()
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX); // Config the peripheral connection with maximum bandwidth 
  Bluefruit.begin();
  Bluefruit.setTxPower(0); // Set max power. Accepted values: -40, -30, -20, -16, -12, -8, -4, 0, 4
  Bluefruit.setName("nrf52R");
  Bluefruit.Periph.setConnectCallback(connect_callback);
  setupServicesAndCharacteristics(); 
  startAdv();   // Set up and start advertising

  Serial.begin(115200);
    if (distanceSensor.begin() != 0) //Begin returns 0 on a good init
  {
    Serial.println("Sensor failed to begin. Please check wiring. Freezing...");
    while (1)
      ;
  }
    Serial.println("Sensor online!");
}

void loop(){ 


    distanceSensor.startRanging(); //Write configuration bytes to initiate measurement
  while (!distanceSensor.checkForDataReady())
  {
    delay(1);
  }
  if(distanceSensor.getDistance() < 610){
  int distance = distanceSensor.getDistance(); //Get the result of the measurement from the sensor
  leftHandVal = map(distance, 15, 610, 0, 101);

  distanceSensor.clearInterrupt();
  distanceSensor.stopRanging();
  Serial.print("Distance(mm): ");
  Serial.print(leftHandVal);
  Serial.println();
  }

  
  if(millis() - lastTime > 500){ //we wiil check the battery only once per 5 seconds. This reduces power consumption greatly.
    value = leftHandVal;
    
    //value = 255.0f;
    lastTime = millis();
    floatNumberCharacteristic.notify(&value,sizeof(value));
  } 
  waitForEvent();  // Request CPU to enter low-power mode until an event/interrupt occurs
}

void startAdv(void){
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);   // Advertising packet
  Bluefruit.Advertising.addTxPower();
  Bluefruit.ScanResponse.addName();   // Secondary Scan Response packet (optional)
  //It is vital that we ADVERTISE the services on the device, if someone is searching
  //for that device based on the services it supports. If not advertised, there is no
  //way for a central to know what services are supported before connecting.
  Bluefruit.Advertising.addService(floatNumberService);   // Include battService uuid  
  Bluefruit.Advertising.restartOnDisconnect(true); //Restart advertising on disconnect.
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

void connect_callback(uint16_t conn_handle){ // callback invoked when central connects
  //floatNumberCharacteristic.notify32(33.0f);
}
