# Dvaita

Dvaita is a **UCI-compliant chess engine** written from scratch in C. It is designed for performance, featuring a **bitboard board representation** and an efficient **alpha-beta search algorithm**. The engine is built with a modular architecture to allow for continuous development and the integration of more advanced search and evaluation techniques.


---

## ⚡ Core Features

- 💡 **UCI Compliant**: Fully compliant with the Universal Chess Interface, making it compatible with a wide range of popular chess GUIs (Arena, Cute Chess, etc.).
- ♟️ **Bitboard Representation**: Utilizes 64-bit integers for the board state, enabling extremely fast move generation and complex board queries.
- 🧠 **Alpha-Beta Search**: Employs a Negamax framework with alpha-beta pruning to efficiently search the game tree and discard irrelevant variations.
- 🔄 **Iterative Deepening**: A robust search manager that deepens its analysis over time, ensuring a good move is always ready, even under tight time controls.
- ⚖️ **Tapered Evaluation**: A sophisticated evaluation function with separate Piece-Square Tables (PSTs) for the middlegame and endgame, allowing the engine to understand the changing value of pieces throughout the game.
- 🚀 **MVV-LVA Move Ordering**: The search is accelerated by a move ordering heuristic that prioritizes the most promising captures (Most Valuable Victim – Least Valuable Attacker), leading to faster and more effective pruning.

---

## ⚙️ Architectural Overview

- **Modular Design**: The engine is broken into logical components (`search`, `evaluate`, `movegen`, `bitboard`) to promote clean code and facilitate future enhancements.  
- **Zobrist Hashing**: Implements 64-bit Zobrist keys for unique position hashing, laying the groundwork for an advanced transposition table.  
- **Custom Debugging Tools**: Includes custom UCI commands for in-depth analysis and debugging of the search algorithm.  
- **Make-Based Build System**: Uses a simple and effective makefile for easy compilation on Linux-based systems (including WSL).  

---

## 🧩 Custom UCI Commands

| Command     | Description                                                                 |
|-------------|-----------------------------------------------------------------------------|
| `printfen`  | Prints the FEN string of the current internal board state.                  |
| `eval`      | Performs a static evaluation of the current position and prints the score.  |
| (Debug)     | Prints per-move node counts at the root during a search for analysis.       |

---

## 🧪 In Development (Roadmap)

Planned features to significantly increase the engine’s playing strength:

- **Quiescence Search**: Improve tactical accuracy and mitigate horizon effect.  
- **Transposition Table**: Memory for search, reducing redundant calculations and increasing depth.  
- **Advanced Move Ordering**:  
  - Hash Move: Prioritize the best move from the transposition table.  
  - Killer Moves: Remember quiet moves that have caused cutoffs.  
- **Opening Book**: Provide instant, theory-backed moves for the opening.  
- **Multithreading**: Parallelized search leveraging multi-core CPUs.  

---

## ⚠️ How to Build and Run

Dvaita is ready to be compiled and used with any UCI-compatible GUI.

### Prerequisites
- A C compiler (e.g., `gcc`)  
- `make`  

### Build Instructions
Navigate to the root directory of the project and run:

```bash
make
```

This will create an executable file named **`chess_engine`** in the root folder.

### Running the Engine

**1. Command Line:**
```bash
./chess_engine
```
The engine will start and wait for UCI commands.

**2. With a GUI (Recommended):**
1. Open your favorite chess GUI (e.g., Arena).  
2. Find the option to install a new engine.  
3. When prompted, select the `chess_engine` executable file.  
4. The GUI will now be able to use Dvaita to play and analyze games.  

**To clean the build files, run:**
```bash
make clean
```
