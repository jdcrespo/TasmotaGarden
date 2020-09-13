#include <Arduino.h>


String topic = "";

#define numSensores  3
#define delayEstabilizacion  500
#define delayPolarizacion  200


int sensores[numSensores][3] = {
  {
    12, // Pin digital A
    11, // Pin digital B
    0   // Pin analógico
  },
  {
    10, // Pin digital A
    9, // Pin digital B
    1   // Pin analógico
  },
  {
    8, // Pin digital A
    7, // Pin digital B
    2   // Pin analógico
  }
};

void setup() {
  Serial.begin(115200);

  // Esperamos 10 segundos a que se inicie Tasmota y limpiamos el buffer de etnrada
  delay(10000);
  while(Serial.available()){
    Serial.read();
  }

  // Enviamos el comando Status para obtener el estado resumido
  Serial.println("Status");
  // Esperamos la respuesta
  while (!Serial.available()){
    delay(50);

  }
  // Leemos la primera linea que nos llegue
  String linea = Serial.readStringUntil('\n');
  
  // Si se corresponde con el ACK de tasmota, seguimos leyendo la siguiente que será el resultado del comando
  if(linea.lastIndexOf(String("CMD: Status")) == 9){
    
    // Leemos la siguiente linea
    String linea = Serial.readStringUntil('\n');
    // Buscamos donde se encuentra el topic dentro de la linea recibida
    int posInicio = linea.lastIndexOf(String(" stat/"));
    int posFinal = linea.lastIndexOf(String("/STATUS"));
    // Cortamos el topic únicamente y lo guardamos
    topic = linea.substring(posInicio + 6, posFinal);
    if(topic != ""){
      //Serial.print("TOPIC: ");
      //Serial.println(topic);
    }else{
      Serial.println("Error al leer TOPIC");
    }
    
  }else{
    Serial.println("Error al leer TOPIC");
  }

  // Inicializamos las salidas de los sensores
  for (int i = 0; i < numSensores; i++)
  {
    pinMode(sensores[i][0], OUTPUT);
    pinMode(sensores[i][1], OUTPUT);
    pinMode(sensores[i][2], INPUT);
  }
  
 
}

void loop() {
  // Si hemos conseguido obtener el topic se ejecuta esto
  if(topic != ""){
    while (!Serial.available()){
      delay(50);

    }
    String linea = Serial.readStringUntil('\n');
    int pos = linea.lastIndexOf(String("MQT: tele/"+ topic + "/STATE"));
    if(pos == 9){
      for (int i = 0; i < numSensores; i++)
      {
       /* int actual = i;
        int pinA = sensores[i][0];
        int pinB = sensores[i][1];
        int pinC = sensores[i][2];
        Serial.println("Leyendo sensor  pin A: " + pinA + " pin B: " + pinB + "input: "+ pinC);*/
        // Iniciamos la primera lectura
        digitalWrite(sensores[i][0], HIGH);
        digitalWrite(sensores[i][1], LOW);
        delay(delayEstabilizacion);
        int lectura1 = analogRead(sensores[i][2]);

        // Esperamos con todo apagado antes de cambiar de polaridad
        digitalWrite(sensores[i][0], LOW);
        digitalWrite(sensores[i][1], LOW);
        delay(delayPolarizacion);

        // Realizamos la segunda lectura
        digitalWrite(sensores[i][0], LOW);
        digitalWrite(sensores[i][1], HIGH);
        delay(delayEstabilizacion);
        int lectura2 = 1023 - analogRead(sensores[i][2]);

        // Apagamos el sensor
        digitalWrite(sensores[i][0], LOW);
        digitalWrite(sensores[i][1], LOW);
        // Hacemos la media
        int media = (lectura1 + lectura2) / 2;
        // Enviamos la media por MQTT
        Serial.println("Publish tele/" + topic + "/MOISTURE_" + i + " " + media);
      }
      
    }
    while(Serial.available()){
      Serial.read();
    }
  }
}