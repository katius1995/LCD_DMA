#pragma once

namespace BATALLA_GAME {

using ::lcd;
using ::mC;
using ::watch_screen;



/*

// Noche
.cieloArriba = 0x0804;
cieloAbajo  = 0x2AEB;
tierraArriba = 0x31E8;
tierraAbajo  = 0x10A2;

// Helado
.cieloArriba = 0x5296;
cieloAbajo  = 0xB6BB;
tierraArriba = 0xD6DD;
tierraAbajo  = 0x94B7;


// Caloroso
.cieloArriba = 0x19D9;
cieloAbajo  = 0x7DFB;
tierraArriba = 0xB48A;
tierraAbajo = 0x61E3;


// Lluvioso
.cieloArriba = 0x2267;
cieloAbajo  = 0x6398;
tierraArriba = 0x4C6A;
tierraAbajo = 0x20E3;

// atardecer
.cieloArriba = 0x18A5;   // azul oscuro
cieloAbajo  = 0xFCA0;   // naranja
tierraArriba = 0xA3A0;  // marrón/naranja
tierraAbajo = 0x4164;   // oscuro


// Amanecer
.cieloArriba = 0x3186;   // azul
cieloAbajo  = 0xFD20;   // naranja
tierraArriba = 0xC48A;  // café claro
tierraAbajo = 0x6204;   // café oscuro


// niebla
.cieloArriba = 0x632C;   // gris azulado
cieloAbajo  = 0xBDF7;   // gris claro
tierraArriba = 0x9CF3;  // gris verdoso
tierraAbajo = 0x5AEB;   // gris oscuro


// Tormenta
.cieloArriba = 0x1082;   // azul casi negro
cieloAbajo  = 0x4A49;   // gris azulado
tierraArriba = 0x4A04;  // marrón oscuro
tierraAbajo = 0x18C2;   // casi negro


// Tropical
.cieloArriba = 0x051F;   // azul intenso
cieloAbajo  = 0x7DFF;   // azul claro
tierraArriba = 0x5D28;  // verde/café
tierraAbajo = 0x29A3;   // verde oscuro


//Extremo calor
.cieloArriba = 0xA800;   // rojo oscuro
cieloAbajo  = 0xFD20;   // naranja
tierraArriba = 0xC440;  // naranja oscuro
tierraAbajo = 0x6200;   // café oscuro
*/


struct Clima {
    uint16_t cieloArriba;
    uint16_t cieloAbajo;
    uint16_t tierraArriba;
    uint16_t tierraAbajo;
    Vector3D sunDir;
    uint16_t ambientColor;
    float ambientIntensity;
    float sunIntensity;
};


Clima climas[10] = {
  // 0 - Noche
  {
    .cieloArriba = 0x0804, .cieloAbajo = 0x2AEB,
    .tierraArriba = 0x31E8, .tierraAbajo = 0x10A2,
    .sunDir = {400, 800, -400},
    .ambientColor = 0x2945, .ambientIntensity = 0.12f, .sunIntensity = 0.08f
  },
  // 1 - Helado
  {
    .cieloArriba = 0x5296, .cieloAbajo = 0xB6BB,
    .tierraArriba = 0xD6DD, .tierraAbajo = 0x94B7,
    .sunDir = {-700, 450, -600},
    .ambientColor = 0xBDF7, .ambientIntensity = 0.35f, .sunIntensity = 0.55f
  },
  // 2 - Caluroso
  {
    .cieloArriba = 0x19D9, .cieloAbajo = 0x7DFB,
    .tierraArriba = 0xB48A, .tierraAbajo = 0x61E3,
    .sunDir = {0, 1000, -150},
    .ambientColor = 0xFFF0, .ambientIntensity = 0.22f, .sunIntensity = 1.00f
  },
  // 3 - Lluvioso
  {
    .cieloArriba = 0x2267, .cieloAbajo = 0x6398,
    .tierraArriba = 0x4C6A, .tierraAbajo = 0x20E3,
    .sunDir = {-300, 550, -700},
    .ambientColor = 0x7BEF, .ambientIntensity = 0.30f, .sunIntensity = 0.35f
  },
  // 4 - Atardecer
  {
    .cieloArriba = 0x18A5, .cieloAbajo = 0xFCA0,
    .tierraArriba = 0xA3A0, .tierraAbajo = 0x4164,
    .sunDir = {800, 250, 400},
    .ambientColor = 0xFD20, .ambientIntensity = 0.20f, .sunIntensity = 0.60f
  },
  // 5 - Amanecer
  {
    .cieloArriba = 0x3186, .cieloAbajo = 0xFD20,
    .tierraArriba = 0xC48A, .tierraAbajo = 0x6204,
    .sunDir = {-800, 250, 400},
    .ambientColor = 0xFD20, .ambientIntensity = 0.22f, .sunIntensity = 0.65f
  },
  // 6 - Niebla
  {
    .cieloArriba = 0x632C, .cieloAbajo = 0xBDF7,
    .tierraArriba = 0x9CF3, .tierraAbajo = 0x5AEB,
    .sunDir = {-800, 250, 400},
    .ambientColor = 0xBDF7, .ambientIntensity = 0.45f, .sunIntensity = 0.25f
  },
  // 7 - Tormenta
  {
    .cieloArriba = 0x1082, .cieloAbajo = 0x4A49,
    .tierraArriba = 0x4A04, .tierraAbajo = 0x18C2,
    .sunDir = {100, 350, -800},
    .ambientColor = 0x4A69, .ambientIntensity = 0.18f, .sunIntensity = 0.15f
  },
  // 8 - Tropical
  {
    .cieloArriba = 0x051F, .cieloAbajo = 0x7DFF,
    .tierraArriba = 0x5D28, .tierraAbajo = 0x29A3,
    .sunDir = {-500, 800, -400},
    .ambientColor = 0xDFFB, .ambientIntensity = 0.25f, .sunIntensity = 0.95f
  },
  // 9 - Calor extremo
  {
    .cieloArriba = 0xA800, .cieloAbajo = 0xFD20,
    .tierraArriba = 0xC440, .tierraAbajo = 0x6200,
    .sunDir = {0, 980, 100},
    .ambientColor = 0xFDB0, .ambientIntensity = 0.18f, .sunIntensity = 1.00f
  }
};


Vector3D camaraBatalla = { 0, 0, 0 };
Vector3D pivotCamaraBatalla = { 0, 1000, 2000 };

const uint8_t MODELO_CAR = 6;
int clima_actual = 0;

void nothing(Obj *obj) {}

uint16_t n_obj = 11;

Obj obj[11] = {
{
.collision_mask = 1,
.id = "Curved001",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 1942,.f_end = 2338},
.grupo_id="Begin001",
.auto_lod = false,
},
{
.collision_mask = 1,
.id = "Begin001.001",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 1392,.f_end = 1546},
.auto_lod = false,
},
{
.collision_mask = 1,
.id = "Curved002",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 2338,.f_end = 2734},
.grupo_id="Curved003",
.auto_lod = false,
},
{
.collision_mask = 1,
.id = "Curved003",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 1546,.f_end = 1942},
.grupo_id="Curved004",
.auto_lod = false,
},
{
.collision_mask = 1,
.id = "Curved004",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 2734,.f_end = 3130},
.grupo_id="Straight005",
.auto_lod = false,
},
{
.collision_mask = 1,
.id = "Straight005",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 622,.f_end = 776},
.grupo_id="GoKart_1381.001",
.auto_lod = false,
},
{
.collision_mask = 1,
.id = "GoKart_1381",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 0,.f_end = 468},
.grupo_id="Curved001",
.auto_lod = false,
},
{
.collision_mask = 1,
.id = "GoKart_1381.001",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 776,.f_end = 1244},
.grupo_id="Straight005.001",
.auto_lod = false,
},
{
.collision_mask = 1,
.id = "Straight005.001",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 1244,.f_end = 1392},
.grupo_id="Begin001.001",
.auto_lod = false,
},
{
.collision_mask = 1,
.id = "Begin001",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 468,.f_end = 622},
.grupo_id="Curved002",
.auto_lod = false,
},
{
.id = "1Luz",
//.level = 1,
.type = LUZSPOT,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 0,.f_end = 468},
.grupo_id="2Luz.001",
.luz = {
    .encendido = true,
    .intensidad = 1000,   // 1.0 (el motor usa escala 1/1000 y limita a 1.0)
    .alcance = 50000,     // 25000 unidades de mundo (~5 celdas)
    .angulo = 90000,      // cono de 60 grados
    .suavidad = 1000,     // exponente 1.5: borde del haz suave
    .direccion = { 0, 0, -FIXED_SCALE },  // se recalcula cada frame
    .color = 0xFFFF
  }
}
};





const int horizonte = LCD_HEIGHT * 0.55f;


// ---------------------------------------------------------------------------
// CONTROL DEL VEHICULO
// ---------------------------------------------------------------------------
// El rumbo del carro usa una convención simple:
//   0°   = +Z
//   90°  = +X
//   180° = -Z
//   270° = -X
//
// El modelo GoKart tiene su frente sobre +X local, por eso su rotación Y es
// rumbo - 90°. Esta separación evita mezclar "rumbo del vehículo" con el
// ángulo interno del modelo.
// ---------------------------------------------------------------------------

const int32_t CAR_SPEED = 500;          // unidades de mundo por frame con A
const int32_t CAR_TURN_SPEED = 10;       // grados por frame con el joystick
const int32_t CAR_MIN_X = 0;
const int32_t CAR_MIN_Z = 0;
const int32_t CAR_MAX_X = (MAP_W - 1) * 4660;
const int32_t CAR_MAX_Z = (MAP_H - 1) * 4660;

// Distancia y altura de la cámara respecto al carro.
const int32_t CAMERA_BEHIND = 4500;
const int32_t CAMERA_HEIGHT = 2000;

// Posición inicial sobre el primer tramo de carretera.
int32_t carroX = 4 * 4660;
int32_t carroZ = 3 * 4660;
int32_t carroY = 0;

