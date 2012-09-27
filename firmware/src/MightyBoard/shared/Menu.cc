
#include "Configuration.hh"

// TODO: Kill this, should be hanlded by build system.
#ifdef HAS_INTERFACE_BOARD

#include "Steppers.hh"
#include "Commands.hh"
#include "Errors.hh"
#include "Host.hh"
#include "Timeout.hh"
#include "InterfaceBoard.hh"
#include "Interface.hh"
#include <util/delay.h>
#include <stdlib.h>
#include "SDCard.hh"
#include <string.h>
#include "Version.hh"
#include "EepromMap.hh"
#include "Eeprom.hh"
#include <avr/eeprom.h>
#include "RGB_LED.hh"
#include "Planner.hh"
#include "stdio.h"
#include "Menu_locales.hh"
#include "Piezo.hh"

CancelBuildMenu cancel_build_menu;
BuildStats build_stats_screen;
FilamentScreen filamentScreen;
SDMenu sdmenu;
ReadyMenu ready;
LevelOKMenu levelOK;
ActiveBuildMenu active_build_menu;
MonitorMode monitorMode;
JogMode jogger;
WelcomeScreen welcome;
BotStats bot_stats;
SettingsMenu set;
PreheatSettingsMenu preheatSettings;
ResetSettingsMenu reset_settings;
FilamentMenu filamentMenu;
NozzleCalibrationScreen alignment;
SplashScreen splash;
HeaterPreheat preheat;
UtilitiesMenu utils;
SelectAlignmentMenu align;
FilamentOKMenu filamentOK;
InfoMenu info;


//#define HOST_PACKET_TIMEOUT_MS 20
//#define HOST_PACKET_TIMEOUT_MICROS (1000L*HOST_PACKET_TIMEOUT_MS)

//#define HOST_TOOL_RESPONSE_TIMEOUT_MS 50
//#define HOST_TOOL_RESPONSE_TIMEOUT_MICROS (1000L*HOST_TOOL_RESPONSE_TIMEOUT_MS)

#define FILAMENT_HEAT_TEMP 230

bool ready_fail = false;
bool cancel_process = true;

enum sucessState{
  SUCCESS,
  FAIL,
  SECOND_FAIL
};

uint8_t levelSuccess;
uint8_t filamentSuccess;

SplashScreen::SplashScreen(){
  hold_on = false;
}

void SplashScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {


  if (forceRedraw) {
    lcd.setCursor(0,0);
    lcd.writeFromPgmspace(SPLASH1_MSG);

    lcd.setCursor(0,1);
    lcd.writeFromPgmspace(SPLASH2_MSG);

    // display internal version number if it exists
    if (internal_version != 0){
      lcd.setCursor(0,2);
      lcd.writeFromPgmspace(SPLASH5_MSG);
      
      lcd.setCursor(17,2);
      lcd.writeInt((uint16_t)internal_version,3);
    } else {
      lcd.setCursor(0,2);
      lcd.writeFromPgmspace(SPLASH3_MSG);
    }

    lcd.setCursor(0,3);
    lcd.writeFromPgmspace(SPLASH4_MSG);
    
    /// get major firmware version number
    uint16_t major_digit = firmware_version / 100;
    /// get minor firmware version number
    uint16_t minor_digit = firmware_version % 100;
    lcd.setCursor(17,3);
    lcd.writeInt(major_digit, 1);
    /// period is written as part of SLASH4_MSG
    lcd.setCursor(19,3);
    lcd.writeInt(minor_digit, 1);
  }
  else if (!hold_on) {
  //   The machine has started, so we're done!
                interface::popScreen();
    }
}

void SplashScreen::SetHold(bool on){
  hold_on = on;
}

void SplashScreen::notifyButtonPressed(ButtonArray::ButtonName button) {
  // We can't really do anything, since the machine is still loading, so ignore.
  switch (button) {
    case ButtonArray::CENTER:
      interface::popScreen();
      break;
        case ButtonArray::LEFT:
      interface::popScreen();
      break;
        case ButtonArray::RIGHT:
        case ButtonArray::DOWN:
        case ButtonArray::UP:
      break;

  }
}

void SplashScreen::reset() {
  
}

HeaterPreheat::HeaterPreheat(){
  itemCount = 4;
  reset();
}

void HeaterPreheat::resetState(){
  uint8_t heatSet = eeprom::getEeprom8(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_ON_OFF_OFFSET, 0);
  _rightActive = (heatSet & (1 << HEAT_MASK_RIGHT)) != 0;
  _platformActive = eeprom::hasHBP() && ((heatSet & (1 << HEAT_MASK_PLATFORM)) != 0);
  _leftActive = (heatSet & (1 << HEAT_MASK_LEFT)) != 0;
  singleTool = eeprom::isSingleTool();
  if(singleTool){ _leftActive = false; }
  Motherboard &board = Motherboard::getBoard();
  if(((board.getExtruderBoard(0).getExtruderHeater().get_set_temperature() > 0) && _rightActive) ||
      ((board.getExtruderBoard(1).getExtruderHeater().get_set_temperature() > 0) && _leftActive) ||
      ((board.getPlatformHeater().get_set_temperature() >0) && _platformActive))
  {   preheatActive = true;}
  else
  {   preheatActive = false;}

  if(eeprom::hasHBP()){
    itemCount = 4;
  }else{
    itemCount = 3;
  }
}

void HeaterPreheat::drawItem(uint8_t index, LiquidCrystalSerial& lcd, uint8_t line_number) {
  
  Motherboard &board = Motherboard::getBoard();

  switch (index) {
  case 0:
        if(preheatActive)
            lcd.writeFromPgmspace(STOP_MSG);
        else
            lcd.writeFromPgmspace(GO_MSG);
    break;
  case 1:
    if(!singleTool){
      lcd.writeFromPgmspace(RIGHT_TOOL_MSG);
      lcd.setCursor(16,line_number);
      if(board.getExtruderBoard(0).getExtruderHeater().has_failed()){
        lcd.writeFromPgmspace(NA2_MSG);
      }else if(_rightActive){
        lcd.writeFromPgmspace(ON_MSG);
      }else{
        lcd.writeFromPgmspace(OFF_MSG);
      }
    }
    break;
  case 2:     
    if(singleTool){
      lcd.writeFromPgmspace(TOOL_MSG);
      lcd.setCursor(16,line_number);
      if(board.getExtruderBoard(0).getExtruderHeater().has_failed()){
        lcd.writeFromPgmspace(NA2_MSG);
      }else if(_rightActive)
        lcd.writeFromPgmspace(ON_MSG);
      else
        lcd.writeFromPgmspace(OFF_MSG);
    }
    else{
      lcd.writeFromPgmspace(LEFT_TOOL_MSG);
      lcd.setCursor(16,line_number);
      if(board.getExtruderBoard(1).getExtruderHeater().has_failed()){
        lcd.writeFromPgmspace(NA2_MSG);
      }else if(_leftActive)
        lcd.writeFromPgmspace(ON_MSG);
      else
        lcd.writeFromPgmspace(OFF_MSG);
    }
    break;
  case 3:
    lcd.writeFromPgmspace(PLATFORM_MSG);
        lcd.setCursor(16,line_number);
        if(board.getPlatformHeater().has_failed()){
      lcd.writeFromPgmspace(NA2_MSG);
    }else if(_platformActive)
      lcd.writeFromPgmspace(ON_MSG);
    else
      lcd.writeFromPgmspace(OFF_MSG);
    break;
  }
}
         
void HeaterPreheat::storeHeatByte(){
    uint8_t heatByte = (_rightActive*(1<<HEAT_MASK_RIGHT)) + (_leftActive*(1<<HEAT_MASK_LEFT)) + (_platformActive*(1<<HEAT_MASK_PLATFORM));
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
      eeprom_write_byte((uint8_t*)(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_ON_OFF_OFFSET), heatByte);
    }
}

void HeaterPreheat::handleSelect(uint8_t index) {
  int temp;
  switch (index) {
    case 0:
            preheatActive = !preheatActive;
            // clear paused state if any
            Motherboard::getBoard().getExtruderBoard(0).getExtruderHeater().Pause(false);
            Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().Pause(false);
            if(preheatActive){
                Motherboard::getBoard().resetUserInputTimeout();
                /// _platformActive cannot be true if hbp is not presend (eeprom record)
                temp = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_PLATFORM_OFFSET,0) *_platformActive;
                Motherboard::getBoard().getPlatformHeater().set_target_temperature(temp);
                temp = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_RIGHT_OFFSET,0) *_rightActive; 
                Motherboard::getBoard().getExtruderBoard(0).getExtruderHeater().set_target_temperature(temp);
                if(!singleTool){
                    temp = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_LEFT_OFFSET,0) *_leftActive;
                    Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().set_target_temperature(temp);
                }
                if(Motherboard::getBoard().getPlatformHeater().isHeating()){
                    Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().Pause(true);
                    Motherboard::getBoard().getExtruderBoard(0).getExtruderHeater().Pause(true);
                }
                
                if(_platformActive || _rightActive || _leftActive){
                  Motherboard::getBoard().setBoardStatus(Motherboard::STATUS_PREHEATING, true);
                }
            }
            else{
                Motherboard::getBoard().getExtruderBoard(0).getExtruderHeater().set_target_temperature(0);
                Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().set_target_temperature(0);
                Motherboard::getBoard().getPlatformHeater().set_target_temperature(0);
                
                Motherboard::getBoard().setBoardStatus(Motherboard::STATUS_PREHEATING, false);
            }
            interface::popScreen();
            interface::pushScreen(&monitorMode);
            //needsRedraw = true;
      break;
    case 1:
      if(!singleTool){
        _rightActive  = !_rightActive;
        storeHeatByte();
        if(preheatActive){
          needsRedraw = true;
        }else{
          lineUpdate = true;
        }
        preheatActive = false;
      }
            
      break;
    case 2:
      if(singleTool)
        _rightActive  = !_rightActive;
      else
        _leftActive  = !_leftActive;
            storeHeatByte();
            if(preheatActive){
              needsRedraw = true;
            }else{
        lineUpdate = true;
        }
            preheatActive = false; 
      break;
    case 3:
      _platformActive = !_platformActive;
            storeHeatByte();
            if(preheatActive){
        needsRedraw = true;
      }else{
        lineUpdate = true;
        }
            preheatActive = false;
      break;
    }
}

void WelcomeScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
    
    if(cancel_process == true){
    welcomeState = WELCOME_DONE;
    cancel_process = false;
  }
    
  if (forceRedraw || needsRedraw) {
    Motherboard::getBoard().setBoardStatus(Motherboard::STATUS_ONBOARD_PROCESS, true);
    lcd.setCursor(0,0);
        switch (welcomeState){
            case WELCOME_START:
                lcd.writeFromPgmspace(START_MSG);
               // _delay_us(1000000);
                Motherboard::getBoard().interfaceBlink(25,15);
                
                 break;
            case WELCOME_BUTTONS1:
        lcd.writeFromPgmspace(BUTTONS1_MSG);
        //_delay_us(1000000);
                Motherboard::getBoard().interfaceBlink(25,15);

                 break;
            case WELCOME_BUTTONS2:
        lcd.writeFromPgmspace(BUTTONS2_MSG);
        //_delay_us(1000000);
                Motherboard::getBoard().interfaceBlink(25,15);
                
                 break;
      case WELCOME_EXPLAIN:
                lcd.writeFromPgmspace(EXPLAIN_MSG);
                //_delay_us(1000000);
                Motherboard::getBoard().interfaceBlink(25,15);

                break;
            case WELCOME_LEVEL:
                lcd.writeFromPgmspace(LEVEL_MSG);
                Motherboard::getBoard().interfaceBlink(25,15);
                //_delay_us(1000000);
                ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
                  eeprom_write_byte((uint8_t*)eeprom_offsets::FIRST_BOOT_FLAG, 1);
                }
                break;
            case WELCOME_LEVEL_OK:
      //  waiting = false;
        interface::pushScreen(&levelOK);
        _delay_us(1000000);
        welcomeState++;
        break;
            case WELCOME_LOAD_PLASTIC:
        if(levelSuccess == SUCCESS){
          lcd.writeFromPgmspace(BETTER_MSG);
        } else if(levelSuccess == FAIL){
          lcd.writeFromPgmspace(TRYAGAIN_MSG);
          welcomeState = WELCOME_LEVEL;
        } else if(levelSuccess == SECOND_FAIL){
          lcd.writeFromPgmspace(GO_ON_MSG);
        }
        _delay_us(500000);
                Motherboard::getBoard().interfaceBlink(25,15);            
                break;
            case WELCOME_READY:
                interface::pushScreen(&ready);
                welcomeState++;
                break;
            case WELCOME_LOAD_SD:
                if(ready_fail){
                    lcd.writeFromPgmspace(FAIL_MSG);
                    welcomeState++;
                }
                else
                 lcd.writeFromPgmspace(SD_MENU_MSG);
                 //_delay_us(1000000);
                 Motherboard::getBoard().interfaceBlink(25,15);
                
                break;
            case WELCOME_DONE:
        host::stopBuild();
                interface::popScreen();
                break;
            
        }
        needsRedraw = false;
  }
}

