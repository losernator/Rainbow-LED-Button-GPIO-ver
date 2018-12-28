/* 
 *  Rainbow LED Button ( GPIO ver )
 *  Author		 : sana2dang ( fly9p ) - sana2dang@naver.com / sana2dang@gmail.com
 *  Creation Date: 2017 - 04 - 01
 *  Cafe			: http://cafe.naver.com/raspigamer
 *  Thanks to	 : zzeromin, smyani, GreatKStar, KimPanda, StarNDevil, angel
 * 
 * - Reference -
 * wiringPi
 * http://wiringpi.com/the-gpio-utility/
 * 
 * 
 * jstest
 * https://github.com/flosse/linuxconsole/blob/master/utils/jstest.c
 * 
 * - install -
 * git clone https://github.com/losernator/rainbowLED.git
 * cd rainbowLED
 * apt-get install wiringpi
 * sudo gcc rainbowledbtn.c -o rainbowledbtn -lwiringPi -lpthread
 *
 *
 *
 * - complie -
 * sudo gcc rainbowled.c -o rainbowled -lwiringPi -lpthread
 * 
 * - install -
 * add this line into autostart.sh
 * /home/pi/rainbowLED/rainbowledbtn /dev/input/js0 &
 * 
 */

#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <wiringPi.h>
#include <pthread.h>

#include <linux/input.h>
#include <linux/joystick.h>

#define NAME_LENGTH 128

/*********
* LED btn map
*  7 8
*
*  4 5 6
*  1 2 3
**********/

#define pin1  21	// GPIO 21-A
#define pin2  20	// GPIO 20-B
#define pin3  16	// GPIO 16-TR
#define pin4  7	// GPIO 07-X
#define pin5  12	// GPIO 12-Y
#define pin6  8	// GPIO 08-TL
#define pin7  26	// GPIO 26-SL
#define pin8  19	// GPIO 19-ST

#define AbtnShotPressTime 700
#define AbtnLongPressTime 1500
#define stayTime 10000

int chargePress = 0;	// charge button press on/off flag
int chargeShot = 0;	// chargeShot cnt
long sTime = 0;	// chargePress Start Time
int chargeCnt = 0;	// Charge Count
int k = 0;	// Led Cycle Count

long sStopTime = 0;	// stop time
int rotationFlag = 0;	// rotation
int rotationCnt = 0;
int rotationspeed = 500; // rotation speed

int on = 1; //defalult LED state LED on,off
int off = 0;

void funAllLightOn(int mode)
{
	if( mode == 0 )		// ALL LED ON
	{
		digitalWrite(pin1,on);
		digitalWrite(pin2,on);
		digitalWrite(pin3,on);
		digitalWrite(pin4,on);
		digitalWrite(pin5,on);
		digitalWrite(pin6,on);
		digitalWrite(pin7,on);
		digitalWrite(pin8,on);
	}
	if( mode == 1 )		// ALL LED ON except SE,ST
	{
		digitalWrite(pin1,on);
		digitalWrite(pin2,on);
		digitalWrite(pin3,on);
		digitalWrite(pin4,on);
		digitalWrite(pin5,on);
		digitalWrite(pin6,on);
	}
}

void funAllLightOff(int mode)
{
	if( mode == 0 )		// ALL LED OFF
	{
		digitalWrite(pin1,off);
		digitalWrite(pin2,off);
		digitalWrite(pin3,off);
		digitalWrite(pin4,off);
		digitalWrite(pin5,off);
		digitalWrite(pin6,off);
		digitalWrite(pin7,off);
		digitalWrite(pin8,off);
	}
	if( mode == 1 )		// ALL LED OFF except SE,ST
	{
		digitalWrite(pin1,off);
		digitalWrite(pin2,off);
		digitalWrite(pin3,off);
		digitalWrite(pin4,off);
		digitalWrite(pin5,off);
		digitalWrite(pin6,off);
	}
}

