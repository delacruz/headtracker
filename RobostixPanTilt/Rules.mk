#################\##########################################################
#
# Rules.mk - Contains common makefile support for the Robostix sameples
#
###########################################################################

ifeq ($(AVR_MCU),)
AVR_MCU = atmega128
endif

all: $(TARGET)

$(TARGET) : % : %.hex
				 
.PHONY: all $(TARGET)

CROSS_COMPILE	= avr-

CC		= $(CROSS_COMPILE)gcc
OBJCOPY	= $(CROSS_COMPILE)objcopy

AVR_MCU_FLAGS	= -mmcu=$(AVR_MCU)

vpath %.c ../Common ../Shared
vpath %.S ../Common ../Shared

CFLAGS += -Os \
	-Wall \
	-Werror \
	-Wimplicit \
	-Wpointer-arith \
	-Wswitch \
	-Wredundant-decls \
	-Wreturn-type \
	-Wshadow \
	-Wunused \
	-Wcast-qual \
	-Wnested-externs \
	-Wmissing-prototypes \
	-Wstrict-prototypes \
	-Wmissing-declarations

CPPFLAGS += $(AVR_MCU_FLAGS) -I . -I ../Common -I ../Shared
LDFLAGS  += $(AVR_MCU_FLAGS) -Wl,-Map,$(basename $@).map

DEP_OUTPUT_OPTION = -MMD -MF $(@:.o=.d)

ECHO	= @echo
RM		= rm

COMMON_DEPS = $(strip $(COMMON_OBJS:.o=.d))

DEP_FILES = $(TARGET).d $(COMMON_DEPS)

#--------------------------------------------------------------------------
#
# 	make 3.80 introduced a new variable, called MAKEFILE_LIST. The
#	last word in that variable will be the name of this makefile. We
#	use this to figure out the directory that Rules.mk is located in
#	and hence avr-mem.sh.
#
#	For earlier versions of make, the use will need to put avr-mem.sh
#	somewhere in their PATH
#

ifneq ($(MAKEFILE_LIST),)

RULES_MK_DIR := $(dir $(shell pwd)/$(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST)))

export PATH := $(PATH):$(RULES_MK_DIR)

endif

#--------------------------------------------------------------------------
#
# 	Run make with v=1 or verbose=1 to get verbose output
#

ifeq ($(v),)
export verbose = 0
else
export verbose = 1
endif

ifeq ($(verbose),)
export verbose = 0
endif

ifeq ($(verbose),0)
	Q = @
	MAKEFLAGS += -s
else
	Q =
endif
export Q

#--------------------------------------------------------------------------
#
#	CFLAGS 		= flags used for C compiler
#	CXXFLAGS	= flags for the C++ compiler
#	CPPFLAGS	= flags for the C preprocessor (-D, -I)
#

PREPROCESS.c = $(CC) $(CPPFLAGS) $(TARGET_ARCH) -E -Wp,-C,-dD,-dI

#--------------------------------------------------------------------------
#
#	The FORCE target can be used for things that you want to have rebuild
# 	every single time (like *.pp files)
#

FORCE:

.PHONY: FORCE

#--------------------------------------------------------------------------
#
# Compile C source files
#
#   COMPILE.c = $(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
#
# Create a rule for when the dependency exists, and one for when it doesn't
#

.PRECIOUS: %.o

%.o : %.c %.d
	$(ECHO) "Compiling $< ..."
	$(Q)$(COMPILE.c) $(DEP_OUTPUT_OPTION) $(OUTPUT_OPTION) $<

%.o : %.c
	$(ECHO) "Compiling $< ..."
	$(Q)$(COMPILE.c) $(DEP_OUTPUT_OPTION) $(OUTPUT_OPTION) $<

#--------------------------------------------------------------------------
#
# Assemble gas assembly files.
#
#   COMPILE.c = $(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
#

%.o : %.S
	$(ECHO) "Assembling $< ..."
	$(Q)$(COMPILE.S) $(OUTPUT_OPTION) $<


# Making the .d's be phony and providing a dummy rule to make them, combined
# with the %.o : %.c %.d causes make to work properly when the .d file is
# removed, but the .o file isn't. make will now regenerate the .d by 
# recompiling the .o
	
.PHONY: %.d
%.d: ;

#--------------------------------------------------------------------------
#
# Generate Preprocessed files from C source (useful for debugging)
#

%.pp : %.c FORCE
	$(ECHO) "Preprocessing $< ..."
	$(Q)$(PREPROCESS.c) $< > $@

#--------------------------------------------------------------------------
#
# Generate C/asm listing
#

%.cod : %.c FORCE
	$(ECHO) "Listing $< ..."
	$(Q)$(COMPILE.c) -gstabs -Wa,-ahdlms=$@ $<

#--------------------------------------------------------------------------
#
# Create a hex file from an elf file
#

%.hex : %.elf
	$(ECHO) "Creating $@ ..."
	$(Q)$(OBJCOPY) -j .text -j .data -O ihex $< $@
	$(ECHO)

#--------------------------------------------------------------------------
#
# Create the EEPROM hex file from an elf file
#

%-eep.hex : %.elf
	$(ECHO) "Creating $@ ..."
	$(Q)$(OBJCOPY) -j .eeprom -O ihex $< $@
	$(ECHO)

#--------------------------------------------------------------------------
#
# Create simple executables
#

.PRECIOUS: %.elf

ifeq ($(MAIN_OBJS),)
MAIN_OBJS = $(TARGET).o
endif

$(TARGET).elf : $(MAIN_OBJS) $(COMMON_OBJS)
	$(ECHO) "Linking $@ ..."
	$(Q)$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@
	$(ECHO)
	avr-mem.sh $@ $(AVR_MCU)
	$(ECHO)

clean: clean-other clean-hex

clean-other:
	$(ECHO) "Removing generated files ..."
	$(Q)$(RM) -f *.d *.o *.elf *.map

clean-hex:
	$(ECHO) "Removing hex files ..."
	$(Q)$(RM) -f *.hex

#--------------------------------------------------------------------------
#
# Include dependencies.
#
# The '-' at the beginning tells make not to complain about missing
# dependency files.
#

ifneq ($(DEP_FILES),)
ifeq ($(strip $(filter clean% exec print-%, $(MAKECMDGOALS))),)
-include $(DEP_FILES)
endif
endif
	
