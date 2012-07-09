import os
import sys
lib_path = os.path.abspath('../')
sys.path.append(lib_path)

"""
A suite of tests to be run on a replicator with the s3g python module.  These tests are broken down into several categories:
  CommonFunctionTests: Tests that ensure functions used by all test cases are valid
  s3gPacketTests: Tests that build malformed packets (i.e. long length, no header, bad crc, etc) and send them off to the replicator to ensure she rejects them
  s3gSendReceiveTest: Tests that make sure the replicator understands all the commands it needs to, and rejects the ones it doesnt understand
  s3gFunctionTests: The meat of this test class; makes sure all commands understodd by the replicator are executed correctly.  Currently this requires some user feedback.  A test rig should be assembled to circumvent user interaction.
  SDCardTests: Tests the ensure the replicator can communicate with its SD card port.  this is broken out into a separate test suite due to its dependence on a set of test files located in ./testFiles/
"""
import os, sys
lib_path = os.path.abspath('../')
sys.path.append(lib_path)

try:
    import unittest2 as unittest
except ImportError:
    import unittest

import optparse
import serial
import io
import struct
import array
import time
import s3g


extensive = True
port = ''
hasInterface = True


def ConvertFromNUL(b):
  if b[-1] != 0:
    raise TypeError("Cannot convert from non-NUL terminated string")
  if len(b) == 1:
    return ''
  return str(b[:-1])

class commonFunctionTests(unittest.TestCase):
 
  def test_ConvertFromNUL(self):
    b = bytearray("asdf\x00")
    expectedReturn = "asdf"
    self.assertEqual(expectedReturn, ConvertFromNUL(b))

class s3gPacketTests(unittest.TestCase):
  def setUp(self):
    self.s3g = s3g.s3g()
    self.s3g.file = serial.Serial(options.serialPort,'115200', timeout=1)

  def tearDown(self):
    self.s3g.file.close()

  def GetVersionPayload(self):
    payload = bytearray()
    payload.append(s3g.host_query_command_dict['GET_VERSION'])
    payload.extend(s3g.encode_uint16(s3g.s3g_version))
    return payload

  def GetVersionPacket(self):
    """
    Helper method to generate a Get Version packet to be modified and sent
    """
    return s3g.encode_payload(self.GetVersionPayload())

  def test_GetVersionPayload(self):
    payload = self.GetVersionPayload()
    self.assertEqual(payload[0], s3g.host_query_command_dict['GET_VERSION'])
    self.assertEqual(payload[1:], s3g.encode_uint16(s3g.s3g_version))

  def test_GetVersionPacket(self):
    testPayload = self.GetVersionPayload()
    packet = self.GetVersionPacket()
    self.assertEqual(packet[0], s3g.header)
    self.assertEqual(packet[1], len(packet[2:-1]))
    self.assertEqual(packet[2:-1], testPayload)
    self.assertEqual(packet[-1], s3g.CalculateCRC(testPayload))

  def test_NoHeader(self):
    packet = self.GetVersionPacket()
    packet[0] = '\x00'
    self.assertRaises(s3g.TransmissionError, self.s3g.send_packet, packet)
      
  def test_EmptyPacket(self):
    packet = bytearray()
    self.assertRaises(s3g.TransmissionError, self.s3g.send_packet, packet)

  def test_TrailingPacket(self):
    packet = self.GetVersionPacket()
    addition = bytearray('\xff\xff')
    packet.extend(addition)
    self.s3g.send_packet(packet)

  def test_PreceedingPacket(self):
    packet = self.GetVersionPacket()
    addition = bytearray('\xa4\x5f')
    addition.extend(packet)
    self.s3g.send_packet(addition)

  def test_BadCRC(self):
    packet = self.GetVersionPacket()
    payload = packet[2:-1]
    crc = s3g.CalculateCRC(payload)
    packet[-1] = crc+1 
    self.assertRaises(s3g.TransmissionError, self.s3g.send_packet, packet)

  def test_LongLength(self):
    packet = self.GetVersionPacket()
    packet[1] = '\x0f'
    self.assertRaises(s3g.TransmissionError, self.s3g.send_packet, packet)

  def test_ShortLength(self):
    packet = self.GetVersionPacket()
    packet[1] = '\x00'
    self.assertRaises(s3g.ProtocolError, self.s3g.send_packet, packet)

  def test_LongPayload(self):
    packet = self.GetVersionPacket()
    packet.insert(2, '\x00')
    self.assertRaises(s3g.TransmissionError, self.s3g.send_packet, packet)

  def test_ShortPayload(self):
    packet = self.GetVersionPacket()
    packet = packet[0:2] + packet[3:]
    self.assertRaises(s3g.TransmissionError, self.s3g.send_packet, packet)

  def test_MaxLength(self):
    FreeEEPROMSpace = 0x01D1
    b = bytearray(s3g.maximum_payload_length-4)
    self.s3g.write_to_EEPROM(FreeEEPROMSpace, b)

  def test_OversizedLength(self):
    payload = bytearray(s3g.maximum_payload_length+1)
    self.assertRaises(s3g.PacketLengthError, s3g.encode_payload, payload)

