#include <Servo.h>
//Para sensor ultrasonico
#define Trig 3
#define Echo 2

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
#define SDer 22

//Variables
/*
  SA, SB, Sc -> nos ayudaran para la lectura de los sensores para saber sobre que se esta moviendo el robot
  Estabilizador -> nos indica que llanta dio mas fuerza para que se estabilizara en la direccion correcta
*/
int SA, SB, SC;
int Estabilizador = 0;
// EstabilizadorAux nos ayuda a ver si el objeto iba recto y se salio de de la linea
int EstabilizadorAux = 0;

// Velocidad Pwm
/*
  Son los pines que indican con que velocidad deben girar las llantas
*/
#define PwmI 11
#define PwmD 9

long duracion;
long distancia;
int vel = 255;

Servo servo;


void evitarObstaculo() {
  digitalWrite(53, HIGH);                // Enciende LED para que nos ayude ver que encontro un obstaculo

  analogWrite(PwmI, 0);
  analogWrite(PwmD, 0);
  digitalWrite(LlantaIA, LOW);
  digitalWrite(LlantaIR, LOW);

  digitalWrite(LlantaDA, LOW);
  digitalWrite(LlantaDR, LOW);
  delay(1000);//aca debe ir el metodo para eviar obstaculsos
  digitalWrite(53, LOW);
}

void detener() {
  //Detenemos el carro
  analogWrite(PwmD, 0);
  analogWrite(PwmI, 0);

  digitalWrite(LlantaIA, LOW);
  digitalWrite(LlantaIR, LOW);

  digitalWrite(LlantaDA, LOW);
  digitalWrite(LlantaDR, LOW);
}

void restablecer() {
  analogWrite(PwmI, 20);
  analogWrite(PwmD, 20);

  digitalWrite(LlantaIA, HIGH);
  digitalWrite(LlantaIR, LOW);

  digitalWrite(LlantaDA, HIGH);
  digitalWrite(LlantaDR, LOW);
  delay(100);
}

void retroceder(int t) {
  analogWrite(PwmD, 25);
  analogWrite(PwmI, 25);

  digitalWrite(LlantaIA, LOW);
  digitalWrite(LlantaIR, HIGH);

  digitalWrite(LlantaDA, LOW);
  digitalWrite(LlantaDR, HIGH);

  delay(t);
}

void setup()
{
  // Sensor ultrasonico
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);

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
  servo.attach(10);

  pinMode(53, OUTPUT);
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
    analogWrite(PwmI, 25);
    analogWrite(PwmD, 25);

    //Colocamos que las llantas vayan hacia Adelante
    digitalWrite(LlantaIA, HIGH);
    digitalWrite(LlantaIR, LOW);

    digitalWrite(LlantaDA, HIGH);
    digitalWrite(LlantaDR, LOW);

    Estabilizador = 0;
    EstabilizadorAux = 1;
    delay(300);
  }

  //Regreso a la linea guia, girando a la izquierda
  if (SA == HIGH && SB == HIGH && SC == LOW)
  {
    //Hacemos girar el robot en contra de las agujas del reloj
    analogWrite(PwmI, 10);
    analogWrite(PwmD, 30);

    digitalWrite(LlantaIA, LOW);
    digitalWrite(LlantaIR, HIGH);

    digitalWrite(LlantaDA, HIGH);
    digitalWrite(LlantaDR, LOW);

    // Tiempo que debe tardar en moverse adelante
    delay(200);

    restablecer();
    Estabilizador = 1;
    EstabilizadorAux = 0;
  }

  //Regresando a la linea guia, girando a la derecha
  if (SA == LOW && SB == HIGH && SC == HIGH)
  {
    //Hacemos girar el robot a favor de las agujas del reloj
    analogWrite(PwmI, 30);
    analogWrite(PwmD, 10);

    //Colocamos que las llantas vayan hacia Adelante
    digitalWrite(LlantaIA, HIGH);
    digitalWrite(LlantaIR, LOW);

    digitalWrite(LlantaDA, LOW);
    digitalWrite(LlantaDR, HIGH);

    delay(200);

    restablecer();
    Estabilizador = 2;
    EstabilizadorAux = 0;
  }

  //Los sensores nos indican que solo uno de los extremos esta activo, por lo cual
  //se procede a que el vehiculo retroseda para que se re ubique a donde debe continuar el giro
  if ((SA == HIGH && SB == LOW && SC == LOW) || (SA == LOW && SB == LOW && SC == HIGH))
  {
    //Detenemos el carro
    detener();
    //Retrosedemos un poco para que el robot sepa a donde girar
    retroceder(100);
  }

  //Estabilizador
  //Esto nos ayudara por si el vehiculo se queda dando vueltas y  asi podemos estabilizarlo para que continue la linea guia
  if (SA == HIGH && SB == HIGH && SC == HIGH)
  {
    //Vemos de que lado fue el que se estabilizo
    if (Estabilizador == 1)
    {
      //Se configura la velocidad para que este pueda dar una vuelta y pueda encontrar a donde debe irse y no salirse
      analogWrite(PwmI, 10);
      analogWrite(PwmD, 30);

      digitalWrite(LlantaIA, HIGH);
      digitalWrite(LlantaIR, LOW);

      digitalWrite(LlantaDA, LOW);
      digitalWrite(LlantaDR, HIGH);

      delay(100);
    }

    if (Estabilizador == 2)
    {
      //Se configura la velocidad para que este pueda dar una vuelta y pueda encontrar a donde debe irse y no salirse
      analogWrite(PwmI, 30);
      analogWrite(PwmD, 10);

      digitalWrite(LlantaIA, LOW);
      digitalWrite(LlantaIR, HIGH);

      digitalWrite(LlantaDA, HIGH);
      digitalWrite(LlantaDR, LOW);

      delay(100);
    }
  }
  // Si en el caso de recolocarnos en la linea nos salimos y ya no detectamos la linea realizamos lo siguiente
  if (SA == LOW && SB == LOW && SC == LOW)
  {
    //Vemos si este Estabilizador iba recto
    if (EstabilizadorAux == 0) {
      detener();
      retroceder(200);
    } else {
      //Configuramos la velociad para que ambas llantas vayan a la misma velocidad y por ende vaya recto
      analogWrite(PwmI, 25);
      analogWrite(PwmD, 25);

      //Colocamos que las llantas vayan hacia Adelante
      digitalWrite(LlantaIA, HIGH);
      digitalWrite(LlantaIR, LOW);

      digitalWrite(LlantaDA, HIGH);
      digitalWrite(LlantaDR, LOW);
      delay(300);
    }
  }
  digitalWrite(Trig, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);
  servo.write(90);
  digitalWrite(PwmI, 10);
  digitalWrite(PwmD, 10);
  duracion = pulseIn(Echo, HIGH, 500);
  distancia = (duracion / 2) / 29;
  Serial.println(distancia);
  if (distancia <= 15 && distancia >= 2) {  // si la distancia es menor de 15cm
    Serial.println("Entro");
    evitarObstaculo();
  }

}
