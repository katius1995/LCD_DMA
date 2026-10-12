#ifndef ST7789_DMA_H
#define ST7789_DMA_H

#include <stdint.h>
#include <Arduino.h>
#include "math_fixed.h"

#define LCD_WIDTH 240
#define LCD_HEIGHT 300
#define LINES_PER_BLOCK LCD_HEIGHT  // Un buffer contiene la pantalla completa
#define BLOCK_PIXELS (LCD_WIDTH * LINES_PER_BLOCK)

#define MASK_RB 63519        // 0b1111100000011111
#define MASK_G 2016          // 0b0000011111100000
#define MASK_MUL_RB 4065216  // 0b1111100000011111000000
#define MASK_MUL_G 129024    // 0b0000011111100000000000
#define MAX_ALPHA 64         // 6bits+1 with rounding

// El periférico SPI/DMA no admite una transferencia de 144000 bytes
// como una sola transacción. Se envía el framebuffer completo en
// transacciones DMA consecutivas. 4092 bytes es un tamaño seguro.
#define DMA_CHUNK_BYTES 4092
#define DMA_CHUNK_PIXELS (DMA_CHUNK_BYTES / sizeof(uint16_t))

// ---- MOTOR DE MATEMÁTICAS DE PUNTO FIJO (16.16) ----
#define FIXED_SHIFT_R 11
#define FIXED_SHIFT_G 5
#define FIXED_SHIFT 16
#define FIXED_SCALE (1 << FIXED_SHIFT)       // 65536
#define FIXED_HALF (1 << (FIXED_SHIFT - 1))  // 32768 (Para redondeo)

#define BODY 0
#define AREA 1
#define LUZPUNTUAL 2
#define LUZDIRECCIONAL 3
#define LUZSPOT 4
#define PARTICULA 5
#define BODY2D 6
#define FUEGO 7

// Tamaño del mapa de sombras (ajústalo según la memoria RAM libre de tu microcontrolador)
#define SHADOW_MAP_W 64
#define SHADOW_MAP_H 64
// ============================================================================
// LOD AUTOMÁTICO PARA OBJETOS 3D
// ============================================================================
// El LOD se decide combinando:
//   1) distancia al centro del objeto
//   2) tamaño aproximado del objeto en píxeles en pantalla
//
// LOD 0 = 100% de caras
// LOD 1 = ~50%
// LOD 2 = ~33%
// LOD 3 = ~17%
//
// Los valores están en las mismas unidades 16.16 usadas por los modelos.
#define GAME_LOD_ENABLED             1
#define GAME_LOD_MIN_FACES           12

#define GAME_LOD_DISTANCE_1          12000
#define GAME_LOD_DISTANCE_2          15000
#define GAME_LOD_DISTANCE_3          21000

#define GAME_LOD_PIXELS_1            55
#define GAME_LOD_PIXELS_2            28
#define GAME_LOD_PIXELS_3            10

// Para objetos pequeños no conviene gastar tiempo calculando una reducción
// demasiado agresiva: se mantiene al menos este porcentaje de caras.
#define GAME_LOD1_FACE_DIV           2   // 1 de cada 2
#define GAME_LOD2_FACE_DIV           3   // 1 de cada 3
#define GAME_LOD3_FACE_DIV           6   // 1 de cada 6





extern const float *verticesF;
extern const int32_t *vertices;
extern const int *faces;

enum GeometryGame : uint8_t {
  GEOMETRY_BATALLA_NAVAL = 0,
  GEOMETRY_SUPERVIVENCIA = 1,
  GEOMETRY_TEXTURED_OBJ = 2
};

void selectGameGeometry(GeometryGame game);
void prepareGameFaceNormals(const int32_t *verts, const int *faceData, uint32_t faceCount);
// Malla texturizada opcional. La ruta antigua del motor no utiliza estas estructuras.
typedef struct {
  uint16_t v0, v1, v2;
  uint16_t uv0, uv1, uv2;
} FaceUV3D;

typedef struct {
  const int32_t *vertices;  // xyz, 16.16
  const int32_t *uv;        // uv, 16.16
  const void *faces;        // array compatible con FaceUV3D
  uint16_t num_vertices;
  uint16_t num_uv;
  uint16_t num_faces;
} TexturedMesh3D;

extern const TexturedMesh3D *activeTexturedMesh;


// Macros de conversión rápida
#define INT_TO_FIXED(x) ((int32_t)(x) << FIXED_SHIFT)
#define FIXED_TO_INT(x) ((int32_t)((x) + FIXED_HALF) >> FIXED_SHIFT)

