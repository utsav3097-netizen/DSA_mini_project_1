# ⚡ Electric Vehicle Charging Slot Manager

> **Data Structures & Algorithms (DSA) College Project**  
> **Language:** Standard C (C99 / C11)  
> **Course:** Data Structures & Algorithms Lab  

---

## 📌 1. Project Overview & Problem Statement

Modern college and corporate campuses face high demand for electric vehicle (EV) charging stations with a limited number of physical docks. This project implements an automated, menu-driven **Electric Vehicle Charging Slot Manager** in standard C.

### Key Objectives:
1. **Dynamic Slot Assignment**: Automatically allocates a physical charging dock if available upon vehicle arrival.
2. **Circular Queue Buffering**: When all physical docks are occupied, arriving vehicles enter a fixed-size Circular Waiting Queue.
3. **Automatic FIFO Dequeue & Dispatch**: Releasing a charging dock immediately dequeues the next waiting vehicle and assigns it to the vacated dock.
4. **Case-Insensitive Duplicate Prevention**: Prevents duplicate Vehicle Registration IDs across **both** active charging docks and the waiting queue.
5. **Arbitrary Queue Cancellation**: Allows any vehicle in the waiting line to cancel its booking, shifting remaining vehicles forward using modular arithmetic while strictly preserving FIFO order.
6. **Live ASCII Visualization**: A terminal-based status dashboard displaying dock occupancy, circular queue state pointers (`front`, `rear`, `count`), and physical circular array wrap-around.

---

## 🏗️ 2. Core Data Structures & Architecture

```
+-----------------------------------------------------------------------------------+
|                        CAMPUS EV CHARGING SYSTEM ARCHITECTURE                     |
+-----------------------------------------------------------------------------------+
                                   |
                         [Arriving Vehicle]
                                   |
                +------------------v-------------------+
                | Duplicate ID Check (Case-Insensitive)|
                | Across Active Slots & Circular Queue |
                +------------------+-------------------+
                                   |
                   +---------------+---------------+
                   | Unique                        | Duplicate
                   v                               v
         [Slots Available?]                  [REJECT ENTRY]
            /          \
      Yes  /            \ No (All Slots Occupied)
          v              v
  [Assign Free Slot]  [Circular Queue Full?]
                         /          \
                   No   /            \ Yes
                       v              v
            [Enqueue (FIFO Wrap)]   [REJECT: Queue Capacity Full]
```

### 1. `struct Vehicle`
Encapsulates driver and EV attributes:
- `char vehicleID[20]`: Unique registration number (e.g., `"KA-01-EV-2024"`).
- `char ownerName[50]`: Full owner name (supports multi-word input with spaces).
- `int batteryLevel`: Current battery percentage ($1\% - 99\%$).

### 2. `struct ChargingSlot slots[MAX_SLOTS]`
Fixed array representing physical charging docks (default `MAX_SLOTS = 4`):
- `int slotID`: Dock identifier ($1$ to $\text{MAX\_SLOTS}$).
- `int isOccupied`: $0 = \text{Free}$, $1 = \text{Occupied}$.
- `Vehicle currentVehicle`: Active vehicle occupying the dock.

### 3. `struct CircularQueue`
Fixed-size circular array implementing FIFO queuing (default `MAX_QUEUE = 6`):
- `Vehicle items[MAX_QUEUE]`: Internal array buffer.
- `int front`: Index of the head vehicle ($-1$ when empty).
- `int rear`: Index of the tail vehicle ($-1$ when empty).
- `int count`: Number of currently queued vehicles ($0$ to $\text{MAX\_QUEUE}$).

#### Modular Arithmetic Formulas:
- **Enqueue**: $\text{rear} = (\text{rear} + 1) \pmod{\text{MAX\_QUEUE}}$
- **Dequeue**: $\text{front} = (\text{front} + 1) \pmod{\text{MAX\_QUEUE}}$
- **Queue Shift on Cancellation**:
  $$\text{curr} = (\text{front} + j) \pmod{\text{MAX\_QUEUE}}$$
  $$\text{next} = (\text{front} + j + 1) \pmod{\text{MAX\_QUEUE}}$$
- **Rear Pointer Decrement**:
  $$\text{rear} = (\text{rear} - 1 + \text{MAX\_QUEUE}) \pmod{\text{MAX\_QUEUE}}$$

---

## ⏱️ 3. Time & Space Complexity Analysis

| Function / Operation | Time Complexity | Auxiliary Space | Complexity Explanation |
| :--- | :---: | :---: | :--- |
| `initQueue()` / `initSlots()` | $O(M)$ | $O(1)$ | Resets slot array and queue indices. |
| `isQueueEmpty()` / `isQueueFull()` | $O(1)$ | $O(1)$ | Direct comparison of `count`. |
| `enqueue()` | $O(1)$ | $O(1)$ | Direct modular index insertion. |
| `dequeue()` | $O(1)$ | $O(1)$ | Direct modular index extraction. |
| `isDuplicateID()` | $O(S + Q)$ | $O(1)$ | Scans $S$ active docks and $Q$ queued elements. |
| `cancelWaitingRequest()` | $O(Q)$ | $O(1)$ | Searches and shifts up to $Q \le \text{MAX\_QUEUE}$ elements. |
| `releaseSlot()` (Auto-Dispatch) | $O(1)$ | $O(1)$ | $O(1)$ dock release + $O(1)$ queue dequeue. |
| `displaySystemStatus()` | $O(S + Q)$ | $O(1)$ | Iterates over docks and queue for dual visualization. |

