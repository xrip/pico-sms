	if (i2c_kbd_start()){i2cKbdMode = true;}else{i2cKbdMode = false;}

	short int i2c_state = i2c_kbd_data_in();
	//printf ("i2c_state: %d\n",i2c_state);

	if(!i2cKbdMode) {
		//printf ("i2c Keyboard not connected\n");
		i2c_kbd_deinit();	
		busy_wait_ms(100);
		start_PS2_capture();
		printf ("PS/2 Keyboard Started\n");
	} else {
		printf ("I2C Keyboard Started\n");
	}
