#ifndef INSTINCUS_KIT_EVENT_QUEUE_H
#define INSTINCUS_KIT_EVENT_QUEUE_H

#include <Arduino.h>
#include <string.h>
#include <stm32h7xx_hal_hsem.h>

// ============================================================================
// HARDWARE SEMAPHORE (HSEM) IDS
// ============================================================================
// The STM32H747 provides 32 hardware semaphores for inter-core synchronization.
// We use one per queue so that push/pop on different queues don't block each other.
// HSEM IDs 0–1 are sometimes used by OpenAMP/libmetal; we start at 2 to avoid
// conflicts.  If you add more shared resources, assign them consecutive IDs.
#define HSEM_ID_M4_QUEUE  2   // protects m4EventQueue (M7→M4 commands)
#define HSEM_ID_M7_QUEUE  3   // protects m7EventQueue (M4→M7 status)

// ============================================================================
// QUEUE SIZING
// ============================================================================
// Both queue objects must fit in D3 SRAM before OpenAMP (see EventQueue.cpp).
// Available space: 0x38000000 → 0x38000400 = 1024 bytes total.
//
// Per-queue memory footprint (approximate, ARM 32-bit, no vtable):
//   EventItem:          sizeof(EventType)=4 + char[MSG_SIZE] + bool=1 + pad=3
//   DualCoreEventQueue: EventItem[QUEUE_SIZE] + head/tail/count(3) + pad(1)
//
// With EVENT_QUEUE_SIZE=6, EVENT_MESSAGE_SIZE=48:
//   EventItem    = 4 + 48 + 1 + 3  = 56 bytes
//   Queue struct = (6 × 56) + 3 + 1 = 340 bytes
//   Two queues   = 680 bytes  ← safely below 1024
//
// A static_assert in EventQueue.cpp will catch it if these grow too large.
// To increase capacity you must use D2 SRAM instead (see EventQueue.cpp).
#define EVENT_MESSAGE_SIZE 48
#define EVENT_QUEUE_SIZE    6

// ============================================================================
// EVENT TYPE DEFINITIONS
// ============================================================================
enum EventType {
  EVENT_NONE = 0,
  EVENT_IMU_DATA_AVAILABLE,

  // M4 Command Events (M7 → M4)
  EVENT_SET_TARGET_POSITION,
  EVENT_SET_TARGET_HEADING,
  EVENT_SET_POSITION_LIMITS,
  EVENT_EMERGENCY_STOP,

  // M7 Status Events (M4 → M7)
  EVENT_BALANCE_IMU_DATA,
  EVENT_TOF_DATA,
  EVENT_PROXIMITY_WARNING,

  // Broadcast Events (Both cores)
  EVENT_SYSTEM_STARTUP,
  EVENT_BATTERY_LOW,
  EVENT_SYSTEM_SHUTDOWN
};

// ============================================================================
// DUAL-CORE EVENT QUEUE
// ============================================================================
class DualCoreEventQueue {
protected:
  struct EventItem {
    EventType type;              // What kind of event
    char      text[EVENT_MESSAGE_SIZE]; // Payload (null-terminated string)
    bool      read;              // True once popped; used only for diagnostics
  };

  EventItem        events[EVENT_QUEUE_SIZE];
  volatile uint8_t head;   // Index where next push() writes
  volatile uint8_t tail;   // Index where next pop() reads
  volatile uint8_t count;  // Number of unconsumed items

  uint8_t hsemId;          // Which HSEM instance protects this queue.
                           // Set by initialize(); not volatile because each
                           // queue's ID is written once and never changes.

public:
  // initialize() MUST be called by exactly one core (M7) before any push/pop.
  // It zeros all counters and assigns the HSEM ID to put the queue in a
  // known-good state.  Because the queue lives in D3 SRAM (not BSS),
  // startup code will NOT zero it.
  void initialize(uint8_t semId);

  // push() and pop() are the only thread-safe entry points.
  // They acquire/release the hardware semaphore before touching shared state.
  bool push(EventType eventType, const char* eventMsg = nullptr);
  bool pop(EventType& eventType, char* eventBuffer = nullptr);

  bool    isEmpty()  const;
  uint8_t getCount() const;
};

// ============================================================================
// CORE-SPECIFIC QUEUE SUBCLASSES
// ============================================================================
// These exist so call sites can express intent ("this is an M4 queue") and
// to leave a clean hook for future per-core access enforcement.
//
// IMPORTANT: pop() is intentionally NOT virtual here.
//
// A virtual method stores a vtable pointer at byte 0 of every instance.
// Each core's vtable pointer is a flash address in *that core's* flash region:
//   M7 vtable ptr → somewhere in M7 flash (0x08040000+)
//   M4 vtable ptr → somewhere in M4 flash (CM4_BINARY_START+)
//
// If the queue lives in shared RAM, the core that "constructs" (or
// reinterpret_cast-initializes) the object sets byte 0 to its own flash
// address.  When the other core tries a virtual dispatch, it dereferences
// that foreign address — which is either code or unmapped — and hard-faults.
//
// Making pop() non-virtual means no vtable pointer is stored in shared RAM.
// The compiler resolves M4EventQueue::pop() at compile time via a direct
// (non-virtual) call to DualCoreEventQueue::pop().
//
// Ref: ARM IHI0042F "C++ ABI for ARM Architecture", §2.3 (vtable layout)
// Ref: https://itanium-cxx-abi.github.io/cxx-abi/abi.html#vtable (Itanium ABI,
//      same vtable-at-offset-0 rule used by ARM EABI)

class M4EventQueue : public DualCoreEventQueue {
public:
  // Non-virtual: compiler resolves call statically; no vtable in shared RAM.
  // TODO: add CORE_CM4 assert here when we want stricter access control.
  bool pop(EventType& eventType, char* eventBuffer = nullptr) {
    return DualCoreEventQueue::pop(eventType, eventBuffer);
  }
};

class M7EventQueue : public DualCoreEventQueue {
public:
  // Non-virtual: same reasoning as M4EventQueue::pop() above.
  // TODO: add CORE_CM7 assert here when we want stricter access control.
  bool pop(EventType& eventType, char* eventBuffer = nullptr) {
    return DualCoreEventQueue::pop(eventType, eventBuffer);
  }
};

// ============================================================================
// QUEUE INSTANCES
// ============================================================================
// The queues live at fixed absolute addresses in D3 SRAM (0x38000000).
// See EventQueue.cpp for the full explanation of why, and the memory map.
//
// These are *references*, not objects.  No object is constructed here; the
// underlying memory is treated as pre-existing storage that M7 initializes
// via initialize() before M4 is booted.  Both cores use the same addresses.
extern M4EventQueue& m4EventQueue;  // M4 reads commands from this queue
extern M7EventQueue& m7EventQueue;  // M7 reads status/events from this queue

// ============================================================================
// EVENT BROADCASTER
// ============================================================================
// Convenience helpers so callers don't have to know which queue to use.
class EventBroadcaster {
public:
  // Push an event into M4's queue (M7 calls this to send commands to M4)
  static bool sendToM4(EventType eventType, const char* eventData = nullptr);

  // Push an event into M7's queue (M4 calls this to send status to M7)
  static bool sendToM7(EventType eventType, const char* eventData = nullptr);

  // Push to both queues (e.g. EVENT_EMERGENCY_STOP so both cores react)
  static bool broadcastEvent(EventType eventType, const char* eventData = nullptr);

  // Queue depth diagnostics
  static uint8_t getM4QueueCount();
  static uint8_t getM7QueueCount();
};

#endif // EVENT_QUEUE_H
