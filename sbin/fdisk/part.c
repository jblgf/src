/*	$OpenBSD: part.c,v 1.59 2014/03/17 16:40:00 krw Exp $	*/

/*
 * Copyright (c) 1997 Tobias Weingartner
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/types.h>
#include <sys/disklabel.h>
#include <stdio.h>
#include <string.h>

#include "disk.h"
#include "misc.h"
#include "part.h"

int	PRT_check_chs(struct prt *partn);

static const struct part_type {
	int	type;
	char	sname[14];
} part_types[] = {
	{ 0x00, "unused      "},   /* unused */
	{ 0x01, "DOS FAT-12  "},   /* Primary DOS with 12 bit FAT */
	{ 0x02, "XENIX /     "},   /* XENIX / filesystem */
	{ 0x03, "XENIX /usr  "},   /* XENIX /usr filesystem */
	{ 0x04, "DOS FAT-16  "},   /* Primary DOS with 16 bit FAT */
	{ 0x05, "Extended DOS"},   /* Extended DOS */
	{ 0x06, "DOS > 32MB  "},   /* Primary 'big' DOS (> 32MB) */
	{ 0x07, "NTFS        "},   /* NTFS */
	{ 0x08, "AIX fs      "},   /* AIX filesystem */
	{ 0x09, "AIX/Coherent"},   /* AIX boot partition or Coherent */
	{ 0x0A, "OS/2 Bootmgr"},   /* OS/2 Boot Manager or OPUS */
	{ 0x0B, "Win95 FAT-32"},   /* Primary Win95 w/ 32-bit FAT */
	{ 0x0C, "Win95 FAT32L"},   /* Primary Win95 w/ 32-bit FAT LBA-mapped */
	{ 0x0E, "DOS FAT-16  "},   /* Primary DOS w/ 16-bit FAT, CHS-mapped */
	{ 0x0F, "Extended LBA"},   /* Extended DOS LBA-mapped */
	{ 0x10, "OPUS        "},   /* OPUS */
	{ 0x11, "OS/2 hidden "},   /* OS/2 BM: hidden DOS 12-bit FAT */
	{ 0x12, "Compaq Diag."},   /* Compaq Diagnostics */
	{ 0x14, "OS/2 hidden "},   /* OS/2 BM: hidden DOS 16-bit FAT <32M or Novell DOS 7.0 bug */
	{ 0x16, "OS/2 hidden "},   /* OS/2 BM: hidden DOS 16-bit FAT >=32M */
	{ 0x17, "OS/2 hidden "},   /* OS/2 BM: hidden IFS */
	{ 0x18, "AST swap    "},   /* AST Windows swapfile */
	{ 0x19, "Willowtech  "},   /* Willowtech Photon coS */
	{ 0x1C, "ThinkPad Rec"},   /* IBM ThinkPad recovery partition */
	{ 0x20, "Willowsoft  "},   /* Willowsoft OFS1 */
	{ 0x24, "NEC DOS     "},   /* NEC DOS */
	{ 0x27, "Win Recovery"},   /* Windows hidden Recovery Partition */
	{ 0x38, "Theos       "},   /* Theos */
	{ 0x39, "Plan 9      "},   /* Plan 9 */
	{ 0x40, "VENIX 286   "},   /* VENIX 286 or LynxOS */
	{ 0x41, "Lin/Minux DR"},   /* Linux/MINIX (sharing disk with DRDOS) or Personal RISC boot */
	{ 0x42, "LinuxSwap DR"},   /* SFS or Linux swap (sharing disk with DRDOS) */
	{ 0x43, "Linux DR    "},   /* Linux native (sharing disk with DRDOS) */
	{ 0x4D, "QNX 4.2 Pri "},   /* QNX 4.2 Primary */
	{ 0x4E, "QNX 4.2 Sec "},   /* QNX 4.2 Secondary */
	{ 0x4F, "QNX 4.2 Ter "},   /* QNX 4.2 Tertiary */
	{ 0x50, "DM          "},   /* DM (disk manager) */
	{ 0x51, "DM          "},   /* DM6 Aux1 (or Novell) */
	{ 0x52, "CP/M or SysV"},   /* CP/M or Microport SysV/AT */
	{ 0x53, "DM          "},   /* DM6 Aux3 */
	{ 0x54, "Ontrack     "},   /* Ontrack */
	{ 0x55, "EZ-Drive    "},   /* EZ-Drive (disk manager) */
	{ 0x56, "Golden Bow  "},   /* Golden Bow (disk manager) */
	{ 0x5C, "Priam       "},   /* Priam Edisk (disk manager) */
	{ 0x61, "SpeedStor   "},   /* SpeedStor */
	{ 0x63, "ISC, HURD, *"},   /* ISC, System V/386, GNU HURD or Mach */
	{ 0x64, "NetWare 2.xx"},   /* Novell NetWare 2.xx */
	{ 0x65, "NetWare 3.xx"},   /* Novell NetWare 3.xx */
	{ 0x66, "NetWare 386 "},   /* Novell 386 NetWare */
	{ 0x67, "Novell      "},   /* Novell */
	{ 0x68, "Novell      "},   /* Novell */
	{ 0x69, "Novell      "},   /* Novell */
	{ 0x70, "DiskSecure  "},   /* DiskSecure Multi-Boot */
	{ 0x75, "PCIX        "},   /* PCIX */
	{ 0x80, "Minix (old) "},   /* Minix 1.1 ... 1.4a */
	{ 0x81, "Minix (new) "},   /* Minix 1.4b ... 1.5.10 */
	{ 0x82, "Linux swap  "},   /* Linux swap */
	{ 0x83, "Linux files*"},   /* Linux filesystem */
	{ 0x84, "OS/2 hidden "},   /* OS/2 hidden C: drive */
	{ 0x85, "Linux ext.  "},   /* Linux extended */
	{ 0x86, "NT FAT VS   "},   /* NT FAT volume set */
	{ 0x87, "NTFS VS     "},   /* NTFS volume set or HPFS mirrored */
	{ 0x8E, "Linux LVM   "},   /* Linux LVM */
	{ 0x93, "Amoeba FS   "},   /* Amoeba filesystem */
	{ 0x94, "Amoeba BBT  "},   /* Amoeba bad block table */
	{ 0x99, "Mylex       "},   /* Mylex EISA SCSI */
	{ 0x9F, "BSDI        "},   /* BSDI BSD/OS */
	{ 0xA0, "NotebookSave"},   /* Phoenix NoteBIOS save-to-disk */
	{ 0xA5, "FreeBSD     "},   /* FreeBSD */
	{ 0xA6, "OpenBSD     "},   /* OpenBSD */
	{ 0xA7, "NEXTSTEP    "},   /* NEXTSTEP */
	{ 0xA8, "MacOS X     "},   /* MacOS X main partition */
	{ 0xA9, "NetBSD      "},   /* NetBSD */
	{ 0xAB, "MacOS X boot"},   /* MacOS X boot partition */
	{ 0xAF, "MacOS X HFS+"},   /* MacOS X HFS+ partition */
	{ 0xB7, "BSDI filesy*"},   /* BSDI BSD/386 filesystem */
	{ 0xB8, "BSDI swap   "},   /* BSDI BSD/386 swap */
	{ 0xBF, "Solaris     "},   /* Solaris */
	{ 0xC0, "CTOS        "},   /* CTOS */
	{ 0xC1, "DRDOSs FAT12"},   /* DRDOS/sec (FAT-12) */
	{ 0xC4, "DRDOSs < 32M"},   /* DRDOS/sec (FAT-16, < 32M) */
	{ 0xC6, "DRDOSs >=32M"},   /* DRDOS/sec (FAT-16, >= 32M) */
	{ 0xC7, "HPFS Disbled"},   /* Syrinx (Cyrnix?) or HPFS disabled */
	{ 0xDB, "CPM/C.DOS/C*"},   /* Concurrent CPM or C.DOS or CTOS */
	{ 0xDE, "Dell Maint  "},   /* Dell maintenance partition */
	{ 0xE1, "SpeedStor   "},   /* DOS access or SpeedStor 12-bit FAT extended partition */
	{ 0xE3, "SpeedStor   "},   /* DOS R/O or SpeedStor or Storage Dimensions */
	{ 0xE4, "SpeedStor   "},   /* SpeedStor 16-bit FAT extended partition < 1024 cyl. */
	{ 0xEB, "BeOS/i386   "},   /* BeOS for Intel */
	{ 0xEE, "EFI GPT     "},   /* EFI Protective Partition */
	{ 0xEF, "EFI Sys     "},   /* EFI System Partition */
	{ 0xF1, "SpeedStor   "},   /* SpeedStor or Storage Dimensions */
	{ 0xF2, "DOS 3.3+ Sec"},   /* DOS 3.3+ Secondary */
	{ 0xF4, "SpeedStor   "},   /* SpeedStor >1024 cyl. or LANstep or IBM PS/2 IML */
	{ 0xFF, "Xenix BBT   "},   /* Xenix Bad Block Table */
};

