# Real-Time Operating System for ARM Cortex-M4

A bare-metal real-time operating system implementation featuring a deadline-driven scheduler, advanced memory management, and sophisticated task control mechanisms.

## Core Architecture

### Advanced Interrupt Management
- Custom interrupt handlers for SVC, PendSV, and SysTick
- Sophisticated interrupt priority management:
```c
SHPR3 |= 0xFFU << 24; // SysTick: Lowest priority
SHPR3 |= 0xFEU << 16; // PendSV: Second lowest
SHPR2 |= 0xFDU << 24; // SVC: Highest priority
```
- Hardware-optimized context switching through PendSV exception
- Atomic operations for critical sections with precise interrupt control

### Deadline-Driven Scheduler
- Earliest Deadline First (EDF) scheduling with O(log n) complexity
- Three-queue system for task management:
  - `scheduling_q`: Ready tasks sorted by deadline
  - `periodic_q`: Periodic tasks with automatic rescheduling
  - `sleeping_q`: Time-blocked tasks with wake-up scheduling
- Dynamic priority adjustment through `osSetDeadline`
- Preemptive scheduling with hardware-assisted context preservation

### Task Control Architecture
```c
typedef struct task_control_block {
    void (*ptask)(void* args);    // Task entry point
    unsigned int stack_high;       // Stack top address
    unsigned int tid;             // Task ID
    char state;                   // Current state
    unsigned int deadline;        // Current deadline
    unsigned int original_deadline;// Base deadline
    pq_elem q_elem;              // Priority queue element
    // Additional control fields
} TCB;
```
- Full hardware context preservation in stack frame:
```c
typedef struct t_task_stack_frame {
    uint32_t R4-R11;             // Preserved registers
    uint32_t R0-R3, R12;         // Scratch registers
    uint32_t LR, PC, xPSR;       // Program status
} task_stack_frame;
```

### Memory Management System
- Sophisticated memory tracking with metadata nodes:
```c
typedef struct mem_metadata {
    unsigned int start;           // Block start
    unsigned int end;            // Block end
    size_t size;                // Block size
    unsigned int tid;           // Owner task ID
    struct mem_metadata *next;   // Free list linkage
    struct mem_metadata *prev;
} mem_node;
```
- Zero-fragmentation design with immediate block coalescing
- Task-specific memory protection and ownership validation
- Efficient free list management with O(1) deallocation

## Advanced Features

### Task Synchronization
- Hardware-optimized context switching mechanism
- Atomic operations for critical sections
- Priority inheritance to prevent priority inversion
- Deadline-based preemption control

### System Services
- Supervisor calls (SVC) for privileged operations:
  - Task creation and termination
  - Memory allocation/deallocation
  - Priority and deadline management
  - Sleep and periodic yield operations

### Scheduling Policies
- Fine-grained deadline management
- Support for both periodic and aperiodic tasks
- Dynamic priority adjustments during runtime
- Time-slice based execution control

## Technical Implementation Details

### Interrupt Handling
- Custom naked interrupt handlers for minimal overhead
- Optimized register preservation
- Direct hardware stack management
- Efficient task state transitions

### Memory Architecture
- Word-aligned allocations for optimal access
- Segregated free lists for different block sizes
- Metadata optimization for minimal overhead
- Protected memory operations through privilege levels

### Performance Optimizations
- O(1) context switch time through direct hardware support
- O(log n) task scheduling with priority queue
- Constant-time memory deallocation
- Optimized register usage in critical paths

## Technical Specifications
- Target: ARM Cortex-M4 (STM32F401RE)
- Core Clock: 84 MHz
- Memory Model: Supervisor/User mode separation
- Interrupt Latency: Deterministic response times
- Context Switch Time: < 1μs typical
