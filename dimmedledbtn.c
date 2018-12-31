/* 
 *  Rainbow LED Button ( GPIO ver )
 *  Author       : sana2dang ( fly9p ) - sana2dang@naver.com / sana2dang@gmail.com
 *  Creation Date: 2017 - 04 - 01
 *  Cafe         : http://cafe.naver.com/raspigamer
 *  Thanks to    : zzeromin, smyani, GreatKStar, KimPanda, StarNDevil, angel
 * 
 * - Reference -
 * wiringPi
 * http://wiringpi.com/the-gpio-utility/
 * apt-get install wiringPi
 * 
 * jstest
 * https://github.com/flosse/linuxconsole/blob/master/utils/jstest.c
  *
 * - complie -
 * sudo gcc dimmedledbtn.c -o dimmedledbtn -lwiringPi -lpthread
 * ./dimmedledbtn /dev/input/js0 4
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
#include <stdlib.h>
#include <stdint.h>
#include <wiringPi.h>
#include <softPwm.h>
#include <pthread.h>

#include <linux/input.h>
#include <linux/joystick.h>

#define NAME_LENGTH 128

/*********
* LED btn map
*  SE ST
*
*  X Y L
*  A B R
**********/

#define pinA  21	// GPIO 21 A
#define pinB  20	// GPIO 20 B
#define pinX  7		// GPIO 7 X
#define pinY  12	// GPIO 12 Y
#define pinL  8		// GPIO 8  L
#define pinR  16	// GPIO 16 R
#define pinSE 26 	// GPIO 26 SELECT
#define pinST 19 	// GPIO 19 START

#define rotationDelay 400
#define pwmRange 100

#define AbtnShotPressTime 700	
#define AbtnLongPressTime 1500
#define stayTime 10000

int chargePress	= 0;	// chargePress on/off flag
int chargeShot	= 0;	// chargeShot cnt
long sTime	= 0;	// chargePress Start Time
int chargeCnt 	= 0;	// Charge Count
int k = 0;		// Led Cycle Count

long sStopTime = 0;	// stop time
int rotationFlag = 0;	// rotation
int rotationCnt = 0;
int mode = 0;

static int dimmPinA = -1;
static int dimmPinB = -1;
static int dimmPinX = -1;
static int dimmPinY = -1;
static int dimmPinL = -1;
static int dimmPinR = -1;
static int dimmPinSE = -1;
static int dimmPinST = -1;

static int dimmFlagA = 0;
static int dimmFlagB = 0;
static int dimmFlagX = 0;
static int dimmFlagY = 0;
static int dimmFlagL = 0;
static int dimmFlagR = 0;
static int dimmFlagSE = 0;
static int dimmFlagST = 0;

static int dimmDelay = 3;


void funAllLightOn(int mode)
{
	if( mode == 0 )		// ALL LED ON
	{
		softPwmWrite(pinA,pwmRange);
		softPwmWrite(pinB,pwmRange);
		softPwmWrite(pinX,pwmRange);
		softPwmWrite(pinY,pwmRange);
		softPwmWrite(pinL,pwmRange);
		softPwmWrite(pinR,pwmRange);
		softPwmWrite(pinSE,pwmRange);
		softPwmWrite(pinST,pwmRange);
	}
	if( mode == 1 )		// ALL LED ON except SE,ST
	{
		softPwmWrite(pinA,pwmRange);
		softPwmWrite(pinB,pwmRange);
		softPwmWrite(pinX,pwmRange);
		softPwmWrite(pinY,pwmRange);
		softPwmWrite(pinL,pwmRange);
		softPwmWrite(pinR,pwmRange);
	}
}

void funAllLightOff(int mode)
{
	if( mode == 0 )		// ALL LED OFF
	{
		softPwmWrite(pinA,0);
		softPwmWrite(pinB,0);
		softPwmWrite(pinX,0);
		softPwmWrite(pinY,0);
		softPwmWrite(pinL,0);
		softPwmWrite(pinR,0);
		softPwmWrite(pinSE,0);
		softPwmWrite(pinST,0);
	}
	if( mode == 1 )		// ALL LED ON except SE,ST
	{
		softPwmWrite(pinA,0);
		softPwmWrite(pinB,0);
		softPwmWrite(pinX,0);
		softPwmWrite(pinY,0);
		softPwmWrite(pinL,0);
		softPwmWrite(pinR,0);
	}

}

void funAllLightDimmOff(int mode)
{
	if( mode == 0 )		// ALL LED OFF
	{
		dimmFlagA = 0;
		dimmFlagB = 0;
		dimmFlagX = 0;
		dimmFlagY = 0;
		dimmFlagL = 0;
		dimmFlagR = 0;
	}
}