// Definición de pines para Joystick y Botones (Ajusta los GPIOs a tu circuito real)
#define JOY_X_PIN 18    // ADC1 Canal para eje X
#define JOY_Y_PIN 17    // ADC1 Canal para eje Y
#define JOY_SEL_PIN 6   // GPIO para botón del Joystick (Pull-up)
#define BTN_BACK_PIN 4  // GPIO para botón de regreso (Pull-up)

// Estados de la interfaz del Smartwatch
enum WatchState {
  STATE_CLOCK_FACE,    // Esfera principal del reloj
  STATE_CALENDAR,      // Calendario
  STATE_MENU,          // Menú de opciones de Apps
  STATE_GAME_NAVAL,    // Ejecutando Batalla Naval
  STATE_GAME_SURVIVE,  // Ejecutando Supervivencia
  STATE_GAME_3D,       // Pruebas 3D con árbol texturizado UV
  STATE_WIFI,          // Menú de redes Wi-Fi
  STATE_SETTINGS,      // Ajustes Generales
  STATE_GAME_NAVAL_START,
  STATE_GAME_SURVIVE_START
};

typedef struct {
  int x = 0, y = 0;
} Vector2Dspr;

typedef struct {
  Vector2Dspr position;
  float distancia;
  bool encendido = false;
} LuzSpr;

typedef struct {
  uint16_t x = 0;
  uint16_t y = 0;
  uint16_t width;
  uint16_t height;
  uint16_t radius = 0;
  bool visible = true;
  bool emiteLuz = false;
  float distanciaLuz = 0.0f;
  float scale = 1.0f;
  float scaleStep = 0.1f;
  float scaleMin = 0.8f, scaleMax = 1.2f;
  uint8_t collision = 0;
  int phase = 0;

  int offset = 0;

  float angle = 0;  // ángulo en grados
  float angleStep = 10;

  // Deslizamiento
  int slideX = 0, slideY = 0;  // desplazamiento temporal
  int slideStep = 0;
  int slideTarget = 0;
} LCDSprite;

// Vector 3D en Punto Fijo (16.16)
struct Vector3D {
  int32_t x;
  int32_t y;
  int32_t z;

  Vector3D()
    : x(0), y(0), z(0) {}
  Vector3D(int32_t x, int32_t y, int32_t z)
    : x(x), y(y), z(z) {}

  void add(const Vector3D &other) {
    x += other.x;
    y += other.y;
    z += other.z;
  }

  Vector3D add(const Vector3D &other) const {
    return Vector3D(x + other.x, y + other.y, z + other.z);
  }

  void add(int32_t x, int32_t y, int32_t z) {
    this->x += x;
    this->y += y;
    this->z += z;
  }

  Vector3D subtract(const Vector3D &other) const {
    return Vector3D(x - other.x, y - other.y, z - other.z);
  }

  void subtract(const Vector3D &other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
  }
};

// Cara o Polígono indexado (Conecta 3 vértices para formar un triángulo)
typedef struct {
  uint16_t v0;  // Índice del primer vértice
  uint16_t v1;  // Índice del segundo vértice
  uint16_t v2;  // Índice del tercer vértice
} Face3D;

// Modelo 3D Completo (Malla de polígonos / Mesh)
typedef struct {
  const Vector3D *vertices;  // Arreglo de vértices en espacio local
  uint16_t num_vertices;
  const Face3D *faces;  // Arreglo de caras/triángulos
  uint16_t num_faces;
} Model3D;

// Matriz de Transformación 4x4 en Punto Fijo para Rotación, Traslación y Escala
typedef struct {
  int32_t m[16];  // Cada elemento es un int32_t escalado por 65536
} Matrix4x4;

// Estructuras del Motor Gráfico / Físico
typedef struct {
  int64_t x;  // Coordenada X en punto fijo
  int64_t y;  // Coordenada Y en punto fijo
} Vector2D;

enum TipoLuz {
  TIPO_FUEGO = FUEGO,
  TIPO_DIRECCIONAL = LUZDIRECCIONAL,
  TIPO_SPOT = LUZSPOT,
  TIPO_BOMBILLO = LUZPUNTUAL
};

enum TipoColision { SOLID_COL,
                    AREA_COL };

struct AABB {
  Vector3D min;  // esquina inferior
  Vector3D max;  // esquina superior
};

