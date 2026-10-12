#include "st7789_dma.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "TEXTURE.h"
#include "SPRITES.h"

#include <xtensa/config/core-isa.h>

#define PIN_NUM_MISO -1
#define PIN_NUM_MOSI 11
#define PIN_NUM_CLK 12
#define PIN_NUM_CS 10
#define PIN_NUM_DC 9
#define PIN_NUM_RST 8

bool zbuffering = false;
bool showShadow = false;


static spi_device_handle_t spi_handle;
static spi_transaction_t spi_trans_A;
static spi_transaction_t spi_trans_B;

const int32_t centerX = INT_TO_FIXED(LCD_WIDTH / 2);
const int32_t centerY = INT_TO_FIXED(LCD_HEIGHT / 2);

const int32_t lcdW = INT_TO_FIXED(LCD_WIDTH);
const int32_t lcdH = INT_TO_FIXED(LCD_HEIGHT);

Matrix4x4 MLocal, MGlobal, MView;

// La cámara se recibe en las llamadas drawSceneGameObject*. Se conserva una
// copia para reconstruir MView solo cuando realmente cambió, evitando recalcular
// senos/cosenos una vez por objeto.
static Vector3D gLastCameraRotation = { 0, 0, 0 };
static bool gCameraRotationValid = false;

static inline void ensureCameraRotation(const Vector3D &camera) {
  if (!gCameraRotationValid ||
      camera.x != gLastCameraRotation.x ||
      camera.y != gLastCameraRotation.y ||
      camera.z != gLastCameraRotation.z) {
    MView = buildMatrixCamera(camera, matIdentity());
    gLastCameraRotation = camera;
    gCameraRotationValid = true;
  }
}

// Luz solar direccional, en coordenadas del mundo.
// El vector puede tener cualquier magnitud; se normaliza antes de usarlo.
Vector3D SUN_DIRECTION_WORLD = { 19661, 52429, 26214 };

// Iluminación ambiental y solar configurables por el clima.
// La luz ambiente aporta una base de iluminación y el sol aporta
// iluminación difusa dependiente de la normal.
static float gAmbientIntensity = 0.20f;
static uint16_t gAmbientColor = 0xFFFF;
static float gSunIntensity = 0.80f;

// Instancia global
ST7789_DMA lcd;

// ============================================================================
// SISTEMA DE LUCES DEL JUEGO
// ============================================================================
static Obj *gLightObjects = nullptr;
static uint16_t gLightObjectCount = 0;

// Lista independiente de las luces registradas. Antes el bucle de iluminación
// recorría gLightObjects[0..gLightObjectCount) suponiendo que las luces eran
// los primeros objetos del arreglo; si la luz estaba en otra posición
// (p. ej. obj[10] en BATALLA) nunca se evaluaba.
#define MAX_LUCES_REGISTRADAS 32
static Obj *gLightList[MAX_LUCES_REGISTRADAS];
static uint16_t gLightListCount = 0;

struct FaceDepth {
    uint16_t faceIndex; // Índice de la cara en el arreglo global 'faces'
    int32_t avgZ;       // Distancia Z promedio de sus 3 vértices
};

// Creamos un búfer lo suficientemente grande para los objetos de la escena.
// Lo ideal es alojarlo en PSRAM dado el tamaño.
static FaceDepth* gSortedFaces = nullptr;
static uint32_t gTotalFacesInScene = 0;


// ============================================================================
// FASE 3: normales de caras precalculadas
// ============================================================================
static Vector3D *gFaceNormals = nullptr;
static uint32_t gFaceNormalsCount = 0;
static const int32_t *gFaceNormalsVertices = nullptr;
static const int *gFaceNormalsFaces = nullptr;


static inline Vector3D transformNormal3x3(const Matrix4x4 &m, const Vector3D &n) {
  Vector3D r;
  r.x = ((int64_t)m.m[0] * n.x + (int64_t)m.m[4] * n.y + (int64_t)m.m[8]  * n.z) >> FIXED_SHIFT;
  r.y = ((int64_t)m.m[1] * n.x + (int64_t)m.m[5] * n.y + (int64_t)m.m[9]  * n.z) >> FIXED_SHIFT;
  r.z = ((int64_t)m.m[2] * n.x + (int64_t)m.m[6] * n.y + (int64_t)m.m[10] * n.z) >> FIXED_SHIFT;
  return r;
}

struct LocalFaceDepth {
    uint16_t globalIndex;
    int32_t z;
};

void clearGameLighting() {
  gLightObjects = nullptr;
  gLightObjectCount = 0;
  gLightListCount = 0;
}


// Calcula la posición inicial de las luces desde la geometría del objeto.
// Se ejecuta una vez: NO reemplaza position durante cada frame.
static inline void initializeLightObjectPosition(Obj &o) {
  if (o.type != LUZSPOT && o.type != LUZDIRECCIONAL && o.type != FUEGO)
    return;

  // Si position ya fue establecida por la carga/centroide del OBJ,
  // se conserva. La traslación representa solamente el desplazamiento
  // posterior realizado por el juego.
}

static inline float fixedToFloatL(int32_t v) {
  // Los parámetros de las luces locales (intensidad, alcance, ángulo,
  // suavidad y posiciones del proyecto) usan la escala histórica 1/1000.
  return (float)v / 1000.0f;
}

static inline float clamp01(float v) {
  if (v < 0.0f) return 0.0f;
  if (v > 1.0f) return 1.0f;
  return v;
}

static inline void normalizeFloatL(float &x, float &y, float &z) {
  float d2 = x * x + y * y + z * z;
  if (d2 <= 0.00000001f) {
    x = y = z = 0.0f;
    return;
  }
  float inv = 1.0f / sqrtf(d2);
  x *= inv;
  y *= inv;
  z *= inv;
}

static inline void color565ToFloat(uint16_t c, float &r, float &g, float &b) {
  r = (float)((c >> 11) & 0x1F) / 31.0f;
  g = (float)((c >> 5)  & 0x3F) / 63.0f;
  b = (float)(c & 0x1F) / 31.0f;
}

static inline uint16_t floatToColor565(float r, float g, float b) {
  r = clamp01(r);
  g = clamp01(g);
  b = clamp01(b);
  uint8_t rr = (uint8_t)(r * 31.0f + 0.5f);
  uint8_t gg = (uint8_t)(g * 63.0f + 0.5f);
  uint8_t bb = (uint8_t)(b * 31.0f + 0.5f);
  return ((uint16_t)rr << 11) | ((uint16_t)gg << 5) | bb;
}

// Interpolación de color de una fuente de luz. Se suma aditivamente a la
// iluminación existente y se limita al rango RGB565 al final.
static inline void addLightColor(float &r, float &g, float &b,
                                 uint16_t lightColor, float factor) {
  if (factor <= 0.0f) return;
  float lr, lg, lb;
  color565ToFloat(lightColor, lr, lg, lb);
  r += lr * factor;
  g += lg * factor;
  b += lb * factor;
}

// Intersección rayo-AABB. Se usa para una sombra de bajo coste.
static bool rayAABBL(float ox, float oy, float oz, float dx, float dy, float dz,
                     float minx, float miny, float minz, float maxx, float maxy, float maxz,
                     float maxT) {
  float tmin = 0.0f, tmax = maxT;
  const float o[3] = { ox, oy, oz }, d[3] = { dx, dy, dz }, mn[3] = { minx, miny, minz }, mx[3] = { maxx, maxy, maxz };
  for (int a = 0; a < 3; a++) {
    if (fabsf(d[a]) < 0.000001f) {
      if (o[a] < mn[a] || o[a] > mx[a]) return false;
    } else {
      float inv = 1.0f / d[a];
      float t1 = (mn[a] - o[a]) * inv, t2 = (mx[a] - o[a]) * inv;
      if (t1 > t2) { float q = t1; t1 = t2; t2 = q; }
      if (t1 > tmin) tmin = t1;
      if (t2 < tmax) tmax = t2;
      if (tmin > tmax) return false;
    }
  }
  return tmax > 0.0f && tmin < maxT;
}

// Sombra aproximada mediante AABB de los objetos.
static float shadowDirectional(const Luz &L, const Obj &receiver,
                               float px, float py, float pz) {
  // L.direccion apunta desde la luz hacia la escena; el rayo de sombra
  // sale desde la superficie hacia el objeto/luz, por eso se invierte.
  float dx = -(float)L.direccion.x;
  float dy = -(float)L.direccion.y;
  float dz = -(float)L.direccion.z;
  normalizeFloatL(dx, dy, dz);
  if (fabsf(dx) + fabsf(dy) + fabsf(dz) < 0.001f) return 0.0f;

  px += dx * 0.02f;
  py += dy * 0.02f;
  pz += dz * 0.02f;
  for (uint16_t i = 0; i < gLightObjectCount; i++) {
    Obj &o = gLightObjects[i];
    if (&o == &receiver || !o.visible || o.type != BODY || o.is_ground) continue;
    float mnx = fixedToFloatL(o.collision.min.x + o.traslation.x);
    float mny = fixedToFloatL(o.collision.min.y + o.traslation.y);
    float mnz = fixedToFloatL(o.collision.min.z + o.traslation.z);
    float mxx = fixedToFloatL(o.collision.max.x + o.traslation.x);
    float mxy = fixedToFloatL(o.collision.max.y + o.traslation.y);
    float mxz = fixedToFloatL(o.collision.max.z + o.traslation.z);
    if (rayAABBL(px, py, pz, dx, dy, dz, mnx, mny, mnz, mxx, mxy, mxz, 1000000.0f)) return 1.0f;
  }
  return 0.0f;
}

static inline void getLightWorldPositionFloat(const Obj &lightObj,
                                               float &x, float &y, float &z) {
  x = fixedToFloatL(lightObj.position.x) + fixedToFloatL(lightObj.traslation.x);
  y = fixedToFloatL(lightObj.position.y) + fixedToFloatL(lightObj.traslation.y);
  z = fixedToFloatL(lightObj.position.z) + fixedToFloatL(lightObj.traslation.z);
}

static float shadowLocal(const Luz &L, const Obj &lightObj, const Obj &receiver,
                         float px, float py, float pz) {
  float lx, ly, lz;
  getLightWorldPositionFloat(lightObj, lx, ly, lz);
  lx -= px;
  ly -= py;
  lz -= pz;
  float dist2 = lx * lx + ly * ly + lz * lz;
  if (dist2 <= 0.000001f) return 0.0f;

  float dist = sqrtf(dist2);
  lx /= dist;
  ly /= dist;
  lz /= dist;

  px += lx * 0.02f;
  py += ly * 0.02f;
  pz += lz * 0.02f;

  for (uint16_t i = 0; i < gLightObjectCount; i++) {
    Obj &o = gLightObjects[i];
    if (&o == &receiver || &o == &lightObj || !o.visible || o.type != BODY || o.is_ground) continue;
    float mnx = fixedToFloatL(o.collision.min.x + o.traslation.x);
    float mny = fixedToFloatL(o.collision.min.y + o.traslation.y);
    float mnz = fixedToFloatL(o.collision.min.z + o.traslation.z);
    float mxx = fixedToFloatL(o.collision.max.x + o.traslation.x);
    float mxy = fixedToFloatL(o.collision.max.y + o.traslation.y);
    float mxz = fixedToFloatL(o.collision.max.z + o.traslation.z);
    if (rayAABBL(px, py, pz, lx, ly, lz, mnx, mny, mnz, mxx, mxy, mxz, dist - 0.03f)) return 1.0f;
  }
  return 0.0f;
}

static uint16_t calcularIluminacionJuego(const Obj &receiver, const Vector3D &posWorld,
                                         const Vector3D &normalWorld, const Vector3D &pivotCamera,
                                         uint16_t baseColor) {
  (void)pivotCamera;

  float br, bg, bb;
  color565ToFloat(baseColor, br, bg, bb);

  float ar, ag, ab;
  color565ToFloat(gAmbientColor, ar, ag, ab);

  // 1) Luz ambiente: siempre presente y no depende de la orientación de la cara.
  float r = ar * clamp01(gAmbientIntensity);
  float g = ag * clamp01(gAmbientIntensity);
  float b = ab * clamp01(gAmbientIntensity);

  // 2) Sol direccional: Lambert + intensidad configurable + sombra.
  Vector3D nFixed = normalize(normalWorld);
  Vector3D sFixed = normalize(SUN_DIRECTION_WORLD);
  int32_t lamFixed = dot(nFixed, sFixed);
  if (lamFixed < 0) lamFixed = 0;
  if (lamFixed > FIXED_SCALE) lamFixed = FIXED_SCALE;

  float lam = (float)lamFixed / (float)FIXED_SCALE;
  float sunFactor = lam * clamp01(gSunIntensity);

  // Aplicar sombra solar si existe una lista de objetos.
  if (gLightObjectCount > 0) {
    float px = fixedToFloatL(posWorld.x);
    float py = fixedToFloatL(posWorld.y);
    float pz = fixedToFloatL(posWorld.z);
    // La sombra solar se evalúa con el mismo vector del sol.
    // SUN_DIRECTION_WORLD apunta desde la superficie hacia el sol,
    // que es exactamente la dirección que debe seguir el rayo de sombra.
    float dx = (float)SUN_DIRECTION_WORLD.x;
    float dy = (float)SUN_DIRECTION_WORLD.y;
    float dz = (float)SUN_DIRECTION_WORLD.z;
    normalizeFloatL(dx, dy, dz);
    if (fabsf(dx) + fabsf(dy) + fabsf(dz) > 0.001f) {
      px += dx * 0.02f;
      py += dy * 0.02f;
      pz += dz * 0.02f;
      for (uint16_t i = 0; i < gLightObjectCount; i++) {
        Obj &o = gLightObjects[i];
        if (&o == &receiver || !o.visible || o.type != BODY || o.is_ground) continue;
        float mnx = fixedToFloatL(o.collision.min.x + o.traslation.x);
        float mny = fixedToFloatL(o.collision.min.y + o.traslation.y);
        float mnz = fixedToFloatL(o.collision.min.z + o.traslation.z);
        float mxx = fixedToFloatL(o.collision.max.x + o.traslation.x);
        float mxy = fixedToFloatL(o.collision.max.y + o.traslation.y);
        float mxz = fixedToFloatL(o.collision.max.z + o.traslation.z);
        if (rayAABBL(px, py, pz, dx, dy, dz, mnx, mny, mnz, mxx, mxy, mxz, 1000000.0f)) {
          sunFactor *= 0.15f;
          break;
        }
      }
    }
  }
  r += sunFactor;
  g += sunFactor;
  b += sunFactor;

  float px = fixedToFloatL(posWorld.x), py = fixedToFloatL(posWorld.y), pz = fixedToFloatL(posWorld.z);
  float nx = (float)normalWorld.x, ny = (float)normalWorld.y, nz = (float)normalWorld.z;
  normalizeFloatL(nx, ny, nz);

  // 3) Luces locales: puntual, spot, fuego y direccional.
  for (uint16_t i = 0; i < gLightListCount; i++) {
    const Obj &o = *gLightList[i];
    if (!o.visible || !o.luz.encendido) continue;
    if (o.type != LUZPUNTUAL && o.type != LUZSPOT &&
        o.type != FUEGO && o.type != LUZDIRECCIONAL) continue;

    const Luz &L = o.luz;
    float factor = 0.0f;

    if (o.type == LUZDIRECCIONAL) {
      float lx = -(float)L.direccion.x;
      float ly = -(float)L.direccion.y;
      float lz = -(float)L.direccion.z;
      normalizeFloatL(lx, ly, lz);
      factor = nx * lx + ny * ly + nz * lz;
      if (factor < 0.0f) factor = 0.0f;
      factor *= clamp01(fixedToFloatL(L.intensidad));
      factor *= 1.0f - shadowDirectional(L, receiver, px, py, pz) * 0.85f;
    } else {
      float lx, ly, lz;
      getLightWorldPositionFloat(o, lx, ly, lz);
      lx -= px;
      ly -= py;
      lz -= pz;
      float d2 = lx * lx + ly * ly + lz * lz;
      if (d2 <= 0.000001f) continue;
      float d = sqrtf(d2);
      lx /= d;
      ly /= d;
      lz /= d;

      factor = nx * lx + ny * ly + nz * lz;
      if (factor < 0.0f) factor = 0.0f;

      float range = fixedToFloatL(L.alcance);
      if (range > 0.0f) {
        if (d >= range) continue;
        float q = 1.0f - d / range;
        factor *= q * q;
      }

      if (o.type == LUZSPOT) {
        float sx = (float)L.direccion.x;
        float sy = (float)L.direccion.y;
        float sz = (float)L.direccion.z;
        normalizeFloatL(sx, sy, sz);
        float c = lx * sx + ly * sy + lz * sz;
        c = clamp01(c);
        float halfAngle = fixedToFloatL(L.angulo) * 0.5f * 0.01745329252f;
        float half = cosf(halfAngle);
        if (c <= half) continue;
        float spot = (c - half) / (1.0f - half);
        float power = fixedToFloatL(L.suavidad);
        if (power < 1.0f) power = 1.0f;
        factor *= powf(clamp01(spot), power);
      }

      factor *= clamp01(fixedToFloatL(L.intensidad));
      if (showShadow)
        factor *= 1.0f - shadowLocal(L, o, receiver, px, py, pz) * 0.85f;
    }

    addLightColor(r, g, b, L.color, factor);
  }

  // La iluminación multiplica el material original; de esta forma el color
  // de la luz modifica el tinte sin poder generar color donde el material no existe.
  r *= br;
  g *= bg;
  b *= bb;

  return floatToColor565(r, g, b);
}

