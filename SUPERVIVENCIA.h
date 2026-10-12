#pragma once

namespace SUPERVIVENCIA_GAME {

using ::lcd;
using ::mC;
using ::watch_screen;


Vector3D camaraSupervivencia = { INT_TO_FIXED(40), 0, 0 };
Vector3D pivotCamaraSupervivencia = { 0, 7500, 0 };


#define MAX_ZOMBIES 20
#define MAX_PICKUPS 16
#define MAX_SLOTS_INVENTARIO 7

#define SMART_ACCENT 0x03FF

const uint8_t MODELO_ALBERGUE = 15;
const uint8_t MODELO_PERSONA = 14;
const uint8_t MODELO_ZOMBIE = 12;
const uint8_t MODELO_PLATAFORMA = 6;
const uint8_t MODELO_CARRO = 7;
const uint8_t MODELO_JUGADOR = 11;
const uint8_t MODELO_BAUL = 13;


const int32_t ZONA_MIN_X = -140000;
const int32_t ZONA_MAX_X = 140000;
const int32_t ZONA_MIN_Z = -140000;
const int32_t ZONA_MAX_Z = 140000;

const int32_t VELOCIDAD_JUGADOR = 420;

const int32_t RADIO_COLISION_JUGADOR = 300;
const int32_t RADIO_COLISION_ZOMBIE = 300;
const int32_t SEPARACION_COLISION = 300;

// Zombies.
// NOTA: estos valores son los del tipo NORMAL. Desde que existen varios
// tipos de zombie, las estadisticas reales se leen de las tablas
// *_POR_TIPO definidas mas abajo (VELOCIDAD_POR_TIPO, DANIO_POR_TIPO, etc).
const float VELOCIDAD_ZOMBIE = 0.55f;
const float DISTANCIA_DETECCION_ZOMBIE = 10000;
const float DISTANCIA_PERDIDA_ZOMBIE = 18000;
const float RADIO_ATAQUE_ZOMBIE = 0.65f;
const float DANIO_ZOMBIE = 1.0f;
const unsigned long INTERVALO_ATAQUE_ZOMBIE = 650;

unsigned long destelloDanioHasta = 0;

// Armas.
const float DANIO_CUCHILLO = 28.0f;
const float ALCANCE_CUCHILLO = 2.8f;
const float DANIO_PISTOLA = 38.0f;
const float ALCANCE_PISTOLA = 20.0f;
const float DANIO_ESCOPETA = 75.0f;
const float ALCANCE_ESCOPETA = 13.0f;
const float COS_ANGULO_ATAQUE = 0.45f;
const unsigned long COOLDOWN_CUCHILLO = 350;
const unsigned long COOLDOWN_PISTOLA = 430;
const unsigned long COOLDOWN_ESCOPETA = 850;

// Carro: aceleracion + direccion + inercia.
const int32_t CARRO_ACELERACION = 45;
const int32_t CARRO_FRENADO = 60;
const int32_t CARRO_FRENADO_SUAVE = 12;
const int32_t CARRO_VELOCIDAD_MAX = 950;
const int32_t CARRO_VELOCIDAD_REVERSA = 420;
const int32_t CARRO_GIRO = 45;
const float RADIO_CARRO = 3.8f;
const float RADIO_ATROPELLO_CARRO = 2.15f;
const int32_t VELOCIDAD_MIN_ATROPELLO = 180;

// Objetos recogibles.
const float RADIO_RECOGIDA = 2.7f;

// Baul del carro: guardar/sacar objetos del inventario.
const float RADIO_BAUL = 3.0f;

// ================================================================
// Albergue: vida, defensa y mejoras
// ================================================================
const float VIDA_ALBERGUE_BASE = 100.0f;
const float REPARACION_ALBERGUE = 25.0f;
const uint16_t COSTO_REPARACION_ALBERGUE = 1;  // 1 botiquin por reparacion
const uint8_t NIVEL_MAX_ALBERGUE = 5;
const uint8_t NIVEL_MAX_BAUL = 5;
const uint8_t NIVEL_MAX_CARRO = 5;
const uint16_t CAPACIDAD_BAUL_BASE = 20;
const uint16_t CAPACIDAD_BAUL_POR_NIVEL = 10;

// Hordas: cada oleada se vuelve progresivamente mas grande y resistente.
const unsigned long PRIMERA_HORDA_MS = 300000UL;    // 5 minutos
const unsigned long INTERVALO_HORDA_MS = 600000UL;  // 10 minutos
const uint8_t ZOMBIES_INICIALES = 8;
const uint8_t ZOMBIES_POR_HORDA_BASE = 3;

// Personas a rescatar. El objetivo total permanece en 5.
const uint8_t MAX_PERSONAS_RESCATE = 5;
const float RADIO_RECOGIDA_PERSONA = 3.0f;
const float RADIO_ENTREGA_ALBERGUE = 5.0f;


// Veneno del zombie toxico (dano continuo tras el golpe).
const unsigned long DURACION_VENENO = 4000;
const unsigned long INTERVALO_TIC_VENENO = 800;
const float DANIO_VENENO_TIC = 2.0f;

// Dia/noche.
const unsigned long DURACION_DIA_MS = 180000UL;
unsigned long inicioCicloDia = 0;
float horaJuego = 8.0f;
bool esNoche = false;

// ================================================================
// Estado jugador
// ================================================================

struct EstadoJugadorZombie {
  float vida;
  bool vivo;
  bool enCarro;
  float dirX;
  float dirZ;
  float angulo;
  float velocidadCarro;

  // Veneno (aplicado por el zombie toxico).
  bool envenenado;
  unsigned long finVeneno;
  unsigned long siguienteTicVeneno;
};

EstadoJugadorZombie jugadorZombie = {
  100.0f, true, false,
  0.0f, -1.0f,
  0.0f, 0.0f,
  false, 0UL, 0UL
};

// ================================================================
// Armas
// ================================================================

enum ArmaZombie {
  ARMA_CUCHILLO = 0,
  ARMA_PISTOLA = 1,
  ARMA_ESCOPETA = 2
};

ArmaZombie armaEquipada = ARMA_PISTOLA;

const char *nombreArma(ArmaZombie arma) {
  switch (arma) {
    case ARMA_CUCHILLO: return "CUCHILLO";
    case ARMA_PISTOLA: return "PISTOLA";
    case ARMA_ESCOPETA: return "ESCOPETA";
  }
  return "ARMA";
}

// ================================================================
// Inventario
// ================================================================

enum TipoItemZombie {
  ITEM_BOTIQUIN,
  ITEM_MUNICION,
  ITEM_COMIDA,
  ITEM_AGUA
};

struct ItemInventarioZombie {
  const char *nombre;
  uint16_t cantidad;
};

ItemInventarioZombie inventarioZombie[] = {
  { "Botiquin", 2 },
  { "Municion", 18 },
  { "Comida", 3 },
  { "Agua", 4 }
};

const uint8_t NUM_ITEMS_INVENTARIO =
  sizeof(inventarioZombie) / sizeof(inventarioZombie[0]);

// 0-2 armas, 3-6 objetos.
uint8_t slotInventario = 1;
bool inventarioAbierto = false;
bool mapaInventario = false;

// ================================================================
// Baul (almacen del carro)
// ================================================================
// Guarda las mismas 4 categorias de objetos consumibles que el
// inventario del jugador (no se pueden guardar armas).

uint16_t baulZombie[4] = { 0, 0, 0, 0 };

// Mejoras persistentes de la base y del carro.
uint8_t nivelAlbergue = 0;
uint8_t nivelBaul = 0;
uint8_t nivelCarro = 0;
float vidaAlbergue = VIDA_ALBERGUE_BASE;

bool mejorasAbiertas = false;
uint8_t mejoraSeleccionada = 0;  // 0=defensa, 1=baul, 2=carro

bool baulAbierto = false;
uint8_t slotBaul = 0;  // Indice dentro de baulZombie/inventarioZombie (0-3).
uint8_t ladoBaul = 0;  // 0 = mochila activa, 1 = baul activo.

bool menuAbierto() {
  return inventarioAbierto || baulAbierto;
}


// ================================================================
// Zombies
// ================================================================

enum EstadoIAZombie {
  ZOMBIE_QUIETO,
  ZOMBIE_PERSIGUIENDO,
  ZOMBIE_REGRESANDO
};

// ================================================================
// Tipos de zombie
// ================================================================
//   NORMAL   : el zombie base, equilibrado.
//   CORREDOR : rapido y agresivo, pero fragil.
//   TANQUE   : lento, muy resistente y golpea muy fuerte. Mas grande.
//   TOXICO   : golpe debil pero envenena al jugador (dano continuo).
// ================================================================

enum TipoZombie {
  ZOMBIE_NORMAL = 0,
  ZOMBIE_CORREDOR,
  ZOMBIE_TANQUE,
  ZOMBIE_TOXICO,
  NUM_TIPOS_ZOMBIE
};

const float VELOCIDAD_POR_TIPO[NUM_TIPOS_ZOMBIE] = {
  0.55f,  // Normal
  1.0f,   // Corredor
  0.32f,  // Tanque
  0.5f    // Toxico
};

const float VIDA_POR_TIPO[NUM_TIPOS_ZOMBIE] = {
  100.0f,  // Normal
  55.0f,   // Corredor
  220.0f,  // Tanque
  85.0f    // Toxico
};

const float DANIO_POR_TIPO[NUM_TIPOS_ZOMBIE] = {
  1.0f,  // Normal
  0.6f,  // Corredor
  2.5f,  // Tanque
  0.8f   // Toxico (mas el veneno)
};

const unsigned long INTERVALO_ATAQUE_POR_TIPO[NUM_TIPOS_ZOMBIE] = {
  650, 420, 900, 700
};

const float DETECCION_POR_TIPO[NUM_TIPOS_ZOMBIE] = {
  10.0f, 13.0f, 8.0f, 11.0f
};

const float PERDIDA_POR_TIPO[NUM_TIPOS_ZOMBIE] = {
  18.0f, 22.0f, 15.0f, 19.0f
};

const float ATAQUE_RADIO_POR_TIPO[NUM_TIPOS_ZOMBIE] = {
  0.65f, 0.65f, 0.85f, 0.65f
};

const int32_t ESCALA_POR_TIPO[NUM_TIPOS_ZOMBIE] = {
  INT_TO_FIXED(1), INT_TO_FIXED(1), INT_TO_FIXED(2), INT_TO_FIXED(1)
};

// Probabilidad de aparicion de cada tipo (debe sumar 100).
const uint8_t PROBABILIDAD_POR_TIPO[NUM_TIPOS_ZOMBIE] = {
  55, 20, 10, 15
};

const char *nombreTipoZombie(TipoZombie tipo) {
  switch (tipo) {
    case ZOMBIE_NORMAL: return "ZOMBIE";
    case ZOMBIE_CORREDOR: return "CORREDOR";
    case ZOMBIE_TANQUE: return "TANQUE";
    case ZOMBIE_TOXICO: return "TOXICO";
    default: return "ZOMBIE";
  }
}

TipoZombie elegirTipoZombieAleatorio() {
  uint8_t r = random(0, 100);
  uint8_t acumulado = 0;

  for (uint8_t t = 0; t < NUM_TIPOS_ZOMBIE; t++) {
    acumulado += PROBABILIDAD_POR_TIPO[t];
    if (r < acumulado) return (TipoZombie)t;
  }
  return ZOMBIE_NORMAL;
}

struct ZombieEstado {
  Obj modelo;
  bool activo;
  float vida;
  float velocidad;
  float distancia;
  unsigned long ultimoAtaque;

  // Estado de la IA.
  EstadoIAZombie estadoIA;

  // Tipo de zombie (define estadisticas, tamaño y comportamiento).
  TipoZombie tipo;

  // Si pertenece a una horda, su objetivo prioritario es el albergue.
  bool esHorda;

