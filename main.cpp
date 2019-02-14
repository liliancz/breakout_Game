#pragma once
#include <iostream>
#include <assert.h>
#include <math.h>
#include <ctime>
#include <cstdlib>
using namespace std;

//include SDL header
#include "SDL2-2.0.8/include/SDL.h"

//load libraries
#pragma comment(lib, "SDL2-2.0.8\\lib\\x86\\SDL2.lib")
#pragma comment(lib, "SDL2-2.0.8\\lib\\x86\\SDL2main.lib")
#pragma comment(linker, "/subsystem:console")

void fill_rectangle(unsigned char*buffer, int buffer_width, int buffer_height, int rect_x, int rect_y, int rect_w, int rect_h, int r, int g, int b, int a)
{
	for (int y = 0; y < rect_h; y++)
	{
		for (int x = 0; x < rect_w; x++)
		{
			int index = 4 * (rect_y + y)*buffer_width + 4 * (x + rect_x);
			buffer[index] = r;
			buffer[index + 1] = g;
			buffer[index + 2] = b;
			buffer[index + 3] = a;
		}

	}
}

SDL_Renderer *renderer = NULL;
int screen_width = 800;
int screen_height = 600;

SDL_Window *window = NULL;

struct Box
{
	float x, y;
	float w, h;
	float speed_x, speed_y;
	int r, g, b, a;
	int count_hits;
};

struct Border
{
	float x, y;
	float w, h;
};


// int collision_with_Dir(const Data *a, const Data *b)
int collision_with_dir(float *rect_A_x, float *rect_A_y, float *rect_A_w, float *rect_A_h, float *rect_B_x, float *rect_B_y, float *rect_B_w, float *rect_B_h)
{
	enum { NO_COLLISION , TOP_OF_B, RIGHT_OF_B, BOTTOM_OF_B, LEFT_OF_B };

	float w = 0.5 * (*rect_A_w + *rect_B_w);
	float h = 0.5 * (*rect_A_h + *rect_B_h);
	float dx = *rect_A_x - *rect_B_x + 0.5*(*rect_A_w - *rect_B_w);
	float dy = *rect_A_y - *rect_B_y + 0.5*(*rect_A_h - *rect_B_h);

	if (dx*dx <= w * w && dy*dy <= h * h)
	{
		float wy = w * dy;
		float hx = h * dx;

		if (wy > hx)
		{
			return (wy + hx > 0) ? BOTTOM_OF_B : LEFT_OF_B;
		}
		else
		{
			return (wy + hx > 0) ? RIGHT_OF_B : TOP_OF_B;
		}
	}
	return NO_COLLISION;
}

//void impulse
void impulse(float *rect_A_x, float *rect_A_y, float *rect_A_w, float *rect_A_h, float *rect_A_speed_x, float *rect_A_speed_y, float *a_inv_mass, 
	float *rect_B_x, float *rect_B_y, float *rect_B_w, float *rect_B_h, float *rect_B_speed_x, float *rect_B_speed_y, float *b_inv_mass)
{
	float normal_x;
	float normal_y;

	int k = collision_with_dir(rect_A_x, rect_A_y, rect_A_w, rect_A_h, rect_B_x, rect_B_y, rect_B_w, rect_B_h);
	if (k == 1)
	{
		normal_x = 0;
		normal_y = -1;
	}
	else if (k == 2)
	{
		normal_x = 1;
		normal_y = 0;
	}
	else if (k == 3)
	{
		normal_x = 0;
		normal_y = 1;
	}
	else if (k == 4)
	{
		normal_x = -1;
		normal_y = 0;
	}
	else
	{
		normal_x = 0;
		normal_y = 0;
	}

	float relative_speed_x = *rect_B_speed_x;
	float relative_speed_y = *rect_B_speed_y;

	relative_speed_x -= *rect_A_speed_x; 
	relative_speed_y -= *rect_A_speed_y; 

	float d = relative_speed_x * normal_x + relative_speed_y * normal_y; // dot product

	if (d < 0) return;

	float e = 1.0; // 1.0 is for elastic bounce, 0.0 for mud
	float j = -(1.0 + e)*d / (*a_inv_mass + *b_inv_mass); // mass of tile is infinity (1/ma+1/mb), if you use the actual mass then objects will bounce

	float impulse_a_x = normal_x;
	float impulse_a_y = normal_y;

	impulse_a_x *= j * *a_inv_mass;  //scale
	impulse_a_y *= j * *a_inv_mass;

	float impulse_b_x = normal_x;
	float impulse_b_y = normal_y;

	impulse_b_x *= j * *b_inv_mass;  //scale
	impulse_b_y *= j * *b_inv_mass;

	*rect_A_speed_x -= impulse_a_x;  // sub
	*rect_A_speed_y -= impulse_a_y;

	*rect_B_speed_x += impulse_b_x;	//add
	*rect_B_speed_y += impulse_b_y;

	float mt = 1.0 / (*a_inv_mass + *b_inv_mass);

	float tangent_x = normal_y;
	float tangent_y = -normal_x;

	float tangent_d = relative_speed_x * tangent_x + relative_speed_y * tangent_y; // dot product

	tangent_x *= (-tangent_d)*mt; //scale
	tangent_y *= (-tangent_d)*mt;

	*rect_A_speed_x -= tangent_x;  //sub
	*rect_A_speed_y -= tangent_y;

	*rect_B_speed_x += tangent_x;  //add
	*rect_B_speed_y += tangent_y;

}

