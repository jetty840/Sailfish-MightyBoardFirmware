#define __STDC_LIMIT_MACROS
#include <gtest/gtest.h>
#include <stdlib.h>
#include <stdint.h>

using namespace std;

/// This doesn't test compiled code atm; instead it's an algorithmic
/// testbed to make sure our timer overflow works correctly in all instances.

typedef uint32_t micros_t;

micros_t current_time;
micros_t getCurrentMicros() { return current_time; }

class Timeout {
private:
  bool active;
  bool elapsed;
  micros_t start_stamp_micros;
  micros_t duration_micros;
public:
  Timeout();
  void start(micros_t duration_micros);
  bool hasElapsed();
  bool isActive() const { return active; }
  void abort();
};

Timeout::Timeout() : active(false), elapsed(false) {}

void Timeout::start(micros_t duration_micros_in) {
	active = true;
	elapsed = false;
	start_stamp_micros = getCurrentMicros();
	duration_micros = duration_micros_in;
}

bool Timeout::hasElapsed() {
  if (active && !elapsed) {
    micros_t delta = getCurrentMicros() - start_stamp_micros;
    if (delta >= duration_micros) {
      active = false;
      elapsed = true;
    }
  }
  return elapsed;
}

void Timeout::abort() {
	active = false;
}

void runTest(micros_t start_time, 
	     uint32_t duration, 
	     micros_t simulated_time,
	     bool expected) {
  current_time = start_time;
  Timeout t;
  t.start(duration);
  current_time = simulated_time;
  ASSERT_EQ(expected,t.hasElapsed()) <<
    " Start " << start_time << endl <<
    " Dur.  " << duration << endl <<
    " Time  " << simulated_time;
}

TEST(TimeoutTest, OrdinaryCase)
{
  srandom(time(NULL));
  for (int i  = 0; i < 100; i++) {
    uint32_t duration = random() % (UINT32_MAX/4);
    if (duration < 2) duration = 2;
    micros_t start = random() % (UINT32_MAX-duration);
    runTest(start,duration,start-1,true);
    runTest(start,duration,start,false);
    runTest(start,duration,start+1,false);
    runTest(start,duration,start+duration-1,false);
    runTest(start,duration,start+duration,true);
    runTest(start,duration,start+duration+1,true);
  }
}

TEST(TimeoutTest, IntervalWrapCase)
{
  srandom(time(NULL));
  for (int i  = 0; i < 100; i++) {
    uint32_t duration = random() % (UINT32_MAX/4);
    if (duration < 2) duration = 2;
    micros_t start = random() % duration + (UINT32_MAX-duration);
    runTest(start,duration,start-1,true);
    runTest(start,duration,start,false);
    runTest(start,duration,start+1,false);
    runTest(start,duration,start+duration-1,false);
    runTest(start,duration,start+duration,true);
    runTest(start,duration,start+duration+1,true);
  }
}


TEST(TimeoutTest, TimerWrapCase)
{
  srandom(time(NULL));
  for (int i  = 0; i < 100; i++) {
    uint32_t duration = random() % (UINT32_MAX/4);
    micros_t start = random() % UINT32_MAX;
    micros_t when = start - random()%(UINT32_MAX-duration);
    runTest(start,duration,when,true);
  }
}

TEST(TimeoutTest, BoundryCase1)
{
  // start time = 0;
  srandom(time(NULL));
  uint32_t duration = random() % (UINT32_MAX/4);
  if (duration < 2) duration = 2;
  micros_t start = 0;
  runTest(start,duration,start-1,true);
  runTest(start,duration,start,false);
  runTest(start,duration,start+1,false);
  runTest(start,duration,start+duration-1,false);
  runTest(start,duration,start+duration,true);
  runTest(start,duration,start+duration+1,true);
}

TEST(TimeoutTest, BoundryCase2)
{
  // start time = max-1;
  srandom(time(NULL));
  uint32_t duration = random() % (UINT32_MAX/4);
  if (duration < 2) duration = 2;
  micros_t start = UINT32_MAX-1;
  runTest(start,duration,start-1,true);
  runTest(start,duration,start,false);
  runTest(start,duration,start+1,false);
  runTest(start,duration,start+duration-1,false);
  runTest(start,duration,start+duration,true);
  runTest(start,duration,start+duration+1,true);
}

TEST(TimeoutTest, BoundryCase3)
{
  // end time = max-1;
  srandom(time(NULL));
  micros_t start = random() % UINT32_MAX;
  micros_t duration = (UINT32_MAX-1) - start;
  runTest(start,duration,start-1,true);
  runTest(start,duration,start,false);
  runTest(start,duration,start+1,false);
  runTest(start,duration,start+duration-1,false);
  runTest(start,duration,start+duration,true);
  runTest(start,duration,start+duration+1,true);
}

TEST(TimeoutTest, BoundryCase4)
{
  // end time = 0;
  srandom(time(NULL));
  micros_t start = random() % UINT32_MAX;
  micros_t duration = 0 - start;
  runTest(start,duration,start-1,true);
  runTest(start,duration,start,false);
  runTest(start,duration,start+1,false);
  runTest(start,duration,start+duration-1,false);
  runTest(start,duration,start+duration,true);
  runTest(start,duration,start+duration+1,true);
}

micros_t call(micros_t in) {
  return in;
}
