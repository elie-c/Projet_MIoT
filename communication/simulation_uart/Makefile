CONTIKI_PROJECT = simulation-uart
all: $(CONTIKI_PROJECT)

# Ensure PRIx64 is defined for builds when <inttypes.h> isn't included
# and avoid treating printf format warnings as errors during cross-builds.
CFLAGS += -DPRIx64=\"llx\" -Wno-error=format

CONTIKI=../../../../contiki-ng
include $(CONTIKI)/Makefile.include