void WelcomeScreen::notifyButtonPressed(ButtonArray::ButtonName button) {
  
  Point position;
  
  switch (button) {
    case ButtonArray::CENTER:
       welcomeState++;
       switch (welcomeState){
         case WELCOME_LEVEL_ACTION:
           Motherboard::getBoard().interfaceBlink(0,0);
           welcomeState++; 
           if(levelSuccess == FAIL)
             host::startOnboardBuild(utility::LEVEL_PLATE_SECOND);
           else
             host::startOnboardBuild(utility::LEVEL_PLATE_STARTUP);
           break;
         case WELCOME_LOAD_ACTION:
           Motherboard::getBoard().interfaceBlink(0,0);
           welcomeState++;
           if(eeprom::getEeprom8(eeprom_offsets::TOOL_COUNT, 1) == 1)
             filamentScreen.setScript(FILAMENT_STARTUP_SINGLE);
           else
             filamentScreen.setScript(FILAMENT_STARTUP_DUAL);
           interface::pushScreen(&filamentScreen);
           break;
          case WELCOME_PRINT_FROM_SD:
            Motherboard::getBoard().interfaceBlink(0,0);
            Motherboard::getBoard().setBoardStatus(Motherboard::STATUS_ONBOARD_PROCESS, false);
            welcomeState++;
            interface::pushScreen(&sdmenu);
            break;
          default:
            needsRedraw = true;
            break;
       }
     break;
     case ButtonArray::LEFT:
      welcomeState--;
      if(welcomeState < WELCOME_START){
        welcomeState = WELCOME_START;
      }
      switch (welcomeState){
        case WELCOME_LEVEL_ACTION:
          Motherboard::getBoard().interfaceBlink(0,0);
          welcomeState++; 
          if(levelSuccess == FAIL)
            host::startOnboardBuild(utility::LEVEL_PLATE_SECOND);
          else
            host::startOnboardBuild(utility::LEVEL_PLATE_STARTUP);
            break;
        case WELCOME_LOAD_ACTION:
          Motherboard::getBoard().interfaceBlink(0,0);
          welcomeState++;
                    
          if(eeprom::getEeprom8(eeprom_offsets::TOOL_COUNT, 1) == 1)
              filamentScreen.setScript(FILAMENT_STARTUP_SINGLE);
          else
            filamentScreen.setScript(FILAMENT_STARTUP_DUAL);
          interface::pushScreen(&filamentScreen);
          break;
        case WELCOME_PRINT_FROM_SD:
            Motherboard::getBoard().interfaceBlink(0,0);
             Motherboard::getBoard().setBoardStatus(Motherboard::STATUS_ONBOARD_PROCESS, false);
            welcomeState++;
            interface::pushScreen(&sdmenu);
            break;
        default:
            needsRedraw = true;
            break;
      } 
      break;
      
   case ButtonArray::RIGHT:
   case ButtonArray::DOWN:
   case ButtonArray::UP:
   break;

  }
}

void WelcomeScreen::reset() {
    needsRedraw = false;
    Motherboard::getBoard().interfaceBlink(25,15);
    welcomeState=WELCOME_START;
    ready_fail = false;
    levelSuccess = SUCCESS;
    level_offset = 0;
    cancel_process = false;
}

void NozzleCalibrationScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
    
  if (forceRedraw || needsRedraw) {
        lcd.setCursor(0,0);
        switch (alignmentState){
            case ALIGNMENT_START:
                lcd.writeFromPgmspace(START_TEST_MSG);
     //           _delay_us(500000);
                Motherboard::getBoard().interfaceBlink(25,15);    
                 break;
            case ALIGNMENT_EXPLAIN1:
        lcd.writeFromPgmspace(EXPLAIN1_MSG);
       //         _delay_us(500000);
                Motherboard::getBoard().interfaceBlink(25,15);    
                 break;
            case ALIGNMENT_EXPLAIN2:
        lcd.writeFromPgmspace(EXPLAIN2_MSG);
         //       _delay_us(500000);
                Motherboard::getBoard().interfaceBlink(25,15);    
                 break;
            case ALIGNMENT_SELECT:
        Motherboard::getBoard().interfaceBlink(0,0);
        interface::pushScreen(&align);
         alignmentState++;
                 break;
            case ALIGNMENT_END:
        lcd.writeFromPgmspace(END_MSG);
        //_delay_us(500000);
                Motherboard::getBoard().interfaceBlink(25,15);
                 break;  
        }
        needsRedraw = false;
  }
}

void NozzleCalibrationScreen::notifyButtonPressed(ButtonArray::ButtonName button) {
  
  Point position;
  
  switch (button) {
    case ButtonArray::CENTER:
           alignmentState++;
           
      switch (alignmentState){
          case ALIGNMENT_PRINT:
            Motherboard::getBoard().interfaceBlink(0,0); 
            host::startOnboardBuild(utility::TOOLHEAD_CALIBRATE);
            alignmentState++;
            break;
          case ALIGNMENT_QUIT:
            Motherboard::getBoard().interfaceBlink(0,0); 
            interface::popScreen();
            break;
          default:
            needsRedraw = true;
            break;
      }
      break;
   case ButtonArray::LEFT:
      interface::pushScreen(&cancel_build_menu);
      break;
      
   case ButtonArray::RIGHT:
   case ButtonArray::DOWN:
   case ButtonArray::UP:
   break;

  }
}

void NozzleCalibrationScreen::reset() {
    needsRedraw = false;
    Motherboard::getBoard().interfaceBlink(25,15);
    alignmentState=ALIGNMENT_START;
}

SelectAlignmentMenu::SelectAlignmentMenu() {
  itemCount = 4;
    reset();
    for (uint8_t i = 0; i < itemCount; i++){
    counter_item[i] = 0;
  }
    counter_item[1] = counter_item[2] = 1;
}

void SelectAlignmentMenu::resetState(){
  itemIndex = 1;
  firstItemIndex = 1;
  xCounter = 7;
  yCounter = 7;
}

void SelectAlignmentMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd, uint8_t line_number) {
  
    
    int32_t offset;
  switch (index) {
    case 0:
      lcd.writeFromPgmspace(SELECT_MSG);
      break;
        case 1:
      lcd.writeFromPgmspace(XAXIS_MSG);
      lcd.setCursor(13, line_number);
      if(selectIndex == 1)
                lcd.writeFromPgmspace(ARROW_MSG);
            else
        lcd.writeFromPgmspace(NO_ARROW_MSG);
            lcd.setCursor(17, line_number);
            lcd.writeInt(xCounter,2);
            break;
         case 2:
      lcd.writeFromPgmspace(YAXIS_MSG);
       lcd.setCursor(13, line_number);
      if(selectIndex == 2)
                lcd.writeFromPgmspace(ARROW_MSG);
            else
        lcd.writeFromPgmspace(NO_ARROW_MSG);
            lcd.setCursor(17, line_number);
            lcd.writeInt(yCounter, line_number);
            break;
         case 3:
      lcd.writeFromPgmspace(DONE_MSG);
      break;
  }
}

void SelectAlignmentMenu::handleCounterUpdate(uint8_t index, bool up){
   
    switch (index) {
        case 1:
            // update platform counter
            // update right counter
            if(up)
                xCounter++;
            else
                xCounter--;
            // keep within appropriate boundaries    
            if(xCounter > 13)
                xCounter = 13;
            else if(xCounter < 1)
        xCounter = 1;     
            break;
        case 2:
            // update right counter
           if(up)
                yCounter++;
            else
                yCounter--;
           // keep within appropriate boundaries    
            if(yCounter > 13)
                yCounter = 13;
            else if(yCounter < 1)
        yCounter = 1; 
            break;
  }  
}


void SelectAlignmentMenu::handleSelect(uint8_t index) {
  
  int32_t offset;
  switch (index) {
    case 1:
      // update toolhead offset (tool tolerance setting) 
      // this is summed with previous offset setting
      offset = (int32_t)(eeprom::getEeprom32(eeprom_offsets::TOOLHEAD_OFFSET_SETTINGS, 0)) + (int32_t)((xCounter-7)*XSTEPS_PER_MM *0.1f * 10);
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        eeprom_write_block((uint8_t*)&offset, (uint8_t*)eeprom_offsets::TOOLHEAD_OFFSET_SETTINGS, 4);
      }
      lineUpdate = 1;
      break;
    case 2:
      // update toolhead offset (tool tolerance setting)
      offset = (int32_t)(eeprom::getEeprom32(eeprom_offsets::TOOLHEAD_OFFSET_SETTINGS + 4, 0)) + (int32_t)((yCounter-7)*YSTEPS_PER_MM *0.1f * 10);
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        eeprom_write_block((uint8_t*)&offset, (uint8_t*)eeprom_offsets::TOOLHEAD_OFFSET_SETTINGS + 4, 4);
      }
      lineUpdate = 1;
      break;
    case 3:
      interface::popScreen();
      break;
    }
}

void FilamentScreen::startMotor(){
    int32_t interval = 300000000;  // 5 minutes
    int32_t steps = interval / 6250;
    if(forward)
        steps *= -1;
    Point target = Point(0,0,0,0,0);
    target[axisID] = steps;
    
    planner::abort();
    planner::setAccelerationOn(false);
    planner::addMoveToBufferRelative(target, interval, 0x1f);
    filamentTimer.clear();
    filamentTimer.start(300000000); //5 minutes
}
void FilamentScreen::stopMotor(){
    
    planner::abort();
    // disable motors if we are not in the middle of a build
    if(host::getHostState() == host::HOST_STATE_READY){
    for(int i = 0; i < STEPPER_COUNT; i++)
      steppers::enableAxis(i, false);
  }

}

void FilamentScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
    
    
    Point target = Point(0,0,0, 0,0);
    int32_t interval;

    if(cancel_process == true){
    filamentState = FILAMENT_EXIT;
    cancel_process = false;
  }
    
  if(filamentState == FILAMENT_WAIT){
    
    /// if extruder has reached hot temperature, start extruding
    if(Motherboard::getBoard().getExtruderBoard(toolID).getExtruderHeater().has_reached_target_temperature()){
      
      int16_t setTemp = (int16_t)(Motherboard::getBoard().getExtruderBoard(toolID).getExtruderHeater().get_set_temperature());
      /// check for externally manipulated temperature (eg by RepG)
      if(setTemp < FILAMENT_HEAT_TEMP){
        Motherboard::getBoard().StopProgressBar();
        interface::popScreen();
        Motherboard::getBoard().errorResponse(ERROR_TEMP_RESET_EXTERNALLY);
        Piezo::playTune(TUNE_ERROR);
        return;
      }
      Piezo::playTune(TUNE_FILAMENT_START);
      
      Motherboard::getBoard().StopProgressBar();
      filamentState++;
      needsRedraw= true;
      startMotor();
      if(!helpText && !startup)
        filamentState = FILAMENT_STOP;
    }
    /// if heating timer has eleapsed, alert user that the heater is not getting hot as expected
    else if (filamentTimer.hasElapsed()){
      Motherboard::getBoard().StopProgressBar();
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.writeFromPgmspace(HEATER_ERROR_MSG);
      Piezo::playTune(TUNE_ERROR);
      Motherboard::getBoard().interfaceBlink(25,15);
      filamentState = FILAMENT_DONE;
    }
    /// if extruder is still heating, update heating bar status
    else{
            int16_t setTemp = (int16_t)(Motherboard::getBoard().getExtruderBoard(toolID).getExtruderHeater().get_set_temperature());
            // check for externally manipulated temperature (eg by RepG)
      if(setTemp < FILAMENT_HEAT_TEMP){
        Motherboard::getBoard().StopProgressBar();
        interface::popScreen();
        Motherboard::getBoard().errorResponse(ERROR_TEMP_RESET_EXTERNALLY);
        Piezo::playTune(TUNE_ERROR);
        return;
      }
    }
  }
  /// if not in FILAMENT_WAIT state and the motor times out (5 minutes) alert the user
  else if(filamentTimer.hasElapsed()){
    if(startup){
      filamentState = FILAMENT_OK;
      interface::pushScreen(&filamentOK);
    }
    else {
      filamentState = FILAMENT_TIMEOUT;
      filamentTimer = Timeout();
      needsRedraw = true;
    }
    }

  
  if (forceRedraw || needsRedraw) {
    Motherboard &board = Motherboard::getBoard();
    board.setBoardStatus(Motherboard::STATUS_ONBOARD_PROCESS, true);
    lcd.setCursor(0,0);
    switch (filamentState){
      /// starting state - set hot temperature for desired tool and start heat up timer
      case FILAMENT_HEATING:
        uint16_t current_temp;
        current_temp = board.getExtruderBoard(toolID).getExtruderHeater().get_current_temperature();
        uint16_t heat_temp;
        /// don't cool the bot down if it is already hot
        heat_temp = current_temp > FILAMENT_HEAT_TEMP ? current_temp : FILAMENT_HEAT_TEMP;
        board.getExtruderBoard(toolID).getExtruderHeater().Pause(false);
        board.getExtruderBoard(toolID).getExtruderHeater().set_target_temperature(heat_temp);
        if(dual){
          current_temp = board.getExtruderBoard(1).getExtruderHeater().get_current_temperature();
          /// don't cool the bot down if it is already hot
          heat_temp = current_temp > FILAMENT_HEAT_TEMP ? current_temp : FILAMENT_HEAT_TEMP;
          board.getExtruderBoard(1).getExtruderHeater().Pause(false);
          board.getExtruderBoard(1).getExtruderHeater().set_target_temperature(heat_temp);      
        }
        /// if running the startup script, go through the explanatory text
        if(startup){
          if(dual)
            lcd.writeFromPgmspace(EXPLAIN_ONE_MSG);
          else
            lcd.writeFromPgmspace(EXPLAIN_ONE_S_MSG);
          board.interfaceBlink(25,15);
          // if z stage is at zero, move z stage down
          target = planner::getPosition();
          if(target[2] < 1000){
            target[2] = 60000;
            planner::addMoveToBuffer(target, 150);
          }
        }
        else{
          lcd.writeFromPgmspace(HEATING_BAR_MSG);
          board.StartProgressBar(3, 0, 20);
          filamentState = FILAMENT_WAIT;
        }
        filamentTimer.clear();
        filamentTimer.start(300000000); //5 minutes
        
        break;
      /// startup script explanation screen
      case FILAMENT_EXPLAIN2:
        if(dual)
          lcd.writeFromPgmspace(EXPLAIN_TWO_MSG);
        else
          lcd.writeFromPgmspace(EXPLAIN_TWO_S_MSG);
        Motherboard::getBoard().interfaceBlink(25,15);
        break;
      /// startup script explanation screen
      case FILAMENT_EXPLAIN3:
        lcd.writeFromPgmspace(EXPLAIN_THRE_MSG);
        Motherboard::getBoard().interfaceBlink(25,15);
        break;
      /// startup script explanation screen
      case FILAMENT_EXPLAIN4:
        lcd.writeFromPgmspace(EXPLAIN_FOUR_MSG);      
        Motherboard::getBoard().interfaceBlink(25,15);
        break;
      /// show heating bar status after explanations are complete
      case FILAMENT_HEAT_BAR:
        Motherboard::getBoard().StartProgressBar(3, 0, 20);
        lcd.writeFromPgmspace(HEATING_BAR_MSG);
        /// go to FILAMENT_WAIT state
        filamentState++;
        break;
      /// show heating bar status
      case FILAMENT_WAIT:
        Motherboard::getBoard().StartProgressBar(3, 0, 20);
        if(startup)
          lcd.writeFromPgmspace(HEATING_BAR_MSG);
        else
            lcd.writeFromPgmspace(HEATING_PROG_MSG);
        break;
      /// alert user that filament is ready to extrude
      case FILAMENT_START:
        if(dual){
          if(axisID == 3)
            lcd.writeFromPgmspace(READY_RIGHT_MSG);
          else
            lcd.writeFromPgmspace(READY_LEFT_MSG);
        }
        else if (startup){
          lcd.writeFromPgmspace(READY_SS_MSG);
        }else if(forward){
          lcd.writeFromPgmspace(READY_SINGLE_MSG);
        }else{
          lcd.writeFromPgmspace(READY_REV_MSG);
          filamentState++;
        } 
        Motherboard::getBoard().interfaceBlink(25,15);
        break;
      /// alert user that filament is reversing
      case FILAMENT_TUG:
        lcd.writeFromPgmspace(TUG_MSG);
        Motherboard::getBoard().interfaceBlink(25,15);
        break;
      /// alert user to press M to stop extusion / reversal
      case FILAMENT_STOP:
        if(startup)
          lcd.writeFromPgmspace(STOP_MSG_MSG);
        else{
          if(forward)
            lcd.writeFromPgmspace(STOP_EXIT_MSG);
          else 
            lcd.writeFromPgmspace(STOP_REVERSE_MSG);
        }
        Motherboard::getBoard().interfaceBlink(25,15);
        break;
      case FILAMENT_DONE:
      /// user indicated that filament has extruded
        stopMotor();
        if(startup){
          if(filamentSuccess == SUCCESS){
            if(dual && (axisID ==3)){
              axisID = 4;
              filamentState = FILAMENT_START;
              startMotor();
              lcd.writeFromPgmspace(READY_LEFT_MSG);
            }
            else
              lcd.writeFromPgmspace(FINISH_MSG);
          } else{
            if(filamentSuccess == FAIL){
            lcd.writeFromPgmspace(PUSH_HARDER_MSG);
            startMotor();
            filamentState = FILAMENT_TUG;
            } else if(filamentSuccess == SECOND_FAIL){ 
              if(dual && (axisID ==3)){
               axisID = 4;
               filamentState = FILAMENT_TUG;
               startMotor();
               lcd.writeFromPgmspace(GO_ON_LEFT_MSG);
              }
              else{
                lcd.writeFromPgmspace(KEEP_GOING_MSG);
              }
            }
          }
        }
        Motherboard::getBoard().interfaceBlink(25,15);
        break;
      case FILAMENT_EXIT:
        stopMotor();
        /// shut the heaters off if we are not in the middle of a build
        if(host::getHostState() == host::HOST_STATE_READY){
          Motherboard::getBoard().getExtruderBoard(0).getExtruderHeater().set_target_temperature(0);
          Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().set_target_temperature(0);
        }
        Motherboard::getBoard().setBoardStatus(Motherboard::STATUS_ONBOARD_PROCESS, false);
        Motherboard::getBoard().StopProgressBar();
        interface::popScreen();
        if(startup){
          host::stopBuild();
        }
        break;
      /// filament motor has been running for 5 minutes
      case FILAMENT_TIMEOUT:
        stopMotor();
        lcd.writeFromPgmspace(TIMEOUT_MSG);
        filamentState = FILAMENT_DONE;
        Motherboard::getBoard().interfaceBlink(25,15);
        break;
        }
        needsRedraw = false;
  } 
}
void FilamentScreen::setScript(FilamentScript script){
    
    filamentState = FILAMENT_HEATING;
    dual = false;
    startup = false;
    helpText = eeprom::getEeprom8(eeprom_offsets::FILAMENT_HELP_SETTINGS, 1);
    /// load settings for correct tool and direction
    switch(script){
        case FILAMENT_STARTUP_DUAL:
            dual = true;
        case FILAMENT_STARTUP_SINGLE:
      startup = true;
        case FILAMENT_RIGHT_FOR:
      toolID = 0;
            axisID = 3;
            forward = true;
            break;
        case FILAMENT_LEFT_FOR:
      toolID = 1;
            axisID = 4;
            forward = true;            
            break;
        case FILAMENT_RIGHT_REV:
      toolID = 0;
            axisID = 3;
            forward = false;
            break;
        case FILAMENT_LEFT_REV:
      toolID = 1;
            axisID = 4;
            forward = false;
            break;
    }
    
}

void FilamentScreen::notifyButtonPressed(ButtonArray::ButtonName button) {
  
  Point position;
  
  switch (button) {
    case ButtonArray::CENTER:
      if(filamentState == FILAMENT_WAIT)
        break;
            filamentState++;
            Motherboard::getBoard().interfaceBlink(0,0);
            switch (filamentState){
        /// go to interactive 'OK' scrreen
                case FILAMENT_OK:
          if(startup){
            filamentState++;
            interface::pushScreen(&filamentOK);
          }
          else{
            stopMotor();
            /// shut the heaters off if we are not in the middle of a build
            if(host::getHostState() == host::HOST_STATE_READY){
              Motherboard::getBoard().getExtruderBoard(0).getExtruderHeater().set_target_temperature(0);
              Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().set_target_temperature(0);
            }
            Motherboard::getBoard().setBoardStatus(Motherboard::STATUS_ONBOARD_PROCESS, false);
            Motherboard::getBoard().StopProgressBar();
            interface::popScreen();
          }
                    break;
                /// exit out of filament menu system
                case FILAMENT_EXIT:
          stopMotor();
          /// shut the heaters off if we are not in the middle of a build
          if(host::getHostState() == host::HOST_STATE_READY){
            Motherboard::getBoard().getExtruderBoard(0).getExtruderHeater().set_target_temperature(0);
            Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().set_target_temperature(0);
          }
          Motherboard::getBoard().setBoardStatus(Motherboard::STATUS_ONBOARD_PROCESS, false);
          Motherboard::getBoard().StopProgressBar();
          interface::popScreen();
                    break;
                default:
                    needsRedraw = true;
                    break;
            }
      break;
        case ButtonArray::LEFT:
      Motherboard::getBoard().StopProgressBar();
      interface::pushScreen(&cancel_build_menu);      
            break;      
        case ButtonArray::RIGHT:
        case ButtonArray::DOWN:
        case ButtonArray::UP:
      break;
            
  }
}

void FilamentScreen::reset() {
    needsRedraw = false;
    filamentState=FILAMENT_HEATING;
    filamentSuccess = SUCCESS;
    filamentTimer = Timeout();
    cancel_process = false;
}

ReadyMenu::ReadyMenu() {
  itemCount = 4;
  reset();
}

void ReadyMenu::resetState() {
  itemIndex = 2;
  firstItemIndex = 2;
}

void ReadyMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd, uint8_t line_number) {
    
  switch (index) {
        case 0:
            lcd.writeFromPgmspace(READY1_MSG);
            break;
        case 1:
            lcd.writeFromPgmspace(READY2_MSG);
            break;
        case 2:
            lcd.writeFromPgmspace(YES_MSG);
            break;
        case 3:
            lcd.writeFromPgmspace(NO_MSG);
            break;
  }
}

void ReadyMenu::handleSelect(uint8_t index) {

  switch (index) {
        case 2:
            interface::popScreen();
            break;
        case 3:
            // set this as a flag to the welcome menu that the bot is not ready to print
            ready_fail = true;
            interface::popScreen();
            break;
  }
}

LevelOKMenu::LevelOKMenu() {
  itemCount = 4;
  reset();
}

void LevelOKMenu::resetState() {
  itemIndex = 2;
  firstItemIndex = 2;
}

void LevelOKMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd, uint8_t line_number) {
    
  switch (index) {
        case 0:
            lcd.writeFromPgmspace(NOZZLE_MSG_MSG);
            break;
        case 1:
            lcd.writeFromPgmspace(HEIGHT_CHK_MSG);
            break;
        case 2:
            lcd.writeFromPgmspace(HEIGHT_GOOD_MSG);
            break;
        case 3:
            lcd.writeFromPgmspace(TRY_AGAIN_MSG);
            break;
  }
}

void LevelOKMenu::handleSelect(uint8_t index) {

  switch (index) {
    case 2:
      levelSuccess = SUCCESS;
      interface::popScreen();
      break;
    case 3:
      // set this as a flag to the welcome menu that the bot is not ready to print
      if(levelSuccess == FAIL)
        levelSuccess = SECOND_FAIL;
      else
        levelSuccess = FAIL;
      interface::popScreen();
      break;
  }
}

FilamentOKMenu::FilamentOKMenu() {
  itemCount = 4;
  reset();
}

void FilamentOKMenu::resetState() {
  itemIndex = 2;
  firstItemIndex = 2;
}

void FilamentOKMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd, uint8_t line_number) {
    
  switch (index) {
        case 0:
            lcd.writeFromPgmspace(QONE_MSG);
            break;
        case 1:
            lcd.writeFromPgmspace(QTWO_MSG);
            break;
        case 2:
            lcd.writeFromPgmspace(YES_MSG);
            break;
        case 3:
            lcd.writeFromPgmspace(NO_MSG);
            break;
  }
}

