#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include "RTL.h"
#include "LPC17xx.H"
#include "GLCD.h"
#include "LED.h"
#include "KBD.h"
#include "ADC.h"
#include "MPU9250.h"
#include "joystick.h"

#define B   Black
#define W   White
#define M   Maroon

#define MAX_BALLS 10

OS_TID t_paddle_motion;   //assigned task ID of Task 1: Paddle Motion
OS_TID t_ball_motion;     //assigned task ID of Task 2: Ball Motion
OS_TID t_life_level;      //assigned task ID of Task 3: Life Level
OS_TID t_game_level;      //assigned task ID of Task 4: Game Level

OS_MUT mut_GLCD;          //mutex to control GLCD access

unsigned int ADCStat = 0;
unsigned int ADCValue = 0;

// July 20th    Farayha & Kristine 
/*----------------------------------------------------------------------------
	GRAPHICS
 *---------------------------------------------------------------------------*/
// Ball graphic
unsigned short ball[] = { W, W, W, B, B, B, B, W, W, W,
                          W, W, B, B, B, B, B, B, W, W,
                          W, B, B, B, B, B, B, B, B, W,
                          W, B, B, B, B, B, B, B, B, W,
                          B, B, B, B, B, B, B, B, B, B,
                          B, B, B, B, B, B, B, B, B, B,
                          W, B, B, B, B, B, B, B, B, W,
                          W, B, B, B, B, B, B, B, B, W,
                          W, W, B, B, B, B, B, B, W, W,
                          W, W, W, B, B, B, B, W, W, W };

// Ball Overwrite graphic
unsigned short ball_overwrite[] = { W, W, W, W, W, W, W, W, W, W,
                                    W, W, W, W, W, W, W, W, W, W,
                                    W, W, W, W, W, W, W, W, W, W,
                                    W, W, W, W, W, W, W, W, W, W,
                                    W, W, W, W, W, W, W, W, W, W,
                                    W, W, W, W, W, W, W, W, W, W,
                                    W, W, W, W, W, W, W, W, W, W,
                                    W, W, W, W, W, W, W, W, W, W,
                                    W, W, W, W, W, W, W, W, W, W,
                                    W, W, W, W, W, W, W, W, W, W };

// Paddle graphic
unsigned short paddle[] = { W, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, W,
                            B, B, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, B, B,
                            B, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, B,
                            B, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, B,
                            B, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, B,
                            B, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, B,
                            B, B, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, M, B, B,
                            W, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, W };
  
// Paddle Overwrite graphic
unsigned short paddle_overwrite[] = { W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W,
                                      W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W,
                                      W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W,
                                      W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W,
                                      W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W,
                                      W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W,
                                      W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W,
                                      W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W };

/*----------------------------------------------------------------------------
	GLOBAL VARIABLES
 *---------------------------------------------------------------------------*/
// Ball struct
typedef struct ball {
    unsigned int x_ball;
	unsigned int y_ball;
	unsigned int dx;
	unsigned int dy;
    int x_dir;
    int y_dir;
} ball_t;

// Ball array; maximum 10 balls on screen at a time
ball_t ball_spawn[10];

// Initialize paddle position and size
unsigned int x_paddle = 120;
unsigned int y_paddle = 300;

// Initialize global variables
unsigned int life = 5;              // number of lives
unsigned int button_press = 0;      // bool for int() button press
unsigned int on_screen = 1;         // number of balls on screen
unsigned int paddle_hit = 0;        // number of paddle hits
unsigned int ball_speed = 20;       // ball speed (delay for ball_motion task)

/*----------------------------------------------------------------------------
	HELPER FUNCTIONS
 *---------------------------------------------------------------------------*/
// Initializes ball struct parameters
void ball_init (void) {
    int i;
	
	for(i = 0; i < MAX_BALLS; i++) {
		ball_spawn[i].x_ball = 100;
		ball_spawn[i].y_ball = 0;
		ball_spawn[i].dx = 10;
		ball_spawn[i].dy = 10;
        ball_spawn[i].x_dir = 1;
        ball_spawn[i].y_dir = -1;
	}
}

// Interrupt handler for INT() button
void EINT3_IRQHandler (void) {
    NVIC_DisableIRQ(EINT3_IRQn);
    LPC_GPIOINT->IO2IntClr |= (1 << 10);
    button_press = 1;
    NVIC_EnableIRQ(EINT3_IRQn);
}

