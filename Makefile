# This file was automagically generated by mbed.org. For more information, 
# see http://mbed.org/handbook/Exporting-to-CodeSourcery

GCC_BIN = 
MBEDDRIVE=H:
PROJECT = loadmon
OBJECTS = ./MODSERIAL/ChangeLog.o ./stepmotor_ctr.o ./SPIA2D.o ./COMfunc.o ./RTCfunc.o ./main.o ./MODSERIAL/FLUSH.o ./MODSERIAL/example_dma.o ./MODSERIAL/example2.o ./MODSERIAL/MODSERIAL.o ./MODSERIAL/example3a.o ./MODSERIAL/GETC.o ./MODSERIAL/ISR_TX.o ./MODSERIAL/example1.o ./MODSERIAL/INIT.o ./MODSERIAL/MODSERIAL_IRQ_INFO.o ./MODSERIAL/PUTC.o ./MODSERIAL/RESIZE.o ./MODSERIAL/ISR_RX.o ./MODSERIAL/example3b.o 
OBJECTS_DOS = .\stepmotor_ctr.o .\SPIA2D.o .\COMfunc.o .\RTCfunc.o .\main.o 
OBJECTS_DOS_ALL = .\COMfunc.o .\RTCfunc.o .\main.o .\MODSERIAL\FLUSH.o .\MODSERIAL\example_dma.o .\MODSERIAL\example2.o .\MODSERIAL\MODSERIAL.o .\MODSERIAL\example3a.o .\MODSERIAL\GETC.o .\MODSERIAL\ISR_TX.o .\MODSERIAL\INIT.o .\MODSERIAL\MODSERIAL_IRQ_INFO.o .\MODSERIAL\PUTC.o .\MODSERIAL\RESIZE.o
 SYS_OBJECTS = ./mbed/LPC1768/GCC_CS/sys.o ./mbed/LPC1768/GCC_CS/cmsis_nvic.o ./mbed/LPC1768/GCC_CS/system_LPC17xx.o ./mbed/LPC1768/GCC_CS/core_cm3.o ./mbed/LPC1768/GCC_CS/startup_LPC17xx.o
 
INCLUDE_PATHS = -I. -I./mbed -I./mbed/LPC1768 -I./mbed/LPC1768/GCC_CS -I./MODSERIAL 
LIBRARY_PATHS = -L./mbed/LPC1768/GCC_CS 
LIBRARIES = -lmbed -lcapi 
LINKER_SCRIPT = ./mbed/LPC1768/GCC_CS/LPC1768.ld

############################################################################### 
CC = $(GCC_BIN)arm-none-eabi-gcc
CPP = $(GCC_BIN)arm-none-eabi-g++
CC_FLAGS = -c -Os -fno-common -fmessage-length=0 -Wall -fno-exceptions -mcpu=cortex-m3 -mthumb -ffunction-sections -fdata-sections 
ONLY_C_FLAGS = -std=gnu99
ONLY_CPP_FLAGS = -std=gnu++98
CC_SYMBOLS = -DTARGET_LPC1768 -DTOOLCHAIN_GCC_CS -DNDEBUG


AS = $(GCC_BIN)arm-none-eabi-as

LD = $(GCC_BIN)arm-none-eabi-gcc
LD_FLAGS = -mcpu=cortex-m3 -mthumb -Wl,--gc-sections
LD_SYS_LIBS = -lstdc++ -lsupc++ -lm -lc -lgcc

OBJCOPY = $(GCC_BIN)arm-none-eabi-objcopy

        
# cs-make all
all: $(PROJECT).bin
#	del .\main.o

#cs-make clean
clean:
	del $(PROJECT).elf 
	del $(OBJECTS_DOS)
	del $(MBEDDRIVE)\$(PROJECT).bin

.s.o:
	$(AS)  $(CC_FLAGS) $(CC_SYMBOLS) -o $@ $<

.c.o:
	$(CC)  $(CC_FLAGS) $(CC_SYMBOLS) $(ONLY_C_FLAGS)   $(INCLUDE_PATHS) -o $@ $<
	$(CC)  $(CC_FLAGS) $(CC_SYMBOLS) $(ONLY_C_FLAGS)   $(INCLUDE_PATHS) -MM -> $@ ./.depend

	
.cpp.o:
	$(CPP) $(CC_FLAGS) $(CC_SYMBOLS) $(ONLY_CPP_FLAGS) $(INCLUDE_PATHS) -o $@ $<




$(PROJECT).elf: $(OBJECTS) $(SYS_OBJECTS)
	$(LD) $(LD_FLAGS) -T$(LINKER_SCRIPT) $(LIBRARY_PATHS) -o $@ $^ $(LIBRARIES) $(LD_SYS_LIBS) $(LIBRARIES) $(LD_SYS_LIBS)

# copy the binary output *.elf to mBed MBEDDRIVE\*.bin
# so that no manually copy *.bin to mBed is required. 
# Press 'RESET' at mBed to start executing the program
$(PROJECT).bin: $(PROJECT).elf
	$(OBJCOPY) -O binary $< $@
	$(OBJCOPY) -O binary $< $(MBEDDRIVE)\$@