// Rumbo del carro: 0° apunta hacia +Z.
// El modelo se dibuja con rotation.y = rumbo - 90°.
int32_t rumboCarro = 0;

// Faro del carro.
const uint8_t INDICE_LUZ_CARRO = 10;   // posición del LUZSPOT en obj[]
const int32_t FARO_ADELANTE  = 900;    // unidades delante del centro del carro
const int32_t FARO_ALTURA    = 900;    // unidades sobre el centro del carro
const int16_t FARO_INCLINACION = 30;   // grados que el haz apunta hacia abajo

// ---------------------------------------------------------------------------
// Actualiza la transformación de las dos piezas que forman el GoKart.
// ---------------------------------------------------------------------------
inline void actualizarCarro() {
  const int32_t rotModelo = rumboCarro + 90;

  obj[MODELO_CAR].traslation.x = carroX;
  obj[MODELO_CAR].traslation.y = carroY;
  obj[MODELO_CAR].traslation.z = carroZ;
  obj[MODELO_CAR].rotation.y = rotModelo;
  obj[MODELO_CAR].rotation.x = 0;
  obj[MODELO_CAR].rotation.z = 0;
  obj[MODELO_CAR].visible = true;

  // La segunda malla es la otra parte del mismo carro.
  obj[MODELO_CAR + 1].traslation.x = carroX;
  obj[MODELO_CAR + 1].traslation.y = carroY;
  obj[MODELO_CAR + 1].traslation.z = carroZ;
  obj[MODELO_CAR + 1].rotation.y = rotModelo;
  obj[MODELO_CAR + 1].rotation.x = 0;
  obj[MODELO_CAR + 1].rotation.z = 0;
  obj[MODELO_CAR + 1].visible = true;

  // -------------------------------------------------------------------------
  // FARO (obj[INDICE_LUZ_CARRO], LUZSPOT) que sigue al carro.
  //
  // Posición del mundo de la luz = obj.position + obj.traslation. Como la luz
  // usa el mismo rango de caras que el GoKart, su position es el centroide del
  // carro; por eso basta sumar carro + avance + altura a la traslación.
  //
  // Convención del motor para LUZSPOT: luz.direccion es el vector que va de la
  // superficie iluminada HACIA la luz (el motor calcula dot(luz-punto, dir)).
  // Es decir, es el opuesto a la dirección en la que sale el haz. Por eso se
  // niega el frente del carro. La componente Y positiva inclina el haz hacia
  // abajo para que el cono llegue al camino y no pase por encima de él.
  // -------------------------------------------------------------------------
  const int32_t frenteX = fixed_sin(rumboCarro);   // sin(rumbo)
  const int32_t frenteZ = fixed_cos(rumboCarro);   // cos(rumbo)
  const int32_t cosInc  = fixed_cos(FARO_INCLINACION);
  const int32_t sinInc  = fixed_sin(FARO_INCLINACION);

  Obj &faro = obj[INDICE_LUZ_CARRO];

  faro.visible = true;            // el motor ignora luces con visible=false
  faro.luz.encendido = true;

  faro.luz.direccion.x = -(int32_t)(((int64_t)frenteX * cosInc) >> FIXED_SHIFT);
  faro.luz.direccion.y =  sinInc;
  faro.luz.direccion.z = -(int32_t)(((int64_t)frenteZ * cosInc) >> FIXED_SHIFT);

  faro.traslation.x = carroX + (int32_t)(((int64_t)frenteX * FARO_ADELANTE) >> FIXED_SHIFT);
  faro.traslation.y = carroY + FARO_ALTURA;
  faro.traslation.z = carroZ + (int32_t)(((int64_t)frenteZ * FARO_ADELANTE) >> FIXED_SHIFT);
}

