#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <time.h>
#include "stationCredentials.h"
long DELAY_ERROR = 5 * 60 * 1000;
long DELAY_NORMAL = 20 * 60 * 1000;

WiFiClient client;
float timezone = 6.5;
int dst = 0;

int hour;
int minute;
int second;
int dayIndex;
int monthIndex;
int day;
int year;

String result;
String weatherLocation = "";
String country = "";
String mainCondition = "";
String weatherDescription = "";
float temperature = 0;
String humidity;
float pressure = 0;


int counter = 300;


void setup() {
  delay(1000);
  Serial.begin(115200);
  WiFi.begin(sSID, PASSWORD);
  displayWifiStatus();
  configTime(6.5 * 3600, dst * 0, "pool.ntp.org", "time.nist.gov");
}

void loop() {
  delay(1000);


  //updating weather data
  if (counter == 300) {
    //pull from server in every 5 minutes
    counter = 0;
    Serial.println("New");
    getForecastDataFromWUnderGround(WUNDERGROUND_FORECAST_REQ, "forecast");
    displayCurrentWeather();
  } else {
    counter++;
    //    Serial.println("Old");
    //    displayPreviousWeather( weatherLocation,
    //                            country, mainCondition, weatherDescription,
    //                            temperature, humidity, pressure);
  }

  //updating time
  time_t now;
  struct tm * timeinfo;
  time(&now);
  timeinfo = localtime(&now);
  hour = timeinfo->tm_hour;
  minute = timeinfo->tm_min + 30;
  second = timeinfo->tm_sec;
  dayIndex = timeinfo->tm_wday;
  monthIndex = timeinfo->tm_mon + 1;
  year = timeinfo->tm_year + 1900;
  day = timeinfo->tm_mday;

  displayCurrentTime();
  displayCurrentDate();
}

void getWeatherDataFromOpenWeatherMap() {

  //connect api
  if (client.connect(SERVER, 80)) {
    client.println(currentWeather + "id=" + cityId + "&units=metric&APPID=" + API_KEY);
    client.println("Host: api.openweathermap.org");
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();
  } else {
    Serial.println("connection failed");
    Serial.println();
  }

  //get data from api
  while (client.connected() && !client.available())
    delay(1);
  while (client.connected() || client.available()) {
    char c = client.read();
    result += c;
  }
  client.stop();
  result.replace('[', ' ');
  result.replace(']', ' ');

  //convert string to charArray
  char jsonArray[result.length() + 1];
  result.toCharArray(jsonArray, sizeof(jsonArray));
  jsonArray[result.length() + 1] = '\0';

  //convert charArray to JsonObject
  StaticJsonBuffer<1024> jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(jsonArray);
  if (!root.success()) {
    Serial.println("Data Passing from JsonArray to JsonObject failed");
  }

  //extract data from jsonObject
  String location = root["name"];
  String coun = root["sys"]["country"];
  String main = root["weather"]["main"];
  String description = root["weather"]["description"];
  float temp = root["main"]["temp"];
  //float humid = root["main"]["humidity"];
  float pres = root["main"]["pressure"];

  weatherLocation = location;
  country = coun;
  mainCondition = main;
  weatherDescription = description;
  temperature = temp;
  //humidity = humid;
  pressure = pres;

}

void getWeatherDataFromWUnderGround(const char *request, String type) {
  //connect api
  if (client.connect(WUNDERGROUND_SERVER, 80)) {
    client.print(request);
    client.flush();
  } else {
    Serial.println("connection failed");
    Serial.println();
  }

  int respLen = 0;
  static char respBuf[4096];
  bool skip_headers = true;
  while (client.connected() || client.available()) {
    if (skip_headers) {
      String aLine = client.readStringUntil('\n');
      //Serial.println(aLine);
      // Blank line denotes end of headers
      if (aLine.length() <= 1) {
        skip_headers = false;
      }
    }
    else {
      int bytesIn;
      bytesIn = client.read((uint8_t *)&respBuf[respLen], sizeof(respBuf) - respLen);
      //Serial.print(F("bytesIn ")); Serial.println(bytesIn);
      if (bytesIn > 0) {
        respLen += bytesIn;
        if (respLen > sizeof(respBuf)) respLen = sizeof(respBuf);
      }
      else if (bytesIn < 0) {
        Serial.print(F("read error "));
        Serial.println(bytesIn);
      }
    }
    delay(1);
  }
  client.stop();

  if (respLen >= sizeof(respBuf)) {
    Serial.print(F("respBuf overflow "));
    Serial.println(respLen);
    delay(DELAY_ERROR);
    return;
  }
  // Terminate the C string
  respBuf[respLen++] = '\0';
  //  Serial.print(F("respLen "));
  //  Serial.println(respLen);

  if (type == "current") {
    extractCurrentFields(respBuf);
  } else if (type == "forecast") {
    extractForecastFields(respBuf);
  }
}

