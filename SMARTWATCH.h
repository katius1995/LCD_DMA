#ifndef SMARTWATCH_H
#define SMARTWATCH_H

#include <Arduino.h>

// Variables de estado del Smartwatch e índices de navegación
WatchState watch_screen = STATE_CLOCK_FACE;
int8_t menu_index = 0;

// Calendario
int16_t calendar_month_offset = 0;
int16_t calendar_year = 2026;
int8_t calendar_month = 1;
int8_t calendar_day = 1;

// Variables de simulación del reloj
int16_t hora = 10, minuto = 42, segundo = 0;
String fecha = "SAB 12 SEP";
uint32_t cronometro_tiempo = 0;
uint32_t last_time = 0;

// Nombres de las opciones del menú (Items del Dock de Mac)
const char *menu_options[] = { "Naval", "Survive", "3D", "Wi-Fi", "Ajustes" };
const char *menu_icons[] = { "A", "B", "C", "D", "E" }; // Sustituir por letras o glifos de tu fuente custom

#define SMART_BG 0x0000
#define SMART_ACCENT 0x03FF
#define SMART_TEXT 0xFFFF
#define SMART_MUTED 0x7BEF
#define SMART_CARD 0x18C3

// NUEVA PALETA DE COLORES ESTILO macOS (Formato RGB565)
#define MAC_BG       0x2113  // Azul oscuro/morado profundo abstracto
#define MAC_DOCK     0xCE79  // Gris translúcido claro para la base del Dock
#define MAC_ACCENT   0x03FF  // Azul Apple brillante para selecciones
#define MAC_TEXT     0xFFFF  // Blanco puro
#define MAC_MUTED    0xA514  // Gris claro para elementos secundarios

void obtenerFechaCompilacion() {
  const char *fechaBuild = __DATE__;
  const char *meses = "JanFebMarAprMayJunJulAugSepOctNovDec";

  char mesTxt[4] = { fechaBuild[0], fechaBuild[1], fechaBuild[2], '\0' };
  int mes = 1;
  for (int i = 0; i < 12; i++) {
    if (mesTxt[0] == meses[i * 3] && mesTxt[1] == meses[i * 3 + 1] && mesTxt[2] == meses[i * 3 + 2]) {
      mes = i + 1;
      break;
    }
  }

  calendar_month = mes;
  calendar_day = (fechaBuild[4] == ' ') ? (fechaBuild[5] - '0') : ((fechaBuild[4] - '0') * 10 + fechaBuild[5] - '0');
  calendar_year = (fechaBuild[7] - '0') * 1000 + (fechaBuild[8] - '0') * 100 + (fechaBuild[9] - '0') * 10 + (fechaBuild[10] - '0');
}

bool esBisiesto(int16_t anio) {
  return ((anio % 4 == 0 && anio % 100 != 0) || (anio % 400 == 0));
}

