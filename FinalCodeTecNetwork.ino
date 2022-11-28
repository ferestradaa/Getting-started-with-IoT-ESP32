//Reto: Implementacion de Internet de las Cosas
//Fernando Estrada Silva - A01736094
//Adrián Moras Acuña - A01552359
//Ángel Estrada Centeno - A01732584

#include <esp_wpa2.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>  
#include <addons/TokenHelper.h>
#include "Arduino.h"
#include "addons/RTDBHelper.h"
#include "DHT.h"

//definicion de sensor DHT11
#define DHTPIN 4
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

//Variabes a utilizar
//------Sensor de Temperatura
float h;
float t;
float f;
float hif; 
float hic; 

//-------Sensor de Distancia (ultrasonico)
const int trigPin = 5; //pines a utilizar (5 y 18)
const int echoPin = 18;

//declarar velocidad del sonido en cm/uS 
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;

//-------Sensor de movimiento
const int PIN_TO_SENSOR = 19; // GIOP19 pin conectado al OUTPUT pin del sensor
int pinStateCurrent   = LOW;  // estado actual del pin
int pinStatePrevious  = LOW;  // estado previo del pin
bool cond; //variable que indica la alerta de movimiento

//-------Sensor de flama
const int sensor = 13; // Pin a utlizar para el sensor
int pinStateCurrent2  = LOW;
bool advise; //variable que indica si hay fuego o no

//-------Display de 7 segmentos
int LEDs[] = {21,23,25,26,27,14,12}; //arreglo para declarar los pines a usar del display
//------------g  f  e   d  c  b a    // representacion de cada pin en tal orden

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

String numero; //variable para leer en firebase
int num; // varaible para almacenar el numero que de desplegara en el display


//Verificacion de conexion a red inalambrica Wi-fi protegida
const char* ssid = "Tec";
#define EAP_IDENTITY "A01736094@tec.mx"
#define EAP_PASSWORD "Arkham:)213"

//Auntenticadores para firebase
#define USER_EMAIL "A01736094@tec.mx"
#define USER_PASSWORD "12345678"

// API Key del proyecto en Firebase
#define API_KEY "AIzaSyAYHUhONVse4aLsBPpbSN1URs8KxTNdNgQ"//AIzaSyAjjTHMIV0y394tayvijhU-aVVcKdkIZxU

// URL de la base de datos en firebase
#define DATABASE_URL "https://pryecto1-b58cc-default-rtdb.firebaseio.com/"

//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int intValue;
float floatValue;
bool signupOK = false; //validacion de sign up en firebase

//----------------------------------------- SETUP!
void setup() {

  //Conexion a wifi 
  Serial.begin(115200); //Abertura un Puerto serie y especifica la velocidad de transmisión.
  dht.begin();
  delay(10);

  Serial.println();
  Serial.print("Connecting to "); //comienza la validacion para conectarse a la red de internet
  Serial.println(ssid); //ssid (nombramiento) de la red 

  // WPA2 enterprise magic starts here
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);   //init wifi mode

  //esp_wifi_set_mac(ESP_IF_WIFI_STA, &masterCustomMac[0]);
  Serial.print("MAC >> ");
  Serial.println(WiFi.macAddress());
  Serial.printf("Connecting to WiFi: %s ", ssid);

  //esp_wifi_sta_wpa2_ent_set_ca_cert((uint8_t *)incommon_ca, strlen(incommon_ca) + 1);
  esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));

  //esp_wpa2_config_t configW = WPA2_CONFIG_INIT_DEFAULT();
  //esp_wifi_sta_wpa2_ent_enable(&configW);
  esp_wifi_sta_wpa2_ent_enable();
  // WPA2 enterprise magic ends here
  WiFi.begin(ssid);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
///* Assign the api key (required) */
  config.api_key = API_KEY;
  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;
  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok!");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  //pinMode(sensor,INPUT);

  
  //se inicializan los pines como salida
  for (int i = 0; i<7; i++) pinMode(LEDs[i], OUTPUT);
}

  int value = 0;

  //funcion de sensor de temperatura
  void temperatura(){
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    h = dht.readHumidity(); // leer temperatura en Celsius (the default)
    t = dht.readTemperature(); // Read temperature as Fahrenheit (isFahrenheit = true)
    f = dht.readTemperature(true);

    // Compute heat index in Fahrenheit (the default)
    hif = dht.computeHeatIndex(f, h);
    // Compute heat index in Celsius (isFahreheit = false)
    hic = dht.computeHeatIndex(t, h, false);
  }

  //funcion de sensor de distancia(ultrasonico)
  void distancia(){
    pinMode(trigPin, OUTPUT); // se establece el trigPin como Output
    pinMode(echoPin, INPUT); // se establece el echoPin como Input}

    // Clears the trigPin
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
  
    // leyendo el echoPin, regresa el tiempo de camino de la onda de sonido
    duration = pulseIn(echoPin, HIGH);
  
    // calcula la disctancia con la duracion y la velocidad del sonido 
    distanceCm = duration * SOUND_SPEED/2;
  
    // Conversion a pulgadas
    distanceInch = distanceCm * CM_TO_INCH;
  
    //Se imprime el valor de la distancia en el serial monitor
    Serial.print("Distance (cm): ");
    Serial.println(distanceCm);
    Serial.print("Distance (inch): ");
    Serial.println(distanceInch);
    delay(1000);
  }

