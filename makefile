##maybe define some macros to include code optimization by compiler "-02"
##PA_USE_COREAUDIO=1
CC = clang
CFLAGS =-DPA_USE_COREAUDIO=1
OBJ = data_types.o input_handling.o log_actions.o main_blocking_stack.o opcodes.o \
	programm\ state.o util_opcodes.o pa_allocation.o pa_converters.o pa_cpuload.o\
	pa_debugprint.o pa_dither.o pa_front.o pa_mac_core_blocking.o\
	pa_mac_core_utilities.o pa_mac_core.o pa_process.o pa_ringbuffer.o\
	pa_stream.o pa_trace.o pa_unix_hostapis.o pa_unix_util.o
LIBS = CoreAudio -framework AudioToolbox -framework AudioUnit\
	-framework CoreFoundation -framework CoreServices
bsound: $(OBJ)
	$(CC) $(CFLAGS) -o bsound $(OBJ) -lcurses -framework $(LIBS)
programm\ state.o: programm\ state.c programm\ state.h
	$(CC) -c programm\ state.c -o programm\ state.o
###.PHONY: clean
clean:
	-rm -f *.o
