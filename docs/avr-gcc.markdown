This firmware **REQUIRES** avr-gcc 4.6.2 or 4.6.3. Higher Versions are untested except:
It will build with avr-gcc 4.9.4 (with avr-libc 1.8.1) and 6.3 (with avr-libc 2.0.0)
It will **NOT** function correctly when built with avr-gcc 4.5.x.  You are strongly encourged to use either avr-gcc 4.6.2 & avr-libc 1.7.2 or avr-gcc 4.6.3 and avr-libc-1.8.1.


For directions on obtaining and building avr-gcc, please see

Below was a invaluable inspiration. Nevertheless it is heavily outdated. (and the VM is VirtualBox and uses Vagrant)
* [https://github.com/dcnewman/Ubuntu-Sailfish-VM](https://github.com/dcnewman/Ubuntu-Sailfish-VM)
which is a repository containing the necessary source files to build a complete
avr-gcc 4.6.3 toolchain.  It also contains links to to pre-built Ubuntu virtual
machines with the toolchain built and installed to `/usr`.

Below is NOT VALID for the Ubuntu script:
*  The shell scripts in the `dist/build-avr-gcc/` subdirectory of this repository for
suggestions on how to build a toolchain.  The links in those scripts may no longer
point at available downloads.  The gcc maintainers eventually remove older tool chains
from their repository thus invalidating the links.

* [http://www.nongnu.org/avr-libc/user-manual/install_tools.html](http://www.nongnu.org/avr-libc/user-manual/install_tools.html)
for general directions on build gcc toolchains.
 
