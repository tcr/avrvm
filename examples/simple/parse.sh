xxd -i < ../test.evm > source.h; echo , 0x00 >> source.h
gcc parse.c -o parse && ./parse