  // Posicion original a la que vuelve cuando pierde al jugador.
  float posicionInicialX;
  float posicionInicialZ;
};

ZombieEstado zombies[MAX_ZOMBIES];
uint8_t cantidadZombies = MAX_ZOMBIES;

// ================================================================
// Plataforma = indicador 3D de recurso disponible
// ================================================================

struct PickupZombie {
  bool activo;
  TipoItemZombie tipo;
  int32_t x;
  int32_t y = 1000;
  int32_t z;
  float fase;
  float escala;
  float altura;
  float distancia;
  Obj plataforma;
};

PickupZombie pickups[MAX_PICKUPS];

struct PersonaRescate {
  bool activa;
  bool rescatada;
  float x;
  float z;
  Obj modelo;
};

PersonaRescate personasRescate[MAX_PERSONAS_RESCATE];
uint8_t personasRescatadas = 0;  // entregadas en el albergue
uint8_t personasEnCarro = 0;

// Estado de las hordas.
unsigned long inicioHordas = 0;
unsigned long siguienteHorda = 0;
uint8_t numeroHorda = 0;
uint8_t zombiesHordaActual = 0;
bool hordaActiva = false;

// ================================================================
// Camara / escena
// ================================================================

float rotEscenaX = 0.0f;
float rotEscenaY = 0.0f;
float rotEscenaZ = 0.0f;
float escalaEscena = 1.0f;

unsigned long ultimoAtaque = 0;
bool anteriorA = false;
bool anteriorB = false;
bool anteriorC = false;
bool anteriorD = false;
bool anteriorArriba = false;
bool anteriorAbajo = false;
bool anteriorIzquierda = false;
bool anteriorDerecha = false;

// Flancos de subida de cada boton, calculados una unica vez por frame
// en juegoLoop() (antes de que ningun sistema los consuma), para que
// todos los sistemas (inventario, armas, carro, baul) vean el mismo
// valor durante ese frame.
bool pulsacionA = false;
bool pulsacionB = false;
bool pulsacionC = false;
bool pulsacionD = false;
bool pulsacionArriba = false;
bool pulsacionAbajo = false;
bool pulsacionIzquierda = false;
bool pulsacionDerecha = false;

// Pulsacion corta/larga de C. Una pulsacion corta mantiene la funcion
// de entrar/salir del carro; una pulsacion larga abre la rueda de armas.
const unsigned long DURACION_C_LARGO_MS = 550UL;
bool cAnterior = false;
bool cEnPulsacion = false;
unsigned long inicioPulsacionC = 0UL;
bool ruedaArmasAbierta = false;
uint8_t armaRuedaSeleccionada = 1;  // 0 cuchillo, 1 pistola, 2 escopeta


float distanciaXZ(int32_t x1, int32_t y1, int32_t z1, int32_t x2, int32_t y2, int32_t z2) {
  Vector3D pos_jug = { x1, y1, z1 };
  Vector3D pos_p = { x2, y2, z2 };
  Vector3D dir = direccionHaciaJugador(pos_p, pos_jug);
  return length(dir);
}

int32_t limitarZombie(int32_t v, int32_t minimo, int32_t maximo) {
  if (v < minimo) return minimo;
  if (v > maximo) return maximo;
  return v;
}

int64_t productoPunto2D(int32_t ax, int32_t az, int32_t bx, int32_t bz) {
  return ax * bx + az * bz;
}

void nothing(Obj *obj) {}

uint16_t n_obj2 = 39;

Obj obj2[39] = {
{
.id = "1Luz",
//.level = 1,
.type = LUZSPOT,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 524,.f_end = 536},
.grupo_id="2Luz.001",
.luz = { .encendido = true, .intensidad = 1000, .alcance = 25000, .angulo = 65000, .suavidad = 2000, .direccion = { 0, 0, -FIXED_SCALE }, .color = 0xFFFF },

},
{
.id = "2Luz.001",
//.level = 1,
.type = LUZSPOT,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 536,.f_end = 548},
.grupo_id="3Luz.002",
.luz = { .encendido = true, .intensidad = 1000, .alcance = 25000, .angulo = 65000, .suavidad = 2000, .direccion = { 0, 0, -FIXED_SCALE }, .color = 0xFFFF },

},
{
.id = "3Luz.002",
//.level = 1,
.type = LUZSPOT,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 548,.f_end = 560},
.grupo_id="4Fogata",
.luz = { .encendido = true, .intensidad = 1000, .alcance = 25000, .angulo = 65000, .suavidad = 2000, .direccion = { 0, 0, -FIXED_SCALE }, .color = 0xFFFF },

},
{
.id = "4Fogata",
//.level = 1,
.type = LUZSPOT,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 560,.f_end = 572},
.grupo_id="12Baul",
.luz = { .encendido = true, .intensidad = 1000, .alcance = 25000, .angulo = 65000, .suavidad = 2000, .direccion = { 0, 0, -FIXED_SCALE }, .color = 0xFFFF },

},   
{
.id = "4Fogata.001",
//.level = 1,
.type = LUZSPOT,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 632,.f_end = 644},
.grupo_id="22MesaTrabajo",

}, 
{
.collision_mask = 1,
.id = "5Terreno",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 10366,.f_end = 20566},
.grupo_id="24Plataforma",
.is_ground=true,
},
{
.collision_mask = 1,
.id = "24Plataforma",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 0,.f_end = 124},
.grupo_id="13Carro",

},
{
.collision_mask = 1,
.id = "13Carro",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 124,.f_end = 272},
.grupo_id="19Lampara",

},
{
.collision_mask = 1,
.id = "19Lampara",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 272,.f_end = 332},
.grupo_id="18Lampara.001",

},
{
.collision_mask = 1,
.id = "18Lampara.001",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 332,.f_end = 392},
.grupo_id="20Lampara.002",

},
{
.collision_mask = 1,
.id = "20Lampara.002",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 392,.f_end = 452},
.grupo_id="21Jugador",

},
{
.collision_mask = 1,
.id = "21Jugador",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 452,.f_end = 480},
.grupo_id="26Zombie",

},
{
.collision_mask = 1,
.id = "26Zombie",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 480,.f_end = 524},
.grupo_id="1Luz",

},
{
.collision_mask = 1,
.id = "12Baul",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 572,.f_end = 584},
.grupo_id="23Persona",

},
{
.collision_mask = 1,
.id = "23Persona",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 584,.f_end = 612},
.grupo_id="25Refugio",

},
{
.collision_mask = 1,
.id = "25Refugio",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 612,.f_end = 632},
.grupo_id="4Fogata.001",

},
{
.collision_mask = 1,
.id = "22MesaTrabajo",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 644,.f_end = 656},
.grupo_id="11Arbol.010",

},
{
.collision_mask = 1,
.id = "11Arbol.010",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 656,.f_end = 1268},
.grupo_id="7Arbol.001",

},
{
.collision_mask = 1,
.id = "7Arbol.001",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 1268,.f_end = 1880},
.grupo_id="8Arbol.002",

},
{
.collision_mask = 1,
.id = "8Arbol.002",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 1880,.f_end = 2492},
.grupo_id="9Arbol.003",

},
{
.collision_mask = 1,
.id = "9Arbol.003",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 2492,.f_end = 3104},
.grupo_id="10Arbol.004",

},
{
.collision_mask = 1,
.id = "10Arbol.004",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 3104,.f_end = 3716},
.grupo_id="27Edificios",

},
{
.collision_mask = 1,
.id = "27Edificios",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 3716,.f_end = 3998},
.grupo_id="26ZombieTanque",

},
{
.collision_mask = 1,
.id = "26ZombieTanque",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 3998,.f_end = 4042},
.grupo_id="16Casa.001",

},
{
.collision_mask = 1,
.id = "16Casa.001",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 4042,.f_end = 4586},
.grupo_id="16Casa.002",

},
{
.collision_mask = 1,
.id = "16Casa.002",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 4586,.f_end = 5130},
.grupo_id="16Casa.003",

},
{
.collision_mask = 1,
.id = "16Casa.003",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 5130,.f_end = 5674},
.grupo_id="16Casa.004",

},
{
.collision_mask = 1,
.id = "16Casa.004",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 5674,.f_end = 6218},
.grupo_id="27Edificios.001",

},
{
.collision_mask = 1,
.id = "27Edificios.001",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 6218,.f_end = 6500},
.grupo_id="10Arbol.001",

},
{
.collision_mask = 1,
.id = "10Arbol.001",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 6500,.f_end = 7112},
.grupo_id="9Arbol.001",

},
{
.collision_mask = 1,
.id = "9Arbol.001",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 7112,.f_end = 7724},
.grupo_id="8Arbol.001",

},
{
.collision_mask = 1,
.id = "8Arbol.001",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 7724,.f_end = 8336},
.grupo_id="10Arbol.002",

},
{
.collision_mask = 1,
.id = "10Arbol.002",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 8336,.f_end = 8948},
.grupo_id="16Casa.005",

},
{
.collision_mask = 1,
.id = "16Casa.005",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 8948,.f_end = 9492},
.grupo_id="27Edificios.002",

},
{
.collision_mask = 1,
.id = "27Edificios.002",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 9492,.f_end = 9774},
.grupo_id="13Carro.001",

},
{
.collision_mask = 1,
.id = "13Carro.001",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 9774,.f_end = 9922},
.grupo_id="13Carro.002",

},
{
.collision_mask = 1,
.id = "13Carro.002",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 9922,.f_end = 10070},
.grupo_id="13Carro.003",

},
{
.collision_mask = 1,
.id = "13Carro.003",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 10070,.f_end = 10218},
.grupo_id="13Carro.004",

},
{
.collision_mask = 1,
.id = "13Carro.004",
//.level = 1,
.type = BODY,
.visible = true,
.onloop = nothing,
.mtl{.f_init = 10218,.f_end = 10366},
.grupo_id="5Terreno.001",

}
};

/*{
  {
    .id = "Luz",
    .type = LUZSPOT,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 3570, .f_end = 3582 },
    .grupo_id = "Luz.001",
    .luz = { .encendido = true, .intensidad = 1000, .alcance = 15000, .angulo = 65000, .suavidad = 2000, .direccion = { 0, 0, -FIXED_SCALE }, .color = 0xFFFF },

  },
  {
    .id = "Luz.001",
    .type = LUZSPOT,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 3582, .f_end = 3594 },
    .grupo_id = "Luz.002",
    .luz = { .encendido = true, .intensidad = 1000, .alcance = 15000, .angulo = 65000, .suavidad = 1500, .direccion = { 0, 0, -FIXED_SCALE }, .color = 0xFFFF },

  },
  {
    .id = "Luz.002",
    .type = LUZSPOT,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 3594, .f_end = 3606 },
    .grupo_id = "Fogata",
    .luz = { .encendido = true, .intensidad = 1000, .alcance =15000, .angulo = 65000, .suavidad = 2000, .direccion = { 0, 0, -FIXED_SCALE }, .color = 0xFFFF },

  },
  {
    .id = "Fogata",
    .type = LUZSPOT,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 3606, .f_end = 3618 },
    .grupo_id = "Baul",
    .luz = { .encendido = true, .intensidad = 1000, .alcance = 28000, .angulo = 175000, .suavidad = 1500, .direccion = { 0, 0, -FIXED_SCALE }, .color = 0xFD20 },

  },
  {
    .collision_mask = 0,
    .id = "Terreno",
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 0, .f_end = 2742 },
    .grupo_id = "Casa.003",
    .is_ground = true,
  },
  {
    .collision_mask = 0,
    .id = "Casa.003",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 2742, .f_end = 2862 },
    .grupo_id = "Casa.006",

  },
  {
    .collision_mask = 0,
    .id = "Casa.006",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 2862, .f_end = 2874 },
    .grupo_id = "Casa.007",

  },
  {
    .collision_mask = 0,
    .id = "Casa.007",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 2874, .f_end = 2886 },
    .grupo_id = "Casa.008",

  },
  {
    .collision_mask = 0,
    .id = "Casa.008",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 2886, .f_end = 2898 },
    .grupo_id = "Casa.009",

  },
  {
    .collision_mask = 0,
    .id = "Casa.009",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 2898, .f_end = 2910 },
    .grupo_id = "Plataforma",

  },
  {
    .collision_mask = 1,
    .id = "Plataforma",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 2910, .f_end = 3034 },
    .grupo_id = "Carro",

  },
  {
    .collision_mask = 1,
    .id = "Carro",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 3034, .f_end = 3182 },
    .grupo_id = "Arbol",

  },
  {
    .collision_mask = 0,
    .id = "Arbol",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 3182, .f_end = 3250 },
    .grupo_id = "Arbol.003",

  },
  {
    .collision_mask = 0,
    .id = "Arbol.003",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 3250, .f_end = 3318 },
    .grupo_id = "Lampara",

  },
  {
    .collision_mask = 1,
    .id = "Lampara",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 3318, .f_end = 3378 },
    .grupo_id = "Lampara.001",

  },
  {
    .collision_mask = 1,
    .id = "Lampara.001",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 3378, .f_end = 3438 },
    .grupo_id = "Lampara.002",

  },
  {
    .collision_mask = 1,
    .id = "Lampara.002",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 3438, .f_end = 3498 },
    .grupo_id = "Jugador",
    .is_cull = false,
  },
  {
    .collision_mask = 1,
    .id = "Jugador",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 3498, .f_end = 3526 },
    .grupo_id = "Zombie",

  },
  {
    .collision_mask = 1,
    .id = "Zombie",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 3526, .f_end = 3570 },
    .grupo_id = "Luz",

  },
  {
    .collision_mask = 1,
    .id = "Baul",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 3618, .f_end = 3630 },
    .grupo_id = "Persona",

  },
  {
    .collision_mask = 1,
    .id = "Persona",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 3630, .f_end = 3658 },
    .grupo_id = "Albergue",

  },
  {
    .collision_mask = 0,
    .id = "Albergue",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 3658, .f_end = 3734 },
    .grupo_id = "Refugio",

  },
  {
    .collision_mask = 1,
    .id = "Refugio",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 3734, .f_end = 3754 },
    .grupo_id = "Arbol.007",

  },
  {
    .collision_mask = 0,
    .id = "Arbol.007",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 3754, .f_end = 4162 },
    .grupo_id = "Arbol.001",

  },
  {
    .collision_mask = 0,
    .id = "Arbol.001",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 4162, .f_end = 4570 },
    .grupo_id = "Arbol.002",

  },
  {
    .collision_mask = 0,
    .id = "Arbol.002",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 4570, .f_end = 4978 },
    .grupo_id = "Arbol.004",

  },
  {
    .collision_mask = 0,
    .id = "Arbol.004",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 4978, .f_end = 5386 },
    .grupo_id = "Casa.001",

  },
  {
    .collision_mask = 0,
    .id = "Casa.001",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 5386, .f_end = 5506 },
    .grupo_id = "Casa.002",

  },
  {
    .collision_mask = 0,
    .id = "Casa.002",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 5506, .f_end = 5626 },
    .grupo_id = "Casa.004",

  },
  {
    .collision_mask = 0,
    .id = "Casa.004",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 5626, .f_end = 5746 },
    .grupo_id = "Casa.005",

  },
  { .collision_mask = 0,
    .id = "Casa.005",
    //.level = 1,
    .type = BODY,
    .visible = true,
    .onloop = nothing,
    .mtl{ .f_init = 5746, .f_end = 5866 } }
};*/

bool aabbSolapadaXZ(const AABB &a, const AABB &b) {
  return (a.min.x < b.max.x && a.max.x > b.min.x && a.min.z < b.max.z && a.max.z > b.min.z);
}

float albergueX() {
  return obj2[MODELO_ALBERGUE].traslation.x + obj2[MODELO_ALBERGUE].position.x;
}
float albergueY() {
  return obj2[MODELO_ALBERGUE].traslation.y + obj2[MODELO_ALBERGUE].position.y;
}
float albergueZ() {
  return obj2[MODELO_ALBERGUE].traslation.z + obj2[MODELO_ALBERGUE].position.z;
}

uint16_t capacidadBaul() {
  return CAPACIDAD_BAUL_BASE + nivelBaul * CAPACIDAD_BAUL_POR_NIVEL;
}

float velocidadMaximaCarro() {
  return CARRO_VELOCIDAD_MAX * (1.0f + nivelCarro * 0.10f);
}

float aceleracionCarro() {
  return CARRO_ACELERACION * (1.0f + nivelCarro * 0.08f);
}

uint16_t totalBaul() {
  uint16_t total = 0;
  for (uint8_t i = 0; i < NUM_ITEMS_INVENTARIO; i++) total += baulZombie[i];
  return total;
}

float multiplicadorDefensaAlbergue() {
  return 1.0f + nivelAlbergue * 0.20f;
}

uint16_t costoMejora(uint8_t tipo, uint8_t nivelActual) {
  // Costes pagados desde el baul: comida + agua + municion + botiquines.
  // Cada nivel aumenta el coste.
  uint16_t base = (tipo == 0 ? 4 : (tipo == 1 ? 5 : 6));
  return base + nivelActual * 3;
}

