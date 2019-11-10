// *******************************************************************************
// **                               Library Includes                            **
// *******************************************************************************

// SPI File system is SPIFFS. Spiffy huh - but we DO need this include.....
#include <FS.h>

//needed for 8266 WiFi connectivity 
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino


//needed for DNS, Web server and initial SSID connection
//#include <WebSocketsServer.h>    //https://github.com/Links2004/arduinoWebSockets
#include <DNSServer.h>           //https://github.com/esp8266/Arduino/tree/master/libraries/DNSServer
#include <ESP8266WebServer.h>    //https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

// This needed for multicastDNS responder which allows us to assign a .local hostname
#include <ESP8266mDNS.h>         //https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266mDNS

// For talking to I2C ADC converter
#include <Adafruit_ADS1015.h>    //https://github.com/adafruit/Adafruit_ADS1X15      Includes Wire.h as a dependancy

// This needed for getting network time
#include <NTPClient.h>          //https://github.com/arduino-libraries/NTPClient
#include <WiFiUdp.h>

// Jons PID code.
#include "jonPID.h"

// JSON libraries - Data is passed to / from web client using JSON
// JSON stands for "Just a String Of Numbers" or "JavaScript Object Notation" depending on your humour level
#define ARDUINOJSON_USE_DOUBLE 1
#define ARDUINOJSON_NEGATIVE_EXPONENTIATION_THRESHOLD 1e-2
#include <ArduinoJson.h>         //https://github.com/bblanchon/ArduinoJson

#define CUEKING_FIRMWARE_VERSION "1.06"

// Over the Air updates. Allows us to update firmware of device via WiFi and without needing physical USB connetion 
#include <ArduinoOTA.h>          //???

// *******************************************************************************
// **                               Global Variables                            **
// *******************************************************************************

const int led = D4;
const int fan = D0;

String mdnsname="cueKing";

String adminpass="21232f297a57a5a743894a0e4a801fc3";

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
long UTCtimeoffset=0;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", UTCtimeoffset);

int datafilesize=0;

bool parametersNeedSaving = false;

ESP8266WebServer server(80);

//WebSocketsServer webSocket = WebSocketsServer(81);

PID mainPID("BBQControl",60000,225.0,2.0,0.2,4.0);
Adafruit_ADS1115 ads;
double temp1,temp2;
double tune=0.0;
double shift=0.0;


double p1sharta=1.98592665e-03;
double p1shartb=6.68028033e-05;
double p1shartc=9.48003051e-07;
double p2sharta=1.98592665e-03;
double p2shartb=6.68028033e-05;
double p2shartc=9.48003051e-07;

// tscale indicates temperature scale to use.
// 1=Kelvin,2=Centigrade,3=Faren
// Usually set from the config file, but if not....choose here
unsigned int tscale=1;



// *******************************************************************************
// **                                Setup Function                             **
// *******************************************************************************

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    pinMode(led, OUTPUT);
    pinMode(fan, OUTPUT);

    if (SPIFFS.begin()) Serial.println("SPIFFS file system enabled"); else Serial.println("Error mounting SPIFFS file system");

    File configFile;
    String myMAC = WiFi.macAddress();
    myMAC.replace(':', '-');
    String cfname = "/config" + myMAC + ".json";
    Serial.print("Reading config file " + cfname + " ");
    configFile = SPIFFS.open(cfname.c_str(),"r");
    if (!configFile) { 
      Serial.print("Failed. Trying config.json ");
      configFile = SPIFFS.open("/config.json","r");
    }
    char json[768];
    configFile.readBytes(json,configFile.size());
    json[configFile.size()]='\0';
    configFile.close();
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(error.c_str());
        return;
    }
    UTCtimeoffset = doc["UTCoffset"];
    double lastPIDtemp = doc["lastpidtemp"];
    bool pidRunning = doc["pidrunning"];
    String strPIDname = doc["pidname"];
    String dnsname = doc["dnsname"]; mdnsname=dnsname;
    mainPID.setPIDname(strPIDname);
    mainPID.setUpdateInterval( doc["pidinterval"] );
    mainPID.setErrOverTime( doc["pideot"] );
    mainPID.setTarget( doc["targettemp"] );
    mainPID.setkP( doc["kP"] );
    mainPID.setkI( doc["kI"] );
    mainPID.setkD( doc["kD"] );
    tscale = doc["tscale"];
    tune = doc["tune"];
    shift = doc["shift"];
    p1sharta = doc["p1sharta"];
    p1shartb = doc["p1shartb"];
    p1shartc = doc["p1shartc"];
    p2sharta = doc["p2sharta"];
    p2shartb = doc["p2shartb"];
    p2shartc = doc["p2shartc"];
    mainPID.control = doc["fanpercent"];
    mainPID.begin( lastPIDtemp );
    if (!pidRunning) mainPID.end();
    Serial.println("Done!");

    if (MDNS.begin(mdnsname)) {
      Serial.println("MDNS responder started - " + mdnsname);
    } else {
      Serial.println("Problem starting MDNS responder - " + mdnsname);
    }
    
    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
    //reset saved settings
    //wifiManager.resetSettings();
    
    //set custom ip for portal
    wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

    //fetches ssid and pass from eeprom and tries to connect
    //if it does not connect it starts an access point with the specified name
    //here  "ConfigureBBQ"
    //and goes into a blocking loop awaiting configuration
    wifiManager.autoConnect("ConfigureBBQ");
    //or use this for auto generated name ESP + ChipID
    //wifiManager.autoConnect();

    
    //if you get here you have connected to the WiFi
    //Serial.println("connected...yeey :)");
    Serial.printf("Connected to %s\n", WiFi.SSID().c_str());


    
    // This is the MD5 of the password needed to load new firmware using OTA updates. Of course MD5 is not
    // considered to be secure so if your want the password then its time to get your rainbow tables out.
    // Just a hint though - The password is long, uses upper and lower case as well as numbers and special
    // characters. Have at it!
    //ArduinoOTA.setPasswordHash("aa38956b7fbfbe36b714ac6b73b0cff3");

    ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else { // U_SPIFFS
        type = "filesystem";
      }
      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