void FilamentOKMenu::handleSelect(uint8_t index) {
  switch (index) {
        case 2:
      filamentSuccess = SUCCESS;
            interface::popScreen();
            break;
        case 3:
            // set this as a flag to the welcome menu that the bot is not ready to print
            if(filamentSuccess == FAIL)
        filamentSuccess = SECOND_FAIL;
      else
        filamentSuccess = FAIL;
            interface::popScreen();
  }
}

FilamentMenu::FilamentMenu() {
  itemCount = 4;
  reset();
}

void FilamentMenu::resetState() {
    singleTool = eeprom::isSingleTool();
    if(singleTool)
        itemCount = 2;
    else
    itemCount = 4;
  itemIndex = 0;
  firstItemIndex = 0;
}

void FilamentMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd, uint8_t line_number) {
    
    singleTool = eeprom::isSingleTool();
    
  switch (index) {
        case 0:
            if(!singleTool){
                lcd.writeFromPgmspace(LOAD_RIGHT_MSG);
            } else {
                lcd.writeFromPgmspace(LOAD_SINGLE_MSG);
            }
            break;
        case 1:
            if(!singleTool){
                lcd.writeFromPgmspace(UNLOAD_RIGHT_MSG);
            } else {
                lcd.writeFromPgmspace(UNLOAD_SINGLE_MSG);
            }
            break;
        case 2:
            if(!singleTool){
                lcd.writeFromPgmspace(LOAD_LEFT_MSG);
            }
            break;
        case 3:
            if(!singleTool){
                lcd.writeFromPgmspace(UNLOAD_LEFT_MSG);
            }
            break;
  }
    
}

void FilamentMenu::handleSelect(uint8_t index) {
    
  switch (index) {
        case 0:
            filamentScreen.setScript(FILAMENT_RIGHT_FOR);
            interface::pushScreen(&filamentScreen);
            break;
        case 1:
            filamentScreen.setScript(FILAMENT_RIGHT_REV);
            interface::pushScreen(&filamentScreen);
            break;
        case 2:
            filamentScreen.setScript(FILAMENT_LEFT_FOR);
            interface::pushScreen(&filamentScreen);
            break;
        case 3:
            filamentScreen.setScript(FILAMENT_LEFT_REV);
            interface::pushScreen(&filamentScreen);
            break;
  }
}



bool MessageScreen::screenWaiting(void){
  return (timeout.isActive() || incomplete || waiting_for_user);
}

void MessageScreen::addMessage(CircularBuffer& buf) {
  char c = buf.pop();
  while (c != '\0' && cursor < BUF_SIZE && buf.getLength() > 0) {
    message[cursor++] = c;
    c = buf.pop();
  }
  // ensure that message is always null-terminated
  if (cursor == BUF_SIZE-1) {
    message[BUF_SIZE-1] = '\0';
  } else {
    message[cursor] = '\0';
  }
}


void MessageScreen::addMessage(char msg[]) {

  char* letter = msg;
  while (*letter != 0) {
    message[cursor++] = *letter;
    letter++;
  }
    
  // ensure that message is always null-terminated
  if (cursor == BUF_SIZE) {
    message[BUF_SIZE-1] = '\0';
  } else {
    message[cursor] = '\0';
  }
}

void MessageScreen::WaitForUser(bool wait){
  waiting_for_user = wait;
}

void MessageScreen::clearMessage() {
  x = y = 0;
  message[0] = '\0';
  cursor = 0;
  needsRedraw = false;
  timeout = Timeout();
  incomplete = false;
  waiting_for_user = false;
}

void MessageScreen::setTimeout(uint8_t seconds) {
  timeout.start((micros_t)seconds * 1000L * 1000L);
}
void MessageScreen::refreshScreen(){
  needsRedraw = true;
}

void MessageScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
  char* b = message;
  int ycursor = y;
  if (timeout.hasElapsed()) {
    interface::popScreen();
    return;
  }
  if (forceRedraw || needsRedraw) {
    needsRedraw = false;
    lcd.clear();

    while (*b != '\0') {
      lcd.setCursor(x, ycursor);
      b = lcd.writeLine(b);
      if (*b == '\n') {
        b++;
        ycursor++;
      }
    }
  }
}

void MessageScreen::reset() {
  //timeout = Timeout();
}

void MessageScreen::notifyButtonPressed(ButtonArray::ButtonName button) {
  host::HostState state;

  switch (button) {
    case ButtonArray::CENTER:
      break;
    case ButtonArray::LEFT:
        state = host::getHostState();
        if((state == host::HOST_STATE_BUILDING_ONBOARD) ||
                (state == host::HOST_STATE_BUILDING) ||
            (state == host::HOST_STATE_BUILDING_FROM_SD)){
                interface::pushScreen(&cancel_build_menu);
        }
    case ButtonArray::RIGHT:
    case ButtonArray::DOWN:
    case ButtonArray::UP:
      break;

  }
}

void JogMode::reset() {
  jogDistance = DISTANCE_LONG;
  distanceChanged = modeChanged = false;
  JogModeScreen = JOG_MODE_X;
}


void JogMode::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
  
  


  if (forceRedraw || distanceChanged || modeChanged) {
    
    Motherboard::getBoard().setBoardStatus(Motherboard::STATUS_MANUAL_MODE, true);
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.writeFromPgmspace(JOG1_MSG);

    switch (JogModeScreen){
      case JOG_MODE_X:
        lcd.setCursor(0,1);
        lcd.writeFromPgmspace(JOG2X_MSG);

        lcd.setCursor(0,2);
        lcd.writeFromPgmspace(JOG3X_MSG);

        lcd.setCursor(0,3);
        lcd.writeFromPgmspace(JOG4X_MSG);
        break;
      case JOG_MODE_Y:
        lcd.setCursor(0,1);
        lcd.writeFromPgmspace(JOG2Y_MSG);

        lcd.setCursor(0,2);
        lcd.writeFromPgmspace(JOG3Y_MSG);

        lcd.setCursor(0,3);
        lcd.writeFromPgmspace(JOG4Y_MSG);
        break;
      case JOG_MODE_Z:
        lcd.setCursor(0,1);
        lcd.writeFromPgmspace(JOG2Z_MSG);

        lcd.setCursor(0,2);
        lcd.writeFromPgmspace(JOG3Z_MSG);

        lcd.setCursor(0,3);
        lcd.writeFromPgmspace(JOG4Z_MSG);
        break;
    }

    distanceChanged = false;
    modeChanged = false;
  }
}

void JogMode::jog(ButtonArray::ButtonName direction) {
  planner::abort();
  Point position = planner::getPosition();  

  int32_t interval = 1000;
  int32_t steps;

  switch(jogDistance) {
  case DISTANCE_SHORT:
    steps = 20;
    break;
  case DISTANCE_LONG:
    steps = 1000;
    break;
  }

  if(JogModeScreen == JOG_MODE_X)
  {
    switch(direction) {
      case ButtonArray::RIGHT:
      JogModeScreen = JOG_MODE_Y;
      modeChanged = true;
      break;
      case ButtonArray::DOWN:
      position[0] -= steps;
      break;
      case ButtonArray::UP:
      position[0] += steps;
      break;
    }
  }
  else if (JogModeScreen == JOG_MODE_Y)
  {
    switch(direction) {
      case ButtonArray::RIGHT:
      JogModeScreen = JOG_MODE_Z;
      modeChanged = true;
      break;
      case ButtonArray::LEFT:
      JogModeScreen = JOG_MODE_X;
      modeChanged = true;
      break;
      case ButtonArray::DOWN:
      position[1] -= steps;
      break;
      case ButtonArray::UP:
      position[1] += steps;
      break;
    }
      
  }
  else if (JogModeScreen == JOG_MODE_Z)
  {
    switch(direction) {
      case ButtonArray::LEFT:
      JogModeScreen = JOG_MODE_Y;
      modeChanged = true;
      break;
      case ButtonArray::DOWN:
      position[2] += steps;
      break;
      case ButtonArray::UP:
      position[2] -= steps;
      break;
    }
  }

  planner::addMoveToBuffer(position, interval);
}

void JogMode::notifyButtonPressed(ButtonArray::ButtonName button) {
  switch (button) {
    case ButtonArray::CENTER:
           interface::popScreen();
           Motherboard::getBoard().setBoardStatus(Motherboard::STATUS_MANUAL_MODE, false);
           for(int i = 0; i < STEPPER_COUNT; i++)
      steppers::enableAxis(i, false);
    break;
        case ButtonArray::LEFT:
        case ButtonArray::RIGHT:
        case ButtonArray::DOWN:
        case ButtonArray::UP:
    jog(button);
    break;
  }
}


void SnakeMode::update(LiquidCrystalSerial& lcd, bool forceRedraw) {

  // If we are dead, restart the game.
  if (!snakeAlive) {
    reset();
    forceRedraw = true;
  }

  if (forceRedraw) {
    lcd.clear();

    for (uint8_t i = 0; i < snakeLength; i++) {
      lcd.setCursor(snakeBody[i].x, snakeBody[i].y);
      lcd.write('O');
    }
  }

  // Always redraw the apple, just in case.
  lcd.setCursor(applePosition.x, applePosition.y);
  lcd.write('*');

  // First, undraw the snake's tail
  lcd.setCursor(snakeBody[snakeLength-1].x, snakeBody[snakeLength-1].y);
  lcd.write(' ');

  // Then, shift the snakes body parts back, deleting the tail
  for(int8_t i = snakeLength-1; i >= 0; i--) {
    snakeBody[i+1] = snakeBody[i];
  }

  // Create a new head for the snake (this causes it to move forward)
  switch(snakeDirection)
  {
  case DIR_EAST:
    snakeBody[0].x = (snakeBody[0].x + 1) % LCD_SCREEN_WIDTH;
    break;
  case DIR_WEST:
    snakeBody[0].x = (snakeBody[0].x +  LCD_SCREEN_WIDTH - 1) % LCD_SCREEN_WIDTH;
    break;
  case DIR_NORTH:
    snakeBody[0].y = (snakeBody[0].y + LCD_SCREEN_HEIGHT - 1) % LCD_SCREEN_HEIGHT;
    break;
  case DIR_SOUTH:
    snakeBody[0].y = (snakeBody[0].y + 1) % LCD_SCREEN_HEIGHT;
    break;
  }

  // Now, draw the snakes new head
  lcd.setCursor(snakeBody[0].x, snakeBody[0].y);
  lcd.write('O');

  // Check if the snake has run into itself
  for (uint8_t i = 1; i < snakeLength; i++) {
    if (snakeBody[i].x == snakeBody[0].x
      && snakeBody[i].y == snakeBody[0].y) {
      snakeAlive = false;

      lcd.setCursor(1,1);
      lcd.writeFromPgmspace(GAMEOVER_MSG);
      updateRate = 5000L * 1000L;
    }
  }

  // If the snake just ate an apple, increment count and make new apple
  if (snakeBody[0].x == applePosition.x
      && snakeBody[0].y == applePosition.y) {
    applesEaten++;

    if(applesEaten % APPLES_BEFORE_GROW == 0) {
      snakeLength++;
      updateRate -= 5L * 1000L;
    }

    applePosition.x = rand()%LCD_SCREEN_WIDTH;
    applePosition.y = rand()%LCD_SCREEN_HEIGHT;

    lcd.setCursor(applePosition.x, applePosition.y);
    lcd.write('*');
  }
}

void SnakeMode::reset() {
  updateRate = 150L * 1000L;
  snakeDirection = DIR_EAST;
  snakeLength = 3;
  applesEaten = 0;
  snakeAlive = true;

  // Put the snake in an initial position
  snakeBody[0].x = 2; snakeBody[0].y = 1;
  snakeBody[1].x = 1; snakeBody[1].y = 1;
  snakeBody[2].x = 0; snakeBody[2].y = 1;

  // Put the apple in an initial position (this could collide with the snake!)
  applePosition.x = rand()%LCD_SCREEN_WIDTH;
  applePosition.y = rand()%LCD_SCREEN_HEIGHT;
  
  
}


void SnakeMode::notifyButtonPressed(ButtonArray::ButtonName button) {
  switch (button) {
        case ButtonArray::DOWN:
    snakeDirection = DIR_SOUTH;
    break;
        case ButtonArray::UP:
    snakeDirection = DIR_NORTH;
    break;
        case ButtonArray::LEFT:
    snakeDirection = DIR_WEST;
    break;
        case ButtonArray::RIGHT:
    snakeDirection = DIR_EAST;
    break;
        case ButtonArray::CENTER:
                interface::popScreen();
    break;
  }
}


