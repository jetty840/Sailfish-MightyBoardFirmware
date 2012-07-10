import unittest
import optparse
import serial
import io
import struct
import array
import time
import os, sys 
lib_path = os.path.abspath('./s3g')
sys.path.append(lib_path)
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
    self.r = s3g.s3g()
    self.r.writer=s3g.Writer.StreamWriter(serial.Serial(options.serialPort, '115200', timeout=1))
    self.r.abort_immediately()

  def tearDown(self):
    self.r = None

  def GetVersionPayload(self):
    payload = bytearray()
    payload.append(s3g.host_query_command_dict['GET_VERSION'])
    payload.extend(s3g.Encoder.encode_uint16(s3g.s3g_version))
    return payload

  def GetVersionPacket(self):
    """
    Helper method to generate a Get Version packet to be modified and sent
    """
    return s3g.Encoder.encode_payload(self.GetVersionPayload())

  def test_GetVersionPayload(self):
    payload = self.GetVersionPayload()
    self.assertEqual(payload[0], s3g.host_query_command_dict['GET_VERSION'])
    self.assertEqual(payload[1:], s3g.Encoder.encode_uint16(s3g.s3g_version))

  def test_GetVersionPacket(self):
    testPayload = self.GetVersionPayload()
    packet = self.GetVersionPacket()
    self.assertEqual(packet[0], s3g.header)
    self.assertEqual(packet[1], len(packet[2:-1]))
    self.assertEqual(packet[2:-1], testPayload)
    self.assertEqual(packet[-1], s3g.Encoder.CalculateCRC(testPayload))

  def test_NoHeader(self):
    packet = self.GetVersionPacket()
    packet[0] = '\x00'
    self.assertRaises(s3g.TransmissionError, self.r.writer.send_packet, packet)
      
  def test_EmptyPacket(self):
    packet = bytearray()
    self.assertRaises(s3g.TransmissionError, self.r.writer.send_packet, packet)

  def test_TrailingPacket(self):
    packet = self.GetVersionPacket()
    addition = bytearray('\xff\xff')
    packet.extend(addition)
    self.r.writer.send_packet(packet)
    self.assertTrue(True)

  def test_PreceedingPacket(self):
    packet = self.GetVersionPacket()
    addition = bytearray('\xa4\x5f')
    addition.extend(packet)
    self.r.writer.send_packet(addition)
    self.assertTrue(True)

  def test_BadCRC(self):
    packet = self.GetVersionPacket()
    payload = packet[2:-1]
    crc = s3g.Encoder.CalculateCRC(payload)
    packet[-1] = crc+1 
    self.assertRaises(s3g.TransmissionError, self.r.writer.send_packet, packet)

  def test_LongLength(self):
    packet = self.GetVersionPacket()
    packet[1] = '\x0f'
    self.assertRaises(s3g.TransmissionError, self.r.writer.send_packet, packet)

  def test_ShortLength(self):
    packet = self.GetVersionPacket()
    packet[1] = '\x00'
    self.assertRaises(s3g.ProtocolError, self.r.writer.send_packet, packet)

  def test_LongPayload(self):
    packet = self.GetVersionPacket()
    packet.insert(2, '\x00')
    self.assertRaises(s3g.TransmissionError, self.r.writer.send_packet, packet)

  def test_ShortPayload(self):
    packet = self.GetVersionPacket()
    packet = packet[0:2] + packet[3:]
    self.assertRaises(s3g.TransmissionError, self.r.writer.send_packet, packet)

  def test_MaxLength(self):
    payload = self.GetVersionPayload()
    for i in range(s3g.maximum_payload_length - len(payload)):
      payload.append(0x00)
    self.r.writer.send_command(payload)

  def test_OversizedLength(self):
    payload = bytearray(s3g.maximum_payload_length+1)
    self.assertRaises(s3g.PacketLengthError, s3g.Encoder.encode_payload, payload)

