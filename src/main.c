
#include <SDL.h>
#ifndef __EMSCRIPTEN__
#include <getopt.h>
#include <sys/stat.h>
#else
#include <emscripten.h>
#endif
#include "host.h"
#include "mixer.h"
#include "render.h"
#include "resource.h"
#include "game.h"

struct Host g_host;

static const char NAME[] = "Pinball Obsession - Aquatic Adventure";

#ifndef __EMSCRIPTEN__
static const char USAGE[] =
	"%s\n"
	"Usage: %s [OPTIONS]...\n"
	"  --filepath=PATH   Path to .BIN file (default is 'AQUA.BIN')\n"
	"  --scrolling       Enable vertical scrolling\n"
	"  --fullscreen      Stretch window resolution to desktop\n"
;
#endif

static SDL_Window *g_window;
static SDL_Renderer *_renderer;
static SDL_PixelFormat *_fmt;
static ResourcesData g_data;
static int g_window_w;
static int g_window_h;
static bool g_scroll;

static SDL_Texture *_textures[DRAW_SURFACE_COUNT];
static int _texturesCount;

static SDL_Texture *GetTexture(int handle) {
	return _textures[handle];
}

static int CreateSurface565(int w, int h) {
	assert(_texturesCount < DRAW_SURFACE_COUNT);
	int handle;
	SDL_Texture *t = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, w, h);
	handle = _texturesCount;
	assert(_texturesCount < DRAW_SURFACE_COUNT);
	_textures[_texturesCount++] = t;
	return handle;
}

#ifndef __EMSCRIPTEN__
static int DestroySurface565(int handle) {
	SDL_DestroyTexture(_textures[handle]);
	_textures[handle] = 0;
	return 0;
}
#endif

static int LockSurface565(int handle, uint16_t **p, int *pitch) {
	SDL_Texture *t = _textures[handle];
	SDL_LockTexture(t, 0, (void **)p, pitch);
	*pitch /= sizeof(uint16_t);
	return 0;
}

static int UnlockSurface565(int handle) {
	SDL_UnlockTexture(_textures[handle]);
	return 0;
}

static uint16_t MapRGB565(int handle, uint16_t rgb444) {
	uint8_t r = (rgb444 >> 8) & 15;
	uint8_t g = (rgb444 >> 4) & 15;
	uint8_t b =  rgb444       & 15;
	r |= r << 4;
	g |= g << 4;
	b |= b << 4;
	return SDL_MapRGB(_fmt, r, g, b);
}

static void AudioSamplesCb(void *userdata, uint8_t *data, int len) {
	assert((len & 3) == 0);
	Mixer_MixStereoS16((int16_t *)data, len / 4);
}

static int LockAudio(int flag) {
	if (flag) {
		SDL_LockAudio();
	} else {
		SDL_UnlockAudio();
	}
	return 1;
}

static void InitAudio() {
	SDL_AudioSpec desired;
	memset(&desired, 0, sizeof(desired));
	desired.freq = AUDIO_SAMPLE_RATE;
	desired.format = AUDIO_S16;
	desired.channels = 2;
	desired.samples = 2048;
	desired.callback = AudioSamplesCb;
	desired.userdata = 0;
	if (SDL_OpenAudio(&desired, 0) == 0) {
		SDL_PauseAudio(0);
	}
}

static void FiniAudio() {
	Mixer_Fini();
	SDL_CloseAudio();
}

#ifdef __EMSCRIPTEN__
static bool g_audio_started;

static void EnsureAudio() {
	if (!g_audio_started) {
		g_audio_started = true;
		InitAudio();
	}
}
#endif

static void Debug(const char *fmt, ...) {
	char buf[1024];
	va_list va;
	va_start(va, fmt);
	vsprintf(buf, fmt, va);
	va_end(va);
	fprintf(stdout, "%s\n", buf);
}

static void Warn(const char *fmt, ...) {
	char buf[1024];
	va_list va;
	va_start(va, fmt);
	vsprintf(buf, fmt, va);
	va_end(va);
	fprintf(stdout, "WARNING: %s\n", buf);
}

static void Error(const char *fmt, ...) {
	char buf[1024];
	va_list va;
	va_start(va, fmt);
	vsprintf(buf, fmt, va);
	va_end(va);
	fprintf(stdout, "ERROR: %s\n", buf);
}

