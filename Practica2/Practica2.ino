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

void retroceder(int t, int x, int y) {
  analogWrite(PwmD, y);
  analogWrite(PwmI, x);

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
    analogWrite(PwmI, 100);
    analogWrite(PwmD, 100);

    //Colocamos que las llantas vayan hacia Adelante
    digitalWrite(LlantaIA, HIGH);
    digitalWrite(LlantaIR, LOW);

    digitalWrite(LlantaDA, HIGH);
    digitalWrite(LlantaDR, LOW);

    Estabilizador = 0;
    EstabilizadorAux = 1;
  }

  //Regreso a la linea guia, girando a la izquierda
  if (SA == HIGH && SB == HIGH && SC == LOW)
  {
    //Este es el caso en donde el robot debe girar a la izquierda
    //Hacemos girar el robot en contra de las agujas del reloj
    analogWrite(PwmI, 30);
    analogWrite(PwmD, 30);

    Estabilizador = 1;
    if (EstabilizadorAux == 0) {
      digitalWrite(LlantaIA, LOW);
      digitalWrite(LlantaIR, HIGH);

      digitalWrite(LlantaDA, HIGH);
      digitalWrite(LlantaDR, LOW);
    } else {
      analogWrite(PwmI, 30);
      analogWrite(PwmD, 30);
      digitalWrite(LlantaIA, LOW);
      digitalWrite(LlantaIR, HIGH);

      digitalWrite(LlantaDA, HIGH);
      digitalWrite(LlantaDR, LOW);
      delay(100);
    }

  }

  //Regresando a la linea guia, girando a la derecha
  if (SA == LOW && SB == HIGH && SC == HIGH)
  {
    //Hacemos girar el robot a favor de las agujas del reloj
    analogWrite(PwmI, 30);
    analogWrite(PwmD, 30);

    Estabilizador = 2;
    if (EstabilizadorAux == 1) {

      //Colocamos que las llantas vayan hacia Adelante
      digitalWrite(LlantaIA, HIGH);
      digitalWrite(LlantaIR, LOW);

      digitalWrite(LlantaDA, LOW);
      digitalWrite(LlantaDR, HIGH);
    } else {
      analogWrite(PwmI, 30);
      analogWrite(PwmD, 30);
      digitalWrite(LlantaIA, HIGH);
      digitalWrite(LlantaIR, LOW);

      digitalWrite(LlantaDA, HIGH);
      digitalWrite(LlantaDR, LOW);
      delay(100);
    }
  }

  //Los sensores nos indican que solo uno de los extremos esta activo, por lo cual
  //se procede a que el vehiculo retroseda para que se re ubique a donde debe continuar el giro
  if (SA == HIGH && SB == LOW && SC == LOW)
  {
    //Detenemos el carro
    detener();
    //Retrosedemos un poco para que el robot sepa a donde girar
    retroceder(100, 40, 10);

    Estabilizador = 1;
  }

  if (SA == LOW && SB == LOW && SC == HIGH) {
    //Detenemos el carro
    detener();
    //Retrosedemos un poco para que el robot sepa a donde girar
    retroceder(100, 10, 40);

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
      analogWrite(PwmI, 30);
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
      analogWrite(PwmD, 30);

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
      retroceder(200, 30, 30);
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
  //Activamos el trigger
  digitalWrite(Trig, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);
  //Colocamos el angulo que queremos el sensor ultrasonico
  servo.write(90);
  //Vemos la duracion que tarde en recibir el pulso enviado del ultrasonico
  //Si es 0 no hay nada
  //los 500 es el tiempo en microsegundo que se tarda en enviar el pulso, sino se pone el default es 1 segundo y vale pepino el loop
  duracion = pulseIn(Echo, HIGH, 500);
  //ecuacion para obtener la distancia del objeto
  distancia = (duracion / 2) / 29;
  Serial.println(distancia);
  if (distancia <= 6 && distancia >= 2) {  // si la distancia es menor de 6cm
    Serial.println("Entro");
    evitarObstaculo();
  }

}

