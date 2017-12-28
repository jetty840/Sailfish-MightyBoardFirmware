All code in this directory with minor modifications is from
Roland Riegel's excellent SD/MMC card library.  You can find
the latest version of the code here:
http://www.roland-riegel.de/sd-reader/doc/

The Sailfish firmware initially started by forking the 2010-01-10 release.
Some changes (including the larger one below) were made over time.
In 2017, some cleanup and rebasing was performed by Ryan Pavlik to
merge newer upstream versions into a combined tree with Sailfish changes,
so the current code here corresponds to the 2012-06-12 release merged with
the Sailfish-originated changes.

Dan Newman <dan.newman@mtbaldy.us> added CRC support and error return
  handling, February 2013.

The code here is licensed undel GPLv2 and LGPLv2.1; we're using
the latter license, as it is compatible with the GPLv3 (which
the rest of our codebase is licensed under).
