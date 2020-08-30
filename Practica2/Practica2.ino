#include <Stepper.h>
#include <Servo.h>
#include <LedControl.h>
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
int SA, SB, SC, SBarredora;
int Estabilizador = 0;
// EstabilizadorAux nos ayuda a ver si el objeto iba recto y se salio de de la linea
int EstabilizadorAux = 1;
bool d=false;//direccion de la barredora

bool matrizObstaculo[8][8];

// Velocidad Pwm
/*
  Son los pines que indican con que velocidad deben girar las llantas
*/
#define PwmI 11
#define PwmD 9

long duracion;
long distancia;
int vel = 255;

Servo servo, servo2;

int posXM, posYM, delayMatriz, delayObstaculo;
bool ArribaAbajoM, IzqDerM, direccionM, enGiro, prenderObstaculo;
unsigned long tiempoGiro, registroGiro;


//Variable que controla la matriz de leds
LedControl matriz = LedControl(23, 27, 25, 1);

Stepper stepperI = Stepper(4, 29, 31, 33, 35);
Stepper stepperD = Stepper(4, 39, 41, 43, 45);

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

  //Sacamos el controlador del modo ahorro de energia
  matriz.shutdown(0, false);

  //Definimos la intensisdad a medio para el controlador
  matriz.setIntensity(0, 8);

  //Prendemos la posicion inicial
  posXM = 3; posYM = 4;
  matriz.setLed(0, posYM, posXM, true);
  delayMatriz = 0;

  direccionM = IzqDerM = ArribaAbajoM = true;
  enGiro = false;
  tiempoGiro = 0;

  servo2.attach(8);
  servo2.write(180);

  limpiarMatrizObjetos();
  
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

    if (delayMatriz >= 1500) {

      
      Serial.print(enGiro);
      Serial.print("  -  ");
      Serial.println(abs(millis() - tiempoGiro));

      stepperD.step(2);
      stepperI.step(2);

      if (!enGiro) pintarCamino();

      delayMatriz = 0;

    } else delayMatriz++;

    if (enGiro && abs(millis() - tiempoGiro) >= 3200) {
      enGiro = false; tiempoGiro = 0;
      Serial.println("Reinicio");
    }

    Estabilizador = 0;
    EstabilizadorAux = 1;
  }

  //Regreso a la linea guia, girando a la izquierda
  if (SA == HIGH && SB == HIGH && SC == LOW)
  {
    //Este es el caso en donde el robot debe girar a la izquierda
    //Hacemos girar el robot en contra de las agujas del reloj
    analogWrite(PwmI, 35);
    analogWrite(PwmD, 35);

    Estabilizador = 1;
    if (EstabilizadorAux == 0) {
      digitalWrite(LlantaIA, HIGH);
      digitalWrite(LlantaIR, LOW);

      digitalWrite(LlantaDA, LOW);
      digitalWrite(LlantaDR, HIGH);

      stepperD.step(-1);
      stepperI.step(1);
    } else {

      //Cosas que pintan
      if (!enGiro&& ( registroGiro == 0 ||  abs(millis() - registroGiro) >= 2000 )) {
        enGiro = true;
        tiempoGiro = millis();
      } else if (enGiro && abs(millis() - tiempoGiro) >= 2700 && abs(millis() - tiempoGiro) <= 3000) {
        //ANTES
        //Si es verdadero va sobre el eje Y
        if (direccionM) {
          //Vamos hacia arriba
          if (ArribaAbajoM) {
            IzqDerM = false;
          } else {
            IzqDerM = true;
          }
        } else {
          //Si IzqDerM es verdadero iba hacia la derecha
          if (IzqDerM) {
            ArribaAbajoM = true;
          } else {
            ArribaAbajoM = false;
          }
        }

        //Cambia de eje en cual se mueve de tal manera que si esta en Y pasa a moverse en X
        direccionM = !direccionM;

        enGiro = false;
        delay(1);
        tiempoGiro = 0;

        registroGiro = millis();
        Serial.print("Girando izquierda - ");
        Serial.println(tiempoGiro);
      }

      analogWrite(PwmI, 35);
      analogWrite(PwmD, 35);
      digitalWrite(LlantaIA, LOW);
      digitalWrite(LlantaIR, HIGH);

      digitalWrite(LlantaDA, HIGH);
      digitalWrite(LlantaDR, LOW);

      stepperD.step(-1);
      stepperI.step(1);
      stepperD.step(1);
      stepperI.step(1);
      delay(50);

      digitalWrite(LlantaIA, HIGH);
      digitalWrite(LlantaIR, LOW);

      digitalWrite(LlantaDA, HIGH);
      digitalWrite(LlantaDR, LOW);
    }


  }

  //Regresando a la linea guia, girando a la derecha
  if (SA == LOW && SB == HIGH && SC == HIGH)
  {
    //Hacemos girar el robot a favor de las agujas del reloj
    analogWrite(PwmI, 35);
    analogWrite(PwmD, 35);

    Estabilizador = 2;
    if (EstabilizadorAux == 0) {
      //Colocamos que las llantas vayan hacia Adelante
      digitalWrite(LlantaIA, LOW);
      digitalWrite(LlantaIR, HIGH);

      digitalWrite(LlantaDA, HIGH);
      digitalWrite(LlantaDR, LOW);

      stepperD.step(1);
      stepperI.step(-1);
    } else {
      if (!enGiro && ( registroGiro == 0 ||  abs(millis() - registroGiro) >= 2000)) {
        enGiro = true;
        tiempoGiro = millis();
      }
      else if (enGiro && abs(millis() - tiempoGiro) >= 2700 && abs(millis() - tiempoGiro) <= 3000) {
        if (direccionM) {
          if (ArribaAbajoM) {
            IzqDerM = true;
          } else {
            IzqDerM = false;
          }
        } else {
          if (IzqDerM) {
            ArribaAbajoM = false;
          } else {
            ArribaAbajoM = true;
          }
        }

        direccionM = !direccionM;

        enGiro = false;
        delay(1);
        tiempoGiro = 0;

        //2700 -> 5401
        registroGiro = millis();

        Serial.print("Girando derecha - ");
        Serial.println(tiempoGiro);
      }
      analogWrite(PwmI, 35);
      analogWrite(PwmD, 35);
      digitalWrite(LlantaIA, HIGH);
      digitalWrite(LlantaIR, LOW);

      digitalWrite(LlantaDA, LOW);
      digitalWrite(LlantaDR, HIGH);

      stepperD.step(1);
      stepperI.step(-1);
      stepperD.step(1);
      stepperI.step(1);
      delay(50);

      digitalWrite(LlantaIA, HIGH);
      digitalWrite(LlantaIR, LOW);

      digitalWrite(LlantaDA, HIGH);
      digitalWrite(LlantaDR, LOW);

    }
  }


  //Los sensores nos indican que solo uno de los extremos esta activo, por lo cual
  //se procede a que el vehiculo retroseda para que se re ubique a donde debe continuar el giro
  if (SA == HIGH && SB == LOW && SC == LOW)
  {
    //Detenemos el carro
    detener();
    //Retrosedemos un poco para que el robot sepa a donde girar
    analogWrite(PwmI, 10);
    analogWrite(PwmD, 40);
    digitalWrite(LlantaIA, HIGH);
    digitalWrite(LlantaIR, LOW);

    digitalWrite(LlantaDA, HIGH);
    digitalWrite(LlantaDR, LOW);
    stepperD.step(-2);
    stepperI.step(-1);
    delay(50);
    analogWrite(PwmI, 30);
    analogWrite(PwmD, 30);
    digitalWrite(LlantaIA, HIGH);
    digitalWrite(LlantaIR, LOW);

    digitalWrite(LlantaDA, HIGH);
    digitalWrite(LlantaDR, LOW);
    stepperD.step(-1);
    stepperI.step(-1);
    delay(50);


    Estabilizador = 1;
    EstabilizadorAux = 0;
  }


  if (SA == LOW && SB == LOW && SC == HIGH) {
    //Detenemos el carro
    detener();
    //Retrosedemos un poco para que el robot sepa a donde girar
    analogWrite(PwmI, 40);
    analogWrite(PwmD, 10);
    digitalWrite(LlantaIA, HIGH);
    digitalWrite(LlantaIR, LOW);

    digitalWrite(LlantaDA, HIGH);
    digitalWrite(LlantaDR, LOW);

    stepperD.step(-1);
    stepperI.step(-2);
    delay(50);
    analogWrite(PwmI, 30);
    analogWrite(PwmD, 30);
    digitalWrite(LlantaIA, HIGH);
    digitalWrite(LlantaIR, LOW);

    digitalWrite(LlantaDA, HIGH);
    digitalWrite(LlantaDR, LOW);
    stepperD.step(-1);
    stepperI.step(-1);
    delay(50);

    Estabilizador = 2;
    EstabilizadorAux = 0;
  }

  //Estabilizador
  //Esto nos ayudara por si el vehiculo se queda dando vueltas y  asi podemos estabilizarlo para que continue la linea guia
  if (SA == HIGH && SB == HIGH && SC == HIGH)
  {
    //Vemos de que lado fue el que se estabilizo
    if (Estabilizador == 2)
    {
      //Se configura la velocidad para que este pueda dar una vuelta y pueda encontrar a donde debe irse y no salirse
      analogWrite(PwmI, 35);
      analogWrite(PwmD, 30);

      digitalWrite(LlantaIA, HIGH);
      digitalWrite(LlantaIR, LOW);

      digitalWrite(LlantaDA, LOW);
      digitalWrite(LlantaDR, HIGH);

      stepperD.step(-1);
      stepperI.step(1);
      delay(200);
      analogWrite(PwmI, 30);
      analogWrite(PwmD, 30);
      digitalWrite(LlantaIA, HIGH);
      digitalWrite(LlantaIR, LOW);

      digitalWrite(LlantaDA, HIGH);
      digitalWrite(LlantaDR, LOW);

      stepperD.step(1);
      stepperI.step(1);
      delay(50);
    }

    if (Estabilizador == 1)
    {
      //Se configura la velocidad para que este pueda dar una vuelta y pueda encontrar a donde debe irse y no salirse
      analogWrite(PwmI, 30);
      analogWrite(PwmD, 35);

      digitalWrite(LlantaIA, LOW);
      digitalWrite(LlantaIR, HIGH);

      digitalWrite(LlantaDA, HIGH);
      digitalWrite(LlantaDR, LOW);

      stepperD.step(1);
      stepperI.step(-1);
      delay(200);

      analogWrite(PwmI, 30);
      analogWrite(PwmD, 30);
      digitalWrite(LlantaIA, HIGH);
      digitalWrite(LlantaIR, LOW);

      digitalWrite(LlantaDA, HIGH);
      digitalWrite(LlantaDR, LOW);

      stepperD.step(1);
      stepperI.step(1);
      delay(50);
    }
    if (Estabilizador == 0) {
      Serial.println("se mueve enfrente");
      analogWrite(PwmI, 30);
      analogWrite(PwmD, 30);
      digitalWrite(LlantaIA, HIGH);
      digitalWrite(LlantaIR, LOW);

      digitalWrite(LlantaDA, HIGH);
      digitalWrite(LlantaDR, LOW);
      delay(100);
    }
  }
  // Si en el caso de recolocarnos en la linea nos salimos y ya no detectamos la linea realizamos lo siguiente
  if (SA == LOW && SB == LOW && SC == LOW)
  {
    //Vemos si este Estabilizador iba recto
    //if (EstabilizadorAux == 0) {
    // detener();
    // retroceder(200, 30, 30);
    //} else {
    //Configuramos la velociad para que ambas llantas vayan a la misma velocidad y por ende vaya recto
    analogWrite(PwmI, 25);
    analogWrite(PwmD, 25);

    //Colocamos que las llantas vayan hacia Adelante
    digitalWrite(LlantaIA, HIGH);
    digitalWrite(LlantaIR, LOW);

    digitalWrite(LlantaDA, HIGH);
    digitalWrite(LlantaDR, LOW);

    stepperD.step(1);
    stepperI.step(1);
    delay(200);
    //}
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
  
  girarServo(); 
  if (distancia <= 10 && distancia >= 2) {  // si la distancia es menor de 6cm
    Serial.println("Entro");
    evitarObstaculo();
  }

  pintarObstaculos();

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

  //Mini validaciones para pintar matriz
  int obstaculoX, obstaculoY, mov1X, mov1Y, mov2X, mov2Y, mov3X, mov3Y;
  bool pos2 = false, pos3 = false;

  if (direccionM) {
    if (ArribaAbajoM) {
      if (posYM > 0) {
        obstaculoX = posXM; obstaculoY = posYM-1;
      } else {
        matriz.clearDisplay(0);
        limpiarMatrizObjetos();
        obstaculoX = 4; obstaculoY = 3;
      }
      
      mov1X = obstaculoX + 1; mov1Y = obstaculoY;
      mov2X = obstaculoX; mov2Y = obstaculoY - 1;
      mov3X = obstaculoX - 1; mov3Y = obstaculoY; 
    } else {
      if (posYM < 7) {
        obstaculoX = posXM; obstaculoY = posYM+1;
      } else {
        matriz.clearDisplay(0);
        limpiarMatrizObjetos();
        obstaculoX = 4; obstaculoY = 3;
      }

      mov1X = obstaculoX - 1; mov1Y = obstaculoY;
      mov2X = obstaculoX; mov2Y = obstaculoY + 1;
      mov3X = obstaculoX + 1; mov3Y = obstaculoY; 
    }
  } 
  else {
    if (IzqDerM) {
      if (posXM < 7)  {
        obstaculoX = posXM+1; obstaculoY = posYM;
      } else {
        matriz.clearDisplay(0);
        limpiarMatrizObjetos();
        obstaculoX = 4; obstaculoY = 3;
      }

      mov1X = obstaculoX; mov1Y = obstaculoY + 1;
      mov2X = obstaculoX + 1; mov2Y = obstaculoY;
      mov3X = obstaculoX; mov3Y = obstaculoY - 1; 

    } else {
      if (posXM > 0) {
        obstaculoX = posXM-1; obstaculoY = posYM;
      } else {
        matriz.clearDisplay(0);
        limpiarMatrizObjetos();
        obstaculoX = 4; obstaculoY = 3;
      }

      mov1X = obstaculoX; mov1Y = obstaculoY - 1;
      mov2X = obstaculoX - 1; mov2Y = obstaculoY;
      mov3X = obstaculoX; mov3Y = obstaculoY + 1; 
      
    }
  }

  matrizObstaculo[obstaculoY][obstaculoX] = 1;

  int estado = 0;
  digitalWrite(53, HIGH);                // Enciende LED
  girarDerecha90();
  continuarRecto(200);

  enGiro = false;

  //Posibles posiciones

  while ( estado < 3 ) {
    Serial.print("Estado:");
    Serial.println(estado);
    pintarObstaculos();
    switch (estado) {
      case 0:
        
        if (!recto(5)) return;
        girarIzquierda90();
        tomarDistancia();
        if (hayObstaculo()) {
          girarDerecha90();
        } else {
          estado++;
        }
        break;
      case 3: matriz.setLed(0, mov3Y, mov3X, true); pos3 = true;
      case 2: matriz.setLed(0, mov2Y, mov2X, true); pos2 = true;
      case 1: matriz.setLed(0, mov1Y, mov1X, true);
        //Cosas para pintar
        if (pos3) {
          posXM = mov3X; posYM = mov3Y;
        } else if (pos2) {
          posXM = mov2X; posYM = mov2Y;
        } else {
          posXM = mov1X; posYM = mov1Y;
        }

        tomarDistancia();
        if (!hayObstaculo()) {
          if (!recto(20)) return;
          girarIzquierda90();
          tomarDistancia();
          if (hayObstaculo()) {
            girarDerecha90();
          } else {
            estado++;
          }
        } else {
          girarDerecha90();
        }
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
  if (distancia > 0 && distancia < 20) {
    Serial.println("Hay Obstaculo");
    if (distancia > 10) {
      detener();
      delay(250);
      //ENDEREZARSE
      //Configuramos la velociad para que la llanta izquierda vaya mas rapido y consiga que el carro comience a girar a la derecha
      analogWrite(PwmI, 100);
      analogWrite(PwmD, 80);
      //Colocamos que las llantas vayan hacia Adelante
      digitalWrite(LlantaIA, HIGH);
      digitalWrite(LlantaIR, LOW);
      digitalWrite(LlantaDA, LOW);
      digitalWrite(LlantaDR, HIGH);
      delay(20);
    }
    return true;
  }
  return false;
}

void girarDerecha90() {
  /*COSAS QUE PINTAN*/
  if (direccionM) {
    if (ArribaAbajoM) {
      IzqDerM = true;
    } else {
      IzqDerM = false;
    }
  } 
  else {
    if (IzqDerM) {
      ArribaAbajoM = false;
    } else {
      ArribaAbajoM = true;
    }
  }

  direccionM = !direccionM;

  /*MOVIMIENTO*/
  Serial.println("DERECHA");

  //Configuramos la velociad para que la llanta izquierda vaya mas rapido y consiga que el carro comience a girar a la derecha
  analogWrite(PwmI, 100);
  analogWrite(PwmD, 100);

  //Colocamos que las llantas vayan hacia Adelante
  digitalWrite(LlantaIA, HIGH);
  digitalWrite(LlantaIR, LOW);

  digitalWrite(LlantaDA, LOW);
  digitalWrite(LlantaDR, HIGH);

  stepperD.step(-1);
  stepperI.step(1);
  delay(1050);
  detenerCachito();
}

void girarIzquierda90() {
  /*COSAS PARA PINTAR*/
  if (direccionM) {
    if (ArribaAbajoM) {
      IzqDerM = false;
    } else {
      IzqDerM = true;
    }
  } else {
    if (IzqDerM) {
      ArribaAbajoM = true;
    } else {
      ArribaAbajoM = false;
    }
  }
  
  direccionM = !direccionM;

  /*MOVERSE XD*/
  Serial.println("IZQUIERDA");
  analogWrite(PwmI, 100);
  analogWrite(PwmD, 100);

  //Colocamos que las llantas vayan hacia Adelante
  digitalWrite(LlantaIA, LOW);
  digitalWrite(LlantaIR, HIGH);

  digitalWrite(LlantaDA, HIGH);
  digitalWrite(LlantaDR, LOW);

  stepperD.step(1);
  stepperI.step(-1);
  delay(1050);
  detenerCachito();
}

boolean recto(int pasadas) {
  Serial.println("RECTO");
  //Configuramos la velociad para que ambas llantas vayan a la misma velocidad y por ende vaya recto
  analogWrite(PwmI, 100);
  analogWrite(PwmD, 100);

  //Colocamos que las llantas vayan hacia Adelante
  digitalWrite(LlantaIA, HIGH);
  digitalWrite(LlantaIR, LOW);

  digitalWrite(LlantaDA, HIGH);
  digitalWrite(LlantaDR, LOW);
  stepperD.step(1);
  stepperI.step(1);
  int contador = 0;
  while (contador < pasadas) {
    if (hayLinea()) {
      return false;
    }
    contador++;
    delay(100);
  }
  return true;
}

void continuarRecto(int _delay) {
  //Configuramos la velociad para que ambas llantas vayan a la misma velocidad y por ende vaya recto
  analogWrite(PwmI, 100);
  analogWrite(PwmD, 100);

  //Colocamos que las llantas vayan hacia Adelante
  digitalWrite(LlantaIA, HIGH);
  digitalWrite(LlantaIR, LOW);

  digitalWrite(LlantaDA, HIGH);
  digitalWrite(LlantaDR, LOW);

  stepperD.step(2);
  stepperI.step(2);
  delay(_delay);
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

void pintarCamino() {
  //Ponemos un puntito delante
  if (direccionM) {
    if (posYM > 0 && posYM < 7) {
      if (ArribaAbajoM) {
        matriz.setLed(0, --posYM, posXM, true);
      } else {
        matriz.setLed(0, ++posYM, posXM, true);
      }
    } else {
      matriz.clearDisplay(0);
      limpiarMatrizObjetos();
      posXM = 3; posYM = 4;
      matriz.setLed(0, posYM, posXM, true);
    }
  } else {
    if (posXM > 0 && posXM < 7) {
      if (IzqDerM) {
        matriz.setLed(0, posYM, ++posXM, true);
      } else {
        matriz.setLed(0, posYM, --posXM, true);
      }
    } else {
      matriz.clearDisplay(0);
      limpiarMatrizObjetos();
      posXM = 3; posYM = 4;
      matriz.setLed(0, posYM, posXM, true);
    }
  }
}

void limpiarMatrizObjetos() {
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      matrizObstaculo[i][j] = 0;
    }
  }
}

void girarServo() {
  SBarredora = analogRead(A0);
  //Serial.println(SA);
  if (SBarredora >= 36 && SBarredora <= 42) {  // si detecta color amarillo
    Serial.println("------------------------------------Obstaculo-------------------------------");
    detenerCachito();
    if (d) {
      servo2.write(180);
      d = false;
    } else {
      servo2.write(0);
      d = true;
    }
    delay(3500);
  }
}

void pintarObstaculos(){
  if (delayObstaculo >= 1000){
    for (int i = 0; i < 8; i++)
    {
      for (int j = 0; j < 8; j++)
      {
        if ( matrizObstaculo[j][i] == 1) matriz.setLed(0,j,i,prenderObstaculo);
      }
    }
    prenderObstaculo = !prenderObstaculo;
  } else delayObstaculo++;
}