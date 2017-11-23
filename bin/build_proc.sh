name=$2
src=$1
i386-elf-gcc -w -std=gnu99 -ggdb3 -O0 -fno-use-cxa-atexit -fno-rtti -fno-exceptions -fno-leading-underscore -I../include -c $src -o $(echo $name).o
i386-elf-ld $(echo $name).o -o $name
mv $name /Volumes/Untitled/bin
rm *.o
