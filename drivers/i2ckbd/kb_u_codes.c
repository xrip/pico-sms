#include "kb_u_codes.h"
#include "string.h"
#include "globals.h"

#define TRANSZX_TABLE_SIZE 99
#define TRANSCH_TABLE_SIZE 62


#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
(byte & 0x80 ? '1' : '0'), \
(byte & 0x40 ? '1' : '0'), \
(byte & 0x20 ? '1' : '0'), \
(byte & 0x10 ? '1' : '0'), \
(byte & 0x08 ? '1' : '0'), \
(byte & 0x04 ? '1' : '0'), \
(byte & 0x02 ? '1' : '0'), \
(byte & 0x01 ? '1' : '0') 


typedef struct kbzx_translate{
   uint32_t mask;
   uint8_t bank;
   uint8_t keys[8];
} __attribute__((packed)) kbzx_translate;

kbzx_translate __in_flash() transzx_table[TRANSZX_TABLE_SIZE]={

	//Bank 1							 0    1    2    3     4    5    6    7
	{KB_U1_ENTER				,0x01,{0x00,0x00,0x00,0x00, 0x00,0x00,1<<0,0x00}}, 
	{KB_U1_SLASH				,0x01,{1<<3,0x00,0x00,0x00, 0x00,0x00,0x00,1<<1}}, 
	{KB_U1_MINUS				,0x01,{0x00,0x00,0x00,0x00, 0x00,0x00,1<<3,1<<1}},
	{KB_U1_EQUALS				,0x01,{0x00,0x00,0x00,0x00, 0x00,0x00,1<<1,1<<1}},
	{KB_U1_BACKSLASH			,0x01,{1<<1,0x00,0x00,0x00, 0x00,0x00,0x00,1<<1}},

	{KB_U1_CAPS_LOCK			,0x01,{1<<0,0x00,0x00,1<<1, 0x00,0x00,0x00,0x00}}, 
	{KB_U1_TAB					,0x01,{1<<0,0x00,0x00,1<<0, 0x00,0x00,0x00,0x00}}, 
	{KB_U1_BACK_SPACE			,0x01,{1<<0,0x00,0x00,0x00, 1<<0,0x00,0x00,0x00}},
	{KB_U1_ESC					,0x01,{1<<0,0x00,0x00,0x00, 0x00,0x00,0x00,1<<0}}, 
	{KB_U1_TILDE				,0x01,{0x00,1<<0,0x00,0x00, 0x00,0x00,0x00,1<<1}}, 
	
	{KB_U1_L_SHIFT				,0x01,{1<<0,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}},
	{KB_U1_R_SHIFT				,0x01,{1<<0,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}},
	{KB_U1_L_CTRL				,0x01,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,1<<1}},
	{KB_U1_R_CTRL				,0x01,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,1<<1}},	
	{KB_U1_SPACE				,0x01,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,1<<0}},
	
	{KB_U1_L_ALT				,0x01,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}},
	{KB_U1_R_ALT				,0x01,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}},
	{KB_U1_L_WIN				,0x01,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}},
	{KB_U1_R_WIN				,0x01,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}},
	{KB_U1_MENU					,0x01,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}},

	//Bank 0							 0    1    2    3     4    5    6    7
	{KB_U0_A					,0x00,{0x00,1<<0,0x00,0x00, 0x00,0x00,0x00,0x00}},
	{KB_U0_B					,0x00,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,1<<4}},
	{KB_U0_C					,0x00,{1<<3,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}},
	{KB_U0_D					,0x00,{0x00,1<<2,0x00,0x00, 0x00,0x00,0x00,0x00}},
	{KB_U0_E					,0x00,{0x00,0x00,1<<2,0x00, 0x00,0x00,0x00,0x00}},
	
	{KB_U0_F					,0x00,{0x00,1<<3,0x00,0x00, 0x00,0x00,0x00,0x00}},
	{KB_U0_G					,0x00,{0x00,1<<4,0x00,0x00, 0x00,0x00,0x00,0x00}},
	{KB_U0_H					,0x00,{0x00,0x00,0x00,0x00, 0x00,0x00,1<<4,0x00}},
	{KB_U0_I					,0x00,{0x00,0x00,0x00,0x00, 0x00,1<<2,0x00,0x00}},
	{KB_U0_J					,0x00,{0x00,0x00,0x00,0x00, 0x00,0x00,1<<3,0x00}},
	
	{KB_U0_K					,0x00,{0x00,0x00,0x00,0x00, 0x00,0x00,1<<2,0x00}},
	{KB_U0_L					,0x00,{0x00,0x00,0x00,0x00, 0x00,0x00,1<<1,0x00}},
	{KB_U0_M					,0x00,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,1<<2}},
	{KB_U0_N					,0x00,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,1<<3}},
	{KB_U0_O					,0x00,{0x00,0x00,0x00,0x00, 0x00,1<<1,0x00,0x00}},
	
	{KB_U0_P					,0x00,{0x00,0x00,0x00,0x00, 0x00,1<<0,0x00,0x00}},
	{KB_U0_Q					,0x00,{0x00,0x00,1<<0,0x00, 0x00,0x00,0x00,0x00}},
	{KB_U0_R					,0x00,{0x00,0x00,1<<3,0x00, 0x00,0x00,0x00,0x00}},
	{KB_U0_S					,0x00,{0x00,1<<1,0x00,0x00, 0x00,0x00,0x00,0x00}},
	{KB_U0_T					,0x00,{0x00,0x00,1<<4,0x00, 0x00,0x00,0x00,0x00}},
	
	{KB_U0_U					,0x00,{0x00,0x00,0x00,0x00, 0x00,1<<3,0x00,0x00}},
	{KB_U0_V					,0x00,{1<<4,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}},
	{KB_U0_W					,0x00,{0x00,0x00,1<<1,0x00, 0x00,0x00,0x00,0x00}},
	{KB_U0_X					,0x00,{1<<2,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}},
	{KB_U0_Y					,0x00,{0x00,0x00,0x00,0x00, 0x00,1<<4,0x00,0x00}},
	
	{KB_U0_Z					,0x00,{1<<1,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}},

	{KB_U1_1					,0x01,{0x00,0x00,0x00,1<<0, 0x00,0x00,0x00,0x00}},
	{KB_U1_2					,0x01,{0x00,0x00,0x00,1<<1, 0x00,0x00,0x00,0x00}},
	{KB_U1_3					,0x01,{0x00,0x00,0x00,1<<2, 0x00,0x00,0x00,0x00}},
	{KB_U1_4					,0x01,{0x00,0x00,0x00,1<<3, 0x00,0x00,0x00,0x00}},
	{KB_U1_5					,0x01,{0x00,0x00,0x00,1<<4, 0x00,0x00,0x00,0x00}},
	
	{KB_U1_6					,0x01,{0x00,0x00,0x00,0x00, 1<<4,0x00,0x00,0x00}},
	{KB_U1_7					,0x01,{0x00,0x00,0x00,0x00, 1<<3,0x00,0x00,0x00}},
	{KB_U1_8					,0x01,{0x00,0x00,0x00,0x00, 1<<2,0x00,0x00,0x00}},
	{KB_U1_9					,0x01,{0x00,0x00,0x00,0x00, 1<<1,0x00,0x00,0x00}},
	{KB_U1_0					,0x01,{0x00,0x00,0x00,0x00, 1<<0,0x00,0x00,0x00}},
	{KB_U2_NUM_1				,0x02,{0x00,0x00,0x00,1<<0, 0x00,0x00,0x00,0x00}},
	{KB_U2_NUM_2				,0x02,{0x00,0x00,0x00,1<<1, 0x00,0x00,0x00,0x00}},
	{KB_U2_NUM_3				,0x02,{0x00,0x00,0x00,1<<2, 0x00,0x00,0x00,0x00}},
	{KB_U2_NUM_4				,0x02,{0x00,0x00,0x00,1<<3, 0x00,0x00,0x00,0x00}},
	{KB_U2_NUM_5				,0x02,{0x00,0x00,0x00,1<<4, 0x00,0x00,0x00,0x00}},
	
	{KB_U2_NUM_6				,0x02,{0x00,0x00,0x00,0x00, 1<<4,0x00,0x00,0x00}},
	{KB_U2_NUM_7				,0x02,{0x00,0x00,0x00,0x00, 1<<3,0x00,0x00,0x00}},
	{KB_U2_NUM_8				,0x02,{0x00,0x00,0x00,0x00, 1<<2,0x00,0x00,0x00}},
	{KB_U2_NUM_9				,0x02,{0x00,0x00,0x00,0x00, 1<<1,0x00,0x00,0x00}},
	{KB_U2_NUM_0				,0x02,{0x00,0x00,0x00,0x00, 1<<0,0x00,0x00,0x00}},
	{KB_U2_NUM_ENTER			,0x02,{0x00,0x00,0x00,0x00, 0x00,0x00,1<<0,0x00}},

	{KB_U0_LEFT_BR				,0x00,{0x00,0x00,0x00,0x00, 1<<2,0x00,0x00,1<<1}},
	{KB_U0_RIGHT_BR				,0x00,{0x00,0x00,0x00,0x00, 1<<1,0x00,0x00,1<<1}},
	{KB_U0_SEMICOLON			,0x00,{0x00,0x00,0x00,0x00, 0x00,1<<1,0x00,1<<1}},
	{KB_U0_QUOTE				,0x00,{0x00,0x00,0x00,0x00, 0x00,1<<0,0x00,1<<1}},

	{KB_U0_COMMA				,0x00,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x0A}},
	{KB_U0_PERIOD				,0x00,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x06}},
	
	//Bank 2							 0    1    2    3     4    5    6    7

	//{KB_U2_NUM_SLASH			,0x02,{1<<4,0x00,0x00,0x00, 0x00,0x00,0x00,1<<1}},
	//{KB_U2_NUM_MINUS			,0x02,{0x00,0x00,0x00,0x00, 0x00,0x00,1<<3,1<<1}},
	//{KB_U2_NUM_PLUS			,0x02,{0x00,0x00,0x00,0x00, 0x00,0x00,1<<2,1<<1}},
	//{KB_U2_NUM_MULT			,0x02,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x12}},
	//{KB_U2_NUM_PERIOD			,0x02,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x06}},
	
	{KB_U2_NUM_LOCK				,0x02,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}},
	{KB_U2_SCROLL_LOCK			,0x02,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}},
	{KB_U2_PRT_SCR				,0x02,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}}, //для принт скрин обработаем только 1 код
	
	{KB_U2_UP					,0x02,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}},
	{KB_U2_DOWN					,0x02,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}},
	{KB_U2_RIGHT				,0x02,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}},
	{KB_U2_LEFT					,0x02,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}},
	{KB_U2_DELETE				,0x02,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}},
	{KB_U2_END					,0x02,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}},
	{KB_U2_PAGE_DOWN			,0x02,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}},
	{KB_U2_PAGE_UP				,0x02,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}},
	{KB_U2_HOME					,0x02,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}},
	{KB_U2_INSERT				,0x02,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}},
	{KB_U2_PAUSE_BREAK			,0x02,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}},
	//Bank 3							 0    1    2    3     4    5    6    7
	{KB_U3_F1					,0x03,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}}, 
	{KB_U3_F2					,0x03,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}}, 
	{KB_U3_F3					,0x03,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}}, 
	{KB_U3_F4					,0x03,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}}, 
	{KB_U3_F5					,0x03,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}}, 
	{KB_U3_F6					,0x03,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}}, 
	{KB_U3_F7					,0x03,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}}, 
	{KB_U3_F8					,0x03,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}}, 
	{KB_U3_F9					,0x03,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}}, 
	{KB_U3_F10					,0x03,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}},
	{KB_U3_F11					,0x03,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}},
	{KB_U3_F12					,0x03,{0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00}},
};

