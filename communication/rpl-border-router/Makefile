CONTIKI_PROJECT = border-router
all: $(CONTIKI_PROJECT)
CONTIKI=../../../../contiki-ng

# Ensure PRIx64 is defined for builds when <inttypes.h> isn't included
# and avoid treating printf format warnings as errors during cross-builds.
CFLAGS += -DPRIx64=\"llx\" -Wno-error=format

# The BR is either native or embedded, and in the latter case must support SLIP
PLATFORMS_EXCLUDE = z1

# Include RPL BR module
include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_SERVICES_DIR)/rpl-border-router
# Include webserver module
MODULES_REL += webserver

include $(CONTIKI)/Makefile.include
