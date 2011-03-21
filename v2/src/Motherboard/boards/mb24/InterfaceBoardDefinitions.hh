#ifndef INTERFACE_BOARD_DEFINITIONS_HH_
#define INTERFACE_BOARD_DEFINITIONS_HH_

#define LCD_SCREEN_WIDTH 16
#define LCD_SCREEN_HEIGHT 4

namespace InterfaceBoardDefinitions {

enum ButtonName {
	ZERO		= 1,
	ZMINUS		= 2,
	ZPLUS		= 3,
	YMINUS		= 4,
	YPLUS		= 5,
	XMINUS		= 6,
	XPLUS		= 7,
	CANCEL		= 11,
	OK			= 12,
};

}

#endif
