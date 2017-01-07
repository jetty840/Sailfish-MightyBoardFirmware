Cygwin does not work now for avr-gcc-4.6.2.

	   %  ../../gcc/doc/cppopts.texi:772: @itemx must follow @item
	   %  Makefile:4270: recipe for target 'doc/cpp.info' failed

The other thing is, that scons doesn't copy the header files to the build path.

Debian build is tested with Debian Version 7.5 (wheezy). You need to be a
sudo user.

	 % sudo visudo -f /etc/sudoers.d/username
	 username	ALL=(ALL) ALL

In the above `username` is your login name.

Ubuntu build is tested with Ubuntu 16.04
Ubuntu build is modified and updated for BINUTIL 2.27 and GCC 6.3.0, both is tested to build correctly

The build-avr-gcc.sh can used to port it on other distributions.