void funBlink(int mode) {
	if( mode == 0 )	{// blink
		funAllLightOn(0);
		delay(40);
		funAllLightOff(0);
		delay(40);
		funAllLightOn(0);
		delay(40);
		funAllLightOff(0);
	}
	if( mode == 1)	{
		softPwmWrite(pinA,pwmRange);
		delay(40);
		funAllLightOff(0);
		softPwmWrite(pinA,pwmRange);
		delay(40);
		funAllLightOff(0);
		softPwmWrite(pinB,pwmRange);
		delay(40);
		funAllLightOff(0);
		softPwmWrite(pinX,pwmRange);
		delay(40);
		funAllLightOff(0);
		softPwmWrite(pinY,pwmRange);
		delay(40);
		funAllLightOff(0);
		softPwmWrite(pinL,pwmRange);
		delay(40);
		funAllLightOff(0);
		softPwmWrite(pinR,pwmRange);
		delay(40);
		funAllLightOff(0);
	}
}

void funRainbowLed()
{	
	if( rotationFlag == 1)	// stay button led
	{
	int bright = 0;
	switch( rotationCnt )
		{
			case 0:
				for( bright = 0 ; bright <=pwmRange; ++bright )
				{
					softPwmWrite(pinA, bright);
					delay(dimmDelay);
				}
				dimmPinA = pinA;
				dimmFlagA = 1;
				dimmFlagY = 0;
				rotationCnt ++;
			break;
			case 1: 
				for( bright = 0 ; bright <=pwmRange; ++bright )
				{
					softPwmWrite(pinB, bright);
					delay(dimmDelay);
				}
				dimmPinB = pinB;
				dimmFlagB = 1;
				dimmFlagX = 0;
				rotationCnt ++;
			break;
			case 2: 
				for( bright = 0 ; bright <=pwmRange; ++bright )
				{
					softPwmWrite(pinR, bright);
					delay(dimmDelay);
				}
				dimmPinR = pinR;
				dimmFlagR = 1;
				dimmFlagA = 0;
				rotationCnt ++;
			break;
			case 3: 
				for( bright = 0 ; bright <=pwmRange; ++bright )
				{
					softPwmWrite(pinL, bright);
					delay(dimmDelay);
				}
				dimmPinL = pinL;
				dimmFlagL = 1;
				dimmFlagB = 0;
				rotationCnt ++;
			break;
			case 4: 
				for( bright = 0 ; bright <=pwmRange; ++bright )
				{
					softPwmWrite(pinY, bright);
					delay(dimmDelay);
				}
				dimmPinY = pinY;
				dimmFlagY = 1;
				dimmFlagR = 0;
				rotationCnt ++;
			break;
			case 5: 
				for( bright = 0 ; bright <=pwmRange; ++bright )
				{
					softPwmWrite(pinX, bright);
					delay(dimmDelay);
				}
				dimmPinX = pinX;
				dimmFlagX = 1;
				dimmFlagL = 0;
				rotationCnt = 0;
			break;	
			default: 
				funAllLightDimmOff(0);	
		}
		delay(rotationDelay);
	}
	else
	{
		rotationCnt = 0;
	}

	// charge Event
	if( chargePress == 1 && chargeShot == 0 )
	{
		//printf("%d\n", chargeCnt );
		switch( chargeCnt )
		{
			case 1: 
			dimmPinA = pinA;
			dimmFlagA = 1;
			break;
			case 2:
			dimmPinB = pinB;
			dimmFlagB = 1;
			break;
			case 3: 
				dimmPinR = pinR;
				dimmFlagR = 1;
			break;
			case 4:
				dimmPinL = pinL;
				dimmFlagL = 1;
			break;
			case 5: 
				dimmPinY = pinY;
				dimmFlagY = 1;
			break;
			case 6:
				dimmPinX = pinX;
				dimmFlagX = 1;
			break;
			default: 
				chargeCnt = 0;
			break;
		}
		delay(20);
		if( ( millis() - sTime ) < AbtnShotPressTime )		// chargePress keep check1
		{
			chargeCnt = 0;
		}
		else
		{
			if( ( millis() - sTime )  > AbtnLongPressTime )	// chargePress keep check2
			{
				chargeShot = 1;
//				funAllLightDimmOff(0);
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
			funAllLightOff(0);
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
			}
			funAllLightOn(0);
			delay(400);
			funAllLightOff(0);
		}
		chargeCnt = 0;		// init value
		chargeShot = 0;		// init value
	}
}

