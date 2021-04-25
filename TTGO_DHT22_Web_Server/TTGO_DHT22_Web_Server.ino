/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-esp8266-mysql-database-php/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

*/
// https://gpio-controller.herokuapp.com/
// 
#include <DHT.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Replace with your network credentials
const char* ssid            = "Your Wifi SSID";
const char* password        = "Your WiFi Password";

// Data updating route for this DHT sensor to remote ESP32 Dashboard
const char* serverName = "https://gpio-controller.herokuapp.com/data";
//const char* serverName = "http://192.168.43.10:2000/data";

// Declaration of variable to be populated to the remote ESP32 Dashboard
String apiKeyValue = "Family API Key";
String sensorName = "sensorName_DHT22";
String sensorLocation = "Living Room";
int counter = 0;

// Set this ESP32 web server port number to 8000
// Please take note: Port number 80 may not work when using Ngrok Port Forwarding Service
WiFiServer server(8000);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output17State = "off";
String output21State = "off";
String output22State = "off";

// Assign output variables to GPIO pins
const int output17 = 17; // Red LED
const int output21 = 21; // Green LED
const int output22 = 22; // Yellow LED

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

// Define the type of DHT sensor and the GPIO for the DHT sensor
// DHT sensor data output pin connected to GPIO 32
#define DHTPIN 32
// Please select either DHT22 or DHT11 by uncomment it according to the DHT sensor been use
// Uncomment this when DHT22 is use
#define DHTTYPE DHT22
// Uncomment this when DHT11 is use
//#define DHTTYPE DHT11

// Define the GPIO for the TTGO onboard switches
#define BUTTON1PIN 35
#define BUTTON2PIN 0

// Initialize the DHT object with it GPIO and type
DHT dht(DHTPIN, DHTTYPE);
TFT_eSPI tft = TFT_eSPI();
TaskHandle_t Task1;
TaskHandle_t Task2;

// Declaration of variable for DHT sensor and TTGO
float   temp;
float   heatIdx;
float   humidity;
bool    asFahrenheit;
bool    useFahrenheit       = false; // Default to Celsius
bool    showTemp            = true; // Default to Temp / Humidity
long    lastDebounceButton1 = 0;    // Holds Button1 last debounce
long    lastDebounceButton2 = 0;    // Holds Button2 last debounce
long    debounceDelay       = 200;  // 200ms between re-polling

// Declarartion of variable for IP address string to be populated to the ESP32 Dashboard
char* myIpAddress       = "";
String myIpAddressString = "";

const char* deviceName      = "esp32dht";


//////////////////////////////////////////////////////////////////////////
//-FUNCTION-[toggleButton1]----------------------------------------------]
//////////////////////////////////////////////////////////////////////////
void toggleButton1() {  
  if ((millis() - lastDebounceButton1) > debounceDelay) {  
    Serial.print("1-");Serial.println(useFahrenheit);
    if (useFahrenheit){useFahrenheit = false;}
    else {useFahrenheit = true;}
    lastDebounceButton1 = millis();
  }
}
//-----------------------------------------------------------------------]


//////////////////////////////////////////////////////////////////////////
//-FUNCTION-[toggleButton2]----------------------------------------------]
//////////////////////////////////////////////////////////////////////////
void toggleButton2() {
  if ((millis() - lastDebounceButton2) > debounceDelay) {  
    Serial.print("2-");Serial.println(showTemp);
    if (showTemp){showTemp = false;}
    else {showTemp = true;}
    lastDebounceButton2 = millis();
  }
}
//-----------------------------------------------------------------------]


//////////////////////////////////////////////////////////////////////////
//-FUNCTION-[showScrn1]--------------------------------------------------]
//////////////////////////////////////////////////////////////////////////
// What to display if showTemp = true
void showScrn1() {
  float t = dht.readTemperature(useFahrenheit);
  float h = dht.readHumidity();
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  temp =t; humidity=h;
  heatIdx = dht.computeHeatIndex(temp, humidity, useFahrenheit);

// *************************************
//  Serial.print("******** Temperature: " + String(t) + "F Humidity: " + String(h) + "% Heat Index: " + String(heatIdx) + "F *********\n");
// *************************************

  asFahrenheit = useFahrenheit;
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(0, 30);
  tft.setFreeFont(&Orbitron_Light_24);
  tft.println("Temp     Humidity");
  tft.drawLine(0, 35, 250, 35, TFT_BLUE);
  tft.setCursor(0, 60);
  tft.print(temp);
  if (useFahrenheit){tft.print(F("f"));}
  else {tft.print(F("c"));}
  tft.setCursor(130, 60);
  tft.print(humidity);tft.print(F("%"));
}
//-----------------------------------------------------------------------]

