// mkkanji for SDL and SDL_ttf
//
// Programmed by Sasaji
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iconv.h>

#include "SDL.h"
#include "SDL_ttf.h"

#define BUFFER_SIZE	1024


static TTF_Font *font = NULL;
static FILE*   fs = NULL;
static FILE*   fd = NULL;
static FILE*   fl = NULL;
static iconv_t ic = -1;

static char app_path[BUFFER_SIZE];
static char app_name[BUFFER_SIZE];
static char font_path[BUFFER_SIZE];
static char *font_name;
static int  font_size;
static int  font_index;
static char txt_path[BUFFER_SIZE];
static char out_path[BUFFER_SIZE];
static char log_path[BUFFER_SIZE];


static void exit_program();
static int get_options(int, char *[]);
static void get_dir_and_basename(const char *, char *, char *);
static void out_log(const char *, ...);


int main(int argc, char* argv[]) {
	SDL_Surface *ttf_suf = NULL;
	SDL_Event event;

	SDL_Color color = {255, 255, 255};

	int x,y;
	int chr;

	char inbuf[BUFFER_SIZE];
	char *inbufp;
	size_t inbuflen = 0;
	char outbuf[10];
	char *outbufp;
	size_t outbuflen = 0;

	Uint32 c;
	Uint32 d;

	/* set application name */
	get_dir_and_basename(argv[0], app_path, app_name);

	/* open log */
	memset(log_path, '\0', BUFFER_SIZE);
	strncpy(log_path, app_path, BUFFER_SIZE-1);
	strncat(log_path, "result.txt", BUFFER_SIZE-1);
	fl = fopen(log_path, "w");
	if(fl == NULL) {
		out_log("Error: %s write error.\n", log_path);
		exit(1);
	}

	/* default setting */
	memset(font_path, '\0', BUFFER_SIZE);
	strncpy(font_path, app_path, BUFFER_SIZE-1);
	strncat(font_path, "ipag.ttf", BUFFER_SIZE-1);
	font_size = 16;
	font_index = 0;

	memset(txt_path, '\0', BUFFER_SIZE);
	strncpy(txt_path, app_path, BUFFER_SIZE-1);
	strncat(txt_path, "kanji.txt", BUFFER_SIZE-1);

	memset(out_path, '\0', BUFFER_SIZE);
	strncpy(out_path, app_path, BUFFER_SIZE-1);
	strncat(out_path, "KANJI.ROM", BUFFER_SIZE-1);

	/* get options */
	get_options(argc, argv);

	/* Init SDL */
	if(SDL_Init(SDL_INIT_VIDEO) == -1) {
		out_log("Error: SDL_Init: %s.\n", SDL_GetError());
		fclose(fl);
		exit(1);
	}
	/* Init TTF */
	if(TTF_Init() == -1) {
		SDL_Quit();
		out_log("Error: TTF_Init: %s.\n", TTF_GetError());
		fclose(fl);
		exit(1);
	}
	/* Clean up on exit */
	atexit(exit_program);

	/* set font */
	font = TTF_OpenFontIndex(font_path, font_size, font_index);
	if (font == NULL) {
		out_log("Error: TTF_OpenFont: %s.\n", TTF_GetError());
		exit(1);
	}
	font_name = TTF_FontFaceFamilyName(font);
	if (font_name != NULL) {
		fprintf(fl, "Using font: %s(%s).\n", font_name, font_path);
	} else {
		fprintf(fl, "Using font: unknown(%s).\n", font_path);
	}

	// convert font
	fs = fopen(txt_path, "r");
	if(fs == NULL) {
		out_log("Error: %s not found.\n",txt_path);
		exit(1);
	}

	ic = iconv_open("UTF-8", "CP932");
	if (ic == (iconv_t)-1) {
		out_log("Error: iconv_open failed.\n");
		exit(1);
	}

	fd = fopen(out_path, "wb");
	if(fd == NULL) {
		out_log("Error: %s write error.\n",out_path);
		exit(1);
	}
	while((fgets(inbuf, BUFFER_SIZE-1, fs)) != NULL) {
		for(chr = 0; chr < 16; chr++) {
			inbufp = &inbuf[chr * 2];
			inbuflen = 2;
			memset(outbuf, 0, sizeof(outbuf));
			outbufp = outbuf;
			outbuflen = 3;

			if (iconv(ic, &inbufp, &inbuflen, &outbufp, &outbuflen) == (size_t)-1) {
				out_log("Error: iconv failed.\n");
				exit(1);
			}

			ttf_suf = TTF_RenderUTF8_Solid(font, outbuf, color);
			if (ttf_suf == NULL) {
				out_log("Error: TTF_RenderUTF8_Solid: %s.\n", TTF_GetError());
				exit(1);
			}

			fprintf(fl, "%s w:%d h:%d\n",outbuf, ttf_suf->w, ttf_suf->h);

			SDL_LockSurface(ttf_suf);
			if (inbuf[chr * 2] == 0x20) {
				// undefined code (space)
				for(y = 0; y < 16; y++) {
					fprintf(fl, "               o");
					fputc(0x00, fd);
					fputc(0x01, fd);
					fprintf(fl, "\n");
				}
			} else {
				// kanji
				for(y = 0; y < 16; y++) {
					d = 0;
					for(x = 0; x < 16; x++) {
						c = *((Uint8 *)(ttf_suf->pixels) + y * 16 + x);
						if (c != 0 && x < 15) {
							d |= (0x8000 >> x);
							fprintf(fl, "o");
						} else {
							fprintf(fl, " ");
						}
					}
					fputc(((d >> 8) & 0xff), fd);
					fputc((d & 0xff), fd);
					fprintf(fl, "\n");
				}
			}
			SDL_UnlockSurface(ttf_suf);

			if (ttf_suf != NULL) {
				SDL_FreeSurface(ttf_suf);
				ttf_suf = NULL;
			}
		}
		while(SDL_PollEvent(&event)) {};
	}

	/* Shutdown all subsystems */
	exit(0);
}