// Funciones estáticas auxiliares de bajo nivel para SPI
static void lcd_cmd(const uint8_t cmd) {
  gpio_set_level((gpio_num_t)PIN_NUM_CS, 0);
  gpio_set_level((gpio_num_t)PIN_NUM_DC, 0);
  spi_transaction_t t;
  memset(&t, 0, sizeof(t));
  t.length = 8;
  t.tx_buffer = &cmd;
  spi_device_polling_transmit(spi_handle, &t);
  gpio_set_level((gpio_num_t)PIN_NUM_CS, 1);
}

static void lcd_data(const uint8_t *data, int len) {
  if (len == 0) return;
  gpio_set_level((gpio_num_t)PIN_NUM_CS, 0);
  gpio_set_level((gpio_num_t)PIN_NUM_DC, 1);
  spi_transaction_t t;
  memset(&t, 0, sizeof(t));
  t.length = len * 8;
  t.tx_buffer = data;
  spi_device_polling_transmit(spi_handle, &t);
  gpio_set_level((gpio_num_t)PIN_NUM_CS, 1);
}

static void lcd_send_pixels_dma_chunk(const uint16_t *pixels, size_t num_bytes) {
  if (num_bytes == 0) return;

  spi_transaction_t t;
  memset(&t, 0, sizeof(t));
  t.length = num_bytes * 8;
  t.tx_buffer = pixels;

  // spi_device_transmit() utiliza DMA porque el bus fue inicializado
  // con SPI_DMA_CH_AUTO. La espera aquí es solamente por ESTE chunk;
  // el siguiente chunk se lanza inmediatamente manteniendo CS activo.
  esp_err_t err = spi_device_transmit(spi_handle, &t);
  if (err != ESP_OK) {
    Serial.printf("SPI DMA error: %d", (int)err);
  }
}

void ST7789_DMA::beginGameLighting(Obj *objects, uint16_t count) {
  gLightObjects = objects;
  gLightObjectCount = 0;
  gLightListCount = 0;
  // Conservamos todos los objetos porque también son posibles
  // oclusores de sombra.

  // La posición de una luz pertenece al objeto: position + traslation.
  // Así las luces pueden moverse junto al jugador, carro, fogata, etc.
  // Se guarda un puntero a cada luz, sin importar en qué índice del arreglo
  // esté; como son punteros al arreglo original, las modificaciones hechas
  // por el juego (traslation, direccion, encendido) se ven cada frame.
  for (uint16_t i = 0; i < count; ++i) {
    Obj &o = gLightObjects[i];
    if (o.type == LUZPUNTUAL || o.type == LUZSPOT || o.type == FUEGO) {
      gLightObjectCount++;
      if (gLightListCount < MAX_LUCES_REGISTRADAS)
        gLightList[gLightListCount++] = &o;
    }
  }
}

// Implementación de la Clase
ST7789_DMA::ST7789_DMA()
  : buffer_A(NULL), buffer_B(NULL),
    draw_buffer(NULL), disp_buffer(NULL),
    z_buffer(NULL),
    current_block_y(0),
    frame_pass(0),
    dma_pending_A(false), dma_pending_B(false),
    wait_dma_us(0), wait_buffer_us(0) {}


bool ST7789_DMA::begin(void) {
  spi_bus_config_t buscfg;
  memset(&buscfg, 0, sizeof(spi_bus_config_t));
  buscfg.mosi_io_num = PIN_NUM_MOSI;
  buscfg.miso_io_num = PIN_NUM_MISO;
  buscfg.sclk_io_num = PIN_NUM_CLK;
  buscfg.quadwp_io_num = -1;
  buscfg.quadhd_io_num = -1;
  buscfg.max_transfer_sz = DMA_CHUNK_BYTES;
  spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);

  spi_device_interface_config_t devcfg;
  memset(&devcfg, 0, sizeof(spi_device_interface_config_t));
  devcfg.clock_speed_hz = 80 * 1000 * 1000;
  devcfg.mode = 0;
  devcfg.spics_io_num = -1;
  devcfg.queue_size = 2;
  devcfg.flags = SPI_DEVICE_NO_DUMMY;
  spi_bus_add_device(SPI2_HOST, &devcfg, &spi_handle);

  gpio_set_direction((gpio_num_t)PIN_NUM_CS, GPIO_MODE_OUTPUT);
  gpio_set_direction((gpio_num_t)PIN_NUM_DC, GPIO_MODE_OUTPUT);
  gpio_set_direction((gpio_num_t)PIN_NUM_RST, GPIO_MODE_OUTPUT);

  gpio_set_level((gpio_num_t)PIN_NUM_RST, 1);
  vTaskDelay(pdMS_TO_TICKS(10));
  gpio_set_level((gpio_num_t)PIN_NUM_RST, 0);
  vTaskDelay(pdMS_TO_TICKS(50));
  gpio_set_level((gpio_num_t)PIN_NUM_RST, 1);
  vTaskDelay(pdMS_TO_TICKS(150));

  lcd_cmd(0x01);
  vTaskDelay(pdMS_TO_TICKS(150));
  lcd_cmd(0x11);
  vTaskDelay(pdMS_TO_TICKS(120));

  uint8_t colmod = 0x55;
  lcd_cmd(0x3A);
  lcd_data(&colmod, 1);
  uint8_t madctl = 0x00;
  lcd_cmd(0x36);
  lcd_data(&madctl, 1);
  lcd_cmd(0x21);
  vTaskDelay(pdMS_TO_TICKS(10));
  lcd_cmd(0x29);
  vTaskDelay(pdMS_TO_TICKS(50));

  if (psramInit()) {
    Serial.printf("PSRAM iniciada");
  } else {
    Serial.printf("NOOOOO PSRAM iniciada");
  }
  if (psramFound()) {
    // Framebuffers en PSRAM.
    buffer_A = (uint16_t *)ps_malloc(BLOCK_PIXELS * sizeof(uint16_t));
    buffer_B = (uint16_t *)ps_malloc(BLOCK_PIXELS * sizeof(uint16_t));

    // 🛠️ REGRESO A PANTALLA COMPLETA: 240 * 300 * 4 bytes = ~288 KB en PSRAM
    z_buffer = (int32_t *)ps_malloc((size_t)LCD_WIDTH * LCD_HEIGHT * sizeof(int32_t));

    if (z_buffer != NULL) {
      Serial.printf("PSRAM detectada: Z-buffer global restaurado");
    } else {
      Serial.printf("Z-buffer no disponible; render sin Z-buffer");
    }

    gSortedFaces = (FaceDepth*)ps_malloc(30000 * sizeof(FaceDepth));
  } else {
    buffer_A = (uint16_t *)heap_caps_malloc(BLOCK_PIXELS * sizeof(uint16_t), MALLOC_CAP_DMA);
    buffer_B = (uint16_t *)heap_caps_malloc(BLOCK_PIXELS * sizeof(uint16_t), MALLOC_CAP_DMA);
    Serial.printf("PSRAM no detectada: framebuffer creado en RAM interna");
  }

  // ---- ASÍ ERA AL PRINCIPIO: RESERVA DE MEMORIA PARA TUS MANDOS EN LÍNEA ----

  draw_buffer = buffer_A;
  disp_buffer = buffer_B;

  // Retorna true solo si los búferes de pantalla se crearon bien
  return (buffer_A != NULL && buffer_B != NULL);
}

void ST7789_DMA::setLightDir(Vector3D light) {
  SUN_DIRECTION_WORLD = light;
}

void ST7789_DMA::setEnvironmentLighting(uint16_t ambientColor,
                                         float ambientIntensity,
                                         float sunIntensity) {
  gAmbientColor = ambientColor;
  gAmbientIntensity = clamp01(ambientIntensity);
  gSunIntensity = clamp01(sunIntensity);
}

void ST7789_DMA::waitForBuffer(uint16_t *buffer) {
  bool *pending = NULL;

  if (buffer == buffer_A) pending = &dma_pending_A;
  else if (buffer == buffer_B) pending = &dma_pending_B;
  else return;

  if (!*pending) return;

  uint32_t t0 = micros();

  // Puede completarse primero la otra transacción. La consumimos y
  // actualizamos su bandera; seguimos esperando hasta liberar 'buffer'.
  while (*pending) {
    spi_transaction_t *result = NULL;
    spi_device_get_trans_result(spi_handle, &result, portMAX_DELAY);

    if (result == &spi_trans_A) dma_pending_A = false;
    else if (result == &spi_trans_B) dma_pending_B = false;
  }

  uint32_t dt = micros() - t0;
  wait_buffer_us += dt;
  wait_dma_us += dt;

  // Sólo liberamos CS cuando ya no queda ninguna transferencia DMA.
  if (!dma_pending_A && !dma_pending_B)
    gpio_set_level((gpio_num_t)PIN_NUM_CS, 1);
}

void ST7789_DMA::drainDMA() {
  if (!dma_pending_A && !dma_pending_B) return;

  uint32_t t0 = micros();

  while (dma_pending_A || dma_pending_B) {
    spi_transaction_t *result = NULL;
    spi_device_get_trans_result(spi_handle, &result, portMAX_DELAY);

    if (result == &spi_trans_A) dma_pending_A = false;
    else if (result == &spi_trans_B) dma_pending_B = false;
  }

  uint32_t dt = micros() - t0;
  wait_dma_us += dt;

  gpio_set_level((gpio_num_t)PIN_NUM_CS, 1);
}

void ST7789_DMA::startFrame(void) {
  // Aún pueden quedar hasta dos bloques en el periférico SPI.
  // Los drenamos antes de enviar los comandos del siguiente frame.
  drainDMA();

  current_block_y = 0;
  frame_pass = 0;
  wait_dma_us = 0;
  wait_buffer_us = 0;

  lcd_cmd(0x2A);
  uint8_t d1[] = { 0, 0,
                   (uint8_t)((LCD_WIDTH - 1) >> 8),
                   (uint8_t)((LCD_WIDTH - 1) & 0xFF) };
  lcd_data(d1, 4);

  lcd_cmd(0x2B);
  uint8_t d2[] = { 0, 0,
                   (uint8_t)((LCD_HEIGHT - 1) >> 8),
                   (uint8_t)((LCD_HEIGHT - 1) & 0xFF) };
  lcd_data(d2, 4);

  lcd_cmd(0x2C);
}

bool ST7789_DMA::nextBlock(void) {
  if (frame_pass >= 2)
    return false;

  waitForBuffer(draw_buffer);
  current_block_y = 0;

  // Dentro de nextBlock(), si usas 'depth < *zrow':
  if (z_buffer != NULL) {
    const size_t z_pixels = (size_t)LCD_WIDTH * LCD_HEIGHT * sizeof(int32_t);
    // 0x7F genera un entero positivo gigante (+2,139,062,143). 
    // Cualquier objeto real tendrá un Z menor (más cerca) y ganará la prueba.
    memset(z_buffer, 0x7F, z_pixels);
  }

  return true;
}

