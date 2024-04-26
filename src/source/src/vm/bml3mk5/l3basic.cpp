/** @file l3basic.cpp

	HITACHI BASIC MASTER LEVEL3 Mark5 / MB-S1 Emulator 'EmuB-6892/EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2018.01.01 -

	@brief [ level-3 basic ]
*/

#ifdef USE_DEBUGGER

#include "l3basic.h"
#include "memory.h"
#include "../../osd/debugger_console.h"
#include "../../depend.h"
#include <stdio.h>
#include <math.h>
#include "../../utility.h"

L3Basic::L3Basic(MEMORY *mem)
{
	this->mem = mem;
	dc = NULL;
	basic_type = 0;

	basic_sentences = NULL;
	basic_functions = NULL;
	basic_errmsgs = NULL;

	enable_trace = false;
	trace_line_num = 0xffff;
	st_traceback = cur_traceback = 0;
	traceback_prev_num = 0xffff;
}

L3Basic::~L3Basic()
{
	delete basic_sentences;
	delete basic_functions;
	delete basic_errmsgs;
}

void L3Basic::SetDebuggerConsole(DebuggerConsole *dc)
{
	this->dc = dc;
}

void L3Basic::Reset(int basic_type)
{
	this->basic_type = basic_type;
	delete basic_sentences;
	delete basic_functions;
	delete basic_errmsgs;
	basic_sentences = NULL;
	basic_functions = NULL;
	basic_errmsgs   = NULL;

	trace_line_num = 0xffff;
	st_traceback = cur_traceback = 0;
}

uint32_t L3Basic::ReadData8(uint32_t addr)
{
	uint32_t data;
#ifdef _MBS1
	data = mem->debug_read_data8(0, addr);
#else
	data = mem->debug_read_data8(-1, addr & 0xffff);
#endif
	return data;
}

uint32_t L3Basic::ReadData16(uint32_t addr)
{
	uint32_t data;
#ifdef _MBS1
	data = mem->debug_read_data16(0, addr);
#else
	data = mem->debug_read_data16(-1, addr & 0xffff);
#endif
	return data;
}

uint32_t L3Basic::ReadData8m(uint32_t addr, int map_num)
{
	uint32_t data;
#ifdef _MBS1
	data = mem->debug_read_data8(0, MapAddr(addr, map_num));
#else
	data = mem->debug_read_data8(-1, addr & 0xffff);
#endif
	return data;
}

uint32_t L3Basic::ReadData16m(uint32_t addr, int map_num)
{
	uint32_t data;
#ifdef _MBS1
	data = mem->debug_read_data16(0, MapAddr(addr, map_num));
#else
	data = mem->debug_read_data16(-1, addr & 0xffff);
#endif
	return data;
}

void L3Basic::WriteData8(uint32_t addr, uint32_t data)
{
#ifdef _MBS1
	mem->debug_write_data8(0, addr, data);
#else
	mem->debug_write_data8(-1, addr, data);
#endif
}

void L3Basic::WriteData16(uint32_t addr, uint32_t data)
{
#ifdef _MBS1
	mem->debug_write_data16(0, addr, data);
#else
	mem->debug_write_data16(-1, addr, data);
#endif
}

uint32_t L3Basic::CopyBuffer(uint32_t addr, int len, int start_pos)
{
	int pos = start_pos;
	for(; pos < (start_pos + len); pos++) {
		buffer[pos] = ReadData8(addr++);
	}
	buffer[pos] = 0;
	return addr;
}

uint32_t L3Basic::CopyBufferM(uint32_t addr, int len, int map_num, int start_pos)
{
	int pos = start_pos;
	for(; pos < (start_pos + len); pos++) {
		buffer[pos] = ReadData8m(addr++, map_num);
	}
	buffer[pos] = 0;
	return addr;
}

void L3Basic::GetMappingTables()
{
#ifdef _MBS1
	if (basic_type == 1) {
		for(int map_num = 0; map_num < 16; map_num++) {
			uint32_t ptr = ReadData16(0x8601c + map_num * 4);
			ptr = MapAddr(ptr);
			if (ptr != 0) {
				for(int i=0; i<13; i++) {
					mmptbl[map_num][i] = ReadData8(ptr++);
				}
			} else {
				memset(mmptbl[map_num], 0, sizeof(uint8_t) * 13);
			}
		}
	}
#endif
}

uint32_t L3Basic::MapAddr(uint32_t addr, int map_num)
{
	uint32_t paddr = 0;
#ifdef _MBS1
	if (basic_type == 1) {
		int map = (addr >> 12);
		if (map < 13) {
			paddr = ((uint32_t)mmptbl[map_num][addr >> 12] << 12) | (addr & 0xfff);
		} else {
			paddr = MapAddr(addr);
		}
	} else {
		paddr = 0xf0000 | (addr & 0xffff);
	}
#else
	paddr = (addr & 0xffff);
#endif
	return paddr;
}

