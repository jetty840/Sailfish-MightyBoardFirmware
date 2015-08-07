
Cygwin does not work now for avr-gcc-4.6.2.
>  ../../gcc/doc/cppopts.texi:772: @itemx must follow @item
>  Makefile:4270: recipe for target 'doc/cpp.info' failed
The other thing is, that scons does'nt copy the header files to the build path

Debian build is tested with Debian Version 7.5 (wheezy).
You need to be a sudo user.
> su
> visudo
# then append the file with
username	ALL=(ALL) ALL
# username is your login name
> exit
> chmod +x *.sh
> ./build-avr-gcc-debian.sh

Ubuntu build is tested with Ubuntu 12.04.

The build-avr-gcc.sh can used to port it on other distributions.
