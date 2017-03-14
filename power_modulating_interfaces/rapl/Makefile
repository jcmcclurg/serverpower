CFLAGS=-g

all: power_gadget

#rapl_lib_shared: msr.o cpuid.o rapl.o
#	gcc $(CFLAGS) -fpic -c msr.c cpuid.c rapl.c -lm
#	gcc $(CFLAGS) -shared -o librapl.so msr.o cpuid.o rapl.o

#power_gadget_dynamic: 
#	gcc $(CFLAGS) power_gadget.c -I. -L. -lrapl -o power_gadget -lm
#	gcc $(CFLAGS) pgtc.c -I. -L. -lrapl -o power_gadget -lm

librapl.a: msr.o cpuid.o rapl.o
	ar rcs librapl.a msr.o cpuid.o rapl.o

msr.o: msr.c
	gcc $(CFLAGS) -c msr.c -lm

cpuid.o: cpuid.c
	gcc $(CFLAGS) -c cpuid.c -lm

rapl.o: rapl.c
	gcc $(CFLAGS) -c rapl.c -lm

power_gadget: librapl.a pgtc.c
#	gcc $(CFLAGS) power_gadget.c -I. -L. -o power_gadget ./librapl.a -lm
	gcc $(CFLAGS) pgtc.c -I. -L. -o power_gadget ./librapl.a -lm

setMSR: librapl.a setMSR.c
	gcc $(CFLAGS) setMSR.c -I. -L. -o setMSR ./librapl.a -lm

clean: 
	rm -f power_gadget librapl.so librapl.a msr.o cpuid.o rapl.o 
