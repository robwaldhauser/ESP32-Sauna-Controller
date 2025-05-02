#include <dummy.h>

#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <NTPClient.h>               
#include <TimeLib.h>     

const int relay = 26;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -14400, 60000);
char DateTime[ ] = " 00/00 00:00 XM ";
byte last_second, second_, minute_, hour_, day_, month_;
int year_;

const char* ssid = "ssid";
const char* password = "password";

const char* mqtt_server = "192.168.1.?";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

// LED Pin
const int ledPin = 4;

// GPIO where the DS18B20 is connected to
const int oneWireBus = 4;

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;

// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

byte degree[8] = {
	0b01110,
	0b01010,
	0b01110,
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b00000
};

void setup(){
  
  // initialize LCD
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();
  // Start the Serial Monitor
  Serial.begin(115200);
  // Start the DS18B20 sensor
  sensors.begin();
  lcd.createChar(0, degree);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(ledPin, OUTPUT);

  timeClient.begin();

  pinMode(relay, OUTPUT);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
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
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "esp32/saunaswitch") {
    Serial.print("Changing output to ");
    if(messageTemp == "ON"){
      Serial.println("ON");
      digitalWrite(relay, HIGH);
    }
    else if(messageTemp == "OFF"){
      Serial.println("OFF");
      digitalWrite(relay, LOW);
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", "saunaESP32", "sAuNa")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/saunaswitch");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop(){

  sensors.requestTemperatures(); 
  String temperatureC = String(sensors.getTempCByIndex(0));
  double tempF = sensors.getTempFByIndex(0);
  String temperatureF = String(tempF);
  temperatureF += " F";
  temperatureC += " C";
  
  lcd.setCursor(4,1);
  lcd.print(temperatureF);
  lcd.setCursor(9, 1);
  lcd.write(0);
  delay(5000);
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  Serial.println(digitalRead(relay));

  if (digitalRead(relay) == HIGH){
    client.publish("esp32/saunastate", "ON");
  }
  if (digitalRead(relay) == LOW){
    client.publish("esp32/saunastate", "OFF");
  }

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    char tempString[8];
    dtostrf(tempF, 1, 2, tempString);
    client.publish("esp32/temperature", tempString);
    //Serial.println("temp published");
  }
  timeStuff();
}
void timeStuff(){
   timeClient.update();
  unsigned long unix_epoch = timeClient.getEpochTime();    // Get Unix epoch time from the NTP server
 
  second_ = second(unix_epoch);
  if (last_second != second_) {
 
    minute_ = minute(unix_epoch);
    hour_   = hour(unix_epoch);
    day_    = day(unix_epoch);
    month_  = month(unix_epoch);
    year_   = year(unix_epoch);
  
    //Serial.println(hour_);

    if(hour_>12){
      hour_ = hour_-12;
      DateTime[11]  = minute_ % 10 + 48;
      DateTime[10]  = minute_ / 10 + 48;
      DateTime[8]  = hour_   % 10 + 48;
      DateTime[7]  = hour_   / 10 + 48;
      DateTime[13] = 'P';
      DateTime[14] = 'M';
    
    }
    else{
      DateTime[14]  = minute_ % 10 + 48;
      DateTime[13]  = minute_ / 10 + 48;
      DateTime[11]  = hour_   % 10 + 48;
      DateTime[10]  = hour_   / 10 + 48;
      DateTime[13] = 'A';
      DateTime[14] = 'M';
    }
  
    DateTime[4]  = day_   / 10 + 48;
    DateTime[5]  = day_   % 10 + 48;
    DateTime[1]  = month_  / 10 + 48;
    DateTime[2]  = month_  % 10 + 48;
 
    //Serial.println(DateTime);
    lcd.setCursor(0, 0);
    lcd.print(DateTime);
    last_second = second_;
 
  }
}