//////////////////////////////////////////////////////////////////////////
//-FUNCTION-[showScrn2]--------------------------------------------------]
//////////////////////////////////////////////////////////////////////////
// What to display if showTemp = false
void showScrn2() {  
  float t = dht.readTemperature(useFahrenheit);
  float h = dht.readHumidity();
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  temp =t; humidity=h;
  heatIdx = dht.computeHeatIndex(temp, humidity, useFahrenheit);

// ********************
//  Serial.print("******** Temperature: " + String(t) + "F Humidity: " + String(h) + "% Heat Index: " + String(heatIdx) + "F *********\n");
// *******************
  
  asFahrenheit = useFahrenheit;
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setCursor(50, 30);
  tft.setFreeFont(&Orbitron_Light_24);
  tft.println("Heat Index");
  tft.drawLine(0, 35, 250, 35, TFT_BLUE);
  tft.setFreeFont(&Orbitron_Light_32);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(60, 100);
  tft.print(heatIdx);
  if (useFahrenheit){tft.print(F("f"));}
  else {tft.print(F("c"));}
}
//-----------------------------------------------------------------------]


//////////////////////////////////////////////////////////////////////////
//-FUNCTION-[showSrcnNetwork]--------------------------------------------]
//////////////////////////////////////////////////////////////////////////
// Startup screen with network info
void showSrcnNetwork(){
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setCursor(0, 30);
  tft.setFreeFont(&Orbitron_Light_24);
  tft.println("IP");
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.println(WiFi.localIP());
  tft.setCursor(0, 90);
  tft.setFreeFont(&FreeSerifBold12pt7b);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.print("http://");
  tft.print(deviceName);
  tft.println(".local");
  delay(5000);
}
//-----------------------------------------------------------------------]

void setup() {
  Serial.begin(115200);
  // *********** Initialize Addition UART to interact to another ESP32 or other device which accept RS232 protocol at 9600 baud rate. 
  // You may disable or change the baud rate as require ***************
  Serial2.begin(9600, SERIAL_8N1, 25,26);
  Serial.print("Start!");
  delay(100);
  // ********************************************************
  
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  
  myIpAddressString = WiFi.localIP().toString();

  // Length (with one extra character for the null terminator)
  int str_len = myIpAddressString.length() + 1; 
  
  tft.begin();
  tft.setRotation(1); //Landscape
  showSrcnNetwork();

  pinMode(BUTTON1PIN, INPUT);
  pinMode(BUTTON2PIN, INPUT);
  //////////////dht.begin();
//  xTaskCreatePinnedToCore(
//      Task1code, /* Function to implement the task */
//      "Task1", /* Name of the task */
//      10000,  /* Stack size in words */
//      NULL,  /* Task input parameter */
//      0,  /* Priority of the task */
//      &Task1,  /* Task handle. */
//      0); /* Core where the task should run */
  xTaskCreatePinnedToCore(Task1code,"Task1", 100000, NULL, 2, &Task1, 0);
//   xTaskCreatePinnedToCore(Task2code,"Task2", 10000, NULL, 3, &Task2, 0);
  Serial.println(xPortGetCoreID());
// ***************** ESP32 Server for GPIO Control ***********************
  server.begin();

// -------------------------------------------------

  // Start Configure the Sensor and Other Component
  Serial.println(F("DHTxx test!"));

  // RGB LED
  pinMode(output17, OUTPUT);// RED
  pinMode(output21, OUTPUT);// GREEN
  pinMode(output22, OUTPUT);// YELLOW

// ------------------------------------------------------
// ------------------------------------------------------
  // Perform Self test for all the LEDs
  ledSelfTest();
  // End of LED Self Test
// ------------------------------------------------------

  // Start the DHT11 Sensor
  dht.begin();
// -------------------------------------------------

// **********************************************************************
}

