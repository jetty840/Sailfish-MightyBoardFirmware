#! /bin/sh

sudo apt-get install patch gcc wget scons libgmp-dev libmpfr-dev libgmp-dev libmpc-dev avrdude

set -x
set -e

USRNAME=`whoami`
PREFIX=/usr/local/
export PREFIX

PATH=${PREFIX}bin:${PATH}

BINUTILS_VERSION=2.22
MPC_VERSION=0.9
MPFR_VERSION=3.1.2
GCC_VERSION=4.6.3
AVRDUDE_VERSION=5.10
#AVR_LIBC_VERSION=1.7.1
#AVR_LIBC_OLD=old-releases/
AVR_LIBC_VERSION=1.8.1
AVR_LIBC_OLD=


MPC_PREFIX=/home/${USRNAME}/opt/mpc-${MPC_VERSION}

#echo Next step: binutils - press ENTER
#read VAR

#if test ! -d binutils-${BINUTILS_VERSION}
#then
	wget -N -P incoming http://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.bz2
	bzip2 -dc incoming/binutils-${BINUTILS_VERSION}.tar.bz2 | tar xf -
#fi
if test ! -d binutils-${BINUTILS_VERSION}/obj-avr
then
	mkdir binutils-${BINUTILS_VERSION}/obj-avr
fi
	cd binutils-${BINUTILS_VERSION}/obj-avr
	../configure --prefix=${PREFIX} --target=avr --disable-nls --enable-install-libbfd
	make
	sudo make install
	cd ../..


#echo Next step: gcc - press ENTER
#read VAR

wget -N -P incoming http://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-core-${GCC_VERSION}.tar.bz2
wget -N -P incoming http://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-g++-${GCC_VERSION}.tar.bz2
	bzip2 -dc incoming/gcc-core-${GCC_VERSION}.tar.bz2 | tar xf -
	bzip2 -dc incoming/gcc-g++-${GCC_VERSION}.tar.bz2 | tar xf -
if test ! -d gcc-${GCC_VERSION}/obj-avr
then
	mkdir gcc-${GCC_VERSION}/obj-avr
fi
	cd gcc-${GCC_VERSION}/obj-avr
	../configure --prefix=$PREFIX --target=avr --enable-languages=c,c++ --enable-lto --disable-nls --disable-libssp --with-dwarf2 # --with-mpc=${MPC_PREFIX}
	make
	sudo make install
	cd ../..


#echo Next step: libc - press ENTER
#read VAR

wget -N -P incoming http://download.savannah.gnu.org/releases/avr-libc/${AVR_LIBC_OLD}avr-libc-${AVR_LIBC_VERSION}.tar.bz2
#if test ! -d avr-libc-${AVR_LIBC_VERSION}
#then
	bzip2 -dc incoming/avr-libc-${AVR_LIBC_VERSION}.tar.bz2 | tar xf -
	if test -f incoming/avr-libc-${AVR_LIBC_VERSION}-gcc-${GCC_VERSION}.patch
	then
		cd avr-libc-${AVR_LIBC_VERSION}
		patch -p1 < ../incoming/avr-libc-${AVR_LIBC_VERSION}-gcc-${GCC_VERSION}.patch
		# autoreconf
		cd ..
	fi
	cd avr-libc-${AVR_LIBC_VERSION}
	./configure --prefix=${PREFIX} --build=$(./config.guess) --host=avr
	make
	sudo make install
	cd ..
#fi
