# Firmware tests

These are tests aimed at checking the compliance of the MightyBoard firmware to the s3g spec.


## Makerbot Test Explanations

### CommonFunctionTests
This suite tests any shared functions contained within Makerbottests.py

### s3gPacketTests
This suite tests the packets that are sent to the machine, and ensures the proper errors (if any) are thrown if a malformed packet is sent to a bot

### s3gSendReceiveTests
This suite tests all functions of the s3g python module, and ensures that all packets can be sent to the machine, and that the machine sends the proper response back.  These tests are pretty simple, and only involve executing the command, and asserting true if no errors are thrown (any errors that arrise will prohibit the assertion from being executed)

### s3gFunctionTests
These tests ensure that the machine processes the command correctly and takes the correct action

### s3gSDCardTests - these tests have not been verified with the current toolchain and are in draft form
This suite tests the machine operations that revolve around the SD card.  They are separate from the s3gFunctionTests because they require an SD card to be inserted into the bot.  The SD card in the bot should have the same contents of testFiles

### User Tests - these tests have not been verified with the current toolchain and are in draft form
This suite of tests requires user interaction as there is no automated process to verify correct behavior.  These tests have been beta tested once but are non verifed with the latest tool chain.
