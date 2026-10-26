/**
 * ============================================================================
 * Project Title : Electric Vehicle Charging Slot Manager
 * Course/Subject: Data Structures & Algorithms (DSA)
 * Language      : Standard C (C99 / C11)
 * Architecture  : Menu-driven Campus EV Management System
 * Author        : DSA Project Architecture Team
 * ============================================================================
 * 
 * DESCRIPTION:
 * A comprehensive EV charging management system designed for campus charging 
 * infrastructure with a fixed number of charging points and a circular waiting
 * queue. Demonstrates arrays of structures, circular queues with modular arithmetic,
 * case-insensitive duplicate prevention, arbitrary queue cancellation with FIFO
 * preservation, and automatic dequeue assignment upon slot release.
 * 
 * CORE DATA STRUCTURES:
 * 1. struct Vehicle      : Encapsulates Vehicle ID, Owner Name, and Battery %.
 * 2. struct ChargingSlot : Fixed array representing physical charging stations.
 * 3. struct CircularQueue: Fixed-size circular array demonstrating modular wrap-around.
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ============================================================================
 * CONFIGURATION CONSTANTS & LIMITS
 * ============================================================================ */
#define MAX_SLOTS      4     /* Total physical charging points on campus */
#define MAX_QUEUE      6     /* Maximum capacity of waiting circular queue */
#define ID_LEN        20     /* Maximum length for Vehicle ID string (incl '\0') */
#define NAME_LEN      50     /* Maximum length for Owner Name string (incl '\0') */
#define BUFFER_SIZE  128     /* General input line buffer size */

/* ============================================================================
 * DATA STRUCTURE DEFINITIONS
 * ============================================================================ */

/**
 * struct Vehicle
 * --------------
 * Represents an individual electric vehicle arriving at the charging station.
 * 
 * Memory Layout:
 * - vehicleID   : Alphanumeric license plate or unique code (e.g., "KA-01-EV-2024")
 * - ownerName   : Full name of the vehicle owner (handles spaces, e.g., "Alan Turing")
 * - batteryLevel: Current battery percentage (valid range: 1% to 99%)
 */
typedef struct {
    char vehicleID[ID_LEN];
    char ownerName[NAME_LEN];
    int batteryLevel;
} Vehicle;

/**
 * struct ChargingSlot
 * -------------------
 * Represents a physical charging dock on the campus.
 * 
 * - slotID        : 1-indexed dock number (1 to MAX_SLOTS)
 * - isOccupied    : 0 = Free/Available, 1 = Currently Charging
 * - currentVehicle: Vehicle currently occupying this slot (valid when isOccupied == 1)
 */
typedef struct {
    int slotID;
    int isOccupied;
    Vehicle currentVehicle;
} ChargingSlot;

/**
 * struct CircularQueue
 * --------------------
 * Circular Queue implementation using a fixed array.
 * 
 * Invariants:
 * - When Empty: count == 0, front == -1, rear == -1
 * - When 1 Item: count == 1, front == rear (any valid index 0 to MAX_QUEUE - 1)
 * - When Full : count == MAX_QUEUE, rear == (front - 1 + MAX_QUEUE) % MAX_QUEUE
 * - Enqueue   : rear = (rear + 1) % MAX_QUEUE
 * - Dequeue   : front = (front + 1) % MAX_QUEUE
 */
typedef struct {
    Vehicle items[MAX_QUEUE];
    int front;
    int rear;
    int count;
} CircularQueue;

/* ============================================================================
 * FUNCTION DECLARATIONS (PROTOTYPES)
 * ============================================================================ */

/* String & Input Utilities */
void trimWhitespace(char *str);
int readLine(char *buffer, int maxLen);
int readInteger(const char *prompt, int minVal, int maxVal);
int caseInsensitiveCompare(const char *s1, const char *s2);

/* Circular Queue Operations */
void initQueue(CircularQueue *q);
int isQueueEmpty(const CircularQueue *q);
int isQueueFull(const CircularQueue *q);
int enqueue(CircularQueue *q, Vehicle v);
int dequeue(CircularQueue *q, Vehicle *outVehicle);
int cancelWaitingRequest(CircularQueue *q, const char *id, Vehicle *outVehicle);

/* Slot & Station Operations */
void initSlots(ChargingSlot slots[], int maxSlots);
int findFreeSlotIndex(const ChargingSlot slots[], int maxSlots);
int isDuplicateID(const ChargingSlot slots[], int maxSlots, const CircularQueue *q, const char *id);
int searchVehicleLocation(const ChargingSlot slots[], int maxSlots, const CircularQueue *q, const char *id);

/* High-Level User Actions */
void handleVehicleArrival(ChargingSlot slots[], int maxSlots, CircularQueue *q);
void handleReleaseSlot(ChargingSlot slots[], int maxSlots, CircularQueue *q);
void handleCancelWaiting(CircularQueue *q, const ChargingSlot slots[], int maxSlots);
void displaySystemStatus(const ChargingSlot slots[], int maxSlots, const CircularQueue *q);
void searchVehicleDetails(const ChargingSlot slots[], int maxSlots, const CircularQueue *q);
void loadDemoScenario(ChargingSlot slots[], int maxSlots, CircularQueue *q);