bool pagarMejora(uint8_t tipo, uint16_t costo) {
  // Se usa una mezcla sencilla de recursos del baul.
  uint16_t comida = baulZombie[ITEM_COMIDA];
  uint16_t agua = baulZombie[ITEM_AGUA];
  uint16_t mun = baulZombie[ITEM_MUNICION];
  uint16_t bot = baulZombie[ITEM_BOTIQUIN];
  uint16_t disponible = comida + agua + mun + bot;
  if (disponible < costo) return false;

  // Prioridad: comida, agua, municion, botiquines.
  uint16_t falta = costo;
  uint16_t usar = (baulZombie[ITEM_COMIDA] < falta ? baulZombie[ITEM_COMIDA] : falta);
  baulZombie[ITEM_COMIDA] -= usar;
  falta -= usar;
  usar = (baulZombie[ITEM_AGUA] < falta ? baulZombie[ITEM_AGUA] : falta);
  baulZombie[ITEM_AGUA] -= usar;
  falta -= usar;
  usar = (baulZombie[ITEM_MUNICION] < falta ? baulZombie[ITEM_MUNICION] : falta);
  baulZombie[ITEM_MUNICION] -= usar;
  falta -= usar;
  usar = (baulZombie[ITEM_BOTIQUIN] < falta ? baulZombie[ITEM_BOTIQUIN] : falta);
  baulZombie[ITEM_BOTIQUIN] -= usar;
  return true;
}

void comprarMejoraSeleccionada() {
  uint8_t *nivel = nullptr;
  uint8_t maximo = 0;
  if (mejoraSeleccionada == 0) {
    nivel = &nivelAlbergue;
    maximo = NIVEL_MAX_ALBERGUE;
  } else if (mejoraSeleccionada == 1) {
    nivel = &nivelBaul;
    maximo = NIVEL_MAX_BAUL;
  } else {
    nivel = &nivelCarro;
    maximo = NIVEL_MAX_CARRO;
  }

  if (*nivel >= maximo) return;
  uint16_t costo = costoMejora(mejoraSeleccionada, *nivel);
  if (!pagarMejora(mejoraSeleccionada, costo)) return;

  (*nivel)++;
  if (mejoraSeleccionada == 0) {
    vidaAlbergue += 25.0f;
    if (vidaAlbergue > VIDA_ALBERGUE_BASE + NIVEL_MAX_ALBERGUE * 25.0f)
      vidaAlbergue = VIDA_ALBERGUE_BASE + NIVEL_MAX_ALBERGUE * 25.0f;
  }
}

void recibirDanioAlbergue(float cantidad) {
  if (vidaAlbergue <= 0.0f) return;
  vidaAlbergue -= (cantidad / multiplicadorDefensaAlbergue()) / 50.0f;
  if (vidaAlbergue < 0.0f) vidaAlbergue = 0.0f;
}


void agregarAlInventario(TipoItemZombie tipo, uint16_t cantidad) {
  inventarioZombie[(uint8_t)tipo].cantidad += cantidad;
}

void generarPickup(uint8_t i, TipoItemZombie tipo, float x, float y, float z, float px, float py, float pz) {
  pickups[i].activo = true;
  pickups[i].tipo = tipo;
  pickups[i].x = (x * 1000) - (px * 1000);
  pickups[i].z = (z * 1000) - (pz * 1000);
  pickups[i].fase = (float)random(0, 628) / 100.0f;
  pickups[i].escala = 0.5f;
  pickups[i].altura = 0.0f;
  pickups[i].plataforma = obj2[MODELO_PLATAFORMA];
  pickups[i].plataforma.visible = true;
  //pickups[i].plataforma.position.x = px * 1000;
  //pickups[i].plataforma.position.z = pz * 1000;
  pickups[i].plataforma.traslation.x = (x * 1000) - (px * 1000);
  pickups[i].plataforma.traslation.y = (y * 1000) - (py * 1000);
  pickups[i].plataforma.traslation.z = (z * 1000) - (pz * 1000);
}

void inicializarPickups() {
  for (uint8_t i = 0; i < MAX_PICKUPS; i++) pickups[i].activo = false;

  generarPickup(0, ITEM_BOTIQUIN, -25.0f, 0.0f, -18.0f, 0.0f, 0.0f, 0.0f);
  generarPickup(1, ITEM_MUNICION, 18.0f, 0.0f, -25.0f, 0.0f, 0.0f, 0.0f);
  generarPickup(2, ITEM_COMIDA, -12.0f, 0.0f, 28.0f, 0.0f, 0.0f, 0.0f);
  generarPickup(3, ITEM_AGUA, 30.0f, 0.0f, 18.0f, 0.0f, 0.0f, 0.0f);
  generarPickup(4, ITEM_MUNICION, -35.0f, 0.0f, 12.0f, 0.0f, 0.0f, 0.0f);
  generarPickup(5, ITEM_BOTIQUIN, 34.0f, 0.0f, -10.0f, 0.0f, 0.0f, 0.0f);
  generarPickup(6, ITEM_COMIDA, 5.0f, 0.0f, -34.0f, 0.0f, 0.0f, 0.0f);
  generarPickup(7, ITEM_AGUA, -28.0f, 0.0f, 35.0f, 0.0f, 0.0f, 0.0f);

  obj2[MODELO_PLATAFORMA].visible = false;
}

void aplicarDanioJugador(float cantidad) {
  if (!jugadorZombie.vivo || jugadorZombie.enCarro) return;

  // Destello rojo al recibir daño
  destelloDanioHasta = millis() + 100;

  jugadorZombie.vida -= cantidad;
  if (jugadorZombie.vida <= 0.0f) {
    jugadorZombie.vida = 0.0f;
    jugadorZombie.vivo = false;
  }
}

void curarJugador(float cantidad) {
  jugadorZombie.vida += cantidad;
  if (jugadorZombie.vida > 100.0f) jugadorZombie.vida = 100.0f;
}

// El veneno del zombie toxico sigue haciendo dano un rato despues del golpe.
void actualizarVeneno() {
  if (!jugadorZombie.vivo || !jugadorZombie.envenenado) return;

  unsigned long ahora = millis();

  if (ahora >= jugadorZombie.finVeneno) {
    jugadorZombie.envenenado = false;
    return;
  }

  if (ahora >= jugadorZombie.siguienteTicVeneno) {
    aplicarDanioJugador(DANIO_VENENO_TIC);
    jugadorZombie.siguienteTicVeneno = ahora + INTERVALO_TIC_VENENO;
  }
}

void actualizarDiaNoche() {
  unsigned long transcurrido = millis() - inicioCicloDia;
  float progreso = (float)(transcurrido % DURACION_DIA_MS) / (float)DURACION_DIA_MS;

  horaJuego = progreso * 24.0f;
  esNoche = (horaJuego >= 20.0f || horaJuego < 6.0f);

  // Angulo del ciclo en grados, usando int32_t para evitar trigonometria en radianes.
  float anguloDiaDeg = progreso * 360.0f;

  /*for (uint8_t i = 0; i < n_obj; i++) {
    if (obj2[i].type == LUZSPOT){
      obj2[i].luz.encendido = esNoche;
    }
  }

  Vector3D direccionSol = {
    1.0f,
    1.0f,
    -(float)cosDegQ15(anguloDiaDeg) / 32767.0f
  };
  lcd.setLightDir(direccionSol);*/

  /*Vector3D direccionSol = {
    0,
    0,
    (int32_t)(FIXED_SCALE * cosf(anguloDiaDeg * (PI / 180.0f)))
  };
  lcd.setLightDir(direccionSol);*/
}

void dibujarFiltroNoche() {
  if (!esNoche) return;

  // Noche mas oscura.
  //lcd.fillRect(0, 0, LCD_WIDTH, LCD_HEIGHT, 0x0008, 125);

  // Luna.
  int lunaX = LCD_WIDTH - 27;
  int lunaY = 28;
  lcd.drawCircle(lunaX, lunaY, 9, 0xFFFF, 170);
  lcd.drawCircle(lunaX - 4, lunaY - 3, 8, 0x0008, 255);

  // Linterna: cono visual en la direccion del jugador.
  /*int cx = LCD_WIDTH / 2;
  int cy = LCD_HEIGHT / 2;
  int lx = cx + (int)(jugadorZombie.dirX * 35.0f);
  int ly = cy + (int)(jugadorZombie.dirZ * 22.0f);

  lcd.fillTriangle(cx, cy, lx - 18, ly + 28, lx + 18, ly - 28, 0xFFE0, 32);
  lcd.drawCircle(lx, ly, 15, 0xFFFF, 45);*/
}

void dibujarHora() {
  int hora = (int)horaJuego;
  int minuto = (int)((horaJuego - hora) * 60.0f);
  char texto[18];
  snprintf(texto, sizeof(texto), "%02d:%02d %s", hora, minuto,
           esNoche ? "NOCHE" : "DIA");

  lcd.drawText(LCD_WIDTH - 120, 275, texto, "SANSBOLD",
               esNoche ? 0xBDF7 : 0xFFE0, INT_TO_FIXED(1));
}

void actualizarRuedaArmas() {
  // Abrir automaticamente al mantener C. La seleccion se hace con el
  // joystick/direccionales y queda resaltada en tiempo real.
  if (mC.t_C && !cAnterior) {
    cEnPulsacion = true;
    inicioPulsacionC = millis();
    ruedaArmasAbierta = false;
    armaRuedaSeleccionada = (uint8_t)armaEquipada;
  }

  if (mC.t_C && cEnPulsacion && millis() - inicioPulsacionC >= DURACION_C_LARGO_MS) {
    ruedaArmasAbierta = true;
  }

  if (ruedaArmasAbierta) {
    if (mC.t_izquierda) armaRuedaSeleccionada = 0;
    if (mC.t_arriba) armaRuedaSeleccionada = 1;
    if (mC.t_derecha) armaRuedaSeleccionada = 2;
    // Abajo tambien selecciona la escopeta para facilitar el uso con
    // cualquier orientacion del mando.
    if (mC.t_abajo) armaRuedaSeleccionada = 2;
  }

  if (!mC.t_C && cAnterior) {
    cEnPulsacion = false;
  }
}

bool slotEsArma(uint8_t slot) {
  return slot < 3;
}

void usarSlotInventario() {
  if (slotEsArma(slotInventario)) {
    armaEquipada = (ArmaZombie)slotInventario;
    return;
  }

  uint8_t item = slotInventario - 3;
  if (item >= NUM_ITEMS_INVENTARIO) return;
  ItemInventarioZombie &obj = inventarioZombie[item];
  if (obj.cantidad == 0) return;

  switch ((TipoItemZombie)item) {
    case ITEM_BOTIQUIN:
      if (jugadorZombie.vida >= 100.0f) return;
      curarJugador(40.0f);
      obj.cantidad--;
      break;
    case ITEM_MUNICION:
      // La municion no se usa manualmente.
      break;
    case ITEM_COMIDA:
      curarJugador(8.0f);
      obj.cantidad--;
      break;
    case ITEM_AGUA:
      curarJugador(5.0f);
      obj.cantidad--;
      break;
  }
}

void actualizarInventario() {
  // D cierra el menu que este abierto; si no hay ninguno abierto, abre
  // la mochila. Asi nunca se solapan el inventario y el baul.
  if (pulsacionD) {
    if (baulAbierto) baulAbierto = false;
    else {
      inventarioAbierto = !inventarioAbierto;
      if (!inventarioAbierto) mapaInventario = false;
    }
  }

  if (inventarioAbierto) {
    // Izquierda/Derecha cambia entre mochila y mapa.
    if (pulsacionIzquierda || pulsacionDerecha) {
      mapaInventario = !mapaInventario;
      pulsacionIzquierda = false;
      pulsacionDerecha = false;
    }

    if (mapaInventario) return;

    if (pulsacionArriba) {
      if (slotInventario == 0) slotInventario = MAX_SLOTS_INVENTARIO - 1;
      else slotInventario--;
    }

    if (pulsacionAbajo) {
      slotInventario++;
      if (slotInventario >= MAX_SLOTS_INVENTARIO) slotInventario = 0;
    }

    if (pulsacionA) usarSlotInventario();
  }
}

float distanciaAlBaul() {
  return distanciaXZ(
    obj2[MODELO_JUGADOR].traslation.x + obj2[MODELO_JUGADOR].position.x,
    obj2[MODELO_JUGADOR].traslation.y + obj2[MODELO_JUGADOR].position.y,
    obj2[MODELO_JUGADOR].traslation.z + obj2[MODELO_JUGADOR].position.z,
    obj2[MODELO_BAUL].traslation.x + obj2[MODELO_BAUL].position.x,
    obj2[MODELO_BAUL].traslation.y + obj2[MODELO_BAUL].position.y,
    obj2[MODELO_BAUL].traslation.z + obj2[MODELO_BAUL].position.z);
}

void actualizarBaul() {
  if (!jugadorZombie.vivo || jugadorZombie.enCarro) {
    baulAbierto = false;
    return;
  }

  bool baulCerca = distanciaAlBaul() <= RADIO_BAUL * RADIO_BAUL;

  // Si el jugador se aleja del baul, el menu se cierra solo.
  if (baulAbierto && !baulCerca) {
    baulAbierto = false;
    return;
  }

  if (!baulAbierto) {
    // Solo se puede abrir estando cerca y con la mochila cerrada.
    if (pulsacionA && baulCerca && !inventarioAbierto) {
      baulAbierto = true;
      mejorasAbiertas = false;
      pulsacionA = false;
    }
    return;
  }

  // B abre el panel de mejoras desde el baul.
  if (pulsacionB) {
    mejorasAbiertas = !mejorasAbiertas;
    pulsacionB = false;
  }

  if (mejorasAbiertas) {
    if (pulsacionIzquierda || pulsacionArriba) {
      if (mejoraSeleccionada == 0) mejoraSeleccionada = 2;
      else mejoraSeleccionada--;
    }
    if (pulsacionDerecha || pulsacionAbajo) {
      mejoraSeleccionada++;
      if (mejoraSeleccionada > 2) mejoraSeleccionada = 0;
    }
    if (pulsacionA) {
      comprarMejoraSeleccionada();
      pulsacionA = false;
    }
    return;
  }

  // Baul abierto: navegar entre objetos y transferirlos.
  if (pulsacionArriba) {
    if (slotBaul == 0) slotBaul = NUM_ITEMS_INVENTARIO - 1;
    else slotBaul--;
  }

  if (pulsacionAbajo) {
    slotBaul++;
    if (slotBaul >= NUM_ITEMS_INVENTARIO) slotBaul = 0;
  }

  if (pulsacionIzquierda || pulsacionDerecha) {
    ladoBaul = !ladoBaul;
  }

  if (pulsacionA) {
    if (ladoBaul == 0) {
      // Guardar: de la mochila al baul.
      if (inventarioZombie[slotBaul].cantidad > 0 && totalBaul() < capacidadBaul()) {
        inventarioZombie[slotBaul].cantidad--;
        baulZombie[slotBaul]++;
      }
    } else {
      // Sacar: del baul a la mochila.
      if (baulZombie[slotBaul] > 0) {
        baulZombie[slotBaul]--;
        inventarioZombie[slotBaul].cantidad++;
      }
    }
    // Ya se uso esta pulsacion para transferir, no debe atacar.
    pulsacionA = false;
  }
}

