
#LIBS := /usr/lib/libcups.so.2 /root/Desktop/build_libusb-1.0/lib/libusb-1.0.a
#LIBS := /usr/lib64/libcups.so.2 /root/Desktop/build_libusb-1.0/lib/libusb-1.0.a

LIBS64 := /usr/lib/x86_64-linux-gnu/libcups.so.2 /opt/libusb-1.0/lib/libusb-1.0.a
LIBS32 := /usr/lib/i386-linux-gnu/libcups.so.2 /opt/libusb-1.0/lib/libusb-1.0.a
INCLUDE_DIR = -I /opt/libusb-1.0/include
OBJS = usb.o
CFLAGS = -O3 -Wall -c -fmessage-length=0 -fno-stack-protector -fPIC 

# All Target
#all:libusb64.so

%o:%c
	gcc $(CFLAGS) -o $@ $< $(INCLUDE_DIR)

# Tool invocations
libusb64.so:$(OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Linker'
	gcc -m64 -Wl,--wrap=memcpy -Xlinker --hash-style=sysv -Xlinker -fno-stack-protector -shared -o "$@" $(OBJS) $(LIBS64) -lrt
	cp $@ /home/jacky/workspace/ricoh/vop/release/x86_64/static_app/libvopusb.so
	@echo 'Finished building target: $@'
	@echo ' '

libusb32.so: $(OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Linker'
	gcc -m32 -Wl,--wrap=memcpy -Xlinker --hash-style=sysv -Xlinker -fno-stack-protector -shared -o "$@" $(OBJS) $(LIBS32) -lrt
	cp $@ /home/jacky/workspace/ricoh/vop/release/i386/static_app/libvopusb.so
	@echo 'Finished building target: $@'
	@echo ' '
# Other Targets
clean:
	-$(RM) usb.o 
	-@echo ' '

.PHONY: all clean
.SECONDARY:

