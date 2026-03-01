#include "EventQueue.h"

// ============================================================================
// WHY THE QUEUES LIVE AT FIXED ABSOLUTE ADDRESSES IN D3 SRAM
// ============================================================================
//
// SHORT ANSWER
// ------------
// The STM32H747's D3-domain SRAM (RAM_D3, starting at 0x38000000) is the only
// SRAM region that is:
//   (a) accessible by BOTH cores at the SAME physical address, and
//   (b) guaranteed non-cacheable on M7 (required for OpenAMP, proven by the
//       fact that OpenAMP uses this region and works correctly).
//
// LONG ANSWER
// -----------
//
// FAILED APPROACH #1: __attribute__((section(".shared_m7/.shared_m4")))
//
//   The original code used a SHARED_MEMORY macro that expanded to
//   __attribute__((section(".shared_m7"))) on M7 and
//   __attribute__((section(".shared_m4"))) on M4.
//
//   Neither section name is defined in either Arduino Giga R1 linker script:
//     variants/GIGA/linker_script.ld               (M7)
//     variants/GENERIC_STM32H747_M4/linker_script.ld (M4)
//
//   When GNU LD encounters an unknown section name it creates an "orphan
//   section" and places it adjacent to similar sections in the default output
//   region.  Data-like sections end up in each core's normal RAM:
//     M7 orphan (.shared_m7) → AXI SRAM at 0x24000000
//     M4 orphan (.shared_m4) → M4 local RAM at 0x10000000
//
//   These are DIFFERENT PHYSICAL SRAM BANKS.  Each core has its own private
//   copy of the queues with no connection to the other core's copy.
//
//   Additionally, orphan sections are NOT part of either core's .bss region,
//   so the startup zero-fill never runs on them.  The mutex field starts with
//   whatever garbage happened to be in RAM at power-on.  If mutex ≠ 0 when
//   M4 first calls push() or pop(), the spin-lock:
//       while (__atomic_exchange_n(&mutex, 1, __ATOMIC_ACQUIRE) != 0);
//   never exits, and M4 hangs silently with no error output (M4 has no Serial).
//   This was the observed symptom: M4's setup() appeared to complete (two blue
//   LED flashes before RPC.begin()), but loop() never ran.
//
// FAILED APPROACH #2: Custom section in M4's RAM_D2 at 0x10000000
//
//   M4's linker uses RAM_D2 at 0x10000000 for its BSS and heap.
//   M7's linker uses RAM (AXI SRAM) at 0x24000000 for its BSS and heap.
//   M7's RAM_D2 is at 0x30000000 (AHB D2 domain SRAM).
//   None of these overlap, so a shared section in any of them still produces
//   two independent copies.
//
// WHY D3 SRAM WORKS
// -----------------
//   Both linker scripts contain identical entries:
//     RAM_D3 (xrw) : ORIGIN = 0x38000000, LENGTH = 64K
//     __OPENAMP_region_start__ = 0x38000400
//     __OPENAMP_region_end__   = 0x38000400 + LENGTH(RAM_D3) - 1K
//
//   The same physical address (0x38000000) appears in both cores' address
//   spaces — there is no aliasing or remapping.  A write by M7 to 0x38000000
//   is immediately visible to M4 reading from 0x38000000.
//
//   D3 SRAM is outside the M7's AXI bus (where its D-cache operates), so it
//   is non-cacheable by default.  OpenAMP depends on this property; if D3 SRAM
//   were cached, OpenAMP would silently corrupt messages.  We inherit the same
//   guarantee for our queues.
//
//   Ref: STM32H747 RM0399 §2.3 "Memory map and register boundary addresses"
//   Ref: STM32H747 RM0399 §8 "Power control (PWR)" — D3 domain description
//   Ref: OpenAMP project: https://github.com/OpenAMP/open-amp
//   Ref: Arduino Giga linker scripts (both located at):
//        ~/Library/Arduino15/packages/arduino/hardware/mbed_giga/<ver>/variants/
//
// D3 SRAM MEMORY MAP
// ------------------
//   0x38000000  m4EventQueue  ← commands from M7 to M4    (~340 bytes)
//   0x38000154  m7EventQueue  ← status/events from M4 to M7 (~340 bytes)
//   0x38000400  OpenAMP VRING buffers (managed by MBED RPC / libmetal)
//   0x3800FC00  PDM audio buffer (.pdm_section, defined in both linker scripts)
//   0x38010000  End of D3 SRAM (64 KB from base)
//
//   The exact offset of m7EventQueue is sizeof(M4EventQueue) bytes past the
//   base.  A static_assert below verifies both queues fit before 0x38000400.
//
// INITIALIZATION ORDER
// --------------------
//   M7 calls m4EventQueue.initialize(HSEM_ID_M4_QUEUE) and
//   m7EventQueue.initialize(HSEM_ID_M7_QUEUE) in its setup() before calling
//   RPC.begin().  RPC.begin() on M7 is what boots the M4 core (it starts the
//   OpenAMP/RPMsg handshake and releases M4's reset).  So by the time M4's
//   setup() runs, both queues are in a known-good state.
//
//   M4 must NOT call initialize() — it would clobber queued events that M7
//   may have already pushed between M7's setup() and M4 first running.
//
// ============================================================================