struct Mtl {
  int f_init;
  int f_end;
  uint16_t color_mtl = 0xFFFF;
};

struct Luz {
  bool encendido = false;
  int32_t intensidad = FIXED_SCALE;
  int32_t alcance = INT_TO_FIXED(30000);
  int32_t angulo = INT_TO_FIXED(35);
  int32_t suavidad = INT_TO_FIXED(2);
  Vector3D direccion = { 0, -FIXED_SCALE, 0 };
  uint16_t color = 0xFFFF;
};

struct Obj {
  uint8_t collision_mask = 0;
  const char *id;
  TipoColision collision_type = SOLID_COL;
  uint8_t type = 0;
  bool visible = true;
  AABB collision;
  Vector3D position;
  Vector3D traslation;
  Vector3D rotation;
  Vector3D rotationGlobal;
  Vector3D scale = { FIXED_SCALE, FIXED_SCALE, FIXED_SCALE };
  void (*onloop)(Obj *self);
  Mtl mtl;
  const char *grupo_id;
  uint8_t type_geometry = 0;
  int w = 0, h = 0, r = 0, alpha = 255;
  bool is_ground = false;
  bool is_cull = true;
  Luz luz;

  // LOD automático:
  // true  = seleccionar automáticamente el nivel según distancia/tamaño.
  // false = mantener LOD 0 (todas las caras).
  bool auto_lod = true;

  // 255 = automático. 0..3 = fuerza manualmente ese nivel.
  uint8_t lod_override = 255;

  // Nivel calculado durante el último draw de este objeto.
  // Es informativo y no es necesario modificarlo desde el juego.
  uint8_t lod_current = 0;

  // Cache para culling por esfera envolvente.
  // Se calcula una sola vez a partir de las caras del objeto.
  int32_t bounds_radius = 0;
  Vector3D bounds_position = { 0, 0, 0 };
  bool bounds_valid = false;

  // 🛠️ NUEVO: Búfer dinámico de índices locales para ordenación interna de caras
  uint16_t* sorted_face_indices = nullptr;
  bool faces_allocated = false;

  // Inicializa el búfer dinámico según el número de caras que tenga este objeto
  void initFaceSorting() {
    if (!faces_allocated && mtl.f_init >= 0 && mtl.f_end > mtl.f_init) {
      int totalFaces = mtl.f_end - mtl.f_init;
      // Reservamos memoria en la PSRAM para no agotar la RAM interna
      sorted_face_indices = (uint16_t*)ps_malloc(totalFaces * sizeof(uint16_t));
      
      // Llenamos inicialmente con el orden secuencial por defecto
      for (int i = 0; i < totalFaces; i++) {
        sorted_face_indices[i] = mtl.f_init + i;
      }
      faces_allocated = true;
    }
  }
};

// Sistema de iluminación de escena. Se registra una vez por frame desde el juego.
#define MAX_LUCES_JUEGO 8
void beginGameLighting(Obj *objects, uint16_t count);
void clearGameLighting();

// Estructuras nativas para fuentes GFX (Adafruit GFX estricto)
typedef struct {
  uint16_t bitmapOffset;
  uint8_t width;
  uint8_t height;
  uint8_t xAdvance;
  int8_t xOffset;
  int8_t yOffset;
} GFXglyph;

typedef struct {
  const uint8_t *bitmap;
  const GFXglyph *glyphs;
  uint8_t first;
  uint8_t last;
  uint8_t yAdvance;
} Font;

struct MandoControl {
  uint16_t joy_x1 = 0;
  uint16_t joy_y1 = 0;
  uint16_t joy_x2 = 0;
  uint16_t joy_y2 = 0;
  bool btn_joy1 = false;
  bool btn_joy2 = false;
  bool t_arriba = false;
  bool t_abajo = false;
  bool t_derecha = false;
  bool t_izquierda = false;
  bool t_A = false;
  bool t_B = false;
  bool t_C = false;
  bool t_D = false;
  bool t_aceptar = false;
  bool t_opciones = false;
};

class ST7789_DMA {
private:
  uint16_t *buffer_A;
  uint16_t *buffer_B;
  uint16_t *draw_buffer;
  uint16_t *disp_buffer;

  // Z-buffer opcional. Se reserva EXCLUSIVAMENTE en PSRAM.
  // Si la reserva falla, permanece en NULL y el rasterizado funciona
  // exactamente como antes, sin pruebas de profundidad.
  int32_t *z_buffer;

