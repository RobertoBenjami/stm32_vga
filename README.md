# stm32_vga

stm32 vga driver

Demo video: https://youtu.be/2rrmiBwJffc

Hardware: https://github.com/RobertoBenjami/stm32_vga/blob/master/Vga_hardware.pdf

App:
- LcdSpeedTest: Lcd speed test
- JpgViewer: JPG file viewer from SD card or pendrive
- 3d filled vector (from https://github.com/cbm80amiga/ST7789_3D_Filled_Vector_Ext)

How to use starting from zero with CubeIDE ?

Create project
- setting the RCC (Crystal/ceramic resonator)
- setting the DEBUG (serial wire or trace assyn sw)
- setting the SYS (timebase source, like the basic timer for this)
- setting the LTDC (RGB666, resolution, sync parameters, layer settings)
- if SDCARD : setting the SDIO mode, enable the FATFS, FATFS: USE_LFN, MAX_SS = 4096, FS_LOCK = 5, RTC enabled
- if JPG : enabled the LIBJPEG
- setting the clock configuration
- project settings: project name, toolchain = truestudio, stack size = 0x800
- generate source code
- open main.c
- add USER CODE BEGIN PFP: void mainApp(void);
- add USER CODE BEGIN 0: int __io_putchar(int ch) { ITM_SendChar(ch); return 0; }
- add USER CODE BEGIN WHILE: mainApp();
- open main.h
- add USER CODE BEGIN Includes (#include "stm32f4xx_hal.h" or #include "stm32h7xx_hal.h" or ...)
- add 3 new folder for Core folder (App, Lcd, Heap)
- copy file(s) from App/... to App
- copy 4 files from Drivers to Lcd (lcd.h, bmp.h, stm32_adafruit_lcd_fb.h / c)
- copy Fonts folder to Lcd folder
- add include path : Core/App, Core/Lcd, Core/Heap
- setting the compile options (Enable paralell build, optimalization)
- compile, run ...

How to adding the SWO support to cheap stlink ? https://lujji.github.io/blog/stlink-clone-trace/