class s3gSendReceiveTests(unittest.TestCase):
  def setUp(self):
    self.s3g = s3g.s3g()
    self.s3g.file = serial.Serial(options.serialPort, '115200', timeout=1)

  def tearDown(self):
    self.s3g.file.close()
    self.s3g.file = None

  def test_ResetToFactoryReply(self):
    self.s3g.reset_to_factory()

  def test_QueueSongReply(self):
    self.s3g.queue_song(1)

  def test_SetBuildPercentReply(self):
    self.s3g.set_build_percent(100)

  def test_SetBeepReply(self):
    self.s3g.set_beep(1000, 3)

  def test_SetPotentiometerValueReply(self):
    self.s3g.set_potentiometer_value(['x', 'y', 'z', 'a', 'b'],0)

  def test_SetRGBLEDReply(self):
    self.s3g.set_RGB_LED(0, 255, 0, 0)

  def test_WaitForButtonReply(self):
    self.s3g.wait_for_button('up', 0, True, False, False)

  def test_ToolheadInitReply(self):
    self.s3g.toolhead_init(0)

  def test_GetPIDStateReply(self):
    self.s3g.get_PID_state(0)

  def testGetToolStatusReply(self):
    self.s3g.get_tool_status(0)

  def test_StoreHomePositionsReply(self):
    self.s3g.store_home_positions(['x', 'y', 'z', 'a', 'b'])

  def test_RecallHomePositionsReply(self):
    self.s3g.recall_home_positions(['x', 'y', 'z', 'a', 'b'])

  def test_QueueExtendedPointNewReply(self):
    self.s3g.queue_extended_point_new([0, 0, 0, 0, 0], 1, ['x', 'y', 'z', 'a', 'b'])

  def test_ToggleAxesReply(self):
    self.s3g.toggle_axes(['x', 'y', 'z', 'a', 'b'], True)

  def test_WaitForPlatformReply(self):
    self.s3g.wait_for_platform_ready(0, 100, 50)

  def test_WaitForToolReadyReply(self):
    self.s3g.wait_for_tool_ready(0, 100, 50)

  def test_DelayReply(self):
    self.s3g.delay(10)

  def test_GetCommunicationStatsReply(self):
    self.s3g.get_communication_stats()

  def test_GetMotherboardStatusReply(self):
    self.s3g.get_motherboard_status()

  def test_ExtendedStopReply(self):
    self.s3g.extended_stop(True, True)

  def test_CaptureToFileReply(self):
    self.s3g.capture_to_file('test')

  def test_EndCaptureToFileReply(self):
    self.s3g.end_capture_to_file()

  def test_ResetReply(self):
    self.s3g.reset()

  def test_IsFinishedReply(self):
    self.s3g.is_finished()

  def test_PauseReply(self):
    self.s3g.pause()

  def test_ClearBufferReply(self):
    self.s3g.clear_buffer()

  def test_InitReply(self):
    self.s3g.init()

  def test_IsPlatformReadyReply(self):
    self.s3g.is_platform_ready(0)

  def test_GetPlatformTargetTemperatureReply(self):
    self.s3g.get_platform_target_temperature(0)

  def test_GetToolheadTargetTemperatureReply(self):
    self.s3g.get_toolhead_target_temperature(0)

  def test_ReadFromToolheadEEPROMReply(self):
    self.s3g.read_from_toolhead_EEPROM(0, 0x00, 0)

  def test_IsToolReadyReply(self):
    self.s3g.is_tool_ready(0)

  def test_GetToolheadTemperatureReply(self):
    self.s3g.get_toolhead_temperature(0)

  def test_GetPlatformTemperatureReply(self):
    self.s3g.get_platform_temperature(0)

  def test_BuildEndNotificationReply(self):
    self.s3g.build_end_notification()

  def test_BuildStartNotificationReply(self):
    self.s3g.build_start_notification(0, 'aTest')

  def test_DisplayMessageReply(self):
    if hasInterface:
      self.s3g.display_message(0, 0, "TESTING", 1, False, False, False)

  def test_FindAxesMaximumsReply(self):
    self.s3g.find_axes_maximums(['x', 'y', 'z'], 1, 0)

  def test_FindAxesMinimumsReply(self):
    self.s3g.find_axes_minimums(['x', 'y', 'z'], 1, 0)

  def test_GetBuildNameReply(self):
    self.s3g.get_build_name()

  def test_GetNextFilenameReply(self):
    self.s3g.get_next_filename(False)

  def test_PlaybackCaptureReply(self):
    self.assertRaises(s3g.SDCardError, self.s3g.playback_capture, 'aTest')

  def test_AbortImmediatelyReply(self):
    self.s3g.abort_immediately()

  def test_GetAvailableBufferSizeReply(self):
    self.s3g.get_available_buffer_size()

  def test_ReadFromEEPROMReply(self):
    self.s3g.read_from_EEPROM(0x00, 0)

  def test_GetVersionReply(self):
    self.s3g.get_version()

  def test_SetPlatformTemperatureReply(self):
    temperature = 100
    toolhead = 0
    self.s3g.set_platform_temperature(toolhead, temperature)
    self.s3g.set_platform_temperature(toolhead, 0)

  def test_SetToolheadTemperatureReply(self):
    temperature = 100
    toolhead = 0
    self.s3g.set_toolhead_temperature(toolhead, temperature)
    self.s3g.set_toolhead_temperature(toolhead, 0)

  def test_GetPositionReply(self):
    self.s3g.get_position()

  def test_GetExtendedPositionReply(self):
    self.s3g.get_extended_position()

  def test_QueuePointReply(self):
    position = [0, 0, 0]
    rate = 500
    self.s3g.QueuePoint(position, rate)

  def test_SetPositionReply(self):
    position = [0, 0, 0]
    self.s3g.set_position(position)

  def test_QueueExtendedPointReply(self):
    position = [0, 0, 0, 0, 0]
    rate = 500
    self.s3g.queue_extended_point(position, rate)

  def test_SetExtendedPositionReply(self):
    position = [0, 0, 0, 0, 0]
    self.s3g.set_extended_position(position)