static void exit_program() {
	if (fd != NULL) fclose(fd);
	if (ic != (iconv_t)-1) iconv_close(ic);
	if (fs != NULL) fclose(fs);
	if (font != NULL) TTF_CloseFont(font);
	TTF_Quit();
	SDL_Quit();
	if (fl != NULL) fclose(fl);
}

static int get_options(int ac, char *av[])
{
	int opt = 0;

#if !defined(_WIN32) || defined(__MINGW32__)
	while ((opt = getopt(ac, av, "hf:p:i:o:n:")) != -1) {
		switch (opt) {
		case 'f':
			memset(font_path, '\0', BUFFER_SIZE);
			strncpy(font_path, optarg, BUFFER_SIZE-1);
			break;
		case 'p':
			font_size = atoi(optarg);
			break;
		case 'i':
			font_index = atoi(optarg);
			break;
		case 'o':
			memset(out_path, '\0', BUFFER_SIZE);
			strncpy(out_path, optarg, BUFFER_SIZE-1);
			break;
		case 'n':
			memset(txt_path, '\0', BUFFER_SIZE);
			strncpy(txt_path, optarg, BUFFER_SIZE-1);
			break;
		case 'h':
			out_log("Usage: %s [-h] [-f ttf font file] [-p ptsize] [-i index] [-o outfile] [-n infile]\n",app_name);
			exit(1);
			break;
		default:
			break;
		}
	}
#endif
	return 0;
}

static void get_dir_and_basename(const char *path, char *dir, char *name)
{
	char *p = NULL;

	memset(dir, '\0', BUFFER_SIZE);
	if (name != NULL) memset(name, '\0', BUFFER_SIZE);
	strncpy(dir, path, BUFFER_SIZE-1);

#ifdef _WIN32
	p = strrchr(dir, '\\');
	if (p == NULL) {
#endif
		p = strrchr(dir, '/');
#ifdef _WIN32
	}
#endif
	if (p != NULL) {
		if (name != NULL) strncpy(name, p+1, BUFFER_SIZE-1);
		*(p+1) = '\0';
	} else {
		if (name != NULL) strncpy(name, dir, BUFFER_SIZE-1);
		*dir = '\0';
	}
	return;
}

void out_log(const char *format, ...)
{
	char buffer[BUFFER_SIZE];
	va_list ap;

	va_start(ap, format);

	vsprintf(buffer, format, ap);

	fprintf(stderr, "%s", buffer);

	if(fl != NULL) {
		fprintf(fl, "%s", buffer);
		fflush(fl);
	}

	va_end(ap);

	return;
}
