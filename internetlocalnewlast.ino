#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>


//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"
#include "DHT.h"

#define DHTPIN 4 
#define DHTTYPE DHT11  
DHT dht(DHTPIN, DHTTYPE);

// Insert your network credentials
#define WIFI_SSID "e78cfe"
#define WIFI_PASSWORD "273974176"

// Insert Firebase project API Key
#define API_KEY "AIzaSyAYHUhONVse4aLsBPpbSN1URs8KxTNdNgQ"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://pryecto1-b58cc-default-rtdb.firebaseio.com/" 

//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

//---------------------------  Temperatura
float h; 
float t; 
float f; 
float hif; 
float hic; 

//---------------------------   ultrasonico
const int trigPin = 5;
const int echoPin = 18;

//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;

//------------------------ Movimiento
const int PIN_TO_SENSOR = 19; // GIOP19 pin connected to OUTPUT pin of sensor
int pinStateCurrent   = LOW;  // current state of pin
int pinStatePrevious  = LOW;  // previous state of pin
bool cond; 

//----------------------- Flama
const int sensor = 13; 
int pinStateCurrent2  = LOW;
bool advise; 


int LEDs[] = {21,23,25,26,27,14,12};    
//------------g  f  e   d  c  b a

//se declaran los arreglos que forman los dígitos
int zero[] = {0, 1, 1, 1, 1, 1, 1};   // cero
int one[] = {0, 0, 0, 0, 1, 1, 0};   // uno
int two[] = {1, 0, 1, 1, 0, 1, 1};   // dos
int three[] = {1, 0, 0, 1, 1, 1, 1};   // tres
int four[] = {1, 1, 0, 0, 1, 1, 0};   // cuatro 
int five[] = {1, 1, 0, 1, 1, 0, 1};   // cinco
int six[] = {1, 1, 1, 1, 1, 0, 1};   // seis
int seven[] = {0, 0, 0, 0, 1, 1, 1};   // siete
int eight[] = {1, 1, 1, 1, 1, 1, 1}; // ocho
int nine[] = {1, 1, 0, 1, 1, 1, 1};   // nueve
int def[] = {1, 0, 0, 0, 0, 0, 0};   // nueve

String numero; 
int num; 


void setup(){

  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(1000);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println(F("DHTxx test!")); //------------ TEMPERATURA
  dht.begin();

  pinMode(sensor,INPUT);

  //---------------------------se inicializan los pines como salida
  for (int i = 0; i<7; i++) pinMode(LEDs[i], OUTPUT);
}

  void temperature() {
    h = dht.readHumidity();
    t = dht.readTemperature();  
    f = dht.readTemperature(true); 

    // Compute heat index in Fahrenheit (the default)
    hif = dht.computeHeatIndex(f, h);
    // Compute heat index in Celsius (isFahreheit = false)
    hic = dht.computeHeatIndex(t, h, false);
  }

  
//----------------------------------- ultrasonico ---------------------------------------------------------
  void ultrasonico(){

    pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
    pinMode(echoPin, INPUT); // Sets the echoPin as an Input}
    digitalWrite(trigPin, LOW); // Clears the trigPin
    delayMicroseconds(2);
  
    digitalWrite(trigPin, HIGH); // Sets the trigPin on HIGH state for 10 micro seconds
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH); // Reads the echoPin, returns the sound wave travel time in microseconds
  
    distanceCm = duration * SOUND_SPEED/2; // Calculate the distance
    distanceInch = distanceCm * CM_TO_INCH; // Convert to inches
  
    Serial.print("Distance (cm): "); // Prints the distance in the Serial Monitor  --- ULTRASONICO
    Serial.println(distanceCm);
    Serial.print("Distance (inch): ");
    Serial.println(distanceInch);
    delay(1000);
  }

//----------------------- MOVIMIENTO -------------------------------------------------

  void motion(){
    pinMode(PIN_TO_SENSOR, INPUT);      // set ESP32 pin to input mode to read value from OUTPUT pin of sensor
    //pinStatePrevious = pinStateCurrent;             // store old state
    pinStateCurrent = digitalRead(PIN_TO_SENSOR);   // read new state

    if(pinStateCurrent == HIGH) {   
        // pin state change: LOW -> HIGH
        //cond = Serial.println(true);
        cond = true; 
        Serial.print("Motion!"); 
        delay(1000); 
        // TODO: turn on alarm, light or activate a device ... here
    }
    else if(pinStateCurrent == LOW){   
        // pin state change: HIGH -> LOW
        //cond = Serial.println(false);
        cond = false; 
        Serial.print("No Motion detected");
        delay(1000);         // TODO: turn off alarm, light or deactivate a device ... here
    }
  }

//----------------------------- FLAMA ----------------------------------------------------------

  void flame(){
    pinStateCurrent2 = digitalRead(sensor);   // read new state

    if (pinStateCurrent2 == HIGH){           
      //advise = Serial.print("FIRE DETECTED!\n"); 
      advise = true; 
      Serial.print("\nFire!"); 
      delay(1000); 
    }
    else if (pinStateCurrent2 == LOW){
      //advise = Serial.println("FIRE NOT DETECTED");
      advise = false;
      Serial.print("\nNo Fire detected"); 
      delay(1000);
    }
  }

  
//función que despliega del 0 al 9
  void segment_display(unsigned char valor){
    switch(valor)
    {
        case 0:
                    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], zero[i]);
                    break;
        case 1:
                    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], one[i]);
                    break;
        case 2:
                    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], two[i]);
                    break;
        case 3:
                    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], three[i]);
                    break;
        case 4:
                    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], four[i]);
                    break;
        case 5:
                    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], five[i]);
                    break;
        case 6:
                    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], six[i]);
                    break;
        case 7:
                    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], seven[i]);
                    break;
        case 8:
                    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], eight[i]);
                    break;
        case 9:
                    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], nine[i]);
                    break;
        default:
                    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], def[i]);
                    break;          
    }
}
  


void loop() {

  temperature(); 
  ultrasonico(); 
  motion(); 
  flame(); 

  //----------------------------- FIREBASE---------------------------------------------------------------------
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    if (Firebase.RTDB.setFloat(&fbdo, "test/humedad", h)){
      Serial.println("\n\nPASSED HUMIDITY");
    }
    else {
      Serial.println("FAILED HUMIDITY");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "test/temperatura C", hic)){
      Serial.println("PASSED TEMP C");
    }
    else {
      Serial.println("FAILED TEMP C");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat(&fbdo, "test/temperatura F", hif)){
      Serial.println("PASSED TEMP F");
    }
    else {
      Serial.println("FAILED TEMP F");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    
//-------------------------ultrasonico
    if (Firebase.RTDB.setFloat(&fbdo, "test/distancia en cm", distanceCm)){
      Serial.println("PASSED DISTANCIA CM");
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat(&fbdo, "test/distancia en inch", distanceInch)){
      Serial.println("PASSED DISTANCIA INCH");
    }
    else {
      Serial.println("FAILED DISTANCE");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    //------------------------------ Movimiento 
    if (Firebase.RTDB.setBool(&fbdo, "test/Movimiento", cond)){
      Serial.println("PASSED MOTION");
    }
    else {
      Serial.println("FAILED MOTION");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    //------------------------------ Flama

    if (Firebase.RTDB.setBool(&fbdo, "test/Flama", advise)){
      Serial.println("PASSED FLAME");
      Serial.print("\n"); 
    }
    else {
      Serial.println("FAILED FLAME");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.getString(&fbdo, "/test/numero")){
      numero = fbdo.stringData();  
      num = numero.toInt(); 
    }  
      segment_display(num);    
  }
}