bool extractCurrentFields(char *json)
{
  StaticJsonBuffer<3 * 1024> jsonBuffer;

  // Skip characters until first '{' found
  // Ignore chunked length, if present
  char *jsonstart = strchr(json, '{');
  //Serial.print(F("jsonstart ")); Serial.println(jsonstart);
  if (jsonstart == NULL) {
    Serial.println(F("JSON data missing"));
    return false;
  }
  json = jsonstart;

  // Parse JSON
  JsonObject& root = jsonBuffer.parseObject(json);
  if (!root.success()) {
    Serial.println(F("jsonBuffer.parseObject() failed"));
    return false;
  }

  // Extract weather info from parsed JSON
  JsonObject& current = root["current_observation"];
  JsonObject& location = root["current_observation"]["display_location"];
  const float temp_c = current["temp_c"];
  const char *humi = current[F("relative_humidity")];
  const char *weather = current["weather"];
  const float pressure_in = current["pressure_in"];
  const char *description = current["icon_url"];
  const char *city = location["city"];
  const char *coun = location["state_name"];

  weatherLocation = city;
  country = coun;
  mainCondition = weather;
  weatherDescription = description;
  temperature = temp_c;
  humidity = humi;
  pressure = pressure_in;

  return true;
}

void getForecastDataFromWUnderGround(const char *request, String type) {
  //connect api
  if (client.connect(WUNDERGROUND_SERVER, 80)) {
    client.print(request);
    client.flush();
  } else {
    Serial.println("connection failed");
    Serial.println();
  }
  bool skip_headers = true;


  while (client.connected() || client.available()) {
    if (skip_headers) {
      String aLine = client.readStringUntil('\n');
      //Serial.println(aLine);
      // Blank line denotes end of headers
      if (aLine.length() <= 1) {
        skip_headers = false;
      }
    }
    else {
      String line = client.readStringUntil('\r');
      if (line != " ") {
        result += line;
      }
      delay(1);
    }
  }

  client.stop();
  result.replace('[', ' ');
  result.replace(']', ' ');
  result.replace(" ", "");
  result.trim();

  Serial.println(result);
  char jsonArray[result.length() + 1];
  try{
  result.toCharArray(jsonArray, sizeof(jsonArray));
  }catch(Exception e){
    e.print();
  }
  //StaticJsonBuffer<3*1024> jsonBuffer;
  //JsonObject &root = jsonBuffer.parseObject(result);
  //Serial.println(jsonArray);
  //extractForecastFields(jsonArray);

  //  //convert string to charArray
  //  char jsonArray[result.length() + 1];
  //  result.toCharArray(jsonArray, sizeof(jsonArray));
  //  jsonArray[result.length() + 1] = '\0';
  //
  //  //convert charArray to JsonObject
  //  StaticJsonBuffer<1024> jsonBuffer;
  //  JsonObject &root = jsonBuffer.parseObject(jsonArray);
  //  if (!root.success()) {
  //    Serial.println("Data Passing from JsonArray to JsonObject failed");
  //  }

  //  int respLen = 0;
  //  static char respBuf[4096];
  //  bool skip_headers = true;
  //  while (client.connected() || client.available()) {
  //    if (skip_headers) {
  //      String aLine = client.readStringUntil('\n');
  //      //Serial.println(aLine);
  //      // Blank line denotes end of headers
  //      if (aLine.length() <= 1) {
  //        skip_headers = false;
  //      }
  //    }
  //    else {
  //      int bytesIn;
  //      bytesIn = client.read((uint8_t *)&respBuf[respLen], sizeof(respBuf) - respLen);
  //      //Serial.print(F("bytesIn ")); Serial.println(bytesIn);
  //      if (bytesIn > 0) {
  //        respLen += bytesIn;
  //        if (respLen > sizeof(respBuf)) respLen = sizeof(respBuf);
  //      }
  //      else if (bytesIn < 0) {
  //        Serial.print(F("read error "));
  //        Serial.println(bytesIn);
  //      }
  //    }
  //    delay(1);
  //  }
  //  client.stop();
  //
  //  if (respLen >= sizeof(respBuf)) {
  //    Serial.print(F("respBuf overflow "));
  //    Serial.println(respLen);
  //    delay(DELAY_ERROR);
  //    return;
  //  }
  //  // Terminate the C string
  //  respBuf[respLen++] = '\0';
  //  //  Serial.print(F("respLen "));
  //  //  Serial.println(respLen);
  //
  //  if (type == "current") {
  //    extractCurrentFields(respBuf);
  //  } else if (type == "forecast") {
  //    extractForecastFields(respBuf);
  //  }
}