class s3gFunctionTests(unittest.TestCase):

  def setUp(self):
    self.s3g = s3g.s3g()
    self.s3g.file = serial.Serial(options.serialPort, '115200', timeout=1)
    self.s3g.writer = s3g.StreamWriter(self.s3g.file)
    self.s3g.set_extended_position([0, 0, 0, 0, 0])
    self.s3g.abort_immediately()
    time.sleep(5)

  def tearDown(self):
    self.s3g.file.close()

  def test_GetVersion(self):
    expectedVersion = raw_input("\nWhat is the version number of your bot? ")
    expectedVersion = int(expectedVersion.replace('.', '0'))
    self.assertEqual(expectedVersion, self.s3g.get_version())

  def test_GetPlatformTemperature(self):
    if extensive:
      obsvTemperature = raw_input("\nWhat is the current platform temperature? ")
      self.assertEqual(str(self.s3g.get_platform_temperature(0)), str(obsvTemperature))

  def test_GetToolheadTemperature(self):
    if extensive:
      obsvTemperature = raw_input("\nWhat is the right extruder's current temperature? ")
      self.assertEqual(str(self.s3g.get_toolhead_temperature(0)), str(obsvTemperature))

  def test_SetPlatformTargetTemperature(self):
    if extensive:
      tolerance = 2
      target = 50
      self.s3g.set_platform_temperature(0, target)
      minutes = 3
      print "\nWaiting %i mintues to heat the Platform up"%(minutes)
      time.sleep(60*minutes)
      self.assertTrue(abs(self.s3g.get_platform_temperature(0)-target) <= tolerance)
      self.s3g.set_platform_temperature(0, 0)

  def test_SetToolheadTemperature(self):
    if extensive:
      tolerance = 2
      target = 50
      self.s3g.set_toolhead_temperature(0, target)
      minutes = 3
      print "\nWaiting %i minutes to heat the Toolhead up"%(minutes)
      time.sleep(60*minutes)
      self.assertTrue(abs(self.s3g.get_toolhead_temperature(0) - target) <= tolerance)
      self.s3g.set_toolhead_temperature(0, 0)

  def test_GetToolheadTargetTemperature(self):
    target = 100
    toolhead = 0
    self.s3g.set_toolhead_temperature(toolhead, target)
    self.assertEqual(self.s3g.get_toolhead_target_temperature(toolhead), target)
    self.s3g.set_toolhead_temperature(toolhead, 0)

  def test_GetPlatformTargetTemperature(self):
    target = 100
    self.s3g.set_platform_temperature(0, target)
    self.assertEqual(self.s3g.get_platform_target_temperature(0), target)
    self.s3g.set_platform_temperature(0, 0)

  def test_ReadFromEEPROMMighty(self):
    """
    Read the VID/PID settings from the MB and compare against s3g's read from eeprom
    """
    vidPID = self.s3g.read_from_EEPROM(0x0044, 2)
    vidPID = s3g.decode_uint16(vidPID)
    mightyVIDPID = [0x23C1, 0xB404]
    self.assertEqual(vidPID, mightyVIDPID[0])

  def test_WriteToEEPROMMighty(self):
    nameOffset = 0x0022
    nameSize = 16
    name = 'ILOVETESTINGALOT'
    self.s3g.write_to_EEPROM(nameOffset, name)
    readName = self.s3g.read_from_EEPROM(nameOffset, 16)
    self.assertEqual(name, readName)

  def test_IsPlatformReady(self):
    """
    Determine if the platform is ready by setting the temperature to its current reading and asking if its ready (should return true, then setting the temperature to double what it is now then querying it agian, expecting a false answer
    """
    curTemp = self.s3g.get_platform_temperature(0)
    self.s3g.set_platform_temperature(0, curTemp)
    self.assertTrue(self.s3g.is_platform_ready(0))
    self.s3g.set_platform_temperature(0, curTemp+50)
    self.assertEqual(self.s3g.is_platform_ready(0), False)
    self.s3g.set_platform_temperature(0, 0)

  def test_IsToolReady(self):
    toolhead = 0
    curTemp = self.s3g.get_toolhead_temperature(toolhead)
    self.s3g.set_toolhead_temperature(toolhead, curTemp)
    self.assertTrue(self.s3g.is_tool_ready(toolhead))
    self.s3g.set_toolhead_temperature(toolhead, curTemp + 50)
    self.assertEqual(self.s3g.is_tool_ready(toolhead), False)
    self.s3g.set_toolhead_temperature(toolhead, 0)

  def test_DisplayMessage(self):
    if hasInterface:
      message = str(time.clock())
      secondMsg = 'success'
      self.s3g.display_message(0, 0, message, 0, False, False, True)
      self.s3g.display_message(0, 0, secondMsg, 0, False, True, True)
      readMessage = raw_input("\nWhat is the message on the replicator's display? ")
      self.assertEqual(message, readMessage)
      obs = raw_input("\nPlease go to the interface board, press the middle button, and type the new message. ")
      self.assertEqual(obs, secondMsg)

  def test_GetPosition(self):
    position = self.s3g.get_position()
    self.assertEqual(position[0], [0, 0, 0])

  def test_GetExtendedPosition(self):
    position = self.s3g.get_extended_position()
    self.assertEqual(position[0], [0, 0, 0, 0, 0])

  def test_SetPosition(self):
    position = [50, 50, 50]
    self.s3g.set_position(position)
    self.assertEqual(position, self.s3g.get_position()[0])

  def test_SetExtendedPosition(self):
    position = [50, 51, 52, 53, 54]
    self.s3g.set_extended_position(position)
    self.assertEqual(position, self.s3g.get_extended_position()[0])

  def test_QueuePoint(self):
    newPosition = [50, 51, 52]
    rate = 500
    self.s3g.QueuePoint(newPosition, rate)
    time.sleep(5)
    self.assertEqual(newPosition, self.s3g.get_position()[0])

  def test_QueueExtendedPosition(self):
    newPosition = [51, 52, 53, 54, 55]
    rate = 500
    self.s3g.queue_extended_point(newPosition, rate)
    time.sleep(5)
    self.assertEqual(newPosition, self.s3g.get_extended_position()[0])

  def test_FindAxesMaximums(self):
    axes = ['x', 'y', 'z']
    rate = 500
    timeout = 10
    xYEndstops = 10
    self.s3g.find_axes_maximums(axes, rate, timeout)
    time.sleep(timeout)
    self.assertEqual(self.s3g.get_position()[1], xYEndstops)
    obs = raw_input("\nDid the Z Platform move towards the bottom of the machine? (y/n) ")
    self.assertEqual('y', obs)


  def test_FindAxesMinimums(self):
    axes = ['x', 'y', 'z']
    rate = 500
    timeout = 5
    self.s3g.find_axes_minimums(axes, rate, timeout)
    time.sleep(timeout)
    xyObs = raw_input("\nDid the gantry move from the back right to the front left of the machine? (y/n) ")
    self.assertEqual('y', xyObs)
    zObs = raw_input("\nDid the Z Platform move towards the top of the machine? (y/n) ")
    self.assertEqual('y', zObs)

  def test_Init(self):
    bufferSize = 512
    expectedPosition = [0, 0, 0, 0, 0]
    position = [10, 9, 8, 7, 6]
    self.s3g.set_extended_position(position)
    #Find the maximum so that if we fail, it wont try to move outside its bounds
    for i in range(5):
      self.s3g.delay(1)
    self.s3g.init()
    self.assertEqual(expectedPosition, self.s3g.get_extended_position()[0])
    self.assertEqual(self.get_available_buffer_size(), bufferSize)

  def test_GetAvailableBufferSize(self):
    bufferSize = 512
    self.assertEqual(bufferSize, self.s3g.get_available_buffer_size())

  def test_AbortImmediately(self):
    bufferSize = 512
    toolheads = [0, 1]
    for toolhead in toolheads:
      self.s3g.set_toolhead_temperature(toolhead, 100)
    self.s3g.set_platform_temperature(0, 100)
    for i in range(5):
      self.s3g.find_axes_minimums(['x', 'y', 'z'], 500, 5)
    self.s3g.abort_immediately()
    self.assertEqual(bufferSize, self.s3g.get_available_buffer_size())
    for toolhead in toolheads:
      self.assertEqual(0, self.s3g.get_toolhead_target_temperature(toolhead))
    self.assertEqual(0, self.s3g.get_platform_target_temperature(0))
    self.assertTrue(self.s3g.is_finished())

  def test_BuildStartNotification(self):
    buildName = "test"
    cc = 10
    self.s3g.build_start_notification(cc, buildName)
    readBuildName = self.s3g.get_build_name()
    readBuildName = ConvertFromNUL(readBuildName)
    self.assertEqual(buildName, readBuildName)

  def test_BuildEndNotification(self):
    noBuild = bytearray('\x00')
    self.s3g.build_start_notification(10, "test")
    self.s3g.build_end_notification()
    self.assertEqual(self.s3g.get_build_name(), noBuild)

  def test_ClearBuffer(self):
    bufferSize = 512
    axes = ['x', 'y', 'z']
    rate = 500
    timeout = 5
    for i in range(10):
      self.s3g.find_axes_minimums(axes, rate, timeout)
    self.assertNotEqual(bufferSize, self.s3g.get_available_buffer_size())
    self.s3g.clear_buffer()
    self.assertEqual(bufferSize, self.s3g.get_available_buffer_size())

  def test_Pause(self):
    """
    Because we cant query the bot to determine if its paused, we using the find_axes_maximums function to help.  We know how long it will take to traverse the build space.  If we start from the front, begin homing to the back then pause for the traversal time, we will know if we paused if we have not reached the end.  If we then unpause and wait the traversal time, we should reach the end.
    """
    yEndStop = 8
    zEndStop = 16
    axes = ['y']
    traverseTime = 5 #Time it takes for the gantry to get from the back to the front
    self.s3g.find_axes_maximums(axes, 500, traverseTime) #At the back
    time.sleep(traverseTime)
    self.s3g.find_axes_minimums(axes, 500, traverseTime) #At the front
    time.sleep(traverseTime)
    self.assertTrue(self.s3g.get_position()[1] < yEndStop or self.s3g.get_position()[1] == zEndStop) #Make sure we are in the right location
    self.s3g.find_axes_maximums(axes, 500, traverseTime*3) #Start to go to the back, give extra long timeout so we dont time out
    self.s3g.pause()
    time.sleep(traverseTime) #Wait for the machine to catch up to do the check
    self.assertTrue(self.s3g.get_position()[1] < yEndStop or self.s3g.get_position()[1] == zEndStop) #Make sure we are still in the same location
    self.s3g.pause() #Unpause
    time.sleep(traverseTime*2) #Wait for the bot to get to the end
    self.assertTrue(self.s3g.get_position()[1] == yEndStop or self.s3g.get_position()[1] == yEndStop + zEndStop) #Make sure we can unpause
    

  def test_IsFinished(self):
    axes = ['y']
    timeout = 3
    self.s3g.find_axes_maximums(axes, 500, timeout)#We dont want to move beyond our bounds
    time.sleep(timeout)
    self.s3g.find_axes_minimums(axes, 500, timeout)
    self.assertFalse(self.s3g.is_finished())
    time.sleep(timeout)
    self.assertTrue(self.s3g.is_finished())

  def test_Reset(self):
    bufferSize = 512
    for i in range(10):
      self.s3g.find_axes_minimums(['x', 'y', 'z'], 500, 10)
    self.s3g.set_toolhead_temperature(0, 100)
    self.s3g.set_platform_temperature(0, 100)
    self.s3g.reset()
    self.assertEqual(self.s3g.get_available_buffer_size(), bufferSize)
    self.assertTrue(self.s3g.is_finished())
    self.assertEqual(self.s3g.get_toolhead_target_temperature(0), 0)
    self.assertEqual(self.s3g.get_platform_target_temperature(0), 0)
 
  def test_ClearBuffer(self):
    bufferSize = 512
    axes = ['x', 'y', 'z']
    rate = 500
    timeout = 5
    for i in range(5):
      self.s3g.find_axes_minimums(axes, rate, timeout)
    self.assertNotEqual(bufferSize, self.s3g.get_available_buffer_size())
    self.s3g.clear_buffer()
    self.assertEqual(bufferSize, self.s3g.get_available_buffer_size())

  def test_CaptureToFile(self):
    filename = str(time.clock())+".s3g" #We want to keep changing the filename so this test stays nice and fresh
    self.s3g.capture_to_file(filename)
    #Get the filenames off the SD card
    files = []
    curFile = ConvertFromNUL(self.s3g.get_next_filename(True))
    while curFile != '':
      curFile = ConvertFromNUL(self.s3g.get_next_filename(False))
      files.append(curFile)
    self.assertTrue(filename in files)

  def test_GetCommunicationStats(self):
    changableInfo = ['PacketsReceived', 'PacketsSent']
    oldInfo = self.s3g.get_communication_stats()
    toSend = 5
    for i in range(toSend):
      self.s3g.is_finished()
    newInfo = self.s3g.get_communication_stats()
    for key in changableInfo:
      self.assertTrue(newInfo[key]-oldInfo[key] == toSend)

  def test_EndCaptureToFile(self):
    filename = str(time.clock())+".s3g"
    self.s3g.capture_to_file(filename)
    findAxesMaximums = 8+32+16
    numCmd = 5
    totalBytes = findAxesMaximums*numCmd/8 + numCmd
    #Add some commands to the file
    for i in range(numCmd):
      self.s3g.find_axes_maximums(['x', 'y'], 500, 10)
    self.assertEqual(totalBytes, self.s3g.end_capture_to_file())

  def test_ExtendedStop(self):
    bufferSize = 512
    self.s3g.find_axes_maximums(['x', 'y'], 200, 5)
    time.sleep(5)
    for i in range(5):
      self.s3g.find_axes_minimums(['x', 'y'], 1600, 2)
    self.s3g.extended_stop(True, True)
    time.sleep(5) #Give the machine time to response
    self.assertTrue(self.s3g.is_finished())
    self.assertEqual(bufferSize, self.s3g.get_available_buffer_size())

  @unittest.skip("delay is broken, delays in miliseconds instead of microseconds.  This woul dmake us delay for a long time, so we skip this step for now")
  def test_Delay(self):
    axes = ['x', 'y']
    feedrate = 500
    timeout = 5
    uSConst = 1000000
    zEndStop = 16
    xyEndStops = 10
    allEndStops = 26
    self.s3g.find_axes_maximums(axes, feedrate, timeout)
    time.sleep(timeout)
    testStart = time.time()
    self.s3g.find_axes_minimums(axes, feedrate, timeout)
    self.s3g.delay(timeout*uSConst)
    self.s3g.find_axes_maximums(axes, feedrate, timeout)
    while testStart + timeout*3  > time.time(): #XY endstops should be low while moving/delaying.  If not, delay didnt delay for the correct time
      self.assertTrue(self.s3g.get_position()[1] == 0 or self.s3g.get_position()[1] == zEndStop)
    time.sleep(.5) #Wait about half a second for the machine to finish its movements
    self.assertTrue(self.s3g.get_position()[1] == allEndStops or self.s3g.get_position()[1] == xyEndStops)

  def test_ToggleEnableAxes(self):
    self.s3g.ToggleEnableAxes(True, True, True, True, True, True)
    obs = raw_input("\nPlease try to move all (x,y,z) the axes!  Can you move them without using too much force? (y/n) ")
    self.assertEqual('n', obs)
    self.s3g.ToggleEnableAxes(True, True, True, True, True, False)
    obs = raw_input("\nPlease try to move all (x,y,z) the axes!  Can you move them without using too much force? (y/n) ")
    self.assertEqual('y', obs)
 
  def test_ExtendedStop(self):
    bufferSize = 512
    self.s3g.find_axes_maximums(['x', 'y'], 200, 5)
    time.sleep(5)
    for i in range(5):
      self.s3g.find_axes_minimums(['x', 'y'], 1600, 2)
    self.s3g.extended_stop(True, True)
    time.sleep(5) #Give the machine time to response
    self.assertTrue(self.s3g.is_finished())
    self.assertEqual(bufferSize, self.s3g.get_available_buffer_size())

  def test_WaitForPlatformReady(self):
    toolhead = 0
    temp = 50
    timeout = 60
    tolerance = 3
    delay = 100
    self.s3g.set_platform_temperature(toolhead, temp)
    self.s3g.wait_for_platform_ready(toolhead, delay, timeout)
    startTime = time.time()
    self.s3g.set_platform_temperature(toolhead, 0)
    while startTime + timeout > time.time() and abs(self.s3g.get_platform_temperature(toolhead) - temp) > tolerance:
      self.assertEqual(self.s3g.get_platform_target_temperature(toolhead), temp)
    time.sleep(5) #Give the bot a couple seconds to catch up
    self.assertEqual(self.s3g.get_platform_target_temperature(toolhead), 0)

  def test_WaitForToolReady(self):
    toolhead = 0
    temp = 100
    timeout = 60
    tolerance = 3
    delay = 100
    self.s3g.set_toolhead_temperature(toolhead, temp)
    self.s3g.wait_for_tool_ready(toolhead, delay, timeout)
    startTime = time.time()
    self.s3g.set_toolhead_temperature(toolhead, 0)
    while startTime + timeout > time.time() and abs(self.s3g.get_toolhead_temperature(toolhead) - temp) > tolerance:
      self.assertEqual(self.s3g.get_toolhead_target_temperature(toolhead), temp)
    time.sleep(5) #Give the bot a couple seconds to catch up
    self.assertEqual(self.s3g.get_toolhead_target_temperature(toolhead), 0)

  def test_QueueExtendedPointNew(self):
    firstPoint = [5, 6, 7, 8, 9]
    self.s3g.set_extended_position(firstPoint)
    newPoint = [1, 2, 3, 4, 5]
    mSConst = 1000
    duration = 5
    self.s3g.queue_extended_point_new(newPoint, duration*mSConst, False, False, False, False, False)
    time.sleep(duration)
    self.assertEqual(newPoint, self.s3g.get_extended_position()[0])
    anotherPoint = [5, 6, 7, 8, 9]
    self.s3g.queue_extended_point_new(anotherPoint, duration, True, True, True, True, True)
    time.sleep(duration)
    finalPoint = []
    for i, j in zip(newPoint, anotherPoint):
      finalPoint.append(i+j)
    self.assertEqual(finalPoint, self.s3g.get_extended_position()[0])
 
  def test_StoreHomePositions(self):
    pointToSet = [1, 2, 3, 4, 5]
    self.s3g.queue_extended_point(pointToSet, 500)
    self.s3g.store_home_positions(['x', 'y', 'z', 'a', 'b'])
    x = self.s3g.read_from_EEPROM(0x000E, 4)
    y = self.s3g.read_from_EEPROM(0x0012, 4)
    z = self.s3g.read_from_EEPROM(0x0016, 4)
    a = self.s3g.read_from_EEPROM(0x001A, 4)
    b = self.s3g.read_from_EEPROM(0x001E, 4)
    readHome = []
    for cor in [x, y, z, a, b]:
      readHome.append(s3g.decode_int32(cor))
    self.assertEqual(readHome, pointToSet)

  def test_RecallHomePositions(self):
    pointToSet = [1, 2, 3, 4, 5]
    self.s3g.queue_extended_point(pointToSet, 500)
    self.s3g.store_home_positions(['x', 'y', 'z', 'a', 'b'])
    newPoint = [50, 51, 52, 53, 54]
    self.s3g.queue_extended_point(newPoint, 500)
    time.sleep(5)
    self.s3g.recall_home_positions(['x', 'y', 'z', 'a', 'b'])
    self.assertEqual(newPoint, self.s3g.get_extended_position()[0])
    self.s3g.recall_home_positions(['x', 'y', 'z', 'a', 'b'])
    time.sleep(5)
    self.assertEqual(pointToSet, self.s3g.get_extended_position()[0])
 
  def test_GetMotherboardStatus(self):
    returnDic = self.s3g.get_motherboard_status()
    self.assertFalse(returnDic["POWER_ERROR"])
    self.assertFalse(returnDic["HEAT_SHUTDOWN"])
    
    #turn on extruder heater and wait 20 minutes
    self.s3g.set_toolhead_temperature(0, 225)
    time.sleep(20*60)
    returnDic = self.s3g.get_motherboard_status()
    self.assertTrue(returnDic["HEAT_SHUTDOWN"])
    self.assertFalse(returnDic["POWER_ERROR"])

  def test_GetToolStatus(self):
    toolhead = 0
    returnDic = self.s3g.get_tool_status(toolhead)
    print returnDic
    self.assertFalse(returnDic["ExtruderReady"])
    self.assertFalse(returnDic["ExtruderNotPluggedIn"])
    self.assertFalse(returnDic["ExtruderOverMaxTemp"])
    self.assertFalse(returnDic["ExtruderNotHeating" ])
    self.assertFalse(returnDic["ExtruderDroppingTemp"])
    self.assertFalse(returnDic["PlatformError"])
    self.assertFalse(returnDic["ExtruderError"])
    self.s3g.set_toolhead_temperature(toolhead, 100)
    returnDic = self.s3g.get_tool_status(toolhead)
    self.assertEqual(returnDic["ExtruderReady"], self.s3g.is_tool_ready(toolhead))
    raw_input("\nPlease unplug the platform!!  Press enter to continue.")
    self.s3g.file = serial.Serial(options.serialPort, '115200', timeout=1)
    self.s3g.set_platform_temperature(toolhead, 100)
    time.sleep(5)
    returnDic = self.s3g.get_tool_status(toolhead)
    self.assertTrue(returnDic["PlatformError"])
    raw_input("\nPlease turn the bot off, plug in the platform and unplug extruder 0's thermocouple!! Press enter to continue.")
    self.s3g.file = serial.Serial(options.serialPort, '115200', timeout=1)
    self.s3g.set_toolhead_temperature(toolhead, 100)
    time.sleep(5)
    returnDic = self.s3g.get_tool_status(toolhead)
    self.assertTrue(returnDic["ExtruderError"])
    self.assertTrue(returnDic["ExtruderNotPluggedIn"])
    raw_input("\nPlease turn the bot off and plug in the platform and Extruder 0's thermocouple!! Press enter to continue.")
 
  def test_GetPIDState(self):
    toolhead = 0
    pidDict = self.s3g.get_PID_state(toolhead)
    for key in pidDict:
      self.assertNotEqual(pidDict[key], None)

  def test_ToolheadInit(self):
    toolhead = 0
    self.s3g.set_toolhead_temperature(toolhead, 100)
    self.s3g.toolhead_init(toolhead)
    self.assertEqual(self.s3g.get_toolhead_target_temperature(toolhead), 0)

  def test_WaitForButton(self):
    self.s3g.wait_for_button('up', 0, False, False, False)
    obs = raw_input("\nIs the center button flashing? (y/n) ")
    self.assertEqual(obs, 'y')
    obs = raw_input("\nPress all the buttons EXCEPT the 'up' button.  Is the center button still flashing? (y/n) ")
    self.assertEqual(obs, 'y')
    obs = raw_input("\nPress the 'up' button.  Did the center button stop flashing? (y/n) ")
    self.assertEqual(obs, 'y')
    raw_input("\nTesting wait_for_button timeout.  Please watch the interface board and note the time! Press enter to continue")
    self.s3g.wait_for_button('up', 5, True, False, False)
    obs = raw_input("\nDid the center button flash for about 5 seconds and stop? (y/n) ")
    self.assertEqualG(obs, 'y')
    raw_inpit("\nTesting bot reset after tiemout.  Please watch/listen to verify if the replicator is resetting. Press enter to continue.")
    self.s3g.wait_for_button('up', 1, False, True, False)
    time.sleep(1)
    obs = raw_input("\nDid the bot just reset? (y/n) ")
    self.assertEqual(obs, 'y')
    self.s3g.wait_for_button('up', 0, False, False, True)
    obs = raw_input("\nPlease press the up button and note if the LCD screen resest or not.  Did the screen reset? (y/n) ")
    self.assertEqual(obs, 'y')

  def test_SetRGBLED(self):
    self.s3g.set_RGB_LED(0, 255, 0, 0)
    obs = raw_input("\nAre the LEDs in the bot green? (y/n) ")
    self.assertEqual(obs, 'y')
    self.s3g.set_RGB_LED(0, 255, 0, 128)
    obs = raw_input("\nAre the LEDs blinking? (y/n) ")
    self.assertEqual('y', obs)
 
  def test_SetBeep(self):
    raw_input("\nAbout to start playing some music.  Start listening! Press enter to continue")
    self.s3g.set_beep(261.626, 5)
    obs = raw_input("\nDid you hear a C note? (y/n) ")
    self.assertEqual('y', obs)

  def test_SetBuildPercent(self):
    percent = 42
    self.s3g.build_start_notification(1, "percentTest")
    self.s3g.set_build_percent(percent)
    obs = raw_input("\nLook at the interface board for your bot.  Does the build percent say that it is %i percent of the way done? (y/n) "%(percent))
    self.assertEqual('y', obs)

  def test_QueueSong(self):
    raw_input("\nGetting ready to play a song.  Make sure you are listening!  Press enter to continue.")
    self.s3g.queue_song(0)
    obs = raw_input("\nDid you hear the song play? (y/n) ")
    self.assertEqual(obs, 'y')