#ifdef _MBS1
uint32_t L3Basic::MapAddr(uint32_t addr)
{
	switch(addr & 0xf000) {
	case 0xf000:
		addr = (addr & 0xfff) | (0xef000);
		break;
	case 0xe000:
		addr = (addr & 0xfff) | (0x84000);
		break;
	case 0xd000:
		addr = (addr & 0xfff) | (0x85000);
		break;
	case 0xc000:
		addr = (addr & 0xfff) | (0x86000);
		break;
	}
	return addr;
}
#endif

bool L3Basic::IsSameLength(int src_len, const uint8_t *dst_str)
{
	return ((int)strlen((const char *)dst_str) == src_len);
}

bool L3Basic::MatchString(bool match, const _TCHAR *str, int name_cnt, const _TCHAR **names)
{
	if (match || name_cnt == 0 || names == NULL) {
		match = true;
	} else {
		for(int i=0; i<name_cnt; i++) {
			if (names[i] == NULL) {
				break;
			} else if (_tcsicmp(str, names[i]) == 0) {
				match = true;
				break;
			}
		}
	}
	return match;
}

/// @param [in,out] var_type : bit7 set when DEF FN variable
/// @param [in]     name_cnt : below names count
/// @param [in]     names    : name list (for search)
/// @return                  : true if variable name exists in name list
bool L3Basic::SetVariableName(int &var_type, int name_cnt, const _TCHAR **names)
{
	const char *sign[] = { "%","$","!","#",NULL };

	bool match = false;
	int  pos = 2;

	if (buffer[2] & 0x80) {
		// DEF FNx name
		var_type |= 0x80;
		buffer[0] = 'F'; buffer[1] = 'N'; buffer[2] &= 0x7f;
		pos = 0;
	}
	variable_name.SetN((const char *)&buffer[pos]);
	match = MatchString(match, variable_name.Get(), name_cnt, names);
	switch(var_type & 0xf) {
	case 3:	// string
		UTILITY::strcat((char *)&buffer[pos], sizeof(buffer) - (size_t)pos, sign[1]);
		break;
	case 4:	// single float
		UTILITY::strcat((char *)&buffer[pos], sizeof(buffer) - (size_t)pos, sign[2]);
		break;
	case 8:	// double float
		UTILITY::strcat((char *)&buffer[pos], sizeof(buffer) - (size_t)pos, sign[3]);
		break;
	default: // case 2: // integer
		UTILITY::strcat((char *)&buffer[pos], sizeof(buffer) - (size_t)pos, sign[0]);
		break;
	}
	variable_name.SetN((const char *)&buffer[pos]);
	match = MatchString(match, variable_name.Get(), name_cnt, names);

	return match;
}

void L3Basic::PrintVariableName()
{
	dc->Print(variable_name.Get(), false);
}

int L3Basic::SetUint8(const uint8_t *val, uint8_t *buf, size_t bufsiz, int pos)
{
	uint8_t svalue[64];
	UTILITY::sprintf((char *)svalue, 64, "%d", (int)(uint32_t)val[0]);
	UTILITY::strcpy((char *)buf, bufsiz, (const char *)svalue);
	return (int)strlen((char *)svalue)+pos;
}

int L3Basic::CalcInt16(const uint8_t *val)
{
	int value = (int)(uint32_t)val[0] * 256 + val[1];
	if (value >= 0x8000) value -= 0x10000;
	return value;
}

int L3Basic::SetInt16(const uint8_t *val, uint8_t *buf, size_t bufsiz, int pos)
{
	uint8_t svalue[64];
	int value = CalcInt16(val);
	UTILITY::sprintf((char *)svalue, 64, "%d", value);
	UTILITY::strcpy((char *)buf, bufsiz, (const char *)svalue);
	return (int)strlen((char *)svalue)+pos;
}

void L3Basic::PrintInt16(const uint8_t *val)
{
	int value = CalcInt16(val);
	dc->Printf(_T(" = %d (&H%X)"), value, (value & 0xffff));
}

void L3Basic::SetChr(uint8_t &val)
{
	if (!((0x20 <= val && val <= 0x7e) || (0xa0 <= val && val <= 0xdf))) {
		val = '.';
	}
}

void L3Basic::PrintString(uint8_t *val)
{
	int len = val[0];

	for(int n=1; n<=len && n<=256; n++) {
		SetChr(val[n]);
	}
	CTchar nstr((const char *)&val[1]);
	dc->Print(_T(" = \""), false);
	dc->Print(nstr.Get(), false);
	dc->Print(_T("\""), false);
}