//impulse against immovable object
void impulse(float *rect_A_x, float *rect_A_y, float *rect_A_w, float *rect_A_h, float *rect_A_speed_x, float *rect_A_speed_y, float *a_inv_mass,
	float *rect_B_x, float *rect_B_y, float *rect_B_w, float *rect_B_h)
{
	float normal_x;
	float normal_y;

	int k = collision_with_dir(rect_A_x, rect_A_y, rect_A_w, rect_A_h, rect_B_x, rect_B_y, rect_B_w, rect_B_h);
	if (k == 1)
	{
		normal_x = { 0 };
		normal_y = { -1 };
	}
	else if (k == 2)
	{
		normal_x = { 1 };
		normal_y = { 0 };
	}
	else if (k == 3)
	{
		normal_x = { 0 };
		normal_y = { 1 };
	}
	else if (k == 4)
	{
		normal_x = { -1 };
		normal_y = { 0 };
	}
	else
	{
		normal_x = { 0 };
		normal_y = { 0 };
	}


	float relative_vel_x = -*rect_A_speed_x;
	float relative_vel_y = -*rect_A_speed_y;

	float d = relative_vel_x * normal_x + relative_vel_y * normal_y;

	if (d < 0) return;

	float e = 1.0;//1.0 is for elastic bounce, 0.0 for mud
	float j = -(1.0 + e) * d / (*a_inv_mass);//mass of tile is infinity (1/ma+1/mb), if you use actual mass then objects will bounce

	float impulse_x = normal_x;
	float impulse_y = normal_y;

	impulse_x *= j;
	impulse_y *= j;

	*rect_A_speed_x -= impulse_x;
	*rect_A_speed_y -= impulse_y;
}


