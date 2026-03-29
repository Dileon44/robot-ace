# RWLock (Shared Mutex)

## General

- LOCK_NO (unlocked): processes can write/read data to/from resource (data consistency warning)
- LOCK_SHARED (shared lock): multiple processes can read data from resource, but not write (same as mux.RLock() in golang)
- LOCK_EXCLUSIVE (exclusive lock): processes can do nothing with resource (full lock)

The most simplified scheme of the rw mutex relation is as follows:

```mermaid
---
title: Global States
---

stateDiagram
    direction LR

    LOCK_NO --> LOCK_SHARED: SharedLock()
    LOCK_SHARED --> LOCK_NO: SharedUnlock()

    LOCK_SHARED --> LOCK_SHARED: SharedLock() / SharedUnlock()

    LOCK_NO --> LOCK_EXCLUSIVE: ExclusiveLock()
    LOCK_EXCLUSIVE --> LOCK_NO: ExclusiveUnlock()

    LOCK_SHARED --> LOCK_EXCLUSIVE: ExclusiveLock()
```

---

## Synchronization Callbacks

- SharedMutex uses event-based synchronization via two callbacks: `WaitEvent` and `SignalEvent`
- These functions can be NULL — then locking is instant (success if free, busy if locked)

```c
SharedMutex_t mutex;

// No blocking — instant success or ERR_BUSY
SharedMutex_Init(&mutex, NULL, NULL, NULL);

// FreeRTOS example
SemaphoreHandle_t sem = xSemaphoreCreateBinary();
SharedMutex_Init(&mutex, sem, MyWaitEvent, MySignalEvent);

// Bare-metal example (custom polling implementation)
SharedMutex_Init(&mutex, &myFlag, MyBareWaitEvent, MyBareSignalEvent);
```

- When using with RTOS functions, you can not use locks inside critical sections. If a resource is locked and the WaitEvent function is running, the program will hang in an infinite loop
- If SharedMutex is used before the start of the OS scheduler and there are no errors in the user code, there will be no locks in a single-threaded environment and the WaitEvent() function will not be called at all

---

## Shared Lock

- Setting the LOCK_SHARED state prohibits actions that modify the resource
- The resource becomes read-only, and it can be read by several processes at the same time
- Each reader performs SharedLock() locking, retrieves the necessary data and returns possession of the resource by calling SharedUnlock()
- To control access of different processes, locks and unlocks within the resource are counted; a zero count means that all processes have released possession of the resource
- The LOCK_EXCLUSIVE state is a fully locking state and SharedLock() will wait for an event until the exclusive lock is released
- A state change request LOCK_SHARED -> LOCK_EXCLUSIVE blocks new read subscribers by setting the RejectNewReaders flag