void evitarObstaculo() {
  /*
    Estado 0 -> ir a la derecha
    EStado 1 -> ir a la izquierda
    Estado 2 -> ir a la izquierda
    Aca tuvimos que haber encontrado la linea, si no la encontramos empezamos a hacer un cuadrado
    con los estados 3 y 4
    Estado 3 -> ir a la izquierda
    En cada estado comprobar si hay obstaculo
        si hay obstaculo --> girar sobre si en el sentido contrario a donde giro
        no hay obstaculo --> avanzamos recto
  */
  int estado = 0;
  digitalWrite(53, HIGH);                // Enciende LED
  while ((distancia > 1 && distancia <= 10 ) || estado < 5 ) {
    Serial.print("Estado:");
    Serial.println(estado);
    switch (estado) {
      case 0:
        girarDerecha90();
        analogWrite(PwmI, 25);
      analogWrite(PwmD, 25);

      //Colocamos que las llantas vayan hacia Adelante
      digitalWrite(LlantaIA, HIGH);
      digitalWrite(LlantaIR, LOW);

      digitalWrite(LlantaDA, HIGH);
      digitalWrite(LlantaDR, LOW);
        Estabilizador = 1;
        tomarDistancia();
        if (hayObstaculo()) {
          //girar a la izquierda ---> posicion original
          girarIzquierda90();
          if (!recto(estado))return;
        }
        else {
          //moverse recto ---> avanzamos
          if (!recto(estado))return;
          estado++;
        }
        break;

      case 1: case 3: case 4:
        //girarIzquierda90();
        servo.write(0);
        detenerCachito();
        if (hayLinea())
          return;
        tomarDistancia();
        servo.write(90);
        Estabilizador = 2;
        if (hayObstaculo()) {
          //girar a la derecha  ---> posicion original
          girarDerecha90();
          if (!recto(estado))return;
        }
        else {
          girarIzquierda90();
          //moverse recto ---> avanzamos
          if (!recto(estado))return;
          estado++;
        }
        break;
      case 2:
        //girarIzquierda90();
        servo.write(0);
        detenerCachito();
        if (hayLinea())
          return;
        tomarDistancia();
        Estabilizador = 1;
        servo.write(90);
        if (!hayObstaculo()) {
          //girar a la derecha  ---> posicion original
          girarDerecha90();
          if (!recto(estado))return;
        }
        else {
          //moverse recto ---> avanzamos
          if (!recto(estado))return;
          if (hayLinea())
            return;
          if (!recto(estado))return;
          if (hayLinea())
            return;
          estado++;
        }
        break;
      default:
        //Saber donde jodidos estamos, doblar a algun lado y rogar a Dios para encontrar linea
        estado = 5;
        girarDerecha90();
        if (!recto(estado))return;
        break;
    }
  }
}

void tomarDistancia() {
  Serial.println("tomando distancia-------------");
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);
  duracion = pulseIn(Echo, HIGH);
  distancia = (duracion / 2) / 29;
}
boolean hayObstaculo() {
  if (distancia > 0 && distancia < 15) {
    Serial.println("Hay Obstaculo");
    return true;
  }
  return false;
}
void girarDerecha90() {
  Serial.println("DERECHA");

  //Configuramos la velociad para que la llanta izquierda vaya mas rapido y consiga que el carro comience a girar a la derecha
  analogWrite(PwmI, 100);
  analogWrite(PwmD, 80);

  //Colocamos que las llantas vayan hacia Adelante
  digitalWrite(LlantaIA, HIGH);
  digitalWrite(LlantaIR, LOW);

  digitalWrite(LlantaDA, LOW);
  digitalWrite(LlantaDR, HIGH);

  delay(1150);
  detenerCachito();
}
void girarIzquierda90() {
  Serial.println("IZQUIERDA");
  analogWrite(PwmI, 80);
  analogWrite(PwmD, 100);

  //Colocamos que las llantas vayan hacia Adelante
  digitalWrite(LlantaIA, LOW);
  digitalWrite(LlantaIR, HIGH);

  digitalWrite(LlantaDA, HIGH);
  digitalWrite(LlantaDR, LOW);
  delay(1150);
  detenerCachito();
}

boolean recto(int estado) {
  Serial.println("RECTO");
  //Configuramos la velociad para que ambas llantas vayan a la misma velocidad y por ende vaya recto
  analogWrite(PwmI, 100);
  analogWrite(PwmD, 100);

  //Colocamos que las llantas vayan hacia Adelante
  digitalWrite(LlantaIA, HIGH);
  digitalWrite(LlantaIR, LOW);

  digitalWrite(LlantaDA, HIGH);
  digitalWrite(LlantaDR, LOW);
  int contador = 0;
  while (contador < 15) {
    if (hayLinea() && estado != 0) {
      return false;
    }
    contador++;
    delay(150);
  }
  return true;
}

void detenerCachito() {
  analogWrite(PwmD, 0);
  analogWrite(PwmI, 0);

  digitalWrite(LlantaIA, LOW);
  digitalWrite(LlantaIR, LOW);

  digitalWrite(LlantaDA, LOW);
  digitalWrite(LlantaDR, LOW);
  delay(200);
}
boolean hayLinea() {
  SA = digitalRead(SIzq);
  SB = digitalRead(SCen);
  SC = digitalRead(SDer);
  if (SA == HIGH || SB == HIGH || SC == HIGH) {
    detenerCachito();
    return true;
  }
  return false;
}