class s3gSDCardTests(unittest.TestCase):

  def setUp(self):
    self.s3g = s3g.s3g()
    self.s3g.file = serial.Serial(options.serialPort,'115200', timeout=1)
    self.s3g.abort_immediately()

  def tearDown(self):
    self.s3g.file.close()

  def test_GetBuildName(self):
    """
    Copy the contents of the testFiles directory onto an sd card to do this test
    """
    buildName = raw_input("\nPlease load the test SD card into the machine, select one of the files and begin to print it.  Then type the name _exactly_ as it appears on the bot's screen. ")
    name = self.s3g.get_build_name()
    self.assertEqual(buildName, ConvertFromNUL(name))

  def test_GetNextFilename(self):
    """
    Copy the contents of the testFiles directory onto an sd card to do this test
    """
    raw_input("\nPlease make sure the only files on the SD card plugged into the bot are the files inside the testFiles directory!! Press enter to continue")
    filename = 'box_1.s3g'
    volumeName = raw_input("\nPlease type the VOLUME NAME of the replicator's SD card exactly! Press enter to continue")
    readVolumeName = self.s3g.get_next_filename(True)
    self.assertEqual(volumeName, ConvertFromNUL(readVolumeName))
    readFilename = self.s3g.get_next_filename(False)
    self.assertEqual(filename, ConvertFromNUL(readFilename))
  
  def test_PlaybackCapture(self):
    filename = 'box_1.s3g'
    self.s3g.playback_capture(filename)
    readName = self.s3g.get_build_name()
    self.assertEqual(filename, ConvertFromNUL(readName))