// Compile-time guard: if EVENT_QUEUE_SIZE or EVENT_MESSAGE_SIZE is increased,
// the queues may overflow the 1 KB window before OpenAMP at 0x38000400.
// Reduce those constants or migrate to D2 SRAM (see comments above) if this fires.
static_assert(
  sizeof(M4EventQueue) + sizeof(M7EventQueue) <= 0x400,
  "Queue objects too large for D3 SRAM before OpenAMP (0x38000400). "
  "Reduce EVENT_QUEUE_SIZE or EVENT_MESSAGE_SIZE in EventQueue.h, "
  "or move the queues to D2 SRAM and update the addresses below."
);

// References to the queue objects at their fixed D3 SRAM addresses.
//
// reinterpret_cast<T*>(addr) is well-defined for embedded use: we are not
// constructing an object (no constructor runs here), we are treating the
// pre-existing D3 SRAM bytes as storage for a T.  M7's initialize() call
// writes all fields to known values before either core uses the queue.
//
// No vtable pointer is written here because M4/M7EventQueue have no virtual
// methods (see EventQueue.h for the explanation of why virtual is excluded).
M4EventQueue& m4EventQueue = *reinterpret_cast<M4EventQueue*>(0x38000000UL);
M7EventQueue& m7EventQueue = *reinterpret_cast<M7EventQueue*>(0x38000000UL + sizeof(M4EventQueue));


// ============================================================================
// DualCoreEventQueue implementation
// ============================================================================

// initialize() — call once from M7 before RPC.begin() boots M4.
// Zeroes all counters and assigns the HSEM ID to put the queue in a
// known-good state.
// (D3 SRAM is not zeroed by either core's startup code — it is not in .bss.)
void DualCoreEventQueue::initialize(uint8_t semId) {
  head   = 0;
  tail   = 0;
  count  = 0;
  hsemId = semId;

  for (int i = 0; i < EVENT_QUEUE_SIZE; i++) {
    events[i].type    = EVENT_NONE;
    events[i].text[0] = '\0';
    events[i].read    = true;
  }
}

// push() — called by the *sending* core to enqueue an event.
//
// Locking uses the STM32H747 Hardware Semaphore (HSEM) peripheral.
// Each queue has its own HSEM ID (assigned via initialize()) so that
// operations on different queues never block each other.
//
// HAL_HSEM_FastTake(id) attempts a 1-step lock:
//   - Returns HAL_OK  if the semaphore was free and is now held by this core.
//   - Returns HAL_ERROR if another core already holds it.
// The HSEM peripheral is an AHB register block, not SRAM — there are no
// cache-coherency or exclusive-monitor issues.  It was designed specifically
// for reliable inter-core synchronization on the STM32H7 dual-core family.
//
// HAL_HSEM_Release(id, 0) unconditionally frees the semaphore.
// The second parameter is a process ID (unused here; pass 0).
//
// Ref: STM32H747 RM0399 §11 "Hardware semaphore (HSEM)"
// Ref: stm32h7xx_hal_hsem.h (HAL_HSEM_FastTake / HAL_HSEM_Release)
bool DualCoreEventQueue::push(EventType eventType, const char* eventMsg) {
  // Acquire hardware semaphore
  while (HAL_HSEM_FastTake(hsemId) != HAL_OK);

  if (count >= EVENT_QUEUE_SIZE) {
    HAL_HSEM_Release(hsemId, 0);
    return false;  // Queue full; caller should log or retry
  }

  events[head].type = eventType;
  events[head].read = false;

  if (eventMsg != nullptr) {
    strncpy(events[head].text, eventMsg, EVENT_MESSAGE_SIZE - 1);
    events[head].text[EVENT_MESSAGE_SIZE - 1] = '\0';
  } else {
    events[head].text[0] = '\0';
  }

  head = (head + 1) % EVENT_QUEUE_SIZE;
  count++;

  // Release hardware semaphore
  HAL_HSEM_Release(hsemId, 0);
  return true;
}

// pop() — called by the *receiving* core to dequeue an event.
// Same HSEM locking protocol as push(); see comments there.
bool DualCoreEventQueue::pop(EventType& eventType, char* eventBuffer) {
  // Acquire hardware semaphore
  while (HAL_HSEM_FastTake(hsemId) != HAL_OK);

  if (count == 0) {
    HAL_HSEM_Release(hsemId, 0);
    return false;
  }

  eventType = events[tail].type;

  if (eventBuffer != nullptr) {
    strncpy(eventBuffer, events[tail].text, EVENT_MESSAGE_SIZE);
  }

  events[tail].read = true;
  tail  = (tail + 1) % EVENT_QUEUE_SIZE;
  count--;

  // Release hardware semaphore
  HAL_HSEM_Release(hsemId, 0);
  return true;
}

bool DualCoreEventQueue::isEmpty() const {
  return count == 0;
}

uint8_t DualCoreEventQueue::getCount() const {
  return count;
}


// ============================================================================
// EventBroadcaster implementation
// ============================================================================

bool EventBroadcaster::sendToM4(EventType eventType, const char* eventData) {
  return m4EventQueue.push(eventType, eventData);
}

bool EventBroadcaster::sendToM7(EventType eventType, const char* eventData) {
  return m7EventQueue.push(eventType, eventData);
}

bool EventBroadcaster::broadcastEvent(EventType eventType, const char* eventData) {
  // Both pushes must succeed for true delivery; partial success is logged by
  // returning false so callers can react (e.g. retry or emit a safety alert).
  bool m4ok = m4EventQueue.push(eventType, eventData);
  bool m7ok = m7EventQueue.push(eventType, eventData);
  return m4ok && m7ok;
}

uint8_t EventBroadcaster::getM4QueueCount() {
  return m4EventQueue.getCount();
}

uint8_t EventBroadcaster::getM7QueueCount() {
  return m7EventQueue.getCount();
}