//      Serial.println("Start");
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      }
    });
    ArduinoOTA.begin();
    Serial.println("OverTheAir (OTA) updates enabled");


  
    server.on("/api/reset", reboot8266 );
    server.on("/api/login",HTTP_POST,logindevice);
    server.on("/api/login",HTTP_GET,logoutdevice);
    server.on("/api/log", displog );
    server.on("/api/reboot", reboot8266);
    server.on("/api/wipegraph",apiwipegraph);
    server.on("/api/probe1",useprobe1);
    server.on("/api/probe2",useprobe2);
    server.on("/api/getparms",apigetparms);
    server.on("/api/setparms",HTTP_POST,apisetparms);
    server.on("/api/setpass",HTTP_POST,apisetpass);
    server.on("/api/togglepid",apitogglepid);
    server.onNotFound(handleOther);
    const char * headerkeys[] = {"Cookie", "User-Agent"} ;
    size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
    //ask server to track these headers
    server.collectHeaders(headerkeys, headerkeyssize);
    server.begin();
    Serial.println("HTTP server started");

    // start webSocket server
    //webSocket.begin();
    //webSocket.onEvent(webSocketEvent);

    Wire.begin(D1,D2);         //      D1,D2   I2C pins - SDA,SCL
    //                                                                ADS1015  ADS1115
    //                                                                -------  -------
    // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
    // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
    // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
    // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
    // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
    // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
    Serial.printf("I2C ADC initialised (%d,%d)\n",D1,D2);

    File dfile = SPIFFS.open("/data.csv","r");
    if (!dfile) Serial.println("Error opening data.csv to get size"); else {
      datafilesize = dfile.size();
      dfile.close();
      Serial.printf("Data log file size (data.csv) : %d\n",datafilesize);
    }

    timeClient.begin();
    Serial.printf("Got correct time after %d attempts\n", getcurrenttime() );   
    Serial.println( timeClient.getFormattedTime() );
    timeClient.setTimeOffset(UTCtimeoffset);
    Serial.println( timeClient.getFormattedTime() );

    addlog("Initialised\n");

}

// *******************************************************************************
// **                              Web Server Functions                         **
// *******************************************************************************

