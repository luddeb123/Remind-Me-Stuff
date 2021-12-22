#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

//=============================================================================
// WiFi stuff
const char *ssid = "Wifi-Name";
const char *password = "Password";

//Firebase project credentials
#define API_KEY "API-KEY"
#define FIREBASE_PROJECT_ID "Firebase project id"

// Firebase user login credentials
#define USER_EMAIL "Email"
#define USER_PASSWORD "Password"

#define UID "Your UID in Firebase"; // My UID
#define sensorLocation "Location";  // Location of the unit (used to differentiate between different devices under the same user´)

#define DHTPIN 13                   // Pin for DHT

const uint8_t _buttonOnePin_ = 12;  // D6 / GPIO12
const uint8_t _buttonTwoPin_ = 14;  // D5 / GPIO14

// DHT stuff
#define DHTTYPE DHT11 // DHT 11
DHT dht(DHTPIN, DHTTYPE);
float temp = 0;
float humidity = 0;
long lastPublished = 0;

//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// NTP
const long utcOffsetInSeconds = 3600; // UTC+1
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

unsigned long dataMillis = 0;    // Time since last data was sent
unsigned long getdataMillis = 0; // Time since last data was retrived

// Reminders stuff
struct reminder
{
  String reminderName;
  boolean state;
  uint8_t ledPin;
};
struct reminder reminders[] = {
    {"Hot", false, 4},
    {"Cold", false, 5}
};
int nreminders = sizeof(reminders) / sizeof(reminders[0]); // Number of Reminders

// Button stuff
struct button
{
  String buttonName;
  uint8_t buttonPin;
  uint8_t currentState;
  uint8_t prevousState;
  uint64_t lastDebounce;
  String task;
};
struct button buttons[] = {
    {"btn1", _buttonOnePin_, 0, 0, 0, "Drink"},
    {"btn2", _buttonTwoPin_, 0, 0, 0, "Test"}};
int nbtns = sizeof(buttons) / sizeof(buttons[0]); // Number of Buttons

//=============================================================================
void updateLastCompleted(uint8_t btn)
{
  String documentPath = "users/" + UID + "/Reminders/" + buttons[btn].task;

  std::vector<struct fb_esp_firestore_document_write_t> writes;                           //The dyamic array of write object fb_esp_firestore_document_write_t.

  struct fb_esp_firestore_document_write_t transform_write;                               //A write object that will be written to the document.

  transform_write.type = fb_esp_firestore_document_write_type_transform;                  //Set the write object write operation type.

  transform_write.document_transform.transform_document_path = documentPath.c_str();      //Set the document path of document to write (transform)

  struct fb_esp_firestore_document_write_field_transforms_t field_transforms;             //Set a transformation of a field of the document.

  field_transforms.fieldPath = "lastCompleted";                                           //Set field path to write.

  field_transforms.transform_type = fb_esp_firestore_transform_type_set_to_server_value;  //Set the transformation type.

  field_transforms.transform_content = "REQUEST_TIME";                                    //set "users/{UID}/Reminders/{buttons[btn].task}/lastCompleted" to timestamp

  transform_write.document_transform.field_transforms.push_back(field_transforms);        //Add a field transformation object to a write object.

  writes.push_back(transform_write);                                                      //Add a write object to a write array.

  if (Firebase.Firestore.commitDocument(&fbdo, FIREBASE_PROJECT_ID, "", writes , ""))
    Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
  else
    Serial.println(fbdo.errorReason());
}

void addSensorData(float newTemp, float newHumidity)
{
  dataMillis = millis();
  String collectionPath = "users/" + UID + "/Sensors/" + sensorLocation + "/Data";
  FirebaseJson content;

  temp = newTemp;
  humidity = newHumidity;

  String documentPath = "users/" + UID + "/Sensors/Bedroom/Data";
  content.set("fields/Temp/doubleValue", newTemp);
  content.set("fields/Humidity/doubleValue", newHumidity);
  content.set("fields/Time/timestampValue", "0001-01-01T00:00:00Z");

  if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "", collectionPath.c_str(), content.raw()))
    Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
  else
    Serial.println(fbdo.errorReason());
}

void setup()
{
  Serial.begin(115200);
  // Init all btns and sensors
  dht.begin();

  Serial.print("Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);
  WiFi.waitForConnectResult();
  Serial.print(", WiFi connected, IP address: ");
  Serial.println(WiFi.localIP());

  // Firebase stuff
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  config.token_status_callback = tokenStatusCallback; 
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  timeClient.begin();

  for (int i = 0; i < nreminders; i++)
  {
    pinMode(reminders[i].ledPin, OUTPUT);
  }
  for (int i = 0; i < nbtns; i++)
  {
    pinMode(buttons[i].buttonPin, INPUT);
  }
}

void loop()
{
  // Debounce function for all buttons
  for (int btn = 0; btn < nbtns; btn++)
  {
    buttons[btn].prevousState = buttons[btn].currentState;
    buttons[btn].currentState = digitalRead(buttons[btn].buttonPin);
    if (
        buttons[btn].currentState == 1 &&
        millis() - buttons[btn].lastDebounce > 50 &&
        buttons[btn].currentState != buttons[btn].prevousState)
    {
      buttons[btn].lastDebounce = millis();
      Serial.print(buttons[btn].buttonName);
      Serial.println("DOWN");

      updateLastCompleted(btn);
    }
  }

  // Every 10 seconds send data DHTsensor data to Firestore
  if (Firebase.ready() && (millis() - dataMillis > 10000 || dataMillis == 0))
  {

    float newTemp = dht.readTemperature();
    float newHumidity = dht.readHumidity();
    if (isnan(newTemp) || isnan(newHumidity))
    {
      Serial.println("Failed to read from DHT sensor!");
    }
    else
    {
      addSensorData(newTemp, newHumidity);
    }
  }


  // Turns on a Led based on the response
  if (Firebase.ready() && (millis() - getdataMillis > 10000 || getdataMillis == 0))
  {
    getdataMillis = millis();
    String documentPath = "users/" + UID + "/Sensors/" + sensorLocation;

    if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str()))
    {
      String jsonData = fbdo.payload().c_str();
      StaticJsonDocument<256> doc;
      DeserializationError error = deserializeJson(doc, jsonData);

      if (error)
      {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
      }

      String tempStatus = doc["fields"]["Status"]["stringValue"];
      if (tempStatus == "Hot")
      {
        digitalWrite(4, HIGH);
        digitalWrite(5, LOW);
      }
      if (tempStatus == "Good")
      {
        digitalWrite(4, LOW);
        digitalWrite(5, LOW);
      }
      if (tempStatus == "Cold")
      {
        digitalWrite(4, LOW);
        digitalWrite(5, HIGH);
      }
      Serial.println(tempStatus);
    }
    else
    {
      Serial.println(fbdo.errorReason());
    }
  }
}