void funRotation(int mode) {
	if( mode == 0 )	{	// rotaition clockwise
		digitalWrite(pin4,on);
		delay(30);
		funAllLightOff(0);
		digitalWrite(pin5,on);
		delay(30);
		funAllLightOff(0);
		digitalWrite(pin6,on);
		delay(30);
		funAllLightOff(0);
		digitalWrite(pin3,on);
		delay(30);
		funAllLightOff(0);
		digitalWrite(pin2,on);
		delay(30);
		funAllLightOff(0);
		digitalWrite(pin1,on);
		delay(30);
		funAllLightOff(0);
	}
	if( mode == 1 )	{	// rotaition counterclockwise
		digitalWrite(pin1,on);
		delay(30);
		funAllLightOff(0);
		digitalWrite(pin2,on);
		delay(30);
		funAllLightOff(0);
		digitalWrite(pin3,on);
		delay(30);
		funAllLightOff(0);
		digitalWrite(pin6,on);
		delay(30);
		funAllLightOff(0);
		digitalWrite(pin5,on);
		delay(30);
		funAllLightOff(0);
		digitalWrite(pin4,on);
		delay(30);
		funAllLightOff(0);
	}
}
void funBlink(int mode) {
	if( mode == 0 )	{	// blink
		funAllLightOn(1);
		delay(40);
		funAllLightOff(1);
		delay(40);
		funAllLightOn(1);
		delay(40);
		funAllLightOff(1);
	}
	if( mode == 1 )	{	// blink alt.
		digitalWrite(pin1,on);
		delay(40);
		funAllLightOff(1);
		digitalWrite(pin2,on);
		delay(40);
		funAllLightOff(1);
		digitalWrite(pin3,on);
		delay(40);
		funAllLightOff(1);
		digitalWrite(pin6,on);
		delay(40);
		funAllLightOff(1);
		digitalWrite(pin5,on);
		delay(40);
		funAllLightOff(1);
		digitalWrite(pin4,on);
		delay(40);
		funAllLightOff(1);
	}
}

void funRainbowLed()
{
	if( rotationFlag == 1)	// idle time
	{
		switch( rotationCnt )
		{
			case 0:
				digitalWrite(pin1,on);
				rotationCnt ++;
			break;
			case 1: 
				digitalWrite(pin2,on);
				rotationCnt ++;
			break;
			case 2: 
				digitalWrite(pin3,on);
				rotationCnt ++;
			break;
			case 3: 
				digitalWrite(pin6,on);
				rotationCnt ++;
			break;
			case 4: 
				digitalWrite(pin5,on);
				rotationCnt ++;
			break;
			case 5: 
				digitalWrite(pin4,on);
				rotationCnt = 0;
			break;	
			default: 
				funAllLightOff(0);
		}
		delay(rotationspeed);
		funAllLightOff(0);
	}
	else
	{
		rotationCnt = 0;
	}

// chargeshot Event
	if( chargePress == 1 && chargeShot == 0  )
	{
		switch( chargeCnt )
		{
			case 0:
				break;
			case 1:
				digitalWrite(pin4,on);
				break;
			case 2: 
				digitalWrite(pin5,on);
				break;
			case 3: 
				digitalWrite(pin6,on);
				break;
			case 4: 
				digitalWrite(pin3,on);
				break;
			case 5: 
				digitalWrite(pin2,on);
				break;
			case 6: 
				digitalWrite(pin1,on);
			default: 
				chargeCnt = 0;
				break;
		}
		delay(50);
		if( ( millis() - sTime ) < AbtnShotPressTime )		// chargePress check1
		{
			chargeCnt = 0;
		} else {
			if( ( millis() - sTime )  > AbtnLongPressTime )	// chargePress check2
			{
				chargeShot = 1;
			}
			else
			{
				chargeCnt++;
			}
		}
	}
	if( chargePress == 0 )
	{
		if( chargeShot == 1 )
		{
			for(k=0; k<2; k++)
			{
				funAllLightOn(0);
				delay(40);
				funAllLightOff(0);
				delay(60);
				funAllLightOn(0);
				delay(40);
				funAllLightOff(0);
				delay(60);
				funAllLightOn(0);
				delay(40);
				funAllLightOff(0);
				delay(60);
				funAllLightOn(0);
				delay(40);
				funAllLightOff(0);
			}
			funAllLightOn(0);
			delay(400);
			funAllLightOff(0);
		}
		chargeCnt	= 0;		// init value
		chargeShot	= 0;		// init value
	}
}