void
PRT_printall(void)
{
	int i, idrows;

	idrows = ((sizeof(part_types)/sizeof(struct part_type))+3)/4;

	printf("Choose from the following Partition id values:\n");
	for (i = 0; i < idrows; i++) {
		printf("%02X %s   %02X %s   %02X %s",
		    part_types[i].type, part_types[i].sname,
		    part_types[i+idrows].type, part_types[i+idrows].sname,
		    part_types[i+idrows*2].type, part_types[i+idrows*2].sname);
		if ((i+idrows*3) < (sizeof(part_types)/sizeof(struct part_type))) {
			printf("   %02X %s\n",
			    part_types[i+idrows*3].type,
			    part_types[i+idrows*3].sname);
		} else
			printf( "\n" );
	}
}

const char *
PRT_ascii_id(int id)
{
	static char unknown[] = "<Unknown ID>";
	int i;

	for (i = 0; i < sizeof(part_types)/sizeof(struct part_type); i++) {
		if (part_types[i].type == id)
			return (part_types[i].sname);
	}

	return (unknown);
}

void
PRT_parse(struct disk *disk, void *prt, off_t offset, off_t reloff,
    struct prt *partn)
{
	unsigned char *p = prt;
	off_t off;

	partn->flag = *p++;
	partn->shead = *p++;