static void HandleEvents() {
	SDL_Event ev;
	while (SDL_PollEvent(&ev)) {
		switch (ev.type) {
		case SDL_QUIT:
			g_host.quit = 1;
			break;
		case SDL_KEYDOWN:
#ifdef __EMSCRIPTEN__
			EnsureAudio();
#endif
			switch (ev.key.keysym.sym) {
			case SDLK_j:
				g_vars.keys[AMIGA_KEY_J] = 1;
				break;
			case SDLK_m:
				g_vars.keys[AMIGA_KEY_M] = 1;
				break;
			case SDLK_n:
				g_vars.keys[AMIGA_KEY_N] = 1;
				break;
			case SDLK_s:
				g_vars.keys[AMIGA_KEY_S] = 1;
				break;
			case SDLK_y:
				g_vars.keys[AMIGA_KEY_Y] = 1;
				break;
			case SDLK_LSHIFT:
				g_vars.keys[AMIGA_KEY_LSHIFT] = 1;
				break;
			case SDLK_RSHIFT:
				g_vars.keys[AMIGA_KEY_RSHIFT] = 1;
				break;
			case SDLK_DOWN:
				g_vars.keys[AMIGA_KEY_DOWN] = 1;
				break;
			case SDLK_SPACE:
				g_vars.keys[AMIGA_KEY_SPACE] = 1;
				break;
			case SDLK_ESCAPE:
				g_vars.keys[AMIGA_KEY_ESCAPE] = 1;
				break;
			}
			break;
		case SDL_KEYUP:
			switch (ev.key.keysym.sym) {
			case SDLK_j:
				g_vars.keys[AMIGA_KEY_J] = 0;
				g_vars.last_key_code = AMIGA_KEY_J;
				break;
			case SDLK_m:
				g_vars.keys[AMIGA_KEY_M] = 0;
				g_vars.last_key_code = AMIGA_KEY_M;
				break;
			case SDLK_n:
				g_vars.keys[AMIGA_KEY_N] = 0;
				g_vars.last_key_code = AMIGA_KEY_N;
				break;
			case SDLK_s:
				g_vars.keys[AMIGA_KEY_S] = 0;
				g_vars.last_key_code = AMIGA_KEY_S;
				break;
			case SDLK_y:
				g_vars.keys[AMIGA_KEY_Y] = 0;
				g_vars.last_key_code = AMIGA_KEY_Y;
				break;
			case SDLK_LSHIFT:
				g_vars.keys[AMIGA_KEY_LSHIFT] = 0;
				g_vars.last_key_code = AMIGA_KEY_LSHIFT;
				break;
			case SDLK_RSHIFT:
				g_vars.keys[AMIGA_KEY_RSHIFT] = 0;
				g_vars.last_key_code = AMIGA_KEY_RSHIFT;
				break;
			case SDLK_DOWN:
				g_vars.keys[AMIGA_KEY_DOWN] = 0;
				g_vars.last_key_code = AMIGA_KEY_DOWN;
				break;
			case SDLK_SPACE:
				g_vars.keys[AMIGA_KEY_SPACE] = 0;
				g_vars.last_key_code = AMIGA_KEY_SPACE;
				break;
			case SDLK_ESCAPE:
				g_vars.keys[AMIGA_KEY_ESCAPE] = 0;
				g_vars.last_key_code = AMIGA_KEY_ESCAPE;
				break;
			}
			break;
		}
	}
}

static void main_loop(void *arg) {
	int *previous_state = (int *)arg;
	HandleEvents();
	table_update();
	g_vars.last_key_code = -1;
	if (g_host.quit) {
		return;
	}
	const int current_state = table_get_state();
	if (current_state != *previous_state) {
		fprintf(stdout, "State change %d -> %d\n", *previous_state, current_state);
		*previous_state = current_state;
	}
	sound_update();
	Draw(&g_data);
	SDL_RenderClear(_renderer);
	SDL_Rect src;
	src.x = 0;
	src.y = 0;
	src.w = TABLE_W;
	SDL_Rect dst;
	dst.x = 0;
	dst.y = 0;
	int screen_w, screen_h;
	SDL_RenderGetLogicalSize(_renderer, &screen_w, &screen_h);
	dst.w = TABLE_W * screen_w / g_window_w;
	dst.h = TABLE_H * screen_h / g_window_h;
	if (g_scroll) {
		src.y = g_vars.vscroll;
		src.h = SCROLL_H;
		dst.h = SCROLL_H * screen_h / g_window_h;
	} else {
		src.h = TABLE_H;
	}
	SDL_RenderCopy(_renderer, GetTexture(DRAW_SURFACE_TABLE), &src, &dst);
	dst.x = 0;
	dst.y = src.h * screen_h / g_window_h;
	dst.w = LED_W * screen_w / g_window_w;
	dst.h = LED_H * screen_h / g_window_h;
	SDL_RenderCopy(_renderer, GetTexture(DRAW_SURFACE_LED), 0, &dst);
	SDL_RenderPresent(_renderer);
}

