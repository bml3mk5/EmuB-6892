/** @file l3basic.h

	HITACHI BASIC MASTER LEVEL3 Mark5 / MB-S1 Emulator 'EmuB-6892/EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2018.01.01 -

	@brief [ level-3 basic ]
*/

#ifndef L3BASIC_H
#define L3BASIC_H

#ifdef USE_DEBUGGER

#include "../../common.h"
#include "../../cchar.h"
#include "../../cptrlist.h"

#define L3BASIC_MAX_TRACE_NUMS	64

class DebuggerConsole;
class MEMORY;

/**
	@brief LEVEL-3 BASIC
*/
class L3Basic
{
private:
	DebuggerConsole *dc;
	MEMORY *mem;

	int basic_type;

	uint8_t buffer[260];
#ifdef _MBS1
	uint8_t mmptbl[16][13];
#endif

	CTchar variable_name;

	CPtrList<CNchar> *basic_sentences;
	CPtrList<CNchar> *basic_functions;
	CPtrList<CTchar> *basic_errmsgs;

	bool enable_trace;
//	bool need_output_trace;
	uint32_t trace_line_num;
	uint32_t traceback_line_num[L3BASIC_MAX_TRACE_NUMS];
	uint32_t traceback_prev_num;
	int st_traceback;
	int cur_traceback;

	uint32_t ReadData8(uint32_t addr);
	uint32_t ReadData16(uint32_t addr);

	uint32_t ReadData8m(uint32_t addr, int map_num);
	uint32_t ReadData16m(uint32_t addr, int map_num);

	void WriteData8(uint32_t addr, uint32_t data);
	void WriteData16(uint32_t addr, uint32_t data);

	uint32_t CopyBuffer(uint32_t addr, int len, int start_pos = 0);
	uint32_t CopyBufferM(uint32_t addr, int len, int map_num, int start_pos = 0);

	void GetMappingTables();
	uint32_t MapAddr(uint32_t addr, int map_num);

	int SetUint8(const uint8_t *val, uint8_t *buf, size_t bufsiz, int pos);
	int CalcInt16(const uint8_t *val);
	int SetInt16(const uint8_t *val, uint8_t *buf, size_t bufsiz, int pos);
	void PrintInt16(const uint8_t *val);
	void SetChr(uint8_t &val);
	void PrintString(uint8_t *val);
	double CalcFloat(uint8_t *val, int siz);
	void FormatFloat(double value, int siz, char *buf, size_t bufsiz);
	int SetFloat(uint8_t *val, int siz, uint8_t *buf, size_t bufsiz, int pos);
	void PrintFloat(uint8_t *val, int siz);

	bool IsSameLength(int src_len, const uint8_t *dst_str);
	bool MatchString(bool match, const _TCHAR *str, int name_cnt, const _TCHAR **names);
	bool SetVariableName(int &var_type, int name_cnt, const _TCHAR **names);
	void PrintVariableName();
	uint32_t CopyVariableValue(int type, uint32_t addr);
	void PrintVariableValue(int var_type);

	void GetSentences();

	uint32_t ReadLine8(uint32_t addr, int pos, uint32_t cur_addr, int &cur_pos);

	void GetErrorMessages();

public:
	L3Basic(MEMORY *mem);
	~L3Basic();
	void SetDebuggerConsole(DebuggerConsole *dc);
	void Reset(int basic_type);

	void PrintCurrentTrace();
	bool IsCurrentLine(uint32_t st_line, uint32_t ed_line);
	void SetTraceBack(uint32_t addr);
	void PrintCurrentLine(int num);
	void PrintTraceBack(int num);
	void PrintVariable(int name_cnt, const _TCHAR **names);
	void PrintList(int start_num, int end_num, uint32_t cur_addr);
	void PrintCommandList();
	void PrintError(int num);

	uint32_t GetLineNumber();
	uint32_t GetLineNumberPtr();
	void TraceOnOff(bool enable);

#ifdef _MBS1
	void PrintMMPTBL(int num);
	void PrintCurrentSpaceMap(int index);
	void PrintMRASGN();
	void PrintMCHTBL(int num);
	void PrintSysCall(int num);
	void PrintMMDPG(int num);
	void EditMMPTBL(int num, const int *values, int count);
	void EditMCHTBL(int num, const int *values, int count);
	void EditMMDPG(int num, const int *values, int count);
	void GetMMPTBL(int num, int *values, int &count);

	static uint32_t MapAddr(uint32_t addr);
#endif
};

#endif /* USE_DEBUGGER */

#endif /* L3BASIC_H */