void (convert_kb_u_to_kb_zx)(kb_u_state* kb_st,uint8_t* zx_kb){
	for (uint8_t idx=TRANSZX_TABLE_SIZE-1;idx--;){
		if(kb_st->u[transzx_table[idx].bank]&transzx_table[idx].mask){
			if(
				(transzx_table[idx].keys[0]>0)||
				(transzx_table[idx].keys[1]>0)||
				(transzx_table[idx].keys[2]>0)||
				(transzx_table[idx].keys[3]>0)||
				(transzx_table[idx].keys[4]>0)||
				(transzx_table[idx].keys[5]>0)||
				(transzx_table[idx].keys[6]>0)||
				(transzx_table[idx].keys[7]>0)
			){
				for(uint8_t idy=0;idy<8;idy++){
					zx_kb[idy]|=transzx_table[idx].keys[idy];
					//busy_wait_us(2);
				}
				//busy_wait_us(2);
				//printf("[%010ld]>>>[%08lX][%08lX][%08lX][%08lX]:[%02X]  [%02X][%02X][%02X][%02X][%02X][%02X][%02X][%02X]  ["BYTE_TO_BINARY_PATTERN"]["BYTE_TO_BINARY_PATTERN"]["BYTE_TO_BINARY_PATTERN"]\n", time_us_32(),kb_st->u[0],kb_st->u[1],kb_st->u[2],kb_st->u[3],kb_st->state,zx_kb[0],zx_kb[1],zx_kb[2],zx_kb[3],zx_kb[4],zx_kb[5],zx_kb[6],zx_kb[7],BYTE_TO_BINARY(0),BYTE_TO_BINARY(0),BYTE_TO_BINARY(0));
			}
		}
	}
};


