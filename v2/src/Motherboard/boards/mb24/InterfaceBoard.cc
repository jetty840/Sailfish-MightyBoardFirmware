#include "InterfaceBoard.hh"
#include <AvrPort.hh>

static PROGMEM prog_uchar welcomeMessage[] = {"MakerBot TOM"};


InterfaceBoard::InterfaceBoard() :
	lcd(Pin(PortC,4), Pin(PortC,3), Pin(PortD,7), Pin(PortG,2), Pin(PortG,1), Pin(PortG,0))
{
	lcd.begin(16,4);
	lcd.clear();
	lcd.home();

	lcd.write_from_pgmspace(welcomeMessage);
}