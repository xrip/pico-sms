#include <string.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"

#include "kb_u_codes.h"
#include "util_i2c_kbd.h"
#include "util_i2c_kbd_markup.h"

/*
adress		A0		A1		A2
			-		-		-	0x20
			+		-		-	0x21
			-		+		-	0x22
			+		+		-	0x23
			-		-		+	0x24
			+		-		+	0X25
			-		+		+	0X26
			+		+		+	0x27
*/
 //RP PICO 2040 USB to I2C      0x77
 /*

ibuff[0] какие устройства подключены по USB

    // 0 - нет устройств usb , а на самом деле клавиатура  видна

    // 1 - есть мышь

    // 2 - есть клавиатура 

    // 3 - клавиатура + мышь

    // 4 - клавиатура + мышь + gamepad
ibuff[1] мышь кнопки 
ibuff[2] мышь X
ibuff[3] мышь Y
ibuff[4] джойстик кемпстон пока зарезервированно данные с него можно наверное уже куда то пихать отдает 0xff
ibuff[5 6 7 8 9] резерв
ibuff[10]   по ibuff[25] клавиатура
ibuff[26] ibuff[31] пока оставил для ровного счета можно их не считывать

*/


void i2c_kbd_init(){
	i2c_init(i2c_port, CLOCK_I2C_kHz*1000);
	gpio_set_function(PICO_I2C_SDA_PIN, GPIO_FUNC_I2C);
	gpio_set_function(PICO_I2C_SCL_PIN, GPIO_FUNC_I2C);
	gpio_pull_up(PICO_I2C_SDA_PIN);
	gpio_pull_up(PICO_I2C_SCL_PIN);
	// //printf ("i2c_port=[%8s] SDA = [%d] SCL = [%d]\n", i2c0, PICO_I2C_SDA_PIN, PICO_I2C_SCL_PIN);
};

void i2c_kbd_deinit(){
	i2c_deinit(i2c_port);
	gpio_set_function(PICO_I2C_SDA_PIN, GPIO_FUNC_NULL);
	gpio_set_function(PICO_I2C_SCL_PIN, GPIO_FUNC_NULL);
	gpio_disable_pulls(PICO_I2C_SDA_PIN);
	gpio_disable_pulls(PICO_I2C_SCL_PIN);  
};

int i2c_kbd_data_in(){ 
	int ret=0;
	
	for (uint8_t dev=0;dev<MAX_QNT_DEVICES;dev++){
		if(i2c_dev[dev] == I2C_40_EXT_KEYBOARD){
			val[1] = I2C_NEGATIVE_SCAN;
			for(int i=0; i<8; i++ ){
				int addr = i+(dev*(KEY_ARRAY_SIZE/cnt_dev));
				i2c_write_blocking(i2c_port, i2c_dev[dev], &val[0], 2, false);
				ret = i2c_read_blocking(i2c_port, i2c_dev[dev], &i2c_data[addr], 1, true);
				i2c_data[addr] = ~i2c_data[addr];
				val[1] = 0x01|val[1]<<1;
			}
		}
		if(i2c_dev[dev] == I2C_STRIKE_KEYBOARD){
			val[1] = I2C_POSITIVE_SCAN;
			for(int i=0; i<8; i++ ){
				int addr = i+(dev*(KEY_ARRAY_SIZE/cnt_dev));
				i2c_write_blocking(i2c_port, i2c_dev[dev], &val[0], 2, false);
				ret = i2c_read_blocking(i2c_port, i2c_dev[dev], &i2c_data[addr], 1, true);
				val[1] = val[1]<<1;
			}
		}
		if(i2c_dev[dev] == I2C_MC7007_KEYBOARD){
			for(int i=0; i<2; i++ ){
				uint8_t temp_val ;
				if (i==0) {val[1] = I2C_NEGATIVE_SCAN & 0xbf;} else {val[1] = I2C_NEGATIVE_SCAN & 0x7f;}					
				for(int j=0; j<7 ; j++){					
					int addr = i+j +(dev*(KEY_ARRAY_SIZE/cnt_dev));
					i2c_write_blocking(i2c_port, i2c_dev[dev], &val[0], 2, false);
					ret = i2c_read_blocking(i2c_port, i2c_dev[dev], &i2c_data[addr], 1, true);
					i2c_data[addr] = ~i2c_data[addr];
					val[1] = 0xc1|(val[1]<<1);				
					if (i==0) {val[1] = val[1] & 0xbf;} else {val[1] = val[1] & 0x7f;}

				}
			}
		}
		if(i2c_dev[dev] == I2C_RP2040USB_KEYBOARD){			
				ret = i2c_read_blocking(i2c_port, i2c_dev[dev], &ibuff[0], KEY_ARRAY_SIZE_RP2040, true);			
		};	
	}
	return ret;
};

bool load_markup(kbd_markup* keyboard){

	layout=keyboard;
	return true;
};