AABB obtenerAABBLocalMundo(const Obj &obj, int32_t pivotX, int32_t pivotY, int32_t pivotZ) {
  int32_t cx = (obj.collision.min.x + obj.collision.max.x) / 2;
  int32_t cy = (obj.collision.min.y + obj.collision.max.y) / 2;
  int32_t cz = (obj.collision.min.z + obj.collision.max.z) / 2;

  int32_t hx = (obj.collision.max.x - obj.collision.min.x) / 2;
  int32_t hy = (obj.collision.max.y - obj.collision.min.y) / 2;
  int32_t hz = (obj.collision.max.z - obj.collision.min.z) / 2;

  // El render aplica la rotacion local alrededor del origen del modelo.
  int32_t rx = cx - obj.position.x;
  int32_t ry = cy - obj.position.y;
  int32_t rz = cz - obj.position.z;

  int32_t rad = obj.rotation.y * PI / 180;
  int32_t c = cosf(obj.rotation.y);
  int32_t si = sinf(obj.rotation.y);

  // Misma convencion que matRotY() de LCD.h:
  // x' = c*x + s*z
  // z' = -s*x + c*z
  int32_t rcx = c * rx + si * rz;
  int32_t rcz = -si * rx + c * rz;

  int32_t ex = fabsf(c) * hx + fabsf(si) * hz;
  int32_t ez = fabsf(si) * hx + fabsf(c) * hz;

  AABB caja;
  caja.min.x = obj.position.x + pivotX + rcx - ex;
  caja.max.x = obj.position.x + pivotX + rcx + ex;
  caja.min.y = obj.position.y + pivotY + cy - obj.position.y - hy;
  caja.max.y = obj.position.y + pivotY + cy - obj.position.y + hy;
  caja.min.z = obj.position.z + pivotZ + rcz - ez;
  caja.max.z = obj.position.z + pivotZ + rcz + ez;

  return caja;
}

bool esObstaculoMundo(uint8_t i) {
  if (i >= n_obj2) return false;
  if (!obj2[i].visible && !obj2[i].collision_mask) return false;
  if (obj2[i].collision_mask != 1) return false;
  if (obj2[i].is_ground) return false;  // El terreno no bloquea en X/Z.
  if (i == MODELO_JUGADOR) return false;
  if (i == MODELO_ZOMBIE) return false;
  if (i == MODELO_PLATAFORMA) return false;
  if (obj2[i].type == LUZSPOT) return false;
  return true;
}

bool colisionConMundo(const Obj &actor, int32_t x, int32_t y, int32_t z,
                      int32_t margen = 0) {
  AABB actorBox = actor.collision;  //obtenerAABBLocalMundo(actor, x, y, z);

  actorBox.min.x += margen;
  actorBox.max.x -= margen;
  actorBox.min.z += margen;
  actorBox.max.z -= margen;

  for (uint8_t i = 0; i < n_obj2; i++) {
    if (!esObstaculoMundo(i)) continue;

    // Un actor nunca debe colisionar consigo mismo.
    // Esto es especialmente importante para el carro: al intentar moverlo,
    // su propia AABB estaba siendo detectada como obstaculo y bloqueaba
    // permanentemente cualquier desplazamiento.
    if (&actor == &obj2[i]) continue;

    AABB obstaculo = obj2[i].collision; /*obtenerAABBLocalMundo(
      obj2[i],
      obj2[i].traslation.x,
      obj2[i].traslation.y,
      obj2[i].traslation.z
    );*/

    // Solo nos interesa la colision horizontal del personaje.
    // Se comprueba Y para no considerar objetos que estan separados verticalmente.
    if (aabbSolapadaXZ(actorBox, obstaculo) && actorBox.min.y < obstaculo.max.y && actorBox.max.y > obstaculo.min.y) {
      return true;
    }
  }

  return false;
}

bool colisionConZombies(int32_t x, int32_t z, uint8_t ignorar) {
  const float distanciaMinimaBase = 0.72f;

  for (uint8_t i = 0; i < cantidadZombies; i++) {
    if (i == ignorar || !zombies[i].activo) continue;

    float zx = (zombies[i].modelo.traslation.x + zombies[i].modelo.position.x) / 1000.0f;
    float zz = (zombies[i].modelo.traslation.z + zombies[i].modelo.position.z) / 1000.0f;

    float dx = (x / 1000.0f) - zx;
    float dz = (z / 1000.0f) - zz;

    // Un zombie mas grande (tanque) ocupa mas espacio fisico.
    float distanciaMinima = distanciaMinimaBase * (ESCALA_POR_TIPO[zombies[i].tipo] / 1000.0f);

    if ((dx * dx + dz * dz) < distanciaMinima * distanciaMinima) {
      return true;
    }
  }

  return false;
}

// Intenta desplazar al actor. Se prueba cada eje por separado para que
// el personaje pueda "deslizarse" por las paredes.
bool moverActorConColision(Obj &actor, int32_t &x, int32_t &z,
                           int32_t dx, int32_t dz,
                           bool comprobarZombies = false,
                           uint8_t zombieIgnorado = 255) {
  bool seMovio = false;

  if (fabsf(dx) > 0) {
    int32_t nuevoX = x + dx;
    x = nuevoX;  // Quitar cuando quiera colisoines
    if (!colisionConMundo(actor, nuevoX, actor.traslation.y, z, SEPARACION_COLISION) && (!comprobarZombies || !colisionConZombies(nuevoX, z, zombieIgnorado))) {
      x = nuevoX;
      seMovio = true;
    }
  }

  if (fabsf(dz) > 0) {
    int32_t nuevoZ = z + dz;
    z = nuevoZ;  // Quitar cuando quiera colisoines
    if (!colisionConMundo(actor, x, actor.traslation.y, nuevoZ, SEPARACION_COLISION) && (!comprobarZombies || !colisionConZombies(x, nuevoZ, zombieIgnorado))) {
      z = nuevoZ;
      seMovio = true;
    }
  }

  return seMovio;
}

int32_t calcular_angulo(int32_t X, int32_t Z) {
  double angulo = atan2((double)X, (double)Z);

  // Ángulo en milésimas de radian
  return (int32_t)((angulo * 180.0 / PI));
}

void moverJugadorAPie() {
  //if (!jugadorZombie.vivo || jugadorZombie.enCarro || menuAbierto()) return;

  float dx = 0;
  float dz = 0;

  if (mC.t_derecha) dx -= 0.850f;
  if (mC.t_izquierda) dx += 0.850f;
  if (mC.t_abajo) dz -= 0.850f;
  if (mC.t_arriba) dz += 0.850f;

  if (dx != 0 || dz != 0) {
    float len = sqrtf(dx * dx + dz * dz);

    // Normalizamos para que diagonal no sea mas rapida.
    dx = (dx / len) * VELOCIDAD_JUGADOR / 1000.0f;
    dz = (dz / len) * VELOCIDAD_JUGADOR / 1000.0f;

    jugadorZombie.dirX = dx / VELOCIDAD_JUGADOR / 1000.0f;
    jugadorZombie.dirZ = dz / VELOCIDAD_JUGADOR / 1000.0f;
    jugadorZombie.angulo =
      atan2f(jugadorZombie.dirX, jugadorZombie.dirZ) * 180.0f / PI;

    obj2[MODELO_JUGADOR].rotation.y = jugadorZombie.angulo;

    int32_t nuevoX = obj2[MODELO_JUGADOR].traslation.x;
    int32_t nuevoZ = obj2[MODELO_JUGADOR].traslation.z;
    moverActorConColision(
      obj2[MODELO_JUGADOR],
      nuevoX, nuevoZ,
      dx * 1000, dz * 1000,
      true);

    obj2[MODELO_JUGADOR].traslation.x = limitarZombie(nuevoX, ZONA_MIN_X, ZONA_MAX_X);
    obj2[MODELO_JUGADOR].traslation.z = limitarZombie(nuevoZ, ZONA_MIN_Z, ZONA_MAX_Z);
  }

  pivotCamaraSupervivencia.x = obj2[MODELO_JUGADOR].traslation.x;
  pivotCamaraSupervivencia.z = obj2[MODELO_JUGADOR].traslation.z + 8500;
}

void dibujarEscena() {

  /*lcd.sortSceneFaces(obj2, 29, pivotCamaraSupervivencia);
  lcd.drawSceneSortedSingleFace(obj2, pivotCamaraSupervivencia, INT_TO_FIXED(1));*/

  for (uint8_t i = 0; i < n_obj2; i++) {
    // Plataforma se dibuja como instancia independiente mas abajo.
    if (i == MODELO_PLATAFORMA) continue;
    if (!obj2[i].visible) continue;

    lcd.drawSceneGameObjectV2(
      obj2[i], rotEscenaX, rotEscenaY, rotEscenaZ,
      camaraSupervivencia, pivotCamaraSupervivencia,
      true, INT_TO_FIXED(1));
  }

  for (uint8_t i = 0; i < cantidadZombies; i++) {
    if (!zombies[i].activo || !zombies[i].modelo.visible) continue;

    lcd.drawSceneGameObjectV2(
      zombies[i].modelo, rotEscenaX, rotEscenaY, rotEscenaZ,
      camaraSupervivencia, pivotCamaraSupervivencia,
      true, INT_TO_FIXED(1));
  }

  for (uint8_t i = 0; i < MAX_PERSONAS_RESCATE; i++) {
    if (!personasRescate[i].activa || personasRescate[i].rescatada) continue;
    lcd.drawSceneGameObjectV2(
      personasRescate[i].modelo, 0, rotEscenaY, rotEscenaZ,
      camaraSupervivencia, pivotCamaraSupervivencia,
      true, INT_TO_FIXED(1));
  }
}

// ================================================================
// Zombies
// ================================================================

bool posicionZombieValida(float x, float y, float z, uint8_t hasta) {
  float px = (obj2[MODELO_JUGADOR].traslation.x + obj2[MODELO_JUGADOR].position.x) / 1000.0f;
  float py = (obj2[MODELO_JUGADOR].traslation.y + obj2[MODELO_JUGADOR].position.y) / 1000.0f;
  float pz = (obj2[MODELO_JUGADOR].traslation.z + obj2[MODELO_JUGADOR].position.z) / 1000.0f;

  if (distanciaXZ(x * 1000, y * 1000, z * 1000, px * 1000, py * 1000, pz * 1000) < 12.0f * 12.0f) return false;

  for (uint8_t i = 0; i < hasta; i++) {
    if (!zombies[i].activo) continue;
    if (distanciaXZ(x * 1000, y * 1000, z * 1000, zombies[i].modelo.traslation.x + zombies[i].modelo.position.x,
                    zombies[i].modelo.traslation.y + zombies[i].modelo.position.y, zombies[i].modelo.traslation.z + zombies[i].modelo.position.z)
        < 4.0f * 4.0f) return false;
  }
  return true;
}

void generarPosicionZombie(uint8_t indice) {
  for (uint16_t intento = 0; intento < 100; intento++) {
    float x = (float)random((long)((ZONA_MIN_X / 1000.0f) * 10.0f),
                            (long)((ZONA_MAX_X / 1000.0f) * 10.0f) + 1)
              / 10.0f;
    float y = 1.0f;
    float z = (float)random((long)((ZONA_MIN_Z / 1000.0f) * 10.0f),
                            (long)((ZONA_MAX_Z / 1000.0f) * 10.0f) + 1)
              / 10.0f;

    if (posicionZombieValida(x, y, z, indice)) {
      zombies[indice].modelo.traslation.x = x * 1000;
      zombies[indice].modelo.traslation.y = 0;
      zombies[indice].modelo.traslation.z = z * 1000;
      return;
    }
  }

  zombies[indice].modelo.traslation.x = 2000 + indice * 1700;
  zombies[indice].modelo.traslation.z = 2000;
}

void nothingZombie(Obj *obj) {}

void inicializarZombies() {
  for (uint8_t i = 0; i < MAX_ZOMBIES; i++) {
    TipoZombie tipo = elegirTipoZombieAleatorio();

    zombies[i].modelo = obj2[MODELO_ZOMBIE];
    zombies[i].modelo.visible = true;
    zombies[i].modelo.onloop = nothingZombie;

    // El tamaño visual varia segun el tipo (el tanque es mas grande,
    // el corredor y el toxico un poco mas pequeños/agiles).
    //zombies[i].modelo.scale.x = ESCALA_POR_TIPO[tipo];
    //zombies[i].modelo.scale.y = ESCALA_POR_TIPO[tipo];
    //zombies[i].modelo.scale.z = ESCALA_POR_TIPO[tipo];

    zombies[i].activo = true;
    zombies[i].tipo = tipo;
    zombies[i].esHorda = false;
    zombies[i].vida = VIDA_POR_TIPO[tipo] + (float)random(0, 15);
    zombies[i].velocidad = VELOCIDAD_POR_TIPO[tipo] + ((float)random(0, 15) / 1000.0f);
    zombies[i].ultimoAtaque = 0;
    zombies[i].estadoIA = ZOMBIE_QUIETO;
    zombies[i].posicionInicialX = 0.0f;
    zombies[i].posicionInicialZ = 0.0f;

    generarPosicionZombie(i);

    // Guardamos el punto de origen para poder regresar a el.
    zombies[i].posicionInicialX = zombies[i].modelo.traslation.x / 1000.0f;
    zombies[i].posicionInicialZ = zombies[i].modelo.traslation.z / 1000.0f;
  }

  // Solo una cantidad inicial queda activa; los demas slots se reservan
  // para las futuras hordas.
  for (uint8_t i = ZOMBIES_INICIALES; i < MAX_ZOMBIES; i++) {
    zombies[i].activo = false;
    zombies[i].modelo.visible = false;
  }

  obj2[MODELO_ZOMBIE].visible = false;
}

// ================================================================
// HUD
// ================================================================

