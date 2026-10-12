
#include "mapa_pueblo.h"

// Escena de prueba independiente para la malla OBJ trees_B_large.
// No modifica Batalla Naval ni Supervivencia.

extern ST7789_DMA lcd;
extern MandoControl mC;
extern WatchState watch_screen;

namespace PRUEBAS3D_GAME {

using ::lcd;
using ::mC;
using ::watch_screen;


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

LCDSprite spriteJugador[5] = {
  { // jugador
    .x = 326,
    .y = 25,
    .width = 25,
    .height = 50,
    .collision = 1,

  },
  { // jugador
    .x = 354,
    .y = 25,
    .width = 25,
    .height = 50,
    .collision = 1,

  },{ // jugador
    .x = 382,
    .y = 25,
    .width = 25,
    .height = 50,
    .collision = 1,

  },{ // jugador
    .x = 410,
    .y = 25,
    .width = 25,
    .height = 50,
    .collision = 1,

  },{ // jugador
    .x = 437,
    .y = 25,
    .width = 25,
    .height = 50,
    .collision = 1,

  }
};

LCDSprite spriteJugadorCaminando[5] = {
  { // jugador
    .x = 326,
    .y = 195,
    .width = 25,
    .height = 50,
    .collision = 1,

  },
  { // jugador
    .x = 354,
    .y = 195,
    .width = 25,
    .height = 50,
    .collision = 1,

  },{ // jugador
    .x = 382,
    .y = 195,
    .width = 25,
    .height = 50,
    .collision = 1,

  },{ // jugador
    .x = 410,
    .y = 195,
    .width = 25,
    .height = 50,
    .collision = 1,

  },{ // jugador
    .x = 437,
    .y = 195,
    .width = 25,
    .height = 50,
    .collision = 1,

  }
};


LCDSprite spriteJugadorCaminandoAD[5] = {
  { // jugador
    .x = 487,
    .y = 110,
    .width = 25,
    .height = 50,
    .collision = 1,

  },
  { // jugador
    .x = 513,
    .y = 110,
    .width = 25,
    .height = 50,
    .collision = 1,

  },{ // jugador
    .x = 541,
    .y = 110,
    .width = 25,
    .height = 50,
    .collision = 1,

  },{ // jugador
    .x = 569,
    .y = 110,
    .width = 25,
    .height = 50,
    .collision = 1,

  },{ // jugador
    .x = 597,
    .y = 110,
    .width = 25,
    .height = 50,
    .collision = 1,

  }
};

LCDSprite sprite[12] = {
  { // pasto
    .x = 50,
    .y = 250,
    .width = 25,
    .height = 25,

  },
  { // tierra
    .x = 75,
    .y = 250,
    .width = 25,
    .height = 25,

  },
  { // pavimento
    .x = 25,
    .y = 250,
    .width = 25,
    .height = 25,

  },
  { // piso casa
    .x = 0,
    .y = 250,
    .width = 25,
    .height = 25,

  },
  { // paredes de casa 
    .x = 0,
    .y = 275,
    .width = 25,
    .height = 75,
    .collision = 1,
    
  },
  { // puerta casa 
    .x = 25,
    .y = 450,
    .width = 25,
    .height = 50,
    .collision = 1,
    
  },
  { // Ventana casa 
    .x = 0,
    .y = 525,
    .width = 25,
    .height = 50,
    
  },
  { // arbusto
    .x = 0,
    .y = 0,
    .width = 25,
    .height = 25,
    
  },
  { // anden
    .x = 25,
    .y = 250,
    .width = 25,
    .height = 25,

  },
  { // mesa
    .x = 0,
    .y = 575,
    .width = 25,
    .height = 25,
    .collision = 1,

  },
  { // poste de luz
    .x = 50,
    .y = 575,
    .width = 25,
    .height = 25,
    .emiteLuz = true,
    .distanciaLuz = 350.0f,
    .collision = 1,
  },
  { // zombie
    .x = 25,
    .y = 75,
    .width = 25,
    .height = 50,
    .collision = 1,

  }
};

const uint8_t MODELO_JUGADOR = 11;
const uint8_t MODELO_ZOMBIE = 12;

const int mapa = 500;

float escala = 1.0f;

float valorEscalado = 25.0f * escala;

int dx = 45 * valorEscalado;
int dz = 45 * valorEscalado;

int offsetx = 0;
int offsetz = 0;

int paso = 25 * escala;

int luzsolx = 120;
int luzsoly = 120;
int luzsolz = 800;
float distanciaSol = 35000.0f * escala * 35000.0f * escala;

float distanciaMinimaLuz = 75.0f * escala * 75.0f * escala;

int totalLuces = 0;
LuzSpr luces[200];

uint8_t posJ = 0;

bool caminando = false;
bool caminandoAD = false;


static float calcularEscalaProfundidad(int y)
{
    float t = (float)y / (float)LCD_HEIGHT;

    // Arriba = pequeño
    // Abajo = grande
    return 0.60f + t * 0.60f;
}

// ============================================================
// COLISIONES
// ============================================================

// Devuelve true si dos rectángulos se están tocando
static bool colisionAABB(
    float ax,
    float az,
    float aw,
    float ah,
    float bx,
    float bz,
    float bw,
    float bh)
{
    return (
        ax < bx + bw &&
        ax + aw > bx &&
        az < bz + bh &&
        az + ah > bz
    );
}


// Comprueba si el jugador puede ocupar una posición determinada.
//
// x,z = nueva posición propuesta del jugador.
// Se revisan las casillas del mapa que podrían tocar al jugador.
//
// Los sprites que consideramos sólidos se definen aquí.
static bool puedeMoverJugador(int nuevaX, int nuevaZ)
{
    const float escalaMapa = valorEscalado;

    // ========================================================
    // TAMAÑO DE LA ZONA DE COLISIÓN DEL JUGADOR
    // ========================================================

    float jugadorW = 14.0f * escala;
    float jugadorH = 10.0f * escala;

    // ========================================================
    // POSICIÓN DE LA ZONA DE COLISIÓN
    //
    // nuevaX / nuevaZ representan la posición de referencia
    // inferior del sprite.
    //
    // La colisión queda centrada en los pies del personaje.
    // ========================================================

    float jugadorX =
        nuevaX + 120 +
        (25.0f * escala - jugadorW) * 0.5f;

    float jugadorZ = 120 + 
        nuevaZ + jugadorH;


    // ========================================================
    // Buscar solamente las celdas cercanas
    // ========================================================

    int celdaMinX = (int)(jugadorX / escalaMapa);
    int celdaMaxX = (int)((jugadorX + jugadorW) / escalaMapa);

    int celdaMinZ = (int)(jugadorZ / escalaMapa);
    int celdaMaxZ = (int)((jugadorZ + jugadorH) / escalaMapa);


    // ========================================================
    // Revisar obstáculos
    // ========================================================

    for (int z = celdaMinZ - 1; z <= celdaMaxZ + 1; z++)
    {
        if (z < 0 || z >= mapa)
            continue;

        for (int x = celdaMinX - 1; x <= celdaMaxX + 1; x++)
        {
            if (x < 0 || x >= mapa)
                continue;

            uint8_t sp = mapaPueblo[z][x];

            if (sprite[sp - 1].collision == 0)
                continue;


            // =================================================
            // Posición del obstáculo
            // =================================================

            float obstaculoX = x * escalaMapa;
            float obstaculoZ = z * escalaMapa;

            float obstaculoW =
                sprite[sp - 1].width * escala;

            float obstaculoH =
                sprite[sp - 1].height * escala;


            // =================================================
            // Colisión
            // =================================================

            if (colisionAABB(
                    jugadorX,
                    jugadorZ,
                    jugadorW,
                    jugadorH,
                    obstaculoX,
                    obstaculoZ,
                    obstaculoW,
                    obstaculoH))
            {
                return false;
            }
        }
    }

    return true;
}

static void procesarJoystick()
{
    int nuevoDX = dx;
    int nuevoDZ = dz;

    // --------------------------------------------------------
    // Movimiento horizontal
    // --------------------------------------------------------
    caminando = false;
    caminandoAD = false;
    if (mC.t_derecha){
        nuevoDX -= paso;
        //caminando = true;
    }

    if (mC.t_izquierda){
        nuevoDX += paso;
        caminando = true;
    }

    // --------------------------------------------------------
    // Movimiento vertical
    // --------------------------------------------------------

    if (mC.t_abajo){
        nuevoDZ -= paso;
        if(caminando){
          caminando = false;
          caminandoAD = true;
        }else
          caminando = true;
    }

    if (mC.t_arriba){
        nuevoDZ += paso;
        caminando = true;
    }


    // --------------------------------------------------------
    // Límites del mapa
    // --------------------------------------------------------

    if (nuevoDZ < 0)
        nuevoDZ = 0;

    if (nuevoDX < 0)
        nuevoDX = 0;

    if (nuevoDZ > (mapa * valorEscalado) - 12)
        nuevoDZ = (mapa * valorEscalado) - 12;

    if (nuevoDX > (mapa * valorEscalado) - 12)
        nuevoDX = (mapa * valorEscalado) - 12;


    // --------------------------------------------------------
    // Colisión
    //
    // Primero intentamos mover X y Z simultáneamente.
    // --------------------------------------------------------

    if (puedeMoverJugador(nuevoDX, nuevoDZ))
    {
        dx = nuevoDX;
        dz = nuevoDZ;
    }
    else
    {
        // ----------------------------------------------------
        // Si falla la diagonal, intentamos solamente X.
        // Esto permite "deslizarse" por las paredes.
        // ----------------------------------------------------

        if (puedeMoverJugador(nuevoDX, dz))
            dx = nuevoDX;

        // ----------------------------------------------------
        // Intentamos solamente Z.
        // ----------------------------------------------------

        if (puedeMoverJugador(dx, nuevoDZ))
            dz = nuevoDZ;
    }
}

static void juegoSetup() {
  for(int i= 0; i < mapa; i++){
    for(int j= 0; j< mapa; j++){
      uint8_t sp = mapaPueblo[i][j];
      if(sprite[sp - 1].emiteLuz && totalLuces < 200){
        luces[totalLuces].position.x = j;
        luces[totalLuces].position.y = i;
        luces[totalLuces].distancia = sprite[sp - 1].distanciaLuz * escala * sprite[sp - 1].distanciaLuz * escala;
        luces[totalLuces].encendido = true;
        totalLuces++;
      }
    }
  }
}

unsigned long time_init = millis();

static void juegoLoop() {
  // B siempre regresa al menú desde la prueba 3D.
  if (mC.t_B) {
    watch_screen = STATE_MENU;
    return;
  }
  lcd.clear(0x0000);

  procesarJoystick();
  
  int cellX = dx / valorEscalado;
  int cellZ = dz / valorEscalado;

  int offsetx = dx % (int)(valorEscalado);
  int offsetz = dz % (int)(valorEscalado);

  for (int j = 0; j < (LCD_HEIGHT / (valorEscalado)) + 1; j++)
  {
      int mapZ = cellZ + j;

      if (mapZ < 0 || mapZ >= mapa)
          continue;

      for (int i = 0; i < (LCD_WIDTH / (valorEscalado)) + 1; i++)
      {
          int mapX = cellX + i;

          if (mapX < 0 || mapX >= mapa)
              continue;
          
          uint8_t sp = mapaPueblo[mapZ][mapX];
          float intensidad = 0.0f;
          float totalintensidad = 0.0f;

          int distX = luzsolx * valorEscalado - dx - i * valorEscalado + offsetx;
          int distY = luzsoly * valorEscalado - dz - j * valorEscalado + offsetz;
          int distZ = luzsolz * valorEscalado - sprite[sp - 1].height * valorEscalado;

          float distTS = distX * distX + distY * distY + distZ * distZ;
          totalintensidad = 1.0f - distTS / (distanciaSol);

          if(totalintensidad < 0.0f) totalintensidad = 0.0f;

          for(int l = 0; l < totalLuces; l++){
            if(luces[l].encendido){
              int distX = luces[l].position.x * valorEscalado - dx - i * valorEscalado + offsetx;
              int distY = luces[l].position.y * valorEscalado - dz - j * valorEscalado + offsetz;

              float distT = distX * distX + distY * distY;
              if(distT > distanciaMinimaLuz)
                intensidad = 1.0f - distT / (luces[l].distancia);
              else
                intensidad = 1.0f;
              if(intensidad < 0.0f)
                intensidad = 0.0f;

              totalintensidad += intensidad;
            }
          }
          if(totalintensidad > 1.0f)
            totalintensidad = 1.0f;

          
          if(sp == 5){
            lcd.drawSceneSpriteGame(
                sprite[3],
                i * valorEscalado,
                j * valorEscalado,
                offsetx,
                offsetz,
                totalintensidad,
                escala
            );
          }
          if(sp == 8){
            lcd.drawSceneSpriteGame(
                sprite[0],
                i * valorEscalado,
                j * valorEscalado,
                offsetx,
                offsetz,
                totalintensidad,
                escala
            );
          }

          lcd.drawSceneSpriteGame(
              sprite[sp - 1],
              i * valorEscalado,
              j * valorEscalado,
              offsetx,
              offsetz,
              totalintensidad,
              escala
          );
      }
  }
  float intensidad = 0.0f;
  float totalintensidad = 0.0f;

  int distX = luzsolx * valorEscalado - dx - 120;
  int distY = luzsoly * valorEscalado - dz - 120;
  int distZ = luzsolz * valorEscalado - sprite[MODELO_JUGADOR].height * valorEscalado;

  float distT = distX * distX + distY * distY + distZ * distZ;
  totalintensidad = 1.0f - distT / (distanciaSol);

  if(totalintensidad < 0.0f) totalintensidad = 0.0f;

  for(int l = 0; l < totalLuces; l++){
    if(luces[l].encendido){
      int distX = luces[l].position.x * valorEscalado - dx - 120;
      int distY = luces[l].position.y * valorEscalado - dz - 120;

      float distT = distX * distX + distY * distY;
      if(distT > distanciaMinimaLuz)
        intensidad = 1.0f - distT / (luces[l].distancia);
      else
        intensidad = 1.0f;
      if(intensidad < 0.0f)
        intensidad = 0.0f;

      totalintensidad += intensidad;
    }
  }
  if(totalintensidad > 1.0f)
    totalintensidad = 1.0f;

  if(caminando){
    lcd.drawSceneSpriteGame(
                spriteJugadorCaminando[posJ],
                120,
                120,
                0,
                0,
                totalintensidad,
                escala
            );
  }else if(caminandoAD){
    lcd.drawSceneSpriteGame(
                spriteJugadorCaminandoAD[posJ],
                120,
                120,
                0,
                0,
                totalintensidad,
                escala
            );
  }else{
    lcd.drawSceneSpriteGame(
                spriteJugador[posJ],
                120,
                120,
                0,
                0,
                totalintensidad,
                escala
            );
  }
  if(millis() - time_init > 100){
    posJ++;
    time_init = millis();
  }
  if(posJ >= 5){
    posJ = 0;
  }
}

}  // namespace PRUEBAS3D_GAME







