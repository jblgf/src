/*	$OpenBSD: pmap_motorola.h,v 1.27 2014/01/30 18:16:41 miod Exp $	*/

/* 
 * Copyright (c) 1987 Carnegie-Mellon University
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)pmap.h	8.1 (Berkeley) 6/10/93
 */

#ifndef	_M68K_M68K_PMAP_MOTOROLA_H_
#define	_M68K_M68K_PMAP_MOTOROLA_H_

#if !defined(_LOCORE)
#include <machine/pte.h>
#endif

#ifdef	_KERNEL

#include <machine/cpu.h>

/*
 * Pmap stuff
 */
struct pmap {
	pt_entry_t		*pm_ptab;	/* KVA of page table */
	st_entry_t		*pm_stab;	/* KVA of segment table */
	int			pm_stfree;	/* 040: free lev2 blocks */
	st_entry_t		*pm_stpa;	/* 040: ST phys addr */
	short			pm_sref;	/* segment table ref count */
	short			pm_count;	/* pmap reference count */
	struct simplelock	pm_lock;	/* lock on pmap */
	struct pmap_statistics	pm_stats;	/* pmap statistics */
	long			pm_ptpages;	/* more stats: PT pages */
};

typedef struct pmap	*pmap_t;

/*
 * On the 040 we keep track of which level 2 blocks are already in use
 * with the pm_stfree mask.  Bits are arranged from LSB (block 0) to MSB
 * (block 31).  For convenience, the level 1 table is considered to be
 * block 0.
 *
 * MAX[KU]L2SIZE control how many pages of level 2 descriptors are allowed.
 * for the kernel and users.  8 implies only the initial "segment table"
 * page is used.  WARNING: don't change MAXUL2SIZE unless you can allocate
 * physically contiguous pages for the ST in pmap.c!
 */
#define	MAXKL2SIZE	32
#define MAXUL2SIZE	8
#define l2tobm(n)	(1 << (n))
#define	bmtol2(n)	(ffs(n) - 1)

/*
 * Macros for speed
 */
#define PMAP_ACTIVATE(pmap, loadhw)					\
{									\
        if ((loadhw))							\
                loadustp(atop((paddr_t)(pmap)->pm_stpa));		\
}

/* XXX - struct pv_entry moved to vmparam.h because of include ordering issues */

extern struct pmap	kernel_pmap_store;

#define pmap_kernel()	(&kernel_pmap_store)
#define	active_pmap(pm) \
	((pm) == pmap_kernel() || (pm) == curproc->p_vmspace->vm_map.pmap)
#define	active_user_pmap(pm) \
	(curproc && \
	 (pm) != pmap_kernel() && (pm) == curproc->p_vmspace->vm_map.pmap)

extern struct pv_entry	*pv_table;	/* array of entries, one per page */

#define	pmap_resident_count(pmap)	((pmap)->pm_stats.resident_count)
#define	pmap_wired_count(pmap)		((pmap)->pm_stats.wired_count)

#define	pmap_copy(dp,sp,d,l,s)		do { /* nothing */ } while (0)
#define	pmap_update(pmap)		do { /* nothing (yet) */ } while (0)
#define	pmap_unuse_final(p)		do { /* nothing */ } while (0)
#define	pmap_remove_holes(map)		do { /* nothing */ } while (0)

extern pt_entry_t	*Sysmap;
extern char		*vmmap;		/* map for mem, dumps, etc. */

int	pmap_enter_cache(pmap_t, vaddr_t, paddr_t, vm_prot_t, int, pt_entry_t);
void	pmap_kenter_cache(vaddr_t, paddr_t, pt_entry_t);

#define PMAP_GROWKERNEL			/* turn on pmap_growkernel interface */

#endif	/* _KERNEL */

#ifndef _LOCORE

struct pv_entry {
	struct pv_entry	*pv_next;	/* next pv_entry */
	struct pmap	*pv_pmap;	/* pmap where mapping lies */
	vaddr_t		pv_va;		/* virtual address for mapping */
	st_entry_t	*pv_ptste;	/* non-zero if VA maps a PT page */
	struct pmap	*pv_ptpmap;	/* if pv_ptste, pmap for PT page */
	int		pv_flags;	/* flags */
};

/*
 * pv_flags carries some PTE permission bits as well - make sure extra flags
 * values are > (1 << PG_SHIFT)
 */
/* header: all entries are cache inhibited */
#define	PV_CI		(0x01 << PG_SHIFT)
/* header: entry maps a page table page */
#define PV_PTPAGE	(0x02 << PG_SHIFT)

struct vm_page_md {
	struct pv_entry pvent;
};

#define	VM_MDPAGE_INIT(pg) do {			\
	(pg)->mdpage.pvent.pv_next = NULL;	\
	(pg)->mdpage.pvent.pv_pmap = NULL;	\
	(pg)->mdpage.pvent.pv_va = 0;		\
	(pg)->mdpage.pvent.pv_ptste = NULL;	\
	(pg)->mdpage.pvent.pv_ptpmap = NULL;	\
	(pg)->mdpage.pvent.pv_flags = 0;	\
} while (0)

#endif	/* _LOCORE */

#endif /* !_M68K_M68K_PMAP_MOTOROLA_H_ */
