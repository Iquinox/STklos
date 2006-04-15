/*
 * thread.h	-- Thread support for STklos
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
 * Last file update: 15-Apr-2006 13:06 (eg)
 */
#ifndef _STK_THREAD_H
#define _STK_THREAD_H

#include <pthread.h>

enum thread_state { th_new, th_runnable, th_terminated, th_blocked};

struct thread_obj {
  stk_header header;
  SCM thunk;
  SCM name;
  SCM specific;
  SCM end_result;
  SCM end_exception;
  enum thread_state state;
  vm_thread_t *vm;
  pthread_t pthread;
  pthread_mutex_t mymutex;
  pthread_cond_t  mycondv;
};


#define THREADP(p)		(BOXED_TYPE_EQ((p), tc_thread))
#define THREAD_THUNK(p)		(((struct thread_obj *) (p))->thunk)
#define THREAD_NAME(p)		(((struct thread_obj *) (p))->name)
#define THREAD_SPECIFIC(p)	(((struct thread_obj *) (p))->specific)
#define THREAD_RESULT(p)	(((struct thread_obj *) (p))->end_result)
#define THREAD_EXCEPTION(p)	(((struct thread_obj *) (p))->end_exception)
#define THREAD_CURMOD(p)	(((struct thread_obj *) (p))->current_module)
#define THREAD_STATE(p)		(((struct thread_obj *) (p))->state)
#define THREAD_VM(p)		(((struct thread_obj *) (p))->vm)
#define THREAD_PTHREAD(p)	(((struct thread_obj *) (p))->pthread)
#define THREAD_MYMUTEX(p)	(((struct thread_obj *) (p))->mymutex)
#define THREAD_MYCONDV(p)	(((struct thread_obj *) (p))->mycondv)

extern SCM STk_primordial_thread; 

#endif /* ! _STK_THREAD_H */
