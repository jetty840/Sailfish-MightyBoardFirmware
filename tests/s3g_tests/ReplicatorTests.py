import unittest
import optparse
import serial
import io
import struct
import array
import time
import os, sys 
import logging

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
    time.sleep(5)

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
    packet[1] = '\x01'
    self.assertRaises(s3g.TransmissionError, self.r.writer.send_packet, packet)

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
    #self.r.abort_immediately()
    #time.sleep(5)

  def tearDown(self):
    self.r = None

  def test_ToolheadPause(self):
    self.r.toolhead_pause(0)

  def test_ToolheadAbort(self):
    self.r.toolhead_abort(0)

  def test_ResetToFactoryReply(self):
    self.r.reset_to_factory()
    time.sleep(5)

  def test_QueueSongReply(self):
    self.r.queue_song(1)

  def test_SetBuildPercentReply(self):
    self.r.set_build_percent(100)

  def test_SetBeepReply(self):
    self.r.set_beep(1000, 3)

  def test_SetPotentiometerValueReply(self):
    self.r.set_potentiometer_value('x', 118)

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
    self.assertRaises(s3g.CommandNotSupportedError, self.r.get_motor1_speed, 0)

  def test_SetMotor1SpeedReply(self):
    self.r.set_motor1_speed_RPM(0, 0) 

  def test_StoreHomePositionsReply(self):
    self.r.store_home_positions(['X', 'Y', 'Z', 'A', 'B'])
    time.sleep(1)

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
    self.assertRaises(s3g.CommandNotSupportedError,self.r.get_communication_stats)

  def test_GetMotherboardStatusReply(self):
    self.r.get_motherboard_status()

  def test_ExtendedStopReply(self):
    self.r.extended_stop(True, True)

  def test_ResetReply(self):
    self.r.reset()
    time.sleep(5)

  def test_IsFinishedReply(self):
    self.r.is_finished()

  def test_PauseReply(self):
    self.r.pause()

  def test_ClearBufferReply(self):
    self.r.clear_buffer()
    time.sleep(5)

  def test_InitReply(self):
    self.r.init()

  def test_ToggleExtraOutputReply(self):
    self.r.toggle_extra_output(0, False)

  def test_ToggleFanReply(self):
    self.r.toggle_fan(0, False)

  def test_SetServo1Reply(self):
    self.r.set_servo1_position(0,10)

  def test_ToggleMotorReply(self):
    self.r.toggle_motor1(0, True, True);

  def test_IsPlatformReadyReply(self):
    self.r.is_platform_ready(0)

  def test_GetPlatformTargetTemperatureReply(self):
    self.r.get_platform_target_temperature(0)

  def test_GetToolheadTargetTemperatureReply(self):
    self.r.get_toolhead_target_temperature(0)

  def test_ReadFromToolheadEEPROMReply(self):
    self.assertRaises(s3g.CommandNotSupportedError, self.r.read_from_toolhead_EEPROM, 0, 0x00, 0)

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
    self.r.build_end_notification() # we need to send this so that the next command doesn't register a build cancelled

  def test_DisplayMessageReply(self):
    if hasInterface:
      self.r.display_message(0, 0, "TESTING", 1, False, False, False)

  def test_FindAxesMaximumsReply(self):
    self.r.find_axes_maximums(['x', 'y'], 1, 0)

  def test_FindAxesMinimumsReply(self):
    self.r.find_axes_minimums(['z'], 1, 0)

  def test_GetBuildNameReply(self):
    self.r.get_build_name()

  def test_GetNextFilenameReply(self):
    self.r.get_next_filename(False)

  def test_AbortImmediatelyReply(self):
    self.r.abort_immediately()
    time.sleep(5)

  def test_GetAvailableBufferSizeReply(self):
    self.r.get_available_buffer_size()

  def test_ReadFromEEPROMReply(self):
    self.r.read_from_EEPROM(0x00, 0)

  def test_GetVersionReply(self):
    self.r.get_version()

  def test_ChangeTool(self):
    self.r.change_tool(0)

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

  def test_GetBuildStatsReply(self):
    self.r.get_build_stats()

  def test_GetAdvancedVersionNumber(self):
    self.r.get_advanced_version()

