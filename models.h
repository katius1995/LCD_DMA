#include "st7789_dma.h"

// Geometría patrón de un cubo unitario básico (Radio 10 píxeles nativos)
const Vector3D cube_v[] = {
    {-10, -10, -10}, { 10, -10, -10}, { 10,  10, -10}, {-10,  10, -10},
    {-10, -10,  10}, { 10, -10,  10}, { 10,  10,  10}, {-10,  10,  10}
};

const Face3D cube_f[] = {
    {0, 1, 2}, {0, 2, 3}, // Frente
    {1, 5, 6}, {1, 6, 2}, // Derecha
    {5, 4, 7}, {5, 7, 6}, // Atrás
    {4, 0, 3}, {4, 3, 7}, // Izquierda
    {3, 2, 6}, {3, 6, 7}, // Abajo
    {4, 5, 1}, {4, 1, 0}  // Arriba
};

const Model3D mesh_cube = { cube_v, 8, cube_f, 12 };

// Estructura de instancia para el control independiente de cada cubo en el motor
typedef struct {
    Vector3D position;     // Ubicación en el espacio 3D
    Vector3D rotation;     // Ángulos de rotación actuales
    Vector3D rot_speed;    // Velocidad de giro en cada eje (grados por frame)
    int32_t scale_fixed;   // Escala de tamaño en Punto Fijo
    uint16_t color;        // Color RGB565 único asignado
} CubeInstance;

#define MAX_CUBES 20
CubeInstance cubes[MAX_CUBES];