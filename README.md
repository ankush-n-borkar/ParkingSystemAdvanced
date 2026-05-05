# 🏢 Multi-Floor Smart Parking System

A full-stack smart parking management system for **Navami Apartments**, built with a **C backend** (HTTP server + BFS pathfinding algorithms) and a **React frontend** with real-time updates.

***

## 📌 Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Tech Stack](#tech-stack)
- [Project Structure](#project-structure)
- [How It Works](#how-it-works)
- [Getting Started](#getting-started)
- [API Reference](#api-reference)
- [Configuration](#configuration)
- [Known Limitations](#known-limitations)

***

## Overview

This system manages a multi-floor parking facility with real-time slot tracking, BFS-based shortest path recommendation, and an interactive web UI. The backend is written entirely in **C** and serves a REST API over raw TCP sockets. The frontend is a **React + Vite** app that polls the backend for live updates.

***

## ✨ Features

- 🔍 **BFS-based Slot Recommendation** — Recommends the nearest available slot using Breadth-First Search on a graph model of each floor
- 🗺️ **Interactive Parking Grid** — Visual floor map with real-time slot status (available / occupied / recommended)
- 🚗 **Park & Exit Operations** — Park a vehicle to a specific slot or exit using vehicle number
- 📊 **Analytics Dashboard** — Occupancy rates, usage counts, and floor-level statistics
- 🏗️ **Multi-Floor Support** — Independent graph per floor with its own entry/exit nodes
- 💾 **Persistent State** — Parking state saved to file and restored on server restart
- 🔄 **Auto-Refresh** — Frontend polls backend every 3 seconds for live updates
- 🗺️ **Layout Visualizer** — Interactive map showing BFS path from entry to recommended slot

***

## 🛠️ Tech Stack

| Layer | Technology |
|-------|-----------|
| Backend | C (GCC), Raw TCP Sockets, BFS Algorithm |
| Frontend | React 18, Vite, Axios, Lucide Icons |
| Data Format | JSON over HTTP/1.1 |
| State Persistence | Plain text file (`data/parking_state.txt`) |
| Build | `compile.bat` (Windows) |

***

## 📁 Project Structure

```
ParkingSystemAdvanced/
│
├── src/
│   ├── core/
│   │   ├── graph.c / graph.h          # Graph data structure (nodes, edges, adjacency list)
│   │   ├── floor.c / floor.h          # Floor model (loads config, manages slots)
│   │   └── parking_system.c / .h      # Top-level system (manages all floors + stats)
│   │
│   ├── algorithms/
│   │   ├── pathfinding.c / .h         # BFS shortest path implementation
│   │   └── recommendation.c / .h     # Multi-criteria slot scoring & ranking
│   │
│   ├── server/
│   │   └── http_server.c / .h         # Raw HTTP server (GET/POST routing, JSON responses)
│   │
│   └── io/
│       ├── file_manager.c / .h        # Save/load parking state
│       └── json_export.c / .h        # Analytics JSON generation
│
├── config/
│   ├── floor_0_config.txt             # Ground floor graph definition
│   └── floor_1_config.txt             # First floor graph definition
│
├── data/
│   ├── parking_state.txt              # Persisted slot occupancy state
│   └── analytics.json                 # Generated analytics output
│
├── frontend/
│   ├── src/
│   │   ├── pages/
│   │   │   ├── LandingPage.jsx        # Home screen with stats & action selection
│   │   │   └── VisualizationPage.jsx  # Interactive layout map
│   │   ├── components/
│   │   │   ├── ParkingGrid.jsx        # Floor slot grid
│   │   │   ├── FloorTabs.jsx          # Floor switcher
│   │   │   ├── RecommendationBanner.jsx # BFS recommendation UI
│   │   │   ├── Analytics.jsx          # Stats panel
│   │   │   ├── ParkModal.jsx          # Park vehicle dialog
│   │   │   └── ExitModal.jsx          # Exit vehicle dialog
│   │   └── App.jsx                    # Main app with routing & state management
│   ├── package.json
│   └── vite.config.js
│
├── main.c                             # Entry point — initializes system & starts server
└── compile.bat                        # Windows build script
```

***

## ⚙️ How It Works

### Graph Model

Each parking floor is represented as a **weighted directed graph**:

- **Nodes**: `ENTRY`, `EXIT`, `ROAD`, `JUNCTION`, `SLOT`
- **Edges**: Weighted by Manhattan distance between nodes
- **Slot Types**: `STANDARD`, `COMPACT`, `LARGE`, `DISABLED`

Slot IDs follow the format: `F{floor}-R{row}S{column}` (e.g., `F0-R2S3`)

### BFS Pathfinding

When recommending a slot, the backend:

1. Runs BFS from the floor's entry node to every available slot
2. Scores each slot using a weighted formula:
   - **80%** — Entry distance (shorter = better)
   - **15%** — Exit distance
   - **5%** — Floor occupancy rate
3. Returns the top 5 results sorted by score
4. The frontend displays the best match in the recommendation banner

### Floor Config Format

Each floor is defined in a plain-text config file with two sections:

```
[NODES]
ENTRY 0 0
ROAD  0 1
SLOT  1 1 R1S1 STANDARD
...

[EDGES]
# from_node  to_node  weight  bidirectional(1/0)
0 1 1 1
1 2 1 1
...
```

Edge weights are **automatically recalculated** as Manhattan distance (`|Δx| + |Δy|`) when the config is loaded, regardless of the value written in the file.

***

## 🚀 Getting Started

### Prerequisites

- **GCC** (MinGW on Windows recommended)
- **Node.js** v18+ and **npm**
- Windows OS (for `compile.bat`; Linux users can adapt the compile command)

### 1. Clone the Repository

```bash
git clone https://github.com/your-username/parking-system-advanced.git
cd parking-system-advanced
```

### 2. Build the Backend

```bash
./compile.bat
```

This compiles all C source files and produces `parking_system.exe`.

### 3. Install Frontend Dependencies

```bash
cd frontend
npm install
npm run build
cd ..
```

> The Vite build output goes into `frontend/dist/`, which the C server serves statically.

### 4. Start the Server

```bash
./parking_system.exe
```

The server starts on **http://localhost:8080**.

### 5. Open the App

Navigate to **http://localhost:8080** in your browser.

***

## 📡 API Reference

| Method | Endpoint | Description |
|--------|----------|-------------|
| `GET` | `/api/state` | Full parking state (all floors, all slots) |
| `GET` | `/api/analytics` | Occupancy stats and usage analytics |
| `GET` | `/api/recommend?vehicle_type=1&floor=0` | BFS-based slot recommendations for a floor |
| `GET` | `/api/path?slot_id=F0-R2S3` | BFS path waypoints from entry to a slot |
| `POST` | `/api/park` | Park a vehicle in a slot |
| `POST` | `/api/exit` | Free a slot and exit a vehicle |

### POST `/api/park` — Request Body

```json
{
  "slot_id": "F0-R2S3",
  "vehicle_number": "KA01AB1234",
  "vehicle_type": 1
}
```

### POST `/api/exit` — Request Body

```json
{
  "slot_id": "F0-R2S3"
}
```

### Vehicle Type Codes

| Code | Type |
|------|------|
| `0` | Compact |
| `1` | Standard |
| `2` | Large |

***

## 🔧 Configuration

### Adding a New Floor

1. Create `config/floor_2_config.txt` following the node/edge format above
2. In `main.c`, add:
   ```c
   parking_system_add_floor(ps, "Second Floor", "config/floor_2_config.txt");
   ```
3. Recompile with `compile.bat`

### Changing Slot Types

Edit the config file for the relevant floor. Each `SLOT` line accepts:
`SLOT  x  y  RowSlotID  STANDARD|COMPACT|LARGE|DISABLED`

***

## ⚠️ Known Limitations

- The HTTP server is **single-threaded** — one request is handled at a time
- Vehicle number validation is minimal (any non-empty string is accepted)
- No authentication or user session management
- Windows-only build script (Linux/macOS requires a custom `Makefile`)
- The `DISABLED` slot type is reserved and not assignable to any vehicle

***

## 📄 License

This project is for educational and personal use. Feel free to fork and adapt.

***

*Built for Navami Apartments — Multi-Level Smart Parking System*
