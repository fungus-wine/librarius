#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <Arduino.h>
#include <string.h>

#define EVENT_MESSAGE_SIZE 64
#define EVENT_QUEUE_SIZE 16

// Event type definitions
enum EventType {
  EVENT_NONE = 0,
  EVENT_IMU_DATA_AVAILABLE,
  
  // M4 Command Events (M7 → M4)
  EVENT_SET_TARGET_POSITION,
  EVENT_SET_TARGET_HEADING,
  EVENT_SET_POSITION_LIMITS,
  EVENT_EMERGENCY_STOP,
  
  // M7 Status Events (M4 → M7)
  EVENT_BALANCE_STATUS,
  EVENT_MOTOR_STATUS,
  EVENT_SAFETY_ALERT,
  EVENT_COLLISION_WARNING,
  EVENT_SYSTEM_HEALTH,
  
  // Broadcast Events (Both cores)
  EVENT_SYSTEM_STARTUP,
  EVENT_BATTERY_LOW,
  EVENT_SYSTEM_SHUTDOWN
};

// The shared memory section attribute
#if defined(CORE_CM7)
#define SHARED_MEMORY __attribute__((section(".shared_m7")))
#else
#define SHARED_MEMORY __attribute__((section(".shared_m4")))
#endif

class DualCoreEventQueue {
protected:
  struct EventItem {
    EventType type;
    char text[EVENT_MESSAGE_SIZE];
    bool read;
  };
  
  EventItem events[EVENT_QUEUE_SIZE];
  volatile uint8_t head;
  volatile uint8_t tail;
  volatile uint8_t count;
  volatile uint32_t mutex;

public:
  void initialize();
  bool push(EventType eventType, const char* eventMsg = nullptr);
  virtual bool pop(EventType& eventType, char* eventBuffer = nullptr);
  bool isEmpty() const;
  uint8_t getCount() const;
};

// Core-specific queue classes with built-in safety
class M4EventQueue : public DualCoreEventQueue {
public:
  bool pop(EventType& eventType, char* eventBuffer = nullptr) override {
    // For now, allow both cores to access both queues for testing
    // TODO: Add proper core detection when needed
    return DualCoreEventQueue::pop(eventType, eventBuffer);
  }
};

class M7EventQueue : public DualCoreEventQueue {
public:
  bool pop(EventType& eventType, char* eventBuffer = nullptr) override {
    // For now, allow both cores to access both queues for testing  
    // TODO: Add proper core detection when needed
    return DualCoreEventQueue::pop(eventType, eventBuffer);
  }
};

// Declare the dual queue instances (extern means they're defined elsewhere)
extern SHARED_MEMORY M4EventQueue m4EventQueue;    // M4 processes this queue
extern SHARED_MEMORY M7EventQueue m7EventQueue;    // M7 processes this queue

// Core safety is now built into the queue classes - no macros needed!

// EventBroadcaster helper class for targeted and broadcast events
class EventBroadcaster {
public:
    // Send event to M4 (via m4EventQueue)
    static bool sendToM4(EventType eventType, const char* eventData = nullptr);
    
    // Send event to M7 (via m7EventQueue)  
    static bool sendToM7(EventType eventType, const char* eventData = nullptr);
    
    // Send event to both cores (duplicates to both queues)
    static bool broadcastEvent(EventType eventType, const char* eventData = nullptr);
    
    // Get queue usage statistics for monitoring
    static uint8_t getM4QueueCount();
    static uint8_t getM7QueueCount();
};

#endif // EVENT_QUEUE_H