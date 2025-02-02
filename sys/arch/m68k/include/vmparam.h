/*	$OpenBSD: vmparam.h,v 1.13 2014/01/30 18:16:41 miod Exp $	*/

/*
 * Copyright (c) 1988 University of Utah.
 * Copyright (c) 1982, 1990 The Regents of the University of California.
 * All rights reserved.
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
 */

/*
 * Machine dependent constants for m68k-based platforms.
 */
#ifndef	_M68K_VMPARAM_H_
#define	_M68K_VMPARAM_H_

/*
 * USRTEXT is the start of the user text/data space, while USRSTACK
 * is the top (end) of the user stack.
 *
 * NOTE: the ONLY reason that USRSTACK is 0xfff00000 instead of -USIZE
 * was for HPUX compatibility.  That's been removed, so this could
 * change now.
 */
#define	USRTEXT		8192
#define	USRSTACK	0xfff00000	/* Start of user stack */

/*
 * Virtual memory related constants, all in bytes
 */
#ifndef MAXTSIZ
#define	MAXTSIZ		(8*1024*1024)		/* max text size */
#endif
#ifndef DFLDSIZ
#define	DFLDSIZ		(32*1024*1024)		/* initial data size limit */
#endif
#ifndef MAXDSIZ
#define	MAXDSIZ		(64*1024*1024)		/* max data size */
#endif
#ifndef BRKSIZ
#define	BRKSIZ		MAXDSIZ			/* heap gap size */
#endif
#ifndef	DFLSSIZ
#define	DFLSSIZ		(2*1024*1024)		/* initial stack size limit */
#endif
#ifndef	MAXSSIZ
#define	MAXSSIZ		MAXDSIZ			/* max stack size */
#endif

#define STACKGAP_RANDOM	256*1024

/*
 * Sizes of the system and user portions of the system page table.
 */
#define	USRPTSIZE 	(1 * NPTEPG)	/* 4mb */

/*
 * PTEs for mapping user space into the kernel for physio operations.
 * One page is enough to handle 4Mb of simultaneous raw IO operations.
 */
#ifndef USRIOSIZE
#define USRIOSIZE	(1 * NPTEPG)	/* 4mb */
#endif

/*
 * PTEs for system V style shared memory.
 * This is basically slop for kmempt which we actually allocate (malloc) from.
 */
#ifndef SHMMAXPGS
#define SHMMAXPGS	1024		/* 4mb */
#endif

/* user/kernel map constants */
#define VM_MIN_ADDRESS		((vaddr_t)0)
#define VM_MAXUSER_ADDRESS	((vaddr_t)(USRSTACK))
#define VM_MAX_ADDRESS		((vaddr_t)(0-(UPAGES*PAGE_SIZE)))
#define VM_MIN_KERNEL_ADDRESS	((vaddr_t)0)
#define VM_MAX_KERNEL_ADDRESS	((vaddr_t)(0-PAGE_SIZE))

/* virtual sizes (bytes) for various kernel submaps */
#define VM_PHYS_SIZE		(USRIOSIZE*PAGE_SIZE)

/* # of kernel PT pages (initial only, can grow dynamically) */
#define VM_KERNEL_PT_PAGES	((vsize_t)2)

/*
 * Constants which control the way the VM system deals with memory segments.
 */
#define	VM_PHYSSEG_NOADD

#endif /* !_M68K_VMPARAM_H_ */
