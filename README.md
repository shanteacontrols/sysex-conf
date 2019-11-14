[![Build Status](https://github.com/paradajz/sysex-conf/workflows/Build+Test/badge.svg?branch=master)](https://github.com/paradajz/sysex-conf/actions)

# MIDI SysEx configuration protocol

User-configurable protocol for configuration of MIDI devices using System Exclusive messages. Protocol is based on three core commands:

- GET - used to retrieve values from target
- SET - used to update values on target
- BACKUP - used to retrieve values from target after which command is reformatted to SET command

The following functions are declared pure virtual and should be implemented in derived class by user:

    //used for parameter retrieval
    virtual bool onGet(uint8_t block, uint8_t section, uint16_t index, sysExParameter_t &value) = 0;

    //used to set new parameter value
    virtual bool onSet(uint8_t block, uint8_t section, uint16_t index, sysExParameter_t newValue) = 0;

    //used to define custom logic for handling custom requests (if any are defined)
    virtual bool onCustomRequest(uint8_t value) = 0;

    //used to send formatted midi array
    virtual void onWrite(uint8_t *sysExArray, uint8_t size) = 0;

Reference implementation can be found in [OpenDeck](https://github.com/paradajz/OpenDeck) repository.