class s3gFunctionTests(unittest.TestCase):

  def setUp(self):
    self.r = s3g.s3g()
    self.r.writer=s3g.Writer.StreamWriter(serial.Serial(options.serialPort, '115200', timeout=1))
    self.r.abort_immediately()
    time.sleep(3)

  def tearDown(self):
    self.r = None

  def test_SetGetPlatformTargetTemperature(self):
    target = 100
    self.assertEqual(self.r.get_platform_target_temperature(0), 0)
    self.r.set_platform_temperature(0, target)
    self.assertEqual(self.r.get_platform_target_temperature(0), target)

  def test_SetGetToolheadTargetTemperature(self):
    target = 100
    toolhead = 0
    self.assertEqual(self.r.get_toolhead_target_temperature(toolhead), 0)
    self.r.set_toolhead_temperature(toolhead, target)
    self.assertEqual(self.r.get_toolhead_target_temperature(toolhead), target)

  def test_ReadFromEEPROMMighty(self):
    """
    Read the VID/PID settings from the MB and compare against s3g's read from eeprom
    """
    vidPID = self.r.read_from_EEPROM(0x0044, 4)
    vidPID = array.array('B', vidPID)
    vidPID = struct.unpack('<HH', vidPID)
    mightyVIDPID = [0x23C1, 0xB404]
    self.assertEqual(vidPID[0], mightyVIDPID[0])
    self.assertEqual(vidPID[1], mightyVIDPID[1])


  def test_WriteToEEPROMMighty(self):
    """
    Write a new bot name to the EEPROM and verify that it reads back correctly
    """
    nameOffset = 0x0022
    nameSize = 16
    name = 'ILOVETESTINGALOT'
    self.r.write_to_EEPROM(nameOffset, name)
    readName = self.r.read_from_EEPROM(nameOffset, 16)
    self.assertEqual(name, readName)

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

  def test_SetGetExtendedPosition(self):
    position = [50, 51, 52, 53, 54]
    self.r.set_extended_position(position)
    self.assertEqual(position, self.r.get_extended_position()[0])

  def test_QueueExtendedPoint(self):
    self.r.find_axes_maximums(['x', 'y'], 500, 100)
    self.r.find_axes_minimums(['z'], 500, 100)
    self.r.recall_home_positions(['x', 'y', 'z', 'a', 'b'])

    newPosition = [51, 52, 10, 0, 0]
    rate = 500
    self.r.queue_extended_point(newPosition, rate)
    time.sleep(5)
    self.assertEqual(newPosition, self.r.get_extended_position()[0])

  def test_FindAxesMaximums(self):
    axes = ['x', 'y']
    rate = 500
    timeout = 10
    xymax_endstops = 0x05
    self.r.find_axes_maximums(axes, rate, timeout)
    time.sleep(timeout)
    self.assertEqual(self.r.get_extended_position()[1]&0x0F, xymax_endstops)

  def test_FindAxesMinimums(self):
    axes = ['z']
    rate = 500
    timeout = 50
    zmin_endstops = 0x20
    self.r.find_axes_minimums(axes, rate, timeout)
    time.sleep(timeout)
    self.assertEqual(self.r.get_extended_position()[1]&0x30, zmin_endstops)

  def test_Init(self):
    position = [10, 9, 8, 7, 6]
    self.r.set_extended_position(position)
    #this function doesn't do anything, so we are testing that position is NOT cleared after command recieved
    self.r.init()
    time.sleep(5)
    self.assertEqual(position, self.r.get_extended_position()[0])

  def test_GetAvailableBufferSize(self):
    bufferSize = 512
    self.assertEqual(bufferSize, self.r.get_available_buffer_size())

  def test_AbortImmediately(self):
    bufferSize = 512
    toolheads = [0, 1]
    for toolhead in toolheads:
      self.r.set_toolhead_temperature(toolhead, 100)
    self.r.set_platform_temperature(0, 100)
    self.r.find_axes_minimums(['z'], 500, 5)
    self.r.find_axes_maximums(['x','y'], 500, 5)
    self.r.abort_immediately()
    time.sleep(5)
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
    stats = self.r.get_build_stats()
    build_running_state = 1
    self.assertEqual(stats['BuildState'], build_running_state)
    self.r.build_end_notification()

  def test_BuildEndNotificationandBuildStats(self):
    self.r.build_start_notification("test")
    time.sleep(2)
    stats = self.r.get_build_stats()
    build_running_state = 1
    self.assertEqual(stats['BuildState'], build_running_state)
    self.r.build_end_notification()
    stats = self.r.get_build_stats()
    build_finished_state = 2
    self.assertEqual(stats['BuildState'], build_finished_state)

  def test_BuildStats(self):
    """
    we've tested build start, stop and pause in other tests
    test build cancel 
    test line number
    test print time
    """
    # start build
    self.r.build_start_notification("test")
    start_time = time.time()
    stats = self.r.get_build_stats()
    build_running_state = 1
    self.assertEqual(stats['BuildState'], build_running_state)
    # send 5 commands
    for cmd in range(0,5):
      self.r.toggle_axes(['x', 'y', 'z', 'a', 'b'], True)
    stats = self.r.get_build_stats()
    self.assertEqual(stats['LineNumber'], 5)
    # send 100 commands
    for cmd in range(0,100):
      self.r.toggle_axes(['x', 'y', 'z', 'a', 'b'], False)
    stats = self.r.get_build_stats()
    self.assertEqual(stats['LineNumber'], 105)
    time.sleep(60)
    #check how much time has passed
    time_minutes = int((time.time()-start_time) / 60)
    stats = self.r.get_build_stats()
    self.assertEqual(stats['BuildMinutes'], time_minutes)
    #cancel build
    self.r.abort_immediately()
    self.assertRaises(s3g.BuildCancelledError, self.r.get_build_stats)
    stats = self.r.get_build_stats()
    build_cancelled_state = 4
    self.assertEqual(stats['BuildState'], build_cancelled_state)

  def test_ClearBuffer(self):
    bufferSize = 512
    self.r.set_platform_temperature(0, 100)
    self.r.wait_for_platform_ready(0, 0, 0xFFFF)
    axes = ['z']
    rate = 500
    timeout = 5
    for i in range(10):
      self.r.find_axes_minimums(axes, rate, timeout)
    self.assertNotEqual(bufferSize, self.r.get_available_buffer_size())
    self.r.clear_buffer()
    time.sleep(5) # we need to sleep after sending any reset functions
    self.assertEqual(bufferSize, self.r.get_available_buffer_size())

  def test_PauseandBuildStats(self):
    stats = self.r.get_build_stats()
    build_paused_state = 3
    self.assertNotEqual(stats['BuildState'], build_paused_state)
    self.r.pause()
    stats = self.r.get_build_stats()
    self.assertEqual(stats['BuildState'], build_paused_state)
    
  def test_ToolheadPauseandBuildStats(self):
    stats = self.r.get_build_stats()
    build_paused_state = 3
    self.assertNotEqual(stats['BuildState'], build_paused_state)
    self.r.toolhead_pause(0)
    stats = self.r.get_build_stats()
    self.assertEqual(stats['BuildState'], build_paused_state)

  def test_IsFinished(self):
    timeout = 30
    # home axes
    self.r.find_axes_maximums(['x', 'y'], 500, timeout)
    self.r.find_axes_minimums(['z'], 500, timeout)
    self.r.recall_home_positions(['x', 'y', 'z', 'a', 'b'])
    time.sleep(timeout)
    # go to a new point
    newPoint = [50, 50, 10, 0, 0]
    duration = 5
    micros = 1000000
    self.r.queue_extended_point_new(newPoint, duration*micros, [])
    # assert that bot is not finished
    self.assertFalse(self.r.is_finished())
    time.sleep(duration)
    # assert that bot is finished
    self.assertTrue(self.r.is_finished())

  def test_Reset(self):
    bufferSize = 512
    self.r.set_toolhead_temperature(0, 100)
    self.r.set_platform_temperature(0, 100)
    self.r.wait_for_platform_ready(0, 0, 0xFFFF)
    for i in range(30):
      self.r.toggle_axes(['x', 'y', 'z'], False)
    self.assertNotEqual(self.r.get_available_buffer_size(), bufferSize)
    self.r.reset()
    self.assertEqual(self.r.get_available_buffer_size(), bufferSize)
    self.assertTrue(self.r.is_finished())
    self.assertEqual(self.r.get_toolhead_target_temperature(0), 0)
    self.assertEqual(self.r.get_platform_target_temperature(0), 0)
 
  def test_Delay(self):
    self.r.find_axes_maximums(['x', 'y'], 500, 50)
    self.r.find_axes_minimums(['z'], 500, 50)
    self.r.recall_home_positions(['x', 'y', 'z', 'a', 'b'])
    delay = 5
    duration = 5
    millis = 1000
    newPoint = [50, 50, 10, 0, 0]
    self.r.delay(delay*millis)
    self.r.queue_extended_point_new(newPoint, duration*millis, [])
    self.assertFalse(self.r.is_finished())
    for i in range(0, duration+delay-1):
      print i
      self.assertFalse(self.r.is_finished())
      time.sleep(1)
    time.sleep(1)
    self.assertTrue(self.r.isFinished())

  def test_ExtendedStop(self):
    bufferSize = 512
    self.r.find_axes_maximums(['x', 'y'], 200, 50)
    self.r.find_axes_minimums(['z'], 500, 50)
    self.r.recall_home_positions(['x', 'y', 'z', 'a', 'b'])
    delay = 5
    duration = 5
    millis = 1000
    newPoint = [50, 50, 10, 0, 0]
    self.r.delay(delay*millis)
    self.r.queue_extended_point_new(newPoint, duration*millis, [])
    for i in range(5):
      self.r.toggle_axes(['x', 'y', 'z'], False)
    self.r.extended_stop(True, True)
    time.sleep(2) #Give the machine time to respond
    self.assertTrue(self.r.is_finished())
    self.assertEqual(bufferSize, self.r.get_available_buffer_size())

  def test_QueueExtendedPointNew(self):
    self.r.find_axes_maximums(['x', 'y'], 200, 50)
    self.r.find_axes_minimums(['z'], 500, 50)
    self.r.recall_home_positions(['x', 'y', 'z', 'a', 'b'])
    while(self.r.is_finished is False):
      time.sleep(1)
    newPoint = [1, 2, 3, 0, 0]
    millis = 1000
    duration = 5
    self.r.queue_extended_point_new(newPoint, duration*millis, [])
    time.sleep(duration)
    self.assertEqual(newPoint, self.r.get_extended_position()[0])
    anotherPoint = [5, 6, 7, 8, 9]
    self.r.queue_extended_point_new(anotherPoint, duration*millis, [])
    time.sleep(duration)
    self.assertEqual(anotherPoint, self.r.get_extended_position()[0])
 
  def test_StoreHomePositions(self):
    position = [20,20,30,40,50]
    self.r.set_extended_position(position)
    self.r.store_home_positions(['X', 'Y', 'Z', 'A', 'B'])
    x = self.r.read_from_EEPROM(0x000E, 4)
    y = self.r.read_from_EEPROM(0x0012, 4)
    z = self.r.read_from_EEPROM(0x0016, 4)
    a = self.r.read_from_EEPROM(0x001A, 4)
    b = self.r.read_from_EEPROM(0x001E, 4)
    readHome = []
    for cor in [x, y, z, a, b]:
      readHome.append(s3g.Encoder.decode_int32(cor))
    self.assertEqual(readHome, position)

  def test_RecallHomePositions(self):
    pointToSet = [1, 2, 3, 4, 5]
    self.r.set_extended_position(pointToSet)
    self.r.store_home_positions(['X', 'Y', 'Z', 'A', 'B'])
    newPoint = [50, 51, 52, 53, 54]
    self.r.set_extended_position(newPoint)
    self.r.recall_home_positions([])
    time.sleep(5)
    self.assertEqual(newPoint, self.r.get_extended_position()[0])
    self.r.recall_home_positions(['X', 'Y', 'Z', 'A', 'B'])
    time.sleep(5)
    self.assertEqual(pointToSet, self.r.get_extended_position()[0])
 
  def test_GetPIDState(self):
    toolhead = 0
    pidDict = self.r.get_PID_state(toolhead)
    for key in pidDict:
      self.assertNotEqual(pidDict[key], None)