	partn->ssect = (*p) & 0x3F;
	partn->scyl = ((*p << 2) & 0xFF00) | (*(p+1));
	p += 2;

	partn->id = *p++;
	partn->ehead = *p++;
	partn->esect = (*p) & 0x3F;
	partn->ecyl = ((*p << 2) & 0xFF00) | (*(p+1));
	p += 2;

	if ((partn->id == DOSPTYP_EXTEND) || (partn->id == DOSPTYP_EXTENDL))
		off = reloff;
	else
		off = offset;

	partn->bs = getlong(p) + off;
	partn->ns = getlong(p+4);

	PRT_fix_CHS(disk, partn);
}

int
PRT_check_chs(struct prt *partn)
{
	if ( (partn->shead > 255) ||
		(partn->ssect >63) ||
		(partn->scyl > 1023) ||
		(partn->ehead >255) ||
		(partn->esect >63) ||
		(partn->ecyl > 1023) )
	{
		return 0;
	}
	return 1;
}
void
PRT_make(struct prt *partn, off_t offset, off_t reloff, void *prt)
{
	unsigned char *p = prt;
	int ecsave, scsave;
	int modified = 0;
	off_t off;

	if ((partn->scyl > 1023) || (partn->ecyl > 1023)) {
		scsave = partn->scyl;
		ecsave = partn->ecyl;
		partn->scyl = (partn->scyl > 1023)? 1023: partn->scyl;
		partn->ecyl = (partn->ecyl > 1023)? 1023: partn->ecyl;
		modified = 1;
	}
	if ((partn->id == DOSPTYP_EXTEND) || (partn->id == DOSPTYP_EXTENDL))
		off = reloff;
	else
		off = offset;

	if (PRT_check_chs(partn)) {
		*p++ = partn->flag & 0xFF;

		*p++ = partn->shead & 0xFF;
		*p++ = (partn->ssect & 0x3F) | ((partn->scyl & 0x300) >> 2);
		*p++ = partn->scyl & 0xFF;

		*p++ = partn->id & 0xFF;

		*p++ = partn->ehead & 0xFF;
		*p++ = (partn->esect & 0x3F) | ((partn->ecyl & 0x300) >> 2);
		*p++ = partn->ecyl & 0xFF;
	} else {
		/* should this really keep flag, id and set others to 0xff? */
		*p++ = partn->flag & 0xFF;
		*p++ = 0xFF;
		*p++ = 0xFF;
		*p++ = 0xFF;
		*p++ = partn->id & 0xFF;
		*p++ = 0xFF;
		*p++ = 0xFF;
		*p++ = 0xFF;
		printf("Warning CHS values out of bounds only saving LBA values\n");
	}

	putlong(p, partn->bs - off);
	putlong(p+4, partn->ns);
	if (modified) {
		partn->scyl = scsave;
		partn->ecyl = ecsave;
	}
}

