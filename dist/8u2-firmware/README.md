USB on the MightyBoard is handled by an ATmega8u2 chip running its own firmware.
That chip does not have its own bootloader.  It is programmed via the 6pin ISP
progamming header labeled "8u2" on the rev E, G, and H MightyBoards.

The actual USB circuit is based upon that of the Arduino Uno Rev 2, including
the [problem caused by capacitor C20](http://www.google.com/url?q=http%3A%2F%2Fforum.arduino.cc%2Findex.php%3Ftopic%3D86389.0&sa=D&sntz=1&usg=AFQjCNEmVejtPTZeCCxMuijoyGosSQ5Gag).
(C20 can accidentally act as a voltage doubler.)

Now, different firmware builds should be used in the 8u2 on a rev E MightyBoard
(Replicator 1) than that used in a rev G or H MightyBoard (Replicator 2 and 2X).

* Replicator 1 (rev E) -- `usbserial-rep1.hex`
* Replicator 2, 2X (rev G, H) -- `usbserial-rep2.hex`

Here is the source of the 8u2 firmware,

  [https://github.com/makerbot/MightyBoardFirmware/blob/cae45acb712b2b68e911dd6ab18c28c375cfbb37/dist/MightyBoard/Makerbot-usbserial.hex](https://github.com/makerbot/MightyBoardFirmware/blob/cae45acb712b2b68e911dd6ab18c28c375cfbb37/dist/MightyBoard/Makerbot-usbserial.hex)

It dates back to this November 2012 MBI checkin,

  [https://github.com/makerbot/MightyBoardFirmware/commit/efd8b5c9842a23f6526b0a171d2c2e6796af6915](https://github.com/makerbot/MightyBoardFirmware/commit/efd8b5c9842a23f6526b0a171d2c2e6796af6915)

Later on, in 2013, MBI stopped worrying about the 8u2 bootloader on rev E
electronics.  Moreover, they stopped having a separate 8u2 hex file for rev E
vs. rev G electronics: they went to a single hex file.  Their decision to do
that likely stems from them having ceased development of the Replicator 1
in 2012 and before they released the Replicator 2 in October of 2012.  And
at some point, MBI probably stopped testing 8u2 firmware changes on rev E
hardware.  Indeed, FlashForge (or their board fab) had picked up a newer 8u2
firmware from the MakerBot github repository, started using it on their rev E
clone MightyBoard, and then had some problems.  (With capacitor C20 installed,
tyou couldn't connect to the bot over USB without having the bot reset itself
multiple times.)