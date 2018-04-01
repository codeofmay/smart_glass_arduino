const char* sSID = "wemosd1";
const char* PASSWORD = "skytheodore";
const char* SERVER="api.openweathermap.org";
const String API_KEY="7c1219fbdb1731a3fbca8fa5d1e1ab7a";
String cityName = "Yangon";
String cityId="1298824";
String currentWeather = "GET /data/2.5/weather?";
String forecastWeather = "GET /data/2.5/forecast?";

const String DAY[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
const String MONTH[]={"Jan","Feb","Mar","April","May","June","July","August","Sep","Oct","Nov","Dec"};

const char* WUNDERGROUND_SERVER="api.wunderground.com";
const String WUNDERGROUND_API_KEY="5abecca5bd642af5";
const char WUNDERGROUND_REQ[] =
    "GET /api/5abecca5bd642af5/conditions/q/Yangon.json HTTP/1.1\r\n"
    "User-Agent: ESP8266/0.1\r\n"
    "Accept: */*\r\n"
    "Host: api.wunderground.com\r\n"
    "Connection: close\r\n"
    "\r\n";

const char WUNDERGROUND_FORECAST_REQ[] =
    "GET /api/5abecca5bd642af5/forecast/q/Yangon.json HTTP/1.1\r\n"
    "User-Agent: ESP8266/0.1\r\n"
    "Accept: */*\r\n"
    "Host: api.wunderground.com\r\n"
    "Connection: close\r\n"
    "\r\n";