void ST7789_DMA::endFrame(void) {
  // Un framebuffer completo mide:
  //
  //   240 * 300 * 2 = 144000 bytes
  //
  // El controlador SPI no permite enviar esos 144000 bytes como una
  // sola transacción. Por eso el framebuffer se mantiene COMPLETO en
  // A/B, pero físicamente se transmite en varios chunks DMA.
  //
  // CS permanece activo durante TODOS los chunks, de modo que para el
  // ST7789 esto sigue siendo una única escritura RAMWR de 240x300.

  // Aseguramos que no quede ninguna transferencia anterior.
  drainDMA();

  uint16_t *rendered = draw_buffer;
  const size_t total_bytes = (size_t)BLOCK_PIXELS * sizeof(uint16_t);
  const uint8_t *src = reinterpret_cast<const uint8_t *>(rendered);

  gpio_set_level((gpio_num_t)PIN_NUM_CS, 0);
  gpio_set_level((gpio_num_t)PIN_NUM_DC, 1);

  size_t offset = 0;
  while (offset < total_bytes) {
    size_t chunk = total_bytes - offset;
    if (chunk > DMA_CHUNK_BYTES)
      chunk = DMA_CHUNK_BYTES;

    lcd_send_pixels_dma_chunk(
      reinterpret_cast<const uint16_t *>(src + offset), chunk);

    offset += chunk;
  }

  gpio_set_level((gpio_num_t)PIN_NUM_CS, 1);

  // Ping-pong: después de transmitir A, el render continúa sobre B;
  // después de B, vuelve a A.
  uint16_t *temp = draw_buffer;
  draw_buffer = disp_buffer;
  disp_buffer = temp;

  frame_pass++;
  current_block_y = 0;
}

void ST7789_DMA::setBufferingZ(bool buff){
  zbuffering = buff;
}

void ST7789_DMA::setShadows(bool shadow){
  showShadow = shadow;
}

// Métodos de Dibujo con Lógica "Sliced" Integrada
void ST7789_DMA::clear(uint16_t color) {
  uint16_t raw_color = __builtin_bswap16(color);
  const size_t z_pixels = (size_t)LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t);
  memset(draw_buffer, raw_color, z_pixels);
}

void ST7789_DMA::fillRect(int x, int y, int w, int h, uint16_t color, uint8_t alpha) {
  uint16_t raw_color = __builtin_bswap16(color);
  int block_end_y = current_block_y + LINES_PER_BLOCK;
  if ((y + h) <= current_block_y || y >= block_end_y) return;

  int start_y = (y < current_block_y) ? current_block_y : y;
  int end_y = ((y + h) > block_end_y) ? block_end_y : (y + h);

  for (int global_y = start_y; global_y < end_y; global_y++) {
    int local_y = global_y - current_block_y;
    for (int col = x; col < x + w; col++) {
      if (col >= LCD_WIDTH || col < 0) continue;
      draw_buffer[local_y * LCD_WIDTH + col] = raw_color;
    }
  }
}

void ST7789_DMA::drawRect(int x, int y, int w, int h, uint16_t color, uint8_t alpha) {
  fillRect(x, y, w, 1, color, alpha);
  fillRect(x, y + h - 1, w, 1, color, alpha);
  fillRect(x, y, 1, h, color, alpha);
  fillRect(x + w - 1, y, 1, h, color, alpha);
}

void ST7789_DMA::drawLine(int x0, int y0, int x1, int y1, uint16_t color, uint8_t alpha) {
  uint16_t raw_color = __builtin_bswap16(color);
  int block_end_y = current_block_y + LINES_PER_BLOCK;
  if ((y0 < current_block_y && y1 < current_block_y) || (y0 >= block_end_y && y1 >= block_end_y)) return;

  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy, e2;

  while (1) {
    if (y0 >= current_block_y && y0 < block_end_y && x0 >= 0 && x0 < LCD_WIDTH) {
      draw_buffer[(y0 - current_block_y) * LCD_WIDTH + x0] = raw_color;
    }
    if (x0 == x1 && y0 == y1) break;
    e2 = 2 * err;
    if (e2 >= dy) {
      err += dy;
      x0 += sx;
    }
    if (e2 <= dx) {
      err += dx;
      y0 += sy;
    }
  }
}

void ST7789_DMA::drawCircle(int x_center, int y_center, int r, uint16_t color, uint8_t alpha) {
  uint16_t raw_color = __builtin_bswap16(color);
  int block_end_y = current_block_y + LINES_PER_BLOCK;
  if ((y_center + r) < current_block_y || (y_center - r) >= block_end_y) return;

  int x = 0;
  int y = r;
  int d = 3 - 2 * r;
  auto draw_pixel = [&](int px, int py) {
    if (py >= current_block_y && py < block_end_y && px >= 0 && px < LCD_WIDTH) {
      draw_buffer[(py - current_block_y) * LCD_WIDTH + px] = raw_color;
    }
  };
  while (y >= x) {
    draw_pixel(x_center + x, y_center + y);
    draw_pixel(x_center - x, y_center + y);
    draw_pixel(x_center + x, y_center - y);
    draw_pixel(x_center - x, y_center - y);
    draw_pixel(x_center + y, y_center + x);
    draw_pixel(x_center - y, y_center + x);
    draw_pixel(x_center + y, y_center - x);
    draw_pixel(x_center - y, y_center - x);
    x++;
    if (d > 0) {
      y--;
      d = d + 4 * (x - y) + 10;
    } else {
      d = d + 4 * x + 6;
    }
  }
}

void ST7789_DMA::drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint16_t color, uint8_t alpha) {
  drawLine(x0, y0, x1, y1, color);
  drawLine(x1, y1, x2, y2, color);
  drawLine(x2, y2, x0, y0, color);
}


// Dibuja un carácter aplicando escala y transparencia usando estrictamente matemáticas de punto fijo
void ST7789_DMA::drawCustomChar(int16_t x, int16_t y, char c, const Font *font, uint16_t color, int32_t scale_fixed, uint8_t alpha) {
  if (alpha == 0) return;
  if (c < font->first || c > font->last) return;

  const GFXglyph *glyph = &(font->glyphs[c - font->first]);
  uint8_t w = glyph->width;
  uint8_t h = glyph->height;

  if (w == 0 || h == 0) return;

  // Calcular dimensiones escaladas convirtiendo primero a punto fijo, multiplicando y regresando a entero
  int16_t scaled_w = FIXED_TO_INT(fixed_mul(INT_TO_FIXED(w), scale_fixed));
  int16_t scaled_h = FIXED_TO_INT(fixed_mul(INT_TO_FIXED(h), scale_fixed));

  // Coordenadas iniciales globales de dibujo en píxeles enteros
  int16_t global_start_y = y + FIXED_TO_INT(fixed_mul(INT_TO_FIXED(glyph->yOffset), scale_fixed));
  int16_t global_start_x = x + FIXED_TO_INT(fixed_mul(INT_TO_FIXED(glyph->xOffset), scale_fixed));

  int block_end_y = current_block_y + LINES_PER_BLOCK;

  // Descarte veloz por bloques
  if ((global_start_y + scaled_h) <= current_block_y || global_start_y >= block_end_y) return;

  uint32_t bitmap_offset = glyph->bitmapOffset;
  uint8_t bits = font->bitmap[bitmap_offset++];
  uint8_t bit_counter = 0;

  uint8_t r_src = (color >> 11) & 0x1F;
  uint8_t g_src = (color >> 5) & 0x3F;
  uint8_t b_src = color & 0x1F;

  for (int yy = 0; yy < h; yy++) {
    // Mapeo vertical del micro-bloque en punto fijo
    int32_t cur_y_fixed = INT_TO_FIXED(yy);
    int32_t nxt_y_fixed = INT_TO_FIXED(yy + 1);
    int16_t start_scaled_y = global_start_y + FIXED_TO_INT(fixed_mul(cur_y_fixed, scale_fixed));
    int16_t end_scaled_y = global_start_y + FIXED_TO_INT(fixed_mul(nxt_y_fixed, scale_fixed));

    for (int xx = 0; xx < w; xx++) {
      bool pixel_active = (bits & 0x80) != 0;

      bits <<= 1;
      bit_counter++;
      if (bit_counter >= 8) {
        bits = font->bitmap[bitmap_offset++];
        bit_counter = 0;
      }

      if (pixel_active) {
        // Mapeo horizontal del micro-bloque en punto fijo
        int32_t cur_x_fixed = INT_TO_FIXED(xx);
        int32_t nxt_x_fixed = INT_TO_FIXED(xx + 1);
        int16_t start_scaled_x = global_start_x + FIXED_TO_INT(fixed_mul(cur_x_fixed, scale_fixed));
        int16_t end_scaled_x = global_start_x + FIXED_TO_INT(fixed_mul(nxt_x_fixed, scale_fixed));

        // Pintar el micro-bloque escalado
        for (int16_t sy = start_scaled_y; sy < end_scaled_y; sy++) {
          if (sy < current_block_y || sy >= block_end_y) continue;
          int local_y = sy - current_block_y;

          for (int16_t sx = start_scaled_x; sx < end_scaled_x; sx++) {
            if (sx < 0 || sx >= LCD_WIDTH) continue;

            uint16_t final_color;

            if (alpha == 255) {
              final_color = __builtin_bswap16(color);
            } else {
              uint16_t bg_raw = __builtin_bswap16(draw_buffer[local_y * LCD_WIDTH + sx]);
              uint8_t r_bg = (bg_raw >> 11) & 0x1F;
              uint8_t g_bg = (bg_raw >> 5) & 0x3F;
              uint8_t b_bg = bg_raw & 0x1F;

              // Alpha Blending con puros enteros
              uint8_t r_out = (r_src * alpha + r_bg * (255 - alpha)) / 255;
              uint8_t g_out = (g_src * alpha + g_bg * (255 - alpha)) / 255;
              uint8_t b_out = (b_src * alpha + b_bg * (255 - alpha)) / 255;

              final_color = __builtin_bswap16((r_out << 11) | (g_out << 5) | b_out);
            }

            draw_buffer[local_y * LCD_WIDTH + sx] = final_color;
          }
        }
      }
    }
  }
}

// Renderizar texto sumando xAdvance calculado mediante punto fijo
void ST7789_DMA::drawText(int16_t x, int16_t y, const char *str, String _font, uint16_t color, int32_t scale_fixed, uint8_t alpha) {
  const Font *f = &FreeSans9pt7b;

  _font.toUpperCase();
  if (_font == "SANS") f = &FreeSans9pt7b;
  else if (_font == "SANSBOLD") f = &FreeSansBold9pt7b;
  else if (_font == "VIOLETA") f = &ArianaVioleta_dz2K18pt7b;
  else if (_font == "SOLARONA") f = &SolaronaDemo_6R83Y18pt7b;
  else if (_font == "BEAT") f = &BeatWordDemo_nRL2018pt7b;
  else f = &FreeSans9pt7b;

  int16_t cursorX = x;
  int16_t cursorY = y;

  while (*str) {
    char c = *str;

    if (c == '\n') {
      cursorX = x;
      cursorY += FIXED_TO_INT(fixed_mul(INT_TO_FIXED(f->yAdvance), scale_fixed));
      str++;
      continue;
    }

    if (c >= f->first && c <= f->last) {
      const GFXglyph *glyph = &(f->glyphs[c - f->first]);

      drawCustomChar(cursorX, cursorY, c, f, color, scale_fixed, alpha);

      int32_t advance_fixed = INT_TO_FIXED(glyph->xAdvance);
      cursorX += FIXED_TO_INT(fixed_mul(advance_fixed, scale_fixed));

      if (cursorX >= LCD_WIDTH) break;
    }
    str++;
  }
}

// ---- IMPLEMENTACIÓN DE CÍRCULO RELLENO EN BLOQUES ----
void ST7789_DMA::fillCircle(int x_center, int y_center, int r, uint16_t color, uint8_t alpha) {
  int block_end_y = current_block_y + LINES_PER_BLOCK;

  // Descarte rápido: Si el círculo no toca verticalmente este bloque, salir
  if ((y_center + r) < current_block_y || (y_center - r) >= block_end_y) return;

  uint16_t raw_color = __builtin_bswap16(color);

  // Determinar las líneas del círculo que caen estrictamente dentro del bloque actual
  int start_y = (y_center - r < current_block_y) ? current_block_y : y_center - r;
  int end_y = (y_center + r >= block_end_y) ? block_end_y - 1 : y_center + r;

  for (int global_y = start_y; global_y <= end_y; global_y++) {
    int local_y = global_y - current_block_y;

    // Calcular el ancho de la línea horizontal en esta altura del círculo usando Pitágoras
    int dy = global_y - y_center;
    int dx = (int)sqrtf(r * r - dy * dy);

    int start_x = x_center - dx;
    int end_x = x_center + dx;

    // Recortar los límites en el eje X de la pantalla
    if (start_x < 0) start_x = 0;
    if (end_x >= LCD_WIDTH) end_x = LCD_WIDTH - 1;

    // Pintar la línea horizontal en el buffer de dibujo actual
    for (int global_x = start_x; global_x <= end_x; global_x++) {
      draw_buffer[local_y * LCD_WIDTH + global_x] = raw_color;
    }
  }
}


