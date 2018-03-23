#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <time.h>
#include "stationCredentials.h"

WiFiClient client;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

float timezone=6.5;
int dst=0;

String result;
String weatherLocation="";
String country="";
String mainCondition="";
String weatherDescription="";
float temperature=0;
float humidity=0;
float pressure=0;

int counter=300;

void setup() {
  delay(1000);
  Serial.begin(115200);
  WiFi.begin(sSID, PASSWORD);
  displayWifiStatus();
  //timeClient.begin();
  configTime(6.5*3600, dst*0, "pool.ntp.org", "time.nist.gov");
}

void loop() {
  //updating time
  delay(1000);
  //timeClient.update();
  time_t now=time(nullptr);
  Serial.println(ctime(&now));
  //displayCurrentTime();
  
  //updating weather data
  if(counter==300){
    //pull from server in 5 minutes interval
    counter=0;
    Serial.println("New");
    getWeatherData();
    displayCurrentWeather();
  }else{
    counter++;
    Serial.println("Old");
//    displayPreviousWeather( weatherLocation,
//                        country,mainCondition,weatherDescription,
//                        temperature,humidity,pressure);
  }
}

void getWeatherData(){

  //connect api
  if(client.connect(SERVER,80)){
    client.println(currentWeather+"q="+cityName+"&units=metric&APPID="+API_KEY);
    client.println("Host: api.openweathermap.org");
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();
  } else {
    Serial.println("connection failed"); 
    Serial.println();
  }

  //get data from api
  while(client.connected() && !client.available())
    delay(1);
  while(client.connected() || client.available()){
    char c=client.read();
    result+=c;  
  }
  client.stop();
  result.replace('[',' ');
  result.replace(']',' ');

  //convert string to charArray
  char jsonArray[result.length()+1];
  result.toCharArray(jsonArray,sizeof(jsonArray));
  jsonArray[result.length()+1]='\0';

  //convert charArray to JsonObject
  StaticJsonBuffer<1024> jsonBuffer;
  JsonObject &root=jsonBuffer.parseObject(jsonArray);
  if(!root.success()){
    Serial.println("Data Passing from JsonArray to JsonObject failed");
  }

  //extract data from jsonObject
  String location=root["name"];
  String coun=root["sys"]["country"];
  String main=root["weather"]["main"];
  String description=root["weather"]["description"];
  float temp=root["main"]["temp"];
  float humid=root["main"]["humidity"];
  float pres=root["main"]["pressure"];
  
  weatherLocation=location;
  country=coun;
  mainCondition=main;
  weatherDescription= description;
  temperature=temp;
  humidity=humid;
  pressure=pres;

}

void displayWifiStatus(){
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
void displayCurrentWeather(){
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

void displayPreviousWeather(String location,String country,String mainCondition,String description,float temperature,
                         float humidity,float pressure){
                          
  
  Serial.print("Location: ");
  Serial.print(location);
  Serial.print(", ");
  Serial.println(country);

  Serial.print("Main Condition: ");
  Serial.println(mainCondition);
  Serial.print("Description: ");
  Serial.println(description);
  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Humidity: ");
  Serial.println(humidity);
  Serial.print("Pressure: ");
  Serial.println(pressure);
}

void displayCurrentTime(){
  Serial.println(timeClient.getFormattedTime());

}

