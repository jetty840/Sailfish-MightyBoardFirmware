This firmware **REQUIRES** avr-gcc 4.6.2 or later.  It will **NOT** function correctly when built with avr-gcc 4.5.x.  While it is possible to compile it with 4.5.x, it will not function correctly.  You are strongly encourged to use avr-gcc 4.6.2 and avr-libc 1.7.2.  If you move to avr-libc 1.8 or later, you will need to code around deprecated features or add -D__DELAY_BACKWARD_COMPATIBLE__ and -D__PROG_TYPES_COMPAT__ to the compile commands as non backwards compatible changes were made to those facilities in avr-libc 1.8.

Note that with newer versions of gcc, avr-gcc 4.6.2's gtype-desc.c may not compile owing to some incorrect sizeof statements.  Inspection of the code quickly shows the problem.  You can look at the 4.6.3 version of that routine where the issue has been corrected.  Don't use the 4.6.3 version of gtype-desc.c: just fix those five or so mistakes in accord with the 4.6.3 version.

For directions on obtaining and building avr-gcc, please see [http://www.nongnu.org/avr-libc/user-manual/install_tools.html]. The following shell script illustrates how to build the tool chain.

    #!/bin/sh

    # Obtaining and building avr-gcc 4.6.2 and avrlibc 1.7.2
    #
    # For complete directions, see
    #
    #    

    PREFIX=$HOME/local/avr
    PATH=$PATH:$PREFIX/bin

    DIR=`pwd`

    curl -O http://ftp.gnu.org/gnu/binutils/binutils-2.22.tar.bz2
    bunzip2 -c binutils-2.22.tar.bz2 | tar xf -
    cd binutils-2.22
    mkdir obj-avr
    cd obj-avr
    ../configure --prefix=$PREFIX --target=avr --disable-nls
    make
    make install

    cd $DIR

    curl -O http://www.netgull.com/gcc/releases/gcc-4.6.3/gcc-core-4.6.3.tar.bz2
    curl -O http://www.netgull.com/gcc/releases/gcc-4.6.3/gcc-g++-4.6.3.tar.bz2
    bunzip2 -c gcc-core-4.6.2.tar.bz2 | tar xf -
    bunzip2 -c gcc-g++-4.6.2.tar.bz2 | tar xf -
    cd gcc-4.6.2
    mkdir obj-avr
    cd obj-avr
    ../configure --prefix=$PREFIX --target=avr --enable-languages=c,c++ \
        --disable-nls --disable-libssp --with-dwarf2
    make
    make install

    cd $DIR

    curl -O http://savannah.spinellicreations.com/avr-libc/avr-libc-1.7.2rc2252.tar.bz2
    bunzip2 -c avr-libc-1.7.2rc2252.tar.bz2 | tar xf -
    cd avr-libc-1.7.2rc2252
    ./configure --prefix=$PREFIX --build=`./config.guess` --host=avr
    make
    make install
 
    cd $DIR
