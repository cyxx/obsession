
SDL_CFLAGS := `sdl2-config --cflags`
SDL_LIBS   := `sdl2-config --libs`

CPPFLAGS += -MMD -Wall -g $(SDL_CFLAGS)

OBJS = ball.o game.o input.o led.o lights.o main.o missions.o mixer.o music.o physmap.o render.o resource.o score.o state.o unpack.o
DEPS = $(OBJS:.o=.d)

aqua: $(OBJS)
	$(CC) -o $@ $^ $(SDL_LIBS) -lm

clean:
	rm -f *.o *.d

-include $(DEPS)
