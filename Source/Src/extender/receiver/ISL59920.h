
//------------------------------------------------------------------------------------------------
// ISL_59920.H
//------------------------------------------------------------------------------------------------
// Define Aread
#define COLOR_MIN_SEG		 0x00 
#define COLOR_MAX_SEG		 0x1F 
#define OPERATION_PLUS		 '+'
#define OPERATION_MINUS		 '-'

void ISL59920_Init(void);
void ISL59920_Write(U8_T color,U8_T delay,U8_T operation);
void ISL59920_Adjust(U8_T color,U8_T operation,U8_T delay,U8_T *current);
void ISL59920_Color_Delay_Setting(U8_T color,U8_T operaton);
void ISL59920_Hotkey_Control(U8_T keycode);

extern bit  bOperateFlag;
extern U8_T cOperateColor;
extern U8_T cOperateCtrl;
extern U8_T cOperateValue;