bool loadFromSpiffs(String path)
{
  //double now=millis();
  String dataType = "text/plain";
  if (path.endsWith("/")) path += "index.htm"; //this is where index.htm is created

  if (path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
//  else if (path.endsWith(".json")) dataType = "application/json";
  else if (path.endsWith(".htm")) dataType = "text/html";
  else if (path.endsWith(".html")) dataType = "text/html";
  else if (path.endsWith(".css")) dataType = "text/css";
  else if (path.endsWith(".csv")) dataType = "text/csv";
  else if (path.endsWith(".js")) dataType = "application/javascript";
  else if (path.endsWith(".png")) dataType = "image/png";
  else if (path.endsWith(".gif")) dataType = "image/gif";
  else if (path.endsWith(".svg")) dataType = "image/svg+xml";
  else if (path.endsWith(".jpg")) dataType = "image/jpeg";
  else if (path.endsWith(".ico")) dataType = "image/x-icon";
  else if (path.endsWith(".xml")) dataType = "text/xml";
  else if (path.endsWith(".pdf")) dataType = "application/pdf";
  else if (path.endsWith(".zip")) dataType = "application/zip";
  else if (server.hasArg("download")) dataType = "application/octet-stream";
  
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz)) { path += ".gz"; }  //prefer compressed over raw
    File dataFile = SPIFFS.open(path, "r");
    if (!dataFile) { Serial.println("Error opening webfile : "+path); return false; }
    int bytesStreamed=server.streamFile(dataFile, dataType);
    if ( bytesStreamed != dataFile.size()) {}
    dataFile.close();
    return true;
  }
  Serial.println("Don't know this command and it's not a file in SPIFFS : "+path);
  return false;
}

void handleOther() {   
  String mypath=server.uri();
  if (!mypath.endsWith("loginpage.htm")) { if (!isauthenticated()) { logoutdevice(); } }
  if (loadFromSpiffs(mypath)) return;   //gotcha - it's a file in SPIFFS
  String message = "Not Found\n\n";           //or not...
  message += "URI: ";     //make a 404 response & provide debug information
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " NAME:" + server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  Serial.println(message);
}



// *******************************************************************************
// **                             Jons Utility Functions                        **
// *******************************************************************************


// Gets the current time from NTP servers. Returns the number of attempts.....
int getcurrenttime() {
  int attempts=0;
  while (1==1) { 
    attempts++;
    if ( timeClient.update() ) break;
  }
  return attempts++; 
}

String addlog(String addme){
  static String logstring="";
  String ts="\n"+timeClient.getFormattedTime()+" -> ";
  if (addme=="") return logstring;

  if (!(addme.lastIndexOf("\n")==-1)) {
    int lnl=addme.lastIndexOf("\n");
    String newaddme=addme.substring(0,lnl);
    newaddme.replace("\n",ts);
    addme=newaddme+addme.substring(lnl);

    lnl=logstring.lastIndexOf("\n");
    if (lnl==-1) {
      String newlogstring=ts+logstring;
      logstring=newlogstring;
    } else {
      String newlogstring = logstring.substring(0,lnl) + ts + logstring.substring(lnl+1);
      logstring=newlogstring;
    }
  }
  logstring += addme;
  if (logstring.length() > 1024) {
    int startchar=logstring.length()-1024;
    logstring = logstring.substring( startchar );
  }
  return logstring;
}

// Data file truncator
// This routine is designed to be called regularly from loop()
// Every 30 mins it will check the data.csv file and if over the 
// trigger size threshold will initiate a truncate operation.
// The truncate operation copies the last "desiredsize" from the
// file, (512B at a time - each time called) to a new file, and when
// complete renames the new file over the old file
long checktruncate(int truncatetrigger, int desiredsize) {
  static long cdp=-1;
  static unsigned long nextcheck=millis()+30*60000;
  
  if (cdp>=0) {
    char obuf[520];
    File dfile=SPIFFS.open("/data.csv","r");
    File ofile=SPIFFS.open("/data.new","a");
    dfile.seek(cdp,SeekSet);
    int bread = dfile.readBytes(obuf,512);
    if ( ofile.write(obuf,bread)>0 ) cdp=dfile.position();
    Serial.print(".");
    if ( dfile.available()==0 ) {
      datafilesize=ofile.size();
      Serial.printf("(%d)\n",datafilesize);
      dfile.close(); ofile.close(); cdp=-1; 
      SPIFFS.remove("/data.csv");
      SPIFFS.rename("/data.new", "/data.csv");
      Serial.println("Truncate completed");
    } else {
      datafilesize=dfile.size();
      dfile.close(); ofile.close();
    }
  } else {
    if ( millis()>nextcheck ) {
      nextcheck=millis()+30*60000; //Schedule next check
      File dfile=SPIFFS.open("/data.csv","r"); datafilesize=dfile.size();
      Serial.printf("Checking data.csv size (%d) ....",datafilesize);
      if ( datafilesize>truncatetrigger ) {
        dfile.seek(desiredsize,SeekEnd);
        String s=dfile.readStringUntil('\n');
        cdp=dfile.position(); 
        Serial.println("Too Big. Starting truncate");
      } else {
        Serial.println("OK.");
      }
      dfile.close();
    }
  }
  return cdp;
  
}
 