/* ============================================================================
 * INPUT & STRING UTILITIES IMPLEMENTATION
 * ============================================================================ */

/**
 * trimWhitespace()
 * ----------------
 * Strips leading and trailing whitespace, carriage returns, and newlines in-place.
 * 
 * Time Complexity : O(L) where L = length of string
 * Space Complexity: O(1)
 */
void trimWhitespace(char *str) {
    if (!str) return;

    /* Trim leading whitespace */
    char *start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }

    /* Trim trailing whitespace */
    int len = (int)strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[len - 1] = '\0';
        len--;
    }
}

/**
 * readLine()
 * ----------
 * Robust console line reader using fgets(). Eliminates leftover trailing newlines
 * and flushes any overflowing characters remaining in stdin buffer.
 * 
 * Time Complexity : O(maxLen)
 * Space Complexity: O(1)
 */
int readLine(char *buffer, int maxLen) {
    if (!fgets(buffer, maxLen, stdin)) {
        buffer[0] = '\0';
        return 0;
    }

    /* Check if newline was captured */
    char *newlinePos = strchr(buffer, '\n');
    if (newlinePos) {
        *newlinePos = '\0';
    } else {
        /* Line exceeded buffer; flush remaining characters from stdin */
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {}
    }

    trimWhitespace(buffer);
    return (int)strlen(buffer);
}

/**
 * readInteger()
 * -------------
 * Reads and validates an integer from stdin within a specified inclusive range [minVal, maxVal].
 * Prevents infinite loops and invalid parsing when non-numeric input is supplied.
 * 
 * Time Complexity : O(1) amortized per user attempt
 * Space Complexity: O(BUFFER_SIZE) stack
 */
int readInteger(const char *prompt, int minVal, int maxVal) {
    char line[BUFFER_SIZE];
    long val;
    char *endptr;

    while (1) {
        printf("%s", prompt);
        if (!readLine(line, sizeof(line)) || line[0] == '\0') {
            printf("  [!] Error: Input cannot be empty. Please enter an integer between %d and %d.\n", minVal, maxVal);
            continue;
        }

        val = strtol(line, &endptr, 10);
        if (*endptr != '\0') {
            printf("  [!] Error: Invalid numeric input '%s'. Please enter numbers only.\n", line);
            continue;
        }

        if (val < minVal || val > maxVal) {
            printf("  [!] Error: Value %ld is out of valid range [%d - %d].\n", val, minVal, maxVal);
            continue;
        }

        return (int)val;
    }
}

/**
 * caseInsensitiveCompare()
 * ------------------------
 * Standard-compliant case-insensitive string comparison (pure C99/C11, portable
 * across Windows MSVC, GCC, Clang, and POSIX).
 * 
 * Returns:
 *   0 if s1 and s2 are identical (ignoring case)
 *  <0 if s1 < s2
 *  >0 if s1 > s2
 * 
 * Time Complexity : O(min(len(s1), len(s2)))
 * Space Complexity: O(1)
 */