// Rasterizador por spans con interpolación UV.
// Se utiliza exclusivamente cuando obj.type_geometry == GEOMETRY_TEXTURED_OBJ.
// La ruta fillTriangle() existente permanece intacta para todos los objetos actuales.
void ST7789_DMA::fillTriangleUV(int x0, int y0, int x1, int y1, int x2, int y2,
                                int32_t u0, int32_t v0, int32_t u1, int32_t v1,
                                int32_t u2, int32_t v2, int32_t intensity) {
  const int block_start = current_block_y;
  const int block_end = current_block_y + LINES_PER_BLOCK - 1;

  // Orden vertical conservando X y UV.
  auto swapVertex = [](int &xA, int &yA, int32_t &uA, int32_t &vA,
                       int &xB, int &yB, int32_t &uB, int32_t &vB) {
    std::swap(yA, yB);
    std::swap(xA, xB);
    std::swap(uA, uB);
    std::swap(vA, vB);
  };
  if (y0 > y1) swapVertex(x0, y0, u0, v0, x1, y1, u1, v1);
  if (y1 > y2) swapVertex(x1, y1, u1, v1, x2, y2, u2, v2);
  if (y0 > y1) swapVertex(x0, y0, u0, v0, x1, y1, u1, v1);

  if (y0 == y2 || y2 < block_start || y0 > block_end) return;

  constexpr int SHIFT = 16;
  constexpr int64_t ONE = (int64_t)1 << SHIFT;

  auto rasterHalf = [&](int ax, int ay, int32_t au, int32_t av,
                        int bx, int by, int32_t bu, int32_t bv,
                        int cx, int cy, int32_t cu, int32_t cv,
                        int dx, int dy, int32_t du, int32_t dv,
                        int yBegin, int yEnd) {
    if (yBegin > yEnd || by == ay || dy == cy) return;

    const int64_t sxA = ((int64_t)(bx - ax) << SHIFT) / (by - ay);
    const int64_t sxB = ((int64_t)(dx - cx) << SHIFT) / (dy - cy);
    const int64_t suA = ((int64_t)(bu - au) << SHIFT) / (by - ay);
    const int64_t suB = ((int64_t)(du - cu) << SHIFT) / (dy - cy);
    const int64_t svA = ((int64_t)(bv - av) << SHIFT) / (by - ay);
    const int64_t svB = ((int64_t)(dv - cv) << SHIFT) / (dy - cy);

    int64_t xa = ((int64_t)ax << SHIFT) + sxA * (yBegin - ay);
    int64_t xb = ((int64_t)cx << SHIFT) + sxB * (yBegin - cy);
    int64_t ua = ((int64_t)au << SHIFT) + suA * (yBegin - ay);
    int64_t ub = ((int64_t)cu << SHIFT) + suB * (yBegin - cy);
    int64_t va = ((int64_t)av << SHIFT) + svA * (yBegin - ay);
    int64_t vb = ((int64_t)cv << SHIFT) + svB * (yBegin - cy);

    for (int y = yBegin; y <= yEnd; ++y) {
      int xaPix = (int)((xa + (ONE >> 1)) >> SHIFT);
      int xbPix = (int)((xb + (ONE >> 1)) >> SHIFT);
      int64_t ul = ua, vl = va, ur = ub, vr = vb;
      if (xaPix > xbPix) {
        std::swap(xaPix, xbPix);
        std::swap(ul, ur);
        std::swap(vl, vr);
      }

      if (xbPix >= 0 && xaPix < LCD_WIDTH && y >= 0 && y < LCD_HEIGHT) {
        int xs = max(0, xaPix), xe = min(LCD_WIDTH - 1, xbPix);
        const int dxPix = xbPix - xaPix;
        int64_t u = ul, v = vl;
        int64_t du_dx = 0, dv_dx = 0;
        if (dxPix > 0) {
          du_dx = (ur - ul) / dxPix;
          dv_dx = (vr - vl) / dxPix;
          const int skip = xs - xaPix;
          u += du_dx * skip;
          v += dv_dx * skip;
        }

        uint16_t *dst = draw_buffer + (y - current_block_y) * LCD_WIDTH + xs;
        for (int x = xs; x <= xe; ++x) {
          int uu = (int)(u >> SHIFT);
          int vv = (int)(v >> SHIFT);
          if (uu < 0) uu = 0;
          else if (uu >= TEXTURE_WIDTH) uu = TEXTURE_WIDTH - 1;
          // OBJ/Blender UV tiene V creciendo hacia arriba; la imagen crece hacia abajo.
          int ty = (int)((((int64_t)(FIXED_SCALE - vv)) * (TEXTURE_HEIGHT - 1)) >> SHIFT);
          if (ty < 0) ty = 0;
          else if (ty >= TEXTURE_HEIGHT) ty = TEXTURE_HEIGHT - 1;
          uint16_t texel = pgm_read_word(&texture[ty * TEXTURE_WIDTH + uu]);
          if (intensity != FIXED_SCALE) texel = scale_color_fixed(texel, intensity);
          dst[x - xs] = __builtin_bswap16(texel);
          u += du_dx;
          v += dv_dx;
        }
      }
      xa += sxA;
      xb += sxB;
      ua += suA;
      ub += suB;
      va += svA;
      vb += svB;
    }
  };

  if (y1 > y0) {
    rasterHalf(x0, y0, u0, v0, x1, y1, u1, v1, x0, y0, u0, v0, x2, y2, u2, v2,
               max(y0, block_start), min(y1, block_end));
  }
  if (y2 > y1) {
    rasterHalf(x1, y1, u1, v1, x2, y2, u2, v2, x0, y0, u0, v0, x2, y2, u2, v2,
               max(y1, block_start), min(y2, block_end));
  }
}

void ST7789_DMA::drawSpanShadow(int y, int x_start, int x_end,
                                int64_t lx_start, int64_t ly_start, int64_t lz_start,
                                int64_t lx_end, int64_t ly_end, int64_t lz_end,
                                uint16_t color, uint8_t alpha) {
  /*if (x_start >= lcdW || x_end < 0) return;
  if (x_start < 0) x_start = 0;
  if (x_end >= lcdW) x_end = lcdW - 1;
  if (x_start > x_end) return;

  int32_t width = x_end - x_start;
  if (width == 0) width = 1; // Evitar división por cero

  // Incrementos por cada paso horizontal en pantalla (X)
  int64_t step_lx = (lx_end - lx_start) / width;
  int64_t step_ly = (ly_end - ly_start) / width;
  int64_t step_lz = (lz_end - lz_start) / width;

  int64_t curr_lx = lx_start;
  int64_t curr_ly = ly_start;
  int64_t curr_lz = lz_start;

  for (int x = x_start; x <= x_end; ++x) {
    // Extraemos los valores tridimensionales reales en el espacio de la luz
    int32_t lightX = (int32_t)(curr_lx >> 16);
    int32_t lightY = (int32_t)(curr_ly >> 16);
    int32_t lightZ = (int32_t)(curr_lz >> 16);

    // Proyectamos estas coordenadas tridimensionales de luz al plano del Shadow Map (escala y centro de la luz)
    // Usamos variables de proyección idénticas a las que definimos en la pasada 1.
    Vector3D lightSpaceP = { lightX, lightY, lightZ };
    Vector2D shadowTexCoord = project(lightSpaceP, SHADOW_SCALE, SHADOW_MAP_W / 2, SHADOW_MAP_H / 2, false);

    bool inShadow = false;

    // Verificar si las coordenadas caen dentro de la textura del Shadow Map
    if (shadowTexCoord.x >= 0 && shadowTexCoord.x < SHADOW_MAP_W && shadowTexCoord.y >= 0 && shadowTexCoord.y < SHADOW_MAP_H) {
      int16_t depthInMap = shadowMap[shadowTexCoord.x + shadowTexCoord.y * SHADOW_MAP_W];
      
      int32_t bias = 60; // Ajuste para mitigar el Shadow Acne basándose en la resolución de tu matriz fija
      if ((lightZ + bias) > depthInMap) {
        inShadow = true;
      }
    }

    // Mezcla de color si está en sombra
    uint16_t pixelColor = inShadow ? darkenColor(color) : color;

    // Dibujamos el píxel utilizando el sistema nativo de tu búfer de bloque DMA actual
    drawPixelInCurrentBlock(x, y, pixelColor, alpha);

    curr_lx += step_lx;
    curr_ly += step_ly;
    curr_lz += step_lz;
  }*/
}


// ---- IMPLEMENTACIÓN DE TRIÁNGULO RELLENO EN BLOQUES (Scanline Algorithm) ----
// Dibuja un span horizontal [x0..x1] en una sola fila del bloque DMA actual.
// El recorte se hace antes del bucle de píxeles para evitar trabajo innecesario.
void ST7789_DMA::drawSpan(int y, int x0, int x1, uint16_t color, uint8_t alpha) {
  const int block_end_y = current_block_y + LINES_PER_BLOCK;

  if (y < current_block_y || y >= block_end_y) return;
  if (x0 > x1) std::swap(x0, x1);

  // Clipping horizontal
  if (x1 < 0 || x0 >= LCD_WIDTH) return;
  if (x0 < 0) x0 = 0;
  if (x1 >= LCD_WIDTH) x1 = LCD_WIDTH - 1;

  uint16_t raw_color = __builtin_bswap16(color);

  // Si hay transparencia (alpha), dejamos el método píxel a píxel tradicional
  if (alpha != 255) {
    uint8_t r_src = (color >> 11) & 0x1F;
    uint8_t g_src = (color >> 5) & 0x3F;
    uint8_t b_src = color & 0x1F;
    
    uint16_t *dst = draw_buffer + (y - current_block_y) * LCD_WIDTH + x0;
    const int count = x1 - x0 + 1;
    for (int i = 0; i < count; ++i) {
      uint16_t bg_raw = __builtin_bswap16(dst[i]);
      uint8_t r_bg = (bg_raw >> 11) & 0x1F;
      uint8_t g_bg = (bg_raw >> 5) & 0x3F;
      uint8_t b_bg = bg_raw & 0x1F;

      uint8_t r_out = (r_src * alpha + r_bg * (255 - alpha)) / 255;
      uint8_t g_out = (g_src * alpha + g_bg * (255 - alpha)) / 255;
      uint8_t b_out = (b_src * alpha + b_bg * (255 - alpha)) / 255;

      dst[i] = __builtin_bswap16((r_out << 11) | (g_out << 5) | b_out);
    }
    return;
  }

  // --- OPTIMIZACIÓN SIMD DE 32 BITS PARA SÓLIDOS (ALPHA = 255) ---
  uint16_t *dst = draw_buffer + (y - current_block_y) * LCD_WIDTH + x0;
  int count = x1 - x0 + 1;

  // 1. Alinear el puntero de destino a un límite de 32 bits (si empieza en píxel impar)
  if (((uintptr_t)dst & 3) != 0 && count > 0) {
    *dst++ = raw_color;
    count--;
  }

  // 2. Empaquetar DOS píxeles de 16 bits en un único registro de 32 bits
  uint32_t double_pixel = ((uint32_t)raw_color << 16) | raw_color;
  uint32_t *dst32 = (uint32_t *)dst;

  // 3. Escribir bloques de 2 píxeles por ciclo de reloj de forma masiva
  int loops = count >> 1; // Dividido entre 2
  for (int i = 0; i < loops; ++i) {
    dst32[i] = double_pixel; // Escribe 2 píxeles de un solo golpe
  }

  // 4. Pintar el píxel sobrante si la cuenta era impar
  if (count & 1) {
    dst[loops << 1] = raw_color;
  }
}





// Rasterizador por spans con aristas incrementales.
// En lugar de calcular x mediante una división para cada scanline,
// cada arista mantiene su intersección X en punto fijo 16.16.
// Así, cada fila sólo necesita:
//   1) obtener las dos intersecciones,
//   2) ordenarlas,
//   3) rellenar el span horizontal.
void ST7789_DMA::fillTriangle(int x0, int y0, int x1, int y1,
                              int x2, int y2, uint16_t color, uint8_t alpha) {
  const int block_start = current_block_y;
  const int block_end = current_block_y + LINES_PER_BLOCK - 1;

  // Orden vertical: y0 <= y1 <= y2
  if (y0 > y1) {
    std::swap(y0, y1);
    std::swap(x0, x1);
  }
  if (y1 > y2) {
    std::swap(y1, y2);
    std::swap(x1, x2);
  }
  if (y0 > y1) {
    std::swap(y0, y1);
    std::swap(x0, x1);
  }

  // Triángulo degenerado.
  if (y0 == y2) return;

  // Si el triángulo no toca este bloque DMA, no hacemos nada.
  if (y2 < block_start || y0 > block_end) return;

  // Punto fijo 16.16.
  constexpr int FP_SHIFT = 16;
  constexpr int64_t FP_ONE = (int64_t)1 << FP_SHIFT;

  // Función para rasterizar un tramo entre dos vértices.
  // Se ejecuta sólo en las filas que pertenecen al bloque actual.
  auto rasterEdgePair = [&](int ax, int ay, int bx, int by,
                            int cx, int cy, int dx, int dy,
                            int y_begin, int y_end) {
    if (y_begin > y_end) return;

    // Ambas aristas deben tener altura.
    if (by == ay || dy == cy) return;

    // Pendiente X/Y calculada UNA sola vez por arista.
    const int64_t step_a =
      ((int64_t)(bx - ax) << FP_SHIFT) / (by - ay);

    const int64_t step_b =
      ((int64_t)(dx - cx) << FP_SHIFT) / (dy - cy);

    // Intersección X inicial exactamente en y_begin.
    int64_t xa =
      ((int64_t)ax << FP_SHIFT) + step_a * (y_begin - ay);

    int64_t xb =
      ((int64_t)cx << FP_SHIFT) + step_b * (y_begin - cy);

    for (int y = y_begin; y <= y_end; ++y) {
      // Conversión de punto fijo a píxel.
      // +0.5 equivale a redondeo al píxel más cercano.
      int x_start = (int)((xa + (FP_ONE >> 1)) >> FP_SHIFT);
      int x_end = (int)((xb + (FP_ONE >> 1)) >> FP_SHIFT);

      drawSpan(y, x_start, x_end, color, alpha);

      // Avance incremental: NO hay división aquí.
      xa += step_a;
      xb += step_b;
    }
  };

  // ------------------------------------------------------------
  // Arista larga P0 -> P2
  // ------------------------------------------------------------
  const int dy02 = y2 - y0;

  // MITAD SUPERIOR: P0 -> P1 contra P0 -> P2
  if (y1 > y0) {
    const int first_y = max(y0, block_start);
    const int last_y = min(y1, block_end);

    if (first_y <= last_y) {
      rasterEdgePair(
        x0, y0, x1, y1,
        x0, y0, x2, y2,
        first_y, last_y);
    }
  }

  // MITAD INFERIOR: P1 -> P2 contra P0 -> P2
  if (y2 > y1) {
    const int first_y = max(y1, block_start);
    const int last_y = min(y2, block_end);

    if (first_y <= last_y) {
      rasterEdgePair(
        x1, y1, x2, y2,
        x0, y0, x2, y2,
        first_y, last_y);
    }
  }
}