  int current_block_y;
  uint8_t frame_pass;  // 0 = buffer A, 1 = buffer B, 2 = frame terminado

  // Estado independiente de cada buffer DMA.
  bool dma_pending_A;
  bool dma_pending_B;

  // Tiempo acumulado esperando DMA/buffer durante el frame (microsegundos).
  uint32_t wait_dma_us;
  uint32_t wait_buffer_us;

  void waitForBuffer(uint16_t *buffer);
  void drainDMA(void);

public:
  ST7789_DMA();
  bool begin(void);

  // ---- AÑADE ESTA LÍNEA PARA EXPONER LA COORDENADA Y ----
  int getBlockY(void) {
    return current_block_y;
  }
  uint16_t *getDrawBuffer(void) {
    return draw_buffer;
  }
  uint32_t getWaitDMAUs(void) {
    return wait_dma_us;
  }
  uint32_t getWaitBufferUs(void) {
    return wait_buffer_us;
  }

  void startFrame(void);
  bool nextBlock(void);
  void endFrame(void);

  void clear(uint16_t color);
  void fillRect(int x, int y, int w, int h, uint16_t color, uint8_t alpha = 255);
  void drawRect(int x, int y, int w, int h, uint16_t color, uint8_t alpha = 255);
  void drawLine(int x0, int y0, int x1, int y1, uint16_t color, uint8_t alpha = 255);
  // ... (Dentro de la clase ST7789_DMA en la sección public:)
  void drawCircle(int x_center, int y_center, int r, uint16_t color, uint8_t alpha = 255);
  void drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint16_t color, uint8_t alpha = 255);

  // ---- NUEVAS FUNCIONES PARA RELLENOS ----
  void fillCircle(int x_center, int y_center, int r, uint16_t color, uint8_t alpha = 255);
  void fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint16_t color, uint8_t alpha = 255);
  void fillTriangleZ(int x0, int y0, int32_t z0,
                     int x1, int y1, int32_t z1,
                     int x2, int y2, int32_t z2,
                     uint16_t color, uint8_t alpha = 255);
  // Rellena un span horizontal dentro del bloque DMA actual.
  void drawSpan(int y, int x0, int x1, uint16_t color, uint8_t alpha = 255);
  void getAABBVertices(const AABB &box, Vector3D *corners);
  void drawAABB(const AABB &box, int32_t scale, int32_t centerX, int32_t centerY, bool perspectiva, uint16_t color);


  // ---- NUEVAS FUNCIONES REESCRITAS CON PUNTO FIJO ----
  void drawCustomChar(int16_t x, int16_t y, char c, const Font *font, uint16_t color, int32_t scale_fixed, uint8_t alpha);
  void drawText(int16_t x, int16_t y, const char *str, String _font = "SANS", uint16_t color = 0xFFFF, int32_t scale_fixed = INT_TO_FIXED(1), uint8_t alpha = 255);
  void drawSpanShadow(int y, int x_start, int x_end,
                      int64_t lx_start, int64_t ly_start, int64_t lz_start,
                      int64_t lx_end, int64_t ly_end, int64_t lz_end,
                      uint16_t color, uint8_t alpha);
  void configInitGame(const Vector3D &camera);
  void beginGameLighting(Obj *objects, uint16_t count);
  void updateMatrizCamera(const Vector3D &camera);
  void getCentroid(Obj &obj, uint8_t opcion);
  void getCentroidV2(Obj &obj, uint8_t opcion);
  void setLightDir(Vector3D light);
  void setEnvironmentLighting(uint16_t ambientColor, float ambientIntensity, float sunIntensity);
  void setBufferingZ(bool buff);
  void setShadows(bool shadow);
  void drawShadowv2(Obj &miObjeto, Vector3D pivotCamera, int32_t fov_precalculado);
  void sortSceneFaces(Obj* objects, uint16_t objectCount, Vector3D pivotCamera);
  void drawSceneSortedSingleFace(Obj* objects, Vector3D pivotCamera, int32_t scale);
  void sortLocalObjectFaces(Obj &obj, Vector3D pivotCamera);

  void fillTriangleUV(int x0, int y0, int x1, int y1, int x2, int y2,
                      int32_t u0, int32_t v0, int32_t u1, int32_t v1,
                      int32_t u2, int32_t v2, int32_t intensity);
  void drawSceneSpriteGame(LCDSprite &spr, int x0, int y0, int offsetx, int offsety, float intensidad = 1.0f, float escala = 1.0f);
  void drawSceneSpriteGameV2(LCDSprite &spr, int x0, int y0, int offsetx, int offsety, float intensidad = 1.0f, float escala = 1.0f);
  void drawSceneGameObject(Obj &obj, int32_t anglex, int32_t angley, int32_t anglez, Vector3D angleCamera, Vector3D pivotCamera, bool select, int32_t scale, int32_t origenxx = 0, int32_t origenyy = 0, int32_t origenzz = 0, int32_t clipZ = 0);
  void drawSceneGameObjectV2(Obj &obj, int32_t anglex, int32_t angley, int32_t anglez, Vector3D angleCamera, Vector3D pivotCamera, bool select, int32_t scale, int32_t origenxx = 0, int32_t origenyy = 0, int32_t origenzz = 0, int32_t clipZ = 0);
  // V3 usa el mismo pipeline LOD optimizado de V2 en este proyecto.
  void drawSceneGameObjectV3(Obj &obj, int32_t anglex, int32_t angley, int32_t anglez, Vector3D angleCamera, Vector3D pivotCamera, bool select, int32_t scale, int32_t origenxx = 0, int32_t origenyy = 0, int32_t origenzz = 0, int32_t clipZ = 0);

  int32_t fixed_div(int32_t a, int32_t b) {
    if (b == 0) return 0;
    return (int32_t)((((int64_t)a << FIXED_SHIFT) + (b / 2)) / b);
  }

  // Versión ultra-optimizada: El FOV se calcula una sola vez por frame, eliminando miles de divisiones
  Vector2D project(
    Vector3D p,
    int32_t scale,
    int32_t cx,
    int32_t cy,
    int32_t fov_precalculado, // Nueva variable en lugar de calcularlo adentro
    bool perspectiva = false) {
    Vector2D result;

    if (perspectiva) {
      int32_t z = p.z;
      if (z == 0) z = 1;

      // Usamos el signo negativo directamente para evitar la doble negación
      int32_t inv_z = -z; 

      // Cambiamos divisiones de 64 bits por multiplicaciones intermedias eficientes
      result.x = cx + (int32_t)(((int64_t)p.x * fov_precalculado) / inv_z);
      result.y = cy - (int32_t)(((int64_t)p.y * fov_precalculado) / inv_z);

    } else {
      result.x = cx + (int32_t)(((int64_t)p.x * scale) >> FIXED_SHIFT);
      result.y = cy - (int32_t)(((int64_t)p.y * scale) >> FIXED_SHIFT);
    }

    return result;
  }