int main(int argc, char *argv[]) {
	const char *filePath = "AQUA.BIN";
	bool scrolling = false;
	bool fullscreen = false;
	(void)argc;
	(void)argv;
	(void)fullscreen;

#ifdef __EMSCRIPTEN__
	filePath = "/AQUA.BIN";
#else
	if (argc == 2) {
		struct stat st;
		if (stat(argv[1], &st) == 0 && S_ISREG(st.st_mode)) {
			filePath = strdup(argv[1]);
		}
	}
	while (1) {
		static struct option options[] = {
			{ "filepath",   required_argument, 0, 1 },
			{ "scrolling",  no_argument,       0, 2 },
			{ "fullscreen", no_argument,       0, 3 },
			{ 0, 0, 0, 0 }
		};
		int index;
		const int c = getopt_long(argc, argv, "", options, &index);
		if (c == -1) {
			break;
		}
		switch (c) {
		case 1:
			filePath = strdup(optarg);
			break;
		case 2:
			scrolling = true;
			break;
		case 3:
			fullscreen = true;
			break;
		default:
			printf(USAGE, NAME, argv[0]);
			return 0;
		}
	}
#endif

	g_host.Debug = Debug;
	g_host.Warn = Warn;
	g_host.Error = Error;

	FILE *fp = fopen(filePath, "rb");
	if (fp) {
		LoadResources(fp, "aqua", &g_data);
		fclose(fp);
	} else {
		fprintf(stderr, "Unable to open '%s'\n", filePath);
		return -1;
	}

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	g_host.LockSurface565 = LockSurface565;
	g_host.UnlockSurface565 = UnlockSurface565;
	g_host.MapRGB565 = MapRGB565;
	g_host.InitAudio = InitAudio;
	g_host.FiniAudio = FiniAudio;
	g_host.LockAudio = LockAudio;

#ifdef __EMSCRIPTEN__
	SDL_DisplayMode dm;
	(void)dm;
#else
	SDL_DisplayMode dm;
	if (SDL_GetDesktopDisplayMode(0, &dm) == 0) {
		fprintf(stdout, "Desktop %d x %d at %d hz ", dm.w, dm.h, dm.refresh_rate);
		const int game_w = TABLE_W;
		const int game_h = TABLE_H + LED_H;
		const float game_ar = game_w / (float)game_h;
		const float host_ar = dm.w   / (float)dm.h;
		fprintf(stdout, "aspect ratio: %f %f ", game_ar, host_ar);
		const int scale_w = game_w * dm.h / game_h;
		const int scale_h = game_h * dm.w / game_w;
		fprintf(stdout, "scale: w %d h %d\n", scale_w, scale_h);

		static const float kAspectRatio = 4.f / 3;
		const float currentAspectRatio = host_ar;

		if (currentAspectRatio > kAspectRatio) {

		}

		if (currentAspectRatio < kAspectRatio) {

		}
	}
#endif
	const int table_w = TABLE_W;
	const int table_h = TABLE_H;
	const int led_w = LED_W;
	const int led_h = LED_H;
	const int window_w = table_w;
	const int window_h = (scrolling ? SCROLL_H : TABLE_H) + LED_H;
#ifdef __EMSCRIPTEN__
	const int h = window_h;
	const int w = window_w;
#else
	const int h = !fullscreen ? window_h : (dm.h - 100);
	const int w = !fullscreen ? window_w : (window_w * h / window_h);
#endif
	const int flags = SDL_WINDOW_RESIZABLE;
	g_window_w = window_w;
	g_window_h = window_h;
	g_scroll = scrolling;
	g_window = SDL_CreateWindow(NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, flags);

	_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);
	SDL_RenderSetLogicalSize(_renderer, w, h);
	_fmt = SDL_AllocFormat(SDL_PIXELFORMAT_RGB565);

	CreateSurface565(table_w, table_h);
	CreateSurface565(led_w, led_h);

	g_scroll = scrolling;
#ifndef __EMSCRIPTEN__
	InitAudio();
#endif

	InitPalette(&g_data, DRAW_SURFACE_TABLE);

	int previous_state = -1;

	g_vars.res = &g_data;
	table_init();

	Mixer_Init(AUDIO_SAMPLE_RATE, LockAudio);
	sound_init();

#ifndef __EMSCRIPTEN__
	static const int interval = 1000 / GAME_HZ;
#endif

	g_host.quit = 0;

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop_arg(main_loop, &previous_state, 0, 1);
	return 0;
#else
	while (!g_host.quit) {
		const int next = SDL_GetTicks() + interval;
		main_loop(&previous_state);
		const int delay = next - SDL_GetTicks();
		if (delay < 10) {

		}
		SDL_Delay(delay < 15 ? 15 : delay);
	}
	FiniAudio();
	DestroySurface565(DRAW_SURFACE_TABLE);
	DestroySurface565(DRAW_SURFACE_LED);
	SDL_DestroyWindow(g_window);
	SDL_Quit();
	return 0;
#endif
}
