#!/bin/bash
directory_match=""
find -type d \( -name 'build' -o -name '*_build' \) -exec echo '{}' ';'
echo OK?
read confirm
if [ $confirm == 'y' ] ; then
	find -type d \( -name 'build' -o -name '*_build' \) -exec rm -rf '{}' ';'
fi
find -type f -name '*.o' -exec echo '{}' ';'
echo OK?
read confirm
if [ $confirm == 'y' ] ; then
	find -type f -name '*.o' -exec rm -rf '{}' ';'
fi