class test(unittest.TestCase):

  def setUp(self):
    self.s3g = s3g.s3g()
    self.s3g.file = serial.Serial(options.serialPort, '115200', timeout=1)
    self.s3g.set_extended_position([0, 0, 0, 0, 0])
    self.s3g.abort_immediately()

  def tearDown(self):
    self.s3g.file.close()

  def test_QueueExtendedPointNew(self):
    home = [0, 0, 0, 0, 0]
    self.s3g.set_extended_position(home)
    newPoint = [1, 2, 3, 4, 5]
    mSConst = 1000
    duration = 5
    self.s3g.queue_extended_point_new(newPoint, duration*mSConst, [])
    time.sleep(duration)
    self.assertEqual(newPoint, self.s3g.get_extended_position()[0])
    anotherPoint = [5, 6, 7, 8, 9]
    self.s3g.queue_extended_point_new(anotherPoint, duration, ['x', 'y', 'z', 'a', 'b'])
    time.sleep(duration)
    finalPoint = []
    for i, j in zip(newPoint, anotherPoint):
      finalPoint.append(i+j)
    self.assertEqual(finalPoint, self.s3g.get_extended_position()[0])
 


if __name__ == '__main__':
  parser = optparse.OptionParser()
  parser.add_option("-e", "--extensive", dest="extensive", default="True")
  parser.add_option("-m", "--mightyboard", dest="isMightyBoard", default="True")
  parser.add_option("-i", "--interface", dest="hasInterface", default="True")
  parser.add_option("-p", "--port", dest="serialPort", default="/dev/tty.usbmodemfa131")
  (options, args) = parser.parse_args()
  if options.extensive.lower() == "false":
    print "Forgoing Heater Tests"
    extensive= False
  if options.hasInterface.lower() == "false":
    print "Forgoing Tests requiring Interface Boards"
    hasInterface = False


  del sys.argv[1:]
  print "*****To do many of these tests, your printer must be reset immediately prior to execution.  If you haven't, please reset your robot and run these tests again!!!*****"
  print "*****Because We are going to be moving the axes around, you should probably move the gantry to the middle of the build area and the Z platform to about halfway!!!*****"

  tests = unittest.TestSuite()
  tests.addTest(s3gFunctionTests('test_GetMotherboardStatus'))

  unittest.TextTestRunner(verbosity=2).run(tests)
  
  """
  commonTests = unittest.TestLoader().loadTestsFromTestCase(commonFunctionTests)
  packetTests = unittest.TestLoader().loadTestsFromTestCase(s3gPacketTests)
  sendReceiveTests = unittest.TestLoader().loadTestsFromTestCase(s3gSendReceiveTests)
  functionTests = unittest.TestLoader().loadTestsFromTestCase(s3gFunctionTests)
  sdTests = unittest.TestLoader().loadTestsFromTestCase(s3gSDCardTests)
  smallTest = unittest.TestLoader().loadTestsFromTestCase(test)
  suites = [commonTests, packetTests, sendReceiveTests, functionTests, sdTests, ]
  for suite in suites[-1]:
    unittest.TextTestRunner(verbosity=2).run(suite)
  """
