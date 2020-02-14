enum suika_mode{
	PLAY,
	DRINK
};

enum suika_state{
 	TONE,
	FEVER,
  BELL,
  SILENT
};

// Sound
#define BZR     8   // Sound output
#define SP_EN   4    // Sound Amp-IC enable
// LED
#define LED_R1  14   // LED-RED1 Control
#define LED_R2  15   // LED-RED2 Control
#define LED_R3  16   // LED-RED3 Control
#define LED_Y1  9    // LED-YELLOW1 Control
#define LED_Y2  10   // LED-YELLOW2 Control
#define LED_Y3  6    // LED-YELLOW3 Control
#define LED_Y4  5    // LED-YELLOW4 Control
#define LED_Y5  3    // LED-YELLOW5 Control
// Switch
#define PSW1    7    // Function SW
#define PSW2    17   // Sound SW
// Expansion IO (unused)
#define EXPIO_0 0
#define EXPIO_1 1
#define EXPIO_2 2
#define EXPIO_3 A7   // Analog-IO ONLY!