// Called from web server. Just reboots.
void reboot8266() {
  if (!isauthenticated()) { server.send(200, "text/plain", "Not logged in"); return; }
  server.send(200, "text/plain", "8266 rebooted. ");
  delay(1000);
  ESP.reset();
}

// Called from web server. Displays log string
void displog() {
  if (!isauthenticated()) { server.send(200, "text/plain", "Not logged in"); return; }
  server.send(200, "text/plain", addlog("") );
}

// Called from web server. Wipes data.csv
void apiwipegraph() {
  if (!isauthenticated()) { server.send(200, "text/plain", "Not logged in"); return; }
  File wipefile = SPIFFS.open("/blank.new","w");wipefile.close();
  SPIFFS.remove("/data.csv");
  SPIFFS.rename("/blank.new", "/data.csv");
  datafilesize=0;
  server.sendHeader("Location","/graph.htm");
  server.send(303);
}

// Called from web server. Sets Steinhart-hart values for probe1
void useprobe1() {
  if (!isauthenticated()) { server.send(200, "text/plain", "Not logged in"); return; }
  p1sharta=1.98592665e-03;
  p1shartb=6.68028033e-05;
  p1shartc=9.48003051e-07;
  p2sharta=1.98592665e-03;
  p2shartb=6.68028033e-05;
  p2shartc=9.48003051e-07;  
  parametersNeedSaving = true;
  server.sendHeader("Location","/advanced.htm");
  server.send(303);
}

// Called from web server. Sets Steinhart-hart values for probe2
void useprobe2() {
  if (!isauthenticated()) { server.send(200, "text/plain", "Not logged in"); return; }
  p1sharta=7.92162042e-04;
  p1shartb=2.07973585e-04;
  p1shartc=1.16208783e-07;
  p2sharta=7.92162042e-04;
  p2shartb=2.07973585e-04;
  p2shartc=1.16208783e-07;
  parametersNeedSaving = true;
  server.sendHeader("Location","/advanced.htm");
  server.send(303);  
}

// Called from web server. Turns on/off PID
void apitogglepid() {
  if (!isauthenticated()) { server.send(200, "application/json", "{ \"authenticated\":false }"); return; }
  bool pidisrunning = mainPID.toggleRunning();
  parametersNeedSaving = true;
  //Dont just send everything - Try composing dedicated JSON response just for the pidrunning response
  //apigetparms();
  if (pidisrunning) {
    server.send(200, "application/json", "{ \"pidrunning\":true }");
  } else {
    server.send(200, "application/json", "{ \"pidrunning\":false }");
  }
}

// Called from web server. This should only be called when POST data is attached
void logindevice() {
  if (server.hasArg("username") && server.hasArg("password") && server.hasArg("md5password")) {
    if (server.arg("username") == "admin" &&  server.arg("md5password") == adminpass) {
      Serial.println( String(server.arg("md5password")) );
      server.sendHeader("Location", "/");
      server.sendHeader("Cache-Control", "no-cache");
      server.sendHeader("Set-Cookie", "ESPSESSIONID=1; Path=/; Max-Age=3000");
      server.send(301);
      Serial.println("Log in Successful");
      return;
    }
  } 
  logoutdevice();
  Serial.println("Log in Failed");    
}

bool isauthenticated() {
  if (server.hasHeader("Cookie")) { 
    String cookie = server.header("Cookie");
    //Serial.println("Auth test : "+cookie);
    if (cookie.indexOf("ESPSESSIONID=1") != -1) { return true; }
  }
  return false;  
}

void logoutdevice() {
  server.sendHeader("Location", "/loginpage.htm");
  server.sendHeader("Cache-Control", "no-cache");
  server.sendHeader("Set-Cookie", "ESPSESSIONID=0; Path=/");
  server.send(301);  
}

void apisetpass() {
  if (!isauthenticated()) { server.send(200, "application/json", "{ \"Result\":-100 }" ); return; }
  char tempbuf[512];
  StaticJsonDocument<1024> doc;
  if (server.args()==1 && server.argName(0)=="plain") {  
    DeserializationError error = deserializeJson(doc, server.arg(0));
    if (error) { Serial.print(F("deserializeJson() failed: ")); Serial.println(error.c_str()); server.send(200, "application/json", "{ \"Result\":-3 }" ); return; }
    String oldpass = doc["oldpass"];
    String newpass = doc["newpass"];
    if (oldpass==adminpass) { 
      adminpass=newpass; 
      Serial.println("Changed Password");
      parametersNeedSaving = true; 
      server.send(200, "application/json", "{ \"Result\":1 }" );
    } else {
      Serial.println("Password Change failed");
      server.send(200, "application/json", "{ \"Result\":0 }" );
    }
  } else {
    server.send(200, "application/json", "{ \"Result\":-2 }" );       //Response to the HTTP request    
  }
}