class s3gUserFunctionTests(unittest.TestCase):

  def setUp(self):
    self.r = s3g.s3g()
    self.r.writer = s3g.Writer.StreamWriter(serial.Serial(options.serialPort, '115200', timeout=1))
    self.r.abort_immediately()

  def tearDown(self):
    self.r = None

  """
  def test_WaitForPlatformReady(self):

  def test_WaitForToolReady(self):
  """
  def test_ToggleAxes(self):
    self.r.toggle_axes(True, True, True, True, True, True)
    obs = raw_input("\nPlease try to move all (x,y,z) the axes!  Can you move them without using too much force? (y/n) ")
    self.assertEqual('n', obs)
    self.r.toggle_axes(True, True, True, True, True, False)
    obs = raw_input("\nPlease try to move all (x,y,z) the axes!  Can you move them without using too much force? (y/n) ")
    self.assertEqual('y', obs)
 
  def test_DisplayMessage(self):
    if hasInterface:
      message = str(time.clock())
      self.r.display_message(0, 0, message, 10, False, False, False)
      readMessage = raw_input("\nWhat is the message on the replicator's display? ")
      self.assertEqual(message, readMessage)

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


class s3gSDCardTests(unittest.TestCase):

  def test_PlaybackCaptureReply(self):
    self.assertRaises(s3g.SDCardError, self.r.playback_capture, 'aTest')

  def test_CaptureToFileReply(self):
    self.r.capture_to_file('test')

  def test_EndCaptureToFileReply(self):
    self.r.end_capture_to_file()

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
  parser.add_option("-p", "--port", dest="serialPort", default="/dev/ttyACM0")
  (options, args) = parser.parse_args()
  if options.extensive.lower() == "false":
    print "Forgoing Heater Tests"
    extensive= False
  if options.hasInterface.lower() == "false":
    print "Forgoing Tests requiring Interface Boards"
    hasInterface = False

  logging.basicConfig()
  del sys.argv[1:]
  
  commonTests = unittest.TestLoader().loadTestsFromTestCase(commonFunctionTests)
  packetTests = unittest.TestLoader().loadTestsFromTestCase(s3gPacketTests)
  sendReceiveTests = unittest.TestLoader().loadTestsFromTestCase(s3gSendReceiveTests)
  functionTests = unittest.TestLoader().loadTestsFromTestCase(s3gFunctionTests)
  #sdTests = unittest.TestLoader().loadTestsFromTestCase(s3gSDCardTests)
  suites = [sendReceiveTests] #packetTests] #, sendReceiveTests, functionTests, sdTests, smallTest]
  for suite in suites:
    unittest.TextTestRunner(verbosity=2).run(suite)
