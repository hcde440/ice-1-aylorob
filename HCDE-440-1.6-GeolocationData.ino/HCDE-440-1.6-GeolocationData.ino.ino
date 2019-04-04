/*A sketch to get the ESP8266 on the network and connect to some open services via HTTP to
 * get our external IP address and (approximate) geolocative information in the getGeo()
 * function. To do this we will connect to http://freegeoip.net/json/, an endpoint which
 * requires our external IP address after the last slash in the endpoint to return location
 * data, thus http://freegeoip.net/json/XXX.XXX.XXX.XXX
 * 
 * This sketch also introduces the flexible type definition struct, which allows us to define
 * more complex data structures to make receiving larger data sets a bit cleaner/clearer.
 * 
 * jeg 2017
 * 
 * updated to new API format for Geolocation data from ipistack.com
 * brc 2019
*/
#include <ESP8266WiFi.h>                                  //Include the Wifi library
#include <ESP8266HTTPClient.h>                            //Include the HTTP library
#include <ArduinoJson.h>                                  //Provides the ability to parse and construct JSON objects
const char* ssid = "Samsung Galaxy S9_1914";              //The name of the Wifi you're accessing goes here
const char* pass = "thga9095";                            //The password of the Wifi goes here
const char* key = "0bf915371829ae8594a909bde9deea02";     //The Api key for Geolocating
const char* weaKey = "f73360891699e4783ef6739e07ec1e57";  //The Api key for weather

typedef struct {                                          //Here we create a new data type definition, a box to hold other data types
  String ip;                                              //The IP address
  String cc;                                              //For each name:value pair coming in from the service, we will create a slot --- Country abbreviation
  String cn;                                              //In our structure to hold our data --- Country
  String rc;                                              //State
  String rn;                                              //"Region Name", not used in serial output
  String cy;                                              //City
  String ln;                                              //Longitude
  String lt;                                              //Latitude
} GeoData;                                                //Then we give our new data structure a name so we can use it in our code

GeoData location;                                         //We have created a GeoData type, but not an instance of that type,
                                                          //So we create the variable 'location' of type 

typedef struct {                                          //Its another new data structure
  String temp;                                            //Temperature
  String humid;                                           //Humidity
  String windS;                                           //Wind speed
  String windD;                                           //Wind direction, which is in degrees it seems
  String cloud;                                           //Cloud patterns, which is really weather description
  String pr;                                              //Pressure because it was in the example code and why not
} MetData;                                                //The name of the newfangled data structure

MetData weather;                                          //Creating an instance of MetData