// Rasterizador por spans con Z-buffer opcional.
//
// El Z-buffer contiene la profundidad de cada píxel de la pantalla completa.
// Se mantiene entre las dos pasadas A/B del framebuffer y se limpia una sola
// vez en startFrame(). Sólo se llama a esta función cuando z_buffer != NULL.
void ST7789_DMA::fillTriangleZ(int x0, int y0, int32_t z0,
                               int x1, int y1, int32_t z1,
                               int x2, int y2, int32_t z2,
                               uint16_t color, uint8_t alpha) {
  const int block_start = current_block_y;
  const int block_end = current_block_y + LINES_PER_BLOCK - 1;

  // Mantener exactamente el mismo orden/rasterización vertical que
  // fillTriangle(), pero añadiendo profundidad a cada extremo del span.
  if (y0 > y1) {
    std::swap(y0, y1);
    std::swap(x0, x1);
    std::swap(z0, z1);
  }
  if (y1 > y2) {
    std::swap(y1, y2);
    std::swap(x1, x2);
    std::swap(z1, z2);
  }
  if (y0 > y1) {
    std::swap(y0, y1);
    std::swap(x0, x1);
    std::swap(z0, z1);
  }

  if (y0 == y2) return;
  if (y2 < block_start || y0 > block_end) return;

  constexpr int FP_SHIFT = 16;
  constexpr int64_t FP_ONE = (int64_t)1 << FP_SHIFT;

  auto rasterEdgePairZ = [&](int ax, int ay, int32_t az,
                             int bx, int by, int32_t bz,
                             int cx, int cy, int32_t cz,
                             int dx, int dy, int32_t dz,
                             int y_begin, int y_end) {
    if (y_begin > y_end) return;
    if (by == ay || dy == cy) return;

    // Igual que fillTriangle(): X se calcula en 16.16 para conservar
    // exactamente la geometría del rasterizador original.
    const int64_t step_a_x = ((int64_t)(bx - ax) << FP_SHIFT) / (by - ay);
    const int64_t step_b_x = ((int64_t)(dx - cx) << FP_SHIFT) / (dy - cy);

    // Z también se mantiene en 16.16 durante la interpolación vertical.
    const int64_t step_a_z = ((int64_t)(bz - az) << FP_SHIFT) / (by - ay);
    const int64_t step_b_z = ((int64_t)(dz - cz) << FP_SHIFT) / (dy - cy);

    int64_t xa = ((int64_t)ax << FP_SHIFT) + step_a_x * (y_begin - ay);
    int64_t xb = ((int64_t)cx << FP_SHIFT) + step_b_x * (y_begin - cy);
    int64_t za = ((int64_t)az << FP_SHIFT) + step_a_z * (y_begin - ay);
    int64_t zb = ((int64_t)cz << FP_SHIFT) + step_b_z * (y_begin - cy);

    for (int y = y_begin; y <= y_end; ++y) {
      int x_start = (int)((xa + (FP_ONE >> 1)) >> FP_SHIFT);
      int x_end = (int)((xb + (FP_ONE >> 1)) >> FP_SHIFT);
      int64_t z_start = za;
      int64_t z_end = zb;

      if (x_start > x_end) {
        std::swap(x_start, x_end);
        std::swap(z_start, z_end);
      }

      if (x_start <= x_end && x_end >= 0 && x_start < LCD_WIDTH) {
        int draw_x0 = max(x_start, 0);
        int draw_x1 = min(x_end, LCD_WIDTH - 1);

        const int64_t dx_span = (int64_t)x_end - x_start;
        const int64_t dz_span = z_end - z_start;

        uint16_t *dst = draw_buffer + (y - current_block_y) * LCD_WIDTH + draw_x0;
        int32_t *zrow = z_buffer + y * LCD_WIDTH + draw_x0;
        const uint16_t raw_color = __builtin_bswap16(color);

        // FASE 1: interpolación incremental de profundidad.
        // Antes se hacía una división entera por CADA píxel del span:
        //   z = z_start + dz * (x-x_start) / dx
        // Ahora calculamos el incremento una sola vez por span y
        // avanzamos z con una suma en cada píxel.
        int64_t z_step = 0;
        if (dx_span != 0) {
          z_step = dz_span / dx_span;
        }

        // Si el span está recortado por el borde izquierdo, arrancamos
        // directamente en draw_x0 para no recorrer píxeles fuera de pantalla.
        int64_t z_fp = z_start;
        if (dx_span != 0 && draw_x0 > x_start) {
          z_fp += z_step * (int64_t)(draw_x0 - x_start);
        }

        for (int x = draw_x0; x <= draw_x1; ++x) {
          // Convertimos la profundidad a entero.
          const int32_t depth = (int32_t)(z_fp >> FP_SHIFT);

          // 🛠️ CONFIGURACIÓN DE ORDEN CORRECTO:
          // Si el fondo se inicializa con un negativo gigante (0x80), cualquier objeto real 
          // tiene un valor de profundidad algebraicamente mayor. 
          // Para evitar que el suelo se trague a los objetos, invertimos la lógica de la comparación:
          // Si tu escala maneja profundidad inversa, cambia '>' por '<'.
          if (depth < *zrow) { 
            *zrow = depth;
            *dst = raw_color;
          }

          ++dst;
          ++zrow;

          // Siguiente píxel: sin división.
          z_fp += z_step;
        }
      }

      xa += step_a_x;
      xb += step_b_x;
      za += step_a_z;
      zb += step_b_z;
    }
  };

  if (y1 > y0) {
    const int first_y = max(y0, block_start);
    const int last_y = min(y1, block_end);
    if (first_y <= last_y) {
      rasterEdgePairZ(x0, y0, z0, x1, y1, z1,
                      x0, y0, z0, x2, y2, z2,
                      first_y, last_y);
    }
  }

  if (y2 > y1) {
    const int first_y = max(y1, block_start);
    const int last_y = min(y2, block_end);
    if (first_y <= last_y) {
      rasterEdgePairZ(x1, y1, z1, x2, y2, z2,
                      x0, y0, z0, x2, y2, z2,
                      first_y, last_y);
    }
  }
}

void ST7789_DMA::getCentroid(Obj &obj, uint8_t opcion) {

  int64_t sumX = 0;
  int64_t sumY = 0;
  int64_t sumZ = 0;

  obj.initFaceSorting();

  int numPoints = obj.mtl.f_end - obj.mtl.f_init;

  for (int i = obj.mtl.f_init;
       i < numPoints + obj.mtl.f_init;
       i++) {

    int f1 = faces[i * 6] - 1;
    int f2 = faces[i * 6 + 1] - 1;
    int f3 = faces[i * 6 + 2] - 1;

    Vector3D p = {
      vertices[f1 * 3],
      vertices[f1 * 3 + 1],
      vertices[f1 * 3 + 2]
    };

    Vector3D p1 = {
      vertices[f2 * 3],
      vertices[f2 * 3 + 1],
      vertices[f2 * 3 + 2]
    };

    Vector3D p2 = {
      vertices[f3 * 3],
      vertices[f3 * 3 + 1],
      vertices[f3 * 3 + 2]
    };

    sumX += p.x + p1.x + p2.x;
    sumY += p.y + p1.y + p2.y;
    sumZ += p.z + p1.z + p2.z;

    if (i == obj.mtl.f_init) {
      obj.collision.min.x = obj.collision.max.x = p.x;
      obj.collision.min.y = obj.collision.max.y = p.y;
      obj.collision.min.z = obj.collision.max.z = p.z;
    } else {

#define MINMAX(v) \
  if (v.x < obj.collision.min.x) obj.collision.min.x = v.x; \
  if (v.x > obj.collision.max.x) obj.collision.max.x = v.x; \
  if (v.y < obj.collision.min.y) obj.collision.min.y = v.y; \
  if (v.y > obj.collision.max.y) obj.collision.max.y = v.y; \
  if (v.z < obj.collision.min.z) obj.collision.min.z = v.z; \
  if (v.z > obj.collision.max.z) obj.collision.max.z = v.z;

      MINMAX(p);
      MINMAX(p1);
      MINMAX(p2);
    }
  }

  int64_t divisor = (int64_t)numPoints * 3;

  if (divisor > 0) {

    obj.position.x = (int32_t)(sumX / divisor);
    obj.position.y = (int32_t)(sumY / divisor);
    obj.position.z = (int32_t)(sumZ / divisor);

  } else {

    obj.position.x = (int32_t)(sumX / 3);
    obj.position.y = (int32_t)(sumY / 3);
    obj.position.z = (int32_t)(sumZ / 3);
  }

  //getObjectBoundsRadius(obj);

  /*if (obj.type == LUZPUNTUAL ||
        obj.type == LUZDIRECCIONAL ||
        obj.type == LUZSPOT) {

        luz[totalLuces] = obj.luz;

        luz[totalLuces].posicion.x = obj.position.x;
        luz[totalLuces].posicion.y = obj.position.y;
        luz[totalLuces].posicion.z = obj.position.z;

        totalLuces++;

        printf("DEtecta luz
");
        printf("%d
", totalLuces);
        printf("
");

    } else if (obj.type == AREA) {

        inicializarHumo3D(
            { obj.position.x, obj.position.y, obj.position.z },
            obj.id
        );

        total_puntos_humo++;
    }*/
}



void ST7789_DMA::getCentroidV2(Obj &obj, uint8_t opcion) {

  int64_t sumX = 0;
  int64_t sumY = 0;
  int64_t sumZ = 0;

  int numPoints = obj.mtl.f_end - obj.mtl.f_init;

  for (int i = obj.mtl.f_init;
       i < numPoints + obj.mtl.f_init;
       i++) {

    int f1 = faces[i * 6] - 1;
    int f2 = faces[i * 6 + 1] - 1;
    int f3 = faces[i * 6 + 2] - 1;

    Vector3D p = {
      (int32_t)(vertices[f1 * 3] * 1000.0f),
      (int32_t)(vertices[f1 * 3 + 1] * 1000.0f),
      (int32_t)(vertices[f1 * 3 + 2] * 1000.0f)
    };

    Vector3D p1 = {
      (int32_t)(vertices[f2 * 3] * 1000.0f),
      (int32_t)(vertices[f2 * 3 + 1] * 1000.0f),
      (int32_t)(vertices[f2 * 3 + 2] * 1000.0f)
    };

    Vector3D p2 = {
      (int32_t)(vertices[f3 * 3] * 1000.0f),
      (int32_t)(vertices[f3 * 3 + 1] * 1000.0f),
      (int32_t)(vertices[f3 * 3 + 2] * 1000.0f)
    };

    sumX += p.x + p1.x + p2.x;
    sumY += p.y + p1.y + p2.y;
    sumZ += p.z + p1.z + p2.z;

    if (i == obj.mtl.f_init) {
      obj.collision.min.x = obj.collision.max.x = p.x;
      obj.collision.min.y = obj.collision.max.y = p.y;
      obj.collision.min.z = obj.collision.max.z = p.z;
    } else {

#define MINMAX(v) \
  if (v.x < obj.collision.min.x) obj.collision.min.x = v.x; \
  if (v.x > obj.collision.max.x) obj.collision.max.x = v.x; \
  if (v.y < obj.collision.min.y) obj.collision.min.y = v.y; \
  if (v.y > obj.collision.max.y) obj.collision.max.y = v.y; \
  if (v.z < obj.collision.min.z) obj.collision.min.z = v.z; \
  if (v.z > obj.collision.max.z) obj.collision.max.z = v.z;

      MINMAX(p);
      MINMAX(p1);
      MINMAX(p2);
    }
  }

  int64_t divisor = (int64_t)numPoints * 3;

  if (divisor > 0) {

    obj.position.x = (int32_t)(sumX / divisor);
    obj.position.y = (int32_t)(sumY / divisor);
    obj.position.z = (int32_t)(sumZ / divisor);

  } else {

    obj.position.x = (int32_t)(sumX / 3);
    obj.position.y = (int32_t)(sumY / 3);
    obj.position.z = (int32_t)(sumZ / 3);
  }
}

// ============================================================================
// CULLING DE OBJETO
// ============================================================================
//
// La esfera se obtiene una sola vez recorriendo las caras del objeto.
// Esto permite descartar un objeto completo antes de transformar todas sus
// caras. La prueba es conservadora: si la esfera toca el frustum, se continúa.
//
// Nota: vertices/faces son los buffers globales usados por el proyecto.
// Las caras usan 0 como índice inválido; por eso se valida antes de hacer -1.
//


int32_t fixed_div(int32_t a, int32_t b) {
  if (b == 0) return 0;
  return (int32_t)((((int64_t)a << FIXED_SHIFT) + (b / 2)) / b);
}


inline bool cullObjectViewSphere(
  Obj &obj,
  const Matrix4x4 &MView,
  Vector3D pivotCamera,
  int32_t scale,
  bool perspective,
  int32_t clipZ) {

  const int32_t radiusLocal = obj.bounds_radius;

  // Sin geometría no hay nada que rasterizar.
  if (radiusLocal <= 0)
    return true;

  // Centro del objeto (MLocal no aplica aquí: es rotación/escala pura,
  // así que sobre el propio centro del objeto su efecto es 0).
  Vector3D center = {
    obj.position.x + obj.traslation.x,
    obj.position.y + obj.traslation.y,
    obj.position.z + obj.traslation.z
  };

  // 🛠️ FIX: estaba comentado. Sin esto, el culling ignora la rotación
  // global de la escena y calcula todo en el espacio equivocado.

  center.x -= pivotCamera.x;
  center.y -= pivotCamera.y;
  center.z -= pivotCamera.z;

  center = mul(MView, center);

  // La matriz actual usa una escala uniforme, pero hacemos la prueba
  // conservadora por si en el futuro cambia a escala no uniforme.
  int32_t maxScale = obj.scale.x;

  if (obj.scale.y > maxScale) maxScale = obj.scale.y;
  if (obj.scale.z > maxScale) maxScale = obj.scale.z;

  if (maxScale < 0) maxScale = -maxScale;

  // 🛠️ FIX: obj.scale puede llegar en 0 si esta función se llama antes de
  // que drawSceneGameObjectV2 lo fuerce a FIXED_SCALE (o si nunca se
  // inicializó en obj2[]). Sin este fallback, radius colapsaba a 1 y
  // el objeto se culeaba casi siempre, sin importar su tamaño real.
  if (maxScale <= 0) maxScale = FIXED_SCALE;

  int32_t radius = (int32_t)(((int64_t)radiusLocal * maxScale) >> FIXED_SHIFT);

  if (radius <= 0)
    radius = 1;

  // ------------------------------------------------------------
  // PLANO Z / NEAR CLIP
  // ------------------------------------------------------------
  if ((int64_t)center.z - radius > clipZ)
    return true;

  // ------------------------------------------------------------
  // CULLING LATERAL / VERTICAL DE OBJETO
  // ------------------------------------------------------------
  if (perspective) {

    const int64_t depth = -(int64_t)center.z;

    if (depth > radius) {

      const int32_t FOV =
        fixed_div(INT_TO_FIXED(180), scale);

      const int64_t projectedRadius =
        ((int64_t)radius * FOV) / depth;

      const int64_t projectedX =
        (int64_t)centerX + ((int64_t)center.x * FOV) / depth;

      const int64_t projectedY =
        (int64_t)centerY - ((int64_t)center.y * FOV) / depth;

      const int64_t left = projectedX - projectedRadius;
      const int64_t right = projectedX + projectedRadius;
      const int64_t top = projectedY - projectedRadius;
      const int64_t bottom = projectedY + projectedRadius;

      if (right < 0 || left >= lcdW || bottom < 0 || top >= lcdH) {
        return true;
      }
    }

  } else {

    const int64_t projectedRadius =
      ((int64_t)radius * scale) >> FIXED_SHIFT;

    const int64_t projectedX =
      (int64_t)centerX + (((int64_t)center.x * scale) >> FIXED_SHIFT);

    const int64_t projectedY =
      (int64_t)centerY - (((int64_t)center.y * scale) >> FIXED_SHIFT);

    if (projectedX + projectedRadius < 0 || projectedX - projectedRadius >= lcdW || projectedY + projectedRadius < 0 || projectedY - projectedRadius >= lcdH) {
      return true;
    }
  }

  return false;
}

