#include "EventQueue.h"

// Define the actual instances here - this allocates the memory
SHARED_MEMORY M4EventQueue m4EventQueue;    // M4 processes this queue
SHARED_MEMORY M7EventQueue m7EventQueue;    // M7 processes this queue

void DualCoreEventQueue::initialize() {
  head = 0;
  tail = 0;
  count = 0;
  mutex = 0;
  
  // Initialize all event items
  for (int i = 0; i < EVENT_QUEUE_SIZE; i++) {
    events[i].type = EVENT_NONE;
    events[i].text[0] = '\0';
    events[i].read = true;
  }
}

bool DualCoreEventQueue::push(EventType eventType, const char* eventMsg) {
  // Lock mutex using atomic operation
  while(__atomic_exchange_n(&mutex, 1, __ATOMIC_ACQUIRE) != 0);
  
  // Queue is full
  if (count >= EVENT_QUEUE_SIZE) {
    __atomic_store_n(&mutex, 0, __ATOMIC_RELEASE);
    return false;
  }
  
  // Store event type
  events[head].type = eventType;
  events[head].read = false;
  
  // Copy event message if provided
  if (eventMsg != nullptr) {
    strncpy(events[head].text, eventMsg, EVENT_MESSAGE_SIZE - 1);
    events[head].text[EVENT_MESSAGE_SIZE - 1] = '\0';
  } else {
    events[head].text[0] = '\0';
  }
  
  // Update pointers
  head = (head + 1) % EVENT_QUEUE_SIZE;
  count++;
  
  // Unlock mutex
  __atomic_store_n(&mutex, 0, __ATOMIC_RELEASE);
  return true;
}

bool DualCoreEventQueue::pop(EventType& eventType, char* eventBuffer) {
  // Lock mutex
  while(__atomic_exchange_n(&mutex, 1, __ATOMIC_ACQUIRE) != 0);
  
  // Queue is empty
  if (count == 0) {
    __atomic_store_n(&mutex, 0, __ATOMIC_RELEASE);
    return false;
  }
  
  // Get event type
  eventType = events[tail].type;
  
  // Copy event message if buffer provided
  if (eventBuffer != nullptr) {
    strncpy(eventBuffer, events[tail].text, EVENT_MESSAGE_SIZE);
  }
  
  // Mark as read
  events[tail].read = true;
  
  // Update pointers
  tail = (tail + 1) % EVENT_QUEUE_SIZE;
  count--;
  
  // Unlock mutex
  __atomic_store_n(&mutex, 0, __ATOMIC_RELEASE);
  return true;
}

bool DualCoreEventQueue::isEmpty() const {
  return count == 0;
}

uint8_t DualCoreEventQueue::getCount() const {
  return count;
}


// EventBroadcaster Implementation
bool EventBroadcaster::sendToM4(EventType eventType, const char* eventData) {
    return m4EventQueue.push(eventType, eventData);
}

bool EventBroadcaster::sendToM7(EventType eventType, const char* eventData) {
    return m7EventQueue.push(eventType, eventData);
}

bool EventBroadcaster::broadcastEvent(EventType eventType, const char* eventData) {
    // Send to both queues - return true only if both succeed
    bool m4Success = m4EventQueue.push(eventType, eventData);
    bool m7Success = m7EventQueue.push(eventType, eventData);
    return m4Success && m7Success;
}

uint8_t EventBroadcaster::getM4QueueCount() {
    return m4EventQueue.getCount();
}

uint8_t EventBroadcaster::getM7QueueCount() {
    return m7EventQueue.getCount();
}