typedef struct kbst_translate{
   uint32_t mask;
   uint8_t  bank;
   uint8_t  symbol;
} __attribute__((packed)) kbst_translate;


kbst_translate __in_flash() transch_table[TRANSCH_TABLE_SIZE]={
	{KB_U0_A					,0x00,'A'},
	{KB_U0_B					,0x00,'B'},
	{KB_U0_C					,0x00,'C'},
	{KB_U0_D					,0x00,'D'},
	{KB_U0_E					,0x00,'E'},
	{KB_U0_F					,0x00,'F'},
	{KB_U0_G					,0x00,'G'},
	{KB_U0_H					,0x00,'H'},
	{KB_U0_I					,0x00,'I'},
	{KB_U0_J					,0x00,'J'},
	{KB_U0_K					,0x00,'K'},
	{KB_U0_L					,0x00,'L'},
	{KB_U0_M					,0x00,'M'},
	{KB_U0_N					,0x00,'N'},
	{KB_U0_O					,0x00,'O'},
	{KB_U0_P					,0x00,'P'},
	{KB_U0_Q					,0x00,'Q'},
	{KB_U0_R					,0x00,'R'},
	{KB_U0_S					,0x00,'S'},
	{KB_U0_T					,0x00,'T'},
	{KB_U0_U					,0x00,'U'},
	{KB_U0_V					,0x00,'V'},
	{KB_U0_W					,0x00,'W'},
	{KB_U0_X					,0x00,'X'},
	{KB_U0_Y					,0x00,'Y'},
	{KB_U0_Z					,0x00,'Z'},
	{KB_U0_LEFT_BR				,0x00,'['},
	{KB_U0_RIGHT_BR				,0x00,']'},
	{KB_U0_SEMICOLON			,0x00,';'},
	{KB_U0_QUOTE				,0x00,'\''},
	{KB_U0_COMMA				,0x00,','},
	{KB_U0_PERIOD				,0x00,'.'},

	{KB_U1_SLASH				,0x01,'/'}, 
	{KB_U1_MINUS				,0x01,'-'},
	{KB_U1_EQUALS				,0x01,'='},
	{KB_U1_BACKSLASH			,0x01,'\\'},
	{KB_U1_TILDE				,0x01,'`'}, 
	{KB_U1_1					,0x01,'1'},
	{KB_U1_2					,0x01,'2'},
	{KB_U1_3					,0x01,'3'},
	{KB_U1_4					,0x01,'4'},
	{KB_U1_5					,0x01,'5'},
	{KB_U1_6					,0x01,'6'},
	{KB_U1_7					,0x01,'7'},
	{KB_U1_8					,0x01,'8'},
	{KB_U1_9					,0x01,'9'},
	{KB_U1_0					,0x01,'0'},

	{KB_U2_NUM_1				,0x02,'1'},
	{KB_U2_NUM_2				,0x02,'2'},
	{KB_U2_NUM_3				,0x02,'3'},
	{KB_U2_NUM_4				,0x02,'4'},
	{KB_U2_NUM_5				,0x02,'5'},
	{KB_U2_NUM_6				,0x02,'6'},
	{KB_U2_NUM_7				,0x02,'7'},
	{KB_U2_NUM_8				,0x02,'8'},
	{KB_U2_NUM_9				,0x02,'9'},
	{KB_U2_NUM_0				,0x02,'0'},
	{KB_U2_NUM_SLASH			,0x02,'/'},
	{KB_U2_NUM_MINUS			,0x02,'-'},
	{KB_U2_NUM_PLUS				,0x02,'+'},
	{KB_U2_NUM_MULT				,0x02,'*'},
	{KB_U2_NUM_PERIOD			,0x02,'.'},
};

