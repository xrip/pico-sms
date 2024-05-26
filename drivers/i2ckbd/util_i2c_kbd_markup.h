#pragma once
#include "inttypes.h"
#include "util_i2c_kbd.h"
#include "kb_u_codes.h"
#include "util_i2c_kbd_mask.h"
#include "util_i2c_kbd_mask_strike.h"

#define KBD_KEYS_SIZE_STD (68)
#define KBD_KEYS_SIZE_STRIKE (65)

const kbd_markup markup_std[KBD_KEYS_SIZE_STD]={
{	.i2c_dev = 0x20, 	.i2c_idx=3, 	.i2c_mask = I2C_KB_1, 		.kbd_idx=1, 	.kbd_mask=KB_U1_1},				//0
{	.i2c_dev = 0x20, 	.i2c_idx=3, 	.i2c_mask = I2C_KB_2, 		.kbd_idx=1, 	.kbd_mask=KB_U1_2},            	//1
{	.i2c_dev = 0x20, 	.i2c_idx=3, 	.i2c_mask = I2C_KB_3, 		.kbd_idx=1, 	.kbd_mask=KB_U1_3},            	//2
{	.i2c_dev = 0x20, 	.i2c_idx=3, 	.i2c_mask = I2C_KB_4, 		.kbd_idx=1, 	.kbd_mask=KB_U1_4},            	//3
{	.i2c_dev = 0x20, 	.i2c_idx=3, 	.i2c_mask = I2C_KB_5, 		.kbd_idx=1, 	.kbd_mask=KB_U1_5},            	//4
{	.i2c_dev = 0x20, 	.i2c_idx=4, 	.i2c_mask = I2C_KB_6, 		.kbd_idx=1, 	.kbd_mask=KB_U1_6},            	//5
{	.i2c_dev = 0x20, 	.i2c_idx=4, 	.i2c_mask = I2C_KB_7, 		.kbd_idx=1, 	.kbd_mask=KB_U1_7},            	//6
{	.i2c_dev = 0x20, 	.i2c_idx=4, 	.i2c_mask = I2C_KB_8, 		.kbd_idx=1, 	.kbd_mask=KB_U1_8},            	//7
{	.i2c_dev = 0x20, 	.i2c_idx=4, 	.i2c_mask = I2C_KB_9, 		.kbd_idx=1, 	.kbd_mask=KB_U1_9},            	//8
{	.i2c_dev = 0x20, 	.i2c_idx=4, 	.i2c_mask = I2C_KB_0, 		.kbd_idx=1, 	.kbd_mask=KB_U1_0},            	//9
{	.i2c_dev = 0x20, 	.i2c_idx=2, 	.i2c_mask = I2C_KB_Q, 		.kbd_idx=0, 	.kbd_mask=KB_U0_Q},            	//10
{	.i2c_dev = 0x20, 	.i2c_idx=2, 	.i2c_mask = I2C_KB_W, 		.kbd_idx=0, 	.kbd_mask=KB_U0_W},            	//11
{	.i2c_dev = 0x20, 	.i2c_idx=2, 	.i2c_mask = I2C_KB_E, 		.kbd_idx=0, 	.kbd_mask=KB_U0_E},            	//12
{	.i2c_dev = 0x20, 	.i2c_idx=2, 	.i2c_mask = I2C_KB_R, 		.kbd_idx=0, 	.kbd_mask=KB_U0_R},            	//13
{	.i2c_dev = 0x20, 	.i2c_idx=2, 	.i2c_mask = I2C_KB_T, 		.kbd_idx=0, 	.kbd_mask=KB_U0_T},            	//14
{	.i2c_dev = 0x20, 	.i2c_idx=5, 	.i2c_mask = I2C_KB_Y, 		.kbd_idx=0, 	.kbd_mask=KB_U0_Y},            	//15
{	.i2c_dev = 0x20, 	.i2c_idx=5, 	.i2c_mask = I2C_KB_U, 		.kbd_idx=0, 	.kbd_mask=KB_U0_U},            	//16
{	.i2c_dev = 0x20, 	.i2c_idx=5, 	.i2c_mask = I2C_KB_I, 		.kbd_idx=0, 	.kbd_mask=KB_U0_I},            	//17
{	.i2c_dev = 0x20, 	.i2c_idx=5, 	.i2c_mask = I2C_KB_O, 		.kbd_idx=0, 	.kbd_mask=KB_U0_O},            	//18
{	.i2c_dev = 0x20, 	.i2c_idx=5, 	.i2c_mask = I2C_KB_P, 		.kbd_idx=0, 	.kbd_mask=KB_U0_P},            	//19
{	.i2c_dev = 0x20, 	.i2c_idx=1, 	.i2c_mask = I2C_KB_A, 		.kbd_idx=0, 	.kbd_mask=KB_U0_A},            	//20
{	.i2c_dev = 0x20, 	.i2c_idx=1, 	.i2c_mask = I2C_KB_S, 		.kbd_idx=0, 	.kbd_mask=KB_U0_S},            	//21
{	.i2c_dev = 0x20, 	.i2c_idx=1, 	.i2c_mask = I2C_KB_D, 		.kbd_idx=0, 	.kbd_mask=KB_U0_D},            	//22
{	.i2c_dev = 0x20, 	.i2c_idx=1, 	.i2c_mask = I2C_KB_F, 		.kbd_idx=0, 	.kbd_mask=KB_U0_F},            	//23
{	.i2c_dev = 0x20, 	.i2c_idx=1, 	.i2c_mask = I2C_KB_G, 		.kbd_idx=0, 	.kbd_mask=KB_U0_G},            	//24
{	.i2c_dev = 0x20, 	.i2c_idx=6, 	.i2c_mask = I2C_KB_H, 		.kbd_idx=0, 	.kbd_mask=KB_U0_H},           	//25
{	.i2c_dev = 0x20, 	.i2c_idx=6, 	.i2c_mask = I2C_KB_J, 		.kbd_idx=0, 	.kbd_mask=KB_U0_J},           	//26
{	.i2c_dev = 0x20, 	.i2c_idx=6, 	.i2c_mask = I2C_KB_K, 		.kbd_idx=0, 	.kbd_mask=KB_U0_K},           	//27
{	.i2c_dev = 0x20, 	.i2c_idx=6, 	.i2c_mask = I2C_KB_L, 		.kbd_idx=0, 	.kbd_mask=KB_U0_L},           	//28
{	.i2c_dev = 0x20, 	.i2c_idx=6, 	.i2c_mask = I2C_KB_ENTER, 	.kbd_idx=1, 	.kbd_mask=KB_U1_ENTER},    		//29
{	.i2c_dev = 0x20, 	.i2c_idx=0, 	.i2c_mask = I2C_KB_CS, 		.kbd_idx=1, 	.kbd_mask=KB_U1_L_SHIFT},     	//30
{	.i2c_dev = 0x20, 	.i2c_idx=0, 	.i2c_mask = I2C_KB_Z, 		.kbd_idx=0, 	.kbd_mask=KB_U0_Z},           	//31
{	.i2c_dev = 0x20, 	.i2c_idx=0, 	.i2c_mask = I2C_KB_X, 		.kbd_idx=0, 	.kbd_mask=KB_U0_X},           	//32
{	.i2c_dev = 0x20, 	.i2c_idx=0, 	.i2c_mask = I2C_KB_C, 		.kbd_idx=0, 	.kbd_mask=KB_U0_C},           	//33
{	.i2c_dev = 0x20, 	.i2c_idx=0, 	.i2c_mask = I2C_KB_V, 		.kbd_idx=0, 	.kbd_mask=KB_U0_V},           	//34
{	.i2c_dev = 0x20, 	.i2c_idx=7, 	.i2c_mask = I2C_KB_B, 		.kbd_idx=0, 	.kbd_mask=KB_U0_B},           	//35
{	.i2c_dev = 0x20, 	.i2c_idx=7, 	.i2c_mask = I2C_KB_N, 		.kbd_idx=0, 	.kbd_mask=KB_U0_N},           	//36
{	.i2c_dev = 0x20, 	.i2c_idx=7, 	.i2c_mask = I2C_KB_M, 		.kbd_idx=0, 	.kbd_mask=KB_U0_M},           	//37
{	.i2c_dev = 0x20, 	.i2c_idx=7, 	.i2c_mask = I2C_KB_SS,		.kbd_idx=1, 	.kbd_mask=KB_U1_L_CTRL},      	//38
{	.i2c_dev = 0x20, 	.i2c_idx=7, 	.i2c_mask = I2C_KB_SPC,		.kbd_idx=1, 	.kbd_mask=KB_U1_SPACE},      	//39
{	.i2c_dev = 0x20, 	.i2c_idx=0, 	.i2c_mask = I2C_KB_RES, 	.kbd_idx=1, 	.kbd_mask=KB_U1_L_CTRL},     	//40
{	.i2c_dev = 0x20, 	.i2c_idx=0, 	.i2c_mask = I2C_KB_RES, 	.kbd_idx=1, 	.kbd_mask=KB_U1_L_ALT},      	//41
{	.i2c_dev = 0x20, 	.i2c_idx=0, 	.i2c_mask = I2C_KB_RES, 	.kbd_idx=2, 	.kbd_mask=KB_U2_DELETE},     	//42
{	.i2c_dev = 0x20, 	.i2c_idx=0, 	.i2c_mask = I2C_KB_NMI, 	.kbd_idx=1, 	.kbd_mask=KB_U1_L_CTRL},     	//43
{	.i2c_dev = 0x20, 	.i2c_idx=0, 	.i2c_mask = I2C_KB_NMI, 	.kbd_idx=1, 	.kbd_mask=KB_U1_L_ALT},      	//44
{	.i2c_dev = 0x20, 	.i2c_idx=0, 	.i2c_mask = I2C_KB_NMI, 	.kbd_idx=2, 	.kbd_mask=KB_U2_INSERT},     	//45
{	.i2c_dev = 0x20, 	.i2c_idx=0, 	.i2c_mask = I2C_KB_WIN, 	.kbd_idx=1, 	.kbd_mask=KB_U1_L_WIN},      	//46
{	.i2c_dev = 0x20, 	.i2c_idx=1, 	.i2c_mask = I2C_KB_LEFT, 	.kbd_idx=2, 	.kbd_mask=KB_U2_LEFT},    		//47
{	.i2c_dev = 0x20, 	.i2c_idx=1, 	.i2c_mask = I2C_KB_RIGHT, 	.kbd_idx=2, 	.kbd_mask=KB_U2_RIGHT},   		//48
{	.i2c_dev = 0x20, 	.i2c_idx=1, 	.i2c_mask = I2C_KB_UP,	 	.kbd_idx=2, 	.kbd_mask=KB_U2_UP},      		//49
{	.i2c_dev = 0x20, 	.i2c_idx=2, 	.i2c_mask = I2C_KB_DOWN, 	.kbd_idx=2, 	.kbd_mask=KB_U2_DOWN},    		//50
{	.i2c_dev = 0x20, 	.i2c_idx=2, 	.i2c_mask = I2C_KB_FIRE, 	.kbd_idx=1, 	.kbd_mask=KB_U1_R_ALT},   		//51
{	.i2c_dev = 0x20, 	.i2c_idx=2, 	.i2c_mask = I2C_KB_CTRL, 	.kbd_idx=1, 	.kbd_mask=KB_U1_L_CTRL},  		//52
{	.i2c_dev = 0x20, 	.i2c_idx=3, 	.i2c_mask = I2C_KB_ALT, 	.kbd_idx=1, 	.kbd_mask=KB_U1_L_ALT},   		//53
{	.i2c_dev = 0x20, 	.i2c_idx=3, 	.i2c_mask = I2C_KB_PG_UP, 	.kbd_idx=2, 	.kbd_mask=KB_U2_PAGE_UP}, 		//54
{	.i2c_dev = 0x20, 	.i2c_idx=3, 	.i2c_mask = I2C_KB_PG_DN, 	.kbd_idx=2, 	.kbd_mask=KB_U2_PAGE_DOWN},		//55
{	.i2c_dev = 0x20, 	.i2c_idx=4, 	.i2c_mask = I2C_KB_F1, 		.kbd_idx=3, 	.kbd_mask=KB_U3_F1},      		//56
{	.i2c_dev = 0x20, 	.i2c_idx=4, 	.i2c_mask = I2C_KB_F2,	 	.kbd_idx=3, 	.kbd_mask=KB_U3_F2},      		//57
{	.i2c_dev = 0x20, 	.i2c_idx=4, 	.i2c_mask = I2C_KB_F3,	 	.kbd_idx=3, 	.kbd_mask=KB_U3_F3},      		//58
{	.i2c_dev = 0x20, 	.i2c_idx=5, 	.i2c_mask = I2C_KB_F4,	 	.kbd_idx=3, 	.kbd_mask=KB_U3_F4},      		//59
{	.i2c_dev = 0x20, 	.i2c_idx=5, 	.i2c_mask = I2C_KB_F5,	 	.kbd_idx=3, 	.kbd_mask=KB_U3_F5},      		//60
{	.i2c_dev = 0x20, 	.i2c_idx=5, 	.i2c_mask = I2C_KB_F6,	 	.kbd_idx=3, 	.kbd_mask=KB_U3_F6},      		//61
{	.i2c_dev = 0x20, 	.i2c_idx=6, 	.i2c_mask = I2C_KB_F7,	 	.kbd_idx=3, 	.kbd_mask=KB_U3_F7},      		//62
{	.i2c_dev = 0x20, 	.i2c_idx=6, 	.i2c_mask = I2C_KB_F8,	 	.kbd_idx=3, 	.kbd_mask=KB_U3_F8},      		//63
{	.i2c_dev = 0x20, 	.i2c_idx=6, 	.i2c_mask = I2C_KB_F9,	 	.kbd_idx=3, 	.kbd_mask=KB_U3_F9},      		//64
{	.i2c_dev = 0x20, 	.i2c_idx=7, 	.i2c_mask = I2C_KB_F10, 	.kbd_idx=3, 	.kbd_mask=KB_U3_F10},     		//65
{	.i2c_dev = 0x20, 	.i2c_idx=7, 	.i2c_mask = I2C_KB_F11, 	.kbd_idx=3, 	.kbd_mask=KB_U3_F11},     		//66
{	.i2c_dev = 0x20, 	.i2c_idx=7, 	.i2c_mask = I2C_KB_F12, 	.kbd_idx=3, 	.kbd_mask=KB_U3_F12},     		//67
};

