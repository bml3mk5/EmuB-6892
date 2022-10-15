/** @file disk_parser.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.09.01

	@note Original author is Takeda.Toshiya

	@brief [ disk parser ]
*/

#ifndef DISK_PARSER_H
#define DISK_PARSER_H

#include "../common.h"
#include "../d88_defs.h"
#include <stdlib.h>


// teledisk decoder constant
#define STRING_BUFFER_SIZE	4096
#define LOOKAHEAD_BUFFER_SIZE	60
#define THRESHOLD		2
#define N_CHAR			(256 - THRESHOLD + LOOKAHEAD_BUFFER_SIZE)
#define TABLE_SIZE		(N_CHAR * 2 - 1)
#define ROOT_POSITION		(TABLE_SIZE - 1)
#define MAX_FREQ		0x8000

class FILEIO;

/**
	@brief parse and decode to d88 format from teledisk image

	this teledisk image decoder is based on:

		LZHUF.C English version 1.0 based on Japanese version 29-NOV-1988
		LZSS coded by Haruhiko OKUMURA
		Adaptive Huffman Coding coded by Haruyasu YOSHIZAKI
		Edited and translated to English by Kenji RIKITAKE
		TDLZHUF.C by WTK
*/
class TELEDISK_PARSER
{
private:
	FILEIO *fio;
	bool temporary;
	_TCHAR tmp_path[_MAX_PATH];

	uint8_t text_buf[STRING_BUFFER_SIZE + LOOKAHEAD_BUFFER_SIZE - 1];
	uint16_t ptr;
	uint16_t bufcnt, bufndx, bufpos;
	uint16_t ibufcnt,ibufndx;
	uint8_t inbuf[512];
	uint16_t freq[TABLE_SIZE + 1];
	short prnt[TABLE_SIZE + N_CHAR];
	short son[TABLE_SIZE];
	uint16_t getbuf;
	uint8_t getlen;

	struct td_hdr_t {
		char sig[3];
		uint8_t unknown;
		uint8_t ver;
		uint8_t dens;
		uint8_t type;
		uint8_t flag;
		uint8_t dos;
		uint8_t sides;
		uint16_t crc;
	};
	struct td_cmt_t {
		uint16_t crc;
		uint16_t len;
		uint8_t ymd[3];
		uint8_t hms[3];
	};
	struct td_trk_t {
		uint8_t nsec, trk, head;
		uint8_t crc;
	};
	struct td_sct_t {
		uint8_t c, h, r, n;
		uint8_t ctrl, crc;
	};

	int next_word();
	int get_bit();
	int get_byte();
	void start_huff();
	void reconst();
	void update(int c);
	short decode_char();
	short decode_position();
	void init_decode();
	int decode(uint8_t *buf, int len);

	void remove_tempfile();

	TELEDISK_PARSER() {}

public:
	TELEDISK_PARSER(FILEIO *fio_, const _TCHAR *file_path_);
	~TELEDISK_PARSER();
	bool convert_to_d88(uint8_t *buffer, size_t buffer_size, int &d88_file_size);
};

/**
	@brief parse and decode to d88 format from any disk image
*/
class DISK_PARSER
{
private:
	FILEIO *fio;
	const _TCHAR *file_path;
	uint8_t *buffer;
	size_t buffer_size;
	int *file_size_ptr;
	uint8_t *tmp_buffer;

	struct fdi_hdr_t {
		uint8_t  unknown[0x10];
		uint32_t sec_size;
		uint32_t n_secs;
		uint32_t n_heads;
		uint32_t n_cyls;
		uint8_t  reserved[0xfe0];
	};

	struct imd_trk_t {
		uint8_t mode;
		uint8_t cyl;
		uint8_t head;
		uint8_t nsec;
		uint8_t size;
	};

	DISK_PARSER() {}
	void alloc_tmp_buffer();
	void free_tmp_buffer();

	bool standard_to_d88(int type, int ncyl, int nside, int nsec, int size);

public:
	DISK_PARSER(FILEIO *fio, const _TCHAR *file_path, uint8_t *buffer, size_t buffer_size, int *file_size_ptr);
	~DISK_PARSER();

	// teledisk image decoder (td0)
	bool teledisk_to_d88();

	// imagedisk image decoder (imd)
	bool imagedisk_to_d88();

	// cpdread image decoder (dsk)
	bool cpdread_to_d88(int extended);

	bool parse_standard(_TCHAR *n_file_path);

	bool parse(int offset, _TCHAR *n_file_path, int &file_size_orig, int &file_offset);
};

#endif /* DISK_PARSER_H */