private:
  // Buffer de profundidad (inicializar con un valor muy lejano, ej: 999999 o INT32_MAX)
  int16_t shadowMap[SHADOW_MAP_W * SHADOW_MAP_H];
};

inline Matrix4x4 matIdentity() {
  Matrix4x4 m = { 0 };

  m.m[0] = FIXED_SCALE;
  m.m[5] = FIXED_SCALE;
  m.m[10] = FIXED_SCALE;
  m.m[15] = FIXED_SCALE;

  return m;
}

inline Matrix4x4 matRotX(int32_t c, int32_t s) {
  Matrix4x4 m = matIdentity();

  m.m[5] = c;
  m.m[6] = s;
  m.m[9] = -s;
  m.m[10] = c;

  return m;
}

inline Matrix4x4 matRotY(int32_t c, int32_t s) {
  Matrix4x4 m = matIdentity();

  m.m[0] = c;
  m.m[2] = -s;
  m.m[8] = s;
  m.m[10] = c;

  return m;
}

inline Matrix4x4 matRotZ(int32_t c, int32_t s) {
  Matrix4x4 m = matIdentity();

  m.m[0] = c;
  m.m[1] = s;
  m.m[4] = -s;
  m.m[5] = c;

  return m;
}

// Funciones utilitarias inline de multiplicación/división en punto fijo
inline int32_t fixed_mul(int32_t a, int32_t b) {
  return (int32_t)(((int64_t)a * b + FIXED_HALF) >> FIXED_SHIFT);
}

/*inline int32_t fixed_div(int32_t a, int32_t b) {
    if (b == 0) return 0;
    return (int32_t)((((int64_t)a << FIXED_SHIFT) + (b / 2)) / b);
}*/