int main(int argc, char**argv)
{
	SDL_Init(SDL_INIT_VIDEO);

	window = SDL_CreateWindow("ALEJANDRA:  teachmehowtocode", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		screen_width, screen_height, SDL_WINDOW_SHOWN);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

	SDL_Surface*your_draw_buffer = SDL_CreateRGBSurfaceWithFormat(0, screen_width, screen_height, 32, SDL_PIXELFORMAT_RGBA32);
	SDL_Surface*screen = SDL_GetWindowSurface(window);

	SDL_SetSurfaceBlendMode(your_draw_buffer, SDL_BLENDMODE_NONE);

	unsigned char *my_own_buffer = (unsigned char*)malloc(sizeof(unsigned char)*screen_width*screen_height * 4);
		
	// BORDERS
	Border *border = (Border*)malloc(sizeof(Border) * 4);
	{	// TOP
		border[0].x = 1;
		border[0].y = 1;
		border[0].w = screen_width - 1;
		border[0].h = 30;
		//BOTTOM
		border[1].x = 1;
		border[1].h = 30;
		border[1].y = screen_height - border[1].h - 1;
		border[1].w = screen_width - 1;
		//LEFT
		border[2].x = 1;
		border[2].y = 1;
		border[2].w = 15;
		border[2].h = screen_height - 1;
		//RIGHT
		border[3].y = 1;
		border[3].w = 15;
		border[3].h = screen_height - 1;
		border[3].x = screen_width - border[3].w - 1;
	}
	
	float a_inv_mass = 1;
	float b_inv_mass = 1;
	
	
	// Inicial values for the ball
	int n_ball = 2;
	Box *ball = (Box*)malloc(sizeof(Box)*n_ball);
	for (int i = 0; i < n_ball; i++)
	{
		ball[i].x = screen_width / 2;
		ball[i].w = 8;
		ball[i].h = 8;
		ball[i].y = 500;
		ball[i].r = 255;
		ball[i].g = 255;
		ball[i].b = 255;
		ball[i].a = 255;
		ball[i].speed_x = 1.0 - 2.0*rand() / RAND_MAX;
		ball[i].speed_y = (- 2.0*rand() / RAND_MAX);

		ball[i].count_hits = 0;
	}

	int one_pallet = 1;
	Box *pallet = (Box*)malloc(sizeof(Box)*one_pallet);
	{
		pallet->x = screen_width / 2;
		pallet->w = 80;
		pallet->h = 5;
		pallet->y = screen_height - pallet->h - border[1].h - 5;
		pallet->r = 255;
		pallet->g = 255;
		pallet->b = 255;
		pallet->a = 255;
		pallet->speed_x = 3;
		pallet->speed_y = 0;
		pallet->count_hits = 0;
	}

	
	int n_bricks_y = 15;
	int n_bricks_x = 28;
	int n_bricks_max = n_bricks_x * n_bricks_y;

	int n_max_hits = 1;
	
	// BRICKS
	Box *bricks = (Box*)malloc(sizeof(Box)*n_bricks_max);
	for (int i = 0; i < n_bricks_max; i++)
	{
		bricks[i].x = 100;
		bricks[i].y = 100;
		bricks[i].w = 20;
		bricks[i].h = 10;
		bricks[i].r = 255;
		bricks[i].g =  rand() % 256;
		bricks[i].b = 255;
		bricks[i].a = 256;
		bricks[i].speed_x = 0;
		bricks[i].speed_y = 0;
		bricks[i].count_hits = 0;
	}

	int count=0;
	int y = 1; // 
	int temp = 0;
	// SCREEEN FULL OF BRIcKS
	
	for (int k = 0; k < n_bricks_max; k++)
	{		
		for (int i = count; i < y*n_bricks_x && count < n_bricks_max; i++)
		{
			bricks[i + 1].x = bricks[i].x + bricks[i].w + 1;
			bricks[i].y = bricks[temp].y;
			count++;
		}
		bricks[count].x = bricks[count - n_bricks_x].x; // if count is 9, count-bricks = 0
		bricks[count].y = bricks[count - n_bricks_x].y + bricks[count - n_bricks_x].h + 1;
		y++;
		temp += n_bricks_x;	
	}
	

	for (;;)
	{
		//Consume all window events first
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				exit(0);
			}
		}

		// TO "DELETE" BOXES
		for (int i = 0; i < n_bricks_max; i++)
		{
			if (bricks[i].count_hits >= n_max_hits)
			{
				bricks[i].r = 0;
				bricks[i].g = 0;
				bricks[i].b = 0;
				bricks[i].a = 256;
				bricks[i].x = 0;
				bricks[i].y = 0;
				bricks[i].w = 0;
				bricks[i].h = 0;
				//Update_inicial_values(&box[i].x, &box[i].y, &box[i].w, &box[i].h, &box[i].r, &box[i].g, &box[i].b, &box[i].a, &box[i].speed_x, &box[i].speed_y, &box[i].count_hits);
			}
				
		}
		

		for (int i = 0; i < n_ball; i++)
		{
			// BALL VS BORDER 
			for (int k = 1; k <= 4; k++)
			{
				impulse(&ball[i].x, &ball[i].y, &ball[i].w, &ball[i].h, &ball[i].speed_x, &ball[i].speed_y, &a_inv_mass, &border[k].x, &border[k].y, &border[k].w, &border[k].h);
			}
		}
		
		// COLLISION 
		for (int i = 0; i < n_ball; i++)
		{
			// BALL VS BRICKS
			for (int j = 0; j < n_bricks_max; j++)
			{
				impulse(&ball[i].x, &ball[i].y, &ball[i].w, &ball[i].h, &ball[i].speed_x, &ball[i].speed_y, &a_inv_mass,
					&bricks[j].x, &bricks[j].y, &bricks[j].w, &bricks[j].h);
			}
		}
		// BALL COLLISION
		for (int i = 0; i < n_ball; i++)
		{	
	
			// BALL VS BALL
			for (int j = i + 1; j < n_ball; j++)
			{
				impulse(&ball[i].x, &ball[i].y, &ball[i].w, &ball[i].h, &ball[i].speed_x, &ball[i].speed_y, &a_inv_mass,
					&ball[j].x, &ball[j].y, &ball[j].w, &ball[j].h, &ball[j].speed_x, &ball[j].speed_y, &b_inv_mass);
			}

			// BALL VS PALLET
			impulse(&ball[i].x, &ball[i].y, &ball[i].w, &ball[i].h, &ball[i].speed_x, &ball[i].speed_y, &a_inv_mass,
							&pallet->x, &pallet->y, &pallet->w, &pallet->h);

		}

		for (int i = 0; i < n_ball; i++)
		{
			// BALL VS BORDER 
			for (int k = 0; k < 4; k++)
			{
				impulse(&ball[i].x, &ball[i].y, &ball[i].w, &ball[i].h, &ball[i].speed_x, &ball[i].speed_y, &a_inv_mass, &border[k].x, &border[k].y, &border[k].w, &border[k].h);
			}
		}


		float middle_pallet = pallet->x + (pallet->w / 2);
		// PALLET FOLLOWS THE BALL
		for (int i=0; i < n_ball; i++)
		{	
			float middle_ball = ball[i].x + ball[i].w / 2;
			if (ball[i].y > screen_height / 2)
			{
				if (middle_pallet < middle_ball) { if ((pallet->x + pallet->w) < screen_width) { pallet->x += pallet->speed_x; } } // move pallet to the right >>>>>
				if (middle_pallet > middle_ball) { if (pallet->x > 0) { pallet->x -= pallet->speed_x; } }// move pallet to the left <<<<<<
			}
			// PALLET GO BACK TO THE MIDDLE
			if (ball[i].y < screen_height / 5)
			{
				if (middle_pallet < screen_width/2) pallet->x += pallet->speed_x*0.5;// move pallet to the right >>>>>
				if (middle_pallet > screen_width/2)	if (pallet->x > 0)pallet->x -= pallet->speed_x*0.5;// move pallet to the left <<<<<<
			}

		}
		
		
		
		
		
		// COUNT HITS
		int k_bricks=0;
		for (int i = 0; i < n_bricks_max; i++)
		{
			for (int j = 0; j < n_ball; j++)
			{
				k_bricks = collision_with_dir(&ball[j].x, &ball[j].y, &ball[j].w, &ball[j].h, &bricks[i].x, &bricks[i].y, &bricks[i].w, &bricks[i].h);
				if (k_bricks != 0) bricks[i].count_hits++;
			}
		}

		memset(my_own_buffer, 0, 4 * screen_width*screen_height);

		// TO FILL BALL
		for (int i = 0; i < n_ball; i++)
		{
			fill_rectangle(my_own_buffer, screen_width, screen_height, ball[i].x, ball[i].y, ball[i].w, ball[i].h, ball[i].r, ball[i].g, ball[i].b, ball[i].a);
			ball[i].x += ball[i].speed_x;
			ball[i].y += ball[i].speed_y;

		}

		// TO FILL PALLET
		fill_rectangle(my_own_buffer, screen_width, screen_height, pallet->x, pallet->y, pallet->w, pallet->h, pallet->r, pallet->g, pallet->b, pallet->a);

		// TO FILL BRICKS   
		for (int i = 0; i < n_bricks_max; i++)
		{
			fill_rectangle(my_own_buffer, screen_width, screen_height, bricks[i].x, bricks[i].y, bricks[i].w, bricks[i].h, bricks[i].r, bricks[i].g, bricks[i].b, bricks[i].a);
		}

		// TO FILL THE BORDERS 

		for (int j = 0; j < 4; j++)
		{
			fill_rectangle(my_own_buffer, screen_width, screen_height, border[j].x, border[j].y, border[j].w, border[j].h, 255, 0, 0, 255);
		}

		//printf("/nBALL 1  pos x %f", ball[0].y);
		//printf("BALL 2  pos x %f", ball[1].y);
		//printf("BALL 3  pos x %f", ball[2].y);
		//printf("BALL 4  pos x %f", ball[3].y);



		memcpy(your_draw_buffer->pixels, my_own_buffer, sizeof(unsigned char)*screen_width*screen_height * 4);

		//BLIT BUFFER TO SCREEN
		SDL_BlitScaled(your_draw_buffer, NULL, screen, NULL);
		SDL_UpdateWindowSurface(window);
		
	}
	return 0;
}