double L3Basic::CalcFloat(uint8_t *val, int siz)
{
	double value;
	int expn = ((int)(uint8_t)val[0] - 129);
	int sign = (val[1] & 0x80 ? -1 : 1);

	if (val[0] != 0) {
		val[1] |= 0x80;

		pair64_t val_tmp;
		val_tmp.u64 = 0;
		if (siz == 4) {
			// single
			val_tmp.b.h4 = val[3];
			val_tmp.b.h5 = val[2];
			val_tmp.b.h6 = val[1];
		} else {
			// double
			val_tmp.b.l = val[7];
			val_tmp.b.h = val[6];
			val_tmp.b.h2 = val[5];
			val_tmp.b.h3 = val[4];
			val_tmp.b.h4 = val[3];
			val_tmp.b.h5 = val[2];
			val_tmp.b.h6 = val[1];
		}

		// shift
		int sft = 0;
		for(; sft < 64; sft++) {
			if (val_tmp.u64 & 1) {
				break;
			} else {
				val_tmp.u64 >>= 1;
			}
		}

		expn -= (55 - sft);

		value = (double)sign * (double)(val_tmp.u64) * pow(2.0, expn);
	} else {
		value = 0.0;
	}
	return value;
}

void L3Basic::FormatFloat(double value, int siz, char *buf, size_t bufsiz)
{
	char tmp[128];
	int exp = 0;
	int len = 0;
//	int dir;
	int wid;
	int i;
	char *base = &tmp[64];
	char *p = NULL;

	memset(tmp, '0', 64);
	if (siz == 4) {
		wid = 6;
		UTILITY::sprintf(base, 64, "%+.6E", value);
	} else {
		wid = 16;
		UTILITY::sprintf(base, 64, "%+.16E", value);
	}
	p = strchr(base, 'E');
	sscanf(p, "E%d", &exp);
//	dir = (exp >= 0 ? 1 : -1);
	*p = '\0';
	p--;
	base[2] = base[1];
	base[0] = base[1] = '0';
	base+=2;
	// floor
	if (*p >= '5' && *p <= '9') {
		(*(p-1))++;
	}
	*p = '\0';
	p--;
	// rtrim0
	for(i = 0; p != base; p--) {
		if (*p == ':') {
			(*(p-1))++;
			*p = '0';
		}
		if (*p != '0') {
			i++;
		} else if (i == 0) {
			*p = '\0';
		}
	}
	len = (int)strlen(base);

	if ((len-exp-1) > wid || exp >= wid) {
		if (base[1] != '\0') {
			base--;
			base[0] = base[1];
			base[1] = '.';
		}
		if (value < 0) {
			base--;
			*base = '-';
		}
		UTILITY::sprintf(buf, bufsiz, "%s%c%+03d", base, siz == 4 ? 'E' : 'D', exp);
	} else {
		if (len > (exp + 1)) {
			p = base + len;
			for(i = 0; i < (len-exp); i++, p--) {
				p[1] = p[0];
			}
			p[1] = '.';
			if (exp < 0) {
				base += (exp+1);
			}
		} else {
			p = base + len;
			for(i = 0; i < (exp+1-len); i++, p++) {
				*p = '0';
			}
			*p = '\0';
		}
		if (value < 0) {
			base--;
			*base = '-';
		}
		UTILITY::sprintf(buf, bufsiz, "%s", base);
	}
}

int L3Basic::SetFloat(uint8_t *val, int siz, uint8_t *buf, size_t bufsiz, int pos)
{
	double value = CalcFloat(val, siz);
	FormatFloat(value, siz, (char *)buf, bufsiz);
	return (int)strlen((char *)buf)+pos;
}

void L3Basic::PrintFloat(uint8_t *val, int siz)
{
	char buf[64];
	double value = CalcFloat(val, siz);
	FormatFloat(value, siz, buf, sizeof(buf));
	CTchar nbuf(buf);
	dc->Printf(_T(" = %s (&H%X)"), nbuf.Get(), ((int)value & 0xffff));
}

uint32_t L3Basic::CopyVariableValue(int type, uint32_t addr)
{
	if (type != 3) {
		addr = CopyBufferM(addr, type & 0xf, 4);
	} else {
		// string variable
		uint8_t sz = ReadData8m(addr++, 4);
		buffer[0] = sz;	// str length
		uint32_t pstr = ReadData16m(addr, 4);
		CopyBufferM(pstr, sz, 5, 1);
		addr += 2;
	}
	return addr;
}

void L3Basic::PrintVariableValue(int var_type)
{
	switch(var_type) {
	case 3:
		// string
		PrintString(buffer);
		break;
	case 4:
		// single float
		PrintFloat(buffer, var_type);
		break;
	case 8:
		// double float
		PrintFloat(buffer, var_type);
		break;
	default: // case 2:
		// integer
		PrintInt16(buffer);
		break;
	}
}