// LED Btn Thread
void* ledBtnThread(void *arg)
{
	int k = 0;
	
	while(1)
	{
		funRainbowLed();
		usleep(1000);
		
		if( ( millis() - sStopTime ) > stayTime )	// rotation check
		{
			rotationFlag = 1;
		}
		
	}
	
}

int main (int argc, char **argv)
{
	int fd;
	unsigned char axes = 2;
	int version = 0x000800;
	unsigned char buttons = 2;
	char name[NAME_LENGTH] = "Unknown";

	// thread value
	int res;
	pthread_t a_thread;
	void* thread_result;

	if (argc < 2 || argc > 5 )
	{
		puts("");
		puts("Usage: rainbowledbtn <#joystick> [button mapping type] [chargeshot] [invert]");
		puts("");
		puts("");
		puts("[button mapping type] : 'ab' or 'ba', default:ba");
		puts("[chargeshot] : 'charge' or 'nocharge', default:charge");
		puts("[invert] : 'invert' or 'normal', default:normal");
		puts("ex)./rainbowledbtn /dev/input/js0");
		puts("   ./rainbowledbtn /dev/input/js0 ab");
		puts("   ./rainbowledbtn /dev/input/js0 ab nocharge");
		puts("   ./rainbowledbtn /dev/input/js0 ab nocharge invert");
		puts("");
		exit(1);
	}
	if ((fd = open(argv[1], O_RDONLY)) < 0) {
		perror("rainbowled");
		exit(1);
	}

	ioctl(fd, JSIOCGVERSION, &version);
	ioctl(fd, JSIOCGAXES, &axes);
	ioctl(fd, JSIOCGBUTTONS, &buttons);
	ioctl(fd, JSIOCGNAME(NAME_LENGTH), name);
	/*
	printf("joystick (%s) has %d axes and %d buttons.\n",
		name, axes, buttons );
	printf("Testing ... (Ctrl+C to exit)\n");
	*/

// default gpio map :		 B A Y X L R sl st
	int gpio_button_maps[] = { 1,0,4,3,6,7,10,11 };
// alt. gpio map :				 A B X Y L R sl st
	int gpio_button_maps_ab[] = { 0,1,3,4,6,7,10,11};
// default charge state
	int ischrg = 1;
// default LED state

	for (int i = 2; i < argc; i++)
	{
		// printf("argv[%d] = %s\n", i, argv[i]);
		if (strcmp(argv[i], "ab")== 0)
		{
			for ( int j = 0; j < 8; j++) 
			{
				gpio_button_maps[j] = gpio_button_maps_ab[j];
			}
		}
		else if (strcmp(argv[i], "nocharge")== 0)
		{
			ischrg = 0; // disable chargeshot
			//chrg_button = atoi(argv[3]);
		}
		else if (strcmp(argv[i], "invert")== 0)
		{
			// reverse on off status
			on = 0;
			off = 1;
		}
	}
	// Start led btn
	if (argc > 1 )
	{
		int *axis;
		char *button;
		int i;
		int btnpressed;
		struct js_event js;

		res = pthread_create(&a_thread, NULL, ledBtnThread, (void*)NULL);
			if( res )
		printf("thread create error!\n");

		if( wiringPiSetupGpio() == -1 )
			return 0;
		
		pinMode(pin1, OUTPUT);
		pinMode(pin2, OUTPUT);
		pinMode(pin3, OUTPUT);
		pinMode(pin4, OUTPUT);
		pinMode(pin5, OUTPUT);
		pinMode(pin6, OUTPUT);
		pinMode(pin7, OUTPUT);
		pinMode(pin8, OUTPUT);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		axis = calloc(axes, sizeof(int));
		button = calloc(buttons, sizeof(char));
		
		while (1)
		{
			if (read(fd, &js, sizeof(struct js_event)) != sizeof(struct js_event)) 
			{
				perror("\nledbtn: error reading");
				exit (1);
			}
			switch(js.type & ~JS_EVENT_INIT) 
			{
				case JS_EVENT_BUTTON:
					button[js.number] = js.value;
				break;
				case JS_EVENT_AXIS:
					axis[js.number] = js.value;
				break;
			}
			
			printf("\r");
			/*
			if (axes) {
				printf("Axes: ");
				for (i = 0; i < axes; i++)
					printf("%2d:%6d ", i, axis[i]);
			}
			*/
			if (buttons) {
				btnpressed = 0;
				for (i = 0; i < buttons; i++)
				{
					btnpressed += button[i];
					//printf("%2d:%s ", i, button[i] ? "on " : "off");
					sStopTime = millis();		// rotation time Start
					rotationFlag = 0;		// stop rotation
					//LED no.1
					if( i==gpio_button_maps[0] && button[i] == 1 )
					{
						digitalWrite(pin1,on);
						if( ischrg == 1 && chargePress != 1 )
						{
							chargePress  = 1;
							sTime = millis();
						}
					}
					if( i==gpio_button_maps[0] && button[i] == 0)
					{
						if(chargePress != 1 )
							digitalWrite(pin1,off);
					}
					//LED no.2
					if( i==gpio_button_maps[1] && button[i] == 1 )
					{
						digitalWrite(pin2,on);
						if( ischrg == 1 && chargePress  != 1  )
						{
							chargePress  = 1;
							sTime = millis();
						}
					}
					if( i==gpio_button_maps[1] && button[i] == 0 )
					{
						if(chargePress != 1 )
							digitalWrite(pin2,off);
					}
					//LED no.4
					if( i==gpio_button_maps[2] && button[i] == 1 )
					{
						digitalWrite(pin4,on);
						if( ischrg == 1 && chargePress  != 1 )
						{
							chargePress  = 1;
							sTime = millis();
						}
					}
					if( i==gpio_button_maps[2] && button[i] == 0 )
					{
						if(chargePress != 1 )
							digitalWrite(pin4,off);
					}
					//LED no.5
					if( i==gpio_button_maps[3] && button[i] == 1 )
					{
						digitalWrite(pin5,on);
						if( ischrg == 1 && chargePress  != 1 )
						{
							chargePress  = 1;
							sTime = millis();
						}
					}
					if( i==gpio_button_maps[3] && button[i] == 0 )
					{
						if(chargePress != 1 )
							digitalWrite(pin5,off);
					}
					//LED no.6
					if( i==gpio_button_maps[4] && button[i] == 1 ) 
					{
						digitalWrite(pin6,on);
						if( ischrg == 1  && chargePress  != 1 ) {
							chargePress  = 1;
							sTime = millis();
						}
					}
					if( i==gpio_button_maps[4] && button[i] == 0 ){
						if(chargePress != 1 )
							digitalWrite(pin6,off);
					}
					//LED no.3
					if( i==gpio_button_maps[5] && button[i] == 1 ) {
						digitalWrite(pin3,on);
						if( ischrg == 1 && chargePress != 1 ) {
							chargePress = 1;
							sTime = millis();
						}
					}
					if( i==gpio_button_maps[5] && button[i] == 0 ){
						if(chargePress != 1 )
							digitalWrite(pin3,off);
					}
					//LED no.7
					if( i==gpio_button_maps[6] && button[i] == 1 ) {
						digitalWrite(pin7,on);
						funBlink(1);
					}
					if( i==gpio_button_maps[6] && button[i] == 0 ){
						digitalWrite(pin7,off);
					}
					//button no.8
					if( i==gpio_button_maps[7] && button[i] == 1 ) {
						digitalWrite(pin8,on);
						funBlink(0);
					}
					if( i==gpio_button_maps[7] && button[i] == 0 ){
						digitalWrite(pin8,off);
					}
				}
				//printf("btnpressed:%d ",btnpressed);
				if (btnpressed == 0) {
					chargePress  = 0;
					funAllLightOff(0);
				}
			}
			fflush(stdout);
		}
	}
	printf("rainbowled: unknown joystick: %s %s \n", argv[2], argv[3]);
	return -1;
}
