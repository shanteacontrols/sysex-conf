[![Build Status](https://github.com/paradajz/sysex-conf/workflows/Build+Test/badge.svg?branch=master)](https://github.com/paradajz/sysex-conf/actions)

# MIDI SysEx configuration protocol

User-configurable protocol for configuration of MIDI devices using System Exclusive messages. Protocol is based on three core commands:

- GET - used to retrieve values from target
- SET - used to update values on target
- BACKUP - used to retrieve values from target after which command is reformatted to SET command

Reference implementation can be found in [OpenDeck](https://github.com/paradajz/OpenDeck) repository.