// ---------------------------------------------------------------------------
// Mueve el carro y coloca la cámara exactamente detrás de él.
// ---------------------------------------------------------------------------
void moverCamara() {

  // ---------------------------------------------------------
  // 1. GIRO CON EL JOYSTICK
  // ---------------------------------------------------------
  if (mC.t_derecha)
    rumboCarro += CAR_TURN_SPEED;

  if (mC.t_izquierda)
    rumboCarro -= CAR_TURN_SPEED;

  if (rumboCarro >= 360)
    rumboCarro -= 360;

  if (rumboCarro < 0)
    rumboCarro += 360;

  // ---------------------------------------------------------
  // 2. AVANCE SOLAMENTE CON LA TECLA A
  // ---------------------------------------------------------
  if (mC.t_A) {
    // Para rumbo 0° el frente es +Z:
    //   X = sin(rumbo)
    //   Z = cos(rumbo)
    const int32_t dirX = fixed_sin(rumboCarro);
    const int32_t dirZ = fixed_cos(rumboCarro);

    carroX += (int32_t)(((int64_t)dirX * CAR_SPEED) >> FIXED_SHIFT);
    carroZ += (int32_t)(((int64_t)dirZ * CAR_SPEED) >> FIXED_SHIFT);
  }

  // ---------------------------------------------------------
  // 3. LIMITES DEL MAPA
  // ---------------------------------------------------------
  if (carroX < CAR_MIN_X) carroX = CAR_MIN_X;
  if (carroZ < CAR_MIN_Z) carroZ = CAR_MIN_Z;
  if (carroX > CAR_MAX_X) carroX = CAR_MAX_X;
  if (carroZ > CAR_MAX_Z) carroZ = CAR_MAX_Z;

  actualizarCarro();

  // ---------------------------------------------------------
  // 4. CÁMARA DETRÁS DEL CARRO
  // ---------------------------------------------------------
  //
  // El carro apunta en:
  //   forward = (sin(rumbo), 0, cos(rumbo))
  //
  // La cámara se coloca en:
  //   carro - forward * CAMERA_BEHIND
  //
  // De esta forma, cuando el carro gira, la cámara gira y se desplaza
  // con él. Nunca se queda orientada al eje global.
  //
  const int32_t dirX = fixed_sin(rumboCarro);
  const int32_t dirZ = fixed_cos(rumboCarro);

  const int32_t behindX =
    (int32_t)(((int64_t)dirX * CAMERA_BEHIND) >> FIXED_SHIFT);

  const int32_t behindZ =
    (int32_t)(((int64_t)dirZ * CAMERA_BEHIND) >> FIXED_SHIFT);

  pivotCamaraBatalla.x = carroX + obj[MODELO_CAR].position.x - behindX;
  pivotCamaraBatalla.y = carroY + obj[MODELO_CAR].position.y + CAMERA_HEIGHT;
  pivotCamaraBatalla.z = carroZ + obj[MODELO_CAR].position.z - behindZ;

  // La cámara mira hacia el frente del carro.
  // buildMatrixCamera trabaja con la convención del espacio de vista
  // utilizada por el rasterizador, por eso el yaw de cámara es 180-rumbo.
  int32_t yawCamara = 180 - rumboCarro;
  if (yawCamara < 0) yawCamara += 360;

  //camaraBatalla.x = INT_TO_FIXED(35);
  camaraBatalla.y = INT_TO_FIXED(yawCamara);
  camaraBatalla.z = 0;

}


void juegoSetup() {
  for (int i = 0; i < n_obj; i++) {
    lcd.getCentroid(obj[i], 1);
  }

  carroX = 4 * 4660;
  carroZ = 3 * 4660;
  rumboCarro = 0;

  clima_actual = random(0, 10);

  lcd.setLightDir(climas[clima_actual].sunDir);
  lcd.setEnvironmentLighting(
      climas[clima_actual].ambientColor,
      climas[clima_actual].ambientIntensity,
      climas[clima_actual].sunIntensity
  );

  actualizarCarro();
  moverCamara();

  lcd.configInitGame(camaraBatalla);
  lcd.beginGameLighting(obj, n_obj);
  lcd.setBufferingZ(true);
  lcd.setShadows(false);
}


int separacionX = 4660;
int separacionZ = 4660;

// ---------------------------------------------------------
// CANTIDAD DE CELDAS VISIBLES
// ---------------------------------------------------------
int columnas = 10;
int filas = 8;

