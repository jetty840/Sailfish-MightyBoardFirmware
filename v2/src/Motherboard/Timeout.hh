#ifndef MB_UTIL_TIMEOUT_HH_
#define MB_UTIL_TIMEOUT_HH_

#include <stdint.h>

/// Timeout objects maintain timestamps and check the universal clock to figure out when they've
/// elapsed.  Resolution is at best that of the system interval.  Maximum timeout length is
/// 2147483648 microseconds.
/// Timeouts much be checked every 1073741824 microseconds to remain valid.
/// After a timeout has elapsed, it can not go back to a valid state without being explicitly reset.
class Timeout {
private:
	bool active_;
	bool elapsed_;
	int32_t end_stamp_micros_;
public:
	Timeout();
	void start(uint32_t duration_micros);
	bool hasElapsed();
	bool isActive() const { return active_; }
	void abort();
};

#endif // MB_UTIL_TIMEOUT_HH_