void L3Basic::PrintVariable(int name_cnt, const _TCHAR **names)
{
	uint8_t data = 0;

	// simple variables
	uint32_t addr;
	uint32_t arr_addr;
	uint32_t end_addr;

	if (basic_type == 1) {
		GetMappingTables();
		addr = ReadData16(0x84f1f);
		arr_addr = ReadData16(0x84f21);
		end_addr = 0xb000;
	} else {
		addr = ReadData16(0xf001f);
		arr_addr = ReadData16(0xf0021);
		end_addr = 0xa000;
	}

	int var_nums = 0;

	// simple variables
	for(; addr < arr_addr; ) {
		data = ReadData8m(addr++, 4);
		// type
		int var_type = (data >> 4);
		if (!(var_type == 2 || var_type == 3 || var_type == 4 || var_type == 8)) {
			break;
		}

		// length
		int var_len = (data & 0xf) + 1;
		// name
		addr = CopyBufferM(addr, var_len, 4, 2);

		if (!IsSameLength(var_len, &buffer[2])) {
			break;
		}

		bool match = SetVariableName(var_type, name_cnt, names);

		if (match) {
			var_nums++;

			// name
			PrintVariableName();

			// value
			addr = CopyVariableValue(var_type, addr);
			PrintVariableValue(var_type);
			dc->Cr();
		} else {
			addr+=(var_type & 0xf);
		}
	}

	// array variables
	addr = arr_addr;
	for(; addr < end_addr; ) {
		data = ReadData8m(addr++, 4);
		// type
		int var_type = (data >> 4);
		if (!(var_type == 2 || var_type == 3 || var_type == 4 || var_type == 8)) {
			break;
		}

		// length
		int var_len  = (data & 0xf) + 1;
		// name
		addr = CopyBufferM(addr, var_len, 4, 2);

		if (!IsSameLength(var_len, &buffer[2])) {
			break;
		}

		bool match = SetVariableName(var_type, name_cnt, names);

		int len = ReadData16m(addr, 4);
		if (len == 0) {
			// error
			break;
		}

		if (match) {
			addr += 2;

			int dim = ReadData8m(addr++, 4);
			int *idx = new int[dim];
			int nums = 1;
			for(int d=0; d<dim; d++) {
				int num = ReadData16m(addr, 4);
				idx[dim-d-1] = num;
				nums *= num;
				addr += 2;
			}
			if (dim == 0 || nums == 0) {
				// error
				break;
			}

			var_nums++;

			if (name_cnt > 0) {
				for(int i=0; i<nums; i++) {
					PrintVariableName();

					dc->Print(_T("("), false);
					for(int d=0; d<dim; d++) {
						if (d == 0) {
							dc->Printf(_T("%2d"), i % idx[d]);
						} else {
							dc->Printf(_T(",%2d"), (i / idx[d-1]) % idx[d]);
						}
					}
					dc->Print(_T(")"), false);

					// value
					addr = CopyVariableValue(var_type, addr);

					PrintVariableValue(var_type);
					dc->Cr();
				}
			} else {
				PrintVariableName();

				dc->Print(_T("("), false);
				for(int d=0; d<dim; d++) {
					if (d == 0) {
						dc->Printf(_T("%2d"), idx[d]);
					} else {
						dc->Printf(_T(",%2d"), idx[d]);
					}
				}
				dc->Print(_T(")"));
				addr += (nums * (var_type & 0xf));
			}
			delete [] idx;
		} else {
			addr += len;
		}
	}


	if (var_nums == 0) {
		if (name_cnt > 0) {
			dc->Print(_T("no variable found."));
		} else {
			dc->Print(_T("no variable defined."));
		}
	}
}

/// get reserved sentences and functions on BASIC
void L3Basic::GetSentences()
{
	uint32_t addr;
	uint8_t data;
	char buf[64];
	int num;
	int pos;
	int mem_num;
	uint32_t start_addr;

	if (basic_sentences) return;

	if (basic_type == 1) {
		// S1
		GetMappingTables();
		start_addr = 0x84581;
		num = ReadData8(start_addr);
		if (num != 0x63) {
			// not ready
			return;
		}
	} else {
		// L3
		start_addr = 0xf0120;
		num = ReadData8(start_addr);
		if (num != 0x5d) {
			// not ready
			return;
		}
	}

	basic_sentences = new CPtrList<CNchar>(64);
	basic_functions = new CPtrList<CNchar>(64);

	for(int n = 0; n < 8; n++) {
		num = ReadData8(start_addr + (n * 5));
		addr = ReadData16(start_addr + 1 + (n * 5));
		if (n >= 2 && basic_type == 1) {
			mem_num = (ReadData8(start_addr + 4 + (n * 5)) & 0xf);
		} else {
			mem_num = 1;
		}
		for(int s = 0; s < num && s < 256; s++) {
			for (pos = 0; pos < 63; pos++) {
				data = ReadData8m(addr++, mem_num);
				if (data >= 0x80) {
					buf[pos] = (char)(data & 0x7f);
					buf[pos+1] = '\0';
					CNchar *str = new CNchar(buf);
					if (n & 1) {
						basic_functions->Add(str);
					} else {
						basic_sentences->Add(str);
					}
					break;
				} else {
					buf[pos] = (char)data;
				}
			}
		}
	}
}

