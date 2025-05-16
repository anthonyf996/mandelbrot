#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <complex.h>
#include <SDL2/SDL.h>

#define WINDOW_TITLE "Mandelbrot"
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 800
#define REAL_MIN -2
#define REAL_MAX 2
#define IMAG_MIN -2
#define IMAG_MAX 2
#define DEFAULT_R 0
#define DEFAULT_G 0
#define DEFAULT_B 0

struct App {
	SDL_Window *window;
	SDL_Renderer *renderer;
	double delay_time;
} app;

bool app_init(struct App *app) {
	if (SDL_Init(SDL_INIT_VIDEO)) {
		fprintf(stderr, "Error initializing SDL! %s\n", SDL_GetError());
		return 1;
	}
	if ((app->window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0)) == NULL) {
		fprintf(stderr, "Error creating window! %s\n", SDL_GetError());
		return 2;
	}
	if ((app->renderer = SDL_CreateRenderer(app->window, -1, 0)) == NULL) {
		fprintf(stderr, "Error creating renderer! %s\n", SDL_GetError());
		return 3;
	}
	srand((unsigned int)time(NULL));
	return 0;
}

void app_reset(struct App *app) {
	app->window = NULL;
	app->renderer = NULL;
}

void app_destroy(struct App *app) {
	SDL_DestroyRenderer(app->renderer);
	SDL_DestroyWindow(app->window);
	SDL_Quit();
	app_reset(app);
}

void draw_point(struct App *app, int x, int y, Uint8 r, Uint8 g, Uint8 b) {
		SDL_SetRenderDrawColor(app->renderer, r, g, b, 255);
		SDL_RenderDrawPoint(app->renderer, x, y);
		SDL_SetRenderDrawColor(app->renderer, DEFAULT_R, DEFAULT_G, DEFAULT_B, 255);
}

double scale_coord(double target_scale_min, double target_scale_max, double coord_to_scale, double coord_scale_max) {
	return target_scale_min + (target_scale_max - target_scale_min) * coord_to_scale / coord_scale_max;
}

unsigned int is_in_mandelbrot_set(double x, double y) {
	const unsigned int MAX_ITERATIONS = 700;
	double complex c = x + y * I;
	double complex z = 0;
	unsigned int i = 0;
	while (abs(z) <= 2 && i < MAX_ITERATIONS) {
		z = z * z + c;
		i++;
	}
	if (i == MAX_ITERATIONS) {
		return 0;
	}
	return i;
}

void render_mandelbrot(struct App *app, double real_min, double real_max, double imag_min, double imag_max, int r, int g, int b, unsigned int pixels[WINDOW_HEIGHT][WINDOW_WIDTH]) {
	const unsigned int rgb_scale = 2;
	for (unsigned int x = 0; x < WINDOW_WIDTH; x++) {
		for (unsigned int y = 0; y < WINDOW_HEIGHT; y++) {
			double x_scaled = scale_coord(real_min, real_max, x, WINDOW_WIDTH);
			double y_scaled = scale_coord(imag_min, imag_max, y, WINDOW_HEIGHT);
			unsigned int result = is_in_mandelbrot_set(x_scaled, y_scaled);
			pixels[y][x] = result;
			if (result == 0) {
				draw_point(app, x, y, 0, 0, 0);
			} else {
				draw_point(app, x, y, 255 - result * rgb_scale - r, 255 - result * rgb_scale - g, 255 - result * rgb_scale - b);
			}
		}
	}
}

void draw_mandelbrot(struct App *app, int r, int g, int b, unsigned int pixels[WINDOW_HEIGHT][WINDOW_WIDTH]) {
	int scale = 2;
	for (unsigned int x = 0; x < WINDOW_WIDTH; x++) {
		for (unsigned int y = 0; y < WINDOW_HEIGHT; y++) {
			unsigned int result = pixels[y][x];
			if (result == 0) {
				draw_point(app, x, y, 0, 0, 0);
			} else {
				draw_point(app, x, y, 255 - result * scale - r, 255 - result * scale - g, 255 - result * scale - b);
			}
		}
	}
}

int main(void) {
	double DELAY_TIME = 1000.0 / 60.0;
	struct App app = {
		.window = NULL,
		.renderer = NULL,
		.delay_time = DELAY_TIME
	};
	unsigned int pixels[WINDOW_HEIGHT][WINDOW_WIDTH] = {0};
	double real_min = REAL_MIN;
	double real_max = REAL_MAX;
	double real_diff = REAL_MAX - REAL_MIN;
	double imag_min = IMAG_MIN;
	double imag_max = IMAG_MAX;
	double imag_diff = IMAG_MAX - IMAG_MIN;
	double real_scaled = 1.0;
	double imag_scaled = 1.0;
	int zoom_factor = 2;
	int zoom_count = 2;
	int rand_r = rand() % 256;
	int rand_g = rand() % 256;
	int rand_b = rand() % 256;

	if (app_init(&app)) {
		fprintf(stderr, "Error initializing app! %s\n", SDL_GetError());
		app_destroy(&app);
		return 1;
	}


	SDL_RenderClear(app.renderer);
	render_mandelbrot(&app, REAL_MIN, REAL_MAX, IMAG_MIN, IMAG_MAX, rand_r, rand_g, rand_b, pixels);
	while (true) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT:
					app_destroy(&app);
					return 0;
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.scancode) {
						case SDL_SCANCODE_ESCAPE:
							app_destroy(&app);
							return 0;
							break;
						case SDL_SCANCODE_SPACE:
							render_mandelbrot(&app, REAL_MIN, REAL_MAX, IMAG_MIN, IMAG_MAX, rand_r, rand_g, rand_b, pixels);
							real_min = REAL_MIN;
							real_max = REAL_MAX;
							imag_min = IMAG_MIN;
							imag_max = IMAG_MAX;
							real_scaled = 1.0;
							imag_scaled = 1.0;
							break;
						case SDL_SCANCODE_C:
							rand_r = rand() % 256;
							rand_g = rand() % 256;
							rand_b = rand() % 256;
							draw_mandelbrot(&app, rand_r, rand_g, rand_b, pixels);
							break;
						default:
							break;
					}
					break;
				case SDL_MOUSEBUTTONDOWN:
						real_scaled = scale_coord(real_min, real_max, event.button.x, WINDOW_WIDTH);
						imag_scaled = scale_coord(imag_min, imag_max, event.button.y, WINDOW_HEIGHT);
						real_diff = real_max - real_min;
						real_min = real_scaled - (real_diff / (2.0 * zoom_factor));
						real_max = real_scaled + (real_diff / (2.0 * zoom_factor));
						imag_diff = imag_max - imag_min;
						imag_min = imag_scaled - (imag_diff / (2.0 * zoom_factor));
						imag_max = imag_scaled + (imag_diff / (2.0 * zoom_factor));
					render_mandelbrot(&app, real_min, real_max, imag_min, imag_max, rand_r, rand_g, rand_b, pixels);
					break;
				default:
					break;
			}
		}
		SDL_RenderPresent(app.renderer);
		SDL_Delay(app.delay_time);
	}
	
	app_destroy(&app);
	return 0;
}
