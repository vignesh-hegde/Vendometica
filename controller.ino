#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <OneWire.h>
#include <DallasTemperature.h>


#define HEATER 16       //D0
#define PUMPOUT 2       //D4
#define LEVEL 14        //D5
#define ONE_WIRE_BUS 12 //D6
#define IR 13           //D7
#define Stepr 15        //D8
#define PUMPIN 10       //SD3


                        // D1 SDL and D2 SDA for 16 x 2 LCD Display

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(0x27, 16, 2);

const char *WIFI_SSID = "<HOTSPOT_NAME>";                   	// Username of Wifi
const char *WIFI_PASSWORD = "<PASSWORD>";                       // Password of Wifi
const char *URL_RETRIVE = "http://vignesh4a.pythonanywhere.com/<REYRIVE_ENDPOINT>";
const char *URL_REFUND = "http://vignesh4a.pythonanywhere.com/<REFUND_ENDPOINT>";
const long utcOffsetInSeconds = 19800;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "in.pool.ntp.org", utcOffsetInSeconds, 60000);
WiFiClient client;
HTTPClient httpClient_retrive;
HTTPClient httpClient_refund; 

void setup()
{

    pinMode(PUMPIN,OUTPUT);
    pinMode(PUMPOUT,OUTPUT);
    pinMode(HEATER,OUTPUT);
    pinMode(LEVEL,INPUT);
    pinMode(IR,INPUT);
    pinMode(Stepr,OUTPUT);

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
    
    while (WiFi.status() != WL_CONNECTED) 
    {
    	delay(1000);
        lcd.setCursor(0, 1);
        lcd.print("Connecting...");
    }
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi connected");
    lcd.setCursor(0,1);
    lcd.print(WiFi.localIP());
    timeClient.begin();
    delay(2000);
}

String getid(String data)
{
	int len = 0;
	while (data[++len]);
	return data.substring(1, len - 1);
}

char getamount(String data)
{
  	int len = 0;
  	while (data[++len]);
  	return data[len - 1];
}

int temp()
{
    sensors.requestTemperatures();   
    return sensors.getTempCByIndex(0);
}

int waterin()
{

	int i;
	int limit_seconds = 10;

	digitalWrite(PUMPIN,digitalRead(LEVEL) == 1 ? LOW : HIGH );

	limit_seconds = (limit_seconds*1000)/500; 

	for(i=0;i<=limit_seconds;i++)
	{
		if( digitalRead(LEVEL) == 0 )
		{
			digitalWrite(PUMPIN,HIGH);
			return 0;
		}
		delay(500);
	}

	digitalWrite(PUMPIN,HIGH);          
	return 1;
     
}

void heat()
{

	temp();
	delay(500);
	temp();
	delay(500);

	int req = 40;
	
	if( temp() < 30 )
		req = 45;
	else if( temp() < 35 )
		req = 47;
	else if( temp() < 40 )
		req = 50;
	else if( temp() < 45 )
		req = 53;
	else if( temp() < 50 )
		req = 55;
	else if( temp() < 55 )
		req = 57;
		
	while(temp() < req)
	{
		digitalWrite(HEATER,LOW); 
		delay(500);
	}
		
	digitalWrite(HEATER,HIGH);
	delay(500);
  
}

void waterout(char amount)
{
	int half_cup_delay = 2200;
	int full_cup_delay = 3750;

	lcd.clear();
	lcd.setCursor(4, 0);
	lcd.print("Place The Cup");

	while(digitalRead(IR)==1)
	{
		delay(1000);
	}
	
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("Enjoy ur coffee.");
	
	digitalWrite(PUMPOUT,LOW);
	delay( amount == '1' ? half_cup_delay : full_cup_delay );
	digitalWrite(PUMPOUT,HIGH);  
}

void powerOut()
{
    int ste = 2000;

     for(int x = 0; x < ste; x++)
    {
        digitalWrite(Stepr,HIGH); 
        delayMicroseconds(1000); 
        digitalWrite(Stepr,LOW); 
        delayMicroseconds(1000); 
    }
}

int start(char amount)
{
	temp();
	delay(500);

	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("Preparing Coffee...");
	
	if(waterin() == 0)
	{
		heat();
		powerOut();
		waterout(amount);
		return 0;
	}
	return 1;
}

void refund(String id)
{
	httpClient_refund.begin(client, URL_REFUND+id);
	httpClient_refund.GET();
	httpClient_refund.end();
	
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("Error!");
	lcd.setCursor(0, 1);
	lcd.print("refund issued");  
}

void loop()
{
    digitalWrite(PUMPIN,HIGH);
    digitalWrite(PUMPOUT,HIGH);
    digitalWrite(HEATER,HIGH);
    
    httpClient_retrive.begin(client, URL_RETRIVE);
    httpClient_retrive.GET();
    String data = httpClient_retrive.getString();

    if( data[0] == '0' )
    {
        timeClient.update();  
        lcd.clear();
        lcd.setCursor(3, 0);
        lcd.print("VENDOMETICA");     
        lcd.setCursor(6,1);
        lcd.print(timeClient.getFormattedTime().substring(0,5));
    }
    else if( data[0] == '1' )
    {
		String id = getid(data);
		char amount = getamount(data);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Payment Received");
        lcd.setCursor(0, 1);
        lcd.print("ID:");
        lcd.setCursor(3, 1);
        lcd.print(id);
        delay(1000);
        
        if( start(amount) == 1 )
        {
          	refund(id);
        }
        
        delay(3000);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Thank u 4 Using");
        lcd.setCursor(3, 1);
        lcd.print("VENDOMETICA"); 
        delay(5000);    
    }
    
    httpClient_retrive.end();
    delay(1000);
}