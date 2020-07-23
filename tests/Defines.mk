ifeq ($(TARGETNAME), 2b)
    DEFINES += SYSEX_2B
else
    DEFINES += SYSEX_1B
endif