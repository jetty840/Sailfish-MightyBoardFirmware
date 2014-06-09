#! /bin/sh

echo Needed packages for cygwin:
echo - pv scons gcc-g++ make patch subversion wget bison 
echo - libgmp-devel libmpfr-devel libmpc-devel libusb-devel libelf-devel  
echo - doxygen
echo - libftdi #not in packages of cygwin
echo Editor: nano / vim
echo ----- build gcc does not work -----
echo Press ENTER to continue
#read VAR

# Define how many cores your host have
HOST_CORES=2

set -x
set -e

USRNAME=`whoami`
#PREFIX=/home/${USRNAME}/opt/avr/
PREFIX=/usr/local/
export PREFIX

PATH=${PREFIX}bin:${PATH}

BINUTILS_VERSION=2.22
#MPC_VERSION=0.9
#MPFR_VERSION=3.1.2
#GCC_VERSION=4.6.2  # not works with cygwin
#GCC_VERSION=4.7.2  # minimum version for cygwin
GCC_VERSION=4.9.0   # not works for binary
AVR_LIBC_VERSION=1.7.2rc2252
#AVR_LIBC_VERSION=1.8.0
#AVRDUDE_VERSION=5.10
AVRDUDE_VERSION=6.1

#MPC_PREFIX=/home/${USRNAME}/opt/mpc-${MPC_VERSION}
MPC_PREFIX=/

#echo Next step: binutils - press ENTER
#read VAR

wget -N -P incoming http://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.bz2
if test ! -d binutils-${BINUTILS_VERSION}
then
#	bzip2 -dc incoming/binutils-${BINUTILS_VERSION}.tar.bz2 | tar xf -
	pv incoming/binutils-${BINUTILS_VERSION}.tar.bz2 | bzip2 -dc | tar xf -
fi
if test ! -d binutils-${BINUTILS_VERSION}/obj-avr
then
	mkdir binutils-${BINUTILS_VERSION}/obj-avr
fi
	cd binutils-${BINUTILS_VERSION}/obj-avr
	../configure --prefix=${PREFIX} --target=avr --disable-nls --enable-install-libbfd
	make -j${HOST_CORES}
	make install
	cd ../..


#echo Next step: mpfr - press ENTER
#read VAR

#if test ! -d  mpfr-${MPFR_VERSION}
#then
#	wget -N -P incoming http://www.mpfr.org/mpfr-current/mpfr-${MPFR_VERSION}.tar.bz2
#	bzip2 -dc incoming/mpfr-${MPFR_VERSION}.tar.bz2 | tar xf -
#fi

#if test ! -d mpfr-${MPFR_VERSION}
#	cd mpfr-${MPFR_VERSION}
#	./configure --prefix=${PREFIX}
#	make -j${HOST_CORES}
#	make install
#	cd ..
#exit


#echo Next step: mpc - press ENTER
#read VAR

#if test ! -d mpc-${MPC_VERSION}
#then
#	wget -N -P incoming http://www.multiprecision.org/mpc/download/mpc-${MPC_VERSION}.tar.gz
# 	gzip -dc incoming/mpc-${MPC_VERSION}.tar.gz | tar xf -
# 	cd mpc-${MPC_VERSION}
#	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/me/local/lib
#	#apt-get install  libgmp4-dev # may be needed for gmp.h header
# 	./configure --prefix=${MPC_PREFIX} 
# 	make -j${HOST_CORES}
# 	make install
# 	cd ..
#fi

# export LD_LIBRARY_PATH=${MPC_PREFIX}/lib/

#echo Next step: gcc - press ENTER
#read VAR

# every gcc version
wget -N -P incoming http://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.bz2
if test ! -d gcc-${GCC_VERSION}
then
	pv incoming/gcc-${GCC_VERSION}.tar.bz2 | bzip2 -dc | tar xf -
#	bzip2 -dc incoming/gcc-${GCC_VERSION}.tar.bz2 | tar xfv -
fi
# works only for avr-gcc version <= 4.6.4
#wget -N -P incoming http://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-core-${GCC_VERSION}.tar.bz2
#wget -N -P incoming http://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-g++-${GCC_VERSION}.tar.bz2
#	bzip2 -dc incoming/gcc-core-${GCC_VERSION}.tar.bz2 | tar xf -
#	bzip2 -dc incoming/gcc-g++-${GCC_VERSION}.tar.bz2 | tar xf -
if test ! -d gcc-${GCC_VERSION}/obj-avr
then
	mkdir gcc-${GCC_VERSION}/obj-avr
fi	
	cd gcc-${GCC_VERSION}/obj-avr
	# < 4.8 but does not work
	#../configure --prefix=$PREFIX --target=avr --enable-languages=c,c++ --enable-lto --disable-nls --disable-libssp --with-dwarf2 MISSING=texinfo 
	# >= 4.8
	../configure --prefix=$PREFIX --target=avr --enable-languages=c,c++ --enable-lto --disable-nls --disable-libssp --with-dwarf2
	make -j${HOST_CORES}
	make install
	cd ../..


#echo Next step: libc - press ENTER
#read VAR

wget -N -P incoming http://download.savannah.gnu.org/releases/avr-libc/avr-libc-${AVR_LIBC_VERSION}.tar.bz2
if test ! -d avr-libc-${AVR_LIBC_VERSION}
then
#	bzip2 -dc incoming/avr-libc-${AVR_LIBC_VERSION}.tar.bz2 | tar xf -
	pv incoming/avr-libc-${AVR_LIBC_VERSION}.tar.bz2 | bzip2 -dc | tar xf -
fi
	if test -f incoming/avr-libc-${AVR_LIBC_VERSION}-gcc-${GCC_VERSION}.patch
	then
		cd avr-libc-${AVR_LIBC_VERSION}
		patch -p1 < ../incoming/avr-libc-${AVR_LIBC_VERSION}-gcc-${GCC_VERSION}.patch
		cd ..
	fi
	cd avr-libc-${AVR_LIBC_VERSION}
	# dwonload newest config.guess the file is to old for cygwin
	mv -f config.guess  config.guessOLD
	wget -q -O config.guess 'http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD'
	chmod +x config.guess
	
	./configure --prefix=${PREFIX} --build=`./config.guess` --host=avr
	make -j${HOST_CORES}
	make install
	cd ..
#fi


#echo Next step: avrdude - press ENTER
#read VAR

#wget -N -P incoming http://download.savannah.gnu.org/releases/avrdude/avrdude-${AVRDUDE_VERSION}.tar.gz
# if test ! -d avrdude-${AVRDUDE_VERSION}
# then
# 	#gzip -dc incoming/avrdude-${AVRDUDE_VERSION}.tar.gz | tar xf -
#	pv incoming/avrdude-${AVRDUDE_VERSION}.tar.gz | gzip -dc | tar xf -
# fi
# 	cd avrdude-${AVRDUDE_VERSION}
# 	./configure --prefix=/home/mws/opt/avrdude-${AVRDUDE_VERSION}
# 	make -j${HOST_CORES}
# 	make install
# 	cd ..