int caseInsensitiveCompare(const char *s1, const char *s2) {
    if (!s1 || !s2) return (s1 == s2) ? 0 : (s1 ? 1 : -1);

    while (*s1 && *s2) {
        int diff = tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
        if (diff != 0) return diff;
        s1++;
        s2++;
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

/* ============================================================================
 * CIRCULAR QUEUE IMPLEMENTATION
 * ============================================================================ */

/**
 * initQueue()
 * -----------
 * Initializes the circular queue to an empty state.
 * 
 * Time Complexity : O(1)
 * Space Complexity: O(1)
 */
void initQueue(CircularQueue *q) {
    q->front = -1;
    q->rear = -1;
    q->count = 0;
}

/**
 * isQueueEmpty()
 * --------------
 * Checks if circular queue has zero waiting vehicles.
 * 
 * Time Complexity : O(1)
 * Space Complexity: O(1)
 */
int isQueueEmpty(const CircularQueue *q) {
    return (q->count == 0);
}

/**
 * isQueueFull()
 * -------------
 * Checks if circular queue has reached maximum storage capacity (MAX_QUEUE).
 * 
 * Time Complexity : O(1)
 * Space Complexity: O(1)
 */
int isQueueFull(const CircularQueue *q) {
    return (q->count == MAX_QUEUE);
}

/**
 * enqueue()
 * ---------
 * Inserts a vehicle at the rear of the circular queue using modular arithmetic:
 *   rear = (rear + 1) % MAX_QUEUE
 * 
 * Returns:
 *   1 on success
 *   0 if queue is full (overflow condition)
 * 
 * Time Complexity : O(1)
 * Space Complexity: O(1)
 */
int enqueue(CircularQueue *q, Vehicle v) {
    if (isQueueFull(q)) {
        return 0; /* Queue Overflow */
    }

    if (isQueueEmpty(q)) {
        q->front = 0;
        q->rear = 0;
    } else {
        q->rear = (q->rear + 1) % MAX_QUEUE;
    }

    q->items[q->rear] = v;
    q->count++;
    return 1;
}

/**
 * dequeue()
 * ---------
 * Removes the front vehicle from the circular queue using modular arithmetic:
 *   front = (front + 1) % MAX_QUEUE
 * 
 * Resets front = rear = -1 when the last remaining item is dequeued.
 * 
 * Returns:
 *   1 on success (stores extracted vehicle in *outVehicle)
 *   0 if queue is empty (underflow condition)
 * 
 * Time Complexity : O(1)
 * Space Complexity: O(1)
 */
int dequeue(CircularQueue *q, Vehicle *outVehicle) {
    if (isQueueEmpty(q)) {
        return 0; /* Queue Underflow */
    }

    if (outVehicle) {
        *outVehicle = q->items[q->front];
    }

    q->front = (q->front + 1) % MAX_QUEUE;
    q->count--;

    if (q->count == 0) {
        q->front = -1;
        q->rear = -1;
    }

    return 1;
}

/**
 * cancelWaitingRequest()
 * ----------------------
 * Cancels an arbitrary vehicle's waiting request from anywhere inside the circular queue.
 * Locates the vehicle by ID, shifts all subsequent queued elements forward by one slot
 * using modular indices `(curr + 1) % MAX_QUEUE`, safely decrements `rear` via
 * `(rear - 1 + MAX_QUEUE) % MAX_QUEUE`, and resets `front = rear = -1` if count drops to 0.
 * 
 * Strict FIFO order of all remaining waiting vehicles is strictly preserved.
 * 
 * Returns:
 *   1 if vehicle was found and successfully removed
 *   0 if vehicle ID not found in waiting queue
 * 
 * Time Complexity : O(N) where N = q->count <= MAX_QUEUE (At most MAX_QUEUE iterations)
 * Space Complexity: O(1)
 */
int cancelWaitingRequest(CircularQueue *q, const char *id, Vehicle *outVehicle) {
    if (isQueueEmpty(q)) {
        return 0;
    }

    int foundRelativePos = -1;

    /* Step 1: Scan for the vehicle in logical queue order (0 to count - 1) */
    for (int i = 0; i < q->count; i++) {
        int physicalIdx = (q->front + i) % MAX_QUEUE;
        if (caseInsensitiveCompare(q->items[physicalIdx].vehicleID, id) == 0) {
            foundRelativePos = i;
            if (outVehicle) {
                *outVehicle = q->items[physicalIdx];
            }
            break;
        }
    }

    if (foundRelativePos == -1) {
        return 0; /* Not found in queue */
    }

    /* Step 2: Shift subsequent queued elements forward by one position */
    for (int j = foundRelativePos; j < q->count - 1; j++) {
        int currIdx = (q->front + j) % MAX_QUEUE;
        int nextIdx = (q->front + j + 1) % MAX_QUEUE;
        q->items[currIdx] = q->items[nextIdx];
    }

    /* Step 3: Safely adjust rear pointer backward using modular arithmetic */
    q->rear = (q->rear - 1 + MAX_QUEUE) % MAX_QUEUE;
    q->count--;

    /* Step 4: If queue is now completely empty, reset pointers to -1 */
    if (q->count == 0) {
        q->front = -1;
        q->rear = -1;
    }

    return 1;
}

/* ============================================================================
 * CHARGING SLOTS & DUPLICATE DETECTION LOGIC
 * ============================================================================ */

/**
 * initSlots()
 * -----------
 * Initializes physical charging slots to free status.
 * 
 * Time Complexity : O(maxSlots)
 * Space Complexity: O(1)
 */
void initSlots(ChargingSlot slots[], int maxSlots) {
    for (int i = 0; i < maxSlots; i++) {
        slots[i].slotID = i + 1;
        slots[i].isOccupied = 0;
        slots[i].currentVehicle.vehicleID[0] = '\0';
        slots[i].currentVehicle.ownerName[0] = '\0';
        slots[i].currentVehicle.batteryLevel = 0;
    }
}

/**
 * findFreeSlotIndex()
 * -------------------
 * Searches for the lowest available free charging slot index.
 * 
 * Returns:
 *   Index (0 to maxSlots - 1) of first free slot
 *  -1 if all slots are occupied
 * 
 * Time Complexity : O(maxSlots)
 * Space Complexity: O(1)
 */
int findFreeSlotIndex(const ChargingSlot slots[], int maxSlots) {
    for (int i = 0; i < maxSlots; i++) {
        if (!slots[i].isOccupied) {
            return i;
        }
    }
    return -1;
}

/**
 * isDuplicateID()
 * ---------------
 * Checks if a given Vehicle ID already exists anywhere in the system:
 * 1. Scans all active/occupied charging slots.
 * 2. Iterates across all `count` items currently in the waiting circular queue.
 * Comparison is completely case-insensitive (e.g. "ev-01" matches "EV-01").
 * 
 * Returns:
 *   1 if duplicate is found
 *   0 if unique
 * 
 * Time Complexity : O(MAX_SLOTS + MAX_QUEUE)
 * Space Complexity: O(1)
 */
int isDuplicateID(const ChargingSlot slots[], int maxSlots, const CircularQueue *q, const char *id) {
    /* 1. Check active charging slots */
    for (int i = 0; i < maxSlots; i++) {
        if (slots[i].isOccupied) {
            if (caseInsensitiveCompare(slots[i].currentVehicle.vehicleID, id) == 0) {
                return 1;
            }
        }
    }

    /* 2. Check circular queue elements */
    if (!isQueueEmpty(q)) {
        for (int i = 0; i < q->count; i++) {
            int idx = (q->front + i) % MAX_QUEUE;
            if (caseInsensitiveCompare(q->items[idx].vehicleID, id) == 0) {
                return 1;
            }
        }
    }

    return 0;
}

/**
 * searchVehicleLocation()
 * -----------------------
 * Searches where a vehicle is currently located.
 * 
 * Returns:
 *   Slot Number (1 to maxSlots) if currently charging
 *   -1 * (queuePosition) (e.g., -1 for 1st in queue, -2 for 2nd) if in queue
 *   0 if not present in the system
 * 
 * Time Complexity : O(MAX_SLOTS + MAX_QUEUE)
 * Space Complexity: O(1)
 */
int searchVehicleLocation(const ChargingSlot slots[], int maxSlots, const CircularQueue *q, const char *id) {
    for (int i = 0; i < maxSlots; i++) {
        if (slots[i].isOccupied && caseInsensitiveCompare(slots[i].currentVehicle.vehicleID, id) == 0) {
            return slots[i].slotID;
        }
    }

    for (int i = 0; i < q->count; i++) {
        int idx = (q->front + i) % MAX_QUEUE;
        if (caseInsensitiveCompare(q->items[idx].vehicleID, id) == 0) {
            return -(i + 1); /* Negative indicates queue position */
        }
    }

    return 0;
}

/* ============================================================================
 * HIGH-LEVEL SYSTEM OPERATIONS
 * ============================================================================ */

/**
 * handleVehicleArrival()
 * ----------------------
 * Registers an arriving EV:
 * - Collects Vehicle ID, Owner Name, and Battery Level (1-99%).
 * - Validates uniqueness against both active slots and waiting queue.
 * - If a free slot exists, assigns it immediately.
 * - Otherwise, places the vehicle into the circular queue.
 * - If both slots and queue are full, displays capacity overflow message.
 * 
 * Time Complexity : O(MAX_SLOTS + MAX_QUEUE)
 * Space Complexity: O(BUFFER_SIZE) stack
 */
void handleVehicleArrival(ChargingSlot slots[], int maxSlots, CircularQueue *q) {
    Vehicle v;
    char tempID[BUFFER_SIZE];
    char tempName[BUFFER_SIZE];

    printf("\n=======================================================\n");
    printf("              VEHICLE ARRIVAL & REGISTRATION            \n");
    printf("=======================================================\n");

    /* 1. Prompt for Vehicle ID */
    while (1) {
        printf(" Enter Vehicle ID (License/Reg #): ");
        if (!readLine(tempID, sizeof(tempID)) || tempID[0] == '\0') {
            printf("  [!] Error: Vehicle ID cannot be empty. Please retry.\n");
            continue;
        }

        if (strlen(tempID) >= ID_LEN) {
            printf("  [!] Error: Vehicle ID too long (max %d chars). Please retry.\n", ID_LEN - 1);
            continue;
        }

        /* Duplicate Check across both Slots & Queue */
        if (isDuplicateID(slots, maxSlots, q, tempID)) {
            int loc = searchVehicleLocation(slots, maxSlots, q, tempID);
            if (loc > 0) {
                printf("  [X] REJECTED: Duplicate ID! Vehicle '%s' is ALREADY charging in Slot #%d.\n", tempID, loc);
            } else {
                printf("  [X] REJECTED: Duplicate ID! Vehicle '%s' is ALREADY waiting in Queue (Position #%d).\n", tempID, -loc);
            }
            return;
        }
        break;
    }
    strncpy(v.vehicleID, tempID, ID_LEN - 1);
    v.vehicleID[ID_LEN - 1] = '\0';

    /* 2. Prompt for Owner Name (allows spaces) */
    while (1) {
        printf(" Enter Owner Name: ");
        if (!readLine(tempName, sizeof(tempName)) || tempName[0] == '\0') {
            printf("  [!] Error: Owner Name cannot be empty. Please retry.\n");
            continue;
        }

        if (strlen(tempName) >= NAME_LEN) {
            printf("  [!] Error: Owner Name too long (max %d chars). Please retry.\n", NAME_LEN - 1);
            continue;
        }
        break;
    }
    strncpy(v.ownerName, tempName, NAME_LEN - 1);
    v.ownerName[NAME_LEN - 1] = '\0';

    /* 3. Prompt for Battery Percentage (1% - 99%) */
    v.batteryLevel = readInteger(" Enter Current Battery Level (1% - 99%): ", 1, 99);

    /* 4. Slot Assignment or Queue Insertion */
    int freeIdx = findFreeSlotIndex(slots, maxSlots);

    if (freeIdx != -1) {
        /* Free slot available: Immediate assignment */
        slots[freeIdx].isOccupied = 1;
        slots[freeIdx].currentVehicle = v;

        printf("\n [OK] SUCCESS: Immediate Charging Slot Assigned!\n");
        printf("      -> Vehicle ID   : %s\n", v.vehicleID);
        printf("      -> Owner Name   : %s\n", v.ownerName);
        printf("      -> Battery Level: %d%%\n", v.batteryLevel);
        printf("      -> Assigned Dock: Slot #%d\n", slots[freeIdx].slotID);
    } else {
        /* All slots occupied: Check Circular Queue */
        if (isQueueFull(q)) {
            printf("\n [X] ADMISSION FAILED: System at Absolute Full Capacity!\n");
            printf("      All %d Charging Slots are OCCUPIED.\n", maxSlots);
            printf("      Waiting Queue is FULL (%d/%d vehicles).\n", q->count, MAX_QUEUE);
            printf("      Please try again after a charging vehicle departs.\n");
        } else {
            enqueue(q, v);
            printf("\n [i] NOTICE: All %d charging slots are currently occupied.\n", maxSlots);
            printf("      -> Vehicle '%s' placed in Circular Waiting Queue.\n", v.vehicleID);
            printf("      -> Queue Position   : %d of %d\n", q->count, MAX_QUEUE);
            printf("      -> Array Slot Index : [%d] (rear pointer)\n", q->rear);
        }
    }
    printf("=======================================================\n");
}

/**
 * handleReleaseSlot()
 * -------------------
 * Releases an occupied charging slot:
 * - Allows user to release either by Slot Number (1-MAX_SLOTS) or Vehicle ID.
 * - Departed vehicle's charging session ends.
 * - Immediately checks if circular waiting queue has vehicles.
 * - If waiting queue has vehicles: automatically dequeues head vehicle,
 *   assigns it to the freed slot, and notifies user.
 * - If queue is empty: marks slot as Free.
 * 
 * Time Complexity : O(1) when selecting by Slot Number, O(MAX_SLOTS) by ID.
 * Space Complexity: O(BUFFER_SIZE) stack
 */
void handleReleaseSlot(ChargingSlot slots[], int maxSlots, CircularQueue *q) {
    char input[BUFFER_SIZE];
    int targetSlotIdx = -1;

    printf("\n=======================================================\n");
    printf("             RELEASE CHARGING SLOT / DEPARTURE          \n");
    printf("=======================================================\n");

    printf(" Enter Slot Number (1 to %d) OR Vehicle ID to release: ", maxSlots);
    if (!readLine(input, sizeof(input)) || input[0] == '\0') {
        printf("  [!] Error: Input cannot be empty.\n");
        return;
    }

    /* Check if input matches a slot number */
    char *endptr;
    long slotNum = strtol(input, &endptr, 10);
    if (*endptr == '\0') {
        if (slotNum >= 1 && slotNum <= maxSlots) {
            targetSlotIdx = (int)slotNum - 1;
        } else {
            printf("  [!] Error: Invalid Slot Number %ld. Must be between 1 and %d.\n", slotNum, maxSlots);
            return;
        }
    } else {
        /* Input is alphanumeric Vehicle ID: search active slots */
        for (int i = 0; i < maxSlots; i++) {
            if (slots[i].isOccupied && caseInsensitiveCompare(slots[i].currentVehicle.vehicleID, input) == 0) {
                targetSlotIdx = i;
                break;
            }
        }
        if (targetSlotIdx == -1) {
            /* Check if vehicle is in waiting queue instead */
            int loc = searchVehicleLocation(slots, maxSlots, q, input);
            if (loc < 0) {
                printf("  [!] Notice: Vehicle '%s' is in Waiting Queue (Pos #%d), not actively charging.\n", input, -loc);
                printf("      Use 'Cancel Waiting Request' (Menu Option 3) to remove it.\n");
            } else {
                printf("  [X] Error: Vehicle '%s' is not found in any active charging slot.\n", input);
            }
            return;
        }
    }

    /* Validate that selected slot is actually occupied */
    if (!slots[targetSlotIdx].isOccupied) {
        printf("  [!] Slot #%d is already FREE! No vehicle to release.\n", slots[targetSlotIdx].slotID);
        return;
    }

    /* Process departure */
    Vehicle departedVehicle = slots[targetSlotIdx].currentVehicle;
    printf("\n [*] DEPARTURE COMPLETED:\n");
    printf("     Slot #%d vacated by:\n", slots[targetSlotIdx].slotID);
    printf("     - Vehicle ID   : %s\n", departedVehicle.vehicleID);
    printf("     - Owner Name   : %s\n", departedVehicle.ownerName);
    printf("     - Start Battery: %d%% -> Fully Charged: 100%%\n", departedVehicle.batteryLevel);

    /* Automatic Assignment from Circular Queue */
    if (!isQueueEmpty(q)) {
        Vehicle nextVehicle;
        int prevFront = q->front;
        dequeue(q, &nextVehicle);

        slots[targetSlotIdx].isOccupied = 1;
        slots[targetSlotIdx].currentVehicle = nextVehicle;

        printf("\n [->] AUTO-ASSIGNMENT TRIGGERED (FIFO Queue Dispatch):\n");
        printf("     Waiting Vehicle Dequeued from Circular Buffer Index [%d]!\n", prevFront);
        printf("     - Vehicle ID   : %s\n", nextVehicle.vehicleID);
        printf("     - Owner Name   : %s\n", nextVehicle.ownerName);
        printf("     - Battery Level: %d%%\n", nextVehicle.batteryLevel);
        printf("     - Newly Assigned to: Slot #%d\n", slots[targetSlotIdx].slotID);
        printf("     -> Remaining waiting vehicles: %d\n", q->count);
        if (q->count > 0) {
            printf("     -> New Queue Head (Front): Index [%d], Tail (Rear): Index [%d]\n", q->front, q->rear);
        } else {
            printf("     -> Waiting queue is now EMPTY (Front = -1, Rear = -1).\n");
        }
    } else {
        slots[targetSlotIdx].isOccupied = 0;
        slots[targetSlotIdx].currentVehicle.vehicleID[0] = '\0';
        slots[targetSlotIdx].currentVehicle.ownerName[0] = '\0';
        slots[targetSlotIdx].currentVehicle.batteryLevel = 0;
        printf("\n [i] No waiting vehicles in circular queue. Slot #%d is now FREE.\n", slots[targetSlotIdx].slotID);
    }
    printf("=======================================================\n");
}

/**
 * handleCancelWaiting()
 * ---------------------
 * Cancels a waiting request for any vehicle currently held in the circular queue.
 * Preserves strict FIFO order of remaining queue items.
 * 
 * Time Complexity : O(MAX_QUEUE)
 * Space Complexity: O(BUFFER_SIZE) stack
 */
void handleCancelWaiting(CircularQueue *q, const ChargingSlot slots[], int maxSlots) {
    char targetID[BUFFER_SIZE];
    Vehicle cancelledVehicle;

    printf("\n=======================================================\n");
    printf("             CANCEL WAITING REQUEST (QUEUE)             \n");
    printf("=======================================================\n");

    if (isQueueEmpty(q)) {
        printf("  [!] The waiting queue is currently empty. Nothing to cancel.\n");
        return;
    }

    printf(" Enter Vehicle ID to cancel from waiting queue: ");
    if (!readLine(targetID, sizeof(targetID)) || targetID[0] == '\0') {
        printf("  [!] Error: Vehicle ID cannot be empty.\n");
        return;
    }

    /* Check if vehicle is actually charging in a slot instead */
    for (int i = 0; i < maxSlots; i++) {
        if (slots[i].isOccupied && caseInsensitiveCompare(slots[i].currentVehicle.vehicleID, targetID) == 0) {
            printf("  [!] Notice: Vehicle '%s' is actively charging in Slot #%d.\n", targetID, slots[i].slotID);
            printf("      To release a charging vehicle, use Menu Option 2 ('Release Slot').\n");
            return;
        }
    }

    int success = cancelWaitingRequest(q, targetID, &cancelledVehicle);
    if (success) {
        printf("\n [OK] CANCELLATION SUCCESSFUL:\n");
        printf("      - Cancelled Vehicle ID : %s\n", cancelledVehicle.vehicleID);
        printf("      - Owner Name           : %s\n", cancelledVehicle.ownerName);
        printf("      - Battery Level        : %d%%\n", cancelledVehicle.batteryLevel);
        printf("      Subsequent queue elements have been shifted forward by 1.\n");
        printf("      -> Updated Queue Count : %d / %d\n", q->count, MAX_QUEUE);
        if (q->count > 0) {
            printf("      -> Updated Pointers    : Front = [%d], Rear = [%d]\n", q->front, q->rear);
        } else {
            printf("      -> Queue is now EMPTY  : Front = -1, Rear = -1\n");
        }
    } else {
        printf("  [X] Vehicle ID '%s' was not found in the waiting queue.\n", targetID);
    }
    printf("=======================================================\n");
}

/**
 * displaySystemStatus()
 * ---------------------
 * Renders an ASCII dashboard showing:
 * 1. Physical Charging Slots table with status, vehicle details, and battery.
 * 2. Circular Queue pointer values (front, rear, count).
 * 3. Physical Array buffer visualization showing circular wrap-around.
 * 4. Logical FIFO queue listing showing queue position and array indices.
 * 
 * Time Complexity : O(MAX_SLOTS + MAX_QUEUE)
 * Space Complexity: O(1)
 */
void displaySystemStatus(const ChargingSlot slots[], int maxSlots, const CircularQueue *q) {
    int occupiedCount = 0;
    for (int i = 0; i < maxSlots; i++) {
        if (slots[i].isOccupied) occupiedCount++;
    }

    printf("\n========================================================================================\n");
    printf("                      ELECTRIC VEHICLE CHARGING STATION DASHBOARD                       \n");
    printf("========================================================================================\n");

    /* 1. Charging Slots Table */
    printf("\n--- [ PHYSICAL CHARGING DOCKS ] --------------------------------------------------------\n");
    printf("+---------+--------------+----------------------+--------------------------------+-----------+\n");
    printf("| Slot #  | Status       | Vehicle ID           | Owner Name                     | Battery %% |\n");
    printf("+---------+--------------+----------------------+--------------------------------+-----------+\n");

    for (int i = 0; i < maxSlots; i++) {
        if (slots[i].isOccupied) {
            printf("| Slot %-2d | OCCUPIED     | %-20s | %-30s |    %2d%%   |\n",
                   slots[i].slotID,
                   slots[i].currentVehicle.vehicleID,
                   slots[i].currentVehicle.ownerName,
                   slots[i].currentVehicle.batteryLevel);
        } else {
            printf("| Slot %-2d | FREE         | %-20s | %-30s |    --     |\n",
                   slots[i].slotID,
                   "--",
                   "--");
        }
    }
    printf("+---------+--------------+----------------------+--------------------------------+-----------+\n");
    printf(" Summary: %d / %d Slots Occupied  |  %d Free\n", occupiedCount, maxSlots, maxSlots - occupiedCount);

    /* 2. Circular Waiting Queue Status */
    printf("\n--- [ WAITING LINE (CIRCULAR QUEUE) ] --------------------------------------------------\n");
    printf(" State Pointers: FRONT = %2d  |  REAR = %2d  |  COUNT = %d / %d (Max Capacity)\n",
           q->front, q->rear, q->count, MAX_QUEUE);

    /* Physical Circular Buffer Visualization */
    printf("\n Physical Array Buffer View [Index 0 to %d]:\n ", MAX_QUEUE - 1);
    for (int idx = 0; idx < MAX_QUEUE; idx++) {
        /* Check if physical index currently holds an active element in circular queue */
        int isActive = 0;
        if (!isQueueEmpty(q)) {
            for (int k = 0; k < q->count; k++) {
                if (((q->front + k) % MAX_QUEUE) == idx) {
                    isActive = 1;
                    break;
                }
            }
        }

        if (isActive) {
            printf("[%d: %s", idx, q->items[idx].vehicleID);
            if (idx == q->front && idx == q->rear) {
                printf(" (F&R)] ");
            } else if (idx == q->front) {
                printf(" (Front)] ");
            } else if (idx == q->rear) {
                printf(" (Rear)] ");
            } else {
                printf("] ");
            }
        } else {
            printf("[%d: EMPTY] ", idx);
        }
    }
    printf("\n");

    /* Logical FIFO Queue Table */
    if (isQueueEmpty(q)) {
        printf("\n Waiting Line is currently EMPTY. No vehicles in queue.\n");
    } else {
        printf("\n Logical FIFO Queue Order:\n");
        printf("+----------+------------+----------------------+--------------------------------+-----------+\n");
        printf("| Position | Arr Index  | Vehicle ID           | Owner Name                     | Battery %% |\n");
        printf("+----------+------------+----------------------+--------------------------------+-----------+\n");
        for (int i = 0; i < q->count; i++) {
            int idx = (q->front + i) % MAX_QUEUE;
            printf("|    %-5d |     %-6d | %-20s | %-30s |    %2d%%   |\n",
                   i + 1,
                   idx,
                   q->items[idx].vehicleID,
                   q->items[idx].ownerName,
                   q->items[idx].batteryLevel);
        }
        printf("+----------+------------+----------------------+--------------------------------+-----------+\n");
    }

    printf("========================================================================================\n");
}

/**
 * searchVehicleDetails()
 * ----------------------
 * Searches for any vehicle by ID and prints comprehensive location details.
 * 
 * Time Complexity : O(MAX_SLOTS + MAX_QUEUE)
 * Space Complexity: O(BUFFER_SIZE) stack
 */
void searchVehicleDetails(const ChargingSlot slots[], int maxSlots, const CircularQueue *q) {
    char targetID[BUFFER_SIZE];

    printf("\n=======================================================\n");
    printf("                    SEARCH VEHICLE                      \n");
    printf("=======================================================\n");

    printf(" Enter Vehicle ID to search: ");
    if (!readLine(targetID, sizeof(targetID)) || targetID[0] == '\0') {
        printf("  [!] Error: Vehicle ID cannot be empty.\n");
        return;
    }

    /* 1. Check Charging Slots */
    for (int i = 0; i < maxSlots; i++) {
        if (slots[i].isOccupied && caseInsensitiveCompare(slots[i].currentVehicle.vehicleID, targetID) == 0) {
            printf("\n [FOUND IN CHARGING SLOT]:\n");
            printf("  -> Status       : Actively Charging\n");
            printf("  -> Dock Slot    : Slot #%d\n", slots[i].slotID);
            printf("  -> Vehicle ID   : %s\n", slots[i].currentVehicle.vehicleID);
            printf("  -> Owner Name   : %s\n", slots[i].currentVehicle.ownerName);
            printf("  -> Battery Level: %d%%\n", slots[i].currentVehicle.batteryLevel);
            printf("=======================================================\n");
            return;
        }
    }

    /* 2. Check Circular Queue */
    for (int i = 0; i < q->count; i++) {
        int idx = (q->front + i) % MAX_QUEUE;
        if (caseInsensitiveCompare(q->items[idx].vehicleID, targetID) == 0) {
            printf("\n [FOUND IN WAITING QUEUE]:\n");
            printf("  -> Status       : Waiting in Circular Queue\n");
            printf("  -> Queue Position: #%d of %d in line (FIFO)\n", i + 1, q->count);
            printf("  -> Array Index  : [%d] in Circular Buffer\n", idx);
            printf("  -> Vehicle ID   : %s\n", q->items[idx].vehicleID);
            printf("  -> Owner Name   : %s\n", q->items[idx].ownerName);
            printf("  -> Battery Level: %d%%\n", q->items[idx].batteryLevel);
            printf("=======================================================\n");
            return;
        }
    }

    printf("\n  [X] Vehicle '%s' was NOT FOUND in active charging slots or waiting queue.\n", targetID);
    printf("=======================================================\n");
}

/**
 * loadDemoScenario()
 * ------------------
 * Populates initial test data quickly so viva evaluators can inspect
 * an active station with occupied slots and a partially filled circular queue.
 * 
 * Time Complexity : O(1)
 * Space Complexity: O(1)
 */
void loadDemoScenario(ChargingSlot slots[], int maxSlots, CircularQueue *q) {
    initSlots(slots, maxSlots);
    initQueue(q);

    /* Sample Vehicles */
    Vehicle v1 = {"EV-KA-01-1001", "Dr. Alan Turing", 25};
    Vehicle v2 = {"EV-MH-12-2002", "Ada Lovelace", 40};
    Vehicle v3 = {"EV-DL-04-3003", "Claude Shannon", 15};
    Vehicle v4 = {"EV-TN-07-4004", "Grace Hopper", 30};
    Vehicle v5 = {"EV-GJ-01-5005", "John von Neumann", 55};
    Vehicle v6 = {"EV-UP-16-6006", "Katherine Johnson", 20};
    Vehicle v7 = {"EV-TS-09-7007", "Linus Torvalds", 45};

    /* Assign to slots 1-4 */
    slots[0].isOccupied = 1; slots[0].currentVehicle = v1;
    slots[1].isOccupied = 1; slots[1].currentVehicle = v2;
    slots[2].isOccupied = 1; slots[2].currentVehicle = v3;
    slots[3].isOccupied = 1; slots[3].currentVehicle = v4;

    /* Enqueue into circular queue */
    enqueue(q, v5);
    enqueue(q, v6);
    enqueue(q, v7);

    printf("\n [OK] Pre-loaded Demo Scenario Loaded Successfully!\n");
    printf("      - 4 Physical Slots Occupied (Slots 1 to 4)\n");
    printf("      - 3 Vehicles Enqueued in Circular Waiting Line (Count = 3, Front = 0, Rear = 2)\n");
    printf("      View Option 4 to see the visual dashboard.\n");
}

/* ============================================================================
 * MAIN APPLICATION ENTRY POINT & MENU LOOP
 * ============================================================================ */

int main(void) {
    ChargingSlot slots[MAX_SLOTS];
    CircularQueue queue;

    /* Initialize system state */
    initSlots(slots, MAX_SLOTS);
    initQueue(&queue);

    printf("\n****************************************************************\n");
    printf("*      WELCOME TO CAMPUS EV CHARGING SLOT MANAGER (C99)        *\n");
    printf("*      Data Structures: Fixed Array & Modular Circular Queue   *\n");
    printf("****************************************************************\n");

    int choice = 0;
    while (choice != 7) {
        printf("\n=======================================================\n");
        printf("                      MAIN MENU                        \n");
        printf("=======================================================\n");
        printf("  [1] Register Arriving Vehicle (Auto-Assign or Enqueue)\n");
        printf("  [2] Release Charging Slot (Depart & Auto-Dispatch)\n");
        printf("  [3] Cancel Waiting Request from Circular Queue\n");
        printf("  [4] Display System Status (ASCII Visualization)\n");
        printf("  [5] Search Vehicle by Registration ID\n");
        printf("  [6] Load Pre-Configured Demo Test Scenario\n");
        printf("  [7] Exit Program\n");
        printf("=======================================================\n");

        choice = readInteger(" Select an option [1-7]: ", 1, 7);

        switch (choice) {
            case 1:
                handleVehicleArrival(slots, MAX_SLOTS, &queue);
                break;
            case 2:
                handleReleaseSlot(slots, MAX_SLOTS, &queue);
                break;
            case 3:
                handleCancelWaiting(&queue, slots, MAX_SLOTS);
                break;
            case 4:
                displaySystemStatus(slots, MAX_SLOTS, &queue);
                break;
            case 5:
                searchVehicleDetails(slots, MAX_SLOTS, &queue);
                break;
            case 6:
                loadDemoScenario(slots, MAX_SLOTS, &queue);
                break;
            case 7:
                printf("\n Exiting EV Charging Slot Manager. System shutdown complete.\n\n");
                break;
            default:
                printf(" [!] Unknown option. Please retry.\n");
                break;
        }
    }

    return 0;
}
