/*
 * thread-pthreads.h	-- Thread support for STklos
 * 
 * Copyright � 2006 Erick Gallesio - I3S-CNRS/ESSI <eg@essi.fr>
 * 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, 
 * USA.
 * 
 *           Author: Erick Gallesio [eg@essi.fr]
 *    Creation date:  4-Feb-2006 11:03 (eg)
 * Last file update: 16-Apr-2006 10:53 (eg)
 */
#ifndef _STK_THREAD_PTHREADS_H
#define _STK_THREAD_PTHREADS_H

#include <pthread.h>

struct thread_obj_specific {
  pthread_t pthread;
  pthread_mutex_t mymutex;
  pthread_cond_t  mycondv;
};


#define THREAD_PTHREAD(p)	(((struct thread_obj *) (p))->sys_thread.pthread)
#define THREAD_MYMUTEX(p)	(((struct thread_obj *) (p))->sys_thread.mymutex)
#define THREAD_MYCONDV(p)	(((struct thread_obj *) (p))->sys_thread.mycondv)

extern void STk_thread_start_specific(SCM thr);
extern int STk_init_threads_specific(vm_thread_t *vm);

#endif /* ! _STK_THREAD_PTHREADS_H */
