/** @file memory.h

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.06.08 -

	@brief [ memory ]
*/

#ifndef MEMORY_H
#define MEMORY_H

#include "../vm_defs.h"
#include "../device.h"

class EMU;
#ifdef USE_DEBUGGER
class L3Basic;
#endif

#define BANK_SIZE	4

/**
	@brief Memory access
*/
class MEMORY : public DEVICE
{
public:
	/// @brief signals on MEMORY
	enum SIG_MEMORY_IDS {
		SIG_MEMORY_DISP		= 0,
		SIG_MEMORY_VSYNC	= 1,
		SIG_MEMORY_PIA_PA	= 2,
		SIG_TRACE_COUNTER	= 4
	};

private:
	DEVICE *d_cpu;
	DEVICE *d_pia, *d_crtc, *d_disp, *d_acia;
	DEVICE *d_key, *d_sound, *d_comm, *d_cmt, *d_timer;
	DEVICE *d_kanji, *d_fdc3, *d_fdc5, *d_fdd, *d_psg3, *d_psg9;
	DEVICE *d_pia_ex, *d_acia_ex, *d_comm1, *d_pia_ex2;
	DEVICE *d_rtc, *d_board;

	uint8_t rom[0x6000];
	uint8_t rom1802[0x800];
	uint8_t rom1805[0x800];
	uint8_t rom1806[0x800];
	uint8_t ram[0x10000];
	uint8_t color_ram[0x4000];	// 0x0400 - 0x43ff
	uint8_t ig_ram[0x0800 * 3];	// 0xa000 - 0xa7ff * 3 (b,r,g)
	uint8_t ram2[0x10000];		// extended ram

	uint8_t wdmy[1 << BANK_SIZE];
	uint8_t rdmy[1 << BANK_SIZE];
	uint8_t* wbank[0x10000 >> BANK_SIZE];
	uint8_t* rbank[0x10000 >> BANK_SIZE];
	uint8_t banktype[0x10000 >> BANK_SIZE];

	uint8_t color_reg;

	uint8_t ig_enable;
	uint8_t ig_enreg;

	uint8_t mem_bank_reg1;	// from PIA port A
	uint8_t mem_bank_reg2;	// from 0xffe8
	bool    mem_vram_sel;	// select vram 

//	uint8_t vram_sel;

	int   trace_counter_id;
	uint32_t pc, pc_prev;

	int   rom_loaded;
	int   rom1802_loaded;
	int   rom1805_loaded;
	int   rom1806_loaded;
	bool  rom_loaded_at_first;

	//for resume
#pragma pack(1)
	struct vm_state_st {
		uint8_t ram[0x10000];
		uint8_t ram2[0x10000];		// extended ram
		uint8_t color_ram[0x4000];	// 0x0400 - 0x43ff
		uint8_t ig_ram[0x0800 * 3];	// 0xa000 - 0xa7ff * 3 (b,r,g)

		uint8_t mem_bank_reg1;	// from PIA port A
		uint8_t mem_bank_reg2;	// from 0xffe8

		int   trace_counter_id;
		uint32_t pc;
		uint32_t pc_prev;

		// from config
		uint8_t dipswitch;
		int fdd_type;
		int io_port;

		char  reserved[9];
	};
#pragma pack()

	void SET_BANK(int s, int e, uint8_t* w, uint8_t* r, uint8_t type);

	void load_rom_files();
	void reset_igram();
	void reset_membank();
	void reset_tracecounter();
	void set_igmode(uint32_t data);
	inline void fetch_trace_counter();
	inline void set_trace_counter();
	inline void clear_tracecounter();

	void change_l3_memory_bank();

#ifdef USE_DEBUGGER
	DebuggerConsole *dc;
	L3Basic *bas;
#endif

public:
	MEMORY(VM* parent_vm, EMU* parent_emu, const char* identifier);
	~MEMORY();

	// common functions
	void initialize();
	void reset();
#ifdef USE_CPU_REAL_MACHINE_CYCLE
	void write_data8w(uint32_t addr, uint32_t data, int *wait);
	uint32_t read_data8w(uint32_t addr, int *wait);
	void latch_address(uint32_t addr, int *wait);
#else
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
#endif
//	void write_data16(uint32_t addr, uint32_t data) {
//		write_data8(addr, data & 0xff); write_data8(addr + 1, data >> 8);
//	}
//	uint32_t read_data16(uint32_t addr) {
//		return read_data8(addr) | (read_data8(addr + 1) << 8);
//	}
	void write_io8(uint32_t addr, uint32_t data);
	void write_signal(int id, uint32_t data, uint32_t mask);