void ST7789_DMA::configInitGame(const Vector3D &camera) {
  MLocal = buildMatrixLocalInit();
  MGlobal = buildMatrixGlobalInit();
  MView = buildMatrixCamera(camera, matIdentity());
  gLastCameraRotation = camera;
  gCameraRotationValid = true;
}

void ST7789_DMA::updateMatrizCamera(const Vector3D &camera) {
  MView = buildMatrixCamera(camera, matIdentity());
  gLastCameraRotation = camera;
  gCameraRotationValid = true;
}

void ST7789_DMA::getAABBVertices(const AABB &box, Vector3D *corners) {
  corners[0] = { box.min.x, box.min.y, box.min.z };
  corners[1] = { box.max.x, box.min.y, box.min.z };
  corners[2] = { box.max.x, box.max.y, box.min.z };
  corners[3] = { box.min.x, box.max.y, box.min.z };
  corners[4] = { box.min.x, box.min.y, box.max.z };
  corners[5] = { box.max.x, box.min.y, box.max.z };
  corners[6] = { box.max.x, box.max.y, box.max.z };
  corners[7] = { box.min.x, box.max.y, box.max.z };
}

void ST7789_DMA::drawAABB(const AABB &box, int32_t scale, int32_t centerX, int32_t centerY, bool perspectiva, uint16_t color) {
  Vector3D corners[8];
  getAABBVertices(box, corners);
  Vector2D proj[8];
  for (int i = 0; i < 8; i++) {
    proj[i] = project(corners[i], scale, centerX, centerY, perspectiva);
  }

  // Dibujar aristas
  int edges[12][2] = {
    { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 },  // base
    { 4, 5 },
    { 5, 6 },
    { 6, 7 },
    { 7, 4 },  // top
    { 0, 4 },
    { 1, 5 },
    { 2, 6 },
    { 3, 7 }  // verticales
  };

  for (int i = 0; i < 12; i++) {
    int a = edges[i][0], b = edges[i][1];
    drawLine(FIXED_TO_INT(proj[a].x),
             FIXED_TO_INT(proj[a].y),
             FIXED_TO_INT(proj[b].x),
             FIXED_TO_INT(proj[b].y), color);
  }
}




// Unión para procesar de forma nativa registros vectoriales alineados a 16 bytes
typedef union {
  int32_t v[4];
} __attribute__((aligned(16))) SimdVector;

// Forzamos la optimización por hardware respetando la estructura por columnas de tu motor
static inline Vector3D simd_mul_matrix_vector(const Matrix4x4 &M, const Vector3D &v) {
    Vector3D r;

    // Multiplicaciones y acumulaciones directas mapeadas por columnas
    // El procesador Xtensa LX7 del ESP32-S3 procesa estas instrucciones MAC consecutivas a máxima velocidad
    r.x = (int32_t)((((int64_t)M.m[0] * v.x) + ((int64_t)M.m[4] * v.y) + ((int64_t)M.m[8] * v.z)) >> FIXED_SHIFT);
    r.y = (int32_t)((((int64_t)M.m[1] * v.x) + ((int64_t)M.m[5] * v.y) + ((int64_t)M.m[9] * v.z)) >> FIXED_SHIFT);
    r.z = (int32_t)((((int64_t)M.m[2] * v.x) + ((int64_t)M.m[6] * v.y) + ((int64_t)M.m[10] * v.z)) >> FIXED_SHIFT);

    // Suma directa de los vectores de traslación puros
    r.x += M.m[12];
    r.y += M.m[13];
    r.z += M.m[14];

    return r;
}

static inline Vector3D simd_vector_sub(const Vector3D &a, const Vector3D &b) {
    Vector3D out;
    out.x = a.x - b.x;
    out.y = a.y - b.y;
    out.z = a.z - b.z;
    return out;
}

static inline Vector3D simd_vector_add(const Vector3D &a, const Vector3D &b) {
    Vector3D out;
    out.x = a.x + b.x;
    out.y = a.y + b.y;
    out.z = a.z + b.z;
    return out;
}

void prepareGameFaceNormals(const int32_t *verts, const int *faceData, uint32_t faceCount) {
  if (!verts || !faceData || faceCount == 0) {
    if (gFaceNormals) {
      free(gFaceNormals);
      gFaceNormals = nullptr;
    }
    gFaceNormalsCount = 0;
    gFaceNormalsVertices = nullptr;
    gFaceNormalsFaces = nullptr;
    return;
  }

  if (gFaceNormals &&
      gFaceNormalsCount == faceCount &&
      gFaceNormalsVertices == verts &&
      gFaceNormalsFaces == faceData) {
    return;
  }

  if (gFaceNormals) {
    free(gFaceNormals);
    gFaceNormals = nullptr;
  }

  gFaceNormals = (Vector3D *)ps_malloc(faceCount * sizeof(Vector3D));
  if (!gFaceNormals) {
    gFaceNormalsCount = 0;
    gFaceNormalsVertices = nullptr;
    gFaceNormalsFaces = nullptr;
    return;
  }

  for (uint32_t i = 0; i < faceCount; ++i) {
    const int base = (int)i * 6;
    const int f1 = (faceData[base] - 1) * 3;
    const int f2 = (faceData[base + 1] - 1) * 3;
    const int f3 = (faceData[base + 2] - 1) * 3;

    Vector3D a(verts[f1], verts[f1 + 1], verts[f1 + 2]);
    Vector3D b(verts[f2], verts[f2 + 1], verts[f2 + 2]);
    Vector3D c(verts[f3], verts[f3 + 1], verts[f3 + 2]);

    Vector3D e1 = simd_vector_sub(b, a);
    Vector3D e2 = simd_vector_sub(c, a);
    gFaceNormals[i] = normalize(cross(e1, e2));
  }

  gFaceNormalsCount = faceCount;
  gFaceNormalsVertices = verts;
  gFaceNormalsFaces = faceData;
}


void ST7789_DMA::sortLocalObjectFaces(Obj &obj, Vector3D pivotCamera) {
  if (!obj.visible || !obj.faces_allocated || obj.is_ground) return;

  int totalFaces = obj.mtl.f_end - obj.mtl.f_init;
  if (totalFaces <= 1) return;

  // Usamos un búfer temporal estático o dinámico para calcular las profundidades
  LocalFaceDepth* tempDepths = (LocalFaceDepth*)malloc(totalFaces * sizeof(LocalFaceDepth));
  if (!tempDepths) return;

  bool rotateLocal = objectHasRotation(obj);
  Matrix4x4 MLocalTrans = rotateLocal ? buildMatrixLocal(obj) : matIdentity();

  for (int i = 0; i < totalFaces; i++) {
    uint16_t globalIdx = obj.mtl.f_init + i;
    const int base = globalIdx * 6;
    const int f1 = (faces[base] - 1) * 3;
    const int f2 = (faces[base + 1] - 1) * 3;
    const int f3 = (faces[base + 2] - 1) * 3;

    // Leemos los 3 vértices
    Vector3D p  = { vertices[f1], vertices[f1 + 1], vertices[f1 + 2] };
    Vector3D p1 = { vertices[f2], vertices[f2 + 1], vertices[f2 + 2] };
    Vector3D p2 = { vertices[f3], vertices[f3 + 1], vertices[f3 + 2] };

    if (rotateLocal) {
      p  = simd_mul_matrix_vector(MLocalTrans, simd_vector_sub(p, obj.position));
      p1 = simd_mul_matrix_vector(MLocalTrans, simd_vector_sub(p1, obj.position));
      p2 = simd_mul_matrix_vector(MLocalTrans, simd_vector_sub(p2, obj.position));
      p  = simd_vector_add(p, obj.position); p1 = simd_vector_add(p1, obj.position); p2 = simd_vector_add(p2, obj.position);
    }

    p = simd_vector_add(p, obj.traslation); p1 = simd_vector_add(p1, obj.traslation); p2 = simd_vector_add(p2, obj.traslation);
    p = simd_vector_sub(p, pivotCamera); p1 = simd_vector_sub(p1, pivotCamera); p2 = simd_vector_sub(p2, pivotCamera);
    p = simd_mul_matrix_vector(MView, p); p1 = simd_mul_matrix_vector(MView, p1); p2 = simd_mul_matrix_vector(MView, p2);

    // Guardamos el índice global y el Z promedio
    int64_t distSq = (int64_t)p.x * p.x + (int64_t)p.y * p.y + (int64_t)p.z * p.z;
    tempDepths[i].globalIndex = globalIdx;
    tempDepths[i].z = (int32_t)(distSq / 100000);
  }

  // Ordenamos las caras locales de atrás hacia adelante usando ShellSort
  for (int gap = totalFaces / 2; gap > 0; gap /= 2) {
    for (int i = gap; i < totalFaces; i++) {
      LocalFaceDepth temp = tempDepths[i];
      int j;
      
      // 🛠️ FIX DE ORIENTACIÓN: Cambiamos '<' por '>' 
      // Esto obliga a que las caras con el Z más negativo (las más lejanas en tu motor) 
      // se coloquen al inicio de la lista para que sean dibujadas primero, 
      // permitiendo que las caras del frente (Z mayor) se pinten encima de ellas.
      for (j = i; j >= gap && tempDepths[j - gap].z < temp.z; j -= gap) {
          tempDepths[j] = tempDepths[j - gap];
      }
      tempDepths[j] = temp;
    }
  }

  // Guardamos el orden de índices optimizado en la estructura del objeto
  for (int i = 0; i < totalFaces; i++) {
    obj.sorted_face_indices[i] = tempDepths[i].globalIndex;
  }

  free(tempDepths);
}

void ST7789_DMA::drawShadowv2(Obj &miObjeto, Vector3D pivotCamera, int32_t fov_precalculado){
  if (!miObjeto.is_ground && miObjeto.visible) {
    // Calculamos dónde cae la sombra proyectando el centroide del objeto al suelo
    // GROUND_Y = 1000 (según tu código base). Escalamos y proyectamos la posición
    Vector3D shadowPos = { miObjeto.position.x + miObjeto.traslation.x, 1000, miObjeto.position.z + miObjeto.traslation.z };
    
    // Convertir al espacio de la cámara y proyectar a la pantalla
    shadowPos.x -= pivotCamera.x; 
    shadowPos.y -= pivotCamera.y; 
    shadowPos.z -= pivotCamera.z;
    Vector3D camSpaceShadow = mul(MView, shadowPos);
    
    if (camSpaceShadow.z < 0) {
        Vector2D screenShadow = project(camSpaceShadow, INT_TO_FIXED(1), centerX, centerY, fov_precalculado, false);
        
        // Dibujamos un círculo de sombra (Ajusta el radio '15' según el tamaño de tu objeto)
        uint16_t SHADOW_COLOR = 0x18C3; // Gris oscuro
        fillCircle(FIXED_TO_INT(screenShadow.x), FIXED_TO_INT(screenShadow.y), 5, SHADOW_COLOR, 120);
    }
  }
}


static inline bool shouldRenderLODface(
    int faceOrder,
    int totalFaces,
    uint8_t lod)
{
    if (lod == 0)
        return true;

    if (totalFaces <= GAME_LOD_MIN_FACES)
        return true;

    if (lod == 1)
        return (faceOrder & 1) == 0;       // 50%

    if (lod == 2)
        return (faceOrder % 3) == 0;       // 33%

    return (faceOrder % 6) == 0;           // ~17%
}
/*static inline bool shouldRenderLODface(
  int faceOrder,
  int totalFaces,
  uint8_t lod)
{
  if (lod == 0)
    return true;

  // Modelos muy pequeños pueden desaparecer visualmente si reducimos
  // demasiado su geometría.
  if (totalFaces <= GAME_LOD_MIN_FACES)
    return true;

  int divisor = GAME_LOD1_FACE_DIV;

  if (lod == 2)
    divisor = GAME_LOD2_FACE_DIV;
  else if (lod >= 3)
    divisor = GAME_LOD3_FACE_DIV;

  // Distribución uniforme de caras en lugar de quedarse solo con las
  // primeras caras del OBJ.
  //
  // Esto produce aproximadamente:
  // LOD1 -> 50%
  // LOD2 -> 33%
  // LOD3 -> 17%
  return (faceOrder % divisor) == 0;
}*/

// ============================================================================
// LOD AUTOMÁTICO: distancia + tamaño proyectado
// ============================================================================

static inline int32_t lodObjectRadius(const Obj &obj)
{
  // bounds_radius puede ser usado por versiones futuras que inicialicen
  // explícitamente esta caché.
  if (obj.bounds_radius > 0)
    return obj.bounds_radius;

  // En este proyecto getCentroid() ya calcula collision.min/max para cada
  // objeto BODY. Aprovechamos esos datos sin recorrer nuevamente las caras.
  const int64_t dx = (int64_t)obj.collision.max.x - obj.collision.min.x;
  const int64_t dy = (int64_t)obj.collision.max.y - obj.collision.min.y;
  const int64_t dz = (int64_t)obj.collision.max.z - obj.collision.min.z;

  int64_t maxd = dx;
  if (dy > maxd) maxd = dy;
  if (dz > maxd) maxd = dz;

  if (maxd <= 0)
    return 1000; // fallback conservador

  return (int32_t)(maxd >> 1);
}

