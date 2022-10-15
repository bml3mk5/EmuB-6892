/** @file mc6809_consts.h

	Skelton for retropc emulator

	@par Origin MAME 0.142
	@author Takeda.Toshiya
	@date   2011.05.06-

	@note Modify By Sasaji on 2021.06.21 -

	@brief [ MC6809 constant values ]
*/

#ifndef MC6809_CONSTS_H
#define MC6809_CONSTS_H

/** @brief int_state v2 on MC6809 */
enum MC6809_INT_BITS_V2 {
	MC6809_IRQ_BIT	= 1,	/**< IRQ line number  */
	MC6809_FIRQ_BIT	= 2,	/**< FIRQ line number */
	MC6809_NMI_BIT	= 4,	/**< NMI line number  */
	MC6809_HALT_BIT	= 8,	/**< HALT line number  */
};

/** @brief flag bits status in MC6809 */
enum MC6809_STATUS_BITS {
	MC6809_CWAI		 = 0x0010,	/**< set when CWAI is waiting for an interrupt */
	MC6809_SYNC		 = 0x0020,	/**< set when SYNC is waiting for an interrupt */
	MC6809_CWAI_IN	 = 0x0040,	/**< (reserved) set when CWAI is waiting for an interrupt */
	MC6809_CWAI_OUT	 = 0x0080,	/**< (reserved) set when CWAI is waiting for an interrupt */
	MC6809_SYNC_IN	 = 0x0100,	/**< (reserved) set when SYNC is waiting for an interrupt */
	MC6809_SYNC_OUT	 = 0x0200,	/**< (reserved) set when SYNC is waiting for an interrupt */
	MC6809_LDS		 = 0x0400,	/**< set when LDS occured at least once */
	MC6809_RESET_BIT = 0x0800,	/**< RES line number */
	MC6809_NMI_LC	 = 0x1000,	/**< (reserved) NMI occur less than 3 cycles */
	MC6809_FIRQ_LC	 = 0x2000,	/**< (reserved) FIRQ occur less than 3 cycles */
	MC6809_IRQ_LC	 = 0x4000,	/**< (reserved) IRQ occur less than 3 cycles */
	MC6809_INSN_HALT = 0x8000,	/**< internal HALT */

	MC6809_HALT_REL	= 0x80000,	/**< HALT release (for debug) */
};

/** @brief int_state v1 on MC6809 */
enum MC6809_INT_BITS_V1 {
	MC6809_CWAI_V1		= 0x08,	/**< set when CWAI is waiting for an interrupt */
	MC6809_SYNC_V1		= 0x10,	/**< set when SYNC is waiting for an interrupt */
	MC6809_LDS_V1		= 0x20,	/**< set when LDS occured at least once */
	MC6809_HALT_BIT_V1	= 0x40,	/**< HALT line number  */
};

/** @brief flag bits in the cc register in MC6809 */
enum MC6809_CC_BITS {
	CC_C	= 0x01,		/**< Carry */
	CC_V	= 0x02,		/**< Overflow */
	CC_Z	= 0x04,		/**< Zero */
	CC_N	= 0x08,		/**< Negative */
	CC_II	= 0x10,		/**< Inhibit IRQ */
	CC_H	= 0x20,		/**< Half (auxiliary) carry */
	CC_IF	= 0x40,		/**< Inhibit FIRQ */
	CC_E	= 0x80,		/**< entire state pushed */
};

#define pPPC    ppc
#define pPC 	pc
#define pU	u
#define pS	s
#define pX	x
#define pY	y
#define pD	acc

#define PPC	ppc.w.l
#define PC	pc.w.l
#define PCD	pc.d
#define U	u.w.l
#define UD	u.d
#define S	s.w.l
#define SD	s.d
#define X	x.w.l
#define XD	x.d
#define Y	y.w.l
#define YD	y.d
#define D	acc.w.l
#define A	acc.b.h
#define B	acc.b.l
#define DP	dp.b.h
#define DPD	dp.d
#define CC	cc

#define EA	ea.w.l
#define EAD	ea.d
#define EAP	ea

#endif /* MC6809_CONSTS_H */