	// unique function
	void set_context_cpu(DEVICE* device) {
		d_cpu = device;
	}
	void set_context_pia(DEVICE* device) {
		d_pia = device;
	}
	void set_context_crtc(DEVICE* device) {
		d_crtc = device;
	}
	void set_context_display(DEVICE* device) {
		d_disp = device;
	}
	void set_context_acia(DEVICE* device) {
		d_acia = device;
	}
	void set_context_key(DEVICE* device) {
		d_key = device;
	}
	void set_context_sound(DEVICE* device) {
		d_sound = device;
	}
	void set_context_psg3(DEVICE* device) {
		d_psg3 = device;
	}
	void set_context_psg9(DEVICE* device) {
		d_psg9 = device;
	}
	void set_context_comm(DEVICE* device) {
		d_comm = device;
	}
	void set_context_cmt(DEVICE* device) {
		d_cmt = device;
	}
	void set_context_timer(DEVICE* device) {
		d_timer = device;
	}
	void set_context_kanji(DEVICE* device) {
		d_kanji = device;
	}
	void set_context_fdc(DEVICE* device3, DEVICE* device5) {
		d_fdc3 = device3;
		d_fdc5 = device5;
	}
	void set_context_fdd(DEVICE* device) {
		d_fdd = device;
	}
	void set_context_pia_ex(DEVICE* device) {
		d_pia_ex = device;
	}
	void set_context_acia_ex(DEVICE* device) {
		d_acia_ex = device;
	}
	void set_context_comm1(DEVICE* device) {
		d_comm1 = device;
	}
	void set_context_pia_ex2(DEVICE* device) {
		d_pia_ex2 = device;
	}
	void set_context_rtc(DEVICE* device) {
		d_rtc = device;
	}
	void set_context_board(DEVICE* device) {
		d_board = device;
	}
	uint8_t* get_vram();

	uint8_t* get_color_ram() {
		return color_ram;
	}
	uint8_t* get_ig_ram() {
		return ig_ram;
	}

	void event_callback(int event_id, int err);
	void update_config();

	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);


#ifdef USE_DEBUGGER
	void set_debugger_console(DebuggerConsole *dc);


	void debug_write_data8(int type, uint32_t addr, uint32_t data);
	uint32_t debug_read_data8(int type, uint32_t addr);
	void debug_write_data16(int type, uint32_t addr, uint32_t data);
	uint32_t debug_read_data16(int type, uint32_t addr);
	void debug_write_data32(int type, uint32_t addr, uint32_t data);
	uint32_t debug_read_data32(int type, uint32_t addr);

	uint32_t debug_physical_addr_mask(int type);
	bool debug_physical_addr_type_name(int type, _TCHAR *buffer, size_t buffer_len);

	uint32_t debug_read_bank(uint32_t addr);
	void debug_memory_map_info(DebuggerConsole *dc);
	void print_memory_bank(uint32_t data, bool w, _TCHAR *buffer, size_t buffer_len);

	bool debug_write_reg(uint32_t reg_num, uint32_t data);
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);

	int  get_debug_graphic_memory_size(int num, int type, int *width, int *height);
	bool debug_graphic_type_name(int type, _TCHAR *buffer, size_t buffer_len);
	bool debug_draw_graphic(int type, int width, int height, scrntype *buffer);
	bool debug_dump_graphic(int type, int width, int height, uint16_t *buffer);

	uint32_t debug_basic_get_line_number_ptr();
	uint32_t debug_basic_get_line_number();

	bool debug_basic_is_supported();
	void debug_basic_variables(DebuggerConsole *dc, int name_cnt, const _TCHAR **names);
	void debug_basic_list(DebuggerConsole *dc, int st_line, int ed_line);
	void debug_basic_trace_onoff(DebuggerConsole *dc, bool enable);
	void debug_basic_trace_current();
	void debug_basic_trace_back(DebuggerConsole *dc, int num);
	void debug_basic_command(DebuggerConsole *dc);
	void debug_basic_error(DebuggerConsole *dc, int num);

	bool debug_basic_check_break_point(uint32_t line, int len);

#endif

};

#endif /* MEMORY_H */
