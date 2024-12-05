#include "config.h"

//DHT11 SESNOR and COOLING FAN
#include<ESP32Servo.h>
#include "DHT.h"  
#define temperatureCelsius
#define DHTTYPE DHT11 
const int DHTPin = 21;
const int fanPin = 2;
DHT dht(DHTPin, DHTTYPE);
Servo myServo;

// set up the 'temperature' and 'humidity' feeds
AdafruitIO_Feed *temperature = io.feed("temperature");
AdafruitIO_Feed *humi = io.feed("humidity");

AdafruitIO_Feed *t = io.feed("tempControl");
AdafruitIO_Feed *h = io.feed("humiControl");

//LDR SENSOR
const int ledPin = 18;  
const int ldrPin = A0;
int sensorValue;
int threshold_value = 500; 

AdafruitIO_Feed *intensity = io.feed("lightIntensity");
AdafruitIO_Feed *ldr = io.feed("ldrControl");

//SOIL MOISTURE SENSOR
const int Moist = 34;
int out_moist=0;

AdafruitIO_Feed *moisture = io.feed("soilmoisture");


//DS18B20 SENSOR
#include <OneWire.h>
#include <DallasTemperature.h>

// GPIO where the DS18B20 is connected to
const int oneWireBus = 4;     
  
// OneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Passing oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);


AdafruitIO_Feed *soiltemp = io.feed("soiltemperature");

int threshold_temp=0, threshold_humi=0, threshold_ldr=0;

void setup() {

  // start the serial connection
  Serial.begin(115200);

  //DHT11 SENSOR AND COOLING FAN
  dht.begin();
  myServo.attach(fanPin);

  //LDR SENSOR AND SOIL MOISTURE
  analogReadResolution(10);
  pinMode(ldrPin, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);

   //DS18B20
   //Start the DS18B20
  sensors.begin();
  
  // wait for serial monitor to open
  while(! Serial);
  WiFi.begin( WIFI_SSID ,WIFI_PASS);
  while(WiFi.status()!= WL_CONNECTED)
  {
    Serial.print("y");
    delay(500);
  }
  
  Serial.print("Connecting to Adafruit IO");
  io.connect();

  t->onMessage(CallBackOnTemperature);
  h->onMessage(CallBackOnHumidity);
  ldr->onMessage(CallBackOnIntensity);

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());

  t->get();
  h->get();
  ldr->get();

}

void loop() {

  io.run();

  Serial.println(String(threshold_temp)+ " " + String(threshold_humi)+" " + String(threshold_ldr));

  //DHT11 SENSOR AND COOLING FAN
  float tempC = dht.readTemperature();
  float tempF = dht.readTemperature(true);
  float humidity = dht.readHumidity();
  
  if (isnan(humidity) || isnan(tempC) ||isnan(tempF)) {
  Serial.println("Failed to read from DHT sensor! Check connections");
  return;
  }

  temperature->save(tempC);  
  humi->save(humidity);  

  if(tempC>threshold_temp || humidity>threshold_humi){
    while(tempC<=threshold_temp || humidity<=threshold_humi){
      myServo.write(180);
      delay(1000);
      myServo.write(0);
      delay(1000);
    }
  }
  
  Serial.print("Humidity: ");
  Serial.println(humidity);
  Serial.print(" Temperature: ");
  Serial.print(tempC);
  Serial.println("°C ");


  //LDR SENSOR 
  sensorValue = analogRead(ldrPin); // read analog input pin 0
  sensorValue = 1024-sensorValue;
  Serial.println(sensorValue); // prints the value read
  

  intensity->save(sensorValue);   

  if(sensorValue>threshold_ldr)
  {
    digitalWrite(ledPin, LOW);
  }
  else
  {
    digitalWrite(ledPin, HIGH);
  }


  //SOIL MOISTURE SENSOR 
  out_moist= analogRead(Moist);
  out_moist= map(out_moist,0,1023,0,100);
  Serial.print("SOIL MOISTURE:");
  Serial.print(out_moist);
  Serial.println("%");
  
  moisture->save(out_moist);  


  //DS18B20
  sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);
  float temperatureF = sensors.getTempFByIndex(0);
  Serial.print(temperatureC);
  Serial.println("ºC");
  Serial.print(temperatureF);
  Serial.println("ºF");

  soiltemp->save(temperatureC); 

  delay(20000);
}


// convert the data to integer
void CallBackOnTemperature(AdafruitIO_Data *data)
{
    threshold_temp = data->toInt();
}
void CallBackOnHumidity(AdafruitIO_Data *data)
{
    threshold_humi = data->toInt();
}
void CallBackOnIntensity(AdafruitIO_Data *data)
{
    threshold_ldr = data->toInt();
}
