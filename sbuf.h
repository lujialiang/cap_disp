#ifndef __S_BUF_H_INCLUDE
#define __S_BUF_H_INCLUDE

#include <stdint.h>
#include <stdbool.h>
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>

typedef struct {
	void **buf;
	int n;
	int front;
	int rear;
	SDL_sem *mutex;
	SDL_sem *slots;
	SDL_sem *items;
} sbuf_t;

typedef struct {
	uint8_t buffer[1024*512];
	uint32_t size;
}
image_buffer_t;

void sbuf_init(sbuf_t *, int);

void sbuf_deinit(sbuf_t *);
	
void sbuf_insert(sbuf_t *, void *);

void *sbuf_remove(sbuf_t *);

#define P(s)	SDL_SemWait(s)
#define V(s)	SDL_SemPost(s)

#endif // __S_BUF_H_INCLUDE