static inline uint8_t getAutomaticObjectLOD(
  const Obj &obj,
  const Vector3D &pivotCamera,
  int32_t scale,
  bool perspective)
{
#if !GAME_LOD_ENABLED
  (void)obj;
  (void)pivotCamera;
  (void)scale;
  (void)perspective;
  return 0;
#else
  if (!obj.auto_lod)
    return 0;

  if (obj.lod_override <= 3)
    return obj.lod_override;

  // El terreno normalmente contiene miles de triángulos que deben
  // conservarse completos para no producir agujeros.
  if (obj.is_ground)
    return 0;

  const int64_t dx =
    (int64_t)obj.position.x + obj.traslation.x - pivotCamera.x;
  const int64_t dy =
    (int64_t)obj.position.y + obj.traslation.y - pivotCamera.y;
  const int64_t dz =
    (int64_t)obj.position.z + obj.traslation.z - pivotCamera.z;

  const int64_t distSq = dx * dx + dy * dy + dz * dz;

  // Nivel determinado por distancia.
  uint8_t lodDistance = 0;
  const int64_t d1 = (int64_t)GAME_LOD_DISTANCE_1 * GAME_LOD_DISTANCE_1;
  const int64_t d2 = (int64_t)GAME_LOD_DISTANCE_2 * GAME_LOD_DISTANCE_2;
  const int64_t d3 = (int64_t)GAME_LOD_DISTANCE_3 * GAME_LOD_DISTANCE_3;

  if (distSq >= d3)
    lodDistance = 3;
  else if (distSq >= d2)
    lodDistance = 2;
  else if (distSq >= d1)
    lodDistance = 1;

  // Tamaño proyectado aproximado del objeto.
  // Solo hacemos una transformación del centro + una división.
  uint8_t lodPixels = 0;

  const int32_t radiusLocal = lodObjectRadius(obj);

  int32_t maxScale = obj.scale.x;
  if (obj.scale.y > maxScale) maxScale = obj.scale.y;
  if (obj.scale.z > maxScale) maxScale = obj.scale.z;
  if (maxScale < 0) maxScale = -maxScale;
  if (maxScale <= 0) maxScale = FIXED_SCALE;

  const int32_t radius =
    (int32_t)(((int64_t)radiusLocal * maxScale) >> FIXED_SHIFT);

  if (radius > 0) {
    Vector3D center = {
      (int32_t)dx,
      (int32_t)dy,
      (int32_t)dz
    };

    center = mul(MView, center);

    int64_t projectedRadius = 0;

    if (perspective) {
      int64_t depth = -(int64_t)center.z;

      // Si está detrás/casi sobre el plano cercano, no dejamos que una
      // división problemática determine el LOD.
      if (depth > 1) {
        const int32_t fov =
          fixed_div(INT_TO_FIXED(180), scale);

        projectedRadius =
          ((int64_t)radius * fov) / depth;
      }
    } else {
      projectedRadius =
        ((int64_t)radius * scale) >> FIXED_SHIFT;
    }

    if (projectedRadius < 0)
      projectedRadius = -projectedRadius;

    const int64_t diameter = projectedRadius * 2;

    if (diameter <= GAME_LOD_PIXELS_3)
      lodPixels = 3;
    else if (diameter <= GAME_LOD_PIXELS_2)
      lodPixels = 2;
    else if (diameter <= GAME_LOD_PIXELS_1)
      lodPixels = 1;
  }

  // Si cualquiera de los dos criterios indica que el objeto ya puede
  // simplificarse, aplicamos ese nivel. Esto es importante para el ESP32-S3:
  // un árbol que está lejos pero todavía ocupa muchos píxeles, por ejemplo,
  // puede seguir usando LOD1/LOD2 en lugar de procesar sus cientos de caras.
  uint8_t lod = (lodDistance > lodPixels) ? lodDistance : lodPixels;

  return lod;
#endif
}



void ST7789_DMA::drawSceneGameObject(
  Obj &obj, int32_t anglex, int32_t angley, int32_t anglez,
  Vector3D angleCamera, Vector3D pivotCamera, bool select,
  int32_t scale, int32_t origenxx, int32_t origenyy, int32_t origenzz, int32_t clipZ) {

  if (!obj.visible || obj.type != BODY) return;
  if (obj.onloop) obj.onloop(&obj);
  if (!obj.visible || obj.mtl.f_init < 0 || obj.mtl.f_end <= obj.mtl.f_init) return;

  if(!obj.is_ground){
    // 🛠️ OPTIMIZACIÓN 2: Culling posicional del objeto completo en enteros puros
    int64_t dxobj = (int64_t)(obj.position.x + obj.traslation.x - pivotCamera.x);
    int64_t dyobj = (int64_t)(obj.position.y + obj.traslation.y - pivotCamera.y);
    int64_t dzobj = (int64_t)(obj.position.z + obj.traslation.z - pivotCamera.z);
    if ((dxobj * dxobj + dyobj * dyobj + dzobj * dzobj) > 625000000LL) return;
  }
  const int32_t fov_precalculado = fixed_div(INT_TO_FIXED(180), scale);
  //drawShadowv2(obj, pivotCamera, fov_precalculado);

  // La rotación de cámara es parte del espacio de vista.
  // angleCamera antes se recibía aquí pero no se aplicaba; ahora se actualiza
  // solo cuando cambia para no penalizar el rendimiento por objeto.
  ensureCameraRotation(angleCamera);

  bool rotateGlobal = false, rotateLocal = false;
  if (objectHasGlobalRotation(obj) || anglex != 0 || angley != 0 || anglez != 0) {
    MGlobal = buildMatrixGlobal(obj, anglex, angley, anglez);
    rotateGlobal = true;
  }
  if (objectHasRotation(obj)) {
    MLocal = buildMatrixLocal(obj);
    rotateLocal = true;
  }

  const int32_t clz = -100;
  

  int totalFaces = obj.mtl.f_end - obj.mtl.f_init;
  const uint8_t autoLOD =
    getAutomaticObjectLOD(obj, pivotCamera, scale, select);
  obj.lod_current = autoLOD;

  // 🛠️ REESTRUCTURACIÓN DEL BUCLE: Iteramos usando el mapeo de caras ordenadas
  for (int f = 0; f < totalFaces; ++f) {
    
    // Si el búfer ordenado local existe, extraemos el índice precalculado; si no, leemos secuencial (fallback seguro)
    const int i = (obj.faces_allocated && obj.sorted_face_indices) ? obj.sorted_face_indices[f] : (obj.mtl.f_init + f);

    // LOD: descartamos caras antes de leer sus vértices. Así el ahorro
    // ocurre antes de la transformación SIMD, iluminación y rasterización.
    if (!shouldRenderLODface(f, totalFaces, autoLOD))
      continue;

    const int base = i * 6;
    const int f1 = (faces[base] - 1) * 3; const int f2 = (faces[base + 1] - 1) * 3; const int f3 = (faces[base + 2] - 1) * 3;
    const uint16_t color = faces[base + 3];

    // [Aquí se mantiene EXACTAMENTE toda la matemática de transformación, culling 2D y fillTriangle que ya optimizamos antes]
    Vector3D p  = { (int32_t)(verticesF[f1] * 1000.0f), (int32_t)(verticesF[f1 + 1] * 1000.0f), (int32_t)(verticesF[f1 + 2] * 1000.0f) };
    Vector3D p1 = { (int32_t)(verticesF[f2] * 1000.0f), (int32_t)(verticesF[f2 + 1] * 1000.0f), (int32_t)(verticesF[f2 + 2] * 1000.0f) };
    Vector3D p2 = { (int32_t)(verticesF[f3] * 1000.0f), (int32_t)(verticesF[f3 + 1] * 1000.0f), (int32_t)(verticesF[f3 + 2] * 1000.0f) };

    if (rotateLocal) {
      p  = simd_mul_matrix_vector(MLocal, simd_vector_sub(p, obj.position));
      p1 = simd_mul_matrix_vector(MLocal, simd_vector_sub(p1, obj.position));
      p2 = simd_mul_matrix_vector(MLocal, simd_vector_sub(p2, obj.position));
      p  = simd_vector_add(p, obj.position); p1 = simd_vector_add(p1, obj.position); p2 = simd_vector_add(p2, obj.position);
    }

    p  = simd_vector_add(p, obj.traslation); p1 = simd_vector_add(p1, obj.traslation); p2 = simd_vector_add(p2, obj.traslation);
    if (rotateGlobal) { p  = simd_mul_matrix_vector(MGlobal, p); p1 = simd_mul_matrix_vector(MGlobal, p1); p2 = simd_mul_matrix_vector(MGlobal, p2); }

    Vector3D worldP = p;

    p  = simd_vector_sub(p, pivotCamera); p1 = simd_vector_sub(p1, pivotCamera); p2 = simd_vector_sub(p2, pivotCamera);
    p  = simd_mul_matrix_vector(MView, p); p1 = simd_mul_matrix_vector(MView, p1); p2 = simd_mul_matrix_vector(MView, p2);

    int64_t distSq = (int64_t)p.x * p.x + (int64_t)p.y * p.y + (int64_t)p.z * p.z;
    if (distSq > 625000000LL) { if (obj.is_ground) continue; else return; }

    if (p.z > clz || p1.z > clz || p2.z > clz) continue;

    Vector2D a2 = project(p,  scale, centerX, centerY, fov_precalculado, true);
    Vector2D b2 = project(p1, scale, centerX, centerY, fov_precalculado, true);
    Vector2D c2 = project(p2, scale, centerX, centerY, fov_precalculado, true);

    if ((a2.x < 0 && b2.x < 0 && c2.x < 0) || (a2.x >= lcdW && b2.x >= lcdW && c2.x >= lcdW) || (a2.y < 0 && b2.y < 0 && c2.y < 0) || (a2.y >= lcdH && b2.y >= lcdH && c2.y >= lcdH)) continue;
    
    const int64_t area2 = (int64_t)(b2.x - a2.x) * (c2.y - a2.y) - (int64_t)(b2.y - a2.y) * (c2.x - a2.x);
    if (area2 <= 0) continue;

    // FASE 3: usar la normal local precalculada. Solo se transforma con
    // las matrices de rotacion del objeto; no se recalcula cross/normalize.
    Vector3D worldNormal;
    const bool uniformLocalScale =
      obj.scale.x == obj.scale.y && obj.scale.y == obj.scale.z;

    if (gFaceNormals &&
        (uint32_t)i < gFaceNormalsCount &&
        gFaceNormalsVertices == vertices &&
        gFaceNormalsFaces == faces &&
        uniformLocalScale) {

      worldNormal = gFaceNormals[i];

      if (rotateLocal)
        worldNormal = transformNormal3x3(MLocal, worldNormal);

      if (rotateGlobal)
        worldNormal = transformNormal3x3(MGlobal, worldNormal);
    } else {
      // Fallback seguro para geometria sin cache o escala no uniforme.
      worldNormal = cross(
        simd_vector_sub(p1, p),
        simd_vector_sub(p2, p)
      );
    }
    

    const uint16_t litColor = calcularIluminacionJuego(obj, worldP, worldNormal, pivotCamera, color);

    if (z_buffer != NULL && zbuffering) {
      fillTriangleZ(FIXED_TO_INT(a2.x), FIXED_TO_INT(a2.y), -p.z, FIXED_TO_INT(b2.x), FIXED_TO_INT(b2.y), -p1.z, FIXED_TO_INT(c2.x), FIXED_TO_INT(c2.y), -p2.z, litColor);
    } else {
      fillTriangle(FIXED_TO_INT(a2.x), FIXED_TO_INT(a2.y), FIXED_TO_INT(b2.x), FIXED_TO_INT(b2.y), FIXED_TO_INT(c2.x), FIXED_TO_INT(c2.y), litColor);
    }
  }
}