uint32_t L3Basic::GetLineNumber()
{
#ifdef _MBS1
	return ReadData16(basic_type == 1 ? 0x84f2d : 0xf002d);
#else
	return ReadData16(0x002d);
#endif
}

uint32_t L3Basic::GetLineNumberPtr()
{
#ifdef _MBS1
	return (basic_type == 1 ? 0x84f2e : 0xf002e);
#else
	return 0x002e;
#endif
}

bool L3Basic::IsCurrentLine(uint32_t st_line, uint32_t ed_line)
{
	uint32_t line = GetLineNumber();
	bool match = (trace_line_num != line && st_line <= line && line <= ed_line);
	if (match) {
		trace_line_num = line;
	}
	return match;
}

void L3Basic::SetTraceBack(uint32_t addr)
{
	if (addr == GetLineNumberPtr()) {
		uint32_t line = GetLineNumber();
		if (line != 0xffff) {
			if (traceback_prev_num != line) {
				traceback_prev_num = line;
				traceback_line_num[cur_traceback++] = line;
				if (cur_traceback >= L3BASIC_MAX_TRACE_NUMS) {
					cur_traceback = 0;
					st_traceback = -1;
				}
			}
		} else {
			cur_traceback = st_traceback = 0;
			traceback_prev_num = 0xffff;
		}
	}
}

void L3Basic::PrintCurrentTrace()
{
	if (!enable_trace) return;

	uint32_t line = GetLineNumber();

	if (line != 0xffff) {
		// current line number
		dc->SetTextColor(dc->Yellow);
		PrintList(line, line, 0);
	}
}

void L3Basic::PrintCurrentLine(int num)
{
	uint32_t line = (num >= 0 ? (uint32_t)num : GetLineNumber());

	if (line != 0xffff) {
		// current line number
		uint32_t addr = ReadData16(basic_type == 1 ? 0x84f35 : 0xf0035);
//		dc->Printf(_T("Current line number : %u"), line);
//		dc->Cr();
		dc->SetTextColor(dc->Yellow);
		PrintList(line, line, addr);
	} else {
		dc->Print(dc->Yellow, _T("Current line number is undefined."));
	}
	dc->SetTextColor(dc->White);
}

void L3Basic::PrintTraceBack(int num)
{
	int st = (st_traceback >= 0 ? 0 : cur_traceback + 1);
	dc->SetTextColor(dc->Yellow);
	for(int i=st; i<(cur_traceback+L3BASIC_MAX_TRACE_NUMS); i++) {
		int idx = (i % L3BASIC_MAX_TRACE_NUMS);
		PrintList(traceback_line_num[idx], traceback_line_num[idx], 0);
	}
	dc->SetTextColor(dc->White);
}

uint32_t L3Basic::ReadLine8(uint32_t addr, int pos, uint32_t cur_addr, int &cur_pos)
{
	uint32_t data;
	data = ReadData8m(addr, 3);
	if (MapAddr(addr, 3) == MapAddr(cur_addr, 1)) {
		cur_pos = pos;
	}
	return data;
}