// Escala un color RGB565 por una intensidad en punto fijo (0 a FIXED_SCALE)
inline uint16_t scale_color_fixed(uint16_t color, int32_t intensity_fixed) {
  if (intensity_fixed <= 0) return 0x0000;           // Oscuridad total
  if (intensity_fixed >= FIXED_SCALE) return color;  // Brillo máximo

  // Extraer canales nativos RGB565
  uint32_t r = (color >> FIXED_SHIFT_R) & 0x1F;
  uint32_t g = (color >> FIXED_SHIFT_G) & 0x3F;
  uint32_t b = color & 0x1F;

  // Multiplicar los canales por la intensidad de punto fijo y desplazar 16 bits (FIXED_SHIFT)
  r = (r * intensity_fixed) >> FIXED_SHIFT;
  g = (g * intensity_fixed) >> FIXED_SHIFT;
  b = (b * intensity_fixed) >> FIXED_SHIFT;

  // Re-empaquetar el color escalado
  return (uint16_t)((r << FIXED_SHIFT_R) | (g << FIXED_SHIFT_G) | b);
}

// Raíz cuadrada por método de aproximación rápida para enteros de 32/64 bits (Método de Newton)
inline uint64_t isqrt64(uint64_t n) {
  if (n == 0) return 0;
  uint64_t x0 = n >> 1;
  if (x0 == 0) x0 = 1;
  uint64_t x1 = (x0 + n / x0) >> 1;
  while (x1 < x0) {
    x0 = x1;
    x1 = (x0 + n / x0) >> 1;
  }
  return (uint64_t)x0;
}

inline Matrix4x4 matMul(const Matrix4x4 &a, const Matrix4x4 &b) {

  Matrix4x4 r;

  for (uint8_t i = 0; i < 4; i++) {
    for (uint8_t j = 0; j < 4; j++) {

      r.m[i + j * 4] =
        (int32_t)((
                    (int64_t)a.m[0 + j * 4] * b.m[i + 0 * 4] + (int64_t)a.m[1 + j * 4] * b.m[i + 1 * 4] + (int64_t)a.m[2 + j * 4] * b.m[i + 2 * 4] + (int64_t)a.m[3 + j * 4] * b.m[i + 3 * 4])
                  >> FIXED_SHIFT);
    }
  }
  return r;
}

inline Matrix4x4 buildMatrixCamera(const Vector3D &camera, const Matrix4x4 &MView) {

  int32_t ax = (camera.x >> FIXED_SHIFT);
  int32_t ay = (camera.y >> FIXED_SHIFT);
  int32_t az = (camera.z >> FIXED_SHIFT);

  int32_t cxl = fixed_cos(ax);
  int32_t sxl = fixed_sin(ax);

  int32_t cyl = fixed_cos(ay);
  int32_t syl = fixed_sin(ay);

  int32_t czl = fixed_cos(az);
  int32_t szl = fixed_sin(az);

  Matrix4x4 MView2 = matMul(matRotZ(czl, szl), MView);
  MView2 = matMul(matRotY(cyl, syl), MView2);
  MView2 = matMul(matRotX(cxl, sxl), MView2);

  return MView2;
}

// ---------------------------------------------------------------------------
// Rotaciones de objetos
// ---------------------------------------------------------------------------
// Obj::rotation se maneja en GRADOS enteros. Algunas versiones antiguas del
// proyecto llegaron a guardar INT_TO_FIXED(grados), por lo que aceptamos
// también ese formato para mantener compatibilidad con escenas existentes.
inline int32_t objectRotationDegrees(int32_t value) {
  int32_t a = value;
  int32_t av = (a < 0) ? -a : a;

  // Valores claramente mayores que un ángulo en grados se interpretan como
  // punto fijo 16.16 (por ejemplo INT_TO_FIXED(90)).
  if (av > 4096) {
    a /= FIXED_SCALE;
  }

  a %= 360;
  if (a < 0) a += 360;
  return a;
}

inline bool objectHasRotation(const Obj &obj) {
  return obj.rotation.x != 0 || obj.rotation.y != 0 || obj.rotation.z != 0;
}

inline bool objectHasGlobalRotation(const Obj &obj) {
  return obj.rotationGlobal.x != 0 || obj.rotationGlobal.y != 0 || obj.rotationGlobal.z != 0;
}

