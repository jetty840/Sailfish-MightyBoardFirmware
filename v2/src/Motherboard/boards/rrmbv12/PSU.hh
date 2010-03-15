#ifndef PSU_HH_
#define PSU_HH_

namespace psu {

/**
 * Initialize PSU system.
 */
void init();

/**
 * Turn on PSU if argument is true.  Shut down PSU if argument
 * is false.
 */
void turnOn(bool on);

}

#endif // PSU_HH_