void dibujarBarraVida() {
  const int x = 7, y = 47, w = 112, h = 13;
  lcd.fillRect(x, y, w, h, 0x39E7, 255);

  int ancho = (int)((w - 4) * (jugadorZombie.vida / 100.0f));
  if (ancho < 0) ancho = 0;

  uint16_t colorVida = jugadorZombie.vida > 60.0f ? 0x07E0 : (jugadorZombie.vida > 30.0f ? 0xFD20 : 0xF800);
  if (ancho > 0) lcd.fillRect(x + 2, y + 2, ancho, h - 4, colorVida, 255);

  String vida = "VIDA " + String((int)jugadorZombie.vida);
  lcd.drawText(x + 4, y + 29, vida.c_str(), "SANSBOLD", 0xFFFF, INT_TO_FIXED(1));

  String arma = nombreArma(armaEquipada);
  lcd.drawText(x + 4, y + 45, arma.c_str(), "SANSBOLD", 0xFFE0, INT_TO_FIXED(1));

  String mun = "BALAS " + String(inventarioZombie[ITEM_MUNICION].cantidad);
  lcd.drawText(x + 4, y + 60, mun.c_str(), "SANS", 0xFFFF, INT_TO_FIXED(1));

  String resc = "RESCATE " + String(personasRescatadas) + "/" + String(MAX_PERSONAS_RESCATE);
  if (personasEnCarro > 0) resc += " +" + String(personasEnCarro);
  lcd.drawText(x + 4, y + 76, resc.c_str(), "SANSBOLD", 0x07FF, INT_TO_FIXED(1), 120);

  String alb = "ALB " + String((int)vidaAlbergue) + " DEF" + String(nivelAlbergue);
  lcd.drawText(LCD_WIDTH - 118, 47, alb.c_str(), "SANSBOLD", vidaAlbergue > 40 ? 0x07E0 : 0xF800, INT_TO_FIXED(1));

  if (vidaAlbergue <= 0.0f) {
    lcd.drawText(LCD_WIDTH - 118, 80, "ALBERGUE CAIDO", "SANSBOLD", 0xF800, INT_TO_FIXED(1));
  }

  if (numeroHorda > 0) {
    String h = "HORDA " + String(numeroHorda);
    lcd.drawText(LCD_WIDTH - 86, 64, h.c_str(), "SANSBOLD", 0xF81F, INT_TO_FIXED(1));
  }

  if (vidaAlbergue <= 0.0f && jugadorZombie.vivo) {
    lcd.drawText(37, 110, "LOS ZOMBIES VAN POR TI", "SANSBOLD", 0xF800, INT_TO_FIXED(1));
  }
}

void dibujarIndicadores() {
  if (menuAbierto()) return;

  /*if (!jugadorZombie.enCarro) {
    lcd.drawText(7, LCD_HEIGHT - 15,
                 "A/B: atacar  D: inventario",
                 "SANS", 0xFFFF, INT_TO_FIXED(1));
  }*/

  if (!jugadorZombie.enCarro && distanciaAlBaul() <= RADIO_BAUL) {
    lcd.fillRect(46, LCD_HEIGHT - 58, LCD_WIDTH - 92, 19, 0x0000, 205);
    lcd.drawText(56, LCD_HEIGHT - 44,
                 "A: abrir baul", "SANSBOLD", 0xFFFF, INT_TO_FIXED(1));
  }

  float dCarro = distanciaXZ(
    obj2[MODELO_JUGADOR].traslation.x + obj2[MODELO_JUGADOR].position.x,
    obj2[MODELO_JUGADOR].traslation.y + obj2[MODELO_JUGADOR].position.y,
    obj2[MODELO_JUGADOR].traslation.z + obj2[MODELO_JUGADOR].position.z,
    obj2[MODELO_CARRO].traslation.x + obj2[MODELO_CARRO].position.x,
    obj2[MODELO_CARRO].traslation.y + obj2[MODELO_CARRO].position.y,
    obj2[MODELO_CARRO].traslation.z + obj2[MODELO_CARRO].position.z);

  float dAlbergue = distanciaXZ(
    obj2[MODELO_JUGADOR].traslation.x + obj2[MODELO_JUGADOR].position.x,
    obj2[MODELO_JUGADOR].traslation.y + obj2[MODELO_JUGADOR].position.y,
    obj2[MODELO_JUGADOR].traslation.z + obj2[MODELO_JUGADOR].position.z,
    albergueX(), albergueY(), albergueZ());

  if (!jugadorZombie.enCarro && dAlbergue <= RADIO_ENTREGA_ALBERGUE * RADIO_ENTREGA_ALBERGUE) {
    float vidaMaxima = VIDA_ALBERGUE_BASE + nivelAlbergue * 25.0f;
    if (vidaAlbergue < vidaMaxima) {
      lcd.fillRect(20, LCD_HEIGHT - 58, 200, 19, 0x0000, 215);
      lcd.drawText(26, LCD_HEIGHT - 44,
                   "A: reparar (-1 botiquin)",
                   "SANSBOLD", 0x07E0, INT_TO_FIXED(1));
    }
  }

  if (dCarro <= RADIO_CARRO * RADIO_CARRO) {
    lcd.fillRect(46, LCD_HEIGHT - 36, LCD_WIDTH - 92, 19, 0x0000, 205);
    lcd.drawText(56, LCD_HEIGHT - 22,
                 jugadorZombie.enCarro ? "C: salir" : "C: entrar al carro",
                 "SANSBOLD", 0xFFFF, INT_TO_FIXED(1));
  }

  if (jugadorZombie.enCarro) {
    String vel = "KM  " + String((int)(fabsf(jugadorZombie.velocidadCarro) * 100.0f));
    lcd.drawText(LCD_WIDTH - 65, LCD_HEIGHT - 15,
                 vel.c_str(), "SANSBOLD", 0x07E0, INT_TO_FIXED(1));
  }

  if (!jugadorZombie.vivo) {
    lcd.fillRect(20, 92, LCD_WIDTH - 40, 78, 0x0000, 240);
    lcd.drawText(60, 120, "HAS MUERTO", "SANSBOLD", 0xF800, INT_TO_FIXED(1));
    lcd.drawText(48, 146, "D: abrir inventario", "SANS", 0xFFFF, INT_TO_FIXED(1));
  }
}

void dibujarInventario() {
  if (!inventarioAbierto) return;

  lcd.fillRect(10, 10, LCD_WIDTH - 20, LCD_HEIGHT - 20, 0x0000, 235);

  if (mapaInventario) {
    lcd.fillRect(20, 20, LCD_WIDTH - 40, LCD_HEIGHT - 40, 0x18C3, 255);
    lcd.drawRect(20, 20, LCD_WIDTH - 40, LCD_HEIGHT - 40, 0xFFFF, 255);
    lcd.drawText(38, 42, "MAPA", "SANSBOLD", 0xFFE0, INT_TO_FIXED(1));
    lcd.drawText(LCD_WIDTH - 145, 42, "IZQ/DER: volver", "SANS", 0xBDF7, INT_TO_FIXED(1));

    const int mx = 30, my = 58, mw = LCD_WIDTH - 60, mh = LCD_HEIGHT - 105;
    lcd.fillRect(mx, my, mw, mh, 0x0502, 255);
    lcd.drawRect(mx, my, mw, mh, 0x7BEF, 255);

    // Coordenadas del mundo (-90..90) -> mapa en pantalla.
    auto mapX = [&](float x) {
      return mx + 5 + (int)((x + 90.0f) * (mw - 10) / 180.0f);
    };
    auto mapY = [&](float z) {
      return my + 5 + (int)((z + 90.0f) * (mh - 10) / 180.0f);
    };

    // Albergue.
    int ax = mapX(albergueX() / 1000.0f), ay = mapY(albergueZ() / 1000.0f);
    lcd.fillCircle(ax, ay, 6, 0x07E0, 255);
    lcd.drawText(ax + 8, ay, "ALB", "SANSBOLD", 0x07E0, INT_TO_FIXED(1));

    // Carro.
    int cx = mapX((obj2[MODELO_CARRO].traslation.x + obj2[MODELO_CARRO].position.x) / 1000.0f);
    int cz = mapY((obj2[MODELO_CARRO].traslation.z + obj2[MODELO_CARRO].position.z) / 1000.0f);
    lcd.fillCircle(cx, cz, 5, 0xFFE0, 255);
    lcd.drawText(cx + 7, cz, "CAR", "SANS", 0xFFE0, INT_TO_FIXED(1));

    // Jugador.
    int jx = mapX((obj2[MODELO_JUGADOR].traslation.x + obj2[MODELO_JUGADOR].position.x) / 1000.0f);
    int jz = mapY((obj2[MODELO_JUGADOR].traslation.z + obj2[MODELO_JUGADOR].position.z) / 1000.0f);
    lcd.fillCircle(jx, jz, 5, 0xFFFF, 255);
    lcd.drawText(jx + 7, jz, "YO", "SANSBOLD", 0xFFFF, INT_TO_FIXED(1));

    // Personas pendientes.
    for (uint8_t i = 0; i < MAX_PERSONAS_RESCATE; i++) {
      if (!personasRescate[i].activa || personasRescate[i].rescatada) continue;
      int px = mapX((personasRescate[i].x) / 1000.0f), pz = mapY((personasRescate[i].z) / 1000.0f);
      lcd.fillCircle(px, pz, 4, 0xF81F, 255);
    }

    // Zombies activos. Las hordas se marcan diferente.
    for (uint8_t i = 0; i < cantidadZombies; i++) {
      if (!zombies[i].activo) continue;
      int32_t zx = zombies[i].modelo.traslation.x + zombies[i].modelo.position.x;
      int32_t zz = zombies[i].modelo.traslation.z + zombies[i].modelo.position.z;
      int px = mapX(zx / 1000.0f), pz = mapY(zz / 1000.0f);
      lcd.fillCircle(px, pz, zombies[i].esHorda ? 4 : 3,
                     zombies[i].esHorda ? 0xF800 : 0xFD20, 255);
    }

    lcd.drawText(30, LCD_HEIGHT - 30, "B: ju  V: al  A: ca",
                 "SANS", 0xFFFF, INT_TO_FIXED(1));
    return;
  }
  lcd.fillRect(20, 20, LCD_WIDTH - 40, LCD_HEIGHT - 40, 0x18C3, 255);
  lcd.drawRect(20, 20, LCD_WIDTH - 40, LCD_HEIGHT - 40,
               0xFFFF, 255);

  lcd.drawText(37, 42, "INVENTARIO", "SANSBOLD", 0xFFFF, INT_TO_FIXED(1));

  const char *nombres[] = {
    "Cuchillo", "Pistola", "Escopeta",
    "Botiquin", "Municion", "Comida", "Agua"
  };

  for (uint8_t i = 0; i < MAX_SLOTS_INVENTARIO; i++) {
    int y = 66 + i * 25;
    uint16_t fondo = (i == slotInventario) ? 0x39E7 : 0x2104;

    lcd.fillRect(32, y - 15, LCD_WIDTH - 64, 21, fondo, 255);
    lcd.drawText(42, y, nombres[i], "SANS",
                 (i == slotInventario) ? 0xFFE0 : 0xFFFF, INT_TO_FIXED(1));

    String cantidad;
    if (i < 3) {
      cantidad = (i == (uint8_t)armaEquipada) ? "EQUIP" : "";
    } else {
      cantidad = "x" + String(inventarioZombie[i - 3].cantidad);
    }

    lcd.drawText(LCD_WIDTH - 70, y, cantidad.c_str(),
                 "SANSBOLD", 0xFFFF, INT_TO_FIXED(1));
  }

  lcd.drawText(32, LCD_HEIGHT - 30,
               "Izq/Der: mapa   A: equipar/usar   D: cerrar",
               "SANS", 0xBDF7, INT_TO_FIXED(1));
}

void dibujarRuedaArmas() {
  if (!ruedaArmasAbierta) return;

  const int cx = LCD_WIDTH / 2;
  const int cy = LCD_HEIGHT / 2;
  const int radio = 48;

  // Anillo central y tres opciones en forma radial.
  lcd.fillCircle(cx, cy, 60, 0x0000, 120);
  lcd.drawCircle(cx, cy, 61, 0xFFFF, 130);

  const int posX[3] = { cx - radio, cx, cx + radio };
  const int posY[3] = { cy, cy - radio, cy };

  for (uint8_t i = 0; i < 3; i++) {
    uint16_t fondo = (i == armaRuedaSeleccionada) ? 0xFFE0 : 0x39E7;
    lcd.fillCircle(posX[i], posY[i], 20, fondo, 185);
    lcd.drawCircle(posX[i], posY[i], 21, 0xFFFF, 180);
    lcd.drawText(posX[i] - 17, posY[i] + 4,
                 i == 0 ? "CUCH" : (i == 1 ? "PIST" : "ESCO"),
                 "SANSBOLD", 0x0000, INT_TO_FIXED(1));
  }

  lcd.fillCircle(cx, cy, 12, 0x0000, 250);
  lcd.drawText(cx - 10, cy + 4, "C", "SANSBOLD", 0xFFFF, INT_TO_FIXED(1));
  lcd.drawText(53, 32, "MANTEN C: ELEGIR ARMA", "SANSBOLD", 0xFFE0, INT_TO_FIXED(1));
}

const char *nombrePickup(TipoItemZombie tipo) {
  switch (tipo) {
    case ITEM_BOTIQUIN: return "BOTIQUIN";
    case ITEM_MUNICION: return "MUNICION";
    case ITEM_COMIDA: return "COMIDA";
    case ITEM_AGUA: return "AGUA";
  }
  return "OBJETO";
}

bool recogerPickupCercano() {
  int32_t px = obj2[MODELO_JUGADOR].traslation.x + obj2[MODELO_JUGADOR].position.x;
  int32_t py = obj2[MODELO_JUGADOR].traslation.y + obj2[MODELO_JUGADOR].position.y;
  int32_t pz = obj2[MODELO_JUGADOR].traslation.z + obj2[MODELO_JUGADOR].position.z;

  for (uint8_t i = 0; i < MAX_PICKUPS; i++) {
    if (!pickups[i].activo) continue;
    float distp = distanciaXZ(px, py, pz, pickups[i].x + pickups[i].plataforma.position.x, pickups[i].y + pickups[i].plataforma.position.y, pickups[i].z + pickups[i].plataforma.position.z);
    pickups[i].distancia = distp;
    if (distp <= RADIO_RECOGIDA * RADIO_RECOGIDA) {
      agregarAlInventario(pickups[i].tipo, pickups[i].tipo == ITEM_MUNICION ? 10 : 1);
      pickups[i].activo = false;
      pickups[i].plataforma.visible = false;
      return true;
    }
  }
  return false;
}

