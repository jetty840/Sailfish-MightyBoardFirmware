// Yep, this is actually -*- c++ -*-


//initialize a tool to its default state.
void init_tool(byte i)
{
  slavePacket.init();

  slavePacket.add_8(i);
  slavePacket.add_8(SLAVE_CMD_INIT);
  send_packet();
}

//select a tool as our current tool, and let it know.
void select_tool(byte tool)
{
  slavePacket.init();

  slavePacket.add_8(tool);
  slavePacket.add_8(SLAVE_CMD_SELECT_TOOL);
  send_packet();
}

//ping the tool until it tells us its ready
void wait_for_tool_ready_state(byte tool, int delay_millis, int timeout_seconds)
{
  //setup some defaults
  if (delay_millis == 0)
    delay_millis = 100;
  if (timeout_seconds == 0)
    timeout_seconds = 60;

  //check for our end time.
  unsigned long end = millis() + (timeout_seconds * 1000);

  //do it until we hear something, or time out.
  while (1)
  {
    //did we time out yet?
    if (millis() >= end)
      return;

    //did we hear back from the tool?
    if (is_tool_ready(tool))
      return;

    //try again...
    delay(delay_millis);
  }
}

//is our tool ready for action?
bool is_tool_ready(byte tool)
{
  slavePacket.init();

  slavePacket.add_8(tool);
  slavePacket.add_8(SLAVE_CMD_IS_TOOL_READY);

  //did we get a response?
  if (send_packet())
  {
    //is it true?
    if (slavePacket.get_8(1) == 1)
      return true;
  }

  //bail.
  return false;
}

boolean send_packet()
{
  //take it easy.  no stomping on each other.
  delayMicrosecondsInterruptible(50);

  digitalWrite(TX_ENABLE_PIN, HIGH); //enable tx

  //take it easy.  no stomping on each other.
  delayMicrosecondsInterruptible(10);

  slavePacket.sendPacket();

  digitalWrite(TX_ENABLE_PIN, LOW); //disable tx

  rs485_packet_count++;

  return read_tool_response(PACKET_TIMEOUT);
}

bool read_tool_response(int timeout)
{
  //figure out our timeout stuff.
  long start = millis();
  long end = start + timeout;

  //keep reading until we got it.
  while (!slavePacket.isFinished())
  {
    //read through our available data
    if (Serial1.available() > 0)
    {
      //grab a byte and process it.
      byte d = Serial1.read();
      slavePacket.process_byte(d);

      rs485_rx_count++;

#ifdef ENABLE_COMMS_DEBUG
      /*
      Serial.print("IN:");
       Serial.print(d, HEX);
       Serial.print("/");
       Serial.println(d, BIN);
       */
#endif
      //keep processing while there's data. 
      start = millis();
      end = start + timeout;

      if (slavePacket.getResponseCode() == RC_CRC_MISMATCH)
      {
        slave_crc_errors++;

#ifdef ENABLE_COMMS_DEBUG
        Serial.println("Slave CRC Mismatch");
#endif
        //retransmit?
      }
    }

    //not sure if we need this yet.
    //our timeout guy.
    if (millis() > end)
    {
      slave_timeouts++;

#ifdef ENABLE_COMMS_DEBUG
      Serial.println("Slave Timeout");
#endif
      return false;
    }
  }

  return true;
}

void set_tool_pause_state(bool paused)
{
  //TODO: pause/unpause tool.
}

#ifdef ENABLE_DEBUG

void exercise_heater()
{
  set_tool_temp(1, 100);
  while (1)
  {
    get_tool_temp(1);
    delay(500);
  }
}

void exercise_motors()
{
  boolean dir = true;

  Serial.println("forward");
  Serial.println("up");
  for (int i=0; i<256; i++)
  {
    set_motor1_pwm(1, i);
    toggle_motor1(1, dir, 1);
    set_motor2_pwm(1, i);
    toggle_motor2(1, dir, 1);
	Serial.println(i, DEC);
  }

  Serial.println("down");
  for (int i=255; i>=0; i--)
  {
    set_motor1_pwm(1, i);
    toggle_motor1(1, dir, 1);
    set_motor2_pwm(1, i);
    toggle_motor2(1, dir, 1);
	Serial.println(i, DEC);
  }

  dir = false;

  Serial.println("forward");
  Serial.println("up");
  for (int i=0; i<256; i++)
  {
    set_motor1_pwm(1, i);
    toggle_motor1(1, dir, 1);
    set_motor2_pwm(1, i);
    toggle_motor2(1, dir, 1);
	Serial.println(i, DEC);
  }

  Serial.println("down");
  for (int i=255; i>=0; i--)
  {
    set_motor1_pwm(1, i);
    toggle_motor1(1, dir, 1);
    set_motor2_pwm(1, i);
    toggle_motor2(1, dir, 1);
	Serial.println(i, DEC);
  }
}

