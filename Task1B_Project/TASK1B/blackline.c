#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <math.h> //included to support power function
#include "lcd.h"
#include "blackline.h"
int fl=0;
void port_init();
void timer5_init();
void velocity(unsigned char, unsigned char);
void motors_delay();

unsigned char ADC_Conversion(unsigned char);
unsigned char ADC_Value;
unsigned char flag = 0;
unsigned char Left_white_line = 0;
unsigned char Center_white_line = 0;
unsigned char Right_white_line = 0;
unsigned int  node_count = 0;

volatile unsigned long int ShaftCountLeft = 0; //to keep track of left position encoder
volatile unsigned long int ShaftCountRight = 0; //to keep track of right position encoder

//Function to configure LCD port
void lcd_port_config (void)
{
 DDRC = DDRC | 0xF7; //all the LCD pin's direction set as output
 PORTC = PORTC & 0x80; // all the LCD pins are set to logic 0 except PORTC 7
}

//ADC pin configuration
void adc_pin_config (void)
{
 DDRF = 0x00;
 PORTF = 0x00;
 DDRK = 0x00;
 PORTK = 0x00;
}

//Function to configure ports to enable robot's motion
void motion_pin_config (void)
{
 DDRA = DDRA | 0x0F;
 PORTA = PORTA & 0xF0;
 DDRL = DDRL | 0x18;   //Setting PL3 and PL4 pins as output for PWM generation
 PORTL = PORTL | 0x18; //PL3 and PL4 pins are for velocity control using PWM.
}

//Function to Initialize PORTS
void port_init()
{
	lcd_port_config();
    left_encoder_pin_config();
	right_encoder_pin_config();
	adc_pin_config();
	motion_pin_config();
}

// Timer 5 initialized in PWM mode for velocity control
// Prescale:256
// PWM 8bit fast, TOP=0x00FF
// Timer Frequency:225.000Hz
void timer5_init()
{
	TCCR5B = 0x00;	//Stop
	TCNT5H = 0xFF;	//Counter higher 8-bit value to which OCR5xH value is compared with
	TCNT5L = 0x01;	//Counter lower 8-bit value to which OCR5xH value is compared with
	OCR5AH = 0x00;	//Output compare register high value for Left Motor
	OCR5AL = 0xFF;	//Output compare register low value for Left Motor
	OCR5BH = 0x00;	//Output compare register high value for Right Motor
	OCR5BL = 0xFF;	//Output compare register low value for Right Motor
	OCR5CH = 0x00;	//Output compare register high value for Motor C1
	OCR5CL = 0xFF;	//Output compare register low value for Motor C1
	TCCR5A = 0xA9;	//{COM5A1=1, COM5A0=0; COM5B1=1, COM5B0=0; COM5C1=1 COM5C0=0}
 					 // For Overriding normal port functionality to OCRnA outputs.
				  	 // {WGM51=0, WGM50=1} Along With WGM52 in TCCR5B for Selecting FAST PWM 8-bit Mode

	TCCR5B = 0x0B;	//WGM12=1; CS12=0, CS11=1, CS10=1 (Prescaler=64)
}

void adc_init()
{
	ADCSRA = 0x00;
	ADCSRB = 0x00;		//MUX5 = 0
	ADMUX = 0x20;		//Vref=5V external --- ADLAR=1 --- MUX4:0 = 0000
	ACSR = 0x80;
	ADCSRA = 0x86;		//ADEN=1 --- ADIE=1 --- ADPS2:0 = 1 1 0
}

//Function For ADC Conversion
unsigned char ADC_Conversion(unsigned char Ch)
{
	unsigned char a;
	if(Ch>7)
	{
		ADCSRB = 0x08;
	}
	Ch = Ch & 0x07;
	ADMUX= 0x20| Ch;
	ADCSRA = ADCSRA | 0x40;		//Set start conversion bit
	while((ADCSRA&0x10)==0);	//Wait for conversion to complete
	a=ADCH;
	ADCSRA = ADCSRA|0x10; //clear ADIF (ADC Interrupt Flag) by writing 1 to it
	ADCSRB = 0x00;
	return a;
}

