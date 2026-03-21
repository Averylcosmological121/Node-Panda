# Node Panda

Gestor personal de notas enlazadas con grafo visual interactivo, motor de conocimiento y API local. Construido desde cero en C++ con Dear ImGui y OpenGL.

![Node Panda](data/panda.png)

---

## Changelog

### v1.1.0 — Knowledge Engine Update
- **Motor de Consultas** — queries tipo SQL sobre el vault (`QUERY notas WHERE tipo = "proyecto" AND enlaces > 3`)
- **Control de Versiones Git** — historial automático de cada nota, diff viewer, restauración de versiones
- **Análisis de Grafo** — PageRank, Betweenness Centrality y detección de comunidades sobre el grafo de notas
- **Repetición Espaciada (SRS)** — algoritmo SM-2 integrado directamente en las notas para revisión activa
- **API REST local** — servidor HTTP en `localhost:8042` para integrar Node Panda con scripts y herramientas externas
- **Sistema de Plugins** — motor Lua 5.4 con sandbox, lifecycle hooks, bindings de ImGui y 8 plugins incluidos

### v1.0.0 — Lanzamiento inicial
- Notas en Markdown con frontmatter YAML
- Grafo de nodos interactivo con force layout y Barnes-Hut
- Editor con preview de Markdown renderizado
- Backlinks automáticos
- Motor de Memoria semántica TF-IDF
- File Watcher en tiempo real
- Exportar contexto para Claude (XML)
- Scripting Lua con consola integrada
- Hub de bienvenida

---

## Características

### Core
- **Notas en Markdown** con frontmatter YAML (tipo, aliases, tags)
- **Grafo de nodos interactivo** con simulación de fuerzas Barnes-Hut
- **Coloreado por tipo**: proyecto (cyan), concepto (purple), referencia (amber), diario (verde), tarea (coral)
- **Editor con preview** de Markdown renderizado
- **Backlinks automáticos** — ve qué notas enlazan a la actual
- **Motor de Memoria TF-IDF** — búsqueda semántica y sugerencia de enlaces
- **File Watcher** — detecta cambios en disco en tiempo real
- **Exportar contexto** estructurado para Claude (XML)
- **Scripting Lua** — consola integrada con acceso completo al vault

### v1.1.0 — Nuevas funcionalidades

#### Motor de Consultas
Consultas estructuradas sobre el vault con sintaxis propia:
```
QUERY notas WHERE tipo = "proyecto" AND enlaces > 3 SORT palabras DESC LIMIT 10
QUERY notas WHERE tags CONTIENE "ia"
QUERY notas WHERE palabras > 500 SORT fecha DESC
```
Campos disponibles: `tipo`, `palabras`, `caracteres`, `enlaces`, `conexiones`, `fecha`, o cualquier clave de frontmatter.

#### Control de Versiones Git
- Auto-commit en cada guardado con timestamp
- Historial completo de cada nota
- Diff viewer inline (verde = añadido, rojo = eliminado)
- Restauración de cualquier versión anterior con un clic
- Requiere Git instalado en el sistema (`git` en el PATH)

#### Análisis de Grafo
- **PageRank** — identifica las notas más importantes por estructura de enlaces
- **Betweenness Centrality** — detecta notas que actúan como puentes entre áreas de conocimiento
- **Detección de comunidades** — agrupa notas en clusters temáticos automáticamente (Label Propagation)

#### Repetición Espaciada (SRS)
- Algoritmo SM-2 (el mismo que usa Anki)
- Cola de revisión diaria integrada en el vault
- Intervalos calculados automáticamente según dificultad (Fácil / Bien / Difícil / De Nuevo)
- Datos almacenados en el frontmatter de cada nota (`srs_interval`, `srs_ease`, `srs_next`)

#### API REST local
Servidor HTTP en `localhost:8042` — solo accesible desde tu propia máquina:

| Endpoint | Descripción |
|---|---|
| `GET /api/notas` | Lista todas las notas con metadatos |
| `GET /api/notas/{id}` | Contenido completo de una nota |
| `GET /api/grafo` | Estructura del grafo (nodos + aristas) |
| `GET /api/memoria/buscar?q=...&n=5` | Búsqueda semántica TF-IDF |
| `GET /api/stats` | Estadísticas del vault |

#### Sistema de Plugins (Lua 5.4)
Motor de extensiones con sandbox de seguridad. Plugins incluidos:

| Plugin | Descripción |
|---|---|
| `daily_notes` | Crea y abre nota diaria automáticamente |
| `stats_dashboard` | Estadísticas del vault en consola Lua |
| `templates` | 5 plantillas predefinidas (Reunión, Proyecto, Concepto, Tarea, Diario) |
| `tag_browser` | Panel para explorar y filtrar notas por tags |
| `word_count` | Estadísticas de la nota activa (palabras, tiempo de lectura) |
| `auto_linker` | Sugiere enlaces basados en similitud TF-IDF |
| `graph_exporter` | Exporta el grafo a DOT (Graphviz) y JSON |
| `theme_switcher` | 5 temas: Default, Nord, Catppuccin, Gruvbox Dark, Dracula |

---

## Requisitos

| Herramienta | Versión mínima | Descarga |
|---|---|---|
| **Visual Studio** (con BuildTools C++) | 2019 o 2022 | https://visualstudio.microsoft.com/downloads/ |
| **CMake** | 3.14+ | https://cmake.org/download/ |
| **Git** | cualquier versión reciente | https://git-scm.com/ |
| **Windows** | 10 / 11 (64-bit) | — |