void MonitorMode::reset() {
  updatePhase = 0;
  buildPercentage = 101;
  singleTool = eeprom::isSingleTool();
  heating = false;
  hasHBP = eeprom::hasHBP();
  if(hasHBP){
    num_update_phases = 6;
  }else{
    num_update_phases = 4;
  }  
}

void MonitorMode::setBuildPercentage(uint8_t percent){
  
  buildPercentage = percent;
}

#define abs(X) ((X) < 0 ? -(X) : (X)) 

void MonitorMode::update(LiquidCrystalSerial& lcd, bool forceRedraw) {

  Motherboard& board = Motherboard::getBoard();
    
  if(!heating && board.isHeating()){
    heating = true;
    lcd.setCursor(0,0);
    lcd.writeFromPgmspace(HEATING_SPACES_MSG);
    board.StartProgressBar(0,8, 20);
  }
   
  char * name;
  if (forceRedraw) {
                
    lcd.clear();
    lcd.setCursor(0,0);
    if(heating){
      lcd.writeFromPgmspace(HEATING_MSG);
      board.StartProgressBar(0,8, 20);
    }
    else{
      RGB_LED::setDefaultColor();
      switch(host::getHostState()) {
            case host::HOST_STATE_READY:
            case host::HOST_STATE_BUILDING_ONBOARD:
                lcd.writeString(host::getMachineName());
                break;
            case host::HOST_STATE_BUILDING:
            case host::HOST_STATE_BUILDING_FROM_SD:
                name = host::getBuildName();
                int8_t name_length;
                //limit name length to 19, which is 15+'.s3g' 
                // we remove the .s3g and we should only print 15 characters of the name
                name_length = strlen(name);
                if (name_length > 19) name_length = 19;
                // assume the last 4 characters are '.s3g' and don't print those
                while(name_length-- > 4)
                    lcd.write(*name++);
                if(!((name[0] == '.') && (name[1] == 's') && (name[2] == '3') && (name[3] == 'g'))){
                    while(name_length-- >= 0)
                      lcd.write(*name++);
                }
                    
                lcd.setCursor(16,0);
                lcd.writeFromPgmspace(BUILD_PERCENT_MSG);
                
                break;
            case host::HOST_STATE_ERROR:
                Piezo::playTune(TUNE_ERROR);
                lcd.writeFromPgmspace(ERROR_MSG);
                break;
            }
      }

      if(singleTool){
          lcd.setCursor(0,1);
          lcd.writeFromPgmspace(CLEAR_MSG);
          
          lcd.setCursor(0,2);
          lcd.writeFromPgmspace(EXTRUDER_TEMP_MSG);
      }else{
          lcd.setCursor(0,1);
          lcd.writeFromPgmspace(EXTRUDER1_TEMP_MSG);
          
          lcd.setCursor(0,2);
          lcd.writeFromPgmspace(EXTRUDER2_TEMP_MSG);
      }

      if(hasHBP){
        lcd.setCursor(0,3);
        lcd.writeFromPgmspace(PLATFORM_TEMP_MSG);
      }
  }

  if(heating){
    if(!board.isHeating()){
            heating = false;
            board.StopProgressBar();
            //redraw build name
            lcd.setCursor(0,0);
            lcd.writeFromPgmspace(CLEAR_MSG);
            lcd.setCursor(0,0);
            switch(host::getHostState()) {
                case host::HOST_STATE_READY:
                case host::HOST_STATE_BUILDING_ONBOARD:
                    lcd.writeString(host::getMachineName());
                    break;
                case host::HOST_STATE_BUILDING:
                case host::HOST_STATE_BUILDING_FROM_SD:
                    name = host::getBuildName();
                    int8_t name_length;
                    //limit name length to 19, which is 15+'.s3g' 
                    // we remove the .s3g and we should only print 15 characters of the name
                    name_length = strlen(name);
                    if (name_length > 19) name_length = 19;
                    // assume the last 4 characters are '.s3g' and don't print those
                    while(name_length-- > 4)
                        lcd.write(*name++);
                    if(!((name[0] == '.') && (name[1] == 's') && (name[2] == '3') && (name[3] == 'g'))){
                        while(name_length-- >= 0)
                          lcd.write(*name++);
                    }
                    
                    lcd.setCursor(16,0);
                    lcd.writeFromPgmspace(BUILD_PERCENT_MSG);
                    break;
            }
            RGB_LED::setDefaultColor();
        }
    }
    
    uint16_t data;
    
  // Redraw tool info
  switch (updatePhase) {
  case 1:
      if(!singleTool){
          lcd.setCursor(12,1);
      data = board.getExtruderBoard(0).getExtruderHeater().get_current_temperature();
      if(board.getExtruderBoard(0).getExtruderHeater().has_failed()){
        lcd.writeFromPgmspace(NA_MSG);
      } else if(board.getExtruderBoard(0).getExtruderHeater().isPaused()){
        lcd.writeFromPgmspace(WAITING_MSG);
      } else
        lcd.writeInt(data,3);
      }
    break;

  case 2:
    if(!singleTool){
       if(!board.getExtruderBoard(0).getExtruderHeater().has_failed() && !board.getExtruderBoard(0).getExtruderHeater().isPaused()){           
          data = board.getExtruderBoard(0).getExtruderHeater().get_set_temperature();
          if(data > 0){
            lcd.setCursor(15,1);
            lcd.writeFromPgmspace(ON_CELCIUS_MSG);
            lcd.setCursor(16,1);
            lcd.writeInt(data,3);
          }else{
            lcd.setCursor(15,1);
            lcd.writeFromPgmspace(CELCIUS_MSG);
          }
      }        
    }
    break;
  case 3:
      lcd.setCursor(12,2);
      data = board.getExtruderBoard(!singleTool * 1).getExtruderHeater().get_current_temperature();

      if(board.getExtruderBoard(!singleTool * 1).getExtruderHeater().has_failed()){
        lcd.writeFromPgmspace(NA_MSG);
      } else if(board.getExtruderBoard(!singleTool * 1).getExtruderHeater().isPaused()){
        lcd.writeFromPgmspace(WAITING_MSG);
      } 
      else{
        lcd.writeInt(data,3);
      }
      break;
  case 4:
        if(!board.getExtruderBoard(!singleTool * 1).getExtruderHeater().has_failed() && !board.getExtruderBoard(!singleTool * 1).getExtruderHeater().isPaused()){
            lcd.setCursor(16,2);
            data = board.getExtruderBoard(!singleTool * 1).getExtruderHeater().get_set_temperature();
            if(data > 0){
                lcd.setCursor(15,2);
                lcd.writeFromPgmspace(ON_CELCIUS_MSG);
                lcd.setCursor(16,2);
                lcd.writeInt(data,3);
            }else{
                lcd.setCursor(15,2);
                lcd.writeFromPgmspace(CELCIUS_MSG);
            }
        }
        break;

  case 5:
      lcd.setCursor(12,3);
      data = board.getPlatformHeater().get_current_temperature();
      if(board.getPlatformHeater().has_failed()){
        lcd.writeFromPgmspace(NA_MSG);
      } else if (board.getPlatformHeater().isPaused()){
        lcd.writeFromPgmspace(WAITING_MSG);
      } else {
        lcd.writeInt(data,3);
      }
      break;

  case 6:
        if(!board.getPlatformHeater().has_failed() && !board.getPlatformHeater().isPaused()){
            lcd.setCursor(16,3);
            data = board.getPlatformHeater().get_set_temperature();
            if(data > 0){
                lcd.setCursor(15,3);
                lcd.writeFromPgmspace(ON_CELCIUS_MSG);
                lcd.setCursor(16,3);
                lcd.writeInt(data,3);
            }
            else{
                lcd.setCursor(15,3);
                lcd.writeFromPgmspace(CELCIUS_MSG);
            }
        }
    break;
  case 0:
    host::HostState state;
    state = host::getHostState();
    if(!heating && ((state == host::HOST_STATE_BUILDING) || (state == host::HOST_STATE_BUILDING_FROM_SD)))
    {
      if(buildPercentage < 100)
      {
        lcd.setCursor(17,0);
        lcd.writeInt(buildPercentage,2);
      }
      else if(buildPercentage == 100)
      {
        lcd.setCursor(16,0);
        lcd.writeFromPgmspace(DONE_MSG);
      }
    }
    break;
  }

  updatePhase++;
  if (updatePhase > num_update_phases) {
    updatePhase = 0;
  }
}

void MonitorMode::notifyButtonPressed(ButtonArray::ButtonName button) {
  switch (button) {
        case ButtonArray::CENTER:
        case ButtonArray::LEFT:
            switch(host::getHostState()) {
            case host::HOST_STATE_BUILDING:
            case host::HOST_STATE_BUILDING_FROM_SD:
        interface::pushScreen(&active_build_menu);
        break;
            case host::HOST_STATE_BUILDING_ONBOARD:
                interface::pushScreen(&cancel_build_menu);
                break;
            default:
        Motherboard::getBoard().StopProgressBar();
                interface::popScreen();
                break;
            }
  }
}


void Menu::update(LiquidCrystalSerial& lcd, bool forceRedraw) {

  /// write the non-menu item lines
  if (forceRedraw || needsRedraw){
    // Redraw the whole menu
    lcd.clear();
    for (uint8_t i = 0; i < firstItemIndex; i++) {
      lcd.setCursor(1,i);
      drawItem(i, lcd, i); 
    }
  }
  
  // Do we need to redraw the whole menu?
  if((!sliding_menu && (itemIndex/LCD_SCREEN_HEIGHT) != (lastDrawIndex/LCD_SCREEN_HEIGHT)) ||
      (zeroIndex != lastZeroIndex) || forceRedraw || needsRedraw){
        
    if(!sliding_menu){
      lcd.clear();
    }

    for (uint8_t i = firstItemIndex; i < LCD_SCREEN_HEIGHT; i++) {
      // Instead of using lcd.clear(), clear one line at a time so there
      // is less screen flickr.

      if ((zeroIndex == 0) && ( i+(itemIndex/LCD_SCREEN_HEIGHT)*LCD_SCREEN_HEIGHT +1 > itemCount)) {
        break;
      }

      lcd.setCursor(1,i);
      if(sliding_menu){
        // Draw one page of items at a time
        drawItem(i + zeroIndex, lcd, i);
      }else{
        drawItem(i+(itemIndex/LCD_SCREEN_HEIGHT)*LCD_SCREEN_HEIGHT, lcd, i);
      }
    }
    cursorUpdate = true;
  }
  else if (lineUpdate){
    lcd.setCursor(1,((itemIndex-zeroIndex)%LCD_SCREEN_HEIGHT));
    drawItem(itemIndex, lcd, (itemIndex-zeroIndex)%LCD_SCREEN_HEIGHT);
  }
  if (cursorUpdate){
    // Only need to clear the previous cursor
    lcd.setCursor(0,((lastDrawIndex-zeroIndex)%LCD_SCREEN_HEIGHT));
    lcd.write(' ');
  
    lcd.setCursor(0,((itemIndex-zeroIndex)%LCD_SCREEN_HEIGHT));
    if((((itemIndex-zeroIndex)%LCD_SCREEN_HEIGHT) == (LCD_SCREEN_HEIGHT - 1)) && (itemIndex < itemCount-1))
      lcd.write(9); //special char "down"
    else if((((itemIndex-zeroIndex)%LCD_SCREEN_HEIGHT) == firstItemIndex) && (itemIndex> firstItemIndex))
      lcd.write('^');
    else    
      lcd.write(8); //special char "right"
  }

  lastDrawIndex = itemIndex;
  lastZeroIndex = zeroIndex;
  lineUpdate = false;
  cursorUpdate = false;
    needsRedraw = false;
}

void Menu::reset() {
  selectMode = false;
    selectIndex = -1;
  firstItemIndex = 0;
  itemIndex = 0;
  lastDrawIndex = 255;
  lineUpdate = false;
  sliding_menu = true;
  resetState();
    needsRedraw = false;
    cursorUpdate = true;
    zeroIndex = 0;
}

void Menu::resetState() {
  cursorUpdate = true;
}

void Menu::handleSelect(uint8_t index) {
}

void Menu::handleCancel() {
  // Remove ourselves from the menu list
        interface::popScreen();
}