void L3Basic::PrintList(int start_num, int end_num, uint32_t cur_addr)
{
	uint8_t data = 0;
	uint32_t addr = 0;
	uint32_t next;
	uint32_t lnum;
	int pos;
	int cur_pos;
	uint8_t line[268];
	uint8_t val[8];

	GetSentences();

	if (!basic_sentences) return;

	if (cur_addr > 0) {
		cur_addr++;
	}

	line[267] = '\0';

	// program start
	if (basic_type == 1) {
		// S1
		GetMappingTables();
		addr = ReadData16(0x84f1d);
	} else {
		// L3
		addr = ReadData16(0xf001d);
	}
	while(1) {
		next = ReadData16m(addr, 3);
		addr += 2;
		if (next == 0) {
			// program end
			break;
		}
		lnum = (int)ReadData16m(addr, 3);
		addr += 2;
		UTILITY::sprintf((char *)line, sizeof(line), "%d ", lnum);
		if (cur_addr > 0) {
			if (MapAddr(addr, 3) == MapAddr(cur_addr + 4, 1)) {
				cur_addr += 4;
			}
		}
		pos = (int)strlen((const char *)line);
		data = 1;
		int phase = 0;
		if ((uint32_t)start_num <= lnum && lnum <= (uint32_t)end_num) {
			cur_pos = -1;
			for(; pos < 267 && data != 0;) {
				data = ReadLine8(addr++, pos, cur_addr, cur_pos);
				if (data == 0) {
					line[pos++] = 0;
					break;
				}
				// parse
				if (phase == 0 && data == 0xff) {
					// function
					phase = 2;
					data = ReadLine8(addr++, pos, cur_addr, cur_pos);
				} else if (basic_type == 1 && phase == 0 && data == 0xfe) {
					// digit value for S1 BASIC
					phase = 3;
				} else if (phase == 0 && data >= 0x80) {
					// sentence
					phase = 1;
				} else if (phase == 0x40 && data == 0x3a) {
					// colon after data
					phase = 0;
				} else if (data == 0x22) {
					// string
					phase = (phase ^ 0x10);
				}

				if (phase == 1) {
					// sentence
					CNchar *p;
					if (data == 0x8d || data == 0x8f) {
						// ' else
						if (pos > 0) pos--;
					}
					if ((data & 0x7f) < basic_sentences->Count()) {
						p = basic_sentences->Item(data & 0x7f);
						UTILITY::strcpy((char *)&line[pos], sizeof(line) - (size_t)pos, p->Get());
						pos += p->Length();
					}
					phase = 0;
					if (data == 0x8c || data == 0x8d) {
						// rem
						phase |= 0x20;
					} else if (data == 0x83) {
						// data
						phase |= 0x40;
					}
				} else if (phase == 2) {
					// function
					CNchar *p;
					if ((data & 0x7f) < basic_functions->Count()) {
						p = basic_functions->Item(data & 0x7f);
						UTILITY::strcpy((char *)&line[pos], sizeof(line) - (size_t)pos, p->Get());
						pos += p->Length();
					}
					phase = 0;
				} else if (phase == 3) {
					// digit
					data = ReadLine8(addr++, pos, cur_addr, cur_pos);
					if (data == 1) {
						// 1byte integer
						val[0] = ReadLine8(addr++, pos, cur_addr, cur_pos);
						pos = SetUint8(val, &line[pos], sizeof(line) - (size_t)pos, pos);
					} else if (data == 0xf2 || data == 2) {
						// 2bytes integer
						for(int i=0; i<2; i++) val[i] = ReadLine8(addr++, pos, cur_addr, cur_pos);
						pos = SetInt16(val, &line[pos], sizeof(line) - (size_t)pos, pos);
					} else if (data == 4) {
						// 4bytes single float
						for(int i=0; i<4; i++) val[i] = ReadLine8(addr++, pos, cur_addr, cur_pos);
						pos = SetFloat(val, 4, &line[pos], sizeof(line) - (size_t)pos, pos);
					} else if (data == 8) {
						// 8bytes double float
						for(int i=0; i<8; i++) val[i] = ReadLine8(addr++, pos, cur_addr, cur_pos);
						pos = SetFloat(val, 8, &line[pos], sizeof(line) - (size_t)pos, pos);
					}
					phase = 0;
				} else {
					SetChr(data);
					line[pos++] = data;
				}
			}

			CTchar nline((const char *)line);
			dc->Print(nline.Get());
			if (cur_pos >= 0) {
				for(int i=0; i < cur_pos; i++) dc->Print(_T(" "), false);
				dc->Print(_T("^"));
			}

		}
		addr = next;
	}
}

void L3Basic::TraceOnOff(bool enable)
{
	enable_trace = enable;
	if (enable) {
		trace_line_num = 0xffff;
	}
	dc->Printf(_T("Trace %s BASIC program list."), enable_trace ? _T("on") : _T("off"));
	dc->Cr();
}

void L3Basic::PrintCommandList()
{
	int i;
	int cnt;
	CNchar *p;

	delete basic_sentences;
	delete basic_functions;
	basic_sentences = NULL;
	basic_functions = NULL;

	GetSentences();

	if (!basic_sentences) return;

	cnt = basic_sentences->Count();
	for(i=0; i<cnt; i++) {
		p = basic_sentences->Item(i);
		dc->Printf(_T(" %02X: %-14s"), i + 0x80, p->Get());
		if ((i % 4) == 3) dc->Cr();
	}
	if ((i % 4) != 0) dc->Cr();
	cnt = basic_functions->Count();
	for(i=0; i<cnt; i++) {
		p = basic_functions->Item(i);
		dc->Printf(_T(" FF%02X: %-12s"), i + 0x80, p->Get());
		if ((i % 4) == 3) dc->Cr();
	}
	if ((i % 4) != 0) dc->Cr();
}

void L3Basic::GetErrorMessages()
{
	uint32_t addrs[4];
	uint32_t addr;
	uint32_t end_addr;
	uint8_t data;
	char buf[64];
	int len;

	if (basic_errmsgs) return;

	basic_errmsgs = new CPtrList<CTchar>(48);

	if (basic_type == 1) {
		// S1
		addrs[0] = 0xe19c4;
		addrs[1] = 0xe25c6;
		addrs[2] = 0;
		end_addr = 0xeffff;
	} else {
		// L3
		addrs[0] = 0xfb841;
		addrs[1] = 0xf9e5c;	// 3inch disk basic
		addrs[2] = 0xf8e69;	// 5inch disk basic
		addrs[3] = 0;
		end_addr = 0xfbfff;
	}

	for(int i=0; addrs[i] != 0; i++) {
		addr = addrs[i];
		data = ReadData8(addr);
		if (data < 0x30 || data >= 0x7f) continue;

		len = 0;
		while(addr < end_addr) {
			data = ReadData8(addr++);
			if (data >= 0x80) break;
			buf[len++] = (char)data;
			if (data == 0) {
				if (len > 1) {
					CTchar *str = new CTchar(buf);
					basic_errmsgs->Add(str);
					len = 0;
				} else {
					break;
				}
			}
		}
	}
}