//Function To Print Sensor Values At Desired Row And Column Location on LCD
void print_sensor(char row, char coloumn,unsigned char channel)
{

	ADC_Value = ADC_Conversion(channel);
	lcd_print(row, coloumn, ADC_Value, 3);
}

//Function for velocity control
void velocity (unsigned char left_motor, unsigned char right_motor)
{
	OCR5AL = (unsigned char)left_motor;
	OCR5BL = (unsigned char)right_motor;
}

//Function used for setting motor's direction
void motion_set (unsigned char Direction)
{
 unsigned char PortARestore = 0;

 Direction &= 0x0F; 		// removing upper nibbel for the protection
 PortARestore = PORTA; 		// reading the PORTA original status
 PortARestore &= 0xF0; 		// making lower direction nibbel to 0
 PortARestore |= Direction; // adding lower nibbel for forward command and restoring the PORTA status
 PORTA = PortARestore; 		// executing the command
}

//Function to configure INT4 (PORTE 4) pin as input for the left position encoder
void left_encoder_pin_config (void)
{
	DDRE = DDRE & 0xEF; //Set the direction of the PORTE 4 pin as input
	PORTE = PORTE | 0x10; //Enable internal pull-up for PORTE 4 pin
}
//Function to configure INT5 (PORTE 5) pin as input for the right position encoder
void right_encoder_pin_config (void)
{
	DDRE = DDRE & 0xDF; //Set the direction of the PORTE 4 pin as input
	PORTE = PORTE | 0x20; //Enable internal pull-up for PORTE 4 pin
}

void left_position_encoder_interrupt_init (void) //Interrupt 4 enable
{
	cli(); //Clears the global interrupt
	EICRB = EICRB | 0x02; // INT4 is set to trigger with falling edge
	EIMSK = EIMSK | 0x10; // Enable Interrupt INT4 for left position encoder
	sei();   // Enables the global interrupt
}

void right_position_encoder_interrupt_init (void) //Interrupt 5 enable
{
	cli(); //Clears the global interrupt
	EICRB = EICRB | 0x08; // INT5 is set to trigger with falling edge
	EIMSK = EIMSK | 0x20; // Enable Interrupt INT5 for right position encoder
	sei();   // Enables the global interrupt
}

ISR(INT5_vect)
{
	ShaftCountRight++;  //increment right shaft position count
}


//ISR for left position encoder
ISR(INT4_vect)
{
	ShaftCountLeft++;  //increment left shaft position count
}

//Function used for moving robot forward by specified distance
void linear_distance_mm(unsigned int DistanceInMM)
{
	float ReqdShaftCount = 0;
	unsigned long int ReqdShaftCountInt = 0;
	ReqdShaftCount = DistanceInMM / 5.338; // division by resolution to get shaft count
	ReqdShaftCountInt = (unsigned long int) ReqdShaftCount;
	ShaftCountRight = 0;
	while(1)
	{
		if(ShaftCountRight > ReqdShaftCountInt)
		{
			break;
		}
	}
	stop(); //Stop robot
}

//Function used for turning robot by specified degrees
void angle_rotate(unsigned int Degrees)
{
	float ReqdShaftCount = 0;
	unsigned long int ReqdShaftCountInt = 0;
	ReqdShaftCount = (float) Degrees/ 4.090; // division by resolution to get shaft count
	ReqdShaftCountInt = (unsigned int) ReqdShaftCount;
	ShaftCountRight = 0;
	ShaftCountLeft = 0;
	while (1)
	{
		if((ShaftCountRight >= ReqdShaftCountInt) | (ShaftCountLeft >= ReqdShaftCountInt))
		break;
	}
	stop(); //Stop robot
}

void forward_mm(unsigned int DistanceInMM)
{
	forward();
	linear_distance_mm(DistanceInMM);
}
void forward (void)
{
  motion_set (0x06);
}