void Menu::notifyButtonPressed(ButtonArray::ButtonName button) {
   switch (button) {
        case ButtonArray::CENTER:
            if((itemIndex < MAX_INDICES) && counter_item[itemIndex]){
                selectMode = !selectMode;
                
        if(selectMode){
          selectIndex = itemIndex;
          lineUpdate = true;
        }
        else{
          selectIndex = -1;
          handleSelect(itemIndex);
          lineUpdate = true;
        }
      }else{
        handleSelect(itemIndex);
      }
            break;
        case ButtonArray::LEFT:
      if(!selectMode)
        interface::popScreen();
      break;
        case ButtonArray::RIGHT:
            break;
        case ButtonArray::UP:
            if(selectMode){
                handleCounterUpdate(itemIndex, true);
                lineUpdate = true;
            }
            // decrement index
            else{
                if (itemIndex > firstItemIndex) {
                    itemIndex--;
                    if(sliding_menu && ((itemIndex - zeroIndex + 1)%LCD_SCREEN_HEIGHT == firstItemIndex) && (zeroIndex > 0)){
            zeroIndex--;
          } 
                    cursorUpdate = true;
                }}
            break;
        case ButtonArray::DOWN:
            if(selectMode){
                handleCounterUpdate(itemIndex, false);
                lineUpdate = true;
            }
            // increment index
            else{    
                if (itemIndex < itemCount - 1) {
                    itemIndex++;
                    if(sliding_menu && ((itemIndex - zeroIndex)%LCD_SCREEN_HEIGHT == LCD_SCREEN_HEIGHT-1) && (zeroIndex < itemCount - LCD_SCREEN_HEIGHT)){
            zeroIndex++;
          } 
                    cursorUpdate = true;
                }}
            break;
  }
}

PreheatSettingsMenu::PreheatSettingsMenu() {
  itemCount = 4;
  reset();
  counter_item[0] = 0;
    for (uint8_t i = 1; i < itemCount; i++){
    counter_item[i] = 1;
  }
}   
void PreheatSettingsMenu::resetState(){
  
  singleTool = eeprom::isSingleTool();
   
  if(singleTool){
    itemIndex = 2;
    firstItemIndex = 2;
  }else{
    itemIndex = 1;
    firstItemIndex = 1;
  }
  if(eeprom::hasHBP()){
    itemCount = 4;
  }else{
    itemCount = 3;
  }
    
  counterRight = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_RIGHT_OFFSET, 230);
  counterLeft = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_LEFT_OFFSET, 230);
  counterPlatform = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_PLATFORM_OFFSET, 110);
    
}

void PreheatSettingsMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd, uint8_t line_number) {
    
  switch (index) {
        case 0:
            lcd.writeFromPgmspace(PREHEAT_SET_MSG);
            break;
        case 1:
            if(!singleTool){
                lcd.writeFromPgmspace(RIGHT_SPACES_MSG);
                if(selectIndex == 1){
                    lcd.setCursor(14,line_number);
                    lcd.writeFromPgmspace(ARROW_MSG);
                }
                lcd.setCursor(17,line_number);
                lcd.writeInt(counterRight,3);
            }
            break;
        case 2:
            if(singleTool){
                lcd.writeFromPgmspace(RIGHT_SPACES_MSG);
                lcd.setCursor(17, line_number);
                lcd.writeInt(counterRight,3);
            }else{
                lcd.writeFromPgmspace(LEFT_SPACES_MSG);
                lcd.setCursor(17,line_number);
                lcd.writeInt(counterLeft,3);
            }
            if(selectIndex == 2){
                lcd.setCursor(14,line_number);
                lcd.writeFromPgmspace(ARROW_MSG);
            }
            break;
         case 3:
            lcd.writeFromPgmspace(PLATFORM_SPACES_MSG);
            if(selectIndex == 3){
                lcd.setCursor(14,line_number);
                lcd.writeFromPgmspace(ARROW_MSG);
            }
            lcd.setCursor(17,line_number);
            lcd.writeInt(counterPlatform,3);
            break;
            
  }
}
void PreheatSettingsMenu::handleCounterUpdate(uint8_t index, bool up){
    switch (index) {
        case 1:
            // update right counter
            if(up)
                counterRight++;
            else
                counterRight--;
            if(counterRight > 260){
                counterRight = 260;
            } if(counterRight < 0){
        counterRight == 0;
      }
            break;
        case 2:
            if(singleTool){
                // update right counter
                if(up)
                    counterRight++;
                else
                    counterRight--;
                if(counterRight > 260){
                    counterRight = 260; 
                }if(counterRight < 0){
          counterRight == 0;
      } 
            }else{
                // update left counter
                if(up)
                    counterLeft++;
                else
                    counterLeft--;
                if(counterLeft > 260){
                    counterLeft = 260;
                }if(counterLeft < 0){
          counterLeft == 0;
      }
            }
            break;
        case 3:
            // update platform counter
            if(up)
                counterPlatform++;
            else
                counterPlatform--;
            if(counterPlatform > 120){
                counterPlatform = 120;
            } if(counterPlatform < 0){
        counterPlatform == 0;
      }
            break;
  }
    
}

void PreheatSettingsMenu::handleSelect(uint8_t index) {
  switch (index) {
        case 1:
            // store right tool setting
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
              eeprom_write_word((uint16_t*)(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_RIGHT_OFFSET), counterRight);
            }
            break;
        case 2:
            if(singleTool){
                // store right tool setting
                ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
                  eeprom_write_word((uint16_t*)(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_RIGHT_OFFSET), counterRight);
                }
            }else{
                // store left tool setting
                ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
                  eeprom_write_word((uint16_t*)(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_LEFT_OFFSET), counterLeft);
                }
            }
            break;
        case 3:
            // store platform setting
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
              eeprom_write_word((uint16_t*)(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_PLATFORM_OFFSET), counterPlatform);
            }
            break;
  }
}


ResetSettingsMenu::ResetSettingsMenu() {
  itemCount = 4;
  reset();
}

void ResetSettingsMenu::resetState() {
  itemIndex = 2;
  firstItemIndex = 2;
}

void ResetSettingsMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd, uint8_t line_number) {
    
  switch (index) {
        case 0:
            lcd.writeFromPgmspace(RESET1_MSG);
            break;
        case 1:
            lcd.writeFromPgmspace(RESET2_MSG);
            break;
        case 2:
            lcd.writeFromPgmspace(NO_MSG);
            break;
        case 3:
            lcd.writeFromPgmspace(YES_MSG);
            break;
  }
}

void ResetSettingsMenu::handleSelect(uint8_t index) {
  switch (index) {
        case 2:
            // Don't reset settings, just close dialog.
            interface::popScreen();
            break;
        case 3:
            // Reset setings to defaults
            eeprom::setDefaultSettings();
            RGB_LED::setDefaultColor();
            interface::popScreen();
            break;
  }
}


ActiveBuildMenu::ActiveBuildMenu(){
#ifdef ACTIVE_COOLING_FAN
  itemCount = 8;
#else
  itemCount = 7;
#endif  
  reset();
  for (uint8_t i = 0; i < itemCount; i++){
    counter_item[i] = 0;
  }
  counter_item[5] = 1;
}
    
void ActiveBuildMenu::resetState(){
  LEDColor = eeprom::getEeprom8(eeprom_offsets::LED_STRIP_SETTINGS, 0);
  itemIndex = 0;
  firstItemIndex = 0;
  is_paused = false;
  is_sleeping = command::isActivePaused();
  FanOn = EX_FAN.getValue();
}

void ActiveBuildMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd, uint8_t line_number){
  
  switch (index) {
        case 2:
            lcd.writeFromPgmspace(STATS_MSG);
            break;
        case 0:
            if(is_paused){
              lcd.writeFromPgmspace(UNPAUSE_MSG);
            }else {
              lcd.writeFromPgmspace(PAUSE_MSG);
            }
            break;
        case 1:
            lcd.writeFromPgmspace(CANCEL_BUILD_MSG);
            break;
        case 4:
            lcd.writeFromPgmspace(CHANGE_FILAMENT_MSG);
            break;
        case 3:
            if(!is_sleeping){
              lcd.writeFromPgmspace(SLEEP_MSG);
            }else {
              lcd.writeFromPgmspace(RESTART_MSG);
            }
            break;
        case 5:
          lcd.writeFromPgmspace(LED_MSG);
          lcd.setCursor(11,line_number);
          if(selectIndex == 5)
            lcd.writeFromPgmspace(ARROW_MSG);
          else
            lcd.writeFromPgmspace(NO_ARROW_MSG);
          lcd.setCursor(14,line_number);
          switch(LEDColor){
              case LED_DEFAULT_RED:
                  lcd.writeFromPgmspace(RED_COLOR_MSG);
                  break;
              case LED_DEFAULT_ORANGE:
                  lcd.writeFromPgmspace(ORANGE_COLOR_MSG);
                  break;
              case LED_DEFAULT_PINK:
                  lcd.writeFromPgmspace(PINK_COLOR_MSG);
                  break;
              case LED_DEFAULT_GREEN:
                  lcd.writeFromPgmspace(GREEN_COLOR_MSG);
                  break;
              case LED_DEFAULT_BLUE:
                  lcd.writeFromPgmspace(BLUE_COLOR_MSG);
                  break;
              case LED_DEFAULT_PURPLE:
                  lcd.writeFromPgmspace(PURPLE_COLOR_MSG);
                  break;
              case LED_DEFAULT_WHITE:
                  lcd.writeFromPgmspace(WHITE_COLOR_MSG);
                  break;
              case LED_DEFAULT_CUSTOM:
                  lcd.writeFromPgmspace(CUSTOM_COLOR_MSG);
                  break;
              case LED_DEFAULT_OFF:
                  lcd.writeFromPgmspace(OFF_COLOR_MSG);
                  break;
            }
            break;
#ifdef ACTIVE_COOLING_FAN
          case 6:
            lcd.writeFromPgmspace(ACTIVE_FAN_MSG);
            lcd.setCursor(14, line_number);
            if(FanOn){
              lcd.writeFromPgmspace(ON_MSG);
            }else{
              lcd.writeFromPgmspace(OFF_MSG);
            }
            break;
          case 7:
#else
          case 6:
#endif
            lcd.writeFromPgmspace(BACK_TO_MONITOR_MSG);
            break;
  }
}

void ActiveBuildMenu::pop(void){
  host::pauseBuild(false);
}

void ActiveBuildMenu::handleCounterUpdate(uint8_t index, bool up){
  
  switch (index){
    case 5:
      // update left counter
      if(up)
          LEDColor++;
      else
          LEDColor--;
      // keep within appropriate boundaries
      if(LEDColor > 7)
          LEDColor = 0;
      else if(LEDColor < 0)
      LEDColor = 7;
      
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        eeprom_write_byte((uint8_t*)eeprom_offsets::LED_STRIP_SETTINGS, LEDColor);
      }
      RGB_LED::setDefaultColor(); 

      break;
  }
}
    
void ActiveBuildMenu::handleSelect(uint8_t index){
  
  switch (index) {
    case 2:
      interface::pushScreen(&build_stats_screen);
      break;
    case 4:
      host::pauseBuild(false);
      is_paused = false;
      interface::pushScreen(&filamentMenu);
      host::activePauseBuild(true, command::SLEEP_TYPE_FILAMENT);
      is_sleeping = true;
      break;
    case 0:
      // pause command execution
      is_paused = !is_paused;
      host::pauseBuild(is_paused);
      if(is_paused){
        for (int i = 3; i < STEPPER_COUNT; i++) 
          steppers::enableAxis(i, false); 
      }else{
        interface::popScreen();
      }
      lineUpdate = true;
      break;
    case 1:
      // Cancel build
      interface::pushScreen(&cancel_build_menu);
      break;
    case 3:
      is_sleeping = !is_sleeping;
      if(is_sleeping){
        host::pauseBuild(false);
        is_paused = false;
        host::activePauseBuild(true, command::SLEEP_TYPE_COLD);
      }else{
        host::activePauseBuild(false, command::SLEEP_TYPE_COLD);
      }
      lineUpdate = true;
      break;
#ifdef ACTIVE_COOLING_FAN
    case 6:
      FanOn = !FanOn;
      Motherboard::getBoard().setExtra(FanOn);
      lineUpdate = true;
      break;
    case 7:
#else
    case 6:
#endif
      interface::popScreen();
      break;
       
  }
}
void BuildFinished::update(LiquidCrystalSerial& lcd, bool forceRedraw){
  
  if (forceRedraw) {
    
    Motherboard::getBoard().interfaceBlink(30,30);
    waiting = true;
    
    lcd.clear();
    
    lcd.setCursor(0,0);
    lcd.writeFromPgmspace(BUILD_FINISHED_MSG);
  
    uint8_t build_hours;
    uint8_t build_minutes;
    host::getPrintTime(build_hours, build_minutes);
    
    lcd.setCursor(14,2);
    lcd.writeInt(build_hours,2);
      
    lcd.setCursor(17,2);
    lcd.writeInt(build_minutes,2);
  }
}