void L3Basic::PrintError(int num)
{
	uint32_t err = 0;
	uint32_t erl;
	CTchar *p;

	GetErrorMessages();

	if (!basic_errmsgs) return;

	if (basic_type == 1) {
		// S1
		err = ReadData8(0x84f8b);
		erl = ReadData16(0x84f8c);
	} else {
		// L3
		err = ReadData8(0xf008b);
		erl = ReadData16(0xf008c);
	}

	if (num > 0) {
		err = (uint32_t)num;
	}
	int i = (int)err-1;
	if (i >= 49) i-=24;

	if (i < 0) {
		dc->Print(_T("No error occured."), false);
	} else {
		dc->Printf(_T("Error %u"), err);
		if ((err <= 25 || 50 <= err) && i < basic_errmsgs->Count()) {
			p = basic_errmsgs->Item(i);
			dc->Printf(_T(" : %s"), p->Get());
		}
		if (erl != 0xffff) {
			dc->Printf(_T(" in %u"), erl);
		}
	}
	dc->Cr();
}

#ifdef _MBS1
void L3Basic::PrintMMPTBL(int num)
{
	uint32_t ptr;
	uint32_t pptr;
	uint32_t data;
	int memst = (0 <= num && num < 16 ? num : 0);
	int memed = (0 <= num && num < 16 ? num : 15);

	for(int mem = memst; mem <= memed; mem++) {
		ptr = ReadData16(0x8601c + mem * 4);
		pptr = MapAddr(ptr);

		dc->Printf(_T("MAP %01X : stored address is %05X(%04X)"), mem, pptr, ptr);
		dc->Cr();
		if (ptr != 0) {
			for(int i = 0; i < 13; i++) {
				data = ReadData8(pptr + i);
				if (i > 0) dc->PutCh(_T(' '));
				dc->Printf(_T("%01X:%02X"),i,(data & 0xff));
			}
			dc->Printf(_T(" D:%02X E:%02X F:%02X"),0x85,0x84,0xef);
		} else {
			dc->Print(_T("Address map is undefined."), false);
		}
		dc->Cr();
	}
}

void L3Basic::PrintCurrentSpaceMap(int index)
{
	int num = (int)(ReadData8(0x84ed3) & 0xff);
	dc->Printf(_T("Current map is %01X."), num);
	dc->Cr();
	switch(index) {
	case 0:
		PrintMMPTBL(num);
		break;
	case 0x400:
		PrintMCHTBL(num);
		break;
	case 0x600:
		PrintSysCall(num);
		break;
	case 0x800:
		PrintMMDPG(num);
		break;
	}
}

void L3Basic::PrintMRASGN()
{
	uint32_t addr = 0;
	uint32_t data;

	dc->Print(_T("      0 1 2 3 4 5 6 7 8 9 A B C D E F"));
	for(int n=0; n < 24; n += 2) {
		data = ReadData16(0x86000 + n);
		dc->Printf(_T("%05X"), addr);
		for(int i=0; i<16; i++) {
			if (data & (0x8000 >> i)) {
				// assigned
				dc->Printf(_T(" o"));
			} else {
				dc->Printf(_T(" -"));
			}
		}
		switch(n) {
		case 0:
			dc->Print(_T("  o: assigned or reserved"), false);
			break;
		case 2:
			dc->Print(_T("  -: no assign"), false);
			break;
		}
		dc->Cr();
		addr += 0x10000;
	}
}

void L3Basic::PrintMCHTBL(int num)
{
	uint32_t pptr = 0x8612c;
	uint32_t data;
	int memst = (0 <= num && num < 16 ? num : 0);
	int memed = (0 <= num && num < 16 ? num : 15);

	for(int mem = memst; mem <= memed; mem++) {
		dc->Printf(_T("MAP %01X"), mem);
		dc->Cr();
		for(int i = 0; i < 13; i++) {
			data = ReadData8(pptr + mem * 13 + i);
			if (i > 0) dc->PutCh(_T(' '));
			dc->Printf(_T("%01X:%02X"),i,(data & 0xff));
		}
		dc->Cr();
	}
}

