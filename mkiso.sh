rm *.iso
rm iso/boot/kernel-dbg
cp NibrasOS iso/boot/
grub-mkrescue -o NibrasOS.iso iso