void ST7789_DMA::drawSceneGameObjectV2(
  Obj &obj, int32_t anglex, int32_t angley, int32_t anglez,
  Vector3D angleCamera, Vector3D pivotCamera, bool select,
  int32_t scale, int32_t origenxx, int32_t origenyy, int32_t origenzz, int32_t clipZ) {

  if (!obj.visible || obj.type != BODY) return;
  if (obj.onloop) obj.onloop(&obj);
  if (!obj.visible || obj.mtl.f_init < 0 || obj.mtl.f_end <= obj.mtl.f_init) return;

  if(!obj.is_ground){
    // 🛠️ OPTIMIZACIÓN 2: Culling posicional del objeto completo en enteros puros
    int64_t dxobj = (int64_t)(obj.position.x + obj.traslation.x - pivotCamera.x);
    int64_t dyobj = (int64_t)(obj.position.y + obj.traslation.y - pivotCamera.y);
    int64_t dzobj = (int64_t)(obj.position.z + obj.traslation.z - pivotCamera.z);
    if ((dxobj * dxobj + dyobj * dyobj + dzobj * dzobj) > 625000000LL) return;
  }

  // La rotación de cámara es parte del espacio de vista.
  // angleCamera antes se recibía aquí pero no se aplicaba; ahora se actualiza
  // solo cuando cambia para no penalizar el rendimiento por objeto.
  ensureCameraRotation(angleCamera);

  bool rotateGlobal = false, rotateLocal = false;
  if (objectHasGlobalRotation(obj) || anglex != 0 || angley != 0 || anglez != 0) {
    MGlobal = buildMatrixGlobal(obj, anglex, angley, anglez);
    rotateGlobal = true;
  }
  if (objectHasRotation(obj)) {
    MLocal = buildMatrixLocal(obj);
    rotateLocal = true;
  }

  const int32_t clz = -100;
  
  const int32_t fov_precalculado = fixed_div(INT_TO_FIXED(180), scale);

  // LOD se calcula una sola vez por objeto, nunca dentro del bucle de caras.
  const uint8_t autoLOD =
    getAutomaticObjectLOD(obj, pivotCamera, scale, select);
  obj.lod_current = autoLOD;

  const float lightDirX = (float)(SUN_DIRECTION_WORLD.x / FIXED_SCALE);
  const float lightDirZ = (float)(SUN_DIRECTION_WORLD.z / FIXED_SCALE);
  const int32_t GROUND_Y = 1000;
  const uint16_t SHADOW_COLOR = 0x18C3;

  int totalFaces = obj.mtl.f_end - obj.mtl.f_init;

  // 🛠️ REESTRUCTURACIÓN DEL BUCLE: Iteramos usando el mapeo de caras ordenadas
  for (int f = 0; f < totalFaces; ++f) {

    if (!shouldRenderLODface(f, totalFaces, autoLOD))
        continue;
    
    // Si el búfer ordenado local existe, extraemos el índice precalculado; si no, leemos secuencial (fallback seguro)
    const int i = (obj.faces_allocated && obj.sorted_face_indices) ? obj.sorted_face_indices[f] : (obj.mtl.f_init + f);

    const int base = i * 6;
    const int f1 = (faces[base] - 1) * 3; const int f2 = (faces[base + 1] - 1) * 3; const int f3 = (faces[base + 2] - 1) * 3;
    const uint16_t color = faces[base + 3];

    // [Aquí se mantiene EXACTAMENTE toda la matemática de transformación, culling 2D y fillTriangle que ya optimizamos antes]
    Vector3D p  = { vertices[f1], vertices[f1 + 1], vertices[f1 + 2] };
    Vector3D p1 = { vertices[f2], vertices[f2 + 1], vertices[f2 + 2] };
    Vector3D p2 = { vertices[f3], vertices[f3 + 1], vertices[f3 + 2] };

    if (rotateLocal) {
      p  = simd_mul_matrix_vector(MLocal, simd_vector_sub(p, obj.position));
      p1 = simd_mul_matrix_vector(MLocal, simd_vector_sub(p1, obj.position));
      p2 = simd_mul_matrix_vector(MLocal, simd_vector_sub(p2, obj.position));
      p  = simd_vector_add(p, obj.position); p1 = simd_vector_add(p1, obj.position); p2 = simd_vector_add(p2, obj.position);
    }

    p  = simd_vector_add(p, obj.traslation); p1 = simd_vector_add(p1, obj.traslation); p2 = simd_vector_add(p2, obj.traslation);
    if (rotateGlobal) { p  = simd_mul_matrix_vector(MGlobal, p); p1 = simd_mul_matrix_vector(MGlobal, p1); p2 = simd_mul_matrix_vector(MGlobal, p2); }

    if (!obj.is_ground && showShadow) {
      Vector3D s = p;
      Vector3D s1 = p1;
      Vector3D s2 = p2;
      float h0 = (float)((s.y - GROUND_Y) / 1000.0f);
      s.y = GROUND_Y;
      s.x -= (int32_t)(h0 * lightDirX * 1000.0f);
      s.z -= (int32_t)(h0 * lightDirZ * 1000.0f);
      float h1 = (float)((s1.y - GROUND_Y) / 1000.0f);
      s1.y = GROUND_Y;
      s1.x -= (int32_t)(h1 * lightDirX * 1000.0f);
      s1.z -= (int32_t)(h1 * lightDirZ * 1000.0f);
      float h2 = ((float)(s2.y - GROUND_Y) / 1000.0f);
      s2.y = GROUND_Y;
      s2.x -= (int32_t)(h2 * lightDirX * 1000.0f);
      s2.z -= (int32_t)(h2 * lightDirZ * 1000.0f);
      s = simd_vector_sub(s, pivotCamera);
      s1 = simd_vector_sub(s1, pivotCamera);
      s2 = simd_vector_sub(s2, pivotCamera);
      s = simd_mul_matrix_vector(MView, s);
      s1 = simd_mul_matrix_vector(MView, s1);
      s2 = simd_mul_matrix_vector(MView, s2);
      if (!(s.z > clz || s1.z > clz || s2.z > clz)) {
        Vector2D sa2 = project(s, scale, centerX, centerY, fov_precalculado, select);
        Vector2D sb2 = project(s1, scale, centerX, centerY, fov_precalculado, select);
        Vector2D sc2 = project(s2, scale, centerX, centerY, fov_precalculado, select);
        if (!((sa2.x < 0 && sb2.x < 0 && sc2.x < 0) || (sa2.x >= lcdW && sb2.x >= lcdW && sc2.x >= lcdW) || (sa2.y < 0 && sb2.y < 0 && sc2.y < 0) || (sa2.y >= lcdH && sb2.y >= lcdH && sc2.y >= lcdH))) {
          if (((int64_t)(sb2.x - sa2.x) * (sc2.y - sa2.y) - (int64_t)(sb2.y - sa2.y) * (sc2.x - sa2.x)) >= 0) {
            fillTriangle(FIXED_TO_INT(sa2.x), FIXED_TO_INT(sa2.y), FIXED_TO_INT(sb2.x), FIXED_TO_INT(sb2.y), FIXED_TO_INT(sc2.x), FIXED_TO_INT(sc2.y), SHADOW_COLOR, 120);
          }
        }
      }
    } 

    // FASE 4: no calcular la normal todavía. Primero descartamos el triángulo
    // con las pruebas baratas de cámara/distancia/pantalla/back-face.
    Vector3D worldP = p;
    
    p  = simd_vector_sub(p, pivotCamera); p1 = simd_vector_sub(p1, pivotCamera); p2 = simd_vector_sub(p2, pivotCamera);
    p  = simd_mul_matrix_vector(MView, p); p1 = simd_mul_matrix_vector(MView, p1); p2 = simd_mul_matrix_vector(MView, p2);

    int64_t distSq = (int64_t)p.x * p.x + (int64_t)p.y * p.y + (int64_t)p.z * p.z;
    //if (distSq > 625000000LL) { if (obj.is_ground) continue; else return; }
    if (distSq > 625000000LL) { continue; }

    if (p.z > clz || p1.z > clz || p2.z > clz) continue;

    Vector2D a2 = project(p,  scale, centerX, centerY, fov_precalculado, true);
    Vector2D b2 = project(p1, scale, centerX, centerY, fov_precalculado, true);
    Vector2D c2 = project(p2, scale, centerX, centerY, fov_precalculado, true);

    if ((a2.x < 0 && b2.x < 0 && c2.x < 0) || (a2.x >= lcdW && b2.x >= lcdW && c2.x >= lcdW) || (a2.y < 0 && b2.y < 0 && c2.y < 0) || (a2.y >= lcdH && b2.y >= lcdH && c2.y >= lcdH)) continue;
    
    const int64_t area2 = (int64_t)(b2.x - a2.x) * (c2.y - a2.y) - (int64_t)(b2.y - a2.y) * (c2.x - a2.x);
    if (area2 >= 0) continue;

    // FASE 4: solo las caras que realmente llegarán al rasterizador
    // necesitan calcular su normal para iluminación.
    
    // FASE 3: usar la normal local precalculada. Solo se transforma con
    // las matrices de rotacion del objeto; no se recalcula cross/normalize.
    Vector3D worldNormal;
    const bool uniformLocalScale =
      obj.scale.x == obj.scale.y && obj.scale.y == obj.scale.z;

    if (gFaceNormals &&
        (uint32_t)i < gFaceNormalsCount &&
        gFaceNormalsVertices == vertices &&
        gFaceNormalsFaces == faces &&
        uniformLocalScale) {

      worldNormal = gFaceNormals[i];

      if (rotateLocal)
        worldNormal = transformNormal3x3(MLocal, worldNormal);

      if (rotateGlobal)
        worldNormal = transformNormal3x3(MGlobal, worldNormal);
    } else {
      // Fallback seguro para geometria sin cache o escala no uniforme.
      worldNormal = cross(
        simd_vector_sub(p1, p),
        simd_vector_sub(p2, p)
      );
    }

    const uint16_t litColor = calcularIluminacionJuego(obj, worldP, worldNormal, pivotCamera, color);

    if (z_buffer != NULL && zbuffering) {
      fillTriangleZ(FIXED_TO_INT(a2.x), FIXED_TO_INT(a2.y), -p.z, FIXED_TO_INT(b2.x), FIXED_TO_INT(b2.y), -p1.z, FIXED_TO_INT(c2.x), FIXED_TO_INT(c2.y), -p2.z, litColor);
    } else {
      fillTriangle(FIXED_TO_INT(a2.x), FIXED_TO_INT(a2.y), FIXED_TO_INT(b2.x), FIXED_TO_INT(b2.y), FIXED_TO_INT(c2.x), FIXED_TO_INT(c2.y), litColor);
    }
  }
}

// ============================================================================
// V3
// ============================================================================
// En la versión actual del proyecto no había una implementación V3 separada.
// Se expone como API compatible y utiliza el pipeline V2, que ahora incluye
// LOD automático. De esta forma cualquier llamada existente a V3 obtiene
// exactamente la misma optimización sin duplicar el código de rasterización.
void ST7789_DMA::drawSceneGameObjectV3(
  Obj &obj, int32_t anglex, int32_t angley, int32_t anglez,
  Vector3D angleCamera, Vector3D pivotCamera, bool select,
  int32_t scale, int32_t origenxx, int32_t origenyy,
  int32_t origenzz, int32_t clipZ)
{
  drawSceneGameObjectV2(
    obj,
    anglex, angley, anglez,
    angleCamera, pivotCamera,
    select, scale,
    origenxx, origenyy, origenzz, clipZ
  );
}


inline uint16_t rgb8_to_rgb565(uint8_t r, uint8_t g, uint8_t b) {
  uint16_t color = 0;

  // Red: 5 bits
  color |= ((r >> 3) & 0x1F) << 11;

  // Green: 6 bits
  color |= ((g >> 2) & 0x3F) << 5;

  // Blue: 5 bits
  color |= (b >> 3) & 0x1F;

  return ((r << 11) | (g << 5) | b);
}

inline float clamp(float valor, float minVal, float maxVal) {
  if (valor < minVal) return minVal;
  if (valor > maxVal) return maxVal;
  return valor;
}

inline uint8_t rgb565_to_r(int color) {
  uint8_t r = (color >> 11) & 0x1F;
  //r = (r * 255) / 31;
  return r;  // Devuelve el color en formato RGB565
}

inline uint8_t rgb565_to_g(int color) {
  uint8_t g = (color >> 5) & 0x3F;
  //g = (g * 255) / 63;
  return g;  // Devuelve el color en formato RGB565
}

inline uint8_t rgb565_to_b(int color) {
  uint8_t b = color & 0x1F;
  //b = (b * 255) / 31;
  return b;  // Devuelve el color en formato RGB565
}

inline uint16_t escalarColor2(uint16_t color, float intensidad) {
  intensidad = clamp(intensidad, 0.0f, 1.0f);

  uint8_t r = rgb565_to_r(color) * intensidad;
  uint8_t g = rgb565_to_g(color) * intensidad;
  uint8_t b = rgb565_to_b(color) * intensidad;

  return rgb8_to_rgb565(r, g, b);
}



/*void ST7789_DMA::drawSceneSpriteGame(
    LCDSprite &spr,
    int x0,
    int y0,
    int offsetx,
    int offsety,
    float intensidad)
{
    if (!spr.visible)
        return;

    uint16_t *dst = draw_buffer;

    int iniciox = x0 - offsetx;
    int inicioy = y0 - offsety + 25 - spr.height;

    for (int j = 0; j < spr.height; j++)
    {
        int py = inicioy + j;

        // Fuera verticalmente
        if (py < 0 || py >= LCD_HEIGHT)
            continue;

        for (int i = 0; i < spr.width; i++)
        {
            int px = iniciox + i;

            // Fuera horizontalmente
            if (px < 0 || px >= LCD_WIDTH)
                continue;

            uint16_t color =
                sprites[
                    (SPRITES_WIDTH * spr.y) +
                    spr.x +
                    (j * SPRITES_WIDTH) +
                    i
                ];

            // Transparencia
            if (color == 0xFFFF)
                continue;

            if(intensidad < 0.0f) intensidad = 0.0f;
            if(intensidad > 1.0f) intensidad = 1.0f;
            color = escalarColor2(color, intensidad);
            dst[
                py * LCD_WIDTH +
                px
            ] = __builtin_bswap16(color);
        }
    }
}*/


void ST7789_DMA::drawSceneSpriteGame(
  LCDSprite &spr,
  int x0,
  int y0,
  int offsetx,
  int offsety,
  float intensidad,
  float escala) {
  if (!spr.visible)
    return;

  // Evitar escalas inválidas
  if (escala <= 0.0f)
    return;

  // Limitar intensidad
  if (intensidad < 0.0f)
    intensidad = 0.0f;

  if (intensidad > 1.0f)
    intensidad = 1.0f;

  uint16_t *dst = draw_buffer;

  // Tamaño escalado
  int anchoEscalado = (int)(spr.width * escala);
  int altoEscalado = (int)(spr.height * escala);

  if (anchoEscalado <= 0 || altoEscalado <= 0)
    return;

  // Posición inicial.
  // Se mantiene el comportamiento original respecto al offset.
  int iniciox = x0 - offsetx;
  int inicioy = y0 - offsety + 25 * escala - altoEscalado;

  for (int j = 0; j < altoEscalado; j++) {
    int py = inicioy + j;

    // Fuera verticalmente
    if (py < 0 || py >= LCD_HEIGHT)
      continue;

    // Coordenada Y original del sprite
    int srcY = (int)(j / escala);

    if (srcY < 0)
      srcY = 0;

    if (srcY >= spr.height)
      srcY = spr.height - 1;

    for (int i = 0; i < anchoEscalado; i++) {
      int px = iniciox + i;

      // Fuera horizontalmente
      if (px < 0 || px >= LCD_WIDTH)
        continue;

      // Coordenada X original del sprite
      int srcX = (int)(i / escala);

      if (srcX < 0)
        srcX = 0;

      if (srcX >= spr.width)
        srcX = spr.width - 1;

      uint16_t color =
        sprites[(SPRITES_WIDTH * spr.y) + spr.x + (srcY * SPRITES_WIDTH) + srcX];

      // Transparencia
      if (color >= 0xf7be)
        continue;

      // Intensidad
      color = escalarColor2(color, intensidad);

      dst[py * LCD_WIDTH + px] = __builtin_bswap16(color);
    }
  }
}



void ST7789_DMA::drawSceneSpriteGameV2(
  LCDSprite &spr,
  int x0,
  int y0,
  int offsetx,
  int offsety,
  float intensidad,
  float escala) {
  if (!spr.visible)
    return;

  // Evitar escalas inválidas
  if (escala <= 0.0f)
    return;

  // Limitar intensidad
  if (intensidad < 0.0f)
    intensidad = 0.0f;

  if (intensidad > 1.0f)
    intensidad = 1.0f;

  uint16_t *dst = draw_buffer;

  // Tamaño escalado
  int anchoEscalado = (int)(spr.width * escala);
  int altoEscalado = (int)(spr.height * escala);

  if (anchoEscalado <= 0 || altoEscalado <= 0)
    return;

  // Posición inicial.
  // Se mantiene el comportamiento original respecto al offset.
  int iniciox = x0 - offsetx;
  int inicioy = y0 - offsety + 25 * escala - altoEscalado;

  for (int j = 0; j < altoEscalado; j++) {
    int py = inicioy + j;

    // Fuera verticalmente
    if (py < 0 || py >= LCD_HEIGHT)
      continue;

    // Coordenada Y original del sprite
    int srcY = (int)(j / escala);

    if (srcY < 0)
      srcY = 0;

    if (srcY >= spr.height)
      srcY = spr.height - 1;

    for (int i = 0; i < anchoEscalado; i++) {
      int px = iniciox + i;

      // Fuera horizontalmente
      if (px < 0 || px >= LCD_WIDTH)
        continue;

      // Coordenada X original del sprite
      int srcX = (int)(i / escala);

      if (srcX < 0)
        srcX = 0;

      if (srcX >= spr.width)
        srcX = spr.width - 1;

      uint16_t color =
        sprites[(SPRITES_WIDTH * spr.y) + spr.x + (srcY * SPRITES_WIDTH) + srcX];

      // Transparencia
      if (color >= 0xf7be)
        continue;

      // Intensidad
      color = escalarColor2(color, intensidad);

      dst[py * LCD_WIDTH + px] = __builtin_bswap16(color);
    }
    escala += (0.6f / 240.0f);
    anchoEscalado = (int)(spr.width * escala);
  }
}