// LED Btn Thread
void* ledBtnThread(void *arg)
{
	int chargeCnt = 0;
	int k = 0;
	
	while(1)
	{
		funRainbowLed();
		usleep(100000);
		
		if( ( millis() - sStopTime ) > stayTime )	// rotation check
		{
			rotationFlag = 1;
		}
	}	
}

// DIMM Btn Thread
void* dimmLedA(void *arg)
{
	int bright = pwmRange;
	while(1)
	{
		usleep(100000);
		if( dimmPinA != -1 )
		{
			for( bright = pwmRange; bright >=0; --bright )
			{
				if( dimmFlagA == 1)
				{
					bright = pwmRange;
				}
				softPwmWrite(dimmPinA, bright);
				delay(dimmDelay);
			}
			dimmPinA = -1;
			dimmFlagA = 0;
		}
	}
}

// DIMM Btn Thread
void* dimmLedB(void *arg)
{
	int bright = pwmRange;
	
	while(1)
	{
		usleep(100000);
		if( dimmPinB != -1 )
		{
			for( bright = pwmRange; bright >=0; --bright )
			{
				if( dimmFlagB == 1)
				{
					bright = pwmRange;
				}
				softPwmWrite(dimmPinB, bright);
				delay(dimmDelay);
			}
			dimmPinB = -1;
			dimmFlagB = 0;
		}
	}
}

void* dimmLedX(void *arg)
{
	int bright = pwmRange;
	
	while(1)
	{
		usleep(100000);
		if( dimmPinX != -1 )
		{
			for( bright = pwmRange; bright >=0; --bright )
			{
				if( dimmFlagX == 1)
				{
					bright = pwmRange;
				}
				softPwmWrite(dimmPinX, bright);
				delay(dimmDelay);
			}
			dimmPinX = -1;
			dimmFlagX = 0;
		}
	}
}

void* dimmLedY(void *arg)
{
	int bright = pwmRange;
	
	while(1)
	{
		usleep(100000);
		if( dimmPinY != -1 )
		{
			for( bright = pwmRange; bright >=0; --bright )
			{
				if( dimmFlagY == 1)
				{
					bright = pwmRange;
				}
				softPwmWrite(dimmPinY, bright);
				delay(dimmDelay);
			}
			dimmPinY = -1;
			dimmFlagY = 0;
		}
	}
}

void* dimmLedL(void *arg)
{
	int bright = pwmRange;
	
	while(1)
	{
		usleep(100000);
		if( dimmPinL != -1 )
		{
			for( bright = pwmRange; bright >=0; --bright )
			{
				if( dimmFlagL == 1)
				{
					bright = pwmRange;
				}
				softPwmWrite(dimmPinL, bright);
				delay(dimmDelay);
			}
			dimmPinL = -1;
			dimmFlagL = 0;
		}
	}
}

void* dimmLedR(void *arg)
{
	int bright = pwmRange;
	
	while(1)
	{
		usleep(100000);
		if( dimmPinR != -1 )
		{
			for( bright = pwmRange; bright >=0; --bright )
			{
				if( dimmFlagR == 1)
				{
					bright = pwmRange;
				}
				softPwmWrite(dimmPinR, bright);
				delay(dimmDelay);
				
			}
			dimmPinR = -1;
			dimmFlagR = 0;
		}
	}
}

void* dimmLedSE(void *arg)
{
	int bright = pwmRange;
	while(1)
	{
		usleep(100000);
		if( dimmPinSE != -1 )
		{
			for( bright = pwmRange; bright >=0; --bright )
			{
				if( dimmFlagSE == 1)
				{
					bright = pwmRange;
				}
				softPwmWrite(dimmPinSE, bright);
				delay(dimmDelay);
			}
			dimmPinSE = -1;
			dimmFlagSE = 0;
		}
	}
}

void* dimmLedST(void *arg)
{
	int bright = pwmRange;
	while(1)
	{
		usleep(100000);
		if( dimmPinST != -1 )
		{
			for( bright = pwmRange; bright >=0; --bright )
			{
				if( dimmFlagST == 1)
				{
					bright = pwmRange;
				}
				softPwmWrite(dimmPinST, bright);
				delay(dimmDelay);
			}
			dimmPinST = -1;
			dimmFlagST = 0;
		}
	}
}

