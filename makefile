CC = clang
CFLAGS =-DPA_USE_COREAUDIO=1 -o2 #moderate optimization
SRCDIR =./src/
PDIR =./portaudio/
_SRC = data_types.o input_handling.o log_actions.o main_blocking_stack.o opcodes.o \
	programm_state.o util_opcodes.o
SRC = $(patsubst %, $(SRCDIR)%, $(_SRC) )
_PORTSRC = pa_allocation.o pa_converters.o pa_cpuload.o\
	pa_debugprint.o pa_dither.o pa_front.o pa_mac_core_blocking.o\
	pa_mac_core_utilities.o pa_mac_core.o pa_process.o pa_ringbuffer.o\
	pa_stream.o pa_trace.o pa_unix_hostapis.o pa_unix_util.o
PORTSRC = $(patsubst %, $(PDIR)%, $(_PORTSRC))
LIBS = CoreAudio -framework AudioToolbox -framework AudioUnit\
	-framework CoreFoundation -framework CoreServices
bsound: $(SRC) $(PORTSRC)
	$(CC) $(CFLAGS) -o bsound $(SRC) $(PORTSRC) -lcurses -framework $(LIBS)
	
##it would be go to force recompilation when header files change, as well
.PHONY: clean
clean: ;-rm -f $(PDIR)*.o
	-rm -f $(SRCDIR)*.o
