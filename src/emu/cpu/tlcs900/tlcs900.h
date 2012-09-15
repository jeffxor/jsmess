#ifndef __TLCS900_H__
#define __TLCS900_H__



enum tlcs900_inputs
{
	TLCS900_NMI=0,
	TLCS900_INTWD,
	TLCS900_INT0,
	TLCS900_INTAD,
	TLCS900_INT1,
	TLCS900_INT2,
	TLCS900_INT3,
	TLCS900_INT4,
	TLCS900_INT5,
	TLCS900_INT6,
	TLCS900_INT7,
	TLCS900_INT8,
	TLCS900_TIO,
	TLCS900_NUM_INPUTS
};


enum
{
	TLCS900_PC=1, TLCS900_SR,
	TLCS900_XWA0, TLCS900_XBC0, TLCS900_XDE0, TLCS900_XHL0,
	TLCS900_XWA1, TLCS900_XBC1, TLCS900_XDE1, TLCS900_XHL1,
	TLCS900_XWA2, TLCS900_XBC2, TLCS900_XDE2, TLCS900_XHL2,
	TLCS900_XWA3, TLCS900_XBC3, TLCS900_XDE3, TLCS900_XHL3,
	TLCS900_XIX, TLCS900_XIY, TLCS900_XIZ, TLCS900_XNSP, TLCS900_XSSP,
	TLCS900_DMAS0, TLCS900_DMAS1, TLCS900_DMAS2, TLCS900_DMAS3,
	TLCS900_DMAD0, TLCS900_DMAD1, TLCS900_DMAD2, TLCS900_DMAD3,
	TLCS900_DMAC0, TLCS900_DMAC1, TLCS900_DMAC2, TLCS900_DMAC3,
	TLCS900_DMAM0, TLCS900_DMAM1, TLCS900_DMAM2, TLCS900_DMAM3
};


struct tlcs900_interface
{
	devcb_write8	to1;
	devcb_write8	to3;
	devcb_read8		port_read;
	devcb_write8	port_write;
};


DECLARE_LEGACY_CPU_DEVICE(TLCS900H, tlcs900h);
DECLARE_LEGACY_CPU_DEVICE(TMP95C063, tmp95c063);

extern CPU_DISASSEMBLE( tlcs900 );

#endif
