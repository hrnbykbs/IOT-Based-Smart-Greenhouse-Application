#include <ESP8266WiFi.h>
#include <Servo.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>

DHT dht(2, DHT11); // GPIO2 = D4
Adafruit_BMP085 bmp;
Servo motor;
BlynkTimer timer;

char auth[] = "Blynk_Auth_Token";
const char* ssid = "Wifi_Name";
const char* password = "Wifi_Password";
WiFiServer server(80);

int ledPin = 13; // GPIO13 = D7
int waterPump= 15;
int fan = 0;
int door = 0;
int fanEvent;
int waterEvent;
int soilMoisturePin = A0;

void sendSensor()
{
  float humidity= dht.readHumidity();
  float temperature = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  int soilMoistureValue = analogRead(soilMoisturePin);
  Blynk.virtualWrite(V5, humidity);
  Blynk.virtualWrite(V6, temperature);
  Blynk.virtualWrite(V7, soilMoistureValue);
}
void setup() {
  dht.begin();
  bmp.begin();
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(1000L, sendSensor);
  Serial.begin(115200);
  delay(10);
  motor.attach(14); // GPIO14 = D5
  pinMode(ledPin, OUTPUT);
  pinMode(soilMoisturePin, INPUT);
  pinMode(waterPump, OUTPUT);
  pinMode(fan, OUTPUT);
  digitalWrite(ledPin, LOW);
  digitalWrite(fan, LOW);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  server.begin();
  Serial.println("Server started");

  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
}

void loop() {
  Blynk.run();
  timer.run();
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  //float bmpTemperature = bmp.readTemperature();
  int pressure = bmp.readPressure();
  int soilMoistureValue = analogRead(soilMoisturePin);
  Serial.print("Temperature : ");
  Serial.println(temperature);
  Serial.print("Nem : ");
  Serial.println(humidity);
  Serial.print("Pressure : ");
  Serial.println(pressure);
  Serial.print("Soil Moisture Value : ");
  Serial.println(soilMoistureValue);
  Serial.println("");
  delay(3000);
  if (soilMoistureValue > 400) {
    digitalWrite(15, LOW);
    delay(5000);
    waterEvent = 1;
  }
  else if (soilMoistureValue <= 400) {
    digitalWrite(15, HIGH);
    delay(2000);
    waterEvent = 0;
  }
  if (temperature >= 23 && humidity >= 70) {
    digitalWrite(fan, HIGH);
    delay(10000);
    fanEvent = 1;
  }
  else {
    digitalWrite(fan, LOW);
    delay(5000);
    fanEvent = 0;
  }
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // we are waiting for the client to send data
  Serial.println("new client");
  while (!client.available()) {
    delay(1);
  }

  // we read incoming requests
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  int value = LOW;
  if (request.indexOf("/LED-ON") != -1) {
    digitalWrite(ledPin, HIGH);
    value = HIGH;
  }
  if (request.indexOf("/LED-OFF") != -1) {
    digitalWrite(ledPin, LOW);
    value = LOW;
  }
  int pos;
  if (request.indexOf("/DOOR-ON") != -1)  {
    motor.write(90);
    door = 1;
  }
  if (request.indexOf("/DOOR-OFF") != -1)  {

    motor.write(0);
    delay(15);
    door = 0;
  }

  if (request.indexOf("/FAN-ON") != -1)  {
    digitalWrite(fan, HIGH);
    delay(2000);
    fanEvent = 1;
  }
  if (request.indexOf("/FAN-OFF") != -1)  {
    digitalWrite(fan, LOW);
    delay(2000);
    fanEvent = 0;
  }

  if (request.indexOf("/WATER-ON") != -1)  {
    digitalWrite(waterPump, LOW);
    waterEvent = 1;
  }
  if (request.indexOf("/WATER-OFF") != -1)  {
    digitalWrite(waterPump, HIGH);
    waterEvent = 0;
  }

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("");
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");

  client.print("LED NOW: ");

  if (value == HIGH) {
    client.print("ON   ");
  } else {
    client.print("OFF   ");
  }
  client.println("<br></br>");
  client.println("<a href=\"/LED-ON\"\"><button>OPEN THE LED </button></a>");
  client.println("<a href=\"/LED-OFF\"\"><button>TURN OFF THE LED </button></a>");
  client.println("<br></br>");

  client.print("DOOR NOW: ");

  if (door == 1) {
    client.print("ON   ");
  } else {
    client.print("OFF   ");
  }
  client.println("<br></br>");
  client.println("<a href=\"/DOOR-ON\"\"><button>OPEN THE DOOR </button></a>");
  client.println("<a href=\"/DOOR-OFF\"\"><button>CLOSE THE DOOR </button></a>");
  client.println("<br></br>");

  client.print("FAN NOW: ");

  if (fanEvent == 1) {
    client.print("ON   ");
  } else {
    client.print("OFF   ");
  }
  client.println("<br></br>");
  client.println("<a href=\"/FAN-ON\"\"><button>OPEN THE FAN </button></a>");
  client.println("<a href=\"/FAN-OFF\"\"><button>CLOSE FAN </button></a>");
  client.println("<br></br>");

  client.print("WATER NOW: ");

  if (waterEvent == 1) {
    client.print("ON   ");
  } else {
    client.print("OFF   ");
  }
  client.println("<br></br>");
  client.println("<a href=\"/WATER-ON\"\"><button>TURN WATER </button></a>");
  client.println("<a href=\"/WATER-OFF\"\"><button>SHUT THE WATER </button></a>");
  client.println("<br></br>");

  client.println("Temperature: ");
  client.println(temperature);
  client.println(" (*C)");
  client.println("<br></br>");
  client.println("Humidity: ");
  client.println(humidity);
  client.println(" (%)");
  client.println("<br></br>");
  client.println("Pressure: ");
  client.println(pressure);
  client.println(" (Pascal)");
  client.println("<br></br>");
  client.println("Soil Moisture: ");
  client.println(soilMoistureValue);
  client.println("<br></br>");

  client.println("</html>");
  delay(1000);
  Serial.println("Client disconnected");
  Serial.println("");

}