void left (void) //Left wheel backward, Right wheel forward
{
	motion_set(0x05);
}
void soft_left (void) //Left wheel stationary, Right wheel forward
{
	motion_set(0x04);
}
void left_degrees(unsigned int Degrees)
{
	// 88 pulses for 360 degrees rotation 4.090 degrees per count
	left(); //Turn left
	angle_rotate(Degrees);
}
void adjust_left()
{
	while(1)
	{

		Left_white_line = ADC_Conversion(3);	//Getting data of Left WL Sensor
		Center_white_line = ADC_Conversion(2);	//Getting data of Center WL Sensor
		Right_white_line = ADC_Conversion(1);	//Getting data of Right WL Sensor

		if(Left_white_line<25 && Right_white_line<25 && Center_white_line>25) //this condition will check weather robot is on black line or not and if it is on black line than it will break the loop
		{
			break;
		}
		velocity(200,200);
		left();//this will take left turn until it find black line


	}
}
void precision_left (void)
{
	
	forward_mm(60);
	left_degrees(70);
	adjust_left();
}


void right (void) //Left wheel forward, Right wheel backward
{
	motion_set(0x0A);
}
void soft_right (void) //Left wheel forward, Right wheel is stationary
{
	motion_set(0x02);
}
void right_degrees(unsigned int Degrees)
{
	// 88 pulses for 360 degrees rotation 4.090 degrees per count
	right(); //Turn right
	angle_rotate(Degrees);
}
void adjust_right()
{
	while(1)
	{


		Left_white_line = ADC_Conversion(3);	//Getting data of Left WL Sensor
		Center_white_line = ADC_Conversion(2);	//Getting data of Center WL Sensor
		Right_white_line = ADC_Conversion(1);	//Getting data of Right WL Sensor

		if(Left_white_line<25 && Right_white_line<25 && Center_white_line>25)//this condition will check weather robot is on black line or not and if it is on black line than it will break the loop
		{
			break;
		}
		velocity(200,200);
		right(); //it will take right turn until it find black line

	}
}
void precision_right (void)
{
	
	forward_mm(60);
	right_degrees(70);
	adjust_right();
}




void stop (void)
{
  motion_set (0x00);
}

void init_devices (void)
{
 	cli(); //Clears the global interrupts
	port_init();
	adc_init();
	timer5_init();
	lcd_set_4bit();
	lcd_init();
    left_position_encoder_interrupt_init();
    right_position_encoder_interrupt_init();
	sei();   //Enables the global interrupts
}

void run(void){
	while(1)
	{
		fl=0;
		Left_white_line = ADC_Conversion(3);	//Getting data of Left WL Sensor
		Center_white_line = ADC_Conversion(2);	//Getting data of Center WL Sensor
		Right_white_line = ADC_Conversion(1);	//Getting data of Right WL Sensor
		
		
        forward();
		/*lcd_print(1,9,Left_white_line,2);
		lcd_print(1,12,Center_white_line,2);
		lcd_print(1,15,Right_white_line,2);*/
		if(Left_white_line > 25 &&  Center_white_line <25 && Right_white_line <25)
		{
			stop();
			velocity(220,250);
			soft_left();
		}


		if(Left_white_line<25 && Center_white_line >25 && Right_white_line <25)
		{

			velocity(250,250);
			forward();
			fl=2;
		}

		if(Left_white_line< 25 && Center_white_line <25 && Right_white_line >25)
		{
			
			stop();
			velocity(250,220);
			soft_right();
		}

		if(((Left_white_line > 25 &&  Center_white_line > 25 ) || (Right_white_line> 25 && Center_white_line> 25 )) && (ShaftCountLeft>60 || ShaftCountRight>60))
		{
			node_count++;
			ShaftCountRight=0,ShaftCountLeft=0;
			lcd_cursor(1,2);
			lcd_string(":");
			lcd_print(1,3,node_count,2);
			stop();
			
			break;
		}
		
	}
}

void begin (void)
{
	while (1)
	{
		Left_white_line = ADC_Conversion(3);	//Getting data of Left WL Sensor
		Center_white_line = ADC_Conversion(2);	//Getting data of Center WL Sensor
		Right_white_line = ADC_Conversion(1);	//Getting data of Right WL Sensor
		forward();
		if(((Left_white_line > 40 &&  Center_white_line > 40 ) || (Right_white_line> 40&& Center_white_line> 40 )) && (ShaftCountLeft>15 || ShaftCountRight>15))
		{
			ShaftCountRight=0,ShaftCountLeft=0;
			stop();
			
			break;
		}
	}
	
}