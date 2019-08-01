#include "ThingSpeak.h"
#include "secrets.h"
#include <ESP8266WiFi.h>
#include "DHT.h"

#include "SdsDustSensor.h"
#include <SDS011.h>
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
SSD1306  display(0x3c, 5, 4); // Initialise the OLED display using Wire library

int rxPin = D7;
int txPin = D6;
SdsDustSensor sds(rxPin, txPin);

#define DHTPIN D5     // what pin we're connected to
#define DHTTYPE DHT11   // DHT 11 
// Initialize DHT sensor for normal 16mhz Arduino
DHT dht(DHTPIN, DHTTYPE);

char ssid[] = SECRET_SSID;   // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

void setup() {
  Serial.begin(9600);
  display.init(); // Initialising the UI will init the display too.
  //display.flipScreenVertically();
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(9, 36, "Starting...");
  display.display();
  sds.begin();

  Serial.println(sds.queryFirmwareVersion().toString()); // prints firmware version
  Serial.println(sds.setActiveReportingMode().toString()); // ensures sensor is in 'active' reporting mode
  Serial.println(sds.setContinuousWorkingPeriod().toString()); // ensures sensor has continuous working period - default but not recommended
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  dht.begin();
}

void loop() {
  // Connect or reconnect to WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);
    }
    Serial.println("\nConnected.");
  }

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index
  // Must send in temp in Fahrenheit!
  float hi = dht.computeHeatIndex(f, h);

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hi);
  Serial.println(" *F");

  String dust_aqi_cat = ""; //https://en.wikipedia.org/wiki/Air_quality_index
  PmResult pm = sds.readPm();
  if ((pm.isOk()) && (pm.pm25 > 2) && (pm.pm10 > 2)) {
    Serial.print("PM2.5 = ");
    Serial.print(pm.pm25);
    Serial.print(", PM10 = ");
    Serial.println(pm.pm10);

    // if you want to just print the measured values, you can use toString() method as well
    Serial.println(pm.toString());
    if (pm.pm10 > 0 and pm.pm10 <= 50) {
      Serial.println("Good");
      dust_aqi_cat = "Good";
    } else if (pm.pm10 > 51 and pm.pm10 <= 100) {
      Serial.println("Satisfactory");
      dust_aqi_cat = "Satisfactory";
    } else if (pm.pm10 > 101 and pm.pm10 <= 250) {
      Serial.println("Moderate");
      dust_aqi_cat = "Moderate";
    } else if (pm.pm10 > 251 and pm.pm10 <= 350) {
      Serial.println("Poor");
      dust_aqi_cat = "Poor";
    } else if (pm.pm10 > 351 and pm.pm10 <= 430) {
      Serial.println("Very Poor");
      dust_aqi_cat = "Very Poor";
    } else if (pm.pm10 > 430) {
      Serial.println("Severe");
      dust_aqi_cat = "Severe";
    }

    if (pm.pm25 > 0 and pm.pm25 <= 30) {
      Serial.println("Good");
      dust_aqi_cat = "Good";
    } else if (pm.pm25 > 31 and pm.pm25 <= 60) {
      Serial.println("Satisfactory");
      dust_aqi_cat = "Satisfactory";
    } else if (pm.pm25 > 61 and pm.pm25 <= 90) {
      Serial.println("Moderate");
      dust_aqi_cat = "Moderate";
    } else if (pm.pm25 > 91 and pm.pm25 <= 120) {
      Serial.println("Poor");
      dust_aqi_cat = "Poor";
    } else if (pm.pm25 > 121 and pm.pm25 <= 250) {
      Serial.println("Very Poor");
      dust_aqi_cat = "Very Poor";
    } else if (pm.pm25 > 250) {
      Serial.println("Severe");
      dust_aqi_cat = "Severe";
    }


    Serial.println("Status:  " + dust_aqi_cat);

    // set the status
    ThingSpeak.setStatus(dust_aqi_cat);

    // set the fields with the values
    ThingSpeak.setField(1, pm.pm25);
    ThingSpeak.setField(2, pm.pm10);
    ThingSpeak.setField(3, t);
    ThingSpeak.setField(4, h);
    ThingSpeak.setField(5, hi);

    // write to the ThingSpeak channel
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if (x == 200) {
      Serial.println("Channel update successful.");
    }
    else {
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }

    // oled display
    String Oled_Row1 = "PM2.5: " + String(pm.pm25) + "ug/m3";
    String Oled_Row2 = "PM10: " + String(pm.pm10) + "ug/m3";
    String Oled_Row3 = String(dust_aqi_cat);
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawString(9, 12, Oled_Row1);
    display.drawString(9, 27, Oled_Row2);
    display.setFont(ArialMT_Plain_16);
    display.drawString(9, 42, Oled_Row3);
    display.display();
  } else {
    // notice that loop delay is set to 0.5s and some reads are not available
    Serial.print("Could not read values from sensor, reason: ");
    Serial.println(pm.statusToString());
  }

  delay(60000);
}