void juegoLoop() {
  // B siempre regresa al menú desde la prueba 3D.
  if (mC.t_B) {
    watch_screen = STATE_MENU;
    return;
  }

  // --- CIELO: azul oscuro arriba -> azul claro en el horizonte ---
  for (int y = 0; y < horizonte; y++) {

    float t = (float)y / (float)(horizonte - 1);

    // Extraer RGB565
    uint8_t r1 = ((climas[clima_actual].cieloArriba >> 11) & 0x1F) << 3;
    uint8_t g1 = ((climas[clima_actual].cieloArriba >> 5)  & 0x3F) << 2;
    uint8_t b1 = (climas[clima_actual].cieloArriba & 0x1F) << 3;

    uint8_t r2 = ((climas[clima_actual].cieloAbajo >> 11) & 0x1F) << 3;
    uint8_t g2 = ((climas[clima_actual].cieloAbajo >> 5)  & 0x3F) << 2;
    uint8_t b2 = (climas[clima_actual].cieloAbajo & 0x1F) << 3;

    uint8_t r = r1 + (uint8_t)((r2 - r1) * t);
    uint8_t g = g1 + (uint8_t)((g2 - g1) * t);
    uint8_t b = b1 + (uint8_t)((b2 - b1) * t);

    uint16_t color =
        ((r >> 3) << 11) |
        ((g >> 2) << 5)  |
        (b >> 3);

    lcd.drawLine(
        0, y,
        LCD_WIDTH - 1, y,
        color
    );
  }


  // ============================================================
  // TIERRA
  // ============================================================

  for (int y = horizonte; y < LCD_HEIGHT; y++) {

    float t = (float)(y - horizonte) /
              (float)(LCD_HEIGHT - horizonte - 1);

    uint8_t r1 = ((climas[clima_actual].tierraArriba >> 11) & 0x1F) << 3;
    uint8_t g1 = ((climas[clima_actual].tierraArriba >> 5)  & 0x3F) << 2;
    uint8_t b1 = (climas[clima_actual].tierraArriba & 0x1F) << 3;

    uint8_t r2 = ((climas[clima_actual].tierraAbajo >> 11) & 0x1F) << 3;
    uint8_t g2 = ((climas[clima_actual].tierraAbajo >> 5)  & 0x3F) << 2;
    uint8_t b2 = (climas[clima_actual].tierraAbajo & 0x1F) << 3;

    uint8_t r = r1 + (uint8_t)((r2 - r1) * t);
    uint8_t g = g1 + (uint8_t)((g2 - g1) * t);
    uint8_t b = b1 + (uint8_t)((b2 - b1) * t);

    uint16_t color =
        ((r >> 3) << 11) |
        ((g >> 2) << 5)  |
        (b >> 3);

    lcd.drawLine(
        0, y,
        LCD_WIDTH - 1, y,
        color
    );
  }

  // A avanza, joystick gira y la cámara se reposiciona detrás.
  moverCamara();

  // ---------------------------------------------------------
  // OCULTAR TODOS LOS OBJETOS
  // ---------------------------------------------------------
  // Las luces NO se ocultan: el motor ignora cualquier luz con visible=false.
  for (int i = 0; i < n_obj; i++) {
    if (obj[i].type == BODY)
      obj[i].visible = false;
  }

  // ---------------------------------------------------------
  // CELDA CENTRAL DEL ÁREA VISIBLE
  // ---------------------------------------------------------
  int cellX = pivotCamaraBatalla.x / separacionX;
  int cellZ = pivotCamaraBatalla.z / separacionZ;

  // Como ahora los objetos se almacenan en coordenadas de mundo, no
  // necesitamos convertirlos a coordenadas relativas a la cámara.
  //
  // Se recorre un área alrededor de la cámara para que también funcione
  // cuando el carro gire 90°/180° y la cámara mire en otra dirección.
  const int mitadColumnas = columnas / 2;
  const int mitadFilas = filas / 2;

  for (int j = 0; j < filas; j++) {
    int mapZ = cellZ + j - mitadFilas;

    if (mapZ < 0 || mapZ >= MAP_H)
      continue;

    for (int i = 0; i < columnas; i++) {
      int mapX = cellX + i - mitadColumnas;

      if (mapX < 0 || mapX >= MAP_W)
        continue;

      uint8_t sp = mapaPueblo[mapZ][mapX];

      // El mapa utiliza únicamente los objetos 0..5.
      // MODELO_CAR y MODELO_CAR+1 son exclusivamente el GoKart.
      if (sp == 0 || sp > n_obj)
        continue;

      Obj &pieza = obj[sp - 1];

      pieza.traslation.x =
        -pieza.position.x + mapX * separacionX;

      pieza.traslation.y = 0;

      pieza.traslation.z =
        -pieza.position.z + mapZ * separacionZ;

      pieza.visible = true;

      lcd.drawSceneGameObjectV2(
        pieza,
        0,
        0,
        0,
        camaraBatalla,
        pivotCamaraBatalla,
        false,
        INT_TO_FIXED(1));
    }
  }

  // ---------------------------------------------------------
  // DIBUJAR EL CARRO
  // ---------------------------------------------------------
  actualizarCarro();

  lcd.drawSceneGameObjectV2(
    obj[MODELO_CAR],
    0,
    0,
    0,
    camaraBatalla,
    pivotCamaraBatalla,
    false,
    INT_TO_FIXED(1));

  lcd.drawSceneGameObjectV2(
    obj[MODELO_CAR + 1],
    0,
    0,
    0,
    camaraBatalla,
    pivotCamaraBatalla,
    false,
    INT_TO_FIXED(1));
}