void dibujarPickups() {
  for (uint8_t i = 0; i < MAX_PICKUPS; i++) {
    if (!pickups[i].activo) continue;

    lcd.drawSceneGameObjectV2(
      pickups[i].plataforma,
      0, 0, 0,
      camaraSupervivencia, pivotCamaraSupervivencia,
      true, INT_TO_FIXED(1));

    float d = distanciaXZ(
      obj2[MODELO_JUGADOR].traslation.x + obj2[MODELO_JUGADOR].position.x,
      obj2[MODELO_JUGADOR].traslation.y + obj2[MODELO_JUGADOR].position.y,
      obj2[MODELO_JUGADOR].traslation.z + obj2[MODELO_JUGADOR].position.z,
      pickups[i].x + pickups[i].plataforma.position.x,
      pickups[i].y + pickups[i].plataforma.position.y,
      pickups[i].z + pickups[i].plataforma.position.z);

    if (d <= 4.0f * 4.0f) {
      const char *txt = nombrePickup(pickups[i].tipo);
      lcd.drawText(LCD_WIDTH / 2 - 28, LCD_HEIGHT / 2 - 24,
                   txt, "SANSBOLD", 0xFFFF, INT_TO_FIXED(1));
      lcd.drawText(LCD_WIDTH / 2 - 34, LCD_HEIGHT / 2 - 9,
                   "A: recoger", "SANS", 0xFFE0, INT_TO_FIXED(1));
    }
  }
}










void actualizarZombies() {
  if (!jugadorZombie.vivo || menuAbierto()) return;

  float multiplicadorNoche = esNoche ? 1.28f : 1.0f;

  // Si el jugador esta en el carro, usamos la posicion del carro como
  // objetivo. Asi los zombies pueden seguir el vehiculo, aunque el jugador
  // este oculto dentro de el.
  float jugadorX;
  float jugadorZ;

  if (jugadorZombie.enCarro) {
    jugadorX = obj2[MODELO_CARRO].traslation.x + obj2[MODELO_CARRO].position.x;
    jugadorZ = obj2[MODELO_CARRO].traslation.z + obj2[MODELO_CARRO].position.z;
  } else {
    jugadorX = obj2[MODELO_JUGADOR].traslation.x + obj2[MODELO_JUGADOR].position.x;
    jugadorZ = obj2[MODELO_JUGADOR].traslation.z + obj2[MODELO_JUGADOR].position.z;
  }

  float jugadorY = obj2[MODELO_JUGADOR].traslation.y + obj2[MODELO_JUGADOR].position.y;

  Vector3D pos_jug = { jugadorX, jugadorY, jugadorZ };

  for (uint8_t i = 0; i < cantidadZombies; i++) {
    if (!zombies[i].activo) continue;

    Obj &z = zombies[i].modelo;

    // Las hordas priorizan el albergue mientras este siga en pie.
    // Cuando el albergue es destruido, cambian inmediatamente su objetivo
    // al jugador y pasan a utilizar la misma logica de persecucion/ataque.
    if (zombies[i].esHorda && vidaAlbergue > 0.0f) {
      float dx = albergueX() / 1000.0f - (z.traslation.x + z.position.x) / 1000.0f;
      float dz = albergueZ() / 1000.0f - (z.traslation.z + z.position.z) / 1000.0f;
      float dist = sqrtf(dx * dx + dz * dz);
      if (dist > 10.25f) {
        int32_t pasoX = dx / dist * zombies[i].velocidad * multiplicadorNoche * 100.0f;
        int32_t pasoZ = dz / dist * zombies[i].velocidad * multiplicadorNoche * 100.0f;
        int32_t nx = z.traslation.x, nz = z.traslation.z;
        moverActorConColision(z, nx, nz, pasoX, pasoZ);
        z.traslation.x = limitarZombie(nx, ZONA_MIN_X, ZONA_MAX_X);
        z.traslation.z = limitarZombie(nz, ZONA_MIN_Z, ZONA_MAX_Z);
        z.rotation.y = atan2f(dx, dz) * 180.0f / PI;
      } else {
        unsigned long ahoraH = millis();
        if (ahoraH - zombies[i].ultimoAtaque >= INTERVALO_ATAQUE_POR_TIPO[zombies[i].tipo]) {
          recibirDanioAlbergue(DANIO_POR_TIPO[zombies[i].tipo] * (1.0f + numeroHorda * 0.12f));
          zombies[i].ultimoAtaque = ahoraH;
        }
      }
      continue;
    }

    // Si el albergue fue destruido, las hordas dejan de atacarlo y
    // entran en la IA normal para perseguir al jugador.
    if (zombies[i].esHorda && vidaAlbergue <= 0.0f) {
      zombies[i].esHorda = false;
      zombies[i].estadoIA = ZOMBIE_PERSIGUIENDO;
    }

    Vector3D pos_p = {
      z.position.x + z.traslation.x,
      z.position.y + z.traslation.y,
      z.position.z + z.traslation.z
    };

    /*Vector3D dirJugador = direccionHaciaJugador(pos_p, pos_jug);
    int32_t distanciaJugador = length(dirJugador);*/
    float distanciaJugador = distanciaXZ(
      pos_jug.x,
      pos_jug.y,
      pos_jug.z,
      pos_p.x,
      pos_p.y,
      pos_p.z);
    zombies[i].distancia = distanciaJugador;

    // ============================================================
    // MAQUINA DE ESTADOS
    //
    // QUIETO:
    //   Espera hasta que el jugador entre a 2 metros.
    //
    // PERSIGUIENDO:
    //   Sigue al jugador hasta que este se aleje mas de 4 metros.
    //
    // REGRESANDO:
    //   Vuelve al punto donde aparecio. Si el jugador vuelve a entrar
    //   a 2 metros durante el regreso, vuelve a perseguirlo.
    // ============================================================

    switch (zombies[i].estadoIA) {

      case ZOMBIE_QUIETO:
        // No hacemos ningun movimiento ni giro.
        if (distanciaJugador <= DETECCION_POR_TIPO[zombies[i].tipo] * DETECCION_POR_TIPO[zombies[i].tipo]) {
          zombies[i].estadoIA = ZOMBIE_PERSIGUIENDO;
        }
        break;

      case ZOMBIE_PERSIGUIENDO:
        // El zombie deja de perseguir cuando se aleja lo suficiente.
        if (distanciaJugador > PERDIDA_POR_TIPO[zombies[i].tipo] * PERDIDA_POR_TIPO[zombies[i].tipo]) {
          zombies[i].estadoIA = ZOMBIE_REGRESANDO;
          break;
        }

        // Perseguir hasta llegar al rango de ataque.
        if (distanciaJugador > ATAQUE_RADIO_POR_TIPO[zombies[i].tipo] * ATAQUE_RADIO_POR_TIPO[zombies[i].tipo]) {

          float inv = (1.0f * ATAQUE_RADIO_POR_TIPO[zombies[i].tipo]) / (distanciaJugador);

          int32_t pasoX = (int32_t)(((pos_jug.x - pos_p.x) / 1000.0f) * inv * zombies[i].velocidad * multiplicadorNoche * 100.0f);
          int32_t pasoZ = (int32_t)(((pos_jug.z - pos_p.z) / 1000.0f) * inv * zombies[i].velocidad * multiplicadorNoche * 100.0f);

          int32_t nuevoZombieX = z.traslation.x;
          int32_t nuevoZombieZ = z.traslation.z;

          // Los zombies tambien respetan paredes, casas, arboles, etc.
          moverActorConColision(
            z,
            nuevoZombieX,
            nuevoZombieZ,
            pasoX,
            pasoZ);

          z.traslation.x = limitarZombie(nuevoZombieX, ZONA_MIN_X, ZONA_MAX_X);
          z.traslation.z = limitarZombie(nuevoZombieZ, ZONA_MIN_Z, ZONA_MAX_Z);
        }

        // Solo gira cuando esta activo persiguiendo.
        if (distanciaJugador > 1.0f) {
          z.rotation.y = atan2f((pos_jug.x - pos_p.x), (pos_jug.z - pos_p.z)) * 180.0f / PI;
        }
        break;

      case ZOMBIE_REGRESANDO:
        {
          // Si el jugador vuelve a acercarse, el zombie lo detecta otra vez.
          if (distanciaJugador <= DETECCION_POR_TIPO[zombies[i].tipo] * DETECCION_POR_TIPO[zombies[i].tipo]) {
            zombies[i].estadoIA = ZOMBIE_PERSIGUIENDO;
            break;
          }

          // Direccion hacia el punto original.
          float dx = zombies[i].posicionInicialX - (z.traslation.x) / 1000.0f;
          float dz = zombies[i].posicionInicialZ - (z.traslation.z) / 1000.0f;
          float distanciaOrigen = dx * dx + dz * dz;

          if (distanciaOrigen <= 80.0f * 80.0f) {
            // Ajuste final para evitar oscilaciones.
            z.traslation.x = zombies[i].posicionInicialX * 1000;
            z.traslation.z = zombies[i].posicionInicialZ * 1000;
            zombies[i].estadoIA = ZOMBIE_QUIETO;
            break;
          }

          float inv = (1.0f * 80.0f) / distanciaOrigen;
          float velocidadRegreso = zombies[i].velocidad * 0.85f;

          int32_t pasoRegresoX = (int32_t)(dx * inv * velocidadRegreso * multiplicadorNoche) * 1000;
          int32_t pasoRegresoZ = (int32_t)(dz * inv * velocidadRegreso * multiplicadorNoche) * 1000;

          int32_t nuevoRegresoX = z.traslation.x;
          int32_t nuevoRegresoZ = z.traslation.z;

          moverActorConColision(
            z,
            nuevoRegresoX,
            nuevoRegresoZ,
            pasoRegresoX,
            pasoRegresoZ);

          z.traslation.x = limitarZombie(nuevoRegresoX, ZONA_MIN_X, ZONA_MAX_X);
          z.traslation.z = limitarZombie(nuevoRegresoZ, ZONA_MIN_Z, ZONA_MAX_Z);

          // Mirar hacia el punto de origen mientras regresa.
          z.rotation.y = atan2f(dx, dz) * 180.0f / PI;
          break;
        }
    }

    // ============================================================
    // ATAQUE
    // ============================================================
    // El ataque solo existe mientras el zombie esta persiguiendo.
    // No puede atacar mientras esta quieto o regresando.
    if (zombies[i].estadoIA == ZOMBIE_PERSIGUIENDO && !jugadorZombie.enCarro && distanciaJugador <= ATAQUE_RADIO_POR_TIPO[zombies[i].tipo] * ATAQUE_RADIO_POR_TIPO[zombies[i].tipo]) {

      unsigned long ahora = millis();

      if (ahora - zombies[i].ultimoAtaque >= INTERVALO_ATAQUE_POR_TIPO[zombies[i].tipo]) {

        aplicarDanioJugador(DANIO_POR_TIPO[zombies[i].tipo]);

        // Destello solamente por golpe directo
        destelloDanioHasta = millis() + 100;

        // El zombie toxico ademas envenena: dano continuo tras el golpe.
        if (zombies[i].tipo == ZOMBIE_TOXICO) {
          jugadorZombie.envenenado = true;
          jugadorZombie.finVeneno = ahora + DURACION_VENENO;
          jugadorZombie.siguienteTicVeneno = ahora + INTERVALO_TIC_VENENO;
        }

        zombies[i].ultimoAtaque = ahora;
      }
    }
  }
}

void aplicarRetrocesoZombie(uint8_t i, float fuerza) {
  if (i >= cantidadZombies || !zombies[i].activo) return;

  int32_t px = obj2[MODELO_JUGADOR].traslation.x + obj2[MODELO_JUGADOR].position.x;
  int32_t py = obj2[MODELO_JUGADOR].traslation.y + obj2[MODELO_JUGADOR].position.y;
  int32_t pz = obj2[MODELO_JUGADOR].traslation.z + obj2[MODELO_JUGADOR].position.z;

  int32_t zx = zombies[i].modelo.traslation.x + zombies[i].modelo.position.x;
  int32_t zy = zombies[i].modelo.traslation.y + zombies[i].modelo.position.y;
  int32_t zz = zombies[i].modelo.traslation.z + zombies[i].modelo.position.z;

  Vector3D dirJugador = direccionHaciaJugador({ px, py, pz }, { zx, zy, zz });
  float distancia = length(dirJugador);

  if (distancia < 0.001f * 0.001f) {
    // Caso excepcional: si estan exactamente en el mismo punto, usamos
    // la direccion en la que mira el jugador.
    dirJugador.x = jugadorZombie.dirX;
    dirJugador.z = jugadorZombie.dirZ;
    distancia = dirJugador.x * dirJugador.x + dirJugador.z * dirJugador.z;
  }

  dirJugador.x /= distancia;
  dirJugador.z /= distancia;

  int32_t nuevoX = zombies[i].modelo.traslation.x;
  int32_t nuevoZ = zombies[i].modelo.traslation.z;

  // El retroceso tambien respeta paredes y objetos del escenario.
  moverActorConColision(
    zombies[i].modelo,
    nuevoX, nuevoZ,
    dirJugador.x * fuerza, dirJugador.z * fuerza);

  zombies[i].modelo.traslation.x = limitarZombie(nuevoX, ZONA_MIN_X, ZONA_MAX_X);
  zombies[i].modelo.traslation.z = limitarZombie(nuevoZ, ZONA_MIN_Z, ZONA_MAX_Z);
}








