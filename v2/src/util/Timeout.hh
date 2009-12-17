#ifndef MB_UTIL_TIMEOUT_HH_
#define MB_UTIL_TIMEOUT_HH_

#include <stdint.h>

class Timeout {
public:
	/// Timeout methods are called from within interrupts, so they must only manipulate
	/// volatiles (if you want the changes to be seen outside the interrupt).  The function
	/// should also return very quicky-- if it's more that a few lines, it's no good.
	virtual void timeout() =0;
};

class TimeoutManager {
public:
	static void addTimeout(Timeout& timeout, uint16_t ms);
	static void cancelTimeout(Timeout& timeout);
};


#endif // MB_UTIL_TIMEOUT_HH_
