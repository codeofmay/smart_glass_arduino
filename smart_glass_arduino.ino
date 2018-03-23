#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "stationCredentials.h"

WiFiClient client;

String result;
String weatherLocation="";
String country="";
String mainCondition="";
String weatherDescription="";
float temperature=0;
float humidity=0;
float pressure=0;

int counter=30;

void setup() {
  delay(1000);
  Serial.begin(115200);
  WiFi.begin(sSID, PASSWORD);
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

void loop() {
  //display
  if(counter==30){
    counter=0;
    delay(1000);
    Serial.println("New");
    getWeatherData();
    displayWeather();
  }else{
    counter++;
    delay(10000);
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

void displayWeather(){
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

