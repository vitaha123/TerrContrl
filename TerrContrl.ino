//////////////////////////////////////////////////////////////////////////
//Время включения и выключения
//1. проверка времени каждые 10 милисикунд,
//2. проверка датчиков температуры каждые 500 милисекунд,
//3. по данным с датчиков включать или отключать реле, 
//функции - 1. часы- если время ВЫКЛЮЧЕНИЯ( _offPoint )
// < времени ВКЛЮЧЕНИЯ (_onPoint) прибавляем к общему времени работы (_workTimeState) 24 часа.
//время работы(_workTimeState) - проверяется при включении и сверяемся со временем выключения 
//(_offPoint) - 
//////////////////////////////////////////////////////////////////////////
//Все данные вынесем в перерменные
//////////////////////////////////////////////////////////////////////////



#include <Sodaq_DS3231.h>
#include <DHT.h>


DHT dht_1(22, 22), //MidSensor
	dht_2(23, 22), //GroundSensor
	dht_3(24, 22); //topSensor
Sodaq_DS3231 _time;


#define _DAY 86400000
#define _HOUR 3600000
#define _MINUTE 60000
#define _SECOND 1000                                                                          

const uint8_t HEATING_pin(29); // пин обогрева ламп и пола
const uint8_t LIGHT1_pin(30); // пин освещения
const uint8_t LIGHT2_pin(31); // пин освещения
const uint8_t FAN_pin(32); // пин Внетелятора
const uint8_t HUMID_pin(33); // пин влажности

//const uint8_t HEATING_pin(3); // пин обогрева ламп и пола
//const uint8_t LIGHT1_pin(5); // пин освещения
//const uint8_t LIGHT2_pin(6); // пин освещения
//const uint8_t FAN_pin(9); // пин Внетелятора
//const uint8_t HUMID_pin(10); // пин влажности

#define DO_EVERY(interval, action)\
{                                 \
  static unsigned long t = 0;     \
  if( millis() - t > interval )   \
  {                               \
    { action; }                   \
    t = millis();                 \
  }                               \
}

		void setup()
		{
			Serial.begin(115200);


			_time.begin();

			delay(200);
			//_time.setDateTime(DateTime((__DATE__), (__TIME__)));
			pinMode(HEATING_pin, OUTPUT); // пин обогрева ламп и пола
			pinMode(LIGHT1_pin, OUTPUT); // пин освещения
			pinMode(LIGHT2_pin, OUTPUT); // пин Внетелятора
			pinMode(FAN_pin, OUTPUT); // пин влажности
			pinMode(HUMID_pin, OUTPUT); //pinMode(36, OUTPUT);
		}


		//среднее зачение влажности
		float MidHumid()
		{
			float MIDHUMI = 0;

			int col = 0;
			if (dht_1.readHumidity() > 50)
			{
				MIDHUMI += dht_1.readHumidity();
				col++;
			}
			if (dht_2.readHumidity() > 50)
			{
				MIDHUMI += dht_2.readHumidity();
				col++;
			}
			if (dht_3.readHumidity() > 50)
			{
				MIDHUMI += dht_3.readHumidity();
				col++;
			}
			if (col == 0) return 0;
			return MIDHUMI / col;
		}

		//среднее значение температуры
		float MidTemp()
		{
			float MIMTEMP = 0;

			int col = 0;
			if (dht_1.readTemperature() > 20)
			{
				MIMTEMP += dht_1.readTemperature();
				col++;
			}
			if (dht_2.readTemperature() > 20)
			{
				MIMTEMP += dht_2.readTemperature();
				col++;
			}
			if (dht_3.readTemperature() > 20)
			{
				MIMTEMP += dht_3.readTemperature();
				col++;
			}
			if (col == 0) return 0;
			return MIMTEMP / col;
		}

		// структура установки времени освещения
		struct SetuPtIME
		{
			unsigned char hour;
			unsigned char minute;
			unsigned char second;
		};

		SetuPtIME onTime1 = {8,0,0};      //бок свет
		SetuPtIME offTime1 = {23,00,0};
		SetuPtIME onTime2 = {9,0,0};      //средн свет
		SetuPtIME offTime2 = {22,0,0};
		SetuPtIME onTime3 = {10,0,0};     //гор лампы
		SetuPtIME offTime3 = {21,0,0};


		// функция реле времени
		void timerFoo(struct SetuPtIME on, struct SetuPtIME off, uint8_t pin)
		{
			long StartW = on.hour * _HOUR + on.minute * _MINUTE <= _time.now()
				.hour() * _HOUR + _time.now().minute() * _MINUTE;

			long Wtime = ((off.hour * _HOUR + off.minute * _MINUTE) - (_time.now()
				.hour() * _HOUR + _time.now().minute() * _MINUTE)) > 0 ? 1 : 0;

			if (Wtime == true && StartW == true)
			{
				digitalWrite(pin, true);
			}
			else
			{
				digitalWrite(pin, false);
			}
		}

		//функция ламп/пола обогрева
		void Heter_lamp_flor()
		{
			if (MidTemp() < 25)
			{
				digitalWrite(HEATING_pin, true);
			}
			else if (MidTemp() > 28)
			{
				digitalWrite(HEATING_pin, false);
			}
		}


		// Реле включения увлажнения
		void humid_reley()
		{
			if (MidHumid() < 80)
			{
				digitalWrite(HUMID_pin, true);
			}
			else if (MidHumid() >= 95)
			{
				digitalWrite(HUMID_pin, false);
			}
		}

		// вытяжка переписать
		void Fan_reley()
		{
			if (MidTemp() > 32 || MidHumid() > 70)
			{
				digitalWrite(FAN_pin, true);
			}
			else if (MidTemp() < 50 || MidHumid() < 25)
			{
				digitalWrite(FAN_pin, false);
			}
			/*Serial.print("temp");
			Serial.println(mid_temp);*/
		}

		//вывод на экран
		void showS()
		{
			Serial.println("======================================");
			Serial.print("MidSensor humid = ");
			Serial.println(dht_1.readHumidity());
			Serial.print("GroundSensor humid = ");
			Serial.println(dht_2.readHumidity());
			Serial.print("topSensor humid = ");
			Serial.println(dht_3.readHumidity());
			Serial.println("======================================");
			Serial.print("MidSensor = temperature ");
			Serial.println(dht_1.readTemperature());
			Serial.print("GroundSensor = temperature ");
			Serial.println(dht_2.readTemperature());
			Serial.print("topSensor = temperature ");
			Serial.println(dht_3.readTemperature());
			Serial.println("======================================");
			String trr;
			_time.now().addToString(trr);
			Serial.println(trr);
			Serial.println(MidHumid());
			Serial.println(MidTemp());
			Serial.print("Temp from clock:  ");
			Serial.println(_time.getTemperature());
			Serial.println("======================================");
		}


		/*void EEPROMWorck()
		{
		}*/

		void loop()
		{
			//Лампы освещение
			DO_EVERY(1000, timerFoo(onTime1, offTime1, LIGHT1_pin));
			DO_EVERY(1000, timerFoo(onTime2, offTime2, LIGHT2_pin));

			DO_EVERY(1003, timerFoo(onTime3, offTime3, HEATING_pin));
			//Лампы обогрева
			//DO_EVERY(1000, Heter_lamp_flor());
			//Влажность
			DO_EVERY(1000, humid_reley());
			//Вытяжка
			DO_EVERY(10000, Fan_reley());
			//вывод функций
			DO_EVERY(3000, showS());
		}


		//1. проверка времени каждые 10 милисикунд