void print_stats()
{
  Serial.println("Stats:");
  Serial.print("Slave TX Count:");
  Serial.println(rs485_tx_count, DEC);
  Serial.print("Slave RX Count:");
  Serial.println(rs485_rx_count, DEC);
  Serial.print("Slave Packet Count: ");
  Serial.println(rs485_packet_count, DEC);
  Serial.print("Slave CRC Errors: ");
  Serial.println(slave_crc_errors, DEC);
  Serial.print("Slave timeouts: ");
  Serial.println(slave_timeouts, DEC);
}

void print_tool(byte id)
{
  Serial.print("tool #");
  Serial.print(id, DEC);
  Serial.print(" ");
}

int get_tool_temp(byte id)
{
  slavePacket.init();
  slavePacket.add_8(id);
  slavePacket.add_8(SLAVE_CMD_GET_TEMP);
  send_packet();

  int temp = slavePacket.get_16(1);
  return temp;
}

void set_tool_temp(byte id, unsigned int temp)
{
  slavePacket.init();
  slavePacket.add_8(id);
  slavePacket.add_8(SLAVE_CMD_SET_TEMP);
  slavePacket.add_16(temp);
  send_packet();

  print_tool(id);
  Serial.print("set temp to: ");
  Serial.println(temp, DEC);
}

void set_motor1_pwm(byte id, byte pwm)
{
  slavePacket.init();
  slavePacket.add_8(id);
  slavePacket.add_8(SLAVE_CMD_SET_MOTOR_1_PWM);
  slavePacket.add_8(pwm);
  send_packet();

  //print_tool(id);
  //Serial.print("set motor1 pwm to: ");
  //Serial.println(pwm, DEC);
}

void set_motor2_pwm(byte id, byte pwm)
{
  slavePacket.init();
  slavePacket.add_8(id);
  slavePacket.add_8(SLAVE_CMD_SET_MOTOR_2_PWM);
  slavePacket.add_8(pwm);
  send_packet();

  //print_tool(id);
  //Serial.print("set motor2 pwm to: ");
  //Serial.println(pwm, DEC);
}


void set_motor1_rpm(byte id, int rpm)
{

}

void toggle_motor1(byte id, boolean dir, boolean enable)
{
  byte flags = 0;

  if (enable)
    flags += 1;

  if (dir)
    flags += 2;

  Serial.println(flags, BIN);

  slavePacket.init();
  slavePacket.add_8(id);
  slavePacket.add_8(SLAVE_CMD_TOGGLE_MOTOR_1);
  slavePacket.add_8(flags);
  send_packet();
}

void toggle_motor2(byte id, boolean dir, boolean enable)
{
  byte flags = 0;

  if (enable)
    flags += 1;

  if (dir)
    flags += 2;

  slavePacket.init();
  slavePacket.add_8(id);
  slavePacket.add_8(SLAVE_CMD_TOGGLE_MOTOR_2);
  slavePacket.add_8(flags);
  send_packet();
}

void toggle_fan(byte id, boolean enable)
{
  slavePacket.init();
  slavePacket.add_8(id);
  slavePacket.add_8(SLAVE_CMD_TOGGLE_FAN);
  slavePacket.add_8(enable);
  send_packet();
}

void toggle_valve(byte id, boolean open)
{
  slavePacket.init();
  slavePacket.add_8(id);
  slavePacket.add_8(SLAVE_CMD_TOGGLE_VALVE);
  slavePacket.add_8(open);
  send_packet();
}

void get_motor1_pwm(byte id)
{
  slavePacket.init();
  slavePacket.add_8(id);
  slavePacket.add_8(SLAVE_CMD_GET_MOTOR_1_PWM);
  send_packet();

  byte temp = slavePacket.get_8(1);
  print_tool(id);
  Serial.print("m1 pwm: ");
  Serial.println(temp, DEC);
}

void get_motor2_pwm(byte id)
{
  slavePacket.init();
  slavePacket.add_8(id);
  slavePacket.add_8(SLAVE_CMD_GET_MOTOR_2_PWM);
  send_packet();

  byte temp = slavePacket.get_8(1);
  print_tool(id);
  Serial.print("m2 pwm: ");
  Serial.println(temp, DEC);
}

void set_servo1_position(byte id, byte pos)
{
  slavePacket.init();
  slavePacket.add_8(id);
  slavePacket.add_8(SLAVE_CMD_SET_SERVO_1_POS);
  slavePacket.add_8(pos);
  send_packet();
}

void set_servo2_position(byte id, byte pos)
{
  slavePacket.init();
  slavePacket.add_8(id);
  slavePacket.add_8(SLAVE_CMD_SET_SERVO_2_POS);
  slavePacket.add_8(pos);
  send_packet();
}

void get_motor1_rpm(byte id)
{

}

void get_filament_status(byte id)
{
  slavePacket.init();
  slavePacket.add_8(id);
  slavePacket.add_8(SLAVE_CMD_FILAMENT_STATUS);
  send_packet();

  byte temp = slavePacket.get_8(1);
  print_tool(id);
  Serial.print("filament: ");
  Serial.println(temp, DEC);
}

#endif