void BuildFinished::reset(){
  waiting = false;
}

bool BuildFinished::screenWaiting(){
  return waiting;
}

void BuildFinished::notifyButtonPressed(ButtonArray::ButtonName button){
  
  switch (button) {
    case ButtonArray::CENTER:
      Motherboard::getBoard().interfaceBlink(0,0);
      interface::popScreen();
      break;
        case ButtonArray::LEFT:
      Motherboard::getBoard().interfaceBlink(0,0);
      interface::popScreen();
      break;
        case ButtonArray::RIGHT:
        case ButtonArray::DOWN:
        case ButtonArray::UP:
      break;
  }
}

void BuildStats::update(LiquidCrystalSerial& lcd, bool forceRedraw){
  
  if (forceRedraw) {
    lcd.clear();
    
    lcd.setCursor(0,2);
    lcd.writeFromPgmspace(BUILD_TIME_MSG);

    lcd.setCursor(0,3);
    lcd.writeFromPgmspace(LINE_NUMBER_MSG);
  }
  
  switch (update_count){
    
    case 0:
      uint8_t build_hours;
      uint8_t build_minutes;
      host::getPrintTime(build_hours, build_minutes);
      
      lcd.setCursor(14,2);
      lcd.writeInt(build_hours,2);
        
      lcd.setCursor(17,2);
      lcd.writeInt(build_minutes,2);
      
      break;
    case 1:
      uint32_t line_number;
      line_number = command::getLineNumber();
      /// if line number greater than counted, print an indicator that we are over count
      if(line_number > command::MAX_LINE_COUNT){
        lcd.setCursor(9,3);
        /// 10 digits is max for a uint32_t.  If we change the line_count to allow overflow, we'll need to update the digit count here
        lcd.writeInt(command::MAX_LINE_COUNT, 10);
        lcd.setCursor(19,3);
        lcd.writeString("+");
      }else{
        
        uint8_t digits = 1;
        for (uint32_t i = 10; i < command::MAX_LINE_COUNT; i*=10){
          if(line_number / i == 0){ break; }
          digits ++;
        }     
        lcd.setCursor(20-digits,3);
        lcd.writeInt(line_number, digits);
      }
      break;
    default:
      break;
  }
  update_count++;
  /// make the update_count max higher than actual updateable fields because
  /// we don't need to update these stats every half second
  if (update_count > UPDATE_COUNT_MAX){
    update_count = 0;
  }
}

void BuildStats::reset(){

  update_count = 0;
}

void BuildStats::notifyButtonPressed(ButtonArray::ButtonName button){
  
  switch (button) {
    case ButtonArray::CENTER:
      interface::popScreen();
      break;
        case ButtonArray::LEFT:
      interface::popScreen();
      break;
        case ButtonArray::RIGHT:
        case ButtonArray::DOWN:
        case ButtonArray::UP:
      break;
  }
}

void BotStats::update(LiquidCrystalSerial& lcd, bool forceRedraw){
  
  if (forceRedraw) {
    lcd.clear();
    
    lcd.setCursor(0,0);
    lcd.writeFromPgmspace(TOTAL_TIME_MSG);
    
    lcd.setCursor(0,2);
    lcd.writeFromPgmspace(LAST_TIME_MSG);
    
    /// TOTAL PRINT LIFETIME
    uint16_t total_hours = eeprom::getEeprom16(eeprom_offsets::TOTAL_BUILD_TIME + build_time_offsets::HOURS_OFFSET,0);
    uint8_t digits = 1;
    for (uint32_t i = 10; i < 10000; i*=10){
      if(total_hours / i == 0){ break; }
      digits ++;
    } 
    lcd.setCursor(19-digits, 1);
    lcd.writeInt32(total_hours, digits);

    /// LAST PRINT TIME
    uint8_t build_hours;
    uint8_t build_minutes;
    host::getPrintTime(build_hours, build_minutes);
    
    lcd.setCursor(14,2);
    lcd.writeInt(build_hours,2);
      
    lcd.setCursor(17,2);
    lcd.writeInt(build_minutes,2);
  }
}

void BotStats::reset(){
}

void BotStats::notifyButtonPressed(ButtonArray::ButtonName button){
  
  switch (button) {
    case ButtonArray::CENTER:
      break;
        case ButtonArray::LEFT:
      interface::popScreen();
      break;
        case ButtonArray::RIGHT:
        case ButtonArray::DOWN:
        case ButtonArray::UP:
      break;
  }
}
  
CancelBuildMenu::CancelBuildMenu() {
  itemCount = 4;
  reset();
}

void CancelBuildMenu::resetState() {

  itemIndex = 2;
  firstItemIndex = 2;
  
}

void CancelBuildMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd, uint8_t line_number) {

  host::HostState state = host::getHostState();
     
  switch (index) {
    case 0:
      if(((state == host::HOST_STATE_BUILDING) ||
              (state == host::HOST_STATE_BUILDING_FROM_SD)) &&
              !(Motherboard::getBoard().GetBoardStatus() & Motherboard::STATUS_ONBOARD_PROCESS)){
        lcd.writeFromPgmspace(CANCEL_MSG);
      }else{
        host::pauseBuild(true);
        lcd.writeFromPgmspace(CANCEL_PROCESS_MSG);
      }
      break;
    case 2:
        lcd.writeFromPgmspace(NO_MSG);
        break;
    case 3:
        lcd.writeFromPgmspace(YES_MSG);
        break;
  }
}
void CancelBuildMenu::pop(void){
  host::pauseBuild(false);
}

void CancelBuildMenu::handleSelect(uint8_t index) {
    
  switch (index) {
    case 2:
      interface::popScreen();
      break;
    case 3:
      if((host::getHostState() != host::HOST_STATE_BUILDING_ONBOARD) && (Motherboard::getBoard().GetBoardStatus() & Motherboard::STATUS_ONBOARD_PROCESS)){
        cancel_process = true;
        interface::popScreen();
      }else{
        // Cancel build
        host::stopBuild();
      }
      break;
  }
}


MainMenu::MainMenu() {
  itemCount = 5;
  reset();
}
void MainMenu::resetState() {
  itemIndex = 1;
  firstItemIndex = 1;
}

void MainMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd, uint8_t line_number) {

  char * name;
  
  switch (index) {
  case 0:
    name = host::getMachineName();
    lcd.setCursor((20 - strlen(name))/2,0);
    lcd.writeString(host::getMachineName());
    break;
  case 1:
    lcd.writeFromPgmspace(BUILD_MSG);
    break;
  case 2:
    lcd.writeFromPgmspace(PREHEAT_MSG);
    break;
  case 3:
    lcd.writeFromPgmspace(UTILITIES_MSG);
    break;
  case 4:
    lcd.writeFromPgmspace(INFO_MSG);
    break;
  }
}

void MainMenu::handleSelect(uint8_t index) {
  switch (index) {
    case 1:
      // Show build from SD screen
      interface::pushScreen(&sdmenu);
      break;
    case 2:
      // Show preheat screen
      interface::pushScreen(&preheat);
      break;
    case 3:
      // utilities menu
      interface::pushScreen(&utils);
      break;
    case 4:
      // info and settings
      interface::pushScreen(&info);
      break;
    }
}


UtilitiesMenu::UtilitiesMenu() {
  itemCount = 9;
  stepperEnable = false;
  blinkLED = false;
  reset();
}
void UtilitiesMenu::resetState(){
  singleTool = eeprom::isSingleTool();
  //if(singleTool){
  //  itemCount = 8;
 // }else{
 //   itemCount = 9;
 // }
}

void UtilitiesMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd, uint8_t line_number) {

  switch (index) {
  case 0:
    lcd.writeFromPgmspace(MONITOR_MSG);
    break;
  case 1:
    lcd.writeFromPgmspace(FILAMENT_OPTIONS_MSG);
    break;
  case 2:
    lcd.writeFromPgmspace(PLATE_LEVEL_MSG);
    break;
  case 4:
    lcd.writeFromPgmspace(JOG_MSG);
    break;  
  case 3:   
    lcd.writeFromPgmspace(HOME_AXES_MSG);
    break;  
  case 5:
    lcd.writeFromPgmspace(STARTUP_MSG);
    break;
  case 6:
    stepperEnable = true;
    // check if any steppers are enabled
    for (int i = 0; i < STEPPER_COUNT; i++){ 
      if(steppers::isEnabled(i)){
        stepperEnable = false;
        break;
      }
    }
    if(stepperEnable)
      lcd.writeFromPgmspace(ESTEPS_MSG);
    else
      lcd.writeFromPgmspace(DSTEPS_MSG);
    break;
  case 7:
    if(blinkLED)
      lcd.writeFromPgmspace(LED_STOP_MSG);
    else
      lcd.writeFromPgmspace(LED_BLINK_MSG);
    break;
  case 8:
   // if(!singleTool){
   //   lcd.writeFromPgmspace(NOZZLES_MSG);
   // }else{
      lcd.writeFromPgmspace(EXIT_MSG);
    break;
  //case 9:
  //  if(!singleTool){
  //    lcd.writeFromPgmspace(EXIT_MSG);
  //  }break;
  }
}

void UtilitiesMenu::handleSelect(uint8_t index) {
    
  switch (index) {
    case 0:
      // Show monitor build screen
            interface::pushScreen(&monitorMode);
      break;
    case 1:
      // load filament script
            interface::pushScreen(&filamentMenu);
      break;
    case 2:
      // level_plate script
            host::startOnboardBuild(utility::LEVEL_PLATE_STARTUP);
      break;
    case 4:
      // Show jog screen
            interface::pushScreen(&jogger);
      break;
    case 3:
      // home axes script
            host::startOnboardBuild(utility::HOME_AXES);
      break;
    case 5:
      // startup wizard script
            interface::pushScreen(&welcome);
      break;
    case 6:
      for (int i = 0; i < STEPPER_COUNT; i++) 
          steppers::enableAxis(i, stepperEnable);
      lineUpdate = true;
      stepperEnable = !stepperEnable;
      break;
    case 7:
      blinkLED = !blinkLED;
      if(blinkLED)
        RGB_LED::setLEDBlink(150);
      else
        RGB_LED::setLEDBlink(0);
      lineUpdate = true;     
       break;
    case 8:
      //if(!singleTool){
        // restore defaults
       // interface::pushScreen(&alignment);
     // }else{
        interface::popScreen();
     // }
      break;
   // case 9:
   //   if(!singleTool){
        // restore defaults
   //     interface::popScreen();
   //   }
   //   break;
    }
}


InfoMenu::InfoMenu() {
  itemCount = 6;
  reset();
}

void InfoMenu::resetState(){
}

void InfoMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd, uint8_t line_number) {

  switch (index) {
  case 0: 
    lcd.writeFromPgmspace(BOT_STATS_MSG);
    break;
  case 1: 
    lcd.writeFromPgmspace(SETTINGS_MSG);
    break;
  case 2:
    lcd.writeFromPgmspace(PREHEAT_SETTINGS_MSG);
    break;
  case 3:
    lcd.writeFromPgmspace(VERSION_MSG);
    break;
  case 4:
    lcd.writeFromPgmspace(RESET_MSG);
    break;
  case 5:
    lcd.writeFromPgmspace(EXIT_MSG);
    break;
  }
}

void InfoMenu::handleSelect(uint8_t index) {
    
  switch (index) {
    case 0:
      // bot stats menu
      interface::pushScreen(&bot_stats);
      break;
    case 1:
      // settings menu
            interface::pushScreen(&set);
      break;
    case 2:
      interface::pushScreen(&preheatSettings);
      break;
    case 3:
      splash.SetHold(true);
      interface::pushScreen(&splash);
      break;
    case 4:
      // restore defaults
      interface::pushScreen(&reset_settings);
      break;
    case 5:
      interface::popScreen();
      break;
    }
}


SettingsMenu::SettingsMenu() {
  itemCount = 8;
  reset();
  for (uint8_t i = 0; i < itemCount; i++){
    counter_item[i] = 0;
  }
  counter_item[1] = 1;
}

void SettingsMenu::resetState(){
  singleExtruder = eeprom::getEeprom8(eeprom_offsets::TOOL_COUNT, 1);
  soundOn = eeprom::getEeprom8(eeprom_offsets::BUZZ_SETTINGS, 1);
  LEDColor = eeprom::getEeprom8(eeprom_offsets::LED_STRIP_SETTINGS, 0);
  heatingLEDOn = eeprom::getEeprom8(eeprom_offsets::LED_STRIP_SETTINGS + blink_eeprom_offsets::LED_HEAT_OFFSET, 1);
  helpOn = eeprom::getEeprom8(eeprom_offsets::FILAMENT_HELP_SETTINGS, 1);
  accelerationOn = eeprom::getEeprom8(eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::ACTIVE_OFFSET, 0x01);
  HBPPresent = eeprom::getEeprom8(eeprom_offsets::HBP_PRESENT, 1);
}

void SettingsMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd, uint8_t line_number) {

  switch (index) {
    case 0:
      lcd.writeFromPgmspace(SOUND_MSG);
      lcd.setCursor(14,line_number);
      if(soundOn)
          lcd.writeFromPgmspace(ON_MSG);
      else
          lcd.writeFromPgmspace(OFF_MSG);
      break;
    case 1:
      lcd.writeFromPgmspace(LED_MSG);
      lcd.setCursor(11,line_number);
      if(selectIndex == 1)
         lcd.writeFromPgmspace(ARROW_MSG);
      else
        lcd.writeFromPgmspace(NO_ARROW_MSG);
        lcd.setCursor(14,line_number);
        switch(LEDColor){
                case LED_DEFAULT_RED:
                    lcd.writeFromPgmspace(RED_COLOR_MSG);
                    break;
                case LED_DEFAULT_ORANGE:
                    lcd.writeFromPgmspace(ORANGE_COLOR_MSG);
                    break;
                case LED_DEFAULT_PINK:
                    lcd.writeFromPgmspace(PINK_COLOR_MSG);
                    break;
                case LED_DEFAULT_GREEN:
                    lcd.writeFromPgmspace(GREEN_COLOR_MSG);
                    break;
                case LED_DEFAULT_BLUE:
                    lcd.writeFromPgmspace(BLUE_COLOR_MSG);
                    break;
                case LED_DEFAULT_PURPLE:
                    lcd.writeFromPgmspace(PURPLE_COLOR_MSG);
                    break;
                case LED_DEFAULT_WHITE:
                    lcd.writeFromPgmspace(WHITE_COLOR_MSG);
                    break;
                case LED_DEFAULT_CUSTOM:
                  lcd.writeFromPgmspace(CUSTOM_COLOR_MSG);
                  break;
                case LED_DEFAULT_OFF:
                  lcd.writeFromPgmspace(OFF_COLOR_MSG);
                  break;
       }
       break;
    case 6:
      lcd.writeFromPgmspace(PLATFORM_EXIST_MSG);
      lcd.setCursor(14, line_number);
      if(HBPPresent){
        lcd.writeFromPgmspace(YES_MSG);
      }else{
        lcd.writeFromPgmspace(NO_MSG);
      }
      break;
    case 5:
      lcd.writeFromPgmspace(TOOL_COUNT_MSG);
      lcd.setCursor(14,line_number);
      if(singleExtruder == 1)
          lcd.writeFromPgmspace(TOOL_SINGLE_MSG);
      else
          lcd.writeFromPgmspace(TOOL_DUAL_MSG);
      break;
    case 4:
      lcd.writeFromPgmspace(LED_HEAT_MSG);
      lcd.setCursor(14,line_number);
      if(heatingLEDOn)
          lcd.writeFromPgmspace(ON_MSG);
      else
          lcd.writeFromPgmspace(OFF_MSG);
      break;
    case 3:
      lcd.writeFromPgmspace(HELP_SCREENS_MSG);
      lcd.setCursor(14,line_number);
      if(helpOn)
          lcd.writeFromPgmspace(ON_MSG);
      else
          lcd.writeFromPgmspace(OFF_MSG);
      break;
    case 2:
      lcd.writeFromPgmspace(ACCELERATE_MSG);
      lcd.setCursor(14,line_number);
      if(accelerationOn)
          lcd.writeFromPgmspace(ON_MSG);
      else
          lcd.writeFromPgmspace(OFF_MSG);
      break;
    case 7:
      lcd.writeFromPgmspace(EXIT_MSG);
      break;
  }
}

void SettingsMenu::handleCounterUpdate(uint8_t index, bool up){
    switch (index) {
      case 1:
        // update left counter
        if(up)
            LEDColor++;
        else
            LEDColor--;
        // keep within appropriate boundaries
        if(LEDColor > 7)
            LEDColor = 0;
        else if(LEDColor < 0)
        LEDColor = 7;
      
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
          eeprom_write_byte((uint8_t*)eeprom_offsets::LED_STRIP_SETTINGS, LEDColor);
        }
        RGB_LED::setDefaultColor(); 
        break;
  }
    
}


void SettingsMenu::handleSelect(uint8_t index) {
  switch (index) {
    case 0:
      // update sound preferences
      soundOn = !soundOn;
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        eeprom_write_byte((uint8_t*)eeprom_offsets::BUZZ_SETTINGS, soundOn);
      }
      lineUpdate = 1;
      break;
    case 1:
      // update LED preferences
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        eeprom_write_byte((uint8_t*)eeprom_offsets::LED_STRIP_SETTINGS, LEDColor);
      }
      RGB_LED::setDefaultColor();
      lineUpdate = 1;
      break;
    case 5:
      // update tool count
      singleExtruder = !singleExtruder;
      eeprom::setToolHeadCount(singleExtruder ? 1 : 2);
      if(!singleExtruder){
        Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().set_target_temperature(0);
      }
      lineUpdate = 1;
      break;
    case 4:
      heatingLEDOn = !heatingLEDOn;
      // update LEDHeatingflag
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        eeprom_write_byte((uint8_t*)eeprom_offsets::LED_STRIP_SETTINGS + blink_eeprom_offsets::LED_HEAT_OFFSET, heatingLEDOn);
      }
      lineUpdate = 1;
      break;
    case 3:
      helpOn = !helpOn;
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        eeprom_write_byte((uint8_t*)eeprom_offsets::FILAMENT_HELP_SETTINGS, helpOn);
      }
      lineUpdate = 1;
      break;
    case 2:
      accelerationOn = !accelerationOn;
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        eeprom_write_byte((uint8_t*)eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::ACTIVE_OFFSET, accelerationOn);
      }
      lineUpdate = 1;
      break;
    case 6:
      // update hbp setting
      HBPPresent = !HBPPresent;
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        eeprom_write_byte((uint8_t*)eeprom_offsets::HBP_PRESENT, HBPPresent);
      }
      Motherboard::getBoard().setUsingPlatform(HBPPresent);
      Motherboard::getBoard().getPlatformHeater().disable(!HBPPresent);
      lineUpdate = 1;
      break;
    case 7:
      interface::popScreen();
      break;
    }
}

SDMenu::SDMenu() {
  reset();
}

void SDMenu::resetState() {
  cardNotFound = false;
  cardBadFormat = false;
  cardReadError = false;
  itemCount = countFiles() + 1;
  sliding_menu = false;
}

//Returns true if the file is an s3g file
//Keeping this in C instead of C++ saves 20 bytes
 
bool isS3GFile(char *filename, uint8_t len) {
    if ((len >= 4) && 
        (filename[len-4]== '.') && (filename[len-3]== 's') &&
        (filename[len-2]== '3') && (filename[len-1]== 'g')) return true;
    return false;
}

// Count the number of files on the SD card
uint8_t SDMenu::countFiles() {
  uint8_t count = 0;
  uint8_t idx = 0;
  sdcard::SdErrorCode e;  

  // First, reset the directory index
  e = sdcard::directoryReset();
  if (e != sdcard::SD_SUCCESS) {
    switch(e) {
      case sdcard::SD_ERR_NO_CARD_PRESENT:
        cardNotFound = true;
        break;
      case sdcard::SD_ERR_OPEN_FILESYSTEM:
        cardBadFormat = true;
        break;
      case sdcard::SD_ERR_PARTITION_READ:
      case sdcard::SD_ERR_INIT_FAILED:
      case sdcard:: SD_ERR_NO_ROOT:
        cardReadError = true;
        break;
      case sdcard::SD_ERR_VOLUME_TOO_BIG:
        cardTooBig = true;
        break;
      default:
        break;
    }
    return 0;
  }

  //file name 
  char fnbuf[SD_CARD_MAX_FILE_LENGTH];

  // Count the files
  do {
    e = sdcard::directoryNextEntry(fnbuf,SD_CARD_MAX_FILE_LENGTH, &idx);
    if (fnbuf[0] == '\0') {
      break;
    }

    //Only count it if it ends in .s3g
    if (isS3GFile(fnbuf,idx) && !(fnbuf[0] == '.')) count++;

  } while (e == sdcard::SD_SUCCESS);

  // TODO: Check for error again?

  return count;
}

bool SDMenu::getFilename(uint8_t index, char buffer[], uint8_t buffer_size) {
  sdcard::SdErrorCode e;
  uint8_t idx;

  // First, reset the directory list
  e = sdcard::directoryReset();
  if (e != sdcard::SD_SUCCESS) {
        switch(e) {
      case sdcard::SD_ERR_NO_CARD_PRESENT:
        cardNotFound = true;
        break;
      case sdcard::SD_ERR_OPEN_FILESYSTEM:
        cardBadFormat = true;
        break;
      case sdcard::SD_ERR_PARTITION_READ:
      case sdcard::SD_ERR_INIT_FAILED:
        case sdcard:: SD_ERR_NO_ROOT:
        cardReadError = true;
        break;
      case sdcard::SD_ERR_VOLUME_TOO_BIG:
        cardTooBig = true;
        break;
      default:
        break;
    }
    return false;
  }
  
  char fnbuf[SD_CARD_MAX_FILE_LENGTH];

  for(uint8_t i = 0; i < index+1; i++) {
    // Ignore dot-files
    do {
      e = sdcard::directoryNextEntry(fnbuf,SD_CARD_MAX_FILE_LENGTH, &idx);
      if (fnbuf[0] == '\0') {
        return false;
      }
      
    } while ((e == sdcard::SD_SUCCESS)  && ( !isS3GFile(fnbuf,idx) || (fnbuf[0] == '.')));
      
    if (e != sdcard::SD_SUCCESS) {
       return false;
    }
  }
  uint8_t bufSize = (buffer_size <= idx) ? buffer_size-1 : idx;   
  for(uint8_t i = 0; i < bufSize; i++)
    buffer[i] = fnbuf[i];
  buffer[bufSize] = 0;
    return true;
}

void SDMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd, uint8_t line_number) {
       
       // print error message if no SD card found;
    if(cardNotFound == true) {
      Piezo::playTune(TUNE_ERROR);
      lcd.writeFromPgmspace(NOCARD_MSG);
      return;
    }else if (cardReadError){
      Piezo::playTune(TUNE_ERROR);
      lcd.writeFromPgmspace(CARDERROR_MSG);
      return;
    }else if (cardBadFormat){
      Piezo::playTune(TUNE_ERROR);
      lcd.writeFromPgmspace(CARDFORMAT_MSG);
      return;
    }else if (cardTooBig){
      Piezo::playTune(TUNE_ERROR);
      lcd.writeFromPgmspace(CARDSIZE_MSG);
      return;
    }
    // print last line for SD card - an exit option
    if (index >= itemCount - 1) {
      lcd.writeFromPgmspace(EXIT_MSG);
      return;
    }

  char fnbuf[SD_CARD_MAX_FILE_LENGTH];

    if ( !getFilename(index, fnbuf, SD_CARD_MAX_FILE_LENGTH)) {
        interface::popScreen();
        Piezo::playTune(TUNE_ERROR);
        Motherboard::getBoard().errorResponse(ERROR_SD_CARD_GENERIC);
        return;
  }

  uint8_t idx;
  for (idx = 0; (idx < LCD_SCREEN_WIDTH-1) && (fnbuf[idx] != 0); idx++) {
    lcd.write(fnbuf[idx]);
  }
  
}

void SDMenu::handleSelect(uint8_t index) {
  
  if(index >= itemCount -1)
  {
    interface::popScreen();
    return;
  }
  if (host::getHostState() != host::HOST_STATE_READY) {
    Piezo::playTune(TUNE_ERROR);
    Motherboard::getBoard().errorResponse(ERROR_SD_CARD_BUILDING);
    return;
  }

  char* buildName = host::getBuildName();
 
  if ( !getFilename(index, buildName, host::MAX_FILE_LEN) ) {
      interface::popScreen();
      Piezo::playTune(TUNE_ERROR);
      Motherboard::getBoard().errorResponse(ERROR_SD_CARD_GENERIC);
      return;
  }
 
  sdcard::SdErrorCode e;
  e = host::startBuildFromSD();
  
  if (e != sdcard::SD_SUCCESS) {
      interface::popScreen();
      Piezo::playTune(TUNE_ERROR);
      Motherboard::getBoard().errorResponse(ERROR_SD_CARD_GENERIC);
      return;
  }

}

#endif
