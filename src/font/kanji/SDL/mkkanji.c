// mkkanji for SDL and SDL_ttf
//
// Programmed by Sasaji
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if !defined(_WIN32) || defined(__MINGW32__)
#include <unistd.h>
#endif

#include "SDL.h"
#include "SDL_ttf.h"

#define BUFFER_SIZE	1024


static TTF_Font *font = NULL;
static FILE*   fl = NULL;

static char app_path[BUFFER_SIZE];
static char app_name[BUFFER_SIZE];
static char font_path[BUFFER_SIZE];
static const char *font_name;
static int  font_size;
static int  font_index;
static int  offset_x;
static int  offset_y;
static char txt_path[BUFFER_SIZE];
static char out_path[BUFFER_SIZE];
static char log_path[BUFFER_SIZE];


static void make_bitmap_font(const char *, const char *);
static void exit_program();
static int get_options(int, char *[]);
static void get_dir_and_basename(const char *, char *, char *);
static void out_log(const char *, ...);


int main(int argc, char* argv[])
{
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
	offset_x = 0;
	offset_y = 0;

	memset(txt_path, '\0', BUFFER_SIZE);
	strncpy(txt_path, app_path, BUFFER_SIZE-1);

	memset(out_path, '\0', BUFFER_SIZE);
	strncpy(out_path, app_path, BUFFER_SIZE-1);

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
	fprintf(fl, "Font index: %d  Font size: %d  Offset X: %d  Offset Y: %d\n",font_index, font_size, offset_x, offset_y);

	/* convert font */
	make_bitmap_font("kanji.txt", "KANJI.ROM");
/*	make_bitmap_font("kanji2.txt", "KANJI2.ROM"); */

	/* Shutdown all subsystems */
	exit(0);
}

static size_t length_utf8_char(const char *str, size_t len)
{
	if (len <= 0) return 3;

	// ascii
	if (str[0] >= 0) return 1; 
	if (((Uint8)str[0] & 0xf0) == 0xe0) {
		if (len >= 3) return 3;
		else return len;
	}
	if (((Uint8)str[0] & 0xf8) == 0xf0) {
		if (len >= 4) return 4;
		else return len;
	}
	if (len < 2) return len;
	return 2;
}