/*
1=pasto 
2=tierra 
3=pavimento, carretera 
4=piso de casa 
5=paredes de casa 
6=puerta casa 
7=ventana casa 
8=arbusto 
9=anden 
10=mesa de madera
11=poste de luz
12=jugador
13=zombie
*/

/*uint16_t n_obj = 34;

Obj obj[34] = {
  {
    .collision_mask = 1,
    .id = "pasto_claro_simple",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 17399, .f_end = 17417 },
    .grupo_id = "costa_simple",
  },
  {
    .collision_mask = 1,
    .id = "pasto_simple",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 17395, .f_end = 17399 },
    .grupo_id = "pasto_claro_simple",

  },
  {
    .collision_mask = 1,
    .id = "agua_simple",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 17389, .f_end = 17395 },
    .grupo_id = "pasto_simple",

  },
  {
    .collision_mask = 1,
    .id = "Arbol",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 3880, .f_end = 3930 },
    .grupo_id = "Torre",

  },
  {
    .collision_mask = 1,
    .id = "pared",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 2556, .f_end = 3060 },
    .grupo_id = "pared_puerta_derecha",

  },
  {
    .collision_mask = 1,
    .id = "hex_pasto_claro",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 0, .f_end = 36 },
    .grupo_id = "hex_pasto",

  },
  {
    .collision_mask = 1,
    .id = "hex_pasto",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 36, .f_end = 56 },
    .grupo_id = "hex_agua",

  },
  {
    .collision_mask = 1,
    .id = "hex_agua",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 56, .f_end = 80 },
    .grupo_id = "hex_costa",

  },
  {
    .collision_mask = 1,
    .id = "costa_simple",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 17417, .f_end = 17491 },
    .grupo_id = "arboles",

  },
  {
    .collision_mask = 1,
    .id = "hex_costa",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 80, .f_end = 178 },
    .grupo_id = "bodega_cajas",

  },
  {
    .collision_mask = 1,
    .id = "bodega_cajas",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 178, .f_end = 1396 },
    .grupo_id = "pared_esquina_puerta_izq",

  },
  {
    .collision_mask = 1,
    .id = "pared_esquina_puerta_izq",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 1396, .f_end = 1636 },
    .grupo_id = "pared_esquina_puerta",

  },
  {
    .collision_mask = 1,
    .id = "pared_esquina_puerta",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 1636, .f_end = 1952 },
    .grupo_id = "pared_esquina_puerta_der",

  },
  {
    .collision_mask = 1,
    .id = "pared_esquina_puerta_der",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 1952, .f_end = 2192 },
    .grupo_id = "pared_esquina_simple",

  },
  { .collision_mask = 1,
    .id = "arboles",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 17491, .f_end = 17841 } },
  {
    .collision_mask = 1,
    .id = "MontanaRoca",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 16639, .f_end = 16927 },
    .grupo_id = "ValleArboles",

  },
  {
    .collision_mask = 1,
    .id = "ValleArboles",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 16927, .f_end = 17333 },
    .grupo_id = "bandera",

  },
  {
    .collision_mask = 1,
    .id = "casa",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 4694, .f_end = 5264 },
    .grupo_id = "Castillo",

  },
  {
    .collision_mask = 1,
    .id = "Castillo",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 5264, .f_end = 8922 },
    .grupo_id = "Arqueria",

  },
  {
    .collision_mask = 1,
    .id = "pared_esquina_simple",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 2192, .f_end = 2556 },
    .grupo_id = "pared",

  },
  {
    .collision_mask = 1,
    .id = "pared_puerta_derecha",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 3060, .f_end = 3300 },
    .grupo_id = "pared_puerta",

  },
  {
    .collision_mask = 1,
    .id = "pared_puerta",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 3300, .f_end = 3640 },
    .grupo_id = "pared_puerta_izquiera",

  },
  {
    .collision_mask = 1,
    .id = "pared_puerta_izquiera",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 3640, .f_end = 3880 },
    .grupo_id = "Arbol",

  },

  {
    .collision_mask = 1,
    .id = "Torre",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 3930, .f_end = 4694 },
    .grupo_id = "casa",

  },
  {
    .collision_mask = 1,
    .id = "Arqueria",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 8922, .f_end = 12929 },
    .grupo_id = "roca",

  },
  {
    .collision_mask = 1,
    .id = "roca",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 12929, .f_end = 12951 },
    .grupo_id = "pared_esquina_90",

  },
  {
    .collision_mask = 1,
    .id = "pared_esquina_90",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 12951, .f_end = 13255 },
    .grupo_id = "valla_madera",

  },
  {
    .collision_mask = 1,
    .id = "valla_madera",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 13255, .f_end = 13467 },
    .grupo_id = "puerta_madera.001",

  },
  {
    .collision_mask = 1,
    .id = "puerta_madera.001",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 13467, .f_end = 13575 },
    .grupo_id = "puerta_madera",

  },
  {
    .collision_mask = 1,
    .id = "puerta_madera",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 13575, .f_end = 13775 },
    .grupo_id = "catapult_arm_blue",

  },
  {
    .collision_mask = 1,
    .id = "catapult_arm_blue",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 13775, .f_end = 14175 },
    .grupo_id = "catapulta",

  },
  {
    .collision_mask = 1,
    .id = "catapulta",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 14175, .f_end = 15101 },
    .grupo_id = "torre_catapulta",

  },
  {
    .collision_mask = 1,
    .id = "torre_catapulta",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 15101, .f_end = 16639 },
    .grupo_id = "MontanaRoca",

  },
  {
    .collision_mask = 1,
    .id = "bandera",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 17333, .f_end = 17389 },
    .grupo_id = "agua_simple",

  }
};

/*{{
.collision_mask = 1,
.id = "Avion",
//.level = 1,
.type = BODY,
.visible = false,
.onloop = nothing,
.mtl{.f_init = 0,.f_end = 170},
.grupo_id="Bala",

},
{
.collision_mask = 1,
.id = "Bala",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 170,.f_end = 194},
.grupo_id="Mar",

},
{
.collision_mask = 1,
.id = "Mar",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 194,.f_end = 540},
.grupo_id="NavalG.001",
.is_ground=true,

},
{
.collision_mask = 1,
.id = "NavalG.001",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 540,.f_end = 780},
.grupo_id="NavalG.002",

},
{
.collision_mask = 1,
.id = "NavalG.002",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 780,.f_end = 1020},
.grupo_id="NavalM.001",

},
{
.collision_mask = 1,
.id = "NavalM.001",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 1020,.f_end = 1246},
.grupo_id="NavalM.002",

},
{
.collision_mask = 1,
.id = "NavalM.002",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 1246,.f_end = 1486},
.grupo_id="NavalM.003",

},
{
.collision_mask = 1,
.id = "NavalM.003",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 1486,.f_end = 1726},
.grupo_id="NavalP.001",

},
{
.collision_mask = 1,
.id = "NavalP.001",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 1726,.f_end = 1966},
.grupo_id="NavalP.002",

},
{
.collision_mask = 1,
.id = "NavalP.002",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 1966,.f_end = 2206},
.grupo_id="NavalP.003",

},
{
.collision_mask = 1,
.id = "NavalP.003",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 2206,.f_end = 2446},
.grupo_id="NavalP.004",

},
{
.collision_mask = 1,
.id = "NavalP.004",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 2446,.f_end = 2686},
.grupo_id="Mar.001",

},
{
.collision_mask = 1,
.id = "Mar.001",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 2686,.f_end = 3032},
.grupo_id="Selector",
.is_ground=true,
},
{
.collision_mask = 1,
.id = "Selector",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing
.mtl{.f_init = 3032,.f_end = 3044}}
};
int32_t dx = 100;
int32_t dz = 100;

int paso = 3;

bool izquierda = false, derecha = false;

void moverCamara() {

  derecha = false;
  izquierda = false;
  if (mC.t_derecha) {
    dx -= paso;
    derecha = true;
  }
  if (mC.t_izquierda) {
    dx += paso;
    izquierda = true;
  }
  if (mC.t_abajo) dz -= paso;
  if (mC.t_arriba) dz += paso;

  if (dx < 0)
    dx = 0;

  if (dz < 0)
    dz = 0;
}

void juegoSetup() {
  for (int i = 0; i < n_obj; i++) {
    lcd.getCentroid(obj[i], 1);
  }
  lcd.configInitGame(camaraBatalla);
  lcd.setBufferingZ(true);
  lcd.setShadows(false);
}

void juegoLoop() {
  // B siempre regresa al menú desde la prueba 3D.
  if (mC.t_B) {
    watch_screen = STATE_MENU;
    return;
  }
  lcd.clear(0x31a6);

  moverCamara();

  int valorEscalado = 25;

  // ---------------------------------------------------------
  // TAMAÑO DE CADA HEXÁGONO EN EL MUNDO
  // ---------------------------------------------------------
  int separacionX = 2000;
  int separacionZ = 2000 - 265;

  // ---------------------------------------------------------
  // CELDA DE LA CÁMARA
  // ---------------------------------------------------------
  int cellX = dx / valorEscalado;
  int cellZ = dz / valorEscalado;

  int offsetx = dx % valorEscalado;
  int offsetz = dz % valorEscalado;

  // ---------------------------------------------------------
  // OCULTAR TODOS LOS OBJETOS
  // ---------------------------------------------------------
  for (int i = 0; i < n_obj; i++) {
    obj[i].visible = false;
  }

  // ---------------------------------------------------------
  // CANTIDAD DE CELDAS VISIBLES
  // ---------------------------------------------------------
  int columnas = 6;
  int filas = 8;

  // ---------------------------------------------------------
  // CENTRO DE LA PANTALLA
  // ---------------------------------------------------------
  int mitad = columnas / 2;
  if (derecha)
    mitad = 0;

  if (izquierda)
    mitad = columnas;

  // =========================================================
  // RECORRIDO DE HEXÁGONOS
  //
  // MITAD IZQUIERDA : izquierda -> derecha
  // MITAD DERECHA   : derecha -> izquierda
  //
  // Las dos mitades se procesan en paralelo.
  // =========================================================

  for (int j = 0; j < filas; j++) {
    int mapZ = cellZ + j;

    if (mapZ < 0 || mapZ >= MAP_W)
      continue;

    // -----------------------------------------------------
    // DESFASE DE FILAS PARA HEXÁGONOS
    //
    // Una fila se desplaza respecto a la siguiente.
    // -----------------------------------------------------
    int offsetHex = (mapZ & 1) ? separacionX / 2 : 0;

    // =====================================================
    // MITAD DERECHA
    // derecha -> izquierda
    // =====================================================

    for (int i = columnas - 1; i >= mitad; i--) {
      int mapX = cellX + i;

      if (mapX < 0 || mapX >= MAP_W)
        continue;

      uint8_t sp = mapaPueblo[mapZ][mapX];

      if (sp == 0 || sp > n_obj)
        continue;

      int posX = i * separacionX + offsetHex;
      int posZ = j * separacionZ;

      if (sp >= 15 || sp == 4 || sp == 5) {
        obj[0].traslation.x =
          -obj[0].position.x
          + posX
          - offsetx * separacionX / 25;

        obj[0].traslation.z =
          -obj[0].position.z
          + posZ
          - offsetz * separacionZ / 25;

        obj[0].visible = true;

        lcd.drawSceneGameObjectV2(
          obj[0],
          0,
          0,
          0,
          camaraBatalla,
          pivotCamaraBatalla,
          false,
          INT_TO_FIXED(1));
      }
      //lcd.sortLocalObjectFaces(obj[sp - 1], pivotCamaraBatalla);

      obj[sp - 1].traslation.x =
        -obj[sp - 1].position.x
        + posX
        - offsetx * separacionX / 25;

      obj[sp - 1].traslation.z =
        -obj[sp - 1].position.z
        + posZ
        - offsetz * separacionZ / 25;

      obj[sp - 1].visible = true;

      lcd.drawSceneGameObjectV2(
        obj[sp - 1],
        0,
        0,
        0,
        camaraBatalla,
        pivotCamaraBatalla,
        false,
        INT_TO_FIXED(1));
    }

    // =====================================================
    // MITAD IZQUIERDA
    // izquierda -> derecha
    // =====================================================

    for (int i = 0; i < mitad; i++) {
      int mapX = cellX + i;

      if (mapX < 0 || mapX >= MAP_W)
        continue;

      uint8_t sp = mapaPueblo[mapZ][mapX];

      if (sp == 0)
        continue;

      int posX = i * separacionX + offsetHex;
      int posZ = j * separacionZ;


      if (sp >= 15 || sp == 4 || sp == 5) {
        obj[0].traslation.x =
          -obj[0].position.x
          + posX
          - offsetx * separacionX / 25;

        obj[0].traslation.z =
          -obj[0].position.z
          + posZ
          - offsetz * separacionZ / 25;

        obj[0].visible = true;

        lcd.drawSceneGameObjectV2(
          obj[0],
          0,
          0,
          0,
          camaraBatalla,
          pivotCamaraBatalla,
          false,
          INT_TO_FIXED(1));
      }
      //lcd.sortLocalObjectFaces(obj[sp - 1], pivotCamaraBatalla);

      obj[sp - 1].traslation.x =
        -obj[sp - 1].position.x
        + posX
        - offsetx * separacionX / 25;

      obj[sp - 1].traslation.z =
        -obj[sp - 1].position.z
        + posZ
        - offsetz * separacionZ / 25;

      obj[sp - 1].visible = true;

      lcd.drawSceneGameObjectV2(
        obj[sp - 1],
        0,
        0,
        0,
        camaraBatalla,
        pivotCamaraBatalla,
        false,
        INT_TO_FIXED(1));
    }
  }
}*/

}