int main (int argc, char **argv)
{
	int fd, i;
	unsigned char axes = 2;
	unsigned char buttons = 2;
	int version = 0x000800;
	char name[NAME_LENGTH] = "Unknown";
	
	// thread value
	int res;
	int resA;
	int resB;
	int resX;
	int resY;
	int resL;
	int resR;
	int resSE;
	int resST;
	
	pthread_t a_thread;
	pthread_t dimm_threadA;
	pthread_t dimm_threadB;
	pthread_t dimm_threadX;
	pthread_t dimm_threadY;
	pthread_t dimm_threadL;
	pthread_t dimm_threadR;
	pthread_t dimm_threadSE;
	pthread_t dimm_threadST;
	
	
	void* thread_result;
	void* thread_resultB;
	void* thread_resultX;
	void* thread_resultY;
	void* thread_resultL;
	void* thread_resultR;
	void* thread_resultSE;
	void* thread_resultST;

// default gpio map :		 B A Y X L R sl st
	int gpio_button_maps[] = { 1,0,4,3,6,7,10,11 };
// alt. gpio map :				 A B X Y L R sl st
	int gpio_button_maps_ab[] = { 0,1,3,4,6,7,10,11 };
// enable chargeshot
	int ischrg = 1;

	if (argc < 2 || argc > 5 )
	{
		puts("");
		puts("Usage: dimmedledbtn <#joystick> [dimming delay] [button mapping type] [chargeshot]");
		puts("");
		puts("[dimming delay] : lower is faster dimming speed, default:3");
		puts("[button mapping type] : 'ab' or 'ba', default:ba");
		puts("[chargeshot] : 'charge' or 'nocharge', default:charge");
		puts("ex)./dimmedledbtn /dev/input/js0");
		puts("   ./dimmedledbtn /dev/input/js0 3 ab");
		puts("   ./dimmedledbtn /dev/input/js0 3 ab nocharge");
		exit(1);
	}
	if ((fd = open(argv[1], O_RDONLY)) < 0) {
		perror("jstest");
		return 1;
	}

	for (int i = 2; i < argc; i++)
	{
		if (atoi(argv[i])!=0 || argv[i]=="0") { // is number
			dimmDelay = atoi(argv[i]);
		}
		else {
			if (strcmp(argv[i], "ab")== 0){
				for ( int j = 0; j < 8; j++) {
					gpio_button_maps[j] = gpio_button_maps_ab[j];
				}
			}
			else if (strcmp(argv[i], "nocharge")== 0){
				ischrg = 0; // disable chargeshot
			}
		}
	}

	ioctl(fd, JSIOCGVERSION, &version);
	ioctl(fd, JSIOCGAXES, &axes);
	ioctl(fd, JSIOCGBUTTONS, &buttons);
	ioctl(fd, JSIOCGNAME(NAME_LENGTH), name);

	//printf("Driver version is %d.%d.%d.\n",		version >> 16, (version >> 8) & 0xff, version & 0xff);
	//printf("Testing ... (interrupt to exit)\n");

/*
 * Event interface, single line readout.
 */

	if (argc > 1) {

		int *axis;
		char *button;
		int btnpressed;
		int i;
		struct js_event js;

		if( wiringPiSetupGpio() == -1 )
			return 0;
		
		
		// thread
		res = pthread_create(&a_thread, NULL, ledBtnThread, (void*)NULL);
		resA = pthread_create(&dimm_threadA, NULL, dimmLedA, (void*)NULL);
		resB = pthread_create(&dimm_threadB, NULL, dimmLedB, (void*)NULL);
		resX = pthread_create(&dimm_threadX, NULL, dimmLedX, (void*)NULL);
		resY = pthread_create(&dimm_threadY, NULL, dimmLedY, (void*)NULL);
		resL = pthread_create(&dimm_threadL, NULL, dimmLedL, (void*)NULL);
		resR = pthread_create(&dimm_threadR, NULL, dimmLedR, (void*)NULL);
		resSE = pthread_create(&dimm_threadSE, NULL, dimmLedSE, (void*)NULL);
		resST = pthread_create(&dimm_threadST, NULL, dimmLedST, (void*)NULL);
				
		if( res )
			printf("thread create error!\n");
		if( resA )
			printf("thread A create error!\n");
		if( resB )
			printf("thread B create error!\n");
		if( resX )
			printf("thread X create error!\n");
		if( resY )
			printf("thread Y create error!\n");
		if( resL )
			printf("thread L create error!\n");
		if( resR )
			printf("thread R create error!\n");
		if( resSE )
			printf("thread SE create error!\n");
		if( resST )
			printf("thread ST create error!\n");
		
		pinMode(pinA, OUTPUT);
		pinMode(pinB, OUTPUT);
		pinMode(pinX, OUTPUT);
		pinMode(pinY, OUTPUT);
		pinMode(pinL, OUTPUT);
		pinMode(pinR, OUTPUT);
		pinMode(pinSE, OUTPUT);
		pinMode(pinST, OUTPUT);
		
		softPwmCreate(pinA, 0, pwmRange);
		softPwmCreate(pinB, 0, pwmRange);
		softPwmCreate(pinX, 0, pwmRange);
		softPwmCreate(pinY, 0, pwmRange);
		softPwmCreate(pinL, 0, pwmRange);
		softPwmCreate(pinR, 0, pwmRange);
		softPwmCreate(pinSE, 0, pwmRange);
		softPwmCreate(pinST, 0, pwmRange);

		axis = calloc(axes, sizeof(int));
		button = calloc(buttons, sizeof(char));

		while (1) {
			if (read(fd, &js, sizeof(struct js_event)) != sizeof(struct js_event)) {
				perror("\njstest: error reading");
				return 1;
			}

			switch(js.type & ~JS_EVENT_INIT) {
			case JS_EVENT_BUTTON:
				button[js.number] = js.value;
				break;
			case JS_EVENT_AXIS:
				axis[js.number] = js.value;
				break;
			}

			//printf("\r");

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
					
/*********
* LED btn map
*  SE ST
*
*  X Y L
*  A B R

**********/
					//LED A
					if( i==gpio_button_maps[0] && button[i] == 1 )
					{
						dimmPinA = pinA;
						dimmFlagA = 1;
						if( ischrg == 1 && chargePress  != 1 )
						{
							chargePress  = 1;
							sTime = millis();
						}
					}
					if( i==gpio_button_maps[0] && button[i] == 0 )
					{
						if(chargePress != 1 )
							dimmFlagA = 0;
					}
					
					//LED B
					if( i==gpio_button_maps[1] && button[i] == 1 )
					{
						dimmPinB = pinB;
						dimmFlagB = 1;
						if( ischrg == 1 && chargePress  != 1 )
						{
							chargePress  = 1;
							sTime = millis();
						}
					}
					if( i==gpio_button_maps[1] && button[i] == 0 )
					{
						if(chargePress != 1 )
							dimmFlagB = 0;
					}
					
					//LED X
					if( i==gpio_button_maps[2] && button[i] == 1 )
					{
						dimmPinX = pinX;
						dimmFlagX = 1;
						if( ischrg == 1 && chargePress  != 1 )
						{
							chargePress  = 1;
							sTime = millis();
						}
					}
					if( i==gpio_button_maps[2] && button[i] == 0 )
					{
						if(chargePress != 1 )
							dimmFlagX = 0;
					}
					//LED Y
					if( i==gpio_button_maps[3] && button[i] == 1 )
					{
						dimmPinY = pinY;
						dimmFlagY = 1;
						if( ischrg == 1 && chargePress  != 1 )
						{
							chargePress  = 1;
							sTime = millis();
						}
					}
					if( i==gpio_button_maps[3] && button[i] == 0 )
					{
						if(chargePress != 1 )
							dimmFlagY = 0;
					}
					//LED L
					if( i==gpio_button_maps[4] && button[i] == 1 )
					{
						dimmPinL = pinL;
						dimmFlagL = 1;
						if( ischrg == 1 && chargePress  != 1 )
						{
							chargePress  = 1;
							sTime = millis();
						}
					}
					if( i==gpio_button_maps[4] && button[i] == 0 )
					{
						if(chargePress != 1 )
							dimmFlagL = 0;
					}
					//LED R
					if( i==gpio_button_maps[5] && button[i] == 1 )
					{
						dimmPinR = pinR;
						dimmFlagR = 1;
						if( ischrg == 1 && chargePress  != 1 )
						{
							chargePress  = 1;
							sTime = millis();
						}
					}
					if( i==gpio_button_maps[5] && button[i] == 0 )
					{
						if(chargePress != 1 )
							dimmFlagR = 0;
					}
					//LED SE (Coin)
					if( i==gpio_button_maps[6] && button[i] == 1 )
					{
						dimmPinSE = pinSE;
						dimmFlagSE = 1;
						funBlink(1);
					}
					if( i==gpio_button_maps[6] && button[i] == 0 )
					{
						dimmFlagSE = 0;
					}
					//LED ST (Start)
					if( i==gpio_button_maps[7] && button[i] == 1 )
					{
						dimmPinST = pinST;
						dimmFlagST = 1;
						funBlink(0);
					}
					if( i==gpio_button_maps[7] && button[i] == 0 )
					{
						dimmFlagST = 0;
					}
				}
				if (btnpressed == 0) {
					chargePress  = 0;
					funAllLightDimmOff(0);
				}
			}
			fflush(stdout);
		}
	}

	printf("jstest: unknown mode: %s\n", argv[1]);
	return -1;
}