void L3Basic::PrintSysCall(int num)
{
	uint32_t ptr;
	uint32_t pptr;
	uint32_t sptr;
	uint32_t spptr = 0;
	int scnt;
	uint32_t data;
	uint32_t sdata;
	int memst = (0 <= num && num < 16 ? num : 0);
	int memed = (0 <= num && num < 16 ? num : 15);
	uint32_t mmap[16];

	for(int mem = memst; mem <= memed; mem++) {
		// get memory map
		ptr = ReadData16(0x8601c + mem * 4);
		sptr = ReadData16(0x8601c + mem * 4 + 2);
		pptr = MapAddr(ptr);

		// get number of system calls
		scnt = (int)ReadData8(0x861fe + mem);
		if (pptr != 0) {
			for(int i = 0; i < 13; i++) {
				mmap[i] = ReadData8(pptr + i);
			}
			mmap[13]=0x85; mmap[14]=0x84; mmap[15]=0xef;

			spptr = (mmap[(sptr >> 12) & 0xf] << 12) | (sptr & 0xfff);
		} else {
			spptr = 0;
		}

		dc->Printf(_T("MAP %01X : %d system call(s), stored address is %05X(%04X)"), mem, scnt, spptr, sptr);
		dc->Cr();
		if (pptr != 0 && scnt > 0) {
			for(int i = 0; i < scnt; i++) {
				data = ReadData16(spptr + i * 2);
				sdata = (mmap[(data >> 12) & 0xf] << 12) | (data & 0xfff);
				dc->Printf(_T("%02X : %05X(%04X)"), i, sdata, data);
				if ((i % 4) == 3 || i == (scnt-1)) dc->Cr();
				else dc->Print(_T("    "), false);
			}
		} else {
			dc->Print(_T("System call is undefined."));
		}
	}
}

void L3Basic::PrintMMDPG(int num)
{
	uint32_t pptr = 0x8620e;
	uint32_t data;
	int memst = (0 <= num && num < 16 ? num : 0);
	int memed = (0 <= num && num < 16 ? num : 15);

	for(int mem = memst; mem <= memed; mem++) {
		data = ReadData8(pptr + mem);
		if (mem > 0) dc->PutCh(_T(' '));
		dc->Printf(_T("%01X:%02X"), mem, data);
	}
	dc->Cr();
}

void L3Basic::EditMMPTBL(int num, const int *values, int count)
{
	uint32_t ptr = ReadData16(0x8601c + num * 4);
	uint32_t pptr = MapAddr(ptr);
	uint32_t data;

	if (ptr == 0) {
		ptr = 0xc05c + num * 13;
		pptr = MapAddr(ptr);
		WriteData16(0x8601c + num * 4, ptr & 0xffff);
	}

	dc->Printf(_T("MAP %01X: Modify addresses"), num);
	dc->Cr();
	dc->Print(_T("        "), false);
	for(int i = 0; i < 13; i++) {
		dc->Printf(_T(" %01X "),i);
	}
	dc->Cr();

	dc->Print(_T("before: "), false);
	for(int i = 0; i < 13; i++) {
		data = ReadData8(pptr + i);
		dc->Printf(_T("%02X "), data);
	}
	dc->Cr();

	dc->Print(_T("after:  "), false);
	for(int i = 0; i < 13; i++) {
		if (i < count) WriteData8(pptr + i, values[i] & 0xff);
		data = ReadData8(pptr + i);
		dc->Printf(_T("%02X "), data);
	}
	dc->Cr();
}

void L3Basic::EditMCHTBL(int num, const int *values, int count)
{
	uint32_t pptr = 0x8612c + num * 13;
	uint32_t data;

	dc->Printf(_T("MAP %01X: Modify ids"), num);
	dc->Cr();
	dc->Print(_T("        "), false);
	for(int i = 0; i < 13; i++) {
		dc->Printf(_T(" %01X "),i);
	}
	dc->Cr();

	dc->Print(_T("before: "), false);
	for(int i = 0; i < 13; i++) {
		data = ReadData8(pptr + i);
		dc->Printf(_T("%02X "), data);
	}
	dc->Cr();

	dc->Print(_T("after:  "), false);
	for(int i = 0; i < 13; i++) {
		if (i < count) WriteData8(pptr + i, values[i] & 0xff);
		data = ReadData8(pptr + i);
		dc->Printf(_T("%02X "), data);
	}
	dc->Cr();
}

void L3Basic::EditMMDPG(int num, const int *values, int count)
{
	uint32_t pptr = 0x8620e;
	uint32_t data;

	dc->Printf(_T("MAP %01X: Modify dp"), num);
	dc->Cr();
	data = ReadData8(pptr + num);
	dc->Printf(_T("before: %02X"), data);
	dc->Cr();
	if (0 < count) WriteData8(pptr + num, values[0] & 0xff);
	data = ReadData8(pptr + num);
	dc->Printf(_T("after:  %02X"), data);
	dc->Cr();
}

void L3Basic::GetMMPTBL(int num, int *values, int &count)
{
	uint32_t ptr = ReadData16(0x8601c + num * 4);
	uint32_t pptr = MapAddr(ptr);
//	uint32_t data;

	if (ptr == 0) {
		ptr = 0xc05c + num * 13;
		pptr = MapAddr(ptr);
	}

	if (count > 13) count = 13;

	for(int i = 0; i < count; i++) {
		values[i] = ReadData8(pptr + i);
	}
}
#endif /* _MBS1 */

#endif /* USE_DEBUGGER */