void extractForecastFields(char *json) {
  StaticJsonBuffer<3 * 1024> jsonBuffer;

  //just filter
  char *jsonStart = strchr(json, '{');
  if (jsonStart == NULL) {
    Serial.println(F("JSON data missing"));
  }
  json = jsonStart;
  JsonObject& root = jsonBuffer.parseObject(json);
  if (!root.success()) {
    Serial.println(F("jsonBuffer.parseObject() failed"));
  }

  // Extract weather info from parsed JSON
  JsonObject& forecast = root["forecast"]["simpleforecast"]["forecastday"];
  JsonObject& firstDay = forecast["1"];
  JsonObject& secondDay = forecast["2"];
  JsonObject& thirdDay = forecast["3"];

  //first day forecast
  char weekDay = firstDay["date"]["weekday_short"];
  float max_temp = firstDay["high"]["celsius"];
  float min_temp = firstDay["low"]["celsius"];
  char condition = firstDay["conditions"];
  char icon = firstDay["icon_url"];

  //second day forecast
  //first day forecast
  char weekDay1 = secondDay["date"]["weekday_short"];
  float max_temp1 = secondDay["high"]["celsius"];
  float min_temp1 = secondDay["low"]["celsius"];
  char condition1 = secondDay["conditions"];
  char icon1 = secondDay["icon_url"];
  displayForecastWeather(weekDay, max_temp, min_temp, condition, icon);

  //thirday forecast
  char weekDay2 = firstDay["date"]["weekday_short"];
  float max_temp2 = firstDay["high"]["celsius"];
  float min_temp2 = firstDay["low"]["celsius"];
  char condition2 = firstDay["conditions"];
  char icon2 = firstDay["icon_url"];

  displayForecastWeather(weekDay, max_temp, min_temp, condition, icon);
}

void displayWifiStatus() {
  Serial.print ("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WIFI has been Connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void displayCurrentTime() {

  if (minute >= 60) {
    minute -= 60;
    hour++;
    if (hour >= 24) {
      hour -= 24;
    }
  }

  Serial.println();
  Serial.print("Time: ");
  Serial.print(hour);
  Serial.print(":");
  Serial.print(minute);
  Serial.print(":");
  Serial.println(second);
}

void displayCurrentDate() {
  Serial.print("Date: ");
  Serial.print(DAY[dayIndex]);
  Serial.print(", ");

  Serial.print(day);
  Serial.print(" ");
  Serial.print(MONTH[monthIndex]);
  Serial.print(" ");
  Serial.println(year);
}

void displayCurrentWeather() {
  Serial.print("Location: ");
  Serial.print(weatherLocation);
  Serial.print(", ");
  Serial.println(country);

  Serial.print("Main Condition: ");
  Serial.println(mainCondition);
  Serial.print("Description: ");
  Serial.println(weatherDescription);
  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Humidity: ");
  Serial.println(humidity);
  Serial.print("Pressure: ");
  Serial.println(pressure);
  Serial.println();
}

void displayForecastWeather(char weekDay, float max_temp, float min_temp, char condition, char icon) {
  //first day forecast
}