const kbd_markup markup_strike[KBD_KEYS_SIZE_STRIKE]={
{	.i2c_dev = 0x21, 	.i2c_idx=3, 	.i2c_mask = I2C_STRIKE_1, 		.kbd_idx=1, 	.kbd_mask=KB_U1_1},				//0
{	.i2c_dev = 0x21, 	.i2c_idx=3, 	.i2c_mask = I2C_STRIKE_2, 		.kbd_idx=1, 	.kbd_mask=KB_U1_2},            	//1
{	.i2c_dev = 0x21, 	.i2c_idx=3, 	.i2c_mask = I2C_STRIKE_3, 		.kbd_idx=1, 	.kbd_mask=KB_U1_3},            	//2
{	.i2c_dev = 0x21, 	.i2c_idx=3, 	.i2c_mask = I2C_STRIKE_4, 		.kbd_idx=1, 	.kbd_mask=KB_U1_4},            	//3
{	.i2c_dev = 0x21, 	.i2c_idx=3, 	.i2c_mask = I2C_STRIKE_5, 		.kbd_idx=1, 	.kbd_mask=KB_U1_5},            	//4
{	.i2c_dev = 0x21, 	.i2c_idx=4, 	.i2c_mask = I2C_STRIKE_6, 		.kbd_idx=1, 	.kbd_mask=KB_U1_6},            	//5
{	.i2c_dev = 0x21, 	.i2c_idx=4, 	.i2c_mask = I2C_STRIKE_7, 		.kbd_idx=1, 	.kbd_mask=KB_U1_7},            	//6
{	.i2c_dev = 0x21, 	.i2c_idx=4, 	.i2c_mask = I2C_STRIKE_8, 		.kbd_idx=1, 	.kbd_mask=KB_U1_8},            	//7
{	.i2c_dev = 0x21, 	.i2c_idx=4, 	.i2c_mask = I2C_STRIKE_9, 		.kbd_idx=1, 	.kbd_mask=KB_U1_9},            	//8
{	.i2c_dev = 0x21, 	.i2c_idx=4, 	.i2c_mask = I2C_STRIKE_0, 		.kbd_idx=1, 	.kbd_mask=KB_U1_0},            	//9
{	.i2c_dev = 0x21, 	.i2c_idx=2, 	.i2c_mask = I2C_STRIKE_Q, 		.kbd_idx=0, 	.kbd_mask=KB_U0_Q},            	//10
{	.i2c_dev = 0x21, 	.i2c_idx=2, 	.i2c_mask = I2C_STRIKE_W, 		.kbd_idx=0, 	.kbd_mask=KB_U0_W},            	//11
{	.i2c_dev = 0x21, 	.i2c_idx=2, 	.i2c_mask = I2C_STRIKE_E, 		.kbd_idx=0, 	.kbd_mask=KB_U0_E},            	//12
{	.i2c_dev = 0x21, 	.i2c_idx=2, 	.i2c_mask = I2C_STRIKE_R, 		.kbd_idx=0, 	.kbd_mask=KB_U0_R},            	//13
{	.i2c_dev = 0x21, 	.i2c_idx=2, 	.i2c_mask = I2C_STRIKE_T, 		.kbd_idx=0, 	.kbd_mask=KB_U0_T},            	//14
{	.i2c_dev = 0x21, 	.i2c_idx=5, 	.i2c_mask = I2C_STRIKE_Y, 		.kbd_idx=0, 	.kbd_mask=KB_U0_Y},            	//15
{	.i2c_dev = 0x21, 	.i2c_idx=5, 	.i2c_mask = I2C_STRIKE_U, 		.kbd_idx=0, 	.kbd_mask=KB_U0_U},            	//16
{	.i2c_dev = 0x21, 	.i2c_idx=5, 	.i2c_mask = I2C_STRIKE_I, 		.kbd_idx=0, 	.kbd_mask=KB_U0_I},            	//17
{	.i2c_dev = 0x21, 	.i2c_idx=5, 	.i2c_mask = I2C_STRIKE_O, 		.kbd_idx=0, 	.kbd_mask=KB_U0_O},            	//18
{	.i2c_dev = 0x21, 	.i2c_idx=5, 	.i2c_mask = I2C_STRIKE_P, 		.kbd_idx=0, 	.kbd_mask=KB_U0_P},            	//19
{	.i2c_dev = 0x21, 	.i2c_idx=1, 	.i2c_mask = I2C_STRIKE_A, 		.kbd_idx=0, 	.kbd_mask=KB_U0_A},            	//20
{	.i2c_dev = 0x21, 	.i2c_idx=1, 	.i2c_mask = I2C_STRIKE_S, 		.kbd_idx=0, 	.kbd_mask=KB_U0_S},            	//21
{	.i2c_dev = 0x21, 	.i2c_idx=1, 	.i2c_mask = I2C_STRIKE_D, 		.kbd_idx=0, 	.kbd_mask=KB_U0_D},            	//22
{	.i2c_dev = 0x21, 	.i2c_idx=1, 	.i2c_mask = I2C_STRIKE_F, 		.kbd_idx=0, 	.kbd_mask=KB_U0_F},            	//23
{	.i2c_dev = 0x21, 	.i2c_idx=1, 	.i2c_mask = I2C_STRIKE_G, 		.kbd_idx=0, 	.kbd_mask=KB_U0_G},            	//24
{	.i2c_dev = 0x21, 	.i2c_idx=6, 	.i2c_mask = I2C_STRIKE_H, 		.kbd_idx=0, 	.kbd_mask=KB_U0_H},           	//25
{	.i2c_dev = 0x21, 	.i2c_idx=6, 	.i2c_mask = I2C_STRIKE_J, 		.kbd_idx=0, 	.kbd_mask=KB_U0_J},           	//26
{	.i2c_dev = 0x21, 	.i2c_idx=6, 	.i2c_mask = I2C_STRIKE_K, 		.kbd_idx=0, 	.kbd_mask=KB_U0_K},           	//27
{	.i2c_dev = 0x21, 	.i2c_idx=6, 	.i2c_mask = I2C_STRIKE_L, 		.kbd_idx=0, 	.kbd_mask=KB_U0_L},           	//28
{	.i2c_dev = 0x21, 	.i2c_idx=6, 	.i2c_mask = I2C_STRIKE_ENTER, 	.kbd_idx=1, 	.kbd_mask=KB_U1_ENTER},    		//29
{	.i2c_dev = 0x21, 	.i2c_idx=0, 	.i2c_mask = I2C_STRIKE_SHIFT_L,	.kbd_idx=1, 	.kbd_mask=KB_U1_L_SHIFT},     	//30
{	.i2c_dev = 0x21, 	.i2c_idx=0, 	.i2c_mask = I2C_STRIKE_Z, 		.kbd_idx=0, 	.kbd_mask=KB_U0_Z},           	//31
{	.i2c_dev = 0x21, 	.i2c_idx=0, 	.i2c_mask = I2C_STRIKE_X, 		.kbd_idx=0, 	.kbd_mask=KB_U0_X},           	//32
{	.i2c_dev = 0x21, 	.i2c_idx=0, 	.i2c_mask = I2C_STRIKE_C, 		.kbd_idx=0, 	.kbd_mask=KB_U0_C},           	//33
{	.i2c_dev = 0x21, 	.i2c_idx=0, 	.i2c_mask = I2C_STRIKE_V, 		.kbd_idx=0, 	.kbd_mask=KB_U0_V},           	//34
{	.i2c_dev = 0x21, 	.i2c_idx=7, 	.i2c_mask = I2C_STRIKE_B, 		.kbd_idx=0, 	.kbd_mask=KB_U0_B},           	//35
{	.i2c_dev = 0x21, 	.i2c_idx=7, 	.i2c_mask = I2C_STRIKE_N, 		.kbd_idx=0, 	.kbd_mask=KB_U0_N},           	//36
{	.i2c_dev = 0x21, 	.i2c_idx=7, 	.i2c_mask = I2C_STRIKE_M, 		.kbd_idx=0, 	.kbd_mask=KB_U0_M},           	//37
{	.i2c_dev = 0x21, 	.i2c_idx=7, 	.i2c_mask = I2C_STRIKE_SHIFT_R,	.kbd_idx=1, 	.kbd_mask=KB_U1_R_SHIFT},      	//38
{	.i2c_dev = 0x21, 	.i2c_idx=7, 	.i2c_mask = I2C_STRIKE_SPC,		.kbd_idx=1, 	.kbd_mask=KB_U1_SPACE},      	//39
{	.i2c_dev = 0x21, 	.i2c_idx=3, 	.i2c_mask = I2C_STRIKE_RES, 	.kbd_idx=1, 	.kbd_mask=KB_U1_L_CTRL},     	//40
{	.i2c_dev = 0x21, 	.i2c_idx=3, 	.i2c_mask = I2C_STRIKE_RES, 	.kbd_idx=1, 	.kbd_mask=KB_U1_L_ALT},      	//41
{	.i2c_dev = 0x21, 	.i2c_idx=3, 	.i2c_mask = I2C_STRIKE_RES, 	.kbd_idx=2, 	.kbd_mask=KB_U2_DELETE},     	//42
{	.i2c_dev = 0x21, 	.i2c_idx=3, 	.i2c_mask = I2C_STRIKE_NMI, 	.kbd_idx=1, 	.kbd_mask=KB_U1_L_CTRL},     	//43
{	.i2c_dev = 0x21, 	.i2c_idx=3, 	.i2c_mask = I2C_STRIKE_NMI, 	.kbd_idx=1, 	.kbd_mask=KB_U1_L_ALT},      	//44
{	.i2c_dev = 0x21, 	.i2c_idx=3, 	.i2c_mask = I2C_STRIKE_NMI, 	.kbd_idx=2, 	.kbd_mask=KB_U2_INSERT},     	//45
{	.i2c_dev = 0x21, 	.i2c_idx=2, 	.i2c_mask = I2C_STRIKE_WIN, 	.kbd_idx=1, 	.kbd_mask=KB_U1_L_WIN},      	//46
{	.i2c_dev = 0x21, 	.i2c_idx=1, 	.i2c_mask = I2C_STRIKE_LEFT, 	.kbd_idx=2, 	.kbd_mask=KB_U2_LEFT},    		//47
{	.i2c_dev = 0x21, 	.i2c_idx=0, 	.i2c_mask = I2C_STRIKE_RIGHT, 	.kbd_idx=2, 	.kbd_mask=KB_U2_RIGHT},   		//48
{	.i2c_dev = 0x21, 	.i2c_idx=7, 	.i2c_mask = I2C_STRIKE_UP,	 	.kbd_idx=2, 	.kbd_mask=KB_U2_UP},      		//49
{	.i2c_dev = 0x21, 	.i2c_idx=1, 	.i2c_mask = I2C_STRIKE_DOWN, 	.kbd_idx=2, 	.kbd_mask=KB_U2_DOWN},    		//50
{	.i2c_dev = 0x21, 	.i2c_idx=0, 	.i2c_mask = I2C_STRIKE_FN, 		.kbd_idx=1, 	.kbd_mask=KB_U1_L_CTRL},   		//51
{	.i2c_dev = 0x21, 	.i2c_idx=0, 	.i2c_mask = I2C_STRIKE_FN, 		.kbd_idx=3, 	.kbd_mask=KB_U3_F11},   		//52
{	.i2c_dev = 0x21, 	.i2c_idx=2, 	.i2c_mask = I2C_STRIKE_CTRL, 	.kbd_idx=1, 	.kbd_mask=KB_U1_L_CTRL},  		//53
{	.i2c_dev = 0x21, 	.i2c_idx=7, 	.i2c_mask = I2C_STRIKE_ALT, 	.kbd_idx=1, 	.kbd_mask=KB_U1_L_ALT},   		//54
{	.i2c_dev = 0x21, 	.i2c_idx=7, 	.i2c_mask = I2C_STRIKE_COMMA,	.kbd_idx=0, 	.kbd_mask=KB_U0_COMMA},    		//55
{	.i2c_dev = 0x21, 	.i2c_idx=6, 	.i2c_mask = I2C_STRIKE_SMCLN, 	.kbd_idx=0, 	.kbd_mask=KB_U0_SEMICOLON},  	//56
{	.i2c_dev = 0x21, 	.i2c_idx=6, 	.i2c_mask = I2C_STRIKE_QUOTE, 	.kbd_idx=0, 	.kbd_mask=KB_U0_QUOTE},    		//57
{	.i2c_dev = 0x21, 	.i2c_idx=5, 	.i2c_mask = I2C_STRIKE_F7,	 	.kbd_idx=0, 	.kbd_mask=KB_U0_LEFT_BR},   	//58
{	.i2c_dev = 0x21, 	.i2c_idx=5, 	.i2c_mask = I2C_STRIKE_F8,	 	.kbd_idx=0, 	.kbd_mask=KB_U0_RIGHT_BR},  	//59
{	.i2c_dev = 0x21, 	.i2c_idx=3, 	.i2c_mask = I2C_STRIKE_ESC,	 	.kbd_idx=1, 	.kbd_mask=KB_U1_ESC},      		//60
{	.i2c_dev = 0x21, 	.i2c_idx=4, 	.i2c_mask = I2C_STRIKE_BACKSPC,	.kbd_idx=1, 	.kbd_mask=KB_U1_BACK_SPACE},	//61
{	.i2c_dev = 0x21, 	.i2c_idx=4, 	.i2c_mask = I2C_STRIKE_MINUS, 	.kbd_idx=1, 	.kbd_mask=KB_U1_MINUS},    		//62
{	.i2c_dev = 0x21, 	.i2c_idx=4, 	.i2c_mask = I2C_STRIKE_PLUS, 	.kbd_idx=2, 	.kbd_mask=KB_U2_NUM_PLUS},   	//63
{	.i2c_dev = 0x21, 	.i2c_idx=2, 	.i2c_mask = I2C_STRIKE_TAB,		.kbd_idx=1, 	.kbd_mask=KB_U1_TAB},      		//64
};  