// INT() button set-up
void button_setup (void) {
    LPC_PINCON->PINSEL4 &= ~(3 << 20);
    LPC_GPIO2->FIODIR &= ~(1 << 10);
    LPC_GPIOINT->IO2IntEnF |= (1 << 10);
    NVIC_EnableIRQ(EINT3_IRQn);
}

// Overwrites paddle graphic for left horizontal movement
void paddle_left (void) {
    os_mut_wait(mut_GLCD, 0xffff);
    GLCD_Bitmap(x_paddle, y_paddle, 50, 8, (unsigned char*)paddle_overwrite);
    if (x_paddle > 0) x_paddle -= 5;
    GLCD_Bitmap(x_paddle, y_paddle, 50, 8, (unsigned char*)paddle);
    os_mut_release(mut_GLCD);
}

// Overwrites paddle graphic for right horizontal movement
void paddle_right (void) {
    os_mut_wait(mut_GLCD, 0xffff);
    GLCD_Bitmap(x_paddle, y_paddle, 50, 8, (unsigned char*)paddle_overwrite);
    if (x_paddle < 190) x_paddle += 5;
    GLCD_Bitmap(x_paddle, y_paddle, 50, 8, (unsigned char*)paddle);
    os_mut_release(mut_GLCD);
}

// Determines ball(s) movement on LCD screen 
void ball_move (unsigned int i) {
    int j;
    
	//Ball movement in horizontal direction
	if (ball_spawn[i].x_ball <= 1 || ball_spawn[i].x_ball+10 >= 239) { //if ball hits left or right wall
		ball_spawn[i].x_dir *= -1;
		ball_spawn[i].x_ball += ball_spawn[i].dx * ball_spawn[i].x_dir;
	} 
	else { //if ball doesn't hit left/right wall
	    ball_spawn[i].x_ball += ball_spawn[i].dx * ball_spawn[i].x_dir;
	}
	
	//Ball movement in vertical direction
	if (ball_spawn[i].y_ball <= 1) { //if ball hits top wall
		ball_spawn[i].y_dir *= -1;
		ball_spawn[i].y_ball += ball_spawn[i].dy * ball_spawn[i].y_dir;
	}
	else if ((ball_spawn[i].y_ball+10 == 300) && (ball_spawn[i].x_ball+6 >= x_paddle) && (ball_spawn[i].x_ball+3 <= x_paddle+50)) { //if ball hits paddle
		ball_spawn[i].y_dir *= -1;
		ball_spawn[i].y_ball += ball_spawn[i].dy * ball_spawn[i].y_dir;
        paddle_hit += 1;
	}
	else if (ball_spawn[i].y_ball > 1 && (ball_spawn[i].y_ball+10) <= 320) { //if ball doesn't hit top wall
        ball_spawn[i].y_ball += ball_spawn[i].dy * ball_spawn[i].y_dir;
    }
	else if ((ball_spawn[i].y_ball+10) > 320) { //if ball falls to bottom edge
		// decrement life
        life -= 1;
        // decrement number of balls on screen
        on_screen--;
        // shift balls in array
        for (j = i; j < MAX_BALLS; j++) {
            ball_spawn[j] = ball_spawn[j+1];
        }
        // initialize new ball for later
        ball_spawn[MAX_BALLS-1].x_ball = 100;
        ball_spawn[MAX_BALLS-1].y_ball = 0;
        ball_spawn[MAX_BALLS-1].dx = 10;
        ball_spawn[MAX_BALLS-1].dy = 10;
        ball_spawn[MAX_BALLS-1].x_dir = 1;
        ball_spawn[MAX_BALLS-1].y_dir = -1;
	}
}

// Output shown on screen at start of game
void game_start (void) {
    GLCD_Clear(W);
    GLCD_DisplayString(6, 2, 1, "DIGI SQUASH");
}

// Output shown on screen when game is over
void game_over (void) {
    GLCD_Clear(W);
    GLCD_DisplayString(6, 2, 1, " GAME OVER");
    GLCD_DisplayString(22, 10, 0, (unsigned char*)paddle_hit/*(unsigned char*)itoa(paddle_hit)*/);
}

