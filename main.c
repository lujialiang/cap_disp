
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include <SDL/SDL_image.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdbool.h>

#include "sbuf.h"
#include "debug.h"

SDL_Surface *image = NULL;

#define SCREEN_X_MAX (800)
#define SCREEN_Y_MAX (800)

uint32_t window_width =  SCREEN_X_MAX;
uint32_t window_heigh =  SCREEN_Y_MAX;

int deal(uint8_t);

sbuf_t my_sbuf;

volatile bool quit = false;
SDL_Event event;

void handle_sdl_events(void);

SDL_Thread *thread0 = NULL;
SDL_Thread *thread1 = NULL;
SDL_Thread *thread2 = NULL;

SDL_sem *MouseSem = NULL;

//保护性互斥锁
//SDL_mutex *bufferLock = NULL;
//条件变量
//SDL_cond *canProduce = NULL;
//SDL_cond *canConsume = NULL;

char *buffer = NULL;

int mouse_x = 0, mouse_y = 0;
int mouse_motion = 0;

typedef enum {
	sm_head,
	sm_data,
	sm_write
} sm_t;

sm_t sm = sm_head;
uint8_t file_buffer[1024*1024*4];
uint8_t *file_buffer_ptr;

int tcp_client(void *par);
int blit_image(uint8_t *buffer,uint32_t filesize);

//void ShowPic(unsigned char *buf, int size, SDL_Surface *screen, int x, int y)    
void ShowPic(unsigned char *buf, int size, int x, int y)    
{    
    SDL_RWops *src;    
    SDL_Surface *image;    
    SDL_Rect dest;    
    
    src = SDL_RWFromMem(buf, size);    
    
    /* 将BMP文件加载到一个surface*/    
    image = IMG_Load_RW(src, 1);    
    if ( image == NULL )    
    {    
        fprintf(stderr, "无法加载 %s\n", SDL_GetError());    
        return;    
    }    
    
    /* Blit到屏幕surface。onto the screen surface.  
 *        这时不能锁住surface。  
 *             */    
    dest.x = x;    
    dest.y = y;    
    dest.w = image->w;    
    dest.h = image->h;    
	static uint32_t freme_cnt = 0;
	freme_cnt ++;
	printf("%d image width = %d,\t image height = %d\n", freme_cnt, image->w, image->h);
	SDL_Surface * screen = SDL_GetVideoSurface();
	if( image->w != window_width ) {
		SDL_Rect rect;
		rect.x = 0;
		rect.y = 0;
		rect.w = SCREEN_X_MAX;
		rect.h = SCREEN_Y_MAX;
	    SDL_FillRect(screen,&rect,0xffff00);
		SDL_Flip(screen);
		window_width = image->w;
	}
    SDL_BlitSurface(image, NULL, screen, &dest);    
    
    /* 刷新屏幕的变化部分 */    
    SDL_UpdateRects(screen, 1, &dest);    
}    
    
SDL_Surface *load_image( char *filename ) 
{
	//加载的图像
	SDL_Surface* loadedImage = NULL;
	
	//优化后的图像
	SDL_Surface* optimizedImage = NULL;
	
	//使用SDL_image加载图像
	loadedImage = IMG_Load( filename );
	
	//如果图像加载成功
	if( loadedImage != NULL )
	{
		//创建一个优化后的图像
		optimizedImage = SDL_DisplayFormat( loadedImage );
		
		//释放原先加载的图像
		SDL_FreeSurface( loadedImage );
	}
	
	//返回优化后的图像
	return optimizedImage;
}

void apply_surface( int x, int y, SDL_Surface* source, SDL_Surface* destination )
{
    //Rectangle to hold the offsets
    SDL_Rect offset;

    //Get offsets
    offset.x = x;
    offset.y = y;

    //Blit the surface
    SDL_BlitSurface( source, NULL, destination, &offset );
}

 
/*
void show_surface(void)
{
	//Load the image
	image = load_image( "tmp.jpg" );
	//If there was a problem in loading the image
	if( image == NULL ) {
		return;
	}
	else {
		//Apply the surface to the screen
		//apply_surface( 0, 0, image, screen );
		apply_surface( 0, 0, image, screen );
		SDL_Flip( screen );
	}
}
*/

