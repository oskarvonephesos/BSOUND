CC = clang
CFLAGS =-DPA_USE_COREAUDIO=1 -o3
#add this for debugging: -DMAC_CORE_VERBOSE_DEBUG  or -DMAC_CORE_VERY_VERBOS_DEBUG
# this changes search directory #-isysroot /Library/Developer/CommandLineTools/MacOSX10.15.sdk/##-o2 is a more moderate optimization, but this doesn't seem to do very much
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
LIBS = -framework CoreAudio -framework AudioToolbox -framework AudioUnit\
	-framework CoreFoundation -framework CoreServices
bsound: $(SRC) $(PORTSRC)
	$(CC) $(CFLAGS) -o bsound $(SRC) $(PORTSRC) -lcurses  $(LIBS)

##this is very ugly. there has to be a more elegant way of solving this
$(SRC): $(SRCDIR)globaltypes.h
$(SRCDIR)data_types.o: $(SRCDIR)data_types.c $(SRCDIR)data_types.h
$(SRCDIR)input_handling.o: $(SRCDIR)input_handling.c $(SRCDIR)input_handling.h
$(SRCDIR)log_actions.o: $(SRCDIR)log_actions.c $(SRCDIR)log_actions.h
$(SRCDIR)opcodes.o: $(SRCDIR)opcodes.c $(SRCDIR)opcodes.h
$(SRCDIR)programm_state.o: $(SRCDIR)programm_state.c $(SRCDIR)programm_state.h
$(SRCDIR)util_opcodes.o: $(SRCDIR)util_opcodes.c $(SRCDIR)util_opcodes.h
$(SRCDIR)main_blocking_stack.o: $(SRCDIR)data_types.h $(SRCDIR)input_handling.h $(SRCDIR)log_actions.h\
	$(SRCDIR)opcodes.h $(SRCDIR)programm_state.h $(SRCDIR)util_opcodes.h
.PHONY: clean
clean: ;-rm -f $(PDIR)*.o
	-rm -f $(SRCDIR)*.o
