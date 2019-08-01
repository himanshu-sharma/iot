# IoT

## Air Quality Sensor: Measure Dust (PM2.5 & 10) Levels With Temperature and Humidity
#### SDS011 Dust Sensor - WeMos OLED Connections
#####
1. TXD    --------> D6
2. RXD    --------> D7
3. GND    --------> GND
4. 5V     --------> 5V

#### DHT11 Temperature Sensor - WeMos OLED Connections
#####
1. VCC --------> 3.3v or 5v
2. Data --------> D5
3. GND --------> GND

#### Thingspeak: Real Time Sensor Data Reporting and Analysis
https://thingspeak.com/channels/338402

#### Download Data in JSON, XML & CSV
https://thingspeak.com/channels/338402


##### To Do
1. Handle Temp and Humidity nan values.
2. Sleep and Wakeup mode of SDS011 sensor to capture the readings.
3. Connect to the mobile app.