/*


escala = 0.600f;
  float valorEscaladoAnterior = 0.0f;
  int cellX = dx / 25;
  int cellZ = dz / 25;
  int offsetx = dx % (int)(25);
  int offsetz = dz % (int)(25);
  int j = 0;
  while(escala < 1.300f){
    valorEscalado = 25.000f * escala;

    //for (int j = 0; j < 1; j++)
    //{
        int mapZ = cellZ + j;

        if (mapZ < 0 || mapZ >= mapa)
            continue;

        for (int i = 0; i < (LCD_WIDTH / (valorEscalado)) + 1; i++)
        {
            int mapX = cellX + i;

            if (mapX < 0 || mapX >= mapa)
                continue;
            
            uint8_t sp = mapaPueblo[mapZ][mapX];
            float intensidad = 0.0f;
            float totalintensidad = 0.0f;

            int distX = luzsolx * valorEscalado - dx - i * valorEscalado + offsetx;
            int distY = luzsoly * valorEscalado - dz - j * valorEscalado + offsetz;
            int distZ = luzsolz * valorEscalado - sprite[sp - 1].height * valorEscalado;

            float distTS = distX * distX + distY * distY + distZ * distZ;
            totalintensidad = 1.0f - distTS / (distanciaSol);

            if(totalintensidad < 0.0f) totalintensidad = 0.0f;

            for(int l = 0; l < totalLuces; l++){
              if(luces[l].encendido){
                int distX = luces[l].position.x * valorEscalado - dx - i * valorEscalado + offsetx;
                int distY = luces[l].position.y * valorEscalado - dz - j * valorEscalado + offsetz;

                float distT = distX * distX + distY * distY;
                if(distT > distanciaMinimaLuz)
                  intensidad = 1.0f - distT / (luces[l].distancia);
                else
                  intensidad = 1.0f;
                if(intensidad < 0.0f)
                  intensidad = 0.0f;

                totalintensidad += intensidad;
              }
            }
            if(totalintensidad > 1.0f)
              totalintensidad = 1.0f;

            
            if(sp == 5){
              lcd.drawSceneSpriteGameV2(
                  sprite[3],
                  i * valorEscalado,
                  valorEscaladoAnterior,
                  offsetx,
                  offsetz,
                  totalintensidad,
                  escala - ((0.6f / 300.0f) * offsetz) 
              );
            }
            if(sp == 8){
              lcd.drawSceneSpriteGameV2(
                  sprite[0],
                  i * valorEscalado,
                  valorEscaladoAnterior,
                  offsetx,
                  offsetz,
                  totalintensidad,
                  escala - ((0.6f / 300.0f) * offsetz) 
              );
            }

            lcd.drawSceneSpriteGameV2(
                sprite[sp - 1],
                i * valorEscalado,
                valorEscaladoAnterior,
                offsetx,
                offsetz,
                totalintensidad,
                escala - ((0.6f / 300.0f) * offsetz) 
            );
        }
        j++;
        valorEscaladoAnterior += (25.0f * (escala - ((0.6f / 300.0f) * offsetz)));
        escala += ((0.6f * 25.0f) / 300.0f);
    //}
  }
  valorEscalado = 25.0f;
    float intensidad = 0.0f;
    float totalintensidad = 0.0f;

    int distX = luzsolx * valorEscalado - dx - 120;
    int distY = luzsoly * valorEscalado - dz - 120;
    int distZ = luzsolz * valorEscalado - sprite[MODELO_JUGADOR].height * valorEscalado;

    float distT = distX * distX + distY * distY + distZ * distZ;
    totalintensidad = 1.0f - distT / (distanciaSol);

    if(totalintensidad < 0.0f) totalintensidad = 0.0f;

    for(int l = 0; l < totalLuces; l++){
      if(luces[l].encendido){
        int distX = luces[l].position.x * valorEscalado - dx - 120;
        int distY = luces[l].position.y * valorEscalado - dz - 120;

        float distT = distX * distX + distY * distY;
        if(distT > distanciaMinimaLuz)
          intensidad = 1.0f - distT / (luces[l].distancia);
        else
          intensidad = 1.0f;
        if(intensidad < 0.0f)
          intensidad = 0.0f;

        totalintensidad += intensidad;
      }
    }
    if(totalintensidad > 1.0f)
      totalintensidad = 1.0f;

    lcd.drawSceneSpriteGame(
                sprite[MODELO_JUGADOR],
                120,
                120,
                0,
                0,
                totalintensidad,
                1.0f
            );




*/
