This firmware **REQUIRES** avr-gcc 4.6.2 or later.  It will **NOT** function correctly when built with avr-gcc 4.5.x.  While it is possible to compile it with 4.5.x, it will not function correctly.

For directions on obtaining and building avr-gcc, please see [http://www.nongnu.org/avr-libc/user-manual/install_tools.html]().  The following shell script illustrates how to build the tool chain.

    #!/bin/sh

    # Obtaining and building avr-gcc 4.6.3
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

    curl -O http://mirrors-us.seosue.com/gcc/releases/gcc-4.6.3/gcc-core-4.6.3.tar.bz2
    curl -O http://mirrors-us.seosue.com/gcc/releases/gcc-4.6.3/gcc-g++-4.6.3.tar.bz2
    bunzip2 -c gcc-core-4.6.3.tar.bz2 | tar xf -
    bunzip2 -c gcc-g++-4.6.3.tar.bz2 | tar xf -
    cd gcc-4.6.3
    mkdir obj-avr
    cd obj-avr
    ../configure --prefix=$PREFIX --target=avr --enable-languages=c,c++ \
        --disable-nls --disable-libssp --with-dwarf2
    make
    make install

    cd $DIR

    curl -O http://savannah.spinellicreations.com/avr-libc/avr-libc-1.8.0.tar.bz2
    bunzip2 -c avr-libc-1.8.0.tar.bz2 | tar xf -
    cd avr-libc-1.8.0
    ./configure --prefix=$PREFIX --build=`./config.guess` --host=avr
    make
    make install

    cd $DIR