#if 0
void write_file(void)
{
	static int cnt = 0;
	FILE *fp = fopen("tmp.jpg","wb");
	if( fp ) {
		//cnt++;
		//printf("cnt = %d\n", cnt);
		fwrite( file_buffer, (file_buffer_ptr - file_buffer), 1, fp);
		fclose(fp);
	}
	else {
		perror("tmp.jpg");
	}
}
#endif

int gui_thread(void *par)
{
    while(!quit) {
		image_buffer_t *image_buffer;
		int i = 0;
		image_buffer = sbuf_remove(&my_sbuf);
		for(i=0;i<image_buffer->size;i++) {
			deal(image_buffer->buffer[i]);
		}
		free(image_buffer);
    }
    return 0;
}

int mouse_thread0(void *par);

int mouse_thread(void *par)
{
    while(!quit) {
		mouse_thread0(par);
    }
    return 0;
}

int mouse_thread0(void *par)
{
    int sock;
    struct sockaddr_in server;
     
	SDL_Delay(500);
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 9003 );
 
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
		while(!quit) {
			SDL_Delay(10000);
		}
		return -1;
    }
    printf("Connected to port %d\n",9003);
     
	char buffer[1024];
	//uint32_t read_size = recv(sock , buffer , 1024 , 0);
	//printf("%s\n", buffer);
    while(!quit) {
		SDL_SemWait(MouseSem);
		if( mouse_x > 0 && mouse_y > 0 ) {
			if( mouse_motion ) {
				sprintf( buffer, "m 0 %d %d 50\nc\n", mouse_x, mouse_y );
			}
			else {
				sprintf( buffer, "d 0 %d %d 50\nc\n", mouse_x, mouse_y );
			}
			if( send(sock , buffer , strlen(buffer), 0) < 0) {
				close(sock);
				my_msg("send error1");
				return -1;
			}
		}
		else {
			sprintf( buffer, "u 0\nc\n");
			if( send(sock , buffer , strlen(buffer), 0) < 0) {
				my_msg("send error2");
				close(sock);
				return -1;
			}
		}
		printf("x = %d, y = %d\n", mouse_x, mouse_y);
    }
	close( sock );
    return 0;
}

int tcp_thread(void *par)
{
    while(!quit) {
		tcp_client(par);
		SDL_Delay(2*1000);
    }
    return 0;
}

int main( int argc, char** argv )
{
	quit = false;
    //screen_init();
	SDL_Init( SDL_INIT_VIDEO );
	SDL_SetVideoMode( SCREEN_X_MAX, SCREEN_Y_MAX, 0, SDL_ANYFORMAT );
	{
		SDL_Rect rect;
		rect.x = 0;
		rect.y = 0;
		rect.w = SCREEN_X_MAX;
		rect.h = SCREEN_Y_MAX;
		SDL_Surface * screen = SDL_GetVideoSurface();
		SDL_FillRect(screen,&rect,0xffff00);
		SDL_Flip(screen);
	}

	sbuf_init( &my_sbuf, 10);
	
	MouseSem = SDL_CreateSemaphore( 0 );
	//创建互斥锁
	//bufferLock = SDL_CreateMutex();
	//创建条件变量
	//canProduce = SDL_CreateCond();
	//canConsume = SDL_CreateCond();

	thread1 = SDL_CreateThread( tcp_thread, NULL );
	thread0 = SDL_CreateThread( gui_thread, NULL );
	thread2 = SDL_CreateThread( mouse_thread, NULL );

    //While the user hasn't quit
    while( quit == false )
    {
        //While there's an event to handle
        while( SDL_PollEvent( &event ) )
        {
            //If the user has Xed out the window
            if( event.type == SDL_QUIT )
            {
                //Quit the program
                quit = true;
            }
            handle_sdl_events();
        }
    }

	SDL_KillThread( thread0 );
	SDL_KillThread( thread1 );
	SDL_KillThread( thread2 );
	SDL_Quit();
	sbuf_deinit(&my_sbuf);
    return 0;
}