/*----------------------------------------------------------------------------
	TASKS
 *---------------------------------------------------------------------------*/
// Task 1 (Paddle Motion): moves paddle position along the horizontal direction
__task void paddle_motion (void) {
	GLCD_Bitmap(x_paddle, y_paddle, 50, 8, (unsigned char*)paddle);
	while(1) {
        // if joystick reads left
		if (joystick_read() == JOY_LEFT) {
            paddle_left();
		}
        // if joystick reads right
        else if (joystick_read() == JOY_RIGHT) {
			paddle_right();
        }
        os_dly_wait(5);
	}
}

// Task 2 (Ball Motion): defines movement of ball(s) in x & y directions
__task void ball_motion (void) {
    unsigned int i;
  
    while(1) {
        // if int() button is pressed, add more balls (max 10 balls on screen)
        if(button_press == 1) {
            if(on_screen < 10) {
                on_screen++;
            }
        button_press = 0;
        }
    
        // draw balls on screen and move them
        for(i = 0; i < on_screen; i++) {
            os_mut_wait(mut_GLCD, 0xffff);
            GLCD_Bitmap(ball_spawn[i].x_ball, ball_spawn[i].y_ball, 10, 10, (unsigned char*)ball_overwrite);
            ball_move(i);
            GLCD_Bitmap(ball_spawn[i].x_ball, ball_spawn[i].y_ball, 10, 10, (unsigned char*)ball);
            os_mut_release(mut_GLCD);
        }
        // ball speed is determined by delay
        os_dly_wait(ball_speed);
  }
}

// Task 3 (Life Level): turns on/off LED lights to indicate life level in game
__task void life_level (void) {	
    while(1) {
        // if lives remaining, output number to LEDs
		if(life > 0)
			LED_display((1 << life) - 1);
        // if no lives left, turn off LEDs, delete tasks, and output game over screen
        else {
            LED_display(0);
            os_tsk_delete(t_ball_motion);
            os_tsk_delete(t_paddle_motion);
            os_tsk_delete(t_game_level);
            game_over();
            os_tsk_delete_self();
        }
        os_dly_wait(10);
	}
}

// Task 4 (Game Level): increases game difficulty level after a period of time
__task void game_level (void) {
    while(1) {
        // Depending on number of paddle hits, add balls and change ball speed by changing delay
        if (paddle_hit < 5) {
            ball_speed = 20;
        }
        else if (paddle_hit >= 5) {
            if(on_screen < 2) on_screen++;
            ball_speed = 15;
        }
        else if (paddle_hit >= 10) {
            if(on_screen < 3) on_screen++;
            ball_speed = 10;
        }
        else if (paddle_hit >= 20) {
            if(on_screen < 5) on_screen++;
            ball_speed = 5;
        }
        else if (paddle_hit >= 40 && paddle_hit < 50) {
            if(on_screen < 7) on_screen++;
            ball_speed = 4;
        }
        else {
            on_screen = 10;
            ball_speed = 2;
        }
        os_dly_wait(10);
    }
}

// Task 5 (Initialization): initializes all main tasks and shows game start up screen
__task void init (void) {
    // Initialize mutex for GLCD screen
	os_mut_init(mut_GLCD);
    
    // Initialize ball values
    ball_init();
    
    // Game Title Screen
    game_start();
	
    os_dly_wait(200);
    GLCD_Clear(W);
    
    // Create/Start tasks
	t_paddle_motion = os_tsk_create (paddle_motion, 0);
	t_ball_motion = os_tsk_create (ball_motion, 0);
	t_life_level = os_tsk_create (life_level, 0);
    t_game_level = os_tsk_create (game_level, 0);
    
	os_tsk_delete_self ();
}

/*----------------------------------------------------------------------------
	MAIN FUNCTION
 *---------------------------------------------------------------------------*/
int main(void) {
	NVIC_EnableIRQ( ADC_IRQn );				/* Enable ADC interrupt handler		*/
	
    button_setup();
	LED_setup();							/* Initialize the LEDs				*/
	GLCD_Init();							/* Initialize the GLCD				*/
	KBD_Init ();							/* initialize Push Button			*/
	ADC_Init ();							/* initialize the ADC				*/

	GLCD_Clear(White);						/* Clear the GLCD					*/

	os_sys_init(init);						/* Initialize RTX and start init	*/
}