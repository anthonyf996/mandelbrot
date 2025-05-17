#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <complex.h>
#include <SDL2/SDL.h>

#define WINDOW_TITLE "Mandelbrot"
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 800
#define DEFAULT_REAL_MIN -2
#define DEFAULT_REAL_MAX 2
#define DEFAULT_IMAG_MIN -2
#define DEFAULT_IMAG_MAX 2
#define DEFAULT_R 0
#define DEFAULT_G 0
#define DEFAULT_B 0

struct App {
	SDL_Window *window;
	SDL_Renderer *renderer;
	double delay_time;
} app;

struct Mandelbrot {
	double real_min;
	double real_max;
	double imag_min;
	double imag_max;
	int zoom_factor;
	int r;
	int g;
	int b;
	unsigned int iter_count_at_pixel[WINDOW_HEIGHT][WINDOW_WIDTH];
};

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
	unsigned int iter_count = 0;
	while (abs(z) <= 2 && iter_count < MAX_ITERATIONS) {
		z = z * z + c;
		iter_count++;
	}
	if (iter_count == MAX_ITERATIONS) {
		return 0;
	}
	return iter_count;
}

void draw_mandelbrot_point(struct App *app, int x, int y, int r, int g, int b, unsigned int iter_count) {
	const unsigned int rgb_scale = 2;
	if (iter_count == 0) {
		draw_point(app, x, y, 0, 0, 0);
	} else {
		draw_point(app, x, y, 255 - iter_count * rgb_scale - r, 255 - iter_count * rgb_scale - g, 255 - iter_count * rgb_scale - b);
	}
}

void render_mandelbrot(struct App *app, struct Mandelbrot *mand) {
	for (unsigned int x = 0; x < WINDOW_WIDTH; x++) {
		for (unsigned int y = 0; y < WINDOW_HEIGHT; y++) {
			double x_scaled = scale_coord(mand->real_min, mand->real_max, x, WINDOW_WIDTH);
			double y_scaled = scale_coord(mand->imag_min, mand->imag_max, y, WINDOW_HEIGHT);
			unsigned int iter_count = is_in_mandelbrot_set(x_scaled, y_scaled);
			mand->iter_count_at_pixel[y][x] = iter_count;
			draw_mandelbrot_point(app, x, y, mand->r, mand->g, mand->b, iter_count);
		}
	}
}

void draw_mandelbrot(struct App *app, struct Mandelbrot *mand) {
	for (unsigned int x = 0; x < WINDOW_WIDTH; x++) {
		for (unsigned int y = 0; y < WINDOW_HEIGHT; y++) {
			unsigned int iter_count = mand->iter_count_at_pixel[y][x];
			draw_mandelbrot_point(app, x, y, mand->r, mand->g, mand->b, iter_count);
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
	struct Mandelbrot mand = {
		.iter_count_at_pixel = {0},
		.real_min = DEFAULT_REAL_MIN,
		.real_max = DEFAULT_REAL_MAX,
		.imag_min = DEFAULT_IMAG_MIN,
		.imag_max = DEFAULT_IMAG_MAX,
		.zoom_factor = 2,
		.r = rand() % 256,
		.g = rand() % 256,
		.b = rand() % 256
	};

	if (app_init(&app)) {
		fprintf(stderr, "Error initializing app! %s\n", SDL_GetError());
		app_destroy(&app);
		return 1;
	}


	SDL_RenderClear(app.renderer);
	render_mandelbrot(&app, &mand);
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
							mand.real_min = DEFAULT_REAL_MIN;
							mand.real_max = DEFAULT_REAL_MAX;
							mand.imag_min = DEFAULT_IMAG_MIN;
							mand.imag_max = DEFAULT_IMAG_MAX;
							render_mandelbrot(&app, &mand);
							break;
						case SDL_SCANCODE_C:
							mand.r = rand() % 256;
							mand.g = rand() % 256;
							mand.b = rand() % 256;
							draw_mandelbrot(&app, &mand);
							break;
						default:
							break;
					}
					break;
				case SDL_MOUSEBUTTONDOWN:
					{
						double real_scaled = scale_coord(mand.real_min, mand.real_max, event.button.x, WINDOW_WIDTH);
						double imag_scaled = scale_coord(mand.imag_min, mand.imag_max, event.button.y, WINDOW_HEIGHT);
						double real_diff = mand.real_max - mand.real_min;
						mand.real_min = real_scaled - (real_diff / (2.0 * mand.zoom_factor));
						mand.real_max = real_scaled + (real_diff / (2.0 * mand.zoom_factor));
						double imag_diff = mand.imag_max - mand.imag_min;
						mand.imag_min = imag_scaled - (imag_diff / (2.0 * mand.zoom_factor));
						mand.imag_max = imag_scaled + (imag_diff / (2.0 * mand.zoom_factor));
						render_mandelbrot(&app, &mand);
					}
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
