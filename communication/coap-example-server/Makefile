CONTIKI_PROJECT = coap-example-server
all: $(CONTIKI_PROJECT)


# Ensure PRIx64 is defined for builds when <inttypes.h> isn't included
# and avoid treating printf format warnings as errors during cross-builds.
CFLAGS += -DPRIx64=\"llx\" -Wno-error=format

# Do not try to build on Sky because of code size limitation
PLATFORMS_EXCLUDE = sky z1

# Include CoAP resources
MODULES_REL += ./resources

CONTIKI=../../../../contiki-ng

# Include the CoAP implementation
include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_APP_LAYER_DIR)/coap

include $(CONTIKI)/Makefile.include