static void make_bitmap_font(const char *txt_name, const char *out_name)
{
	SDL_Event event;
	SDL_Surface *ttf_suf = NULL;

	SDL_Color color = {255, 255, 255};

	int x,y;
	int chr;

	char txt_file[BUFFER_SIZE];
	char out_file[BUFFER_SIZE];

	char inbuf[BUFFER_SIZE];
	char *inbufp;
	size_t inbuflen = 0;
	char outbuf[10];

	int pos;
	Uint32 c;
	Uint32 d;

	int working;

	FILE*   fs = NULL;
	FILE*   fd = NULL;

	memcpy(txt_file, txt_path, BUFFER_SIZE);
	strncat(txt_file, txt_name, BUFFER_SIZE-1);

	memcpy(out_file, out_path, BUFFER_SIZE);
	strncat(out_file, out_name, BUFFER_SIZE-1);

	fs = fopen(txt_file, "r");
	if(fs == NULL) {
		out_log("Error: %s not found.\n",txt_file);
	}

	fd = fopen(out_file, "wb");
	if(fd == NULL) {
		out_log("Error: %s write error.\n",out_file);
	}

	working = (fs != NULL && fd != NULL ? 1 : 0);
	while(working && (fgets(inbuf, BUFFER_SIZE-1, fs)) != NULL) {
		int inbufpos = 0;
		for(chr = 0; working && chr < 16; chr++) {
			inbufp = &inbuf[inbufpos];
			inbuflen = length_utf8_char(inbufp, strlen(inbufp));
			memcpy(outbuf, inbufp, inbuflen);
			outbuf[inbuflen] = 0;

			ttf_suf = TTF_RenderUTF8_Solid(font, outbuf, color);
			if (ttf_suf == NULL) {
				out_log("Error: TTF_RenderUTF8_Solid: %s.\n", TTF_GetError());
				working = 0;
				break;
			}
			fprintf(fl, "%s w:%d h:%d pitch:%d unit:%d\n",outbuf, ttf_suf->w, ttf_suf->h, ttf_suf->pitch, ttf_suf->format->BytesPerPixel);

			if (*inbufp == 0x20) {
				// undefined code (space)
				for(y = 0; y < 16; y++) {
					fprintf(fl, "               o");
					fputc(0x00, fd);
					fputc(0x01, fd);
					fprintf(fl, "\n");
				}
			} else {
				// kanji
				SDL_LockSurface(ttf_suf);
				for(y = 0; y < 16; y++) {
					d = 0;
					for(x = 0; x < 16; x++) {
						if (x + offset_x >= 0 && x + offset_x < ttf_suf->w && y + offset_y >= 0 && y + offset_y < ttf_suf->h) {
							pos = (y + offset_y) * ttf_suf->pitch + (x + offset_x) * ttf_suf->format->BytesPerPixel;
							c = *((Uint8 *)(ttf_suf->pixels) + pos);
						} else {
							c = 0;
						}
						if (c != 0 && (x != 15 || y != 0)) {
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
				SDL_UnlockSurface(ttf_suf);
			}

			inbufpos += inbuflen;
			if (inbuflen <= 1) {
				// space or ascii
				inbufpos++;
			}

			if (ttf_suf != NULL) {
				SDL_FreeSurface(ttf_suf);
				ttf_suf = NULL;
			}
		}
		while(SDL_PollEvent(&event)) {};
	}

	if (fd != NULL) fclose(fd);
	if (fs != NULL) fclose(fs);
}

static void exit_program()
{
	if (font != NULL) TTF_CloseFont(font);
	TTF_Quit();
	SDL_Quit();
	if (fl != NULL) fclose(fl);
}

static void usage()
{
	out_log("Make Kanji ROM image. SDL Version 0.1.1\n");
	out_log("Usage: %s [-h] [-f <ttf font file>] [-p <font size>] [-n <index>] [-x <offset x>] [-y <offset y>] [-o <output path>] [-i <input path>]\n",app_name);
	out_log("Options: -h : Show this usage.\n");
	out_log("         -f : Set the ttf or otf font file.\n");
	out_log("         -p : Set font size. (default:16)(range:8-24)\n");
	out_log("         -n : Set font index in ttf font file. (default:0)\n");
	out_log("         -x : Set offset x on a charactor bitmap. (default:0)(range:0-8)\n");
	out_log("         -y : Set offset y on a charactor bitmap. (default:0)(range:0-8)\n");
	out_log("         -o : Set path for output.\n");
	out_log("         -i : Set path where kanji.txt is stored in.\n");
	exit(1);
}

#if defined(_WIN32) && !defined(__MINGW32__)
static const struct st_arg_list {
	const char *key;
	char argcnt;
} arg_list[] = {
	{ "-h", 0 },
	{ "-f", 1 },
	{ "-i", 1 },
	{ "-o", 1 },
	{ "-p", 1 },
	{ "-n", 1 },
	{ "-x", 1 },
	{ "-y", 1 },
	{ NULL }
};

static int optidx = 1;
static const char *optarg = NULL;
static int getopt(int ac, char *av[], const char *dummy)
{
	int argopt = -1;
	int argcnt = 0;
	int i;
	const char *arg;

	while(optidx<ac) {
		int match = -1;
		for(i=0; arg_list[i].key; i++) {
			arg = av[optidx];
			if (stricmp(arg, arg_list[i].key) == 0) {
				match = i;
				break;
			}
		}
		optidx++;
		if (match < 0) {
			if (argcnt) {
				optarg = arg;
			} else {
				argopt = -1;
			}
			break;
		}
		argopt = arg_list[match].key[1];
		argcnt = arg_list[match].argcnt; 
		if (argcnt == 0) {
			break;
		}
	}
	return argopt;
}
#endif

static int get_options(int ac, char *av[])
{
	int opt = 0;
	long val;
	char *err;

	while ((opt = getopt(ac, av, "hf:p:i:o:n:x:y:")) != -1) {
		switch (opt) {
		case 'f':
			memset(font_path, '\0', BUFFER_SIZE);
			strncpy(font_path, optarg, BUFFER_SIZE-1);
			break;
		case 'p':
			err = NULL;
			val = strtol(optarg, &err, 10);
			if ((err && *err > 0x20) || val < 8 || val > 24) {
				usage();
			}
			font_size = (int)val;
			break;
		case 'n':
			err = NULL;
			val = strtol(optarg, &err, 10);
			if ((err && *err > 0x20) || val < 0) {
				usage();
			}
			font_index = (int)val;
			break;
		case 'x':
			err = NULL;
			val = strtol(optarg, &err, 10);
			if ((err && *err > 0x20) || val < 0 || val > 8) {
				usage();
			}
			offset_x = (int)val;
			break;
		case 'y':
			err = NULL;
			val = strtol(optarg, &err, 10);
			if ((err && *err > 0x20) || val < 0 || val > 8) {
				usage();
			}
			offset_y = (int)val;
			break;
		case 'o':
			memset(out_path, '\0', BUFFER_SIZE);
			strncpy(out_path, optarg, BUFFER_SIZE-1);
			break;
		case 'i':
			memset(txt_path, '\0', BUFFER_SIZE);
			strncpy(txt_path, optarg, BUFFER_SIZE-1);
			break;
		case 'h':
			usage();
			break;
		default:
			break;
		}
	}
	return 1;
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
