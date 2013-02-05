mkdir -p .build/uno
cp firmware.hex .build/uno/firmware.hex
./clean.sh
ino upload