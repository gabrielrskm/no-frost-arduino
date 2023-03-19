#include <Arduino.h>
#include <TimerOne.h>
//uso de la libreria TimerOne la cual me brinda un objeto  el cual
//me permite con simplemente inicializarlo utilizar interrupciones por tiempo
#include <math.h>

//////////////////////////////////////////////////

  const int Rc = 10950; //valor de la resistencia
  const int Vcc = 5;
  const int SensorPIN = A2;
  float A = 1.11492089e-3;
  float B = 2.372075385e-4;
  float C = 6.954079529e-8;
  float K = 2.5; //factor de disipacion en mW/C
  float raw,V,Rt,R = 0;
  
///////////////////////////////////////

  int motor = 2;//verde IN4
  int ventilador = 4;//celeste-blanco IN3
  int resistencia = 6;//naranja-blanco IN1
  int nada = 8;
  int ON= LOW;
  int OFF= HIGH;
//////////////////////////////////////

  int estadoCompresor = false;
  int counterSecond = 0;

  bool cycleCompresor = true;     //5 minutos
  bool cycleVentilador = false;   //8 horas 
  bool cycleResintencia = false;  // 20 minutos
  bool cycleEspera = false;       // 2 minutos

  const int constResistencia = 1500; // 25 minutos
  const int constVentilador = 28800;//8 horas

  
  int timerFreezer = constVentilador;//8 horas
  bool flagDescongelador = false;//flag que determina si se enciende el descongelador
  bool flagActivarDescongelador = false;//flag que en true si o si enciende el descongelador

//////////////////////////////////////

float temperatura()
{
  
      Rt=0;
      for(int y=0;y<40;y++)
      {
          raw = analogRead(SensorPIN);
          V =  raw / 1024 * Vcc;
          Rt += (Rc * V ) / (Vcc - V);
          delay(10);
      }
      R=(Rt/40);
      float logR  = log(R);
      float R_th = 1.0 / (A + B * logR + C * logR * logR * logR );
     
      float kelvin = R_th - V*V/(K * R)*1000;
      float cel= kelvin - 273.15;
      return cel;
}

//el contador se activa cada un segundo y cada cuatro horas activara el descongelador
void timer1_isr()
{
  counterSecond++;
  
  //compara el contador y verifica que se cumpla la condicion
  //si coincide tanto con 20 minutos u 8 horas se activa el
  //manejador de tiempos

  if(counterSecond == timerFreezer)
  {
    counterSecond = 0;
    //si flagDescongelador es false quiere decir que es hora de 
    //empezar el proceso de descongelado de 20 minutos
    if(flagDescongelador == false){
      flagDescongelador = true;
      timerFreezer = constResistencia;
      }
    //si flagDescongelador es true quiere decir que es hora de 
    //empezar el proceso de congelado de 8 horas

    else {
      flagDescongelador = false;
      timerFreezer = constVentilador;
      }
    
  }
}

void onResistencia()
{
        digitalWrite(motor,OFF);
        float temp = temperatura();
        Serial.print("Resistencia prendida, temperatura :");
        Serial.println(temp);
        digitalWrite(LED_BUILTIN,0);
        while(flagDescongelador || flagActivarDescongelador)
        {
            digitalWrite(motor,OFF);
            digitalWrite(ventilador,OFF);
            digitalWrite(resistencia,ON);
            temp = temperatura();
            Serial.print("Resistencia funcionando, temperatura  : ");
            Serial.println(temp);
            digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN)); 
            delay(1000);
            if(temp > 10){
              flagDescongelador = false;
              timerFreezer = constVentilador;
              Serial.println("Temperatura mayor a 10 grados");
            }
        }
        digitalWrite(LED_BUILTIN,0);
        digitalWrite(motor,OFF);
        digitalWrite(ventilador,OFF);
        digitalWrite(resistencia,OFF);
        Serial.println("Resistencia apagada");
        delay(100000);
        return;
}

void setup() 
      {
          pinMode(LED_BUILTIN, OUTPUT);
          pinMode(motor,OUTPUT);
          pinMode(ventilador,OUTPUT);
          pinMode(resistencia,OUTPUT);
          pinMode(nada,OUTPUT);
          pinMode(13,OUTPUT);
          digitalWrite(motor,OFF);
          digitalWrite(ventilador,ON);
          digitalWrite(resistencia,OFF);
          digitalWrite(nada,OFF);
          Serial.begin(9600);
          digitalWrite(13,LOW);
          Timer1.initialize(1000000);  // 1segundo
          Timer1.attachInterrupt(timer1_isr);
      }

void loop() {                             
      //Lectura del termistor
      float celsius = temperatura();

      if(flagDescongelador){
        onResistencia();
      }
      //manejo de encendido y apagado del motocompresor
      if (celsius > -10 ){

          digitalWrite(motor,ON);
          digitalWrite(LED_BUILTIN,1);
          digitalWrite(ventilador,ON);
          digitalWrite(resistencia,OFF);
          estadoCompresor = true;
        }
      if (celsius < -20 ){

          digitalWrite(motor,OFF);
          digitalWrite(LED_BUILTIN,0);
          digitalWrite(resistencia,OFF);
          digitalWrite(ventilador,ON);
          estadoCompresor  = false;         
        }

      delay(10000);
      Serial.print("Temperatura : ");
      Serial.println(celsius);
      Serial.print("estado del motocompresor : ");
      Serial.println(estadoCompresor);
}