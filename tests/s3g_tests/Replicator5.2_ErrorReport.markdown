#Error Report

This doc contains errors found in one set of tests.  For know non-working commands, see replicator_s3g_handling.markdown (located in the docs folder)

##Errors found
All errors listed below were discovered when connecting to a Replicator using version 5.2 of the MB firmware.

### Get Next Filename (not verified)
I supply a set of testFiles in the s3g repo that the tester should burn onto their SD card.  It contains only one file, named box 1.s3g.  The tester supplies the volumeName of their sd card, and get_next_filename is used to query the replicator for filenames.  According to the spec, this function should return either the volume name or filenames of the sd card.  Also, passing in a non-zero value will reset the pointer to the beginning of the directory.  I pass in True for the first call, which is supposed to give me the first item in the directory.  Intuitively, this probably should be the volume name, since its spec'd that it will return the volume name.  But, instead I get the first file on the SD card.

###get_tool_status (this is likely fixed in 5.5 but haven't yet tested)
I first test to make sure the return values are correct for a machine in the ready state (extruder(s) ready, no errors).  I then Set toolhead 0's temperature to 100 and compare get_tool_status' 0th bit (which is set if the Extruder has reached target temperature) to IsToolheadReady (which returns true if the toolhead has reached temperature, false otherwise).  IsToolheadReady returned False (toolhead not ready), and get_tool_status' 0th bit was set (toolhead is ready).

###wait_for_button (likely, unverfied)
First, the user gives feedback regarding whether or not  waitForButton works with no timeout.  Then timeouts are tested.  A timeout of 5 seconds is specified, and the user watches the replicator to make sure it stops waiting for a button push after 5 seconds.  _It does not_.   The replicator then waits for a button push with a 1 second timeout, with the resetAfterTimeout flag set as true.  Instead of resetting once, it goes into a constant loop of resetting.
_tldr_
Throwing the timeout reset flag, and letting the timeout elapse causes the mightyboard to be in a constant loop of resetting.
Throwing the Timeout ready state flag and letting the timeout elapse causes the center button to constantly flash