inline Matrix4x4 buildMatrixLocal(Obj &obj) {

  int32_t ax = objectRotationDegrees(obj.rotation.x);
  int32_t ay = objectRotationDegrees(obj.rotation.y);
  int32_t az = objectRotationDegrees(obj.rotation.z);

  if (ax < 0) ax += 360;
  if (ay < 0) ay += 360;
  if (az < 0) az += 360;

  int32_t cxl = fixed_cos(ax);
  int32_t sxl = fixed_sin(ax);

  int32_t cyl = fixed_cos(ay);
  int32_t syl = fixed_sin(ay);

  int32_t czl = fixed_cos(az);
  int32_t szl = fixed_sin(az);

  Matrix4x4 MxLocal = matRotX(cxl, sxl);
  Matrix4x4 MyLocal = matRotY(cyl, syl);
  Matrix4x4 MzLocal = matRotZ(czl, szl);

  // Convención única para objetos: X -> Y -> Z.
  // Con vectores columna esto se representa como Rz * Ry * Rx.
  Matrix4x4 MRot = matMul(MzLocal, matMul(MyLocal, MxLocal));

  // Escala local por instancia.
  Matrix4x4 MScale = matIdentity();

  MScale.m[0] = obj.scale.x;
  MScale.m[5] = obj.scale.y;
  MScale.m[10] = obj.scale.z;

  return matMul(MRot, MScale);
}

inline Matrix4x4 buildMatrixLocalInit() {

  int32_t ax = 0 % 360;
  int32_t ay = 0 % 360;
  int32_t az = 0 % 360;

  if (ax < 0) ax += 360;
  if (ay < 0) ay += 360;
  if (az < 0) az += 360;

  int32_t cxl = fixed_cos(ax);
  int32_t sxl = fixed_sin(ax);

  int32_t cyl = fixed_cos(ay);
  int32_t syl = fixed_sin(ay);

  int32_t czl = fixed_cos(az);
  int32_t szl = fixed_sin(az);

  Matrix4x4 MxLocal = matRotX(cxl, sxl);
  Matrix4x4 MyLocal = matRotY(cyl, syl);
  Matrix4x4 MzLocal = matRotZ(czl, szl);

  Matrix4x4 MRot = matMul(MxLocal, matMul(MzLocal, MyLocal));

  // Escala local por instancia.
  Matrix4x4 MScale = matIdentity();

  MScale.m[0] = FIXED_SCALE;
  MScale.m[5] = FIXED_SCALE;
  MScale.m[10] = FIXED_SCALE;

  return matMul(MRot, MScale);
}

inline Matrix4x4 buildMatrixGlobal(Obj &obj, int32_t anglex, int32_t angley, int32_t anglez) {

  int32_t ax = (anglex + objectRotationDegrees(obj.rotationGlobal.x)) % 360;
  int32_t ay = (angley + objectRotationDegrees(obj.rotationGlobal.y)) % 360;
  int32_t az = (anglez + objectRotationDegrees(obj.rotationGlobal.z)) % 360;

  if (ax < 0) ax += 360;
  if (ay < 0) ay += 360;
  if (az < 0) az += 360;

  int32_t cx = fixed_cos(ax);
  int32_t sx = fixed_sin(ax);

  int32_t cy = fixed_cos(ay);
  int32_t sy = fixed_sin(ay);

  int32_t cz = fixed_cos(az);
  int32_t sz = fixed_sin(az);

  Matrix4x4 MxLocal = matRotX(cx, sx);
  Matrix4x4 MyLocal = matRotY(cy, sy);
  Matrix4x4 MzLocal = matRotZ(cz, sz);

  // Misma convención que la rotación local.
  return matMul(MzLocal, matMul(MyLocal, MxLocal));
}

inline Matrix4x4 buildMatrixGlobalInit() {

  int32_t ax = 0 % 360;
  int32_t ay = 0 % 360;
  int32_t az = 0 % 360;

  if (ax < 0) ax += 360;
  if (ay < 0) ay += 360;
  if (az < 0) az += 360;

  int32_t cx = fixed_cos(ax);
  int32_t sx = fixed_sin(ax);

  int32_t cy = fixed_cos(ay);
  int32_t sy = fixed_sin(ay);

  int32_t cz = fixed_cos(az);
  int32_t sz = fixed_sin(az);

  Matrix4x4 MxLocal = matRotX(cx, sx);
  Matrix4x4 MyLocal = matRotY(cy, sy);
  Matrix4x4 MzLocal = matRotZ(cz, sz);

  return matMul(MxLocal, matMul(MzLocal, MyLocal));
}