// Called from web server. This should only be called when POST data is attached
// JSON data in POST is interpreted as parameters that can be set with values
// eg: { "UTCoffset":3600,"targettemp":220 } means set the UTC offset to 3600
// and the target temp to 220
// Values that can be set are
// UTCoffset,targettemp,kP,kI,kD,tscale,p1sharta,p1shartb,p1shartc,p2sharta,p2shartb,p2shartc,tune,shift
// Reply returned in JSON is the original request with a "Result" key added.
// Non zero Result value indicates the number of parameters that failed to be set.
void apisetparms() {
  if (!isauthenticated()) { server.send(200, "application/json", "{ \"Result\":-100 }" ); return; }
  char tempbuf[512];
  int setresult=0;
  StaticJsonDocument<1024> doc;
  if (server.args()==1 && server.argName(0)=="plain") {
    //Serial.println(server.arg(0));
    DeserializationError error = deserializeJson(doc, server.arg(0));
    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      server.send(200, "application/json", "{ \"Result\":-1 }" );
      return;
    }
    JsonObject object = doc.as<JsonObject>();

    for (JsonObject::iterator it=object.begin(); it!=object.end(); ++it) {
      if (it->key()=="UTCoffset") { 
        long newutc = (long)( it->value().as<float>() );
        if (newutc>=-50000 && newutc<=50000) { timeClient.setTimeOffset(newutc); UTCtimeoffset=newutc; } else setresult++;
      }
      if (it->key()=="targettemp") { 
        if ( !mainPID.setTarget( it->value().as<double>()) ) setresult++;
      }
      if (it->key()=="pidinterval") { 
        long newui = (long)( it->value().as<float>() );
        if ( !mainPID.setUpdateInterval(newui) ) setresult++;
      }
      if (it->key()=="kP") { 
        if ( !mainPID.setkP( it->value().as<double>()) ) setresult++;
      }
      if (it->key()=="kI") { 
        if ( !mainPID.setkI( it->value().as<double>()) ) setresult++;
      }
      if (it->key()=="kD") { 
        if ( !mainPID.setkD( it->value().as<double>()) ) setresult++;
      }
      if (it->key()=="tscale") { 
        long newts = (unsigned int)( it->value().as<float>() );
        if (newts>0 && newts < 4) tscale = newts; else setresult++;
      }
      if (it->key()=="p1sharta") { 
        p1sharta = it->value().as<double>();
      }
      if (it->key()=="p1shartb") { 
        p1shartb = it->value().as<double>();
      }
      if (it->key()=="p1shartc") { 
        p1shartc = it->value().as<double>();
      }
      if (it->key()=="p2sharta") { 
        p2sharta = it->value().as<double>();
      }
      if (it->key()=="p2shartb") { 
        p2shartb = it->value().as<double>();
      }
      if (it->key()=="p2shartc") { 
        p2shartc = it->value().as<double>();
      }
      if (it->key()=="tune") { 
        tune = it->value().as<double>();
      }
      if (it->key()=="shift") { 
        shift = it->value().as<double>();
      }
    }
    
    doc["Result"]=setresult;
    if (serializeJson(doc, tempbuf) == 0) {
      Serial.println("Failed to serialze JSON");
      server.send(200, "application/json", "{ \"Result\":-1 }" );
    } else {
      server.send(200, "application/json", tempbuf );
      parametersNeedSaving = true;
      //Serial.println(tempbuf);
    }
  } else {
    server.send(200, "application/json", "{ \"Result\":-1 }" );       //Response to the HTTP request
  }
}

// Collects all parameters into a JSON string and writes it to a config file
// This file is read on reboot so a power failure should not interrupt operation
// Should only be called from loop() when global flag has been set
void saveparms() {
    String parms=collateparms();
    File confwritefile=SPIFFS.open("/config.new","w");
    confwritefile.printf("%s",parms.c_str());
    confwritefile.close();
    String myMAC = WiFi.macAddress();
    myMAC.replace(':', '-');
    String cfname = "/config" + myMAC + ".json";
    SPIFFS.remove(cfname.c_str());
    SPIFFS.rename("/config.new", cfname.c_str());
    parametersNeedSaving = false;      
}

