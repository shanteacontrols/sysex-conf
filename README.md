[![Build Status](https://travis-ci.org/paradajz/sysex-conf.svg?branch=master)](https://travis-ci.org/paradajz/sysex-conf)
[![Coverage Status](https://coveralls.io/repos/github/paradajz/sysex-conf/badge.svg?branch=master&service=github)](https://coveralls.io/github/paradajz/sysex-conf?branch=master)

# MIDI SysEx configuration protocol

User-configurable protocol for configuration of MIDI devices using System Exclusive messages. Protocol is based on three core commands:

- GET - used to retrieve values from target
- SET - used to update values on target
- BACKUP - used to retrieve values from target after which command is reformatted to SET command

All commands are configured using callbacks:

    ::setHandleGet(onGet_callback);
    ::setHandleSet(onSet_callback);

Protocol doesn't have any dependencies. Sending of MIDI data is also done using callback:

    ::setHandleSysExWrite(writeSysEx_callback);

Callback is called when response is ready to be sent.

Reference implementation can be found in [OpenDeck](https://github.com/paradajz/OpenDeck) repository.
