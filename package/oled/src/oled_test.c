/******
Demo for ssd1306 i2c driver for  Raspberry Pi 
******/

#include "ssd1306_i2c.h"

void main() {

	ssd1306_begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);

	char* text = "This is demo for SSD1306 i2c driver for Embedded Linux";
	ssd1306_drawString(text);
	ssd1306_display();
	sleep(1);

	ssd1306_dim(1);
	ssd1306_startscrollright(00,0xFF);
	sleep(1);

	ssd1306_clearDisplay();
	while(1);

}
