


# constants
# time is in us.
# these values are guesses from memory
time_compute_new_move = 80
time_interrupt_with_no_motion = 20
time_per_step_per_axis = 10

# we will assume two steppers at max speed and one stepper at half speed for time computations
# this is a worst case - we don't expect the third (filament) stepper to be at half speed and 
# for most moves only one of XY steppers will be at full speed


def CPUTimeFixedInterrupts() :

  # for each speed
  # compute steps per interrupt period
  # compute steps for half speed stepper
  # compute time per interrupt
  # compute chart free time vs busy time

def CPUTimeVariableInterrupts() :

  # for each speed
  # compute interrupt period
  # compute steps per interrupt period
  # compute steps for half speed stepper
  # compute time per interrupt
  # compute chart free time vs busy time


def StepSequenceVsIdealSingle() :

  # for a given speed and distance
  # compute ideal step sequence
  # compute step sequence for fixed interrupt
  # compute step sequence for variable interrupt
  # chart step sequences
  # chart step speed differences


def StepSequenceVsIdealMulti() :
  """ 
  for a given speed and two distances
  compare the step output from stepper.cc with the ideal
  """

  # for a given speed and two given distances
  # compute ideal step sequence for both distances
  # compute step sequence for fixed interrupt
  # compute step sequence for shorter distance for fixed interrupt
  # compute shorter and longer distance step sequences for variable interrupt
  # chart step sequences, one chart per distance with three sequences
  # chart step speed differences, one chart per distance with three sequences
  # evaluate is distance two ever incorrectly ahead of distance one? 

def StepSequenceVsIdealAccelerated():
  """ 
  for a given start and end speed and two distances
  compare the step output from stepper.cc with the ideal
  """

  # compute the step sequence for fixed interrupt for long leg including acceleration
  # compute the step sequence for shorter distance leg
  # chart step sequences
  # chart step speed differences
  
