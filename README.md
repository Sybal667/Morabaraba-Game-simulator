# Morabaraba Game

## Overview
This project is a simulation of the traditional **Morabaraba** board game implemented in C++.  
The game is played between two automated algorithms:

- **Alg_1** → Random selection strategy
- **Alg_2** → Heuristic-based AI strategy

The implementation follows the standard Morabaraba rules, including:

- Placement Phase
- Movement Phase
- Mill detection and removals
- Win and draw conditions

The **Flying Phase** is intentionally excluded to reduce the possibility of endless draws.

---

# Game Description

The game is played on a board consisting of:

- **3 concentric squares**
- **8 intersection points per square**
- **24 total board positions**

Each player places and moves cows strategically to form **mills** (three cows in a straight line).

When a player forms a mill:
- One opponent cow is removed.
- Preference is given to cows not already in mills.

The game ends when:
- A player has fewer than **3 cows remaining**
- Or a draw condition is met

---

# Algorithms

## Alg_1 — Random Strategy
This algorithm:
- Selects random valid placements
- Selects random valid moves
- Primarily used for testing game functionality

---

## Alg_2 — Heuristic AI
This algorithm evaluates board positions using:
- Number of cows remaining
- Potential mill formations
- Blocking opponent mills
- Center square control
- Immediate mill opportunities

### Heuristic Bonuses
| Action | Bonus |
|---|---|
| Immediate mill during placement | +500 |
| Immediate mill during movement | +1000 |

---

# Project Structure

```text
Project Folder/
├── Morabaraba.h          # Header file with class definitions
├── Morabaraba.cpp        # Game logic and algorithm implementation
├── main.cpp              # Main program and file I/O
├── input.txt             # User-created input file
└── morabarabaResults.txt # Generated output file
```

---

# How to Run

## Method 1 — Using MinGW (GCC Compiler)

### Step 1
Install MinGW:

https://sourceforge.net/projects/mingw/

### Step 2
Add `MinGW/bin` to your system PATH.

### Step 3
Open Command Prompt in the project directory.

### Step 4 — Compile

```bash
g++ -std=c++11 Morabaraba.cpp main.cpp -o Morabaraba.exe
```

### Step 5
Create an `input.txt` file.

### Step 6 — Run

```bash
Morabaraba.exe
```

---

## Method 2 — Using Code::Blocks

1. Open Code::Blocks
2. Create a new **Console Application**
3. Add all source files to the project
4. Build and Run using **F9**

---

# Input File Format

Create a file named:

```text
input.txt
```

Each line should contain the number of cows per player.

## Example

```text
6
9
12
15
```

---

# Output File

The program automatically generates:

```text
morabarabaResults.txt
```

The output file contains:
- Every move made by both algorithms
- Mill formations
- Cow removals
- Remaining cow counts
- Final game result

Possible outcomes:
- `Alg_1 wins`
- `Alg_2 wins`
- `It's a draw`

---

# Game Constraints

## Board Constraints

- 3 concentric squares
- 8 positions per square
- 24 total positions
- Movement only along connected adjacent lines

---

## Placement Phase

- Players alternately place cows
- Placement only allowed on empty positions

---

## Movement Phase

- Begins after all cows are placed
- Players move cows to adjacent positions only

---

## Flying Phase

Not included in this implementation.

---

# Mill Rules

A mill is formed when:
- A player aligns 3 cows in a straight line

When a mill is formed:
- One opponent cow is removed
- Cows not in mills are prioritized
- If all opponent cows are in mills, any cow may be removed

---

# Loop Prevention Rule

To prevent infinite loops:
- A player cannot immediately reform the same mill that was just broken

---

# Win Conditions

A player wins when:
- The opponent has fewer than 3 cows remaining

---

# Draw Conditions

A draw occurs when:
- Both players have exactly 3 cows remaining
- This state persists for 10 consecutive moves

---

# Features

- Full Morabaraba game simulation
- Automated gameplay
- Random and heuristic AI agents
- Mill detection system
- File-based input/output
- Win and draw detection
- Strategic AI evaluation

---

# Technologies Used

- C++
- Object-Oriented Programming
- File Handling
- Heuristic Search Strategies

---

# Example Compilation

```bash
g++ -std=c++11 Morabaraba.cpp main.cpp -o Morabaraba.exe
```

---


Developed as part of an academic project on algorithmic game simulation and AI strategy implementation.
