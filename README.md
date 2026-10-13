# CS2 Real-Time Bomb Timer & Overlay

A lightweight, high-performance Game State Integration (GSI) system for Counter-Strike 2. This project decouples game-state processing from presentation by pairing a **C++ backend server** with a **Python transparent GUI overlay** using UDP Inter-Process Communication (IPC).

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue)
![Python](https://img.shields.io/badge/Python-3.x-yellow)
![Protocol](https://img.shields.io/badge/IPC-UDP%20Socket-orange)
![Game](https://img.shields.io/badge/Game-CS2%20GSI-red)

---

## Architecture Overview

The system uses a **Producer-Consumer / Client-Server** pattern designed for low latency and zero impact on in-game performance.

1. **CS2 Game Engine:** Broadcasts state updates as JSON payloads to a local HTTP endpoint.
2. **C++ Backend (`cpp_backend`):** 
   - Runs an asynchronous `httplib` HTTP server to intercept payloads.
   - Safely parses partial game updates using `nlohmann::json`.
   - Spawns background worker threads (`std::thread`) with a drift-corrected timer loop using `std::chrono::sleep_until`.
   - Broadcasts real-time events to the frontend via lightweight UDP datagrams.
3. **Python Frontend (`python_gui`):** 
   - Listens on a non-blocking UDP socket.
   - Renders a borderless, transparent, "always-on-top" Tkinter overlay synchronized with game events (`TIME`, `EXPLODED`, `DEFUSED`, `CANCEL`).

---

## Features

- **Real-Time Synchronized Timer:** Drift-compensated countdown loop accounts for system processing delays and `sleep_for` accumulation error.
- **Asynchronous & Non-Blocking:** High-frequency network packets are handled without freezing the HTTP listener or causing frame drops in-game.
- **Robust State Engine:** Handles edge cases including premature CT/T elimination, round phase changes, defuse interrupts, and partial JSON packet payloads.
- **Zero In-Game Memory Injection:** Reads official Valve GSI HTTP POST broadcasts rather than reading process memory, making it 100% safe against VAC (Valve Anti-Cheat).

---

## Tech Stack & Dependencies

### C++ Backend
- **Standard:** C++17
- **HTTP Server:** [cpp-httplib](https://github.com/yhirose/cpp-httplib)
- **JSON Parsing:** [nlohmann/json](https://github.com/nlohmann/json)
- **Networking:** Windows Sockets 2 (`ws2_32`)
- **Concurrency:** `std::thread`, `std::atomic`, `std::chrono`

### Python Overlay
- **GUI:** `tkinter` (Standard Library)
- **Networking:** `socket`, `threading`

---

## Project Structure

```text
.
├── main.cpp                 # C++ Server listener, GSI logic, thread manager & UDP sender
├── cs2_timer_gui.py         # Python transparent overlay GUI & UDP socket receiver
├── gamestate_integration_timer.cfg # CS2 GSI configuration file
├── httplib.h                # C++ HTTP library header
└── json.hpp                 # C++ JSON library header