bool i2c_scan_devices(){
	

	int ask_device;
	uint8_t nmb_dev = 0;
	cnt_dev = 0;
	bool k;

	for(uint8_t i=I2C_ADDRESS;i<(I2C_ADDRESS+8);i++){
		ask_device=i2c_write_blocking(i2c_port, i, &val[0], 1, true);
		if(ask_device>0){
			i2c_dev[cnt_dev] = i;
			if(i2c_dev[cnt_dev] == I2C_40_EXT_KEYBOARD) {
				load_markup((kbd_markup*)markup_std);
				//printf ("I2C_Device (%1d) address [%02X] Counter [%02X]\n",nmb_dev,i2c_dev[cnt_dev],cnt_dev);
			};
			if(i2c_dev[cnt_dev] == I2C_STRIKE_KEYBOARD) {
				load_markup((kbd_markup*)markup_strike);
				//printf ("I2C_Device (%1d) address [%02X] Counter [%02X]\n",nmb_dev,i2c_dev[cnt_dev],cnt_dev);
			};
			// if(i2c_dev[cnt_dev] == I2C_MC7007_KEYBOARD) {k = load_markup((kbd_markup*)markup_mc7007);};
			cnt_dev++;
		} else {
			i2c_dev[cnt_dev] = 0;
		}
		nmb_dev++;
	}

	ask_device = i2c_read_blocking(i2c_port, I2C_RP2040USB_KEYBOARD, &ibuff[0], 1, true);		// проверка наличия RP2040USBtoI2C по адресу 0х77
	if(ask_device>0){
		i2c_dev[cnt_dev] = I2C_RP2040USB_KEYBOARD;
		//printf ("I2C_Device (%1d) address [%02X] Counter [%02X]\n",nmb_dev,i2c_dev[cnt_dev],cnt_dev);
		cnt_dev++;
	} else {
		i2c_dev[cnt_dev] = 0;
	}

	if (cnt_dev) {return true;}
	return false;
};

void translate_keys(uint8_t* i2c_keys){
	memset(kb_st_ps2.u,0,sizeof(kb_st_ps2.u));
	for(uint8_t dev=0; dev<MAX_QNT_DEVICES;dev++){

		if(i2c_dev[dev]>0){
		// if (layout!=nullptr){
			for (uint8_t key = 0; key < KBD_KEYS_SIZE_STD; key++){
				kbd_markup* keystroke = &layout[key];
				//{.i2c_dev = 1, .i2c_idx=3, .i2c_mask = I2C_KB_1, .kbd_idx=1, .kbd_mask=KB_U1_1},
				
				if(keystroke->i2c_dev==(i2c_dev[dev])){
					uint8_t i2c_addr =(dev*KEY_ARRAY_SIZE)+(keystroke->i2c_idx);
					uint8_t data = (uint8_t)i2c_keys[i2c_addr];
					if(((data&keystroke->i2c_mask)>I2C_KEY_NOT_PRESSED)){
						kb_st_ps2.u[keystroke->kbd_idx]|=(uint32_t)keystroke->kbd_mask;
						////printf(">keystroke[%d]   data[%02X]   kbd_mask[%08X] \n",key,data,(uint32_t)keystroke->kbd_mask);
						//printf("keystroke[%d]->i2c_dev[%02X]->i2c_idx[%02X]->i2c_mask[%02X],i2c_addr[%02X],data[%02X],kbd_idx[%02X]->kbd_mask[%08lX]-kbd_res[%08lX] \n",key,keystroke->i2c_dev,keystroke->i2c_idx,keystroke->i2c_mask,i2c_addr,data,keystroke->kbd_idx,(uint32_t)keystroke->kbd_mask,kb_st_ps2.u[keystroke->kbd_idx]);
					} 
				}
			}
		}
	}
	kb_st_ps2.u[0] |= (uint32_t)ibuff[13] | ((uint32_t)ibuff[12]<<8) | ((uint32_t)ibuff[11]<<16) | ((uint32_t)ibuff[10]<<24);
	kb_st_ps2.u[1] |= (uint32_t)ibuff[17] | ((uint32_t)ibuff[16]<<8) | ((uint32_t)ibuff[15]<<16) | ((uint32_t)ibuff[14]<<24);
	kb_st_ps2.u[2] |= (uint32_t)ibuff[21] | ((uint32_t)ibuff[20]<<8) | ((uint32_t)ibuff[19]<<16) | ((uint32_t)ibuff[18]<<24);
	kb_st_ps2.u[3] |= (uint32_t)ibuff[25] | ((uint32_t)ibuff[24]<<8) | ((uint32_t)ibuff[23]<<16) | ((uint32_t)ibuff[22]<<24);	
}

static uint32_t pressed_timeout;
static uint32_t repeat_timeout ;

bool i2c_decode_kbd(){

	state = i2c_kbd_data_in();
	scan = 0;
	for(int i=0;i<KEY_ARRAY_SIZE;i++){scan |= i2c_data[i];}		
	for(int i=10;i<26;i++){scan |= ibuff[i];}

	if(scan==0){
		pressed_timeout = 0;
		repeat_timeout = 0;
	}
	if ((scan != oldscan)) {
		// //printf("scan = %02X oldscan = %02X \n" ,scan,oldscan);
		translate_keys(i2c_data);
		oldscan = scan;
		pressed_timeout = time_us_64();
		return true;
	}
	if((scan>0)&&(pressed_timeout>0)&&((time_us_64()-pressed_timeout)>I2C_KEY_PRESSED_TIMEOUT)){
		pressed_timeout = 0;
		repeat_timeout = time_us_64();
		translate_keys(i2c_data);
		return true;
	}
	if((scan>0)&&(repeat_timeout>0)&&((time_us_64()-repeat_timeout)>I2C_KEY_REPEAT_TIMEOUT)){
		repeat_timeout = time_us_64();
		translate_keys(i2c_data);
		return true;
	}
	oldscan = scan;
	return false;	
};

bool i2c_kbd_start(){
	bool is_present;
	val[0] = 0xff;
	val[1] = 0xfe;
	// load_markup((kbd_markup*)default_markup_std);
	memset(i2c_dev,0,MAX_QNT_DEVICES);
	memset(i2c_data,0,KEY_ARRAY_SIZE*MAX_QNT_DEVICES);
	i2c_kbd_init();
	if(i2c_scan_devices()){is_present=true;} else {is_present=false;};
	return is_present;
}; 