> **Git en el PATH:** para que el Control de Versiones funcione, Git debe estar disponible en la terminal (`git --version` debe responder). Si no lo está, reinstala Git marcando *"Add Git to the system PATH"*.

---

## Instalación desde cero

### 1. Clonar el repositorio

```bash
git clone https://github.com/charbelochoa/NodePanda.git
cd NodePanda
```

### 2. Crear la carpeta de build

```bash
mkdir build
cd build
```

### 3. Generar el proyecto con CMake

Abre una **Developer PowerShell for VS 2022** — búscala en el menú Inicio — y ejecuta desde dentro de la carpeta `build`:

```powershell
cmake .. -G "Visual Studio 17 2022"
```

> Si usas Visual Studio **2019**:
> ```powershell
> cmake .. -G "Visual Studio 16 2019"
> ```
> CMake descargará automáticamente GLFW, ImGui y Lua via FetchContent. Requiere conexión a internet en el primer build.

### 4. Compilar

```powershell
cmake --build . --config Release
```

La compilación tarda 1–3 minutos la primera vez.

### 5. Ejecutar

```powershell
cd Release
.\NodePanda.exe
```

---

## Estructura del proyecto

```
NodePanda/
├── CMakeLists.txt
├── PLUGIN_API.md               # Documentación de la API de plugins
├── data/
│   ├── panda.png
│   ├── notas/                  # Tus notas (Markdown)
│   └── plugins/                # Plugins Lua
│       ├── daily_notes/
│       ├── templates/
│       ├── tag_browser/
│       ├── word_count/
│       ├── auto_linker/
│       ├── graph_exporter/
│       └── theme_switcher/
├── include/                    # Headers
│   ├── app.h
│   ├── query_engine.h          # Motor de consultas
│   ├── graph_analytics.h       # PageRank + clustering
│   ├── srs_engine.h            # Repetición espaciada SM-2
│   ├── git_manager.h           # Control de versiones
│   ├── http_server.h           # API REST local
│   ├── plugin_manager.h        # Sistema de plugins
│   └── ...
├── src/                        # Implementaciones
└── third_party/
    └── stb_image.h
```

---

## Uso básico

### Crear y enlazar notas
Usa el botón **+ Nota** en el Explorador. Enlaza notas con la sintaxis `[[NombreDeLaNota]]`. El grafo se actualiza en tiempo real.

### Frontmatter
```yaml
---
tipo: proyecto
aliases: MiProyecto
tags: diseño, web
---
```
Tipos disponibles: `proyecto`, `concepto`, `referencia`, `diario`, `tarea`.

### Motor de Consultas
Abre el panel "Motor de Consultas" (tab inferior) y escribe queries:
```
QUERY notas WHERE tipo = "proyecto" AND palabras > 200
QUERY notas WHERE enlaces = 0
```

### Control de Versiones
El historial es automático. Usa el panel "Control de Versiones" para ver cambios, comparar versiones y restaurar.

### API REST
Con la app abierta, desde el navegador o una terminal:
```
http://localhost:8042/api/stats
http://localhost:8042/api/notas
```

### Plugins
Los plugins se ubican en `data/plugins/`. Cada uno es una carpeta con `manifest.json` y `plugin.lua`. Ver [PLUGIN_API.md](PLUGIN_API.md) para la documentación completa.

### Grafo de nodos
- **Scroll** → zoom
- **Click medio + arrastrar** → pan
- **Click en un nodo** → selecciona esa nota
- **Menú Herramientas → Recalcular Analíticas** → PageRank y comunidades

---

## Problemas frecuentes

**`cmake` no se reconoce**
Reinstala CMake desde https://cmake.org/download/ marcando *"Add CMake to the system PATH"*.

**Error de generador al rehacer cmake**
Si ya existe una carpeta `build` de un build anterior, bórrala y vuélvela a crear:
```powershell
cd ..
Remove-Item -Recurse -Force build
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
```

**El Control de Versiones no aparece activo**
Verifica que `git --version` funciona en tu terminal. Si no, instala Git desde https://git-scm.com marcando *"Add Git to PATH"*.

**El servidor API no responde**
Verifica en el panel "Servidor API" que el estado sea "Activo". Si no arrancó automáticamente, usa **Herramientas → Iniciar Servidor API**.

**El grafo no se actualiza**
Ve a **Archivo → Reescanear Notas**.

---

## Dependencias (gestionadas automáticamente por CMake)

| Librería | Versión | Propósito |
|---|---|---|
| [Dear ImGui](https://github.com/ocornut/imgui) | v1.90.1-docking | Interfaz gráfica |
| [GLFW](https://github.com/glfw/glfw) | 3.3.8 | Ventana y contexto OpenGL |
| [Lua](https://www.lua.org) | 5.4.6 | Motor de scripting y plugins |
| [sol2](https://github.com/ThePhD/sol2) | v3.3.0 | Bindings C++ ↔ Lua |
| [stb_image](https://github.com/nothings/stb) | — | Carga de texturas PNG |
| OpenGL | sistema | Renderizado |
| comdlg32 + ws2_32 | sistema (Windows) | Diálogos nativos + sockets |

---

## Licencia

Copyright (c) 2026 Charbel Ochoa 

All rights reserved.

Este proyecto es personal/educativo. 
No se permite el uso, copia, modificación o distribución del código sin permiso explícito del autor.