// Called from web server. 
// Collects all parameters into a JSON string and writes it to web client
void apigetparms() {
  if (!isauthenticated()) { server.send(200, "application/json", "{ \"Result\":-100 }" ); return; }
  String parms;
  parms=collateparms();
  server.send(200, "application/json", parms);
}

// Called from saveparms, apigetparms
// Collates all parameters into JSON string
String collateparms() {
  char jsonbuf[768];
  String retval="{\"Result\":-1}";
  StaticJsonDocument<1024> doc;
  char tbuf[16];
  sprintf(tbuf,"%s\0",timeClient.getFormattedTime().c_str());
  doc["version"] = CUEKING_FIRMWARE_VERSION;
  doc["uptime"] = millis();
  doc["ipaddr"] = WiFi.localIP().toString();
  doc["ssid"] = WiFi.SSID();
  doc["mac"] = WiFi.macAddress();
  doc["dnsname"] = mdnsname;
  doc["UTCoffset"] = UTCtimeoffset;
  doc["temp1"] = temp1;
  doc["temp2"] = temp2;
  doc["tune"] = tune;
  doc["shift"] = shift;
  doc["currenttime"] = tbuf;
  doc["dfilesize"] = datafilesize;
  doc["pidname"] = mainPID.getPIDname();
  doc["pidrunning"] = mainPID.isRunning();
  doc["pidinterval"] = mainPID.getUpdateInterval();
  doc["targettemp"] = mainPID.getTarget();
  doc["lastpidtemp"] = mainPID.getLastResult();
  doc["pideot"] = mainPID.getErrOverTime();
  doc["fanpercent"] = mainPID.control;
  doc["kP"] = mainPID.getkP();
  doc["kI"] = mainPID.getkI();
  doc["kD"] = mainPID.getkD();
  doc["tscale"] = tscale;
  doc["p1sharta"] = p1sharta;
  doc["p1shartb"] = p1shartb;
  doc["p1shartc"] = p1shartc;
  doc["p2sharta"] = p2sharta;
  doc["p2shartb"] = p2shartb;
  doc["p2shartc"] = p2shartc;
  doc["pass"] = adminpass;
  if (serializeJson(doc, jsonbuf) == 0) {
    Serial.println("Failed to serialze parameters to JSON");
  } else {
    retval = jsonbuf;
  }  
  return retval;
}

// Averages 4 difference values from ADC0,1 and converts them to temp scale
double readtemp1(int usetempscale) {
  long adctot=0;
  double adjust,tshift;
  if (usetempscale==3) {adjust=tune;tshift=shift;} else {adjust=(tune*5)/9;tshift=(shift*5)/9;}
  for (int i=0;i<4;i++) { adctot += (1.0 * ads.readADC_Differential_0_1()); }
  double sadc=(adctot/4.0);
//  double sadc=ads.readADC_Differential_0_1();
  return convertadc(sadc,usetempscale,5.00,4150.0,p1sharta,p1shartb,p1shartc) - adjust + tshift;
}

// Averages 4 difference values from ADC2,3 and converts them to temp scale
double readtemp2(int usetempscale) {
  long adctot=0;
  double adjust,tshift;
  if (usetempscale==3) {adjust=tune;tshift=shift;} else {adjust=(tune*5)/9;tshift=(shift*5)/9;}
  for (int i=0;i<4;i++) { adctot += (1.0 * ads.readADC_Differential_2_3());  }
  double sadc=(adctot/4.0);
//  double sadc=ads.readADC_Differential_2_3();
  return convertadc(sadc,usetempscale,5.00,4150.0,p2sharta,p2shartb,p2shartc) + adjust + tshift;
}