void
PRT_print(int num, struct prt *partn, char *units)
{
	double size;
	int i;
	i = unit_lookup(units);

	if (partn == NULL) {
		printf("            Starting         Ending         LBA Info:\n");
		printf(" #: id      C   H   S -      C   H   S [       start:        size ]\n");
		printf("-------------------------------------------------------------------------------\n");
	} else {
		size = ((double)partn->ns * unit_types[SECTORS].conversion) /
		    unit_types[i].conversion;
		printf("%c%1d: %.2X %6u %3u %3u - %6u %3u %3u [%12u:%12.0f%s] %s\n",
		    (partn->flag == 0x80)?'*':' ',
		    num, partn->id,
		    partn->scyl, partn->shead, partn->ssect,
		    partn->ecyl, partn->ehead, partn->esect,
		    partn->bs, size,
		    unit_types[i].abbr,
		    PRT_ascii_id(partn->id));
	}
}

void
PRT_fix_BN(struct disk *disk, struct prt *part, int pn)
{
	u_int32_t spt, tpc, spc;
	u_int32_t start = 0;
	u_int32_t end = 0;

	/* Zero out entry if not used */
	if (part->id == DOSPTYP_UNUSED) {
		memset(part, 0, sizeof(*part));
		return;
	}

	/* Disk geometry. */
	spt = disk->sectors;
	tpc = disk->heads;
	spc = spt * tpc;

	start += part->scyl * spc;
	start += part->shead * spt;
	start += part->ssect - 1;

	end += part->ecyl * spc;
	end += part->ehead * spt;
	end += part->esect - 1;

	/* XXX - Should handle this... */
	if (start > end)
		warn("Start of partition #%d after end!", pn);

	part->bs = start;
	part->ns = (end - start) + 1;
}

void
PRT_fix_CHS(struct disk *disk, struct prt *part)
{
	u_int32_t spt, tpc, spc;
	u_int32_t start, end, size;
	u_int32_t cyl, head, sect;

	/* Zero out entry if not used */
	if (part->id == DOSPTYP_UNUSED || part->ns == 0) {
		memset(part, 0, sizeof(*part));
		return;
	}

	/* Disk geometry. */
	spt = disk->sectors;
	tpc = disk->heads;
	spc = spt * tpc;

	start = part->bs;
	size = part->ns;
	end = (start + size) - 1;

	/* Figure out starting CHS values */
	cyl = (start / spc); start -= (cyl * spc);
	head = (start / spt); start -= (head * spt);
	sect = (start + 1);

	part->scyl = cyl;
	part->shead = head;
	part->ssect = sect;

	/* Figure out ending CHS values */
	cyl = (end / spc); end -= (cyl * spc);
	head = (end / spt); end -= (head * spt);
	sect = (end + 1);

	part->ecyl = cyl;
	part->ehead = head;
	part->esect = sect;
}

