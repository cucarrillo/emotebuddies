// [Emote Buddies]
//
// Author	: Cesar Carrillo
// About	: Watch emoticon move around

// Imports !
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <string.h>
#include <time.h>
#include <ncurses.h>

// *Globals*
#define STR_MAX 15
#define EAV 0.01

#define RELEASE

const char* EB_EYES 	= ":;=Bx";			// List of possible eyes
const char* EB_MOUTH	= "()[]/|^oPD3c";	// List of possible mouths

int SCR_WIDTH 	= 0; // Screen/Terminal dimensions
int SCR_HEIGHT 	= 0;

// Utility 
// TODO: replace this with ncurses method
void setCursor(bool enabled)
{
	if(enabled) { printf("\033[?25h"); }
	else 		{ printf("\033[?25l"); }
}

// returns random int using given range
int randr(int min, int max) { return (rand() % (max - min + 1)) + min; }

// The Emote Buddy Structure!
typedef struct Emote
{
	float	x;			// Position
	float 	y;
	float	velx;		// Velocity
	float 	vely;
	char	emote[2];	// Face (i.e: ":D");
	
	struct Emote* next;	// Next in list

} Emote;

Emote* EB_HEAD = NULL; // Head of the list

// generates a random face and sets it to given emote
void cpyEmote(Emote* emote)
{
	emote->emote[0] = EB_EYES[randr(0, strlen(EB_EYES) - 1)];
	emote->emote[1] = EB_MOUTH[randr(0, strlen(EB_MOUTH) - 1)];
}

// adds a emote to the list
bool addEmote(float x, float y)
{
	if(EB_HEAD == NULL) // empty list ? start a new one
	{
		EB_HEAD = (Emote*) calloc(1, sizeof(Emote)); // TODO: mem check
		
		EB_HEAD->x = x;
		EB_HEAD->y = y;

		cpyEmote(EB_HEAD);

		return true;
	}

	Emote* travel = EB_HEAD;

	while(travel != NULL)
	{
		if(travel->x == x && travel->y == y) { return false; }

		if(travel->next == NULL)
		{
			Emote* np = (Emote*) calloc(1, sizeof(Emote)); // TODO: mem check

			np->x = x;
			np->y = y;

			cpyEmote(np);

			travel->next = np; return true;
		}
		else
		{
			travel = travel->next;
		}
	}

	return false;
}

// draws given emote
void drawEmote(Emote* emote)
{
	move((int)emote->y, (int)emote->x);

	printw("%s", emote->emote);
}

// draws all emotes in list
void drawEmotes()
{
	Emote* travel = EB_HEAD;

	while(travel != NULL)
	{
		drawEmote(travel);

		travel = travel->next;
	}
}

// updates all emotes in list
void updateEmotes()
{
	Emote* travel = EB_HEAD;

	while(travel != NULL)
	{
		// 40 % chance for emote to move
		if(randr(0, 100) > 60)
		{
			if(randr(0, 1) == 0) // which direction ?
			{
				travel->velx += randr(-1, 1); // apply force

				if(travel->velx > 1) { travel->velx = 1; }
				if(travel->velx < -1) { travel->velx = -1; }
			}
			else
			{
				travel->vely += randr(-1, 1); // apply force

				if(travel->vely > 1) { travel->vely = 1; }
				if(travel->vely < -1) { travel->vely = -1; }
			}

			// decrease emote velocities
			if(travel->velx > 0)
			{
				travel->velx -= EAV;
				
				if(travel->velx < 0) { travel->velx = 0; }
			}
			
			if(travel->velx < 0)
			{
				travel->velx += EAV;
				
				if(travel->velx > 0) { travel->velx = 0; }
			}
			if(travel->vely > 0)
			{
				travel->vely -= EAV;
				
				if(travel->vely < 0) { travel->vely = 0; }
			}
			if(travel->vely < 0)
			{
				travel->vely += EAV;
				
				if(travel->vely > 0) { travel->vely = 0; }
			}

			// apply velocites
			travel->x += travel->velx;
			travel->y += travel->vely;

			// apply world constraints
			if(travel->x < 0) { travel->x = 0; travel->velx *= -1; }
			if(travel->y < 0) { travel->y = 0; travel->vely *= -1; }

			if(travel->x > SCR_WIDTH) { travel->x = SCR_WIDTH - 3; travel->velx *= -1; }
			if(travel->y > SCR_HEIGHT) { travel->y = SCR_HEIGHT - 1; travel->vely *= -1; }
		}

		travel = travel->next;
	}
}

// free calloc'd structs
void freeEmotes()
{
	Emote* travel = EB_HEAD;

	while(travel != NULL)
	{
		Emote* tbf = travel;

		travel = travel->next;

		free(tbf);
	}
}

int main(int argc, char** argv)
{
	// TODO: add help page & options
	
	srand(time(NULL));							// random seed
	setCursor(false);							// no cursor
	initscr();									// init ncurses !
	getmaxyx(stdscr, SCR_HEIGHT, SCR_WIDTH); 	// get screen info
	nodelay(stdscr, 1);							// no screen delay

	int c = 0;	// input character
	int ec = 0;	// emote count

	while((c=getch())!='q')
	{
		move(0, 0); // clear screen
		clear();

		if(ec < 50) // add new emote
		{
			if(addEmote(SCR_WIDTH / 2, SCR_HEIGHT / 2))
			{
				ec++;
			}
		}

		// update info on screen size, TODO: make this event driven (maybe)
		getmaxyx(stdscr, SCR_HEIGHT, SCR_WIDTH);

		updateEmotes(); // update & draw emotes
		drawEmotes();

#ifdef DEBUG // debug stuff
		if(EB_HEAD != NULL)
		{
			move(0, 0);
			printw("[Emote Head]\n");
			printw("x: %f\n", EB_HEAD->x);
			printw("y: %f\n", EB_HEAD->y);
			printw("vel x: %f\n", EB_HEAD->velx);
			printw("vel y: %f\n", EB_HEAD->vely);
		}
#endif

		refresh();		// refresh & stuff
		usleep(55000);
	}

	// free them
	freeEmotes();

	endwin();
	setCursor(true);
	
	return 0;
}
