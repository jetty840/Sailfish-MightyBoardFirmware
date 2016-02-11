#ifndef MACHINEID_HH_
#define MACHINEID_HH_

#if !defined(PLATFORM_THE_REPLICATOR_STR)
#define THE_REPLICATOR_STR "MakerBot"
#else
#define THE_REPLICATOR_STR PLATFORM_THE_REPLICATOR_STR
#endif

#if !defined(PLATFORM_MACHINE_ID)
#define MACHINE_ID 0xD314 // Replicator 1
#else
#define MACHINE_ID PLATFORM_MACHINE_ID
#endif

#if !defined(PLATFORM_HBP_PRESENT)
#define DEFAULT_HBP_PRESENT 1
#else
#define DEFAULT_HBP_PRESENT PLATFORM_HBP_PRESENT
#endif

#if !defined(PLATFORM_TOOLHEAD_OFFSET_X)
#define TOOLHEAD_OFFSET_X 3107
#else
#define TOOLHEAD_OFFSET_X PLATFORM_TOOLHEAD_OFFSET_X
#endif

#if !defined(PLATFORM_TOOLHEAD_OFFSET_Y)
#define TOOLHEAD_OFFSET_Y 0
#else
#define TOOLHEAD_OFFSET_Y PLATFORM_TOOLHEAD_OFFSET_Y
#endif

// --- Axis configuration ---
// Define the number of stepper axes supported by the board.  The axes are
// denoted by X, Y, Z, A and B.

#if !defined(PLATFORM_EXTRUDERS)
#define EXTRUDERS 2
#else
#define EXTRUDERS PLATFORM_EXTRUDERS
#endif

#if EXTRUDERS == 1
#if !defined(SINGLE_EXTRUDER)
#define SINGLE_EXTRUDER
#endif
#define EXTRUDERS_(a,b) a
#elif EXTRUDERS == 2
#define EXTRUDERS_(a,b) a, b
#else
#error Only 1 or 2 extruders supported; correct EXTRUDERS / PLATFORM_EXTRUDERS
#endif

#define STEPPER_COUNT (3 + EXTRUDERS)

#if STEPPER_COUNT == 4
#define STEPPERS_(a,b,c,d,e) a, b, c, d
#elif STEPPER_COUNT == 5
#define STEPPERS_(a,b,c,d,e) a, b, c, d, e
#else
// Should never happen owing to check of EXTRUDERS above
#error Only 4 or 5 steppers supported; correct STEPPER_COUNT
#endif

#endif