class s3gSendReceiveTests(unittest.TestCase):
  def setUp(self):
    self.r = s3g.s3g()
    self.r.writer=s3g.Writer.StreamWriter(serial.Serial(options.serialPort, '115200', timeout=1))
    self.r.abort_immediately()

  def tearDown(self):
    self.r = None

  def test_ToolheadPause(self):
    self.r.toolhead_pause(0)

  def test_ToolheadAbort(self):
    self.r.toolhead_abort(0)

  def test_ResetToFactoryReply(self):
    self.r.reset_to_factory()

  def test_QueueSongReply(self):
    self.r.queue_song(1)

  def test_SetBuildPercentReply(self):
    self.r.set_build_percent(100)

  def test_SetBeepReply(self):
    self.r.set_beep(1000, 3)

  def test_SetPotentiometerValueReply(self):
    self.r.set_potentiometer_value([], 0)

  def test_SetRGBLEDReply(self):
    self.r.set_RGB_LED(255, 0, 0, 0)

  def test_WaitForButtonReply(self):
    self.r.wait_for_button('up', 0, True, False, False)

  def test_ToolheadInitReply(self):
    self.r.toolhead_init(0)

  def test_GetPIDStateReply(self):
    self.r.get_PID_state(0)

  def testGetToolStatusReply(self):
    self.r.get_tool_status(0)

  def test_GetMotor1SpeedReply(self):
    self.r.get_motor1_speed(0)

  def test_StoreHomePositionsReply(self):
    self.r.store_home_positions(['X', 'Y', 'Z', 'A', 'B'])

  def test_RecallHomePositionsReply(self):
    self.r.recall_home_positions(['X', 'Y', 'Z', 'A', 'B'])

  def test_QueueExtendedPointNewReply(self):
    self.r.queue_extended_point_new([0, 0, 0, 0, 0], 1, ['X', 'Y', 'Z', 'A', 'B'])

  def test_ToggleAxesReply(self):
    self.r.toggle_axes(['X', 'Y', 'Z', 'A', 'B'], True)

  def test_WaitForPlatformReply(self):
    self.r.wait_for_platform_ready(0, 100, 50)

  def test_WaitForToolReadyReply(self):
    self.r.wait_for_tool_ready(0, 100, 50)

  def test_DelayReply(self):
    self.r.delay(10)

  def test_GetCommunicationStatsReply(self):
    self.r.get_communication_stats()

  def test_GetMotherboardStatusReply(self):
    self.r.get_motherboard_status()

  def test_ExtendedStopReply(self):
    self.r.extended_stop(True, True)

  def test_CaptureToFileReply(self):
    self.r.capture_to_file('test')

  def test_EndCaptureToFileReply(self):
    self.r.end_capture_to_file()

  def test_ResetReply(self):
    self.r.reset()

  def test_IsFinishedReply(self):
    self.r.is_finished()

  def test_PauseReply(self):
    self.r.pause()

  def test_ClearBufferReply(self):
    self.r.clear_buffer()

  def test_InitReply(self):
    self.r.init()

  def test_ToggleExtraOutputReply(self):
    self.r.toggle_extra_output(0, False)

  def test_ToggleFanReply(self):
    self.r.toggle_fan(0, False)

  def test_IsPlatformReadyReply(self):
    self.r.is_platform_ready(0)

  def test_GetPlatformTargetTemperatureReply(self):
    self.r.get_platform_target_temperature(0)

  def test_GetToolheadTargetTemperatureReply(self):
    self.r.get_toolhead_target_temperature(0)

  def test_ReadFromToolheadEEPROMReply(self):
    self.r.read_from_toolhead_EEPROM(0, 0x00, 0)

  def test_IsToolReadyReply(self):
    self.r.is_tool_ready(0)

  def test_GetToolheadTemperatureReply(self):
    self.r.get_toolhead_temperature(0)

  def test_GetPlatformTemperatureReply(self):
    self.r.get_platform_temperature(0)

  def test_GetToollheadVersionReply(self):
    self.r.get_toolhead_version(0)

  def test_BuildEndNotificationReply(self):
    self.r.build_end_notification()

  def test_BuildStartNotificationReply(self):
    self.r.build_start_notification('aTest')

  def test_DisplayMessageReply(self):
    if hasInterface:
      self.r.display_message(0, 0, "TESTING", 1, False, False, False)

  def test_FindAxesMaximumsReply(self):
    self.r.find_axes_maximums(['x', 'y', 'z'], 1, 0)

  def test_FindAxesMinimumsReply(self):
    self.r.find_axes_minimums(['x', 'y', 'z'], 1, 0)

  def test_GetBuildNameReply(self):
    self.r.get_build_name()

  def test_GetNextFilenameReply(self):
    self.r.get_next_filename(False)

  def test_PlaybackCaptureReply(self):
    self.assertRaises(s3g.SDCardError, self.r.playback_capture, 'aTest')

  def test_AbortImmediatelyReply(self):
    self.r.abort_immediately()

  def test_GetAvailableBufferSizeReply(self):
    self.r.get_available_buffer_size()

  def test_ReadFromEEPROMReply(self):
    self.r.read_from_EEPROM(0x00, 0)

  def test_GetVersionReply(self):
    self.r.get_version()

  def test_SetPlatformTemperatureReply(self):
    temperature = 100
    toolhead = 0
    self.r.set_platform_temperature(toolhead, temperature)
    self.r.set_platform_temperature(toolhead, 0)

  def test_SetToolheadTemperatureReply(self):
    temperature = 100
    toolhead = 0
    self.r.set_toolhead_temperature(toolhead, temperature)
    self.r.set_toolhead_temperature(toolhead, 0)

  def test_GetExtendedPositionReply(self):
    self.r.get_extended_position()

  def test_QueueExtendedPointReply(self):
    position = [0, 0, 0, 0, 0]
    rate = 500
    self.r.queue_extended_point(position, rate)

  def test_SetExtendedPositionReply(self):
    position = [0, 0, 0, 0, 0]
    self.r.set_extended_position(position)