void actualizarCarro() {
  if (!jugadorZombie.vivo || menuAbierto()) return;

  float d = distanciaXZ(
    obj2[MODELO_JUGADOR].traslation.x + obj2[MODELO_JUGADOR].position.x,
    obj2[MODELO_JUGADOR].traslation.y + obj2[MODELO_JUGADOR].position.y,
    obj2[MODELO_JUGADOR].traslation.z + obj2[MODELO_JUGADOR].position.z,
    obj2[MODELO_CARRO].traslation.x + obj2[MODELO_CARRO].position.x,
    obj2[MODELO_CARRO].traslation.y + obj2[MODELO_CARRO].position.y,
    obj2[MODELO_CARRO].traslation.z + obj2[MODELO_CARRO].position.z);

  bool carroCerca = d <= RADIO_CARRO * RADIO_CARRO;

  // La tecla C se procesa al soltarla para distinguir pulsacion corta de
  // pulsacion larga. La pulsacion larga abre la rueda de seleccion de armas.
  bool cSoltada = cAnterior && !mC.t_C;
  if (cSoltada) {
    unsigned long duracionC = millis() - inicioPulsacionC;

    if (duracionC >= DURACION_C_LARGO_MS) {
      // Si ya estaba abierta durante la pulsacion, aqui se confirma la seleccion.
      if (ruedaArmasAbierta) {
        armaEquipada = (ArmaZombie)armaRuedaSeleccionada;
        ruedaArmasAbierta = false;
      } else {
        ruedaArmasAbierta = true;
        armaRuedaSeleccionada = (uint8_t)armaEquipada;
      }
    } else if (!ruedaArmasAbierta && carroCerca) {
      jugadorZombie.enCarro = !jugadorZombie.enCarro;

      if (jugadorZombie.enCarro) {
        pivotCamaraSupervivencia.y = 14000;
        obj2[MODELO_JUGADOR].visible = false;
        obj2[MODELO_CARRO].visible = true;
        //obj2[MODELO_CARRO].traslation.x = obj2[MODELO_JUGADOR].traslation.x;
        //obj2[MODELO_CARRO].traslation.z = obj2[MODELO_JUGADOR].traslation.z;
        obj2[MODELO_JUGADOR].traslation.x = obj2[MODELO_CARRO].traslation.x + obj2[MODELO_CARRO].position.x;
        obj2[MODELO_JUGADOR].traslation.z = obj2[MODELO_CARRO].traslation.z + obj2[MODELO_CARRO].position.y;
        jugadorZombie.angulo = obj2[MODELO_CARRO].rotation.y;
        jugadorZombie.velocidadCarro = 0.0f;
      } else {
        pivotCamaraSupervivencia.y = 9000;
        obj2[MODELO_JUGADOR].visible = true;
        jugadorZombie.velocidadCarro = 0.0f;

        // Sale por el costado, respetando la orientacion del carro.
        float rad = (jugadorZombie.angulo + 90.0f) * PI / 180.0f;
        obj2[MODELO_JUGADOR].traslation.x = obj2[MODELO_CARRO].traslation.x + cosf(jugadorZombie.angulo + 90.0f) * 1.8f;
        obj2[MODELO_JUGADOR].traslation.z = obj2[MODELO_CARRO].traslation.z + sinf(jugadorZombie.angulo + 90.0f) * 1.8f;
        jugadorZombie.dirX = sinf(jugadorZombie.angulo);
        jugadorZombie.dirZ = cosf(jugadorZombie.angulo);
      }
    }
  }

  if (pulsacionB && jugadorZombie.enCarro) {
    jugadorZombie.enCarro = !jugadorZombie.enCarro;
    pivotCamaraSupervivencia.y = 9000;
    obj2[MODELO_JUGADOR].visible = true;
    jugadorZombie.velocidadCarro = 0.0f;

    // Sale por el costado, respetando la orientacion del carro.
    float rad = (jugadorZombie.angulo + 90.0f) * PI / 180.0f;
    obj2[MODELO_JUGADOR].traslation.x = obj2[MODELO_CARRO].traslation.x + cosf(jugadorZombie.angulo + 90.0f) * 1.8f;
    obj2[MODELO_JUGADOR].traslation.z = obj2[MODELO_CARRO].traslation.z + sinf(jugadorZombie.angulo + 90.0f) * 1.8f;
    jugadorZombie.dirX = sinf(jugadorZombie.angulo);
    jugadorZombie.dirZ = cosf(jugadorZombie.angulo);
  }
}

void soltarPickupDesdeZombie(float x, float y, float z, float px, float py, float pz) {
  for (uint8_t i = 0; i < MAX_PICKUPS; i++) {
    if (!pickups[i].activo) {
      uint8_t r = random(0, 100);
      TipoItemZombie tipo;
      if (r < 50) tipo = ITEM_MUNICION;
      else if (r < 72) tipo = ITEM_COMIDA;
      else if (r < 90) tipo = ITEM_AGUA;
      else tipo = ITEM_BOTIQUIN;
      generarPickup(i, tipo, x, y, z, px, py, pz);
      return;
    }
  }
}

void eliminarZombie(uint8_t i) {
  if (!zombies[i].activo) return;
  float x = (zombies[i].modelo.traslation.x) / 1000.0f;
  float y = (zombies[i].modelo.traslation.y) / 1000.0f;
  float z = (zombies[i].modelo.traslation.z) / 1000.0f;
  zombies[i].activo = false;
  zombies[i].modelo.visible = false;
  soltarPickupDesdeZombie(x, y, z, obj2[MODELO_PLATAFORMA].position.x / 1000.0f, obj2[MODELO_PLATAFORMA].position.y / 1000.0f, obj2[MODELO_PLATAFORMA].position.z / 1000.0f);
}

void comprobarAtropellosCarro(float velocidad) {
  float cx = obj2[MODELO_CARRO].traslation.x + obj2[MODELO_CARRO].position.x;
  float cy = obj2[MODELO_CARRO].traslation.y + obj2[MODELO_CARRO].position.y;
  float cz = obj2[MODELO_CARRO].traslation.z + obj2[MODELO_CARRO].position.z;

  float velocidadAbs = fabsf(velocidad);
  if (velocidadAbs < VELOCIDAD_MIN_ATROPELLO) return;

  float dirX = jugadorZombie.dirX;
  float dirZ = jugadorZombie.dirZ;
  if (velocidad < 0.0f) {
    dirX = -dirX;
    dirZ = -dirZ;
  }

  for (uint8_t i = 0; i < cantidadZombies; i++) {
    if (!zombies[i].activo) continue;

    float zx = zombies[i].modelo.traslation.x + zombies[i].modelo.position.x;
    float zy = zombies[i].modelo.traslation.y + zombies[i].modelo.position.y;
    float zz = zombies[i].modelo.traslation.z + zombies[i].modelo.position.z;

    Vector3D dirJugador = direccionHaciaJugador({ zx, zy, zz }, { cx, cy, cz });
    float distancia = length(dirJugador);

    if (distancia > RADIO_ATROPELLO_CARRO * RADIO_ATROPELLO_CARRO) continue;

    // Evita que un zombie al lado del carro sea atropellado sin que el
    // vehiculo realmente avance hacia el.
    float inv = (1.0f * 0.001f) / (distancia > 0.001f * 0.001f ? distancia : 0.001f * 0.001f);
    float dot = dirX * (dirJugador.x * inv) + dirZ * (dirJugador.z * inv);
    if (dot < -0.35f) continue;

    // A mayor velocidad, mayor impacto.
    float danio = 70.0f + velocidadAbs * 60.0f;
    zombies[i].vida -= danio;

    // Empujon adicional para que el zombie no quede debajo del carro.
    //aplicarRetrocesoZombie(i, 0.35f + velocidadAbs * 0.35f);

    if (zombies[i].vida <= 0.0f) {
      eliminarZombie(i);
    }
  }
}

// ================================================================
// IA
// ================================================================

void activarZombieHorda(uint8_t indice) {
  if (indice >= MAX_ZOMBIES) return;
  TipoZombie tipo = elegirTipoZombieAleatorio();
  zombies[indice].activo = true;
  zombies[indice].esHorda = true;
  zombies[indice].tipo = tipo;
  zombies[indice].modelo = obj2[MODELO_ZOMBIE];
  zombies[indice].modelo.visible = true;
  zombies[indice].modelo.scale.x = ESCALA_POR_TIPO[tipo];
  zombies[indice].modelo.scale.y = ESCALA_POR_TIPO[tipo];
  zombies[indice].modelo.scale.y = ESCALA_POR_TIPO[tipo];
  zombies[indice].vida = (VIDA_POR_TIPO[tipo] + (float)random(0, 20)) * (1.0f + numeroHorda * 0.08f);
  zombies[indice].velocidad = VELOCIDAD_POR_TIPO[tipo] + ((float)random(0, 15) / 1000.0f);
  zombies[indice].ultimoAtaque = 0;
  zombies[indice].estadoIA = ZOMBIE_PERSIGUIENDO;

  // Aparecen en el borde del mapa, en un punto cercano al albergue.
  float ang = (float)random(0, 628) / 100.0f;
  float rr = 55.0f + (float)random(0, 20);
  int32_t radio = rr * 1000;
  zombies[indice].modelo.traslation.x = limitarZombie(albergueX() + cosf(ang * 57.29577951308232f) * radio, ZONA_MIN_X + 2000, ZONA_MAX_X - 2000);
  zombies[indice].modelo.traslation.y = 0.0f;
  zombies[indice].modelo.traslation.z = limitarZombie(albergueZ() + sinf(ang * 57.29577951308232f) * radio, ZONA_MIN_Z + 2000, ZONA_MAX_Z - 2000);
  zombies[indice].posicionInicialX = zombies[indice].modelo.traslation.x / 1000.0f;
  zombies[indice].posicionInicialZ = zombies[indice].modelo.traslation.z / 1000.0f;
}

void actualizarHordas() {
  unsigned long ahora = millis();
  if (inicioHordas == 0) {
    inicioHordas = ahora;
    siguienteHorda = ahora + PRIMERA_HORDA_MS;
    return;
  }
  if (ahora < siguienteHorda || vidaAlbergue <= 0.0f) return;

  numeroHorda++;
  hordaActiva = true;
  zombiesHordaActual = ZOMBIES_POR_HORDA_BASE + (numeroHorda - 1) * 2;

  uint8_t activados = 0;
  for (uint8_t i = 0; i < MAX_ZOMBIES && activados < zombiesHordaActual; i++) {
    if (!zombies[i].activo) {
      activarZombieHorda(i);
      activados++;
    }
  }
  siguienteHorda = ahora + INTERVALO_HORDA_MS;
}

// ================================================================
// Combate
// ================================================================

int objetivoZombie(float alcance) {
  int32_t px = obj2[MODELO_JUGADOR].traslation.x + obj2[MODELO_JUGADOR].position.x;
  int32_t py = obj2[MODELO_JUGADOR].traslation.y + obj2[MODELO_JUGADOR].position.y;
  int32_t pz = obj2[MODELO_JUGADOR].traslation.z + obj2[MODELO_JUGADOR].position.z;

  int objetivo = -1;
  float mejor = alcance * alcance + 1.0f;

  for (uint8_t i = 0; i < cantidadZombies; i++) {
    if (!zombies[i].activo) continue;

    int32_t dx = zombies[i].modelo.traslation.x + zombies[i].modelo.position.x;
    int32_t dy = zombies[i].modelo.traslation.y + zombies[i].modelo.position.y;
    int32_t dz = zombies[i].modelo.traslation.z + zombies[i].modelo.position.z;

    Vector3D dirJugador = direccionHaciaJugador({ dx, dy, dz }, { px, py, pz });
    float d = length(dirJugador);

    if (d > alcance * alcance || d < 0.01f * 0.01f) continue;

    float inv = (1.0f * 0.01f) / d;
    float dot = productoPunto2D(jugadorZombie.dirX, jugadorZombie.dirZ,
                                (dx / 1000.0f) * inv, (dz / 1000.0f) * inv);
    //if (dot >= COS_ANGULO_ATAQUE && d < mejor) {
    if (d < mejor * mejor) {
      mejor = d;
      objetivo = i;
    }
  }
  return objetivo;
}

void ataqueCuchillo() {
  if (millis() - ultimoAtaque < COOLDOWN_CUCHILLO) return;
  ultimoAtaque = millis();

  int objetivo = objetivoZombie(ALCANCE_CUCHILLO);
  if (objetivo < 0) return;
  zombies[objetivo].vida -= DANIO_CUCHILLO;
  if (zombies[objetivo].vida <= 0.0f) eliminarZombie(objetivo);
}

void ataquePistola() {
  if (millis() - ultimoAtaque < COOLDOWN_PISTOLA) return;
  if (inventarioZombie[ITEM_MUNICION].cantidad < 1) return;
  ultimoAtaque = millis();
  inventarioZombie[ITEM_MUNICION].cantidad--;

  int objetivo = objetivoZombie(ALCANCE_PISTOLA);
  if (objetivo < 0) return;
  zombies[objetivo].vida -= DANIO_PISTOLA;
  if (zombies[objetivo].vida <= 0.0f) eliminarZombie(objetivo);
  aplicarRetrocesoZombie(objetivo, 0.40f);
}

void ataqueEscopeta() {
  if (millis() - ultimoAtaque < COOLDOWN_ESCOPETA) return;
  if (inventarioZombie[ITEM_MUNICION].cantidad < 2) return;
  ultimoAtaque = millis();
  inventarioZombie[ITEM_MUNICION].cantidad -= 2;

  // La escopeta golpea hasta 3 zombies dentro de un cono corto.
  uint8_t golpeados = 0;
  int32_t px = obj2[MODELO_JUGADOR].traslation.x + obj2[MODELO_JUGADOR].position.x;
  int32_t py = obj2[MODELO_JUGADOR].traslation.y + obj2[MODELO_JUGADOR].position.y;
  int32_t pz = obj2[MODELO_JUGADOR].traslation.z + obj2[MODELO_JUGADOR].position.z;

  for (uint8_t i = 0; i < cantidadZombies && golpeados < 3; i++) {
    if (!zombies[i].activo) continue;

    int32_t dx = (zombies[i].modelo.traslation.x + zombies[i].modelo.position.x);
    int32_t dy = (zombies[i].modelo.traslation.y + zombies[i].modelo.position.y);
    int32_t dz = (zombies[i].modelo.traslation.z + zombies[i].modelo.position.z);

    Vector3D dirJugador = direccionHaciaJugador({ dx, dy, dz }, { px, py, pz });
    float d = length(dirJugador);
    if (d > ALCANCE_ESCOPETA * ALCANCE_ESCOPETA || d < 0.01f * 0.01f) continue;

    float inv = (1.0f * 0.01f) / d;
    float dot = productoPunto2D(jugadorZombie.dirX, jugadorZombie.dirZ,
                                (dx / 1000.0f) * inv, (dz / 1000.0f) * inv);
    //if (dot < 0.20f) continue;

    // Daño decrece con distancia.
    float factor = 1.0f - (d / ALCANCE_ESCOPETA * ALCANCE_ESCOPETA) * 0.45f;
    zombies[i].vida -= DANIO_ESCOPETA * factor;
    golpeados++;
    if (zombies[i].vida <= 0.0f) eliminarZombie(i);
    aplicarRetrocesoZombie(i, 1.40f);
  }
}

void atacarConArma() {
  if (!jugadorZombie.vivo || jugadorZombie.enCarro || inventarioAbierto) return;

  switch (armaEquipada) {
    case ARMA_CUCHILLO: ataqueCuchillo(); break;
    case ARMA_PISTOLA: ataquePistola(); break;
    case ARMA_ESCOPETA: ataqueEscopeta(); break;
  }
}

void actualizarArmas() {
  if (!jugadorZombie.vivo || jugadorZombie.enCarro || menuAbierto()) return;

  if (pulsacionA) {
    if (!recogerPickupCercano()) atacarConArma();
  }

  if (pulsacionB) atacarConArma();
}

