#Acceleration Testing 

## Objective
All testing up to now on acceleration has either been  
1. print based  
2. logic analyzer based   
We have been able to verify some aspects of our solution this way but there are important pieces we have not verified.  
We have JTAG now!  
We need to do basic verification of our algorithm against edge cases and build a chain of trust so that we can address known issues.  

## Known Issues
* retraction issues
    * skipping on push back (causes gaps in filament)
    * motion must come to a full stop each time there is a retract as XY motion are zero (due to algorithm design)
* intermittent blobbing on slow to near stop - filament does not slow exactly will motor motion... pressure? 
* grinding noise on fast circles, causes jerky filament behavior
* bug : very occasional super slow segment
* bug : axes skipping - this has been reported and may be bot related, pullys or power supply (one bot known to work in one location, but not in another) But lets make sure we don't have any speed jumps beyond allowable acceleration rate
* lets review with support and the makebot operaters group to see if any other known issues have come up that I am forgetting

## Some additional Questions
* what are differences between pololu drivers and botsteps?
* how would motors / motion behave differently with non fixed stepper interrupt?
* how does filament behave with pressure?
* how do filament diameter settings change with acceleration (1 user reporting needing to change settings with update.... verify if this user is sane)
* can we make the planner faster (yes) and would this be a valuable use of time?

## Testing 
### Test Prints
* Curves at different sizes/resolutions and different speeds
* single shell box x 6 for retraction test
* high detail edges inset (inner shell)
* miracle grue vs skeinforge vs slicer prints

### Tests
1. Test that all junction speed changes proscribed by planner are > minspeedchange and < maxspeedchange  
     1. test with jtag - write flag test for speed change, run test prints and stop on flag raised
     2. write speed changes to a packet and query over s3g?
     3. use PC based simulator (write or use Jetty's)
2. Test that acceleration rates are always > minrate and < maxrate
     * same possible tests here as in 1.
3. Test that speed goal and actual speed acheived by steppers are the same, or if different, below a threshold (speed junction changes are still within allowable range)
     * same possible tests here as in 1.
4. Run retraction test print at different settings (includes skeinforge settings + filament acceleration parameters)
     * this is a printing test - ie it will take many man hours
5. Grinding on fast circles....?  
    1. review motor characteristics
         * looking at the revision notes on the motor datasheet, we may be over driving the motors so they cannot reach the full microstepping current levels.... verify whether this is true with moons, try running the steppers at lower vrefs
         * find expected step change time for the motor using L/R - are we stepping faster than the motor can step?  this would cause slipping.
    2. look at trapezoids generated for the circle
         * jtag - stop at each junction point around the circle
         * PC based simulator - dump trapezoids for circle
         * send trapezoid info back in s3g packet
    3. if trapezoids are OK, assess non fixed vs fixed interrupt behavior - smoothness of motion
         * PC based simulator to look at real difference in stepper pattern
         * print test with logic analyzer to view stepper pattern difference
         * sound / print quality comparison
    4. another thing to consider is the difference between stepper drivers
         * run the same print with differnt stepper drivers and asses sound / print quality differences
6. Asses fixed vs non fixed stepper interrupts
    1. how does the cpu available time compare? during fastest motion, slowest motion
         * mathematical assessment based on interrupt schedule scheme & speed range
    2. how does stepper motion smoothness compare with ideal?
         * mathematical assessment based on step schedule scheme 
    3. how does inter-stepper scheduling compare with ideal?
         * mathematical assessment based on step schedule scheme 
7. Filament speed up slow down test.....? Filament diameter tests?
    some interesting links:

    https://github.com/repetier/Repetier-Firmware/wiki/Hardware-settings-and-print-quality

    http://softsolder.com/2012/02/01/reversal-zits-extruder-pressure-vs-flow-vs-acceleration/
