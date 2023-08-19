#include <DHT.h>  // Including library for dht
#include <ESP8266WiFi.h>
#include <ThingSpeak.h>

const char * Wapi_key = "P1YDL30G********";     //  Enter your Write API key from ThingSpeak
const char *ssid =  "SSID";     // replace with your wifi ssid and wpa2 key
const char *pass =  "Password";
const char* server = "api.thingspeak.com";
unsigned long ChnlNum = 1824970;

#define DHTPIN 14
#define Type DHT11

DHT HT(DHTPIN, Type);
float t, h;

//------------------------------------------------------
int dry = 1024;
int wet = 465;
int SV = 13;
//------------------------------------------------------                 motor GPIO 15
const int pingPin = 4;
const int echoPin = 5;
int empty = 28;
int full = 8;
//------------------------------------------------------
int motor = 15;
int morun;
WiFiClient client;

void Connect()
{
  WiFi.mode(WIFI_STA);

  WiFi.disconnect();
  delay(100);
  String ssid;
  int32_t rssi;
  uint8_t encryptionType;
  uint8_t* bssid;
  int32_t channel;
  bool hidden;
  int scanResult;

  Serial.println(F("Starting WiFi scan..."));

  scanResult = WiFi.scanNetworks(/*async=*/false, /*hidden=*/true);

  if (scanResult == 0) {
    Serial.println(F("No networks found"));
  } else if (scanResult > 0) {
    Serial.printf(PSTR("%d networks found:\n"), scanResult);


    for (int8_t i = 0; i < scanResult; i++) {                   //i<1 this one should be changed to scan result(it is set to one just to connect the desired wifi network)
      WiFi.getNetworkInfo(i, ssid, encryptionType, rssi, bssid, channel, hidden);

      Serial.printf(PSTR("  %02d: [CH %02d] [%02X:%02X:%02X:%02X:%02X:%02X] %ddBm %c %c %s\n"),
                    i,
                    channel,
                    bssid[0], bssid[1], bssid[2],
                    bssid[3], bssid[4], bssid[5],
                    rssi,
                    (encryptionType == ENC_TYPE_NONE) ? ' ' : '*',
                    hidden ? 'H' : 'V',
                    ssid.c_str());
      delay(10);
    }
  } else {
    Serial.printf(PSTR("WiFi scan error %d"), scanResult);
  }


  delay(5000);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Wifi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}
void tempHumid()
{
  h = HT.readHumidity();
  t = HT.readTemperature();

  if (isnan(h) || isnan(t))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  ThingSpeak.setField(1, t);
  ThingSpeak.setField(2, h);


  Serial.print("Temperature");
  Serial.println(t);
  Serial.print("Humidity");
  Serial.println(h);

}

void setup()
{
  Serial.begin(115200);
  delay(10);
  Connect();
  ThingSpeak.begin(client);
  pinMode(SV, OUTPUT);
  pinMode(motor, OUTPUT);
  HT.begin();


}

void loop()
{
  delay(2000);
  tempHumid();

  int sensorVal = analogRead(A0);

  int percentage = map(sensorVal, wet, dry, 100, 0);
  Serial.print(percentage);
  Serial.println("%");
  delay (500);


  int D = percentage;
  ThingSpeak.setField(3, D);
  if (D <= 30)
  {
    Serial.println("Valve Open");
    delay(50);
    digitalWrite(SV, LOW);
  }
  else
  {
    digitalWrite(SV, HIGH);
    Serial.println("Valve Closed");
  }


  //------------------------------------
  long duration, inches, cm;
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(pingPin, LOW);
  pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);
  inches = microsecondsToInches(duration);
  cm = microsecondsToCentimeters(duration);
  //Serial.print(inches);
  //Serial.print("in, ");
  Serial.print(cm);
  Serial.print("cm");
  Serial.println();
  int per = map(cm, empty, full, 0, 100);
  Serial.print("water level");
  Serial.println(per);
  ThingSpeak.setField(4, per);
  if (per < 30)
  {
    digitalWrite(motor, LOW);
    delay(100);
    morun = 1;
  }
  if (per >= 100)
  {
    digitalWrite(motor, HIGH);
    delay(100);
    morun = 0;
  }
  ThingSpeak.setField(5, morun);
  ThingSpeak.writeFields(ChnlNum, Wapi_key);
  delay(100);
  // ---------------


  client.stop();


  Serial.println("Waiting...");


  delay(15000);
}


long microsecondsToInches(long microseconds) {
  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds) {
  return microseconds / 29 / 2;
}
