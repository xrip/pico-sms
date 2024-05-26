#pragma once
#include "inttypes.h"
#include "stdbool.h"

typedef struct kb_u_state{
   uint32_t u[4];
   uint8_t state;
}kb_u_state;

void convert_kb_u_to_kb_zx(kb_u_state* kb_st,uint8_t* zx_kb);
uint8_t convert_kb_u_to_char(char* str_buf, kb_u_state kb_st);
//для тестирования
void keys_to_str(char* str_buf,char s_char,kb_u_state kb_state);


// блок 0 буквы
#define KB_U0_A (1<<0)
#define KB_U0_B (1<<1)
#define KB_U0_C (1<<2)
#define KB_U0_D (1<<3)
#define KB_U0_E (1<<4)
#define KB_U0_F (1<<5)
#define KB_U0_G (1<<6)
#define KB_U0_H (1<<7)
#define KB_U0_I (1<<8)
#define KB_U0_J (1<<9)
#define KB_U0_K (1<<10)
#define KB_U0_L (1<<11)
#define KB_U0_M (1<<12)
#define KB_U0_N (1<<13)
#define KB_U0_O (1<<14)
#define KB_U0_P (1<<15)
#define KB_U0_Q (1<<16)
#define KB_U0_R (1<<17)
#define KB_U0_S (1<<18)
#define KB_U0_T (1<<19)
#define KB_U0_U (1<<20)
#define KB_U0_V (1<<21)
#define KB_U0_W (1<<22)
#define KB_U0_X (1<<23)
#define KB_U0_Y (1<<24)
#define KB_U0_Z (1<<25)

#define KB_U0_SEMICOLON (1<<26)
#define KB_U0_QUOTE (1<<27)
#define KB_U0_COMMA (1<<28)
#define KB_U0_PERIOD (1<<29)
#define KB_U0_LEFT_BR (1<<30)
#define KB_U0_RIGHT_BR (1<<31)

// блок 1 цифры и контролы


#define KB_U1_0 (1<<0)
#define KB_U1_1 (1<<1)
#define KB_U1_2 (1<<2)
#define KB_U1_3 (1<<3)
#define KB_U1_4 (1<<4)
#define KB_U1_5 (1<<5)
#define KB_U1_6 (1<<6)
#define KB_U1_7 (1<<7)
#define KB_U1_8 (1<<8)
#define KB_U1_9 (1<<9)

#define KB_U1_ENTER (1<<10)
#define KB_U1_SLASH (1<<11)
#define KB_U1_MINUS (1<<12)

#define KB_U1_EQUALS (1<<13)
#define KB_U1_BACKSLASH (1<<14)
#define KB_U1_CAPS_LOCK (1<<15)
#define KB_U1_TAB (1<<16)
#define KB_U1_BACK_SPACE (1<<17)
#define KB_U1_ESC (1<<18)
#define KB_U1_TILDE (1<<19)
#define KB_U1_MENU (1<<20)

#define KB_U1_L_SHIFT (1<<21)
#define KB_U1_L_CTRL (1<<22)
#define KB_U1_L_ALT (1<<23)
#define KB_U1_L_WIN (1<<24)
#define KB_U1_R_SHIFT (1<<25)
#define KB_U1_R_CTRL (1<<26)
#define KB_U1_R_ALT (1<<27)
#define KB_U1_R_WIN (1<<28)

#define KB_U1_SPACE (1<<29)




// блок 2 нум клавиши и доп клавиши



#define KB_U2_NUM_0 (1<<0)
#define KB_U2_NUM_1 (1<<1)
#define KB_U2_NUM_2 (1<<2)
#define KB_U2_NUM_3 (1<<3)
#define KB_U2_NUM_4 (1<<4)
#define KB_U2_NUM_5 (1<<5)
#define KB_U2_NUM_6 (1<<6)
#define KB_U2_NUM_7 (1<<7)
#define KB_U2_NUM_8 (1<<8)
#define KB_U2_NUM_9 (1<<9)

#define KB_U2_NUM_ENTER (1<<10)
#define KB_U2_NUM_SLASH (1<<11)
#define KB_U2_NUM_MINUS (1<<12)

#define KB_U2_NUM_PLUS (1<<13)
#define KB_U2_NUM_MULT (1<<14)
#define KB_U2_NUM_PERIOD  (1<<15)
#define KB_U2_NUM_LOCK (1<<16)


#define KB_U2_DELETE (1<<17)
#define KB_U2_SCROLL_LOCK (1<<18)
#define KB_U2_PAUSE_BREAK (1<<19)
#define KB_U2_INSERT (1<<20)
#define KB_U2_HOME (1<<21)
#define KB_U2_PAGE_UP (1<<22)
#define KB_U2_PAGE_DOWN (1<<23)

#define KB_U2_PRT_SCR (1<<24)
#define KB_U2_END (1<<25)
#define KB_U2_UP (1<<26)
#define KB_U2_DOWN (1<<27)
#define KB_U2_LEFT (1<<28)
#define KB_U2_RIGHT (1<<29)

// блок 3 F клавиши и прочие допы


#define KB_U3_ (1<<0)
#define KB_U3_F1 (1<<1)
#define KB_U3_F2 (1<<2)
#define KB_U3_F3 (1<<3)
#define KB_U3_F4 (1<<4)
#define KB_U3_F5 (1<<5)
#define KB_U3_F6 (1<<6)
#define KB_U3_F7 (1<<7)
#define KB_U3_F8 (1<<8)
#define KB_U3_F9 (1<<9)
#define KB_U3_F10 (1<<10)
#define KB_U3_F11 (1<<11)
#define KB_U3_F12 (1<<12)
