# Graph Engine

Grafový engine vytvořený pro reprezentaci a simulaci grafu protokolu OSPF (Open Shortest Path First). Engine umožňuje vizualizaci síťové topologie, simulaci routerů a sledování šíření LSA (Link State Advertisements).

## 🎯 Features

### 🔌 Síťová topologie a routery
- **Vizuální editor** - Interaktivní tvorba síťových grafů pomocí drag & drop
- **OSPF routing protokol** - Plně funkční implementace OSPF s podporou:
  - Link State Database (LSDB) pro každý router
  - Flooding LSA zpráv mezi sousedy
  - Dijkstra algoritmus pro výpočet nejkratší cesty
  - Automatická synchronizace topologie
- **Dynamické routery** - Každý router udržuje vlastní LSDB a zná kompletní topologii sítě

### 📡 Simulace zpráv
- **Message routing** - Vizualizace cesty zprávy mezi routery
- **Hop-by-hop přenos** - Sledování průchodu zprávy jednotlivými routery
- **Shortest path** - Automatický výpočet optimální cesty pomocí OSPF

### 💾 Save/Load systém
- **Automatické ukládání** - Stisknutím klávesy pro save
- **JSON formát** - Čitelný a editovatelný formát
- **Perzistence** - Ukládání celé topologie včetně:
  - Pozice všech uzlů (routerů)
  - Všechny hrany a jejich rychlosti
  - Pozice kamery
  - Zoom level

### 🎨 Interaktivní UI
- **Sandbox režim** - Volný pohyb po velkém plátně (4000x4000)
- **Zoom** - Přiblížení/oddálení
- **Widget systém** - Tlačítka a UI prvky pro ovládání
- **Edit módy** - Různé režimy editace (přidávání uzlů, hran, posílání zpráv)

## 🛠️ Kompilace

### Požadavky
- **CMake** 3.10 nebo vyšší
- **C++ Compiler** s podporou C++20
- **Git** (pro stažení závislostí)

### Závislosti
Projekt automaticky stahuje závislosti pomocí CPM (CMake Package Manager):
- **Raylib 5.5** - Grafická knihovna pro rendering
- **RapidJSON** - JSON parser (součástí projektu)

### Build proces

#### Linux/macOS
```bash
# Vytvoření build složky
mkdir build
cd build

# Konfigurace CMake
cmake ..

# Kompilace
cmake --build .

# Výsledný spustitelný soubor
./EngineExecutable
```

#### Windows (Visual Studio)
```bash
# Vytvoření build složky
mkdir build
cd build

# Konfigurace CMake pro Visual Studio
cmake .. -G "Visual Studio 17 2022"

# Kompilace
cmake --build . --config Release

# Spuštění
Release\EngineExecutable.exe
```

## 🚀 Spuštění

### Základní spuštění
```bash
./EngineExecutable
```
nebo
```bash
./EngineExecutable ../save.json
```
Pro načtení předpřipravené sítě v root directory

Program se spustí s defaultním `save.json` souborem. Pokud soubor neexistuje, vytvoří se prázdný.

### Spuštění s vlastním save souborem
```bash
./EngineExecutable my_network.json
```
Načte topologii z `my_network.json` nebo vytvoří nový soubor.

## 💾 Používání Save systému

### Automatické ukládání
Program ukládá stav při určité akci (klávesa ESC).

### Formát save souboru
Save soubor je v JSON formátu:

```json
{
  "CameraX": 1570.0,
  "CameraY": 1680.0,
  "Nodes": {
    "1": {
      "X": 2150.0,
      "Y": 2100.0,
      "Type": "Router",
      "IP": "192.168.1.1"
    },
    "2": {
      "X": 2450.0,
      "Y": 2300.0,
      "Type": "Router",
      "IP": "192.168.1.2"
    }
  },
  "Edges": {
    "1": {
      "NodeA": 1,
      "NodeB": 2,
      "Speed": 1
    }
  }
}
```

### Co se ukládá
- **CameraX, CameraY** - Pozice kamery v sandboxu
- **Nodes** - Všechny routery včetně:
  - Pozice (X, Y)
  - Typ uzlu
  - IP adresa
- **Edges** - Všechny spojení mezi routery:
  - ID obou uzlů (NodeA, NodeB)
  - Rychlost spojení (Speed)

### Load při startu
Program automaticky načte save file při spuštění:
1. Pokud je zadán jako parametr: `./EngineExecutable custom.json`
2. Jinak použije defaultní `save.json`
3. Pokud soubor neexistuje, vytvoří prázdný

## 🎮 Ovládání

### Základní ovládání
- **Myš** - Klikání a výběr uzlů
- **Drag & Drop** - Přesouvání uzlů
- **Scroll** - Zoom in/out
- **Klávesnice** - Save a další funkce

### Edit módy
- **Add Node** - Přidání nového routeru
- **Add Edge** - Vytvoření spojení mezi routery
- **Send Message** - Poslání zprávy mezi routery (vizualizace OSPF cesty)

### UI Tlačítka
- **Clear** - Zrušení výběru
- **Remove** - Odstranění vybraných uzlů/hran
- **Send Message** - Aktivace režimu posílání zpráv

## 📁 Struktura projektu

```
graph_engine/
├── CMakeLists.txt          # Hlavní build konfigurace
├── README.md               # Tento soubor
├── save.json              # Defaultní save file
├── assets/                # Grafické assety
│   └── nebula_background.png
├── Engine/                # Jádro engine
│   ├── include/          # Public API
│   │   ├── Engine.hpp
│   │   ├── EngineTypes.hpp
│   │   ├── Node.hpp
│   │   ├── Widget.hpp
│   │   └── SandboxSave.hpp
│   ├── src/              # Implementace
│   │   ├── Engine.cpp
│   │   ├── SandboxSave.cpp
│   │   └── ...
│   └── third_party/      # Závislosti
│       └── rapidjson/
├── include/              # Aplikační headery
│   ├── Router.hpp
│   └── LSDB.hpp
└── src/                  # Aplikační kód
    ├── main.cpp
    ├── Router.cpp
    └── LSDB.cpp
```

## 🔧 Konvence kódu

- **Public** metody a data jsou psány velkými písmeny
- **Private** metody a data jsou psány malými písmeny

Příklad:
```cpp
class Router {
public:
    void SyncWithNetwork();    // Public - velké
    NodeData GetData();        // Public - velké
    
private:
    void updateLocalLSA();     // Private - malé
    uint16_t routerId;         // Private - malé
};
```

- Engine používá **singleton pattern** pro hlavní Engine třídu
- **Raylib** zajišťuje rendering a window management
- **RapidJSON** pro rychlé zpracování save/load
- OSPF implementace zahrnuje kompletní LSDB a Dijkstra shortest path