```mermaid
---
title: LOCK_NO ↔ LOCK_SHARED states on SharedLock()/SharedUnlock()
---

flowchart LR
    classDef state fill:#7f0

    LOCK_NO((LOCK_NO)):::state
    LOCK_SHARED((LOCK_SHARED)):::state
    LOCK_EXCLUSIVE((LOCK_EXCLUSIVE)):::state

    ACTION_S_LOCK_FIRST(["sharedLockCnt = 1"])
    ACTION_S_LOCK_INC_CNT(["sharedLockCnt++"])
    ACTION_S_LOCK_DEC_CNT(["sharedLockCnt--"])
    ACTION_S_UNLOCK_CHECK_CNT{sharedLockCnt == 0 ?}
    ACTION_S_LOCK_CHECK_READERS_PERMISS{New readers\nrejected?}

    subgraph one [LOCK_SHARED]
        ACTION_S_LOCK_FIRST --> LOCK_SHARED -- "SharedLock()" -->
        ACTION_S_LOCK_CHECK_READERS_PERMISS -- NO --> ACTION_S_LOCK_INC_CNT --> LOCK_SHARED
        ACTION_S_LOCK_CHECK_READERS_PERMISS -- YES --> LOCK_SHARED

        LOCK_SHARED -- "SharedUnlock()" --> ACTION_S_LOCK_DEC_CNT --> ACTION_S_UNLOCK_CHECK_CNT
        ACTION_S_UNLOCK_CHECK_CNT -- NO --> LOCK_SHARED

    end

    ACTION_EX_UNLOCK_CHECK_TMO{LOCK_EXCLUSIVE\nis released in\ntimeout?}

    subgraph three [LOCK_EXCLUSIVE]
        LOCK_EXCLUSIVE -- "SharedUnlock()" --> LOCK_EXCLUSIVE
        LOCK_EXCLUSIVE -- "SharedLock()" --> ACTION_EX_UNLOCK_CHECK_TMO
        ACTION_EX_UNLOCK_CHECK_TMO -- NO --> LOCK_EXCLUSIVE
        ACTION_EX_UNLOCK_CHECK_TMO -- YES --> ACTION_S_LOCK_FIRST
    end

    subgraph two [LOCK_NO]
        LOCK_NO -- "SharedLock()" --> ACTION_S_LOCK_FIRST
        ACTION_S_UNLOCK_CHECK_CNT -- "YES" --> LOCK_NO

        LOCK_NO -- "SharedUnlock()" --> LOCK_NO
    end
```

---

## Exclusive Lock

- Setting the LOCK_EXCLUSIVE state is a complete locking of the resource, prohibiting any read and write operations
- When setting LOCK_EXCLUSIVE lock, the first action is to lock LOCK_SHARED, and then wait until the number of read subscribers drops to 1 - so, only the current lock remains; once this happens, the LOCK_EXCLUSIVE state is set

```mermaid
---
title: LOCK_NO/LOCK_SHARED ↔ LOCK_EXCLUSIVE states on ExclusiveLock()/ExclusiveUnlock()
---

flowchart LR
    classDef state fill:#7f0

    LOCK_NO((LOCK_NO)):::state
    LOCK_SHARED((LOCK_SHARED)):::state
    LOCK_EXCLUSIVE((LOCK_EXCLUSIVE)):::state

    ACTION_READERS_REJECT(["rejectNewReaders = true"])
    ACTION_READERS_ACCEPT(["rejectNewReaders = false"])

    subgraph two [LOCK_EXCLUSIVE]
        ACTION_READERS_REJECT -- "SharedLock()" --> ACTION_S_LOCK_IS_OK
        ACTION_S_LOCK_IS_OK -- "NO" --> ACTION_TIMEOUT_READERS_REJECT -- "NO" --> ACTION_S_LOCK_IS_OK
        ACTION_TIMEOUT_READERS_REJECT -- "YES" --> ACTION_READERS_ACCEPT
        ACTION_S_LOCK_IS_OK -- "YES" --> LOCK_EXCLUSIVE

        LOCK_EXCLUSIVE -- "ExclusiveUnlock()" --> ACTION_READERS_ACCEPT
    end

    subgraph one [LOCK_NO or LOCK_SHARED]
        ACTION_READERS_ACCEPT -- "SharedUnlock()" --> LOCK_NO
        LOCK_SHARED -- "ExclusiveLock()" --> ACTION_READERS_REJECT
        LOCK_NO -- "ExclusiveLock()" --> ACTION_READERS_REJECT
    end

    ACTION_S_LOCK_IS_OK{"concurrentReaders == 1?\n(wait for others LOCK_SHARED's\nbe released)"}
    ACTION_TIMEOUT_READERS_REJECT{"rejectNewReaders\nawait timeouted?"}

```

---

## External references

- [Golang Mutexes — What Is RWMutex For?](https://medium.com/bootdotdev/golang-mutexes-what-is-rwmutex-for-5360ab082626)
- [Mutexes and RWMutex in Golang](https://dev.to/cristicurteanu/mutexes-and-rwmutex-in-golang-4ij)