inline Vector3D mul(const Matrix4x4 &m, const Vector3D &v) {

  Vector3D r;

  r.x = ((int64_t)m.m[0] * v.x + (int64_t)m.m[4] * v.y + (int64_t)m.m[8] * v.z) >> FIXED_SHIFT;

  r.y = ((int64_t)m.m[1] * v.x + (int64_t)m.m[5] * v.y + (int64_t)m.m[9] * v.z) >> FIXED_SHIFT;

  r.z = ((int64_t)m.m[2] * v.x + (int64_t)m.m[6] * v.y + (int64_t)m.m[10] * v.z) >> FIXED_SHIFT;

  r.x += m.m[12];
  r.y += m.m[13];
  r.z += m.m[14];

  return r;
}

inline Vector3D cross(const Vector3D &a, const Vector3D &b) {

  Vector3D r;

  r.x = ((int64_t)a.y * b.z - (int64_t)a.z * b.y) >> FIXED_SHIFT;

  r.y = ((int64_t)a.z * b.x - (int64_t)a.x * b.z) >> FIXED_SHIFT;

  r.z = ((int64_t)a.x * b.y - (int64_t)a.y * b.x) >> FIXED_SHIFT;

  return r;
}

inline Vector3D normalize(const Vector3D &v) {

  int64_t x = v.x;
  int64_t y = v.y;
  int64_t z = v.z;

  // Longitud² en Q32.32
  uint64_t len2 =
    (uint64_t)(x * x) + (uint64_t)(y * y) + (uint64_t)(z * z);

  if (len2 == 0) {
    return { 0, 0, 0 };
  }

  // sqrt(Q32.32) -> Q16.16
  uint64_t len = isqrt64(len2);

  if (len == 0) {
    return { 0, 0, 0 };
  }

  Vector3D r;

  // v / len
  // Q16.16 / Q16.16 -> Q0.16
  // Se vuelve a escalar a Q16.16.
  r.x = (int32_t)(((int64_t)v.x << FIXED_SHIFT) / len);
  r.y = (int32_t)(((int64_t)v.y << FIXED_SHIFT) / len);
  r.z = (int32_t)(((int64_t)v.z << FIXED_SHIFT) / len);

  return r;
}

inline Vector3D sub(Vector3D a, Vector3D b) {
  return { a.x - b.x, a.y - b.y, a.z - b.z };
}

inline int64_t dot(const Vector3D &a, const Vector3D &b) {

  return (int64_t)((
                     (int64_t)a.x * b.x + (int64_t)a.y * b.y + (int64_t)a.z * b.z)
                   >> FIXED_SHIFT);
}

inline Vector3D direccionHaciaJugador(const Vector3D &desde, const Vector3D &hacia) {
  Vector3D dir = {
    hacia.x - desde.x,
    hacia.y - desde.y,
    hacia.z - desde.z
  };
  return dir;
}

inline float length(const Vector3D &v) {
  float x = (v.x / 1000.0f) * (v.x / 1000.0f);
  float y = (v.y / 1000.0f) * (v.y / 1000.0f);
  float z = (v.z / 1000.0f) * (v.z / 1000.0f);
  //return sqrt(x + y + z);
  return x + y + z;
}

inline int32_t floatToFixed(float v) {
  return (int32_t)(v * FIXED_SCALE);
}

inline uint16_t alphablend(uint16_t fg, uint16_t bg, uint8_t alpha) {

  // alpha for foreground multiplication
  // convert from 8bit to (6bit+1) with rounding
  // will be in [0..64] inclusive
  alpha = (alpha + 2) >> 2;
  // "beta" for background multiplication; (6bit+1);
  // will be in [0..64] inclusive
  uint8_t beta = MAX_ALPHA - alpha;
  // so (0..64)*alpha + (0..64)*beta always in 0..64

  return (uint16_t)((((alpha * (uint32_t)(fg & MASK_RB) + beta * (uint32_t)(bg & MASK_RB))
                      & MASK_MUL_RB)
                     | ((alpha * (fg & MASK_G) + beta * (bg & MASK_G)) & MASK_MUL_G))
                    >> 6);
}


// Exponer tus fuentes globalmente al enlazador
extern const Font FreeSans9pt7b;
extern const Font FreeSansBold9pt7b;
extern const char Retro8x8[];
extern const char Retro8x16[];
extern const Font ArianaVioleta_dz2K18pt7b;
extern const Font SolaronaDemo_6R83Y18pt7b;
extern const Font BeatWordDemo_nRL2018pt7b;


extern const Model3D mesh_cube;  // Tu cubo de pruebas anterior

extern ST7789_DMA lcd;

#endif  // ST7789_DMA_H