// Converts ADC value to temp 
// adc is the value read from the ADS1115, tempscale is the temperature scale to use to return the temp (1=Kelvin,2=Centi,3=Faren)
// Vge is the voltage going into the voltage divider resistors (calculated if mesureing from Vdd instead of Gnd)
// Rref is the resistor value being used. Mesure for accuracy! sharta,b,c values are for the Steinhart-Hart value for the thermistor.
double convertadc(double adc, int tempscale, double Vge, double Rref, double sharta, double shartb, double shartc) {
  double Vref=Vge/6.144*32768;  // Vge is only used in mesure from ground method. 
  if (adc<0) {
    adc*=-1.0;  // adc hardware measures from ground. We just need to make it positive
  } else {
    //Vref=ads.readADC_SingleEnded(0);   // adc hardware measures from Vdd, so we have a value for Vdd and can calculate Vge on the fly
    Vref=26666.6666;
    Vge=Vref*0.0001875;
    adc=Vref-adc;  // adc hardware mesures from Vdd/Vref so we subtract from Vref to get an absolute value
  }
  double retval;
  double Vin=(adc/Vref)*Vge;
  double Rtherm=(Rref*Vin)/(Vge-Vin);
  //Serial.printf("ADC:%f, Vge:%.2f, Vref:%.2f, Rref:%.2f, Vin:%.2f, Rtherm:%.2f\n",adc,Vge,Vref,Rref,Vin,Rtherm);
  double lnR=log(Rtherm);
  double Kelvin = 1/(sharta + shartb*lnR + shartc*lnR*lnR*lnR);
  double Centi = Kelvin - 273.15;
  double Faren = 9.0*Centi/5.0 + 32;
  retval=Kelvin;
  if (tempscale==2) retval=Centi;
  if (tempscale==3) retval=Faren;
  return retval; 
}

// Temp conversion functions
double cel2far(double cel) {  return (cel*9.0/5.0) + 32.0; }
double cel2kel(double cel) {  return cel + 273.15; }
double far2cel(double far) {  return (far-32.0)*5.0/9.0; }
double far2kel(double far) {  return far2cel(far) + 273.15; }
double kel2cel(double kel) {  return kel - 273.15; }
double kel2far(double kel) {  return cel2far( kel2cel(kel) ); }

//void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
//  switch(type) {
//    case WStype_ERROR: 
//        Serial.printf("[%u] get error: %s\n", num, payload); 
//        break;
//    case WStype_DISCONNECTED:
//        Serial.printf("[%u] Disconnected!\n", num);
//        break;
//    case WStype_CONNECTED: {
//        IPAddress ip = webSocket.remoteIP(num);
//        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
//        // send message to client
//        webSocket.sendTXT(num, "Connected"); }        
//        break;
//    case WStype_TEXT:
//        //Serial.printf("[%u] get Text: %s\n", num, payload);
//        if(payload[0] == '#') {
//            // we get RGB data - decode rgb data
//            uint32_t rgb = (uint32_t) strtol((const char *) &payload[1], NULL, 16);
//
//            //analogWrite(LED_RED, ((rgb >> 16) & 0xFF));
//            //analogWrite(LED_GREEN, ((rgb >> 8) & 0xFF));
//            //analogWrite(LED_BLUE, ((rgb >> 0) & 0xFF));
//            webSocket.broadcastTXT(payload,7); 
//        }
//        break;
//  }
//}


// Only actually checks the filesystem every so often. (Currently rvery 2 hours)
// Returns number of instances of corruption found and fixed. 0 otherwise.
int checkSPIFFScorrupted() {
  int rc=0;
  int cfiles=0;
  static unsigned long lastchecked=0;
  char rbuf[8];
  
  unsigned long now=millis();
  if (now < lastchecked || (now-lastchecked) > 7140000) {
    lastchecked=now;
    addlog("Checking SPIFFS filesystem");
    Dir cdir = SPIFFS.openDir("/");
    while (cdir.next()) { 
      if (cdir.fileName().startsWith("/CORRUPT",0) ) {
        cfiles++;
        //addlog("\nFound Corruption marker file ("+String(cdir.fileSize())+" bytes)\n");
      }
    }    
    Dir dir = SPIFFS.openDir("/");

    while (dir.next()) {
      //Serial.println("Checking " + dir.fileName());
      addlog(".");
      if ( !dir.fileName().startsWith("/CORRUPT",0) ) {
        File fd = dir.openFile("r");
        if ( fd.available() > 0 ) {
          if ( fd.readBytes(rbuf,1)==0 ) { // Found Corruption. Fix it!
            String ofname,cfname,rfname;
            ofname=dir.fileName();
            addlog("\n"+ofname+" is corrupt! ("+String(fd.size())+" bytes)\n");
            cfname= String("/CORRUPT" + String(cfiles++,DEC));          
            if ( ofname.startsWith("/back",0) ){
              rfname = ofname.substring(6);
            } else {
              rfname="";
              if (SPIFFS.exists("/back"+ofname)) rfname="/back"+ofname;
            }
            addlog("\nRenaming " + ofname + " -> " + cfname +"\n");
            SPIFFS.rename( ofname, cfname );
            if (rfname.length()==0) {
              addlog("\nCan't recreate this file - Sorry. Was it important?\n");  
            } else {
              char obuf[520];
              Serial.println("\nRecreating "+ofname+" from "+rfname+"\n");
              File rfile=SPIFFS.open(rfname,"r");
              File ofile=SPIFFS.open("/RECREATE.new","a");
              while (rfile.available() > 0 ) {
                int bread = rfile.readBytes(obuf,512);
                ofile.write(obuf,bread);
              }
              rfile.close();
              ofile.close();
              SPIFFS.rename( "/RECREATE.new", ofname );
            }
            rc++; 
          }
        }
        fd.close();
      } else {
        addlog("!");
      }
    }
  addlog("Done!\n");
  }
  return rc;  
}

