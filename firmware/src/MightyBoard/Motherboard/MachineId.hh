#ifndef MACHINEID_HH_
#define MACHINEID_HH_

#ifdef THE_REPLICATOR_STR
#undef THE_REPLICATOR_STR
#endif

#ifdef MACHINE_ID
#undef MACHINE_ID
#endif

#ifdef FF_CREATOR

#define THE_REPLICATOR_STR "The FF Creator"
#define MACHINE_ID 0xD314 // Replicator 1

#elif FF_CREATOR_X

#define THE_REPLICATOR_STR "The FF Creator X"
#define MACHINE_ID 0xD314 // Replicator 1

#elif WANHAO_DUP4

#define THE_REPLICATOR_STR "Wanhao Duplicatr"
#define MACHINE_ID 0xD314 // Replicator 1

#elif MODEL_REPLICATOR

#define THE_REPLICATOR_STR "The Replicator"
#define MACHINE_ID 0xD314 // Replicator 1

#elif MODEL_REPLICATOR2
#if defined(SINGLE_EXTRUDER)

#define THE_REPLICATOR_STR "Replicator 2"
#define MACHINE_ID 0xB015 // Replicator 2

#else

#define THE_REPLICATOR_STR "Replicator 2X"
#define MACHINE_ID 0xB017 // Replicator 2X

#endif
#else

#define THE_REPLICATOR_STR "Makerbot"
#define MACHINE_ID 0xD314 // Replicator 1

#endif

#ifdef TOOLHEAD_OFFSET_X
#undef TOOLHEAD_OFFSET_X
#endif

#if defined(FF_CREATOR) || defined(FF_CREATOR_X)
#define TOOLHEAD_OFFSET_X 34.0
#elif defined(MODEL_REPLICATOR2)
#define TOOLHEAD_OFFSET_X 35.0
#else
#define TOOLHEAD_OFFSET_X 33.0
#endif

#endif