void setup() {                                            //The setup of the arduino, runs once
  Serial.begin(115200);                                   //Speed of processing
  delay(10);                                              //A delay, for less gibberish in the first line of the monitor?
  
  Serial.print("This board is running: ");
  Serial.println(F(__FILE__));                            //These four lines give description of of file name and date 
  Serial.print("Complied: ");
  Serial.println(F(__DATE__ " " __TIME__));

  Serial.print("Connecting to "); Serial.println(ssid);   //Print name of Wifi
  WiFi.mode(WIFI_STA);                                    //Start the Wifi in station mode rather than soft point mode
  WiFi.begin(ssid, pass);                                 //Begin the Wifi attempt
  while (WiFi.status() != WL_CONNECTED) {                 //While not connected, chill and dots. Connect, move on
    delay(500);
    Serial.print(".");
  }
  
  Serial.println(); Serial.println("WiFi connected"); Serial.println();                   //Print Wifi connected on a new line with space before and after
  Serial.print("Your ESP has been assigned the internal IP address ");                    //Print that text
  Serial.println(WiFi.localIP());                                                         //Print the LocalIP
  
  getGeo();                                                                               //Run getGeo, defined below, and it will fill out Geodata
  
  Serial.println("Your external IP address is " + location.ip);                           //Print your external IP address
  Serial.print("Your ESP is currently in " + location.cn + " (" + location.cc + "),");    //Print off the country and abbreviation
  Serial.println(" in or near " + location.cy + ", " + location.rc + ".");                //Print city and state
  Serial.println("and located at (roughly) ");                                            //Just print that
  Serial.println(location.lt + " latitude by " + location.ln + " longitude.");            //Print location coordinates

  getMet(location.cy);                                                                    //Run getMet, defined below, and it will fill out Met data. Also it inputs your city location

  Serial.println();                                                                                               //Empty line
  Serial.println("With " + weather.cloud + ", the temperature in " + location.cy + ", " + location.rc);           //The rest of setup prints the forecast using Metdata
  Serial.println("is " + weather.temp + "C, with a humidity of " + weather.humid + "%. The winds are blowing");
  Serial.println(weather.windD + " at " + weather.windS + " Kilometers per hour, and the ");
  Serial.println("barometric pressure is at " + weather.pr + " millibars.");
  
}
void loop() {                                               //Loops infinately on arduino, except this is empty
                                                            //if we put getIP() here, it would ping the endpoint over and over . . . DOS attack?
}
String getIP() {                                            //The getIP fuction returns a string
  HTTPClient theClient;                                     //Instance of HTTPCilient
  String ipAddress;                                         //Make a string called ipAddress
  theClient.begin("http://api.ipify.org/?format=json");     //run that webpage using theClient
  int httpCode = theClient.GET();                           //get the httpcode from the client
  if (httpCode > 0) {                                       //If less than one you didn't connect
    if (httpCode == 200) {                                  //200 is the number of success
      DynamicJsonBuffer jsonBuffer;                         //Make a place to put JSON of some size
      String payload = theClient.getString();               //Get the not yet json as a big string
      //Serial.println(payload);
      JsonObject& root = jsonBuffer.parse(payload);         //Parse the payload string as JSON, making it the root
      ipAddress = root["ip"].as<String>();                  //Set ipAddress to the IP address by accessing that key, set as string
      //Serial.println(ipAddress);
    } else {                                                                    //If didn't connect
      Serial.println("Something went wrong with connecting to the endpoint.");  //Say so
      return "error";                                                           //set result as error
    }
  }
  return ipAddress;                                                             //return the IP address
}
void getGeo() {                                                                                       //getGeo doesn't return anything
  HTTPClient theClient;                                                                               //Make a HTTP Client
  Serial.println("Making HTTP request");                                                              //Print that
  theClient.begin("http://api.ipstack.com/" + getIP() + "?access_key=" + key);                        //Access the api with your IP and access key thrown in 
  int httpCode = theClient.GET();                                                                     //Get the results
  if (httpCode > 0) {                                                                                 //Less than 0 didnt connect
    if (httpCode == 200) {                                                                            //200 means success
      Serial.println("Received HTTP payload.");                                                       //Print that
      DynamicJsonBuffer jsonBuffer;                                                                   //Make a space for json of undetermined size
      String payload = theClient.getString();                                                         //Get the results as a string
      Serial.println("Parsing...");                                                                   //Print that
      JsonObject& root = jsonBuffer.parse(payload);                                                   //Parse the string as json and set it as the root
                                                                                                               
      if (!root.success()) {                                                                          // Test if parsing succeeds. If not:
        Serial.println("parseObject() failed");                                                       //Print that
        Serial.println(payload);                                                                      //Print that too
        return;                                                                                       //Exit the function
      }
      //Some debugging lines below:
      //      Serial.println(payload);
      //      root.printTo(Serial);
                                                              //Using .dot syntax, we refer to the variable "location" which is of
                                                              //type GeoData, and place our data into the data structure.
      location.ip = root["ip"].as<String>();                  //we cast the values as Strings b/c
      location.cc = root["country_code"].as<String>();        //the 'slots' in GeoData are Strings
      location.cn = root["country_name"].as<String>();
      location.rc = root["region_code"].as<String>();
      location.rn = root["region_name"].as<String>();
      location.cy = root["city"].as<String>();
      location.lt = root["latitude"].as<String>();
      location.ln = root["longitude"].as<String>();
    } else {
      Serial.println("Something went wrong with connecting to the endpoint.");     //If all else fails, print this              
    }
  }
}

void getMet(String city) {                                                    //Takes string input, outputs nothing
  HTTPClient theClient;                                                       //Make a client
  String apiCall = "http://api.openweathermap.org/data/2.5/weather?q=";       //Start forming the api call
  apiCall += city;                                                            //Add the city
  apiCall += "&units=imperial&appid=";                                        //Make sure imperial units and prep the key
  apiCall += weaKey;                                                          //Add the key itself
  theClient.begin(apiCall);                                                   //Make the call on the client
  int httpCode = theClient.GET();                                             //Get the response
  if (httpCode > 0) {                                                         //If it did connect

    if (httpCode == HTTP_CODE_OK) {                                           //If you got a good connection
      String payload = theClient.getString();                                 //Get json as string
      DynamicJsonBuffer jsonBuffer;                                           //Make json buffer of adaptable size
      JsonObject& root = jsonBuffer.parseObject(payload);                     //Parse string and set it as root
      if (!root.success()) {                                                  //If that didnt work
        Serial.println("parseObject() failed in getMet().");                  //Print that
        return;                                                               //Exit
      }
      weather.temp = root["main"]["temp"].as<String>();                       //Assigning json to MetData as strings casue thats what it expects
      weather.pr = root["main"]["pressure"].as<String>();
      weather.humid = root["main"]["humidity"].as<String>();
      weather.cloud = root["weather"][0]["description"].as<String>();
      weather.windS = root["wind"]["speed"].as<String>();
      weather.windD = root["wind"]["deg"].as<int>();
    }
  }
  else {                                                                                  //If all else fails
    Serial.printf("Something went wrong with connecting to the endpoint in getMet().");   //Print that
  }
}
