#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 12 //D6
#define PUMPIN 10       //SD3
#define PUMPOUT 14       //D5
#define HEATER 16       //D0
#define LEVEL 2        //D4
#define IR 13           //D7


// D1 SDL and D2 SDA for 16 x 2 LCD Display

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(0x27, 16, 2);

const char *WIFI_SSID = "Redmi Note 7 Pro";                   // Username of Wifi
const char *WIFI_PASSWORD = "rn7p2022";                       // Password of Wifi
const char *URL = "http://vignesh4a.pythonanywhere.com/data";
const long utcOffsetInSeconds = 19800;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "in.pool.ntp.org", utcOffsetInSeconds, 60000);
WiFiClient client;
HTTPClient httpClient;
 
void setup()
{
    Serial.begin(9600);
    pinMode(PUMPIN,OUTPUT);
    pinMode(PUMPOUT,OUTPUT);
    pinMode(HEATER,OUTPUT);
    pinMode(LEVEL,INPUT);
    pinMode(IR,INPUT);

    
    digitalWrite(PUMPIN,HIGH);
    digitalWrite(PUMPOUT,HIGH);
    digitalWrite(HEATER,HIGH);
    
    ESP.wdtDisable();
    *((volatile uint32_t*) 0x60000900) &= ~(1);
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    lcd.begin();
    lcd.backlight();
    lcd.setCursor(3, 0);
    lcd.print("VENDOMETICA");
    delay(2000);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
 
    Serial.println("Connected");
    Serial.println("");
    Serial.println("WiFi connected");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi connected");
    lcd.setCursor(0,1);
    lcd.print(WiFi.localIP());
    Serial.println(WiFi.localIP());
    timeClient.begin();
    delay(2000);

}

int temp()
{
    sensors.requestTemperatures(); 
    return sensors.getTempCByIndex(0);  
}

int waterin()
{
    Serial.println(">>>>>> Water In Start <<<<<<<");
    int t = 0;
    int val = digitalRead(LEVEL);
    Serial.println(val);
    while(val==1)
    {
      val = digitalRead(LEVEL);
      Serial.println(digitalRead(LEVEL));
      digitalWrite(PUMPIN,LOW);  
      delay(1000);
      if(t++ > 10)
      {
        digitalWrite(PUMPIN,HIGH);  
        return 0;
      }
    }
     digitalWrite(PUMPIN,LOW);  
     delay(100);
     Serial.print("LEVEL is");
     digitalWrite(PUMPIN,HIGH);
     Serial.println(digitalRead(LEVEL));

     Serial.println(">>>>>> Water In Stop <<<<<<<");
     return 1;
}

int heat()
{
  Serial.println(">>>>>> Heat Start <<<<<<<");
  
  delay(500);
  int x = digitalRead(LEVEL);
  delay(500);

  if(x==0)
  {
    Serial.println(temp());
    delay(500);
    
      while(temp()<40)
      {
        digitalWrite(HEATER,LOW); 
        Serial.println(temp());
        delay(500);
      }
      
      digitalWrite(HEATER,HIGH);
      delay(500);

      Serial.println(">>>>>> Heat Stop with return 1 <<<<<<<");
      return 1;
  }
  digitalWrite(HEATER,HIGH);
  Serial.print("STOPPED >>  ");
  Serial.println(x);
  Serial.println(">>>>>> Heat Stop with return 0 <<<<<<<");
  return 0;
}

int waterout()
{
    Serial.println(">>>>>> water out Start <<<<<<<");
    while(digitalRead(IR)==1)
    {
        delay(1000);
    }
      digitalWrite(PUMPOUT,LOW);
      delay(4000);
      digitalWrite(PUMPOUT,HIGH);  
    Serial.println(">>>>>> water out Stop <<<<<<<");
    return 1;
}

void s()
{
    Serial.println(temp());
  delay(1000);

    Serial.println("xxxxxxxxxxxxxxxTRIGgeredXXXXXXXXXXX");
    int data = waterin();
    Serial.println(data);
    if(data==1)
    {
        if(heat()==1)
        {
          waterout();
        }
    }  
    Serial.println(">>>>>>>>>>>stop TRIGgered<<<<<<");
}

void loop()
{
    digitalWrite(PUMPIN,HIGH);
    digitalWrite(PUMPOUT,HIGH);
    digitalWrite(HEATER,HIGH);
    
    httpClient.begin(client, URL);
    httpClient.GET();
    String content = httpClient.getString();

    if(content.equals("0"))
    {
        timeClient.update();  
        lcd.clear();
        lcd.setCursor(3, 0);
        lcd.print("VENDOMETICA");     
        lcd.setCursor(6,1);
        lcd.print(timeClient.getFormattedTime().substring(0,5));
        Serial.println(timeClient.getFormattedTime().substring(0,5));  
    }
    else
    {
        Serial.println(content);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Payment Received");

        s();
        
        delay(3000);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Thank u 4 Using");
        lcd.setCursor(3, 1);
        lcd.print("VENDOMETICA"); 
        delay(5000);    
    }
    
    httpClient.end();
    delay(1000);
}