void SendEvent(int x, int y)
{			
	mouse_x = x;
	mouse_y = y;
	SDL_SemPost(MouseSem);
	SDL_Delay(10);
}

void handle_sdl_events(void)
{
	static int key_pressed = 0;
    //If the mouse moved
    if( event.type == SDL_MOUSEMOTION )
    {
		if( key_pressed ) {
			mouse_motion = 1;
			SendEvent(event.button.x*2, event.button.y*2);
		}
		else {			
			mouse_motion = 0;
			//SendEvent(-1,-1);
		}
    }
    //If a mouse button was pressed
    if( event.type == SDL_MOUSEBUTTONDOWN )
    {
		mouse_motion = 0;
        //If the left mouse button was pressed
        if( event.button.button == SDL_BUTTON_LEFT )
        {
			key_pressed = 1;
			SendEvent(event.button.x*2, event.button.y*2);
        }
    }
    //If a mouse button was released
    if( event.type == SDL_MOUSEBUTTONUP )
    {
		mouse_motion = 0;
        //If the left mouse button was released
        if( event.button.button == SDL_BUTTON_LEFT )
        {
			key_pressed = 0;
			SendEvent(-1,-1);
        }
    }
}

uint32_t skip_cnt = 0;
static uint8_t image_info[5];

int deal(uint8_t data)
{
	static uint8_t pre_data = 0;
	switch( sm ) {
		case sm_head:
			file_buffer_ptr = file_buffer;
			if( data == 0xd8 && pre_data == 0xff ) {
				file_buffer[0] = 0xff;
				file_buffer[1] = 0xd8;
				file_buffer_ptr += 2;
				sm = sm_data;
				uint32_t image_size = (image_info[3] | image_info[2]<<8 | image_info[1] << 16 | image_info[0] << 24);
				printf("expect image size %d, rotation = %d\n", image_size, image_info[4]);
			}
			else {
				image_info[4] = image_info[3];
				image_info[3] = image_info[2];
				image_info[2] = image_info[1];
				image_info[1] = image_info[0];
				image_info[0] = pre_data;
				//my_msg("skip = %d %02x\n", ++skip_cnt, pre_data);
			}
			pre_data = data;
			return 0;
		case sm_data:
			if( file_buffer_ptr - file_buffer >= sizeof(file_buffer) ) {
				my_msg("error image too larget\n");
			}
			else {
				*file_buffer_ptr++ = data;
			}
			if( data == 0xd9 && pre_data == 0xff ) {
				sm = sm_write;
			}
			pre_data = data;
			return 0;
		case sm_write:
			//ShowPic( file_buffer, (file_buffer_ptr-file_buffer), screen, 0, 0);
			ShowPic( file_buffer, (file_buffer_ptr-file_buffer),  0, 0);
			printf("real recv size = %d\n", (file_buffer_ptr-file_buffer));
			pre_data = data;
			sm = sm_head;
			return 1;
		default:
			pre_data = 0;
			sm = sm_head;
			return -1;
	}
	return 0;
}

int tcp_client(void *par)
{
    int sock;
    struct sockaddr_in server;
     
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 9002 );
 
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
     
    printf("Connected to port %d\n", 9002);
     
	uint8_t message[1024*100];
	int read_size;
	uint32_t total_recv_size = 0;
    while(1)
    {
		image_buffer_t *image_buffer = malloc(sizeof(image_buffer_t));
		if( image_buffer == NULL ) {
			printf("here\n");
			continue;
		}
		read_size = recv(sock , image_buffer->buffer, 1024*128 , 0);
		if( read_size > 0 ) {
		#if 0
			int i = 0;
			for(i=0;i<read_size;i++) {
				deal(image_buffer->buffer[i]);
			}
		#else
			image_buffer->size = read_size;
			total_recv_size += read_size;
			sbuf_insert( &my_sbuf, image_buffer );
		#endif
		}
		else {
			free(image_buffer);
			goto out;
		}
    }
out:
	printf("\n total_recv_size = %d\n", total_recv_size);
    close(sock);
    return 0;
}

