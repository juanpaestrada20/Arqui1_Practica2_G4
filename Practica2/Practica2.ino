#include <Servo.h>
//Para sensor ultrasonico
#define Trigger 3
#define Echo 4

//Definicion de LLantas
/*
  LlantaIA -> Llanta Izquierda Adelante
  LlantaIR -> Llanta Izquierda Atras
  LLantaDA -> Llanta Derecha Adelante
  LlantaDR -> Llanta Derecha Atras
*/
#define LlantaIA 7
#define LlantaIR 6
#define LlantaDA 5
#define LlantaDR 4

//Definicion de los sensores
/*
  SIzq -> Sensor Izquierdo
  SCen -> Sensor Central
  SDer -> Sensor Derecho
*/
#define SIzq 13
#define SCen 12
#define SDer 11

//Variables
/*
  SA, SB, Sc -> nos ayudaran para la lectura de los sensores para saber sobre que se esta moviendo el robot
  Velocidad ->
  Estabilizador -> nos indica que llanta dio mas fuerza para que se estabilizara en la direccion correcta
  Graduador -> 
  Referencia ->
  Centraliza ->
  Direccion ->
*/
int SA, SB, SC;
int Velocidad = 0;
int Estabilizador = 0;
int Graduador, Referencia;
int Centraliza = 0;
int Direccion = 0;

// Velocidad Pwm
/*
  Son los pines que indican con que velocidad deben girar las llantas
*/
#define PwmI 10
#define PwmD 9

Servo jkr;
int Distancia;

void setup()
{
  // Sensor ultrasonico
  pinMode(Trigger, OUTPUT);
  pinMode(Echo, INPUT);
  pinMode(A0, INPUT);

  // Salidas de LLantas
  pinMode(LlantaDA, OUTPUT);
  pinMode(LlantaDR, OUTPUT);
  pinMode(LlantaIA, OUTPUT);
  pinMode(LlantaIR, OUTPUT);

  //Configuracion de sensores
  pinMode(SIzq, INPUT);
  pinMode(SCen, INPUT);
  pinMode(SDer, INPUT);

  //Configuracion de Pwm
  pinMode(PwmI, OUTPUT);
  pinMode(PwmD, OUTPUT);

  Serial.begin(9600);

  //Nombre y pin Servo
  /* jkr.attach(3);
  jkr.write(90);
  delayMicroseconds(15); */
}

