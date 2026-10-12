#include "st7789_dma.h"
#include "models.h"      // Mantiene el cubo y las estructuras del reloj
#include "fonts.h"
#include "JUEGO.h"
#include "JUEGO2.h"
#include "JUEGO3.h"


MandoControl mC;


#include "PRUEBAS3D.h"
#include "BATALLA.h"
#include "SUPERVIVENCIA.h"
#include "SMARTWATCH.h"

#define JOY_X 16
#define JOY_Y 15
#define JOY2_X 17
#define JOY2_Y 18
#define BTN 4
#define BTN2 5
#define BTN3 6
#define BTN4 7

#define DEADZONE 500
#define VALOR_ANALOG 2048

// Tablas de geometría de ambos juegos. LCD.h utiliza los punteros activos.
const int32_t *vertices = verticesBatalla;
const float *verticesF = verticesMaquina;
const int *faces = facesBatalla;

void selectGameGeometry(GeometryGame game) {
  if (game == GEOMETRY_BATALLA_NAVAL) {
    vertices = verticesBatalla;
    faces = facesBatalla;
    prepareGameFaceNormals(
      verticesBatalla,
      facesBatalla,
      (uint32_t)(sizeof(facesBatalla) / sizeof(facesBatalla[0]) / 6)
    );
  } else if (game == GEOMETRY_SUPERVIVENCIA) {
    vertices = verticesSupervivencia;
    faces = facesSupervivencia;
    prepareGameFaceNormals(
      verticesSupervivencia,
      facesSupervivencia,
      (uint32_t)(sizeof(facesSupervivencia) / sizeof(facesSupervivencia[0]) / 6)
    );
  } else {
    verticesF = verticesMaquina;
    faces = facesMaquina;
    prepareGameFaceNormals(nullptr, nullptr, 0);
  }
}

bool buttonPressed() {
  return digitalRead(BTN) == LOW;  // Pulldown, presionado = LOW
}

bool buttonPressed2() {
  return digitalRead(BTN2) == LOW;  // Pulldown, presionado = LOW
}

bool buttonPressed3() {
  return digitalRead(BTN3) == LOW;  // Pulldown, presionado = LOW
}

bool buttonPressed4() {
  return digitalRead(BTN4) == LOW;  // Pulldown, presionado = LOW
}


void updateJoystick() {
  bool moved = false;
  mC.t_arriba = false;
  mC.t_abajo = false;
  mC.t_izquierda = false;
  mC.t_derecha = false;
  mC.t_A = false;
  mC.t_B = false;
  mC.t_C = false;
  mC.t_D = false;
  mC.t_opciones = false;


#ifdef ESP32_S3_N16R8

  int xVal = analogRead(JOY_X);
  int yVal = analogRead(JOY_Y);

  int x2Val = analogRead(JOY2_X);
  int y2Val = analogRead(JOY2_Y);

  mC.joy_x1 = xVal;
  mC.joy_y1 = yVal;

  mC.joy_x2 = x2Val;
  mC.joy_y2 = y2Val;

  if (buttonPressed()) {
    mC.t_A = true;
  }
  if (buttonPressed2()) {
    mC.t_B = true;
  }
  if (buttonPressed3()) {
    mC.t_C = true;
  }
  if (buttonPressed4()) {
    mC.t_D = true;
  }

  if (xVal < VALOR_ANALOG - DEADZONE) {  // derecha
    //mC.t_arriba = true;
    moved = true;
  } else if (xVal > VALOR_ANALOG + DEADZONE) {  // izquierda
    //mC.t_abajo = true;
    moved = true;
  }
  if (yVal > VALOR_ANALOG + DEADZONE) {  // arriba
    //mC.t_izquierda = true;
    moved = true;
  } else if (yVal < VALOR_ANALOG - DEADZONE) {  // abajo
    //mC.t_derecha = true;
    moved = true;
  }

  if (x2Val < VALOR_ANALOG - DEADZONE) {  // derecha
    mC.t_abajo = true;
    moved = true;
  } else if (x2Val > VALOR_ANALOG + DEADZONE) {  // izquierda
    mC.t_arriba = true;
    moved = true;
  }
  if (y2Val > VALOR_ANALOG + DEADZONE) {  // arriba
    mC.t_derecha = true;
    moved = true;
  } else if (y2Val < VALOR_ANALOG - DEADZONE) {  // abajo
    mC.t_izquierda = true;
    moved = true;
  }

#else
  int xVal = analogRead(JOY_X);
  int yVal = analogRead(JOY_Y);

  mC.joy_x1 = xVal;
  mC.joy_y1 = yVal;

  if (buttonPressed()) {
    mC.t_A = true;
    moved = true;
  }
  if (buttonPressed2()) {
    mC.t_C = true;
    moved = true;
  }

  if (buttonPressed3()) {
    mC.t_B = true;
    moved = true;
  }
  if (buttonPressed4()) {
    mC.t_D = true;
    moved = true;
  }

  if (xVal < VALOR_ANALOG - DEADZONE) {  // derecha
    mC.t_arriba = true;
    moved = true;
  } else if (xVal > VALOR_ANALOG + DEADZONE) {  // izquierda
    mC.t_abajo = true;
    moved = true;
  }
  if (yVal > VALOR_ANALOG + DEADZONE) {  // arriba
    mC.t_izquierda = true;
    moved = true;
  } else if (yVal < VALOR_ANALOG - DEADZONE) {  // abajo
    mC.t_derecha = true;
    moved = true;
  }
#endif

  if (moved) {
    //delay(1);  // debounce movimiento joystick
  }
}


void setup() {
    Serial.begin(115200);
    pinMode(BTN, INPUT_PULLUP);   // Botón con resistencia pull-up
    pinMode(BTN2, INPUT_PULLUP);  // Botón con resistencia pull-up

    pinMode(BTN3, INPUT_PULLUP);  // Botón con resistencia pull-up
    pinMode(BTN4, INPUT_PULLUP);  // Botón con resistencia pull-up
    

    if (!lcd.begin()) {
        while(1) { vTaskDelay(1); }
    }

    smartWatchSetup();
}


void loop() {
    while (1) {
        updateJoystick();
        smartWatchLoop();
    }
}