class s3gFunctionTests(unittest.TestCase):

  def setUp(self):
    self.r = s3g.s3g()
    self.r.writer=s3g.Writer.StreamWriter(serial.Serial(options.serialPort, '115200', timeout=1))
    self.r.set_extended_position([0, 0, 0, 0, 0])
    self.r.abort_immediately()

  def tearDown(self):
    self.r = None

  def test_ToggleFan(self):
    toolhead = 0
    self.r.toggle_fan(toolhead, True)
    obs = raw_input("\nIs toolhead %i's fan on? (y/n) "%toolhead)
    self.assertEqual(obs, 'y')

  def test_GetVersion(self):
    expectedVersion = raw_input("\nWhat is the version number of your bot? ")
    expectedVersion = int(expectedVersion.replace('.', '0'))
    self.assertEqual(expectedVersion, self.r.get_version())

  def test_GetToolheadVersion(self):
    expectedVersion = raw_input("\nWhat is the version number of toolhead 0 on your bot? ")
    expectedVersion = int(expectedVersion.replace('.', '0'))
    self.assertEqual(expectedVersion, self.r.get_toolhead_version(0))

  def test_GetPlatformTemperature(self):
    if extensive:
      obsvTemperature = raw_input("\nWhat is the current platform temperature? ")
      self.assertEqual(str(self.r.get_platform_temperature(0)), str(obsvTemperature))

  def test_GetToolheadTemperature(self):
    if extensive:
      obsvTemperature = raw_input("\nWhat is the right extruder's current temperature? ")
      self.assertEqual(str(self.r.get_toolhead_temperature(0)), str(obsvTemperature))

  def test_SetPlatformTargetTemperature(self):
    if extensive:
      tolerance = 2
      target = 50
      self.r.set_platform_temperature(0, target)
      minutes = 3
      print "\nWaiting %i mintues to heat the Platform up"%(minutes)
      time.sleep(60*minutes)
      self.assertTrue(abs(self.r.get_platform_temperature(0)-target) <= tolerance)
      self.r.set_platform_temperature(0, 0)

  def test_SetToolheadTemperature(self):
    if extensive:
      tolerance = 2
      target = 50
      self.r.set_toolhead_temperature(0, target)
      minutes = 3
      print "\nWaiting %i minutes to heat the Toolhead up"%(minutes)
      time.sleep(60*minutes)
      self.assertTrue(abs(self.r.get_toolhead_temperature(0) - target) <= tolerance)
      self.r.set_toolhead_temperature(0, 0)

  def test_GetToolheadTargetTemperature(self):
    target = 100
    toolhead = 0
    self.r.set_toolhead_temperature(toolhead, target)
    self.assertEqual(self.r.get_toolhead_target_temperature(toolhead), target)
    self.r.set_toolhead_temperature(toolhead, 0)

  def test_GetPlatformTargetTemperature(self):
    target = 100
    self.r.set_platform_temperature(0, target)
    self.assertEqual(self.r.get_platform_target_temperature(0), target)
    self.r.set_platform_temperature(0, 0)

  def test_ReadFromEEPROMMighty(self):
    """
    Read the VID/PID settings from the MB and compare against s3g's read from eeprom
    """
    vidPID = self.r.read_from_EEPROM(0x0044, 2)
    vidPID = array.array('B', vidPID)
    vidPID = struct.unpack('<H', vidPID)[0]
    mightyVIDPID = [0x23C1, 0xB404]
    self.assertEqual(vidPID, mightyVIDPID[0])

  def test_WriteToEEPROMMighty(self):
    nameOffset = 0x0022
    nameSize = 16
    name = 'ILOVETESTINGALOT'
    self.r.write_to_EEPROM(nameOffset, name)
    readName = self.r.read_from_EEPROM(nameOffset, 16)
    self.assertEqual(name, readName)

  def test_ReadFromToolEEPROMMighty(self):
    """
    Read the backoff forward time from the mighty board tool eeprom
    """
    t0Database = 0x0100
    bftOffset = 0x0006
    readBFT = self.r.read_from_toolhead_EEPROM(0, bftOffset, 2)
    readBFT = array.array('B', readBFT)
    readBFT = struct.unpack('<H', readBFT)[0]
    mightyBFT = 500
    self.assertEqual(mightyBFT, readBFT)

  def test_IsPlatformReady(self):
    """
    Determine if the platform is ready by setting the temperature to its current reading and asking if its ready (should return true, then setting the temperature to double what it is now then querying it agian, expecting a false answer
    """
    curTemp = self.r.get_platform_temperature(0)
    self.r.set_platform_temperature(0, curTemp)
    self.assertTrue(self.r.is_platform_ready(0))
    self.r.set_platform_temperature(0, curTemp+50)
    self.assertEqual(self.r.is_platform_ready(0), False)
    self.r.set_platform_temperature(0, 0)

  def test_IsToolReady(self):
    toolhead = 0
    curTemp = self.r.get_toolhead_temperature(toolhead)
    self.r.set_toolhead_temperature(toolhead, curTemp)
    self.assertTrue(self.r.is_tool_ready(toolhead))
    self.r.set_toolhead_temperature(toolhead, curTemp + 50)
    self.assertEqual(self.r.is_tool_ready(toolhead), False)
    self.r.set_toolhead_temperature(toolhead, 0)

  def test_DisplayMessage(self):
    if hasInterface:
      message = str(time.clock())
      self.r.display_message(0, 0, message, 10, False, False, False)
      readMessage = raw_input("\nWhat is the message on the replicator's display? ")
      self.assertEqual(message, readMessage)

  def test_GetExtendedPosition(self):
    position = self.r.get_extended_position()
    self.assertEqual(position[0], [0, 0, 0, 0, 0])

  def test_SetExtendedPosition(self):
    position = [50, 51, 52, 53, 54]
    self.r.set_extended_position(position)
    self.assertEqual(position, self.r.get_extended_position()[0])

  def test_QueueExtendedPosition(self):
    newPosition = [51, 52, 53, 54, 55]
    rate = 500
    self.r.queue_extended_point(newPosition, rate)
    time.sleep(5)
    self.assertEqual(newPosition, self.r.get_extended_position()[0])

  def test_FindAxesMaximums(self):
    axes = ['x', 'y', 'z']
    rate = 500
    timeout = 10
    xYEndstops = 10
    self.r.find_axes_maximums(axes, rate, timeout)
    time.sleep(timeout)
    self.assertEqual(self.r.get_extended_position()[1], xYEndstops)
    obs = raw_input("\nDid the Z Platform move towards the bottom of the machine? (y/n) ")
    self.assertEqual('y', obs)


  def test_FindAxesMinimums(self):
    axes = ['x', 'y', 'z']
    rate = 500
    timeout = 5
    self.r.find_axes_minimums(axes, rate, timeout)
    time.sleep(timeout)
    xyObs = raw_input("\nDid the gantry move from the back right to the front left of the machine? (y/n) ")
    self.assertEqual('y', xyObs)
    zObs = raw_input("\nDid the Z Platform move towards the top of the machine? (y/n) ")
    self.assertEqual('y', zObs)

  def test_Init(self):
    bufferSize = 512
    expectedPosition = [0, 0, 0, 0, 0]
    position = [10, 9, 8, 7, 6]
    self.r.set_extended_position(position)
    #Find the maximum so that if we fail, it wont try to move outside its bounds
    self.r.delay(5)
    self.r.init()
    self.assertEqual(expectedPosition, self.r.get_extended_position()[0])
    self.assertEqual(self.get_available_buffer_size(), bufferSize)

  def test_GetAvailableBufferSize(self):
    bufferSize = 512
    self.assertEqual(bufferSize, self.r.get_available_buffer_size())

  def test_AbortImmediately(self):
    bufferSize = 512
    toolheads = [0, 1]
    for toolhead in toolheads:
      self.r.set_toolhead_temperature(toolhead, 100)
    self.r.set_platform_temperature(0, 100)
    for i in range(5):
      self.r.find_axes_minimums(['x', 'y', 'z'], 500, 5)
    self.r.abort_immediately()
    self.assertEqual(bufferSize, self.r.get_available_buffer_size())
    for toolhead in toolheads:
      self.assertEqual(0, self.r.get_toolhead_target_temperature(toolhead))
    self.assertEqual(0, self.r.get_platform_target_temperature(0))
    self.assertTrue(self.r.is_finished())

  def test_BuildStartNotification(self):
    buildName = "test"
    self.r.build_start_notification(buildName)
    readBuildName = self.r.get_build_name()
    readBuildName = ConvertFromNUL(readBuildName)
    self.assertEqual(buildName, readBuildName)

  def test_BuildEndNotification(self):
    noBuild = bytearray('\x00')
    self.r.build_start_notification("test")
    self.r.build_end_notification()
    self.assertEqual(self.r.get_build_name(), noBuild)

  def test_ClearBuffer(self):
    bufferSize = 512
    axes = ['x', 'y', 'z']
    rate = 500
    timeout = 5
    for i in range(10):
      self.r.find_axes_minimums(axes, rate, timeout)
    self.assertNotEqual(bufferSize, self.r.get_available_buffer_size())
    self.r.clear_buffer()
    self.assertEqual(bufferSize, self.r.get_available_buffer_size())

  def test_Pause(self):
    """
    Because we cant query the bot to determine if its paused, we using the find_axes_maximums function to help.  We know how long it will take to traverse the build space.  If we start from the front, begin homing to the back then pause for the traversal time, we will know if we paused if we have not reached the end.  If we then unpause and wait the traversal time, we should reach the end.
    """
    yEndStop = 8
    zEndStop = 16
    axes = ['y']
    traverseTime = 5 #Time it takes for the gantry to get from the back to the front
    self.r.find_axes_maximums(axes, 500, traverseTime) #At the back
    time.sleep(traverseTime)
    self.r.find_axes_minimums(axes, 500, traverseTime) #At the front
    time.sleep(traverseTime)
    self.assertTrue(self.r.get_extended_position()[1] < yEndStop or self.r.get_extended_position()[1] == zEndStop) #Make sure we are in the right location
    self.r.find_axes_maximums(axes, 500, traverseTime*3) #Start to go to the back, give extra long timeout so we dont time out
    self.r.pause()
    time.sleep(traverseTime) #Wait for the machine to catch up to do the check
    self.assertTrue(self.r.get_extended_position()[1] < yEndStop or self.r.get_extended_position()[1] == zEndStop) #Make sure we are still in the same location
    self.r.pause() #Unpause
    time.sleep(traverseTime*2) #Wait for the bot to get to the end
    self.assertTrue(self.r.get_extended_position()[1] == yEndStop or self.r.get_extended_position()[1] == yEndStop + zEndStop) #Make sure we can unpause
    

  def test_IsFinished(self):
    axes = ['y']
    timeout = 3
    self.r.find_axes_maximums(axes, 500, timeout)#We dont want to move beyond our bounds
    time.sleep(timeout)
    self.r.find_axes_minimums(axes, 500, timeout)
    self.assertFalse(self.r.is_finished())
    time.sleep(timeout)
    self.assertTrue(self.r.is_finished())

  def test_Reset(self):
    bufferSize = 512
    for i in range(10):
      self.r.find_axes_minimums(['x', 'y', 'z'], 500, 10)
    self.r.set_toolhead_temperature(0, 100)
    self.r.set_platform_temperature(0, 100)
    self.r.reset()
    self.assertEqual(self.r.get_available_buffer_size(), bufferSize)
    self.assertTrue(self.r.is_finished())
    self.assertEqual(self.r.get_toolhead_target_temperature(0), 0)
    self.assertEqual(self.r.get_platform_target_temperature(0), 0)
 
  def test_ClearBuffer(self):
    bufferSize = 512
    axes = ['x', 'y', 'z']
    rate = 500
    timeout = 5
    for i in range(5):
      self.r.find_axes_minimums(axes, rate, timeout)
    self.assertNotEqual(bufferSize, self.r.get_available_buffer_size())
    self.r.clear_buffer()
    self.assertEqual(bufferSize, self.r.get_available_buffer_size())

  def test_CaptureToFile(self):
    filename = str(time.clock())+".s3g" #We want to keep changing the filename so this test stays nice and fresh
    self.r.capture_to_file(filename)
    #Get the filenames off the SD card
    files = []
    curFile = ConvertFromNUL(self.r.get_next_filename(True))
    while curFile != '':
      curFile = ConvertFromNUL(self.r.get_next_filename(False))
      files.append(curFile)
    self.assertTrue(filename in files)

  def test_GetCommunicationStats(self):
    changableInfo = ['PacketsReceived', 'PacketsSent']
    oldInfo = self.r.get_communication_stats()
    toSend = 5
    for i in range(toSend):
      self.r.is_finished()
    newInfo = self.r.get_communication_stats()
    for key in changableInfo:
      self.assertTrue(newInfo[key]-oldInfo[key] == toSend)

  def test_EndCaptureToFile(self):
    filename = str(time.clock())+".s3g"
    self.r.capture_to_file(filename)
    findAxesMaximums = 8+32+16
    numCmd = 5
    totalBytes = findAxesMaximums*numCmd/8 + numCmd
    #Add some commands to the file
    for i in range(numCmd):
      self.r.find_axes_maximums(['x', 'y'], 500, 10)
    self.assertEqual(totalBytes, self.r.end_capture_to_file())

  def test_ExtendedStop(self):
    bufferSize = 512
    self.r.find_axes_maximums(['x', 'y'], 200, 5)
    time.sleep(5)
    for i in range(5):
      self.r.find_axes_minimums(['x', 'y'], 1600, 2)
    self.r.extended_stop(True, True)
    time.sleep(5) #Give the machine time to response
    self.assertTrue(self.r.is_finished())
    self.assertEqual(bufferSize, self.r.get_available_buffer_size())

  @unittest.skip("delay is broken, delaysin mili instead of micro.  This woul dmake us delay for a long time, so we skip this step for now")
  def test_Delay(self):
    axes = ['x', 'y']
    feedrate = 500
    timeout = 5
    uSConst = 1000000
    zEndStop = 16
    xyEndStops = 10
    allEndStops = 26
    self.r.find_axes_maximums(axes, feedrate, timeout)
    time.sleep(timeout)
    testStart = time.time()
    self.r.find_axes_minimums(axes, feedrate, timeout)
    self.r.delay(timeout*uSConst)
    self.r.find_axes_maximums(axes, feedrate, timeout)
    while testStart + timeout*3  > time.time(): #XY endstops should be low while moving/delaying.  If not, delay didnt delay for the correct time
      self.assertTrue(self.r.get_extended_position()[1] == 0 or self.r.get_extended_position()[1] == zEndStop)
    time.sleep(.5) #Wait about half a second for the machine to finish its movements
    self.assertTrue(self.r.get_extended_position()[1] == allEndStops or self.r.get_extended_position()[1] == xyEndStops)

  def test_ToggleAxes(self):
    self.r.toggle_axes(True, True, True, True, True, True)
    obs = raw_input("\nPlease try to move all (x,y,z) the axes!  Can you move them without using too much force? (y/n) ")
    self.assertEqual('n', obs)
    self.r.toggle_axes(True, True, True, True, True, False)
    obs = raw_input("\nPlease try to move all (x,y,z) the axes!  Can you move them without using too much force? (y/n) ")
    self.assertEqual('y', obs)
 
  def test_ExtendedStop(self):
    bufferSize = 512
    self.r.find_axes_maximums(['x', 'y'], 200, 5)
    time.sleep(5)
    for i in range(5):
      self.r.find_axes_minimums(['x', 'y'], 1600, 2)
    self.r.extended_stop(True, True)
    time.sleep(5) #Give the machine time to response
    self.assertTrue(self.r.is_finished())
    self.assertEqual(bufferSize, self.r.get_available_buffer_size())

  def test_WaitForPlatformReady(self):
    toolhead = 0
    temp = 50
    timeout = 60
    tolerance = 3
    delay = 100
    self.r.set_platform_temperature(toolhead, temp)
    self.r.wait_for_platform_ready(toolhead, delay, timeout)
    startTime = time.time()
    self.r.set_platform_temperature(toolhead, 0)
    while startTime + timeout > time.time() and abs(self.r.get_platform_temperature(toolhead) - temp) > tolerance:
      self.assertEqual(self.r.get_platform_target_temperature(toolhead), temp)
    time.sleep(5) #Give the bot a couple seconds to catch up
    self.assertEqual(self.r.get_platform_target_temperature(toolhead), 0)

  def test_WaitForToolReady(self):
    toolhead = 0
    temp = 100
    timeout = 60
    tolerance = 3
    delay = 100
    self.r.set_toolhead_temperature(toolhead, temp)
    self.r.wait_for_tool_ready(toolhead, delay, timeout)
    startTime = time.time()
    self.r.set_toolhead_temperature(toolhead, 0)
    while startTime + timeout > time.time() and abs(self.r.get_toolhead_temperature(toolhead) - temp) > tolerance:
      self.assertEqual(self.r.get_toolhead_target_temperature(toolhead), temp)
    time.sleep(5) #Give the bot a couple seconds to catch up
    self.assertEqual(self.r.get_toolhead_target_temperature(toolhead), 0)

  def test_QueueExtendedPointNew(self):
    firstPoint = [5, 6, 7, 8, 9]
    self.r.set_extended_position(firstPoint)
    newPoint = [1, 2, 3, 4, 5]
    mSConst = 1000
    duration = 5
    self.r.queue_extended_point_new(newPoint, duration*mSConst, ['X', 'Y', 'Z', 'A', 'B'])
    time.sleep(duration)
    self.assertEqual(newPoint, self.r.get_extended_position()[0])
    anotherPoint = [5, 6, 7, 8, 9]
    self.r.queue_extended_point_new(anotherPoint, duration, ['X', 'Y', 'Z', 'A', 'B'])
    time.sleep(duration)
    finalPoint = []
    for i, j in zip(newPoint, anotherPoint):
      finalPoint.append(i+j)
    self.assertEqual(finalPoint, self.r.get_extended_position()[0])
 
  def test_StoreHomePositions(self):
    pointToSet = [1, 2, 3, 4, 5]
    self.r.queue_extended_point(pointToSet, 500)
    self.r.store_home_positions(['X', 'Y', 'Z', 'A', 'B'])
    x = self.r.read_from_EEPROM(0x000E, 4)
    y = self.r.read_from_EEPROM(0x0012, 4)
    z = self.r.read_from_EEPROM(0x0016, 4)
    a = self.r.read_from_EEPROM(0x001A, 4)
    b = self.r.read_from_EEPROM(0x001E, 4)
    readHome = []
    for cor in [x, y, z, a, b]:
      readHome.append(s3g.decode_int32(cor))
    self.assertEqual(readHome, pointToSet)

  def test_RecallHomePositions(self):
    pointToSet = [1, 2, 3, 4, 5]
    self.r.queue_extended_point(pointToSet, 500)
    self.r.store_home_positions(['X', 'Y', 'Z', 'A', 'B'])
    newPoint = [50, 51, 52, 53, 54]
    self.r.queue_extended_point(newPoint, 500)
    time.sleep(5)
    self.r.recall_home_positions([])
    self.assertEqual(newPoint, self.r.get_extended_position()[0])
    self.r.recall_home_positions(['X', 'Y', 'Z', 'A', 'B'])
    time.sleep(5)
    self.assertEqual(pointToSet, self.r.get_extended_position()[0])
 

  def test_GetToolStatus(self):
    toolhead = 0
    return_list = self.r.get_tool_status(toolhead)
    self.assertTrue(return_list[0])
    self.assertFalse(return_list[6])
    self.assertFalse(return_list[7])
    self.r.set_toolhead_temperature(toolhead, 100)
    returnDic = self.r.get_tool_status(toolhead)
    self.assertEqual(returnDic[0], self.r.is_tool_ready(toolhead))
    raw_input("\nPlease unplug the platform!!  Press enter to continue.")
    self.r.file = serial.Serial(options.serialPort, '115200', timeout=1)
    self.r.set_platform_temperature(toolhead, 100)
    time.sleep(5)
    returnDic = self.r.get_tool_status(toolhead)
    self.assertTrue(returnDic[6])
    raw_input("\nPlease turn the bot off, plug in the platform and unplug extruder 0's thermocouple!! Press enter to continue.")
    self.r.file = serial.Serial(options.serialPort, '115200', timeout=1)
    self.r.set_toolhead_temperature(toolhead, 100)
    time.sleep(5)
    returnDic = self.r.get_tool_status(toolhead)
    self.assertTrue(returnDic["EXTRUDER_ERROR"])
    raw_input("\nPlease turn the bot off and plug in the platform and Extruder 0's thermocouple!! Press enter to continue.")
 
  def test_GetPIDState(self):
    toolhead = 0
    pidDict = self.r.get_PID_state(toolhead)
    for key in pidDict:
      self.assertNotEqual(pidDict[key], None)

  def test_ToolheadInit(self):
    toolhead = 0
    self.r.set_toolhead_temperature(toolhead, 100)
    self.r.toolhead_init(toolhead)
    self.assertEqual(self.r.get_toolhead_target_temperature(toolhead), 0)
    self.r.toggle_fan(toolhead, true)
    time.sleep(5)
    self.r.toolhead_init(toolhead)
    obs = raw_input("\nDid Extruder " + str(toolhead) + "'s fan turn off? (y/s) ")
    self.assertEqual('y', obs)
    raw_input("\nStarting Motor %i.  Press enter to continue"%(toolhead))
    self.r.toggle_motor1(toolhead, True, True)
    self.r.toolhead_init(toolhead)
    obs = raw_input = ("\nDid Extruder " + str(toolhead) + "'s motor stop turning? (y/n) ")
    self.assertEqual(obs, 'y')

  @unittest.skip("Theres really no way to test this without toggle_motor1 working")
  def test_SetMotor1SpeedRPM(self):
    """
    FUTURE DAVE, DO THIS TEST!
    """
    pass

  def test_WaitForButton(self):
    self.r.wait_for_button('up', 0, False, False, False)
    obs = raw_input("\nIs the center button flashing? (y/n) ")
    self.assertEqual(obs, 'y')
    obs = raw_input("\nPress all the buttons EXCEPT the 'up' button.  Is the center button still flashing? (y/n) ")
    self.assertEqual(obs, 'y')
    obs = raw_input("\nPress the 'up' button.  Did the center button stop flashing? (y/n) ")
    self.assertEqual(obs, 'y')
    raw_input("\nTesting wait_for_button timeout.  Please watch the interface board and note the time! Press enter to continue")
    self.r.wait_for_button('up', 5, True, False, False)
    obs = raw_input("\nDid the center button flash for about 5 seconds and stop? (y/n) ")
    self.assertEqual(obs, 'y')
    raw_input("\nTesting bot reset after tiemout.  Please watch/listen to verify if the replicator is resetting. Press enter to continue.")
    self.r.wait_for_button('up', 1, False, True, False)
    time.sleep(1)
    obs = raw_input("\nDid the bot just reset? (y/n) ")
    self.assertEqual(obs, 'y')
    self.r.wait_for_button('up', 0, False, False, True)
    obs = raw_input("\nPlease press the up button and note if the LCD screen resest or not.  Did the screen reset? (y/n) ")
    self.assertEqual(obs, 'y')

  def test_SetRGBLED(self):
    self.r.set_RGB_LED(0, 255, 0, 0)
    obs = raw_input("\nAre the LEDs in the bot green? (y/n) ")
    self.assertEqual(obs, 'y')
    self.r.set_RGB_LED(0, 255, 0, 128)
    obs = raw_input("\nAre the LEDs blinking? (y/n) ")
    self.assertEqual('y', obs)
 
  def test_SetBeep(self):
    raw_input("\nAbout to start playing some music.  Start listening! Press enter to continue")
    self.r.set_beep(261.626, 5)
    obs = raw_input("\nDid you hear a C note? (y/n) ")
    self.assertEqual('y', obs)

  def test_SetBuildPercent(self):
    percent = 42
    self.r.build_start_notification("percentTest")
    self.r.set_build_percent(percent)
    obs = raw_input("\nLook at the interface board for your bot.  Does the build percent say that it is %1 percent of the way done? (y/n) " %(percent))
    self.assertEqual('y', obs)

  def test_QueueSong(self):
    raw_input("\nGetting ready to play a song.  Make sure you are listening!  Press enter to continue.")
    self.r.queue_song(1)
    obs = raw_input("\nDid you hear the song play? (y/n) ")
    self.assertEqual(obs, 'y')