// Following functions are called "every so often" Only currently used to get the 
// network time every 30 minutes, and also to reboot if we run out of heap space

void every5sec() {
}

void everyday() {
}

void everyhour() {
  static int myhour=0;
  if (++myhour>23) { myhour=0; everyday(); }
}

void everysecond() {
  static int mysec=0;
  if (++mysec>59) { mysec=0; everyminute(); }
  if (mysec%5==0) { every5sec(); }
}

void every30minutes() {
  Serial.print("Attempting to get time ... ");
  if ( timeClient.update() ) { Serial.println("Succeeded"); } else { Serial.println("Failed"); }  
}

void every5minutes() {
  //Serial.printf("Chip time is %02d:%02d:%02d\n",timeClient.getHours(), timeClient.getMinutes(), timeClient.getSeconds() );
}

void everyminute() {
  static int mymin=0;
  if (++mymin>59) { mymin=0; everyhour(); }
  if (mymin%30 == 0) { every30minutes(); }
  if (mymin%5 == 0) { every5minutes(); }
  if (ESP.getFreeHeap() < 16384) { Serial.println("Heap is low - Rebooting to fix"); ESP.reset(); } 
}

void loop() {
  static unsigned long last=0;
  unsigned long now;
  static double pidupdatetemp,piddftemp;
  static bool grabnewtemp=true;
  static unsigned long turnofffantime=(millis()-100);

//  server.handleClient();
//  MDNS.update();
//  ArduinoOTA.handle();
  
  now=millis();
  if ( (now - last) >= 1000 || now < last) { digitalWrite(led, 0); everysecond(); grabnewtemp=true; last=now; }
  if ( (now - last) >= 25 ) { digitalWrite(led, 1); }
  if ( now >= turnofffantime ) { digitalWrite(fan, 0); } // Turn off fan if it is time.

  // Monitors data file data.csv and if it is above 160K truncates it to 128K
  checktruncate(160*1024,128*1024);

  //Grabs current temp readings every second 
  if ( (now - last) >= 300 && grabnewtemp==true ) { 
    temp1=readtemp1(tscale);    temp2=readtemp2(tscale); // Warning readtemp takes a while! 
    grabnewtemp=false;
    pidupdatetemp=temp1;
    if (tscale==1) pidupdatetemp=kel2far(temp1); if (tscale==2) pidupdatetemp=cel2far(temp1);
    piddftemp=temp2;
    if (tscale==1) piddftemp=kel2far(temp2); if (tscale==2) piddftemp=cel2far(temp2);
  }  
  

  // PID will only updates when its time. Most of the time the update method returns 
  // instantly with a value of false.
  if (mainPID.update(pidupdatetemp))  { 
    // If PID updates we see if fan needs to be turned on, and also calculate when to turn it off.
    if (mainPID.control > 0.0) {
      digitalWrite(fan, 1); // Turn on fan, and calculate off time.
      turnofffantime=millis() + (mainPID.control/100.0) * mainPID.getUpdateInterval();
    }
    // Set parameter save flag also
    parametersNeedSaving = true; 
    // Write PID log to data.csv file
    File appendfile=SPIFFS.open("/data.csv","a");
    int sizebeforeappend = appendfile.size();
    appendfile.printf("%s,%.2f,%s\n",timeClient.getFormattedTime().c_str(),piddftemp,mainPID.datalogbuffer);
    datafilesize=appendfile.size();
    appendfile.close();

  }

  // Save running parameters in case of power fail or reboot
  if (parametersNeedSaving) saveparms();

  // Only actually runs every so often.....most of the time it returns immedietly
  checkSPIFFScorrupted();

  // Service Webserver, microDNS, and "OverTheAir" updating functionality (See libs...)
  //webSocket.loop();
  server.handleClient();
  MDNS.update();
  ArduinoOTA.handle();


  // Good practice
  yield();
}