void conducirCarro() {
  if (!jugadorZombie.enCarro || menuAbierto() || !jugadorZombie.vivo) return;

  float velocidad = jugadorZombie.velocidadCarro;
  /*if (mC.t_C) {
    jugadorZombie.enCarro = false;
    return;
  }*/
  // Acelerar / reversa.
  if (mC.t_arriba) {
    velocidad += aceleracionCarro();
  } else if (mC.t_abajo) {
    velocidad -= aceleracionCarro();
  } else {
    // Inercia: desaceleracion suave cuando se suelta el joystick.
    if (velocidad > 0.0f) velocidad -= CARRO_FRENADO_SUAVE;
    else if (velocidad < 0.0f) velocidad += CARRO_FRENADO_SUAVE;
  }

  velocidad = limitarZombie(velocidad,
                            -CARRO_VELOCIDAD_REVERSA,
                            velocidadMaximaCarro());

  // Frenado adicional al ir hacia atras/soltar.
  if (mC.t_abajo && velocidad > 0.05f) {
    velocidad -= CARRO_FRENADO;
  }

  // Direccion: cuanto mas rapido, mas gira el vehiculo.
  float factorGiro = 0.45f + fabsf(velocidad) / velocidadMaximaCarro();
  if (mC.t_izquierda) jugadorZombie.angulo -= CARRO_GIRO * factorGiro * 100.0f;
  if (mC.t_derecha) jugadorZombie.angulo += CARRO_GIRO * factorGiro * 100.0f;

  jugadorZombie.velocidadCarro = velocidad;

  float rad = jugadorZombie.angulo * PI / 180.0f;
  // LUT Q15: el calculo trigonometrico usa solamente enteros.
  float anguloDirDeg = jugadorZombie.angulo;
  jugadorZombie.dirX = (float)sinf(anguloDirDeg);
  jugadorZombie.dirZ = (float)cosf(anguloDirDeg);

  obj2[MODELO_CARRO].rotation.y = jugadorZombie.angulo + 270.0f;

  int32_t nuevoCarroX = obj2[MODELO_CARRO].traslation.x;
  int32_t nuevoCarroZ = obj2[MODELO_CARRO].traslation.z;

  // El carro tambien usa las colisiones del escenario.
  moverActorConColision(
    obj2[MODELO_CARRO],
    nuevoCarroX, nuevoCarroZ,
    jugadorZombie.dirX * velocidad,
    jugadorZombie.dirZ * velocidad);

  obj2[MODELO_CARRO].traslation.x = limitarZombie(nuevoCarroX,
                                                  ZONA_MIN_X, ZONA_MAX_X);
  obj2[MODELO_CARRO].traslation.z = limitarZombie(nuevoCarroZ,
                                                  ZONA_MIN_Z, ZONA_MAX_Z);

  // El carro puede atravesar a los zombies y atropellarlos.
  comprobarAtropellosCarro(velocidad);

  // El jugador permanece sentado dentro del carro.
  obj2[MODELO_JUGADOR].traslation.x = obj2[MODELO_CARRO].traslation.x;
  obj2[MODELO_JUGADOR].traslation.z = obj2[MODELO_CARRO].traslation.z;
  obj2[MODELO_JUGADOR].rotation.y = jugadorZombie.angulo;

  pivotCamaraSupervivencia.x = obj2[MODELO_JUGADOR].traslation.x + 4.0f;
  pivotCamaraSupervivencia.z = obj2[MODELO_JUGADOR].traslation.z + 8.0f;
}

void dibujarBaul() {
  if (!baulAbierto) return;

  lcd.fillRect(10, 10, LCD_WIDTH - 20, LCD_HEIGHT - 20, 0x0000, 235);
  lcd.fillRect(20, 20, LCD_WIDTH - 40, LCD_HEIGHT - 40, 0x18C3, 255);
  lcd.drawRect(20, 20, LCD_WIDTH - 40, LCD_HEIGHT - 40,
               0xFFFF, 255);

  lcd.drawText(37, 42, "BAUL DEL CARRO", "SANSBOLD", 0xFFFF, INT_TO_FIXED(1));
  String cap = "CAP " + String(totalBaul()) + "/" + String(capacidadBaul());
  lcd.drawText(150, 42, cap.c_str(), "SANSBOLD", 0x07FF, INT_TO_FIXED(1));

  const int nombreX = 32;
  const int qtyMochilaX = 138;
  const int qtyBaulX = 188;

  lcd.drawText(qtyMochilaX - 8, 62, "MOCHILA", "SANSBOLD",
               (ladoBaul == 0) ? 0xFFE0 : 0xBDF7, INT_TO_FIXED(1));
  lcd.drawText(qtyBaulX - 4, 62, "BAUL", "SANSBOLD",
               (ladoBaul == 1) ? 0xFFE0 : 0xBDF7, INT_TO_FIXED(1));

  const char *nombres[] = { "Botiquin", "Municion", "Comida", "Agua" };

  for (uint8_t i = 0; i < NUM_ITEMS_INVENTARIO; i++) {
    int y = 88 + i * 25;
    uint16_t fondo = (i == slotBaul) ? 0x39E7 : 0x2104;

    lcd.fillRect(30, y - 15, LCD_WIDTH - 60, 21, fondo, 255);
    lcd.drawText(nombreX, y, nombres[i], "SANS", 0xFFFF, INT_TO_FIXED(1));

    String cantMochila = "x" + String(inventarioZombie[i].cantidad);
    String cantBaul = "x" + String(baulZombie[i]);

    lcd.drawText(qtyMochilaX, y, cantMochila.c_str(),
                 "SANSBOLD", 0xFFFF, INT_TO_FIXED(1));
    lcd.drawText(qtyBaulX, y, cantBaul.c_str(),
                 "SANSBOLD", 0xFFFF, INT_TO_FIXED(1));
  }

  lcd.drawText(30, LCD_HEIGHT - 30,
               "Izq/Der: lado  A: mover  B: mejoras  D: cerrar",
               "SANS", 0xBDF7, INT_TO_FIXED(1));

  if (mejorasAbiertas) {
    lcd.fillRect(24, 72, LCD_WIDTH - 48, 132, 0x0000, 245);
    lcd.drawRect(28, 76, LCD_WIDTH - 56, 124, 0xFFFF, 255);
    lcd.drawText(48, 94, "MEJORAS DEL ALBERGUE", "SANSBOLD", 0xFFE0, INT_TO_FIXED(1));
    const char *nombresMejora[] = { "DEFENSA", "BAUL", "CARRO" };
    uint8_t niveles[] = { nivelAlbergue, nivelBaul, nivelCarro };
    for (uint8_t i = 0; i < 3; i++) {
      int y = 120 + i * 22;
      uint16_t c = (i == mejoraSeleccionada) ? 0xFFE0 : 0xFFFF;
      String txt = String(nombresMejora[i]) + "  NIV " + String(niveles[i]);
      if (i == 0) txt += "  VIDA+25/DEF";
      if (i == 1) txt += "  CAP+10";
      if (i == 2) txt += "  VEL+10%";
      if (niveles[i] < (i == 0 ? NIVEL_MAX_ALBERGUE : (i == 1 ? NIVEL_MAX_BAUL : NIVEL_MAX_CARRO)))
        txt += "  COST " + String(costoMejora(i, niveles[i]));
      else txt += "  MAX";
      lcd.drawText(40, y, txt.c_str(), "SANSBOLD", c, INT_TO_FIXED(1));
    }
    lcd.drawText(40, 188, "A: comprar   B: volver", "SANS", 0xBDF7, INT_TO_FIXED(1));
  }
}


// El albergue puede repararse cuando el jugador se encuentra cerca.
// Se consume un botiquin y se recuperan 25 puntos, sin superar su vida maxima.
/*bool repararAlbergue() {
  if (!jugadorZombie.vivo || jugadorZombie.enCarro) return false;

  float d = distanciaXZ(
    obj2[MODELO_JUGADOR].pivotx + obj2[MODELO_JUGADOR].origenx,
    obj2[MODELO_JUGADOR].pivoty + obj2[MODELO_JUGADOR].origeny,
    obj2[MODELO_JUGADOR].pivotz + obj2[MODELO_JUGADOR].origenz,
    albergueX(), albergueY(), albergueZ()
  );

  if (d > RADIO_ENTREGA_ALBERGUE) return false;

  float vidaMaxima = VIDA_ALBERGUE_BASE + nivelAlbergue * 25.0f;
  if (vidaAlbergue >= vidaMaxima) return false;
  if (inventarioZombie[ITEM_BOTIQUIN].cantidad < COSTO_REPARACION_ALBERGUE) return false;

  inventarioZombie[ITEM_BOTIQUIN].cantidad -= COSTO_REPARACION_ALBERGUE;
  vidaAlbergue += REPARACION_ALBERGUE;
  if (vidaAlbergue > vidaMaxima) vidaAlbergue = vidaMaxima;
  return true;
}*/


void juegoSetup() {
  for (int i = 0; i < n_obj2; i++) {
    lcd.getCentroid(obj2[i], 1);
  }

  lcd.configInitGame(camaraSupervivencia);
  lcd.beginGameLighting(obj2, n_obj2);
  lcd.setBufferingZ(true);
  lcd.setShadows(true);

  randomSeed((unsigned long)micros());

  jugadorZombie.vida = 100.0f;
  jugadorZombie.vivo = true;
  jugadorZombie.enCarro = false;
  jugadorZombie.dirX = 0.0f;
  jugadorZombie.dirZ = -1.0f;
  jugadorZombie.angulo = 0.0f;
  jugadorZombie.velocidadCarro = 0.0f;
  jugadorZombie.envenenado = false;
  jugadorZombie.finVeneno = 0UL;
  jugadorZombie.siguienteTicVeneno = 0UL;

  armaEquipada = ARMA_PISTOLA;
  slotInventario = 1;
  inventarioAbierto = false;
  mapaInventario = false;

  baulAbierto = false;
  mejorasAbiertas = false;
  mejoraSeleccionada = 0;
  slotBaul = 0;
  ladoBaul = 0;
  for (uint8_t i = 0; i < NUM_ITEMS_INVENTARIO; i++) baulZombie[i] = 0;
  nivelAlbergue = 0;
  nivelBaul = 0;
  nivelCarro = 0;
  vidaAlbergue = VIDA_ALBERGUE_BASE;
  inicioHordas = 0;
  siguienteHorda = 0;
  numeroHorda = 0;
  personasRescatadas = 0;
  personasEnCarro = 0;
  hordaActiva = false;
  zombiesHordaActual = 0;

  inicioCicloDia = millis();
  horaJuego = 8.0f;
  esNoche = false;

  obj2[MODELO_JUGADOR].traslation.x = 0;
  obj2[MODELO_JUGADOR].traslation.z = 0;
  obj2[MODELO_JUGADOR].visible = true;

  obj2[MODELO_CARRO].traslation.x = INT_TO_FIXED(16) / 100;
  obj2[MODELO_CARRO].traslation.y = 0;
  obj2[MODELO_CARRO].traslation.z = INT_TO_FIXED(8) / 100;
  obj2[MODELO_CARRO].rotation.y = 0;
  obj2[MODELO_CARRO].visible = true;

  inicializarZombies();
  inicializarPickups();
  /*inicializarPersonasRescate();*/

  anteriorA = anteriorB = anteriorC = anteriorD = false;
  anteriorArriba = anteriorAbajo = false;
  anteriorIzquierda = anteriorDerecha = false;
  cAnterior = false;
  cEnPulsacion = false;
  inicioPulsacionC = 0UL;
  ruedaArmasAbierta = false;
  armaRuedaSeleccionada = (uint8_t)armaEquipada;
}

void juegoLoop() {

  // B siempre regresa al menú desde la prueba 3D.
  /*if (mC.t_B) {
    watch_screen = STATE_MENU;
    return;
  }*/

  //lcd.clear(0x03f8);
  lcd.clear(0x0000);

  actualizarDiaNoche();

  actualizarRuedaArmas();

  // Flancos de subida calculados una sola vez por frame, antes de que
  // ningun sistema (inventario, armas, carro, baul) los consuma.
  pulsacionA = mC.t_A && !anteriorA;
  pulsacionB = mC.t_B && !anteriorB;
  pulsacionC = mC.t_C && !anteriorC;
  pulsacionD = mC.t_D && !anteriorD;
  pulsacionArriba = mC.t_arriba && !anteriorArriba;
  pulsacionAbajo = mC.t_abajo && !anteriorAbajo;
  pulsacionIzquierda = mC.t_izquierda && !anteriorIzquierda;
  pulsacionDerecha = mC.t_derecha && !anteriorDerecha;

  actualizarInventario();
  actualizarBaul();

  if (!menuAbierto() && (!ruedaArmasAbierta || (!mC.t_C && cAnterior))) {
    actualizarCarro();

    if (jugadorZombie.enCarro) conducirCarro();
    else moverJugadorAPie();

    // Cerca del albergue, A intenta repararlo antes de ejecutar el ataque.
    /*if (pulsacionA && repararAlbergue()) {
      pulsacionA = false;
    }*/

    actualizarArmas();
    actualizarHordas();
    actualizarZombies();
    actualizarVeneno();
    //actualizarRescatePersonas();
    recogerPickupCercano();
  }

  // Los "anterior*" se guardan una unica vez, ya que todos los
  // sistemas de este frame usaron las variables pulsacionX de arriba.
  anteriorA = mC.t_A;
  anteriorB = mC.t_B;
  anteriorC = mC.t_C;
  anteriorD = mC.t_D;
  anteriorArriba = mC.t_arriba;
  anteriorAbajo = mC.t_abajo;
  anteriorIzquierda = mC.t_izquierda;
  anteriorDerecha = mC.t_derecha;
  cAnterior = mC.t_C;

  //actualizarAnimacionPlataformas();

  dibujarEscena();
  dibujarPickups();
  dibujarFiltroNoche();
  dibujarBarraVida();
  dibujarHora();
  dibujarIndicadores();
  dibujarRuedaArmas();
  dibujarInventario();
  dibujarBaul();
  lcd.drawText(LCD_WIDTH - 160, 40, "SUPERVIVENCIA", "SANSBOLD", SMART_ACCENT, INT_TO_FIXED(1));
}

}

// Compatibilidad: el programa principal sigue pudiendo arrancar
// directamente el juego de Supervivencia si se ejecuta fuera del Smart Watch.
void juegoSetup() {
  SUPERVIVENCIA_GAME::juegoSetup();
}
void juegoLoop() {
  SUPERVIVENCIA_GAME::juegoLoop();
}