---

## 📂 4. File Organization

```
ev_charging_manager/
│
├── ev_charging_manager.c   # Complete, single-file C implementation (C99/C11)
├── ev_charging_manager.exe # Pre-compiled executable binary
├── README.md               # Project documentation and viva guide
│
└── .vscode/                # VS Code workspace automation
    ├── tasks.json          # Build task using GCC compiler
    ├── launch.json         # Debug configuration for GDB
    └── settings.json       # Interactive terminal execution settings
```

---

## 🚀 5. How to Compile & Run

### In VS Code (Recommended)
1. Open **VS Code** and go to **File $\rightarrow$ Open Folder...**
2. Select the `ev_charging_manager` folder.
3. Open the integrated terminal: press **`Ctrl` + `~`** *(or **Terminal $\rightarrow$ New Terminal**)*.
4. Run the program:
   ```powershell
   .\ev_charging_manager.exe
   ```

### Command Line (GCC / MinGW)
```bash
gcc -std=c99 -Wall -Wextra -pedantic ev_charging_manager.c -o ev_charging_manager.exe
.\ev_charging_manager.exe
```

### Visual Studio (MSVC Command Prompt)
```cmd
cl /W4 /std:c11 ev_charging_manager.c
ev_charging_manager.exe
```

---

## 🎯 6. Step-by-Step Viva / Evaluation Demonstration

Follow this sequence to demonstrate all algorithmic edge cases during project evaluation:

### Step 1: Load Sample Test Dataset (Option 6)
- **Action**: Enter `6`.
- **Result**: Fills all $4$ physical charging slots and enqueues $3$ vehicles in the circular waiting queue (`front = 0`, `rear = 2`, `count = 3`).

### Step 2: Show Visual ASCII Dashboard (Option 4)
- **Action**: Enter `4`.
- **Result**: Displays:
  - Table of $4$ physical charging docks (all marked `OCCUPIED`).
  - Queue pointers: `FRONT = 0 | REAR = 2 | COUNT = 3 / 6`.
  - Physical circular buffer array cells: `[0: Active] [1: Active] [2: Active] [3: EMPTY] [4: EMPTY] [5: EMPTY]`.
  - Logical FIFO queue table.

### Step 3: Test Duplicate ID Rejection (Option 1)
- **Action**: Enter `1`.
- **Test with**: `ev-ka-01-1001` *(lowercase test for vehicle in Slot 1)*.
- **Result**: Rejected with:
  ```
  [X] REJECTED: Duplicate ID! Vehicle 'ev-ka-01-1001' is ALREADY charging in Slot #1.
  ```
- **Test with**: `EV-GJ-01-5005` *(vehicle waiting in queue)*.
- **Result**: Rejected with:
  ```
  [X] REJECTED: Duplicate ID! Vehicle 'EV-GJ-01-5005' is ALREADY waiting in Queue (Position #1).
  ```

### Step 4: Cancel a Waiting Vehicle in Middle of Queue (Option 3)
- **Action**: Enter `3`.
- **Enter ID**: `EV-UP-16-6006` *(vehicle at array index `[1]`)*.
- **Result**: Removed cleanly. Subsequent vehicle (`EV-TS-09-7007`) is shifted forward to index `[1]`. `rear` safely moves from `2` to `1`. FIFO order is preserved.

### Step 5: Release Slot & Trigger Auto-Dispatch (Option 2)
- **Action**: Enter `2`.
- **Enter Slot**: `1`.
- **Result**:
  1. Dr. Alan Turing departs Slot #1 (charge completed: 100%).
  2. Next queued vehicle (`EV-GJ-01-5005` - John von Neumann) is dequeued from index `[0]` and automatically assigned to Slot #1.
  3. Circular queue `front` pointer advances to `[1]`.

### Step 6: Verify Final State (Option 4)
- **Action**: Enter `4`.
- **Result**: Confirm Slot #1 now holds the auto-dispatched vehicle and the queue pointers reflect the updated FIFO state.

---

## 🛡️ 7. Edge Cases Handled

1. **Buffer Overflow & Newline Flushes**: `readLine()` discards leftover characters from `stdin` and strips trailing `\r\n`.
2. **Invalid Input Protection**: `readInteger()` safely parses inputs using `strtol()`, preventing infinite loops when non-numeric text is entered.
3. **Case-Insensitive String Comparison**: Custom `caseInsensitiveCompare()` works across all operating systems without platform-dependent POSIX dependencies.
4. **Circular Queue Reset**: Resets `front = -1` and `rear = -1` whenever `count` reaches $0$ (either by dequeue or cancellation).
5. **Battery Validation**: Strictly enforces EV battery percentage between $1\%$ and $99\%$.

---

## 👨‍💻 Project Information

- **Project Title**: Electric Vehicle Charging Slot Manager
- **Course**: Data Structures & Algorithms
- **Author**: Utsava Patil
- **Year**: 2026
