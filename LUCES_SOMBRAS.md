# Luces y sombras implementadas

## Tipos de objeto
- `LUZPUNTUAL`: luz omnidireccional con alcance y atenuación.
- `LUZDIRECCIONAL`: luz tipo sol.
- `LUZSPOT`: foco con cono y alcance.
- `FUEGO`: luz puntual pensada para fogatas/fuego.

## Encendido
Cada `Obj` tiene:
```cpp
obj.luz.encendido = true;  // encendida
obj.luz.encendido = false; // apagada
```

También se puede cambiar dinámicamente durante el juego, por ejemplo según día/noche.

## Parámetros
```cpp
obj.luz.intensidad
obj.luz.alcance
obj.luz.angulo
obj.luz.suavidad
obj.luz.direccion
obj.luz.color
```

La posición se sincroniza automáticamente con `obj.position + obj.traslation` para luces puntuales/spot/fuego.

## Sombras
Se agregó sombra directa mediante prueba rayo-AABB contra los objetos `BODY`. Es una primera implementación de bajo coste adecuada al ESP32-S3: no crea un segundo framebuffer ni interfiere con DMA.

La sombra se calcula por cara, usando el centro de la cara y la luz activa. La geometría del oclusor se aproxima mediante su AABB, por lo que las sombras son funcionales pero no tienen todavía el detalle de un shadow-map por triángulo.

## Compatibilidad
Cuando no se registran luces con `beginGameLighting()`, `drawSceneGameObjectV2()` conserva la iluminación solar anterior.
