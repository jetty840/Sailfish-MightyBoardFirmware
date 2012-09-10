##File Name Changes##
MBI files Planner.xx, Steppers.xx (movements called from Command.xx, Host.xx, Menu.xx)
Jetty files StepperAccelPlaner.xx, StepperAccel.xx, StepperAxis.xx, Steppers.xx, StepperAccelSpeedTable* (movments called from same)

StepperAccelPlannner.xx->Planner.xx
StepperAccel.xx->Steppers.xx
Steppers.xx top level namespace : calls StepperAccel functions, performs initialization functions
StepperAxis.xx define ports, initialize stepper axes, define read write functions to steppers
StepperAccelSpeedTable (a lookup table for stepper rates)

Advance
two constants are stored in eeprom: K, K2

StepperAxis.hh
  update e_steps in dda_step if JKN_ADVANCE is defined
Configuration.hh
  defines JKN_ADVANCE
StepperAccelPlanner.hh
  if JKN_ADVANCE is defined, the block struct contains extra fields: use_advance_lead, advance_lead_entry, advance_lead_exit, advance_pressure_relax, advance_lead_prime, advance_lead_deprime

All behavior only occurs if JKN_ADVNACE is defined:
StepperAccel.cc
  define advance_state, advance_pressure_relax_accumulator, lastAdvanceDeprime, ADVANCE_INTERRUPT_FREQUENCY = 10000, st_extruder_interrupt_rate, extruder_interrupt_steps_per_call, st_extruder_interrupt_rate_counter
  if current_block->use_accel and deprime_enabled, for each extruder, if extruder is stepping in this block and it has been deprimed, if extruden_when_negative, subtract extruder_deprim_stpes from e_steps, if ADVANCE_LEAD_DE_PRIME, also subtract block_avance_lead_prime from e_steps, else add these values to e_steps , set deprimed to false (extrude_when_negative indicates the direction we tell the steppers to move in order to push plastic)
  if current_block has no extruder steps, apply deprime, which the same logic as for applying prime, with the direction reversed (ie subtract steps to deprime, add steps to prime
  if use_accel, for each extruder, if avance_state==ACCEL, shift_phase16(advance_lead_entry), if advace_state==DECEL, shift_phase16(-advance_lead_exit), shift_phase32(-advance_pressure_relax_accumulator >> 8)
  call stepperAcisSetHardwareEnabledToMatch only when e_steps[x] are 0
  if decel leg is starting, update advance_state and advance_pressure_relax_accumulator
  if all steps completed, update lastAdvanceDeprime[]
  extruder interrupt is defined (where extruder steps are taken)
  compute the number of steps we take per extruder interrupt (there is a separate interrupt for the extruders with a fixed frequency of 10kHz)
  in block setup, set advance_state to ACCEL
StepperPlannerAccel.cc
  define extruder_advance_k, extruder_advance_k2 and initialize them
  final_speed_step_rate function defined : this is a fixed point function to compute the final speed of the block.  not sure why it is only defined with JKN_ADVANCE, 
  in calculate_trapezoid:
    clear the advance variables
    if advance_lead true for the block, compute final step rate
    if JKN_ADVANCE_LEAD_ACCEL : advance_lead_entry = extruder_advance_k * accelration rate, else advance_lead_entry = extruder_advance_k * (max_rate - initial_rate)
    advance_lead_prime = extruder_advance_k * initial_rate - KCONSTANT_0_5 (used when there is no previous e move, ie extruder starts from 0)
    advance_lead_entry and advance_lead_prime are capped to be greater or equal 0 (neagtive indicates overflow)
    advance_lead_exit and advance_lead_deprime are computed in exactly the same way.
    advance_pressure_relax = extruder_advance_k2 * acceleration_rate * KCONSTANT_100 / decelerate_steps
    advance_pressure_relax, advance_lead_exit and adance lead_deprime are capped to be greater or equal to 0 (negative indicates overflow)
    store these values in the current block_t
  in planner_add, block->use_advance_lead is true when there is a non-zero stepper move and the advance constants are not set to zero
Motherboard.cc
  calls doExtruderInterrupt() from within timer2 interrupt, which also manages the micros timer and the blinking leds

advance_pressure_relax_accumulator
  accumulates current_block->advance_pressure_relax each step of the deceleration phase, and this is applied as a shift_phase32 in the advance section of the stepper interrupt

lastAdvanceDeprime
  set by calculations of previous block, applied in the negative direction when the buffer is empty, or the extruder steps are zero

shift_phase functions
adding steps to the counter for the axis, slowing down or speeding up when the stepps occur for this axis


Deprime
Can only be enabled if ADVANCE is enabled
deprime is forced on if advance is enabled (it is forced on in steppers::reset())
deprime adds steps to esteps, which are exectuted whenever the extruder interrupt executes - these are extra steps in addition to the steps specified in the G1 command
deprime steps look to be on the order of 10s, up to 1xx  - so this is roughly equivalent to a retract of 1mm

Jerk
jerk applied to all axes, scaling factor applied to current speed as well as vmax_junction.  this means that the jerk for the next segment will be computed using the vmax_junction speed instead of the nominal speed.....

Homing System


Buffer System for planner blocks


Command States and calling move functions


Acceleration On vs Acceleration Off


New move Commands describe


What is a good plan for getting the TOM to work with the new tool chain?
a) implement the new command
b) modify the firmware to accept old style commands (cost?)

movement calls within Host and Menu.cc

fixed point vs float

stepper timer
timer 3.  

dda_step vs regular?  
there is no regular.  dda is the name of the struct containing the relevant stepper info like counter, delta etc

SRAM_tests

OVERSAMPLED_DDA