//Funcion de sensor de movimiento
  void movimiento(){
    pinMode(PIN_TO_SENSOR, INPUT); // se establece un pin a input para leer el valor de output de un pin del sensor
    //pinStatePrevious = pinStateCurrent;             // store old state
    pinStateCurrent = digitalRead(PIN_TO_SENSOR);   // se lee el estado actual

    if(pinStateCurrent == HIGH) {   //si el estado actual del pin es HIGH
        cond = true; //variable booleana que valida el estado del pin
        Serial.print("Motion!"); //se imprime movimiento detectado
        delay(1000); 
    }
    else if(pinStateCurrent == LOW){ //caso contrario
        // pin state change: HIGH -> LOW
        cond = false; 
        Serial.print("No Motion detected");
        delay(1000);  
    }
  }

//Funcion de sensor de flama
  void flama(){
    pinMode(sensor,INPUT); //declarar pin de entrada
    pinStateCurrent2 = digitalRead(sensor);   //lectura de estado actual

    if (pinStateCurrent2 == HIGH) { //si el estado actual del pin es HIGH          
     //advise = Serial.print("FIRE DETECTED!\n"); 
      advise = true; //variable booleana para validad la activacion del sensor
      Serial.print("\nFire!\n"); //se imprime fuego detectado
      delay(1000); 
   }
    else if (pinStateCurrent2 == LOW){ //caso contrario
      //advise = Serial.println("FIRE NOT DETECTED");
      advise = false;
      Serial.print("\nNo Fire detected\n"); 
      delay(1000);
    }
  }

    
  //función que despliega del 0 al 9
  void segment_display(unsigned char valor){
    switch(valor) //indicar HIGH O LOW para cada pin a encender dependiendo del numero leido. 
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

//----------------------------------------- LOOP! 

void loop() {

  temperatura(); //llamado de funciones de los sensores
  distancia(); 
  movimiento(); 
  flama(); 

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
  sendDataPrevMillis = millis(); //en caso de que el sign in sea exitoso y la conexion a internet, se mandan los datos a firebase

  //--------------------------------temperatura 
  if (Firebase.RTDB.setFloat(&fbdo, "Informacion/Temperatura C", hic)){ //hic se manda a firebase
    Serial.println("\n\nPASSED TEMP C: ");
  }
  else {
    Serial.println("FAILED TEMP C"); //si no es posible, se imprime el error de por que
    Serial.println("REASON: " + fbdo.errorReason());
  }
  if (Firebase.RTDB.setFloat(&fbdo, "Informacion/Temperatura F", hif)){ //hif se manda a firebase
    Serial.println("PASSED TEMP F");
  }
  else {
    Serial.println("FAILED TEMP F");
    Serial.println("REASON: " + fbdo.errorReason()); 
  }
  if (Firebase.RTDB.setFloat(&fbdo, "Informacion/Humedad", h)){ //h se manda a firebase
    Serial.println("PASSED HUM");
  }
  else {
    Serial.println("FAILED TEMP HUM");
    Serial.println("REASON: " + fbdo.errorReason());
  }

//-------------------------Distancia
  if (Firebase.RTDB.setFloat(&fbdo, "Informacion/Distancia en cm", distanceCm)){ //distanceCM se manda a firebase
    Serial.println("PASSED DISTANCE CM");
  }
  else {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }

  if (Firebase.RTDB.setFloat(&fbdo, "Informacion/Distancia en inch", distanceInch)){ //distanceInch se manda a firebase
    Serial.println("PASSED DISTANCE INCH");

  }
  else {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }

//------------------Movimiento 
  if (Firebase.RTDB.setBool(&fbdo, "Informacion/Movimiento", cond)){ //cond se manda a firebase
    Serial.println("PASSED MOTION");
   }
  else {
    Serial.println("FAILED MOTION");
    Serial.println("REASON: " + fbdo.errorReason());
  }

//------------------sensor de flama

  if (Firebase.RTDB.setBool(&fbdo, "Informacion/Flama", advise)){ //advise se manda a firebase
    Serial.println("PASSED FLAME\n");
   }
  else {
    Serial.println("FAILED FLAME");
    Serial.println("REASON: " + fbdo.errorReason());
   }

  if (Firebase.RTDB.getString(&fbdo, "/test/numero")){ //se lee un numero de la base de datos con getString
    numero = fbdo.stringData(); //se lee de la varaible numerol, la cual se manda con la aplicacion en forma de string
    num = numero.toInt(); //se convierte el valor a entero
  }  
  segment_display(num);//se manda a llamar a la funcion de display con el nuemero leido
  }
}
