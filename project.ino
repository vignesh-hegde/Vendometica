//////////////////////////////////
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Servo.h>
//////////////////////////////////
#define water_Level 0
#define water_pump 1
#define Temperature_sensor 2
#define rasp_start 3
#define rasp_state 4 
#define ultrasonic_echoPin_coffee 5
#define ultrasonic_trigPin_coffee 6
#define ultrasonic_echoPin_milk 7
#define ultrasonic_trigPin_milk 8
#define servo_milk 9
#define servo_coffee 10
#define flow_sensor 11
#define IR_sensor 12
#define rasp_finish 13
#define Heater A0
#define despence_pump A1
#define LED_water A3
#define LED_coffee A4
#define LED_milk A5
/////////////////////////////////
const float max_temperature = 80.00;  // in degree Celcius
const int min_coffee_level =  5;      // in cm
const int min_milk_level =  5;        // in cm
const int time_for_hot_water = 10;      // in sec
const int max_water_voulme = 10;     // in mL
/////////////////////////////////
long duration; 
int distance; 
//////////////////////////////////
OneWire oneWire(Temperature_sensor); 
DallasTemperature sensors(&oneWire);

Servo servoCoffee;
Servo servoMilk; 
/////////////////////////////////

void setup() 
{ 
	Serial.begin(9600); 
	sensors.begin(); 

	pinMode(water_Level,INPUT);	
	pinMode(water_pump,OUTPUT);
  	pinMode(Temperature_sensor,INPUT);
	pinMode(rasp_start,INPUT);	
	pinMode(rasp_state,OUTPUT);
	pinMode(ultrasonic_echoPin_coffee, INPUT);
  	pinMode(ultrasonic_trigPin_coffee, OUTPUT); 
  	pinMode(ultrasonic_echoPin_milk, INPUT);
	pinMode(ultrasonic_trigPin_milk, OUTPUT); 
	pinMode(flow_sensor,INPUT);
	pinMode(rasp_finish,OUTPUT);
	
	pinMode(Heater,OUTPUT);
	pinMode(despence_pump,OUTPUT);
	pinMode(LED_water,OUTPUT);
	pinMode(LED_coffee,OUTPUT);
	pinMode(LED_milk,OUTPUT);
	

	servoMilk.attach(servo_milk);
	servoCoffee.attach(servo_coffee);

} 

float get_temperature()
{
	sensors.requestTemperatures(); 
	return sensors.getTempCByIndex(0);
}

bool check_water_level()
{
	return !digitalRead(water_Level);
}

bool check_coffee_level()
{
	digitalWrite(ultrasonic_trigPin_coffee, LOW);
  	delayMicroseconds(2);
	digitalWrite(ultrasonic_trigPin_coffee, HIGH);
	delayMicroseconds(10);
	digitalWrite(ultrasonic_trigPin_coffee, LOW);
	duration = pulseIn(ultrasonic_echoPin_coffee, HIGH);
	distance = duration * 0.034 / 2; 
	
	return distance < min_coffee_level;
	
}

bool check_milk_level()
{
	digitalWrite(ultrasonic_trigPin_milk, LOW);
  	delayMicroseconds(2);
	digitalWrite(ultrasonic_trigPin_milk, HIGH);
	delayMicroseconds(10);
	digitalWrite(ultrasonic_trigPin_milk, LOW);
	duration = pulseIn(ultrasonic_echoPin_milk, HIGH);
	distance = duration * 0.034 / 2; 

	return distance < min_milk_level;
}

void turn_on_coffee_valve()
{
	int angle = 0;

	for (angle = 0; angle<180; angle++)
	{
		servoCoffee.write(angle);
	} 
}

void turn_on_milk_valve()
{
	int angle = 0;

	for (angle = 0; angle<180; angle++)
	{
		servoMilk.write(angle);
	}
}

void how_water_out()
{
	while(digitalRead(IR_sensor)==LOW);

	digitalWrite(despence_pump,HIGH);
	delay(time_for_hot_water);
	digitalWrite(despence_pump,LOW);

}

int get_voulme()
{
	/*
		return volume by flow sensor;
	*/
}

bool check_IR()
{
	return digitalRead(IR_sensor);
}

bool initialize()
{	bool t1 = check_water_level();
	bool t2 = check_coffee_level();
	bool t3 = check_milk_level();

	digitalWrite(LED_water,t1);
	digitalWrite(LED_coffee,t2);
	digitalWrite(LED_milk,t3);

	if(t1 && t2 && t3 )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void despence()
{
	if(initialize() == 0)
	{
		digitalWrite(rasp_state,LOW);
		digitalWrite(rasp_finish,LOW);
		delay(1000);
	}
	else
	{
		digitalWrite(rasp_state,HIGH);
		delay(1000);
		
		while(get_voulme() < max_water_voulme)
		{
			digitalWrite(water_pump,HIGH);
		}
		digitalWrite(water_pump,LOW);

		while(get_temperature() < max_temperature)
		{
			digitalWrite(Heater,HIGH);
		}
		digitalWrite(Heater,LOW);

		if(check_IR()==1)
		{
			turn_on_coffee_valve();
			turn_on_coffee_valve();
			how_water_out();
		}
		
		digitalWrite(rasp_finish,HIGH);
		delay(1000);
	}	
}
int d = 1;
void loop() 
{ 
  if(d==1)
  {
    d = 0;
    delay(1000);
    }
	/*
	digitalWrite(rasp_finish,LOW);
	digitalWrite(rasp_state,LOW);
	
	if( digitalRead(rasp_start) == HIGH )
	{
		despence();
	}
	*/
int temp = get_temperature();
Serial.println(temp);
  delay(10);
  if(temp>40)
  {
    digitalWrite(rasp_state,LOW);
  }
  else{
    digitalWrite(rasp_state,HIGH);
    }
    
	
} 