//////////////////////////////////////////////////////////////////////////
//-SYSTEM-[Task1code]----------------------------------------------------]
//////////////////////////////////////////////////////////////////////////
void Task1code( void * pvParameters ){
//  updateDataToESP32DashBoard();
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());
  int button1State = 0;
  int button2State = 0;
    for(;;){
      button1State = digitalRead(BUTTON1PIN);
      button2State = digitalRead(BUTTON2PIN);
      if (button1State == LOW) {
        toggleButton1();
      }
      if (button2State == LOW) {
        toggleButton2();
      }
      delay(10);
    }
}
//-----------------------------------------------------------------------]

//////////////////////////////////////////////////////////////////////////
// Updating Data and Listening to Request main programme in forever loop ----------------------------------------------------]
//////////////////////////////////////////////////////////////////////////

void loop() {

 if (showTemp){showScrn1();} // Temp Humidity
      else {showScrn2();}         // Heat Index
      
 // Updating Data to the ESP32 Dashboard
 updateDataToESP32DashBoard();

 // ESP32 Server listening to Http Request for GPIO
 esp32ServerListenForRequest();
// -------------------------------------------------------
}// End of Forever Loop()

//-----------------------------------------------------------------------]

void ledSelfTest(){
// Perform Self test for all the LEDs
  digitalWrite(17, HIGH); 
  delay(250); 
  digitalWrite(17, LOW); 
  delay(250);

  digitalWrite(21, HIGH); 
  delay(250); 
  digitalWrite(21, LOW); 
  delay(250);

  digitalWrite(22, HIGH); 
  delay(250); 
  digitalWrite(22, LOW); 
  delay(250);
}
// ------------------------------------------------------
//-----------------------------------------------------------------------]

void runningRgbLed(){
  // RGB LED CONTINUE TO ROTATE INDICATING THE APPLICATION IS RUNNING
  // TESTING RGB LED
  digitalWrite(17, HIGH); 
  delay(250); 
  digitalWrite(17, LOW); 
  delay(250);

  digitalWrite(21, HIGH); 
  delay(250); 
  digitalWrite(21, LOW); 
  delay(250);

  digitalWrite(22, HIGH); 
  delay(250); 
  digitalWrite(22, LOW); 
  delay(250);
}
//-----------------------------------------------------------------------]

void updateDataToESP32DashBoard(){
if(WiFi.status()== WL_CONNECTED){
    // ****************** TTGO ******************
    delay(500);  // Updating interval
    
    HTTPClient http;
    // Your Domain name with URL path or IP address with path
    // "serverName" is the IP address or URL for your ESP32 Dashboard
    http.begin(serverName);
    
    // Specify content-type header
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    
    // Prepare your HTTP POST request data string
    String httpRequestData = "api_key=" + myIpAddressString + "&sensorName=" + sensorName + "&sensorLocation=" + sensorLocation + "&counter=" + String(counter) + "&temperature=" + String(temp) + "&humidity=" + String(humidity) + "&heatIndex=" + String(heatIdx) + "";

    Serial.print("httpRequestData: ");
    Serial.println(httpRequestData);

    counter++;

    // Send HTTP POST request
    int httpResponseCode = http.POST(httpRequestData);
        
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
  }
}

//-----------------------------------------------------------------------]

void esp32ServerListenForRequest(){
  // **************** Code Added on 190221 for This Web Server to Control GPIO and Ukit Communication ********************
  WiFiClient client = server.available();   // Listen for incoming clients
  // *******************************************************
  if (client) {                             // If a new client connects,
   currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();           
// ------------------------------------------------------------------------------
    // You may change or add additional Http Request route according to your requirement
    // Currently there are Http Request route from GPIO 0 to 5, GPIO 10 to 15, GPIO 20 to 25 and GPIO 30 to 35 for testing purpose only
    // The Http Request and Response can be track from the serial monitor
    if (header.indexOf("GET /0/on") >= 0) {
              Serial.println("GPIO 0 on");
              output17State = "on";
              digitalWrite(output17, HIGH);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 0 on");
            } else if (header.indexOf("GET /0/off") >= 0) {
              Serial.println("GPIO 0 off");
              output17State = "off";
              digitalWrite(output17, LOW);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 0 off");
            } else if (header.indexOf("GET /1/on") >= 0) {
              Serial.println("GPIO 1 on");
              output21State = "on";
              digitalWrite(output21, HIGH);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 1 on");
            } else if (header.indexOf("GET /1/off") >= 0) {
              Serial.println("GPIO 1 off");
              output21State = "off";
              digitalWrite(output21, LOW);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 1 off");
            } else if (header.indexOf("GET /2/on") >= 0) {
              Serial.println("GPIO 2 on");
              output22State = "on";
              digitalWrite(output22, HIGH);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 2 on");
            } else if (header.indexOf("GET /2/off") >= 0) {
              Serial.println("GPIO 2 off");
              output22State = "off";
              digitalWrite(output22, LOW);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 2 off");
            }   if (header.indexOf("GET /3/on") >= 0) {
              Serial.println("GPIO 3 on");
              output17State = "on";
              digitalWrite(output17, HIGH);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 3 on");
            } else if (header.indexOf("GET /3/off") >= 0) {
              Serial.println("GPIO 3 off");
              output17State = "off";
              digitalWrite(output17, LOW);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 3 off");
            } else if (header.indexOf("GET /4/on") >= 0) {
              Serial.println("GPIO 4 on");
              output21State = "on";
              digitalWrite(output21, HIGH);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 4 on");
            } else if (header.indexOf("GET /4/off") >= 0) {
              Serial.println("GPIO 4 off");
              output21State = "off";
              digitalWrite(output21, LOW);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 4 off");
            } else if (header.indexOf("GET /5/on") >= 0) {
              Serial.println("GPIO 5 on");
              output22State = "on";
              digitalWrite(output22, HIGH);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 5 on");
            } else if (header.indexOf("GET /5/off") >= 0) {
              Serial.println("GPIO 5 off");
              output22State = "off";
              digitalWrite(output22, LOW);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 5 off");
            } else if (header.indexOf("GET /10/on") >= 0) {
              Serial.println("GPIO 10 on");
              output17State = "on";
              digitalWrite(output17, HIGH);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 10 on");
            } else if (header.indexOf("GET /10/off") >= 0) {
              Serial.println("GPIO 10 off");
              output17State = "off";
              digitalWrite(output17, LOW);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 10 off");
            } else if (header.indexOf("GET /11/on") >= 0) {
              Serial.println("GPIO 11 on");
              output21State = "on";
              digitalWrite(output21, HIGH);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 11 on");
            } else if (header.indexOf("GET /11/off") >= 0) {
              Serial.println("GPIO 11 off");
              output21State = "off";
              digitalWrite(output21, LOW);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 11 off");
            } else if (header.indexOf("GET /12/on") >= 0) {
              Serial.println("GPIO 12 on");
              output22State = "on";
              digitalWrite(output22, HIGH);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 12 on");
            } else if (header.indexOf("GET /12/off") >= 0) {
              Serial.println("GPIO 12 off");
              output22State = "off";
              digitalWrite(output22, LOW);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 12 off");
            }   if (header.indexOf("GET /13/on") >= 0) {
              Serial.println("GPIO 13 on");
              output17State = "on";
              digitalWrite(output17, HIGH);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 13 on");
            } else if (header.indexOf("GET /13/off") >= 0) {
              Serial.println("GPIO 13 off");
              output17State = "off";
              digitalWrite(output17, LOW);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 13 off");
            } else if (header.indexOf("GET /14/on") >= 0) {
              Serial.println("GPIO 14 on");
              output21State = "on";
              digitalWrite(output21, HIGH);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 14 on");
            } else if (header.indexOf("GET /14/off") >= 0) {
              Serial.println("GPIO 14 off");
              output21State = "off";
              digitalWrite(output21, LOW);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 14 off");
            } else if (header.indexOf("GET /15/on") >= 0) {
              Serial.println("GPIO 15 on");
              output22State = "on";
              digitalWrite(output22, HIGH);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 15 on");
            } else if (header.indexOf("GET /15/off") >= 0) {
              Serial.println("GPIO 15 off");
              output22State = "off";
              digitalWrite(output22, LOW);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 15 off");
            } else if (header.indexOf("GET /20/on") >= 0) {
              Serial.println("GPIO 20 on");
              output17State = "on";
              digitalWrite(output17, HIGH);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 20 on");
            } else if (header.indexOf("GET /20/off") >= 0) {
              Serial.println("GPIO 20 off");
              output17State = "off";
              digitalWrite(output17, LOW);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 20 off");
            } else if (header.indexOf("GET /21/on") >= 0) {
              Serial.println("GPIO 21 on");
              output21State = "on";
              digitalWrite(output21, HIGH);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 21 on");
            } else if (header.indexOf("GET /21/off") >= 0) {
              Serial.println("GPIO 21 off");
              output21State = "off";
              digitalWrite(output21, LOW);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 21 off");
            } else if (header.indexOf("GET /22/on") >= 0) {
              Serial.println("GPIO 22 on");
              output22State = "on";
              digitalWrite(output22, HIGH);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 22 on");
            } else if (header.indexOf("GET /22/off") >= 0) {
              Serial.println("GPIO 22 off");
              output22State = "off";
              digitalWrite(output22, LOW);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 22 off");
            }   if (header.indexOf("GET /23/on") >= 0) {
              Serial.println("GPIO 23 on");
              output17State = "on";
              digitalWrite(output17, HIGH);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 23 on");
            } else if (header.indexOf("GET /23/off") >= 0) {
              Serial.println("GPIO 23 off");
              output17State = "off";
              digitalWrite(output17, LOW);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 23 off");
            } else if (header.indexOf("GET /24/on") >= 0) {
              Serial.println("GPIO 24 on");
              output21State = "on";
              digitalWrite(output21, HIGH);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 24 on");
            } else if (header.indexOf("GET /24/off") >= 0) {
              Serial.println("GPIO 24 off");
              output21State = "off";
              digitalWrite(output21, LOW);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 24 off");
            } else if (header.indexOf("GET /25/on") >= 0) {
              Serial.println("GPIO 25 on");
              output22State = "on";
              digitalWrite(output22, HIGH);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 25 on");
            } else if (header.indexOf("GET /25/off") >= 0) {
              Serial.println("GPIO 25 off");
              output22State = "off";
              digitalWrite(output22, LOW);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 25 off");
            } else if (header.indexOf("GET /30/on") >= 0) {
              Serial.println("GPIO 30 on");
              output17State = "on";
              digitalWrite(output17, HIGH);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 30 on");
            } else if (header.indexOf("GET /30/off") >= 0) {
              Serial.println("GPIO 30 off");
              output17State = "off";
              digitalWrite(output17, LOW);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 30 off");
            } else if (header.indexOf("GET /31/on") >= 0) {
              Serial.println("GPIO 31 on");
              output21State = "on";
              digitalWrite(output21, HIGH);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 31 on");
            } else if (header.indexOf("GET /31/off") >= 0) {
              Serial.println("GPIO 31 off");
              output21State = "off";
              digitalWrite(output21, LOW);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 31 off");
            } else if (header.indexOf("GET /32/on") >= 0) {
              Serial.println("GPIO 32 on");
              output22State = "on";
              digitalWrite(output22, HIGH);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 32 on");
            } else if (header.indexOf("GET /32/off") >= 0) {
              Serial.println("GPIO 32 off");
              output22State = "off";
              digitalWrite(output22, LOW);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 32 off");
            }   if (header.indexOf("GET /33/on") >= 0) {
              Serial.println("GPIO 33 on");
              output17State = "on";
              digitalWrite(output17, HIGH);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 33 on");
            } else if (header.indexOf("GET /33/off") >= 0) {
              Serial.println("GPIO 33 off");
              output17State = "off";
              digitalWrite(output17, LOW);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 33 off");
            } else if (header.indexOf("GET /34/on") >= 0) {
              Serial.println("GPIO 34 on");
              output21State = "on";
              digitalWrite(output21, HIGH);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 34 on");
            } else if (header.indexOf("GET /34/off") >= 0) {
              Serial.println("GPIO 34 off");
              output21State = "off";
              digitalWrite(output21, LOW);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 34 off");
            } else if (header.indexOf("GET /35/on") >= 0) {
              Serial.println("GPIO 35 on");
              output22State = "on";
              digitalWrite(output22, HIGH);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 35 on");
            } else if (header.indexOf("GET /35/off") >= 0) {
              Serial.println("GPIO 35 off");
              output22State = "off";
              digitalWrite(output22, LOW);
              // *********** Additional UART for interacting to other device ***************
              Serial2.write("GPIO 35 off");
            }
// ------------------------------------------------------------------------------         
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