class test(unittest.TestCase):
  def setUp(self):
    self.r = s3g.s3g()
    self.r.writer=s3g.Writer.StreamWriter(serial.Serial(options.serialPort, '115200', timeout=1))
    self.r.abort_immediately()

  def tearDown(self):
    self.r = None

  """
  def test_MaxLength(self):
    commandCount = 0
    maxSize = s3g.maximum_payload_length
    maxSize -= 1 #For the payload header
    maxSize -= len(s3g.encode_uint32(commandCount))
    maxSize -= 1 #For the null sign for the nullTerminated string
    buildName = ''
    for i in range(maxSize):
      buildName += 'a'
    self.r.build_start_notification(buildName)
  def test_MaxLength(self):
    b = bytearray(s3g.maximum_payload_length)
    self.r.send_command(b)"""



class s3gSDCardTests(unittest.TestCase):

  def setUp(self):
    self.r = s3g.s3g()
    self.r.writer = s3g.Writer.StreamWriter(serial.Serial(options.serialPort, '115200', timeout=1))
    self.r.abort_immediately()

  def tearDown(self):
    self.r = None

  def test_GetBuildName(self):
    """
    Copy the contents of the testFiles directory onto an sd card to do this test
    """
    buildName = raw_input("\nPlease load the test SD card into the machine, select one of the files and begin to print it.  Then type the name _exactly_ as it appears on the bot's screen. ")
    name = self.r.get_build_name()
    self.assertEqual(buildName, ConvertFromNUL(name))

  def test_GetNextFilename(self):
    """
    Copy the contents of the testFiles directory onto an sd card to do this test
    """
    raw_input("\nPlease make sure the only files on the SD card plugged into the bot are the files inside the testFiles directory!! Press enter to continue")
    filename = 'box_1.s3g'
    volumeName = raw_input("\nPlease type the VOLUME NAME of the replicator's SD card exactly! Press enter to continue")
    readVolumeName = self.r.get_next_filename(True)
    self.assertEqual(volumeName, ConvertFromNUL(readVolumeName))
    readFilename = self.r.get_next_filename(False)
    self.assertEqual(filename, ConvertFromNUL(readFilename))
  
  def test_PlaybackCapture(self):
    filename = 'box_1.s3g'
    self.r.playback_capture(filename)
    readName = self.r.get_build_name()
    self.assertEqual(filename, ConvertFromNUL(readName))
    

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
  commonTests = unittest.TestLoader().loadTestsFromTestCase(commonFunctionTests)
  packetTests = unittest.TestLoader().loadTestsFromTestCase(s3gPacketTests)
  sendReceiveTests = unittest.TestLoader().loadTestsFromTestCase(s3gSendReceiveTests)
  functionTests = unittest.TestLoader().loadTestsFromTestCase(s3gFunctionTests)
  sdTests = unittest.TestLoader().loadTestsFromTestCase(s3gSDCardTests)
  smallTest = unittest.TestLoader().loadTestsFromTestCase(test)
  suites = [commonTests, packetTests, sendReceiveTests, functionTests, sdTests, smallTest]
  for suite in suites:
    unittest.TextTestRunner(verbosity=2).run(suite)