void loop()
{
  //Lectura de sensores
  // La lectura de los sensores nos ayudara para saber
  // si turtle robot sigue en la linea
  SA = digitalRead(SIzq);
  SB = digitalRead(SCen);
  SC = digitalRead(SDer);

  //Sensor Central, nos indica si el carro va por el centro
  if (SA == LOW && SB == HIGH && SC == LOW)
  {
    //Configuramos la velociad para que ambas llantas vayan a la misma velocidad y por ende vaya recto
    analogWrite(PwmI, 100);
    analogWrite(PwmD, 100);

    //Colocamos que las llantas vayan hacia Adelante
    digitalWrite(LlantaIA, HIGH);
    digitalWrite(LlantaIR, LOW);

    digitalWrite(LlantaDA, HIGH);
    digitalWrite(LlantaDR, LOW);

    Referencia = 0;
    Graduador = 0;
    Estabilizador = 0;
    delay(300);
    Direccion = 0;
  }

  // Sensor Central e Izquierda, verifica si debe comenzar a girar a la Izquierda
  if (SA == HIGH && SB == HIGH && SC == LOW)
  {
    //Configuramos la velociad para que la llanta derecha vaya mas rapido y consiga que el carro comience a girar a la izquierda
    analogWrite(PwmI, 50);
    analogWrite(PwmD, 150 + Graduador);

    //Colocamos que las llantas vayan hacia Adelante
    digitalWrite(LlantaIA, HIGH);
    digitalWrite(LlantaIR, LOW);

    digitalWrite(LlantaDA, HIGH);
    digitalWrite(LlantaDR, LOW);

    delay(100);

    // Reducimos la velocidad para que se estabilice y asi no se salga del camino
    analogWrite(PwmI, 30);
    analogWrite(PwmD, 100 + Graduador);

    //Colocamos que las llantas vayan hacia Adelante
    digitalWrite(LlantaIA, HIGH);
    digitalWrite(LlantaIR, LOW);

    digitalWrite(LlantaDA, HIGH);
    digitalWrite(LlantaDR, LOW);
  }

  // Sensor Central y derecha, verifica si debe comenzar a girar a la derecha
  if (SA == LOW && SB == HIGH && SC == HIGH)
  {
    //Configuramos la velociad para que la llanta izquierda vaya mas rapido y consiga que el carro comience a girar a la derecha
    analogWrite(PwmI, 150 + Graduador);
    analogWrite(PwmD, 50);

    //Colocamos que las llantas vayan hacia Adelante
    digitalWrite(LlantaIA, HIGH);
    digitalWrite(LlantaIR, LOW);

    digitalWrite(LlantaDA, HIGH);
    digitalWrite(LlantaDR, LOW);

    Referencia = 0;
    delay(100);

    // Reducimos la velocidad para que se estabilice y asi no se salga del camino
    analogWrite(PwmI, 100 + Graduador);
    analogWrite(PwmD, 30);

    //Colocamos que las llantas vayan hacia Adelante
    digitalWrite(LlantaIA, HIGH);
    digitalWrite(LlantaIR, LOW);

    digitalWrite(LlantaDA, HIGH);
    digitalWrite(LlantaDR, LOW);
    Referencia = 0;
  }

  // Sensor izquierda, nos dice que el robot debe regresar rapidamente a la izquierda para que pueda continuar en la linea
  if (SA == HIGH && SB == LOW && SC == LOW)
  {
    //Configuramos la velociad para que la llanta derecha vaya mas rapido y consiga que el carro comience a girar a la izquierda rapidamente
    analogWrite(PwmI, 15);
    analogWrite(PwmD, 140 + Graduador);

    //Colocamos que las llantas vayan hacia Adelante
    digitalWrite(LlantaIA, HIGH);
    digitalWrite(LlantaIR, LOW);

    digitalWrite(LlantaDA, HIGH);
    digitalWrite(LlantaDR, LOW);

    //Se estabilizo por la derecha
    Estabilizador = 1;
  }

  // Sensor derecho, nos dice que el robot debe regresar rapidamente a la derecha para que pueda continuar en la linea
  if (SA == LOW && SB == LOW && SC == HIGH)
  {
    //Configuramos la velociad para que la llanta derecha vaya mas rapido y consiga que el carro comience a girar a la izquierda rapidamente
    analogWrite(PwmI, 140 + Graduador);
    analogWrite(PwmD, 15);

    //Colocamos que las llantas vayan hacia Adelante
    digitalWrite(LlantaIA, HIGH);
    digitalWrite(LlantaIR, LOW);

    digitalWrite(LlantaDA, HIGH);
    digitalWrite(LlantaDR, LOW);

    //Se estabilizo por la izquierda
    Estabilizador = 2;
  }

  //Estabilizador
  //Esto nos ayudara por si el vehiculo se queda dando vueltas y  asi podemos estabilizarlo para que continue la linea guia
  if (SA == HIGH && SB == HIGH && SC == HIGH)
  {
    //Vemos de que lado fue el que se estabilizo
    if (Estabilizador == 1)
    {
      //Se configura la velocidad para que este pueda dar una vuelta y pueda encontrar a donde debe irse y no salirse
      analogWrite(PwmI, 80);
      analogWrite(PwmD, 100);

      //Colocamos que las llantas vayan hacia atras girando a la derecha
      digitalWrite(LlantaIA, HIGH);
      digitalWrite(LlantaIR, LOW);

      digitalWrite(LlantaDA, LOW);
      digitalWrite(LlantaDR, HIGH);

      Referencia = 0;
      Graduador = 0;
      delay(700);
    }

    if (Estabilizador == 2)
    {
      //Se configura la velocidad para que este pueda dar una vuelta y pueda encontrar a donde debe irse y no salirse
      analogWrite(PwmI, 100);
      analogWrite(PwmD, 80);

      //Colocamos que las llantas vayan hacia atras girando a la izquierda
      digitalWrite(LlantaIA, LOW);
      digitalWrite(LlantaIR, HIGH);

      digitalWrite(LlantaDA, HIGH);
      digitalWrite(LlantaDR, LOW);

      Referencia = 0;
      Graduador = 0;
      delay(700);
    }

    // Si en el caso de recolocarnos en la linea nos salimos y ya no detectamos la linea realizamos lo siguiente
    if (SA == LOW && SB == LOW && SC == LOW)
    {
      //Vemos de que lado fue el que se estabilizo
      if (Estabilizador == 1)
      {
        //Se configura la velocidad para que este pueda dar una vuelta y pueda encontrar a donde debe irse y no salirse
        analogWrite(PwmI, 20);
        analogWrite(PwmD, 200);

        //Colocamos que las llantas vayan hacia adelante girando a la izquierda
        digitalWrite(LlantaIA, HIGH);
        digitalWrite(LlantaIR, LOW);

        digitalWrite(LlantaDA, HIGH);
        digitalWrite(LlantaDR, LOW);

        Referencia = 0;
        Graduador = 0;
      }

      if (Estabilizador == 2)
      {
        //Se configura la velocidad para que este pueda dar una vuelta y pueda encontrar a donde debe irse y no salirse
        analogWrite(PwmI, 200);
        analogWrite(PwmD, 20);

        //Colocamos que las llantas vayan hacia adelante girando a la derecha
        digitalWrite(LlantaIA, HIGH);
        digitalWrite(LlantaIR, LOW);

        digitalWrite(LlantaDA, HIGH);
        digitalWrite(LlantaDR, LOW);

        Referencia = 0;
        Graduador = 0;
      }
    }
  }
  Serial.print("Hola");
}