uint8_t convert_kb_u_to_char(char* str_buf, kb_u_state kb_st){
	for (uint8_t idx=TRANSCH_TABLE_SIZE-1;idx--;){
		if(kb_st.u[transch_table[idx].bank]&transch_table[idx].mask){
			return transch_table[idx].symbol;
		}
	}
	return 0;
}

void keys_to_str(char* str_buf,char s_char,kb_u_state kb_state){
	char s_str[2];
	s_str[0]=s_char;
	s_str[1]='\0';

	str_buf[0]=0;
	//strcat(str_buf,"KEY PRESSED: ");
//0 набор
	if (kb_state.u[0]&KB_U0_A) {strcat(str_buf,"A");strcat(str_buf,s_str);};
	if (kb_state.u[0]&KB_U0_B) {strcat(str_buf,"B");strcat(str_buf,s_str);};
	if (kb_state.u[0]&KB_U0_C) {strcat(str_buf,"C");strcat(str_buf,s_str);};
	if (kb_state.u[0]&KB_U0_D) {strcat(str_buf,"D");strcat(str_buf,s_str);};
	if (kb_state.u[0]&KB_U0_E) {strcat(str_buf,"E");strcat(str_buf,s_str);};
	if (kb_state.u[0]&KB_U0_F) {strcat(str_buf,"F");strcat(str_buf,s_str);};
	if (kb_state.u[0]&KB_U0_G) {strcat(str_buf,"G");strcat(str_buf,s_str);};
	if (kb_state.u[0]&KB_U0_H) {strcat(str_buf,"H");strcat(str_buf,s_str);};
	if (kb_state.u[0]&KB_U0_I) {strcat(str_buf,"I");strcat(str_buf,s_str);};
	if (kb_state.u[0]&KB_U0_J) {strcat(str_buf,"J");strcat(str_buf,s_str);};

	if (kb_state.u[0]&KB_U0_K) {strcat(str_buf,"K");strcat(str_buf,s_str);};
	if (kb_state.u[0]&KB_U0_L) {strcat(str_buf,"L");strcat(str_buf,s_str);};
	if (kb_state.u[0]&KB_U0_M) {strcat(str_buf,"M");strcat(str_buf,s_str);};
	if (kb_state.u[0]&KB_U0_N) {strcat(str_buf,"N");strcat(str_buf,s_str);};
	if (kb_state.u[0]&KB_U0_O) {strcat(str_buf,"O");strcat(str_buf,s_str);};
	if (kb_state.u[0]&KB_U0_P) {strcat(str_buf,"P");strcat(str_buf,s_str);};
	if (kb_state.u[0]&KB_U0_Q) {strcat(str_buf,"Q");strcat(str_buf,s_str);};
	if (kb_state.u[0]&KB_U0_R) {strcat(str_buf,"R");strcat(str_buf,s_str);};
	if (kb_state.u[0]&KB_U0_S) {strcat(str_buf,"S");strcat(str_buf,s_str);};
	if (kb_state.u[0]&KB_U0_T) {strcat(str_buf,"T");strcat(str_buf,s_str);};

	if (kb_state.u[0]&KB_U0_U) {strcat(str_buf,"U");strcat(str_buf,s_str);};
	if (kb_state.u[0]&KB_U0_V) {strcat(str_buf,"V");strcat(str_buf,s_str);};
	if (kb_state.u[0]&KB_U0_W) {strcat(str_buf,"W");strcat(str_buf,s_str);};
	if (kb_state.u[0]&KB_U0_X) {strcat(str_buf,"X");strcat(str_buf,s_str);};
	if (kb_state.u[0]&KB_U0_Y) {strcat(str_buf,"Y");strcat(str_buf,s_str);};
	if (kb_state.u[0]&KB_U0_Z) {strcat(str_buf,"Z");strcat(str_buf,s_str);};

	if (kb_state.u[0]&KB_U0_SEMICOLON)	{strcat(str_buf,";");strcat(str_buf,s_str);};
	if (kb_state.u[0]&KB_U0_QUOTE) 		{strcat(str_buf,"\"");strcat(str_buf,s_str);};
	if (kb_state.u[0]&KB_U0_COMMA) 		{strcat(str_buf,",");strcat(str_buf,s_str);};
	if (kb_state.u[0]&KB_U0_PERIOD)	 	{strcat(str_buf,".");strcat(str_buf,s_str);};
	if (kb_state.u[0]&KB_U0_LEFT_BR) 	{strcat(str_buf,"[");strcat(str_buf,s_str);};
	if (kb_state.u[0]&KB_U0_RIGHT_BR) 	{strcat(str_buf,"]");strcat(str_buf,s_str);};
//1 набор
	if (kb_state.u[1]&KB_U1_0) {strcat(str_buf,"0");strcat(str_buf,s_str);};
	if (kb_state.u[1]&KB_U1_1) {strcat(str_buf,"1");strcat(str_buf,s_str);};
	if (kb_state.u[1]&KB_U1_2) {strcat(str_buf,"2");strcat(str_buf,s_str);};
	if (kb_state.u[1]&KB_U1_3) {strcat(str_buf,"3");strcat(str_buf,s_str);};
	if (kb_state.u[1]&KB_U1_4) {strcat(str_buf,"4");strcat(str_buf,s_str);};
	if (kb_state.u[1]&KB_U1_5) {strcat(str_buf,"5");strcat(str_buf,s_str);};
	if (kb_state.u[1]&KB_U1_6) {strcat(str_buf,"6");strcat(str_buf,s_str);};
	if (kb_state.u[1]&KB_U1_7) {strcat(str_buf,"7");strcat(str_buf,s_str);};
	if (kb_state.u[1]&KB_U1_8) {strcat(str_buf,"8");strcat(str_buf,s_str);};
	if (kb_state.u[1]&KB_U1_9) {strcat(str_buf,"9");strcat(str_buf,s_str);};
	if (kb_state.u[1]&KB_U1_SLASH) {strcat(str_buf,"/");strcat(str_buf,s_str);};
	if (kb_state.u[1]&KB_U1_EQUALS) {strcat(str_buf,"=");strcat(str_buf,s_str);};

	if (kb_state.u[1]&KB_U1_ENTER) {strcat(str_buf,"ENTER");strcat(str_buf,s_str);};
	if (kb_state.u[1]&KB_U1_MINUS) {strcat(str_buf,"MINUS");strcat(str_buf,s_str);};
	if (kb_state.u[1]&KB_U1_BACKSLASH) {strcat(str_buf,"BACKSLASH");strcat(str_buf,s_str);};
	if (kb_state.u[1]&KB_U1_CAPS_LOCK) {strcat(str_buf,"CAPS_LOCK");strcat(str_buf,s_str);};
	if (kb_state.u[1]&KB_U1_TAB) {strcat(str_buf,"TAB");strcat(str_buf,s_str);};
	if (kb_state.u[1]&KB_U1_BACK_SPACE) {strcat(str_buf,"BACK_SPACE");strcat(str_buf,s_str);};
	if (kb_state.u[1]&KB_U1_ESC) {strcat(str_buf,"ESC");strcat(str_buf,s_str);};
	if (kb_state.u[1]&KB_U1_TILDE) {strcat(str_buf,"TILDE");strcat(str_buf,s_str);};
	if (kb_state.u[1]&KB_U1_MENU) {strcat(str_buf,"MENU");strcat(str_buf,s_str);};
	if (kb_state.u[1]&KB_U1_L_SHIFT) {strcat(str_buf,"L_SHIFT");strcat(str_buf,s_str);};
	if (kb_state.u[1]&KB_U1_L_CTRL) {strcat(str_buf,"L_CTRL");strcat(str_buf,s_str);};
	if (kb_state.u[1]&KB_U1_L_ALT) {strcat(str_buf,"L_ALT");strcat(str_buf,s_str);};
	if (kb_state.u[1]&KB_U1_L_WIN) {strcat(str_buf,"L_WIN");strcat(str_buf,s_str);};
	if (kb_state.u[1]&KB_U1_R_SHIFT) {strcat(str_buf,"R_SHIFT");strcat(str_buf,s_str);};
	if (kb_state.u[1]&KB_U1_R_CTRL) {strcat(str_buf,"R_CTRL");strcat(str_buf,s_str);};
	if (kb_state.u[1]&KB_U1_R_ALT) {strcat(str_buf,"R_ALT");strcat(str_buf,s_str);};
	if (kb_state.u[1]&KB_U1_R_WIN) {strcat(str_buf,"R_WIN");strcat(str_buf,s_str);};
	if (kb_state.u[1]&KB_U1_SPACE) {strcat(str_buf,"SPACE");strcat(str_buf,s_str);};
//2 набор
	if (kb_state.u[2]&KB_U2_NUM_0) {strcat(str_buf,"NUM_0");strcat(str_buf,s_str);};
	if (kb_state.u[2]&KB_U2_NUM_1) {strcat(str_buf,"NUM_1");strcat(str_buf,s_str);};
	if (kb_state.u[2]&KB_U2_NUM_2) {strcat(str_buf,"NUM_2");strcat(str_buf,s_str);};
	if (kb_state.u[2]&KB_U2_NUM_3) {strcat(str_buf,"NUM_3");strcat(str_buf,s_str);};
	if (kb_state.u[2]&KB_U2_NUM_4) {strcat(str_buf,"NUM_4");strcat(str_buf,s_str);};
	if (kb_state.u[2]&KB_U2_NUM_5) {strcat(str_buf,"NUM_5");strcat(str_buf,s_str);};
	if (kb_state.u[2]&KB_U2_NUM_6) {strcat(str_buf,"NUM_6");strcat(str_buf,s_str);};
	if (kb_state.u[2]&KB_U2_NUM_7) {strcat(str_buf,"NUM_7");strcat(str_buf,s_str);};
	if (kb_state.u[2]&KB_U2_NUM_8) {strcat(str_buf,"NUM_8");strcat(str_buf,s_str);};
	if (kb_state.u[2]&KB_U2_NUM_9) {strcat(str_buf,"NUM_9");strcat(str_buf,s_str);};
	if (kb_state.u[2]&KB_U2_NUM_ENTER) {strcat(str_buf,"ENTER");strcat(str_buf,s_str);};
	if (kb_state.u[2]&KB_U2_NUM_SLASH) {strcat(str_buf,"/");strcat(str_buf,s_str);};
	if (kb_state.u[2]&KB_U2_NUM_MINUS) {strcat(str_buf,"MINUS");strcat(str_buf,s_str);};
	if (kb_state.u[2]&KB_U2_NUM_PLUS) {strcat(str_buf,"NUM_PLUS");strcat(str_buf,s_str);};
	if (kb_state.u[2]&KB_U2_NUM_MULT) {strcat(str_buf,"NUM_MULT");strcat(str_buf,s_str);};
	if (kb_state.u[2]&KB_U2_NUM_PERIOD) {strcat(str_buf,"NUM_PERIOD");strcat(str_buf,s_str);};
	if (kb_state.u[2]&KB_U2_NUM_LOCK) {strcat(str_buf,"NUM_LOCK");strcat(str_buf,s_str);};
	if (kb_state.u[2]&KB_U2_DELETE) {strcat(str_buf,"DEL");strcat(str_buf,s_str);};
	if (kb_state.u[2]&KB_U2_SCROLL_LOCK) {strcat(str_buf,"SCROLL_LOCK");strcat(str_buf,s_str);};
	if (kb_state.u[2]&KB_U2_PAUSE_BREAK) {strcat(str_buf,"PAUSE_BREAK");strcat(str_buf,s_str);};
	if (kb_state.u[2]&KB_U2_INSERT) {strcat(str_buf,"INSERT");strcat(str_buf,s_str);};
	if (kb_state.u[2]&KB_U2_HOME) {strcat(str_buf,"HOME");strcat(str_buf,s_str);};
	if (kb_state.u[2]&KB_U2_PAGE_UP) {strcat(str_buf,"PG_UP");strcat(str_buf,s_str);};
	if (kb_state.u[2]&KB_U2_PAGE_DOWN) {strcat(str_buf,"PG_DOWN");strcat(str_buf,s_str);};
	if (kb_state.u[2]&KB_U2_PRT_SCR) {strcat(str_buf,"PRT_SCR");strcat(str_buf,s_str);};
	if (kb_state.u[2]&KB_U2_END) {strcat(str_buf,"END");strcat(str_buf,s_str);};
	if (kb_state.u[2]&KB_U2_UP) {strcat(str_buf,"UP");strcat(str_buf,s_str);};
	if (kb_state.u[2]&KB_U2_DOWN) {strcat(str_buf,"DOWN");strcat(str_buf,s_str);};
	if (kb_state.u[2]&KB_U2_LEFT) {strcat(str_buf,"LEFT");strcat(str_buf,s_str);};
	if (kb_state.u[2]&KB_U2_RIGHT) {strcat(str_buf,"RIGHT");strcat(str_buf,s_str);};
//3 набор
	if (kb_state.u[3]&KB_U3_F1) {strcat(str_buf,"F1");strcat(str_buf,s_str);};
	if (kb_state.u[3]&KB_U3_F2) {strcat(str_buf,"F2");strcat(str_buf,s_str);};
	if (kb_state.u[3]&KB_U3_F3) {strcat(str_buf,"F3");strcat(str_buf,s_str);};
	if (kb_state.u[3]&KB_U3_F4) {strcat(str_buf,"F4");strcat(str_buf,s_str);};
	if (kb_state.u[3]&KB_U3_F5) {strcat(str_buf,"F5");strcat(str_buf,s_str);};
	if (kb_state.u[3]&KB_U3_F6) {strcat(str_buf,"F6");strcat(str_buf,s_str);};
	if (kb_state.u[3]&KB_U3_F7) {strcat(str_buf,"F7");strcat(str_buf,s_str);};
	if (kb_state.u[3]&KB_U3_F8) {strcat(str_buf,"F8");strcat(str_buf,s_str);};
	if (kb_state.u[3]&KB_U3_F9) {strcat(str_buf,"F9");strcat(str_buf,s_str);};
	if (kb_state.u[3]&KB_U3_F10) {strcat(str_buf,"F10");strcat(str_buf,s_str);};
	if (kb_state.u[3]&KB_U3_F11) {strcat(str_buf,"F11");strcat(str_buf,s_str);};
	if (kb_state.u[3]&KB_U3_F12) {strcat(str_buf,"F12");strcat(str_buf,s_str);};

	//strcat(str_buf,"\n");
};