uint8_t diasDelMes(int16_t anio, int8_t mes) {
  static const uint8_t dias[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  if (mes == 2 && esBisiesto(anio)) return 29;
  return dias[mes - 1];
}

uint8_t diaSemana(int16_t anio, int8_t mes, int8_t dia) {
  int y = anio;
  int m = mes;
  if (m < 3) {
    m += 12;
    y--;
  }
  int k = y % 100;
  int j = y / 100;
  int h = (dia + (13 * (m + 1)) / 5 + k + k / 4 + j / 4 + 5 * j) % 7;
  return (h + 6) % 7;
}

void ajustarMesCalendario(int16_t delta) {
  calendar_month_offset += delta;
}

void obtenerMesMostrado(int16_t &anio, int8_t &mes) {
  anio = calendar_year;
  mes = calendar_month;
  int16_t offset = calendar_month_offset;

  while (offset > 0) { mes++; if (mes > 12) { mes = 1; anio++; } offset--; }
  while (offset < 0) { mes--; if (mes < 1) { mes = 12; anio--; } offset++; }
}

void dibujarCalendario() {
  static const char *nombresMes[] = {
    "ENERO", "FEBRERO", "MARZO", "ABRIL", "MAYO", "JUNIO",
    "JULIO", "AGOSTO", "SEPTIEMBRE", "OCTUBRE", "NOVIEMBRE", "DICIEMBRE"
  };
  static const char *dias[] = { "D", "L", "M", "M", "J", "V", "S" };

  int16_t anio;
  int8_t mes;
  obtenerMesMostrado(anio, mes);

  char titulo[32];
  snprintf(titulo, sizeof(titulo), "%s %d", nombresMes[mes - 1], anio);
  lcd.drawText(20, 45, titulo, "SANSBOLD", MAC_ACCENT, INT_TO_FIXED(1));

  lcd.drawText(12, 75, "<", "SANSBOLD", MAC_MUTED, INT_TO_FIXED(1));
  lcd.drawText(218, 75, ">", "SANSBOLD", MAC_MUTED, INT_TO_FIXED(1));

  const int16_t x0 = 28;
  const int16_t y0 = 90;
  const int16_t cellW = 28;
  const int16_t cellH = 28;

  for (int i = 0; i < 7; i++) {
    lcd.drawText(x0 + i * cellW - 3, y0, dias[i], "SANSBOLD", MAC_MUTED, INT_TO_FIXED(1));
  }

  uint8_t primero = diaSemana(anio, mes, 1);
  uint8_t total = diasDelMes(anio, mes);

  for (uint8_t d = 1; d <= total; d++) {
    uint16_t pos = primero + d - 1;
    uint8_t col = pos % 7;
    uint8_t fila = pos / 7;
    int16_t cx = x0 + col * cellW + 7;
    int16_t cy = y0 + 24 + fila * cellH;

    bool esHoy = (calendar_month_offset == 0 && d == calendar_day && mes == calendar_month && anio == calendar_year);

    if (esHoy) {
      lcd.fillCircle(cx + 4, cy - 4, 11, MAC_ACCENT);
      char numero[3];
      snprintf(numero, sizeof(numero), "%d", d);
      lcd.drawText(cx - ((d >= 10) ? 6 : 3), cy + 1, numero, "SANSBOLD", MAC_TEXT, INT_TO_FIXED(1));
    } else {
      char numero[3];
      snprintf(numero, sizeof(numero), "%d", d);
      lcd.drawText(cx - ((d >= 10) ? 6 : 3), cy + 1, numero, "SANS", MAC_TEXT, INT_TO_FIXED(1));
    }
  }

  lcd.drawText(37, 292, "Joystick: < > meses", "SANS", MAC_MUTED, INT_TO_FIXED(1));
  lcd.drawText(62, 312, "B: volver", "SANS", MAC_MUTED, INT_TO_FIXED(1));
}

void procesar_inputs_hardware() {
  static uint32_t last_input_time = 0;

  // Botón B: volver desde cualquier vista secundaria.
  if (mC.t_B) {
    if (watch_screen == STATE_CALENDAR) {
      watch_screen = STATE_CLOCK_FACE;
      calendar_month_offset = 0;
    } else if (watch_screen != STATE_CLOCK_FACE) {
      watch_screen = (watch_screen == STATE_MENU) ? STATE_CLOCK_FACE : STATE_MENU;
    }
    vTaskDelay(pdMS_TO_TICKS(200));
    return;
  }

  if (millis() - last_input_time < 200) return;

  if (watch_screen == STATE_CLOCK_FACE) {
    if (mC.t_arriba) {
      watch_screen = STATE_CALENDAR;
      calendar_month_offset = 0;
      last_input_time = millis();
    } else if (mC.t_abajo) {
      watch_screen = STATE_MENU;
      menu_index = 0;
      last_input_time = millis();
    }
  } else if (watch_screen == STATE_CALENDAR) {
    if (mC.t_derecha) {
      ajustarMesCalendario(-1);
      last_input_time = millis();
    } else if (mC.t_izquierda) {
      ajustarMesCalendario(+1);
      last_input_time = millis();
    }
  } else if (watch_screen == STATE_MENU) {
    if (mC.t_izquierda) {
      menu_index--;
      if (menu_index < 0) menu_index = 4;
      last_input_time = millis();
    } else if (mC.t_derecha) {
      menu_index++;
      if (menu_index > 4) menu_index = 0;
      last_input_time = millis();
    } else if (mC.t_A) {
      if (menu_index == 0) {
        watch_screen = STATE_GAME_NAVAL;
        selectGameGeometry(GEOMETRY_BATALLA_NAVAL);
        BATALLA_GAME::juegoSetup();
      } else if (menu_index == 1) {
        watch_screen = STATE_GAME_SURVIVE;
        selectGameGeometry(GEOMETRY_SUPERVIVENCIA);
        SUPERVIVENCIA_GAME::juegoSetup();
      } else if (menu_index == 2) {
        watch_screen = STATE_GAME_3D;
        selectGameGeometry(GEOMETRY_TEXTURED_OBJ);
        PRUEBAS3D_GAME::juegoSetup();
      } else if (menu_index == 3) watch_screen = STATE_WIFI;
      else if (menu_index == 4) watch_screen = STATE_SETTINGS;
      last_input_time = millis();
    }
  }
}

unsigned long time_init = millis();
uint8_t frames = 0;
int fps = 0;

static void smartWatchSetup() {
  cronometro_tiempo = millis();
  obtenerFechaCompilacion();
}

static void smartWatchLoop() {
  if (watch_screen != STATE_GAME_NAVAL && watch_screen != STATE_GAME_SURVIVE && watch_screen != STATE_GAME_3D){
    procesar_inputs_hardware();
  }

  uint32_t t_actual = millis();
  if (t_actual - last_time >= 1000) {
    segundo++;
    if (segundo >= 60) { segundo = 0; minuto++; }
    if (minuto >= 60) { minuto = 0; hora++; }
    if (hora >= 24) { hora = 0; }
    last_time = t_actual;
  }

  char string_hora[9];
  char string_mini_hora[6];
  sprintf(string_hora, "%02d:%02d:%02d", hora, minuto, segundo);
  sprintf(string_mini_hora, "%02d:%02d", hora, minuto);

  lcd.startFrame();
  while (lcd.nextBlock()) {
    // Fondo de escritorio abstracto macOS
    lcd.clear(MAC_BG);

    // 1. BARRA DE MENÚ SUPERIOR GLOBAL (Estilo Mac)
    lcd.fillRect(0, 0, LCD_WIDTH, 20, 0x39E7); // Fondo translúcido oscuro de la barra
    lcd.drawText(8, 14, "Finder", "SANSBOLD", MAC_TEXT, INT_TO_FIXED(1));
    lcd.drawText(LCD_WIDTH - 45, 14, string_mini_hora, "SANS", MAC_TEXT, INT_TO_FIXED(1));

    // 2. RENDERIZADO SEGÚN LA PANTALLA ACTIVA
    if (watch_screen == STATE_CLOCK_FACE) {
      // Escritorio limpio con Reloj Central Minimalista
      lcd.drawText(30, 130, string_hora, "SOLARONA", MAC_TEXT, INT_TO_FIXED(1));
      lcd.drawText(70, 170, fecha.c_str(), "SANS", MAC_MUTED, INT_TO_FIXED(1));
      
      // Indicador inferior sutil para abrir el menú/dock
      lcd.drawText(112, 295, "...", "SANSBOLD", MAC_MUTED, INT_TO_FIXED(1));
    } 
    else if (watch_screen == STATE_CALENDAR) {
      dibujarCalendario();
    } 
    else if (watch_screen == STATE_MENU) {
      // ==========================================================
      // ESCRITORIO CON INTERFAZ DOCK INTERACTIVO (ESTILO macOS)
      // ==========================================================
      
      // 1. Título flotante de la Aplicación en el centro de la pantalla
      char app_title[32];
      snprintf(app_title, sizeof(app_title), "%s", menu_options[menu_index]);
      lcd.drawText(40, 130, app_title, "SANSBOLD", MAC_TEXT, INT_TO_FIXED(1));
      
      // 2. Definición geométrica de la base del Dock
      int16_t dockX = 12;
      int16_t dockY = 245;
      int16_t dockW = LCD_WIDTH - 24;
      int16_t dockH = 50;
      
      // Dibujar base del Dock (Gris claro translúcido con efecto de bordes suaves)
      lcd.fillRect(dockX, dockY, dockW, dockH, MAC_DOCK);
      
      // 3. Renderizado de los Iconos Redondeados del Dock
      int16_t num_items = 5;
      int16_t spacing = dockW / num_items;
      int16_t iconSize = 32; // Tamaño del icono cuadrado

      // Efecto mecánico de rebote elástico (Bounce/Spring) por software
      static int16_t bounceY = 0;
      static int8_t last_index = -1;
      
      if (menu_index != last_index) {
        bounceY = -12; // Impulso inicial hacia arriba (Simula la gravedad/inercia al activarse)
        last_index = menu_index;
      }
      if (bounceY < 0) {
        bounceY += 2; // Atenuación progresiva del salto hasta volver a su posición base (Y=0)
      }

      for (int i = 0; i < num_items; i++) {
        // Calcular centro horizontal para cada slot del Dock
        int16_t centerX = dockX + (i * spacing) + (spacing / 2);
        int16_t iconX = centerX - (iconSize / 2);
        int16_t iconY = dockY + (dockH - iconSize) / 2; // Centrado vertical por defecto
        
        if (i == menu_index) {
          // APLICACIÓN SELECCIONADA: Aplica el desfase del rebote elástico
          iconY += bounceY; 
          
          // Renderizar Marco Redondeado Estilo Apple (Capa exterior de realce)
          lcd.fillRect(iconX - 2, iconY - 2, iconSize + 4, iconSize + 4, MAC_ACCENT);
          
          // Cuerpo del Icono Interno (Simulación de esquinas curvas suavizadas mediante esquinas recortadas)
          lcd.fillRect(iconX, iconY, iconSize, iconSize, MAC_TEXT);
          lcd.fillRect(iconX, iconY, 2, 2, MAC_ACCENT); // Matar esquinas superiores
          lcd.fillRect(iconX + iconSize - 2, iconY, 2, 2, MAC_ACCENT);
          lcd.fillRect(iconX, iconY + iconSize - 2, 2, 2, MAC_ACCENT); // Matar esquinas inferiores
          lcd.fillRect(iconX + iconSize - 2, iconY + iconSize - 2, 2, 2, MAC_ACCENT);

          // Glifo o detalle de color identificador en el centro del icono
          uint16_t colores_apps[] = { 0xF800, 0x07E0, 0x001F, 0xF81F, 0xE71C }; // Colores Mac únicos por app
          lcd.fillRect(iconX + 10, iconY + 10, 12, 12, colores_apps[i]);
          
          // Punto indicador inferior característico de macOS (App activa/abierta)
          lcd.fillCircle(centerX, dockY + dockH - 4, 2, MAC_TEXT);
        } 
        else {
          // APLICACIONES EN REPOSO (Estilo minimalista oscuro integrado en el Dock)
          uint16_t color_icono_reposo = 0x3186; // Gris oscuro aluminio
          
          // Dibujar cuadro base
          lcd.fillRect(iconX, iconY, iconSize, iconSize, color_icono_reposo);
          
          // Suavizado de esquinas para el modo reposo (Mismo color que la base del Dock)
          lcd.fillRect(iconX, iconY, 2, 2, MAC_DOCK);
          lcd.fillRect(iconX + iconSize - 2, iconY, 2, 2, MAC_DOCK);
          lcd.fillRect(iconX, iconY + iconSize - 2, 2, 2, MAC_DOCK);
          lcd.fillRect(iconX + iconSize - 2, iconY + iconSize - 2, 2, 2, MAC_DOCK);

          // Detalle interno sutil
          lcd.fillRect(iconX + 12, iconY + 12, 8, 8, MAC_MUTED);
        }
      }
      
      // Guía de controles inferiores
      lcd.drawText(48, 312, "A: Abrir  B: Escritorio", "SANS", MAC_MUTED, INT_TO_FIXED(1));
    }else if (watch_screen == STATE_GAME_NAVAL) {
      selectGameGeometry(GEOMETRY_BATALLA_NAVAL);
      BATALLA_GAME::juegoLoop();
    } else if (watch_screen == STATE_GAME_SURVIVE) {
      selectGameGeometry(GEOMETRY_SUPERVIVENCIA);
      SUPERVIVENCIA_GAME::juegoLoop();
    } else if (watch_screen == STATE_GAME_3D) {
      selectGameGeometry(GEOMETRY_TEXTURED_OBJ);
      PRUEBAS3D_GAME::juegoLoop();
    } else if (watch_screen == STATE_WIFI) {
      lcd.drawText(15, 45, "Redes Wi-Fi", "SANSBOLD", SMART_ACCENT, INT_TO_FIXED(1));
      lcd.drawText(20, 95, "[*] Casa_Fibra", "SANS", SMART_TEXT, INT_TO_FIXED(1));
      lcd.drawText(20, 135, "[ ] ESP32_Mesh_Net", "SANS", SMART_MUTED, INT_TO_FIXED(1));
      lcd.drawText(20, 175, "[ ] Red_Publica", "SANS", SMART_MUTED, INT_TO_FIXED(1));
    } else if (watch_screen == STATE_SETTINGS) {
      lcd.drawText(15, 45, "Ajustes Generales", "SANSBOLD", SMART_ACCENT, INT_TO_FIXED(1));
      lcd.drawText(20, 95, " Brillo: 100%", "SANS", SMART_TEXT, INT_TO_FIXED(1));
      lcd.drawRect(20, 110, 150, 6, SMART_CARD);
      lcd.fillRect(20, 110, 150, 6, SMART_ACCENT);
      lcd.drawText(20, 155, " Bus SPI: 80 MHz", "SANS", SMART_TEXT, INT_TO_FIXED(1));
      lcd.drawText(20, 195, " Sistema: V4.0-DMA", "SANS", SMART_TEXT, INT_TO_FIXED(1));
    }

    frames++;

    if (frames >= 20) {
      fps = (frames * 1000) / (millis() - time_init);
      time_init = millis();
      frames = 0;
    }

    lcd.drawText(7, LCD_HEIGHT - 15,
                 String(fps).c_str(),
                 "SANS", 0xFFFF, INT_TO_FIXED(1));
    lcd.endFrame();
  }
}

#endif // SMARTWATCH_H
