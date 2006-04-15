/*
 * stklos.h	-- stklos.h
 * 
 * Copyright � 1999-2006 Erick Gallesio - I3S-CNRS/ESSI <eg@unice.fr>
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
 *           Author: Erick Gallesio [eg@unice.fr]
 *    Creation date: 28-Dec-1999 22:58 (eg)
 * Last file update: 14-Apr-2006 20:17 (eg)
 */

#ifndef STKLOS_H
#define STKLOS_H

#ifdef __cplusplus
extern "C" 
{
#endif

#define STK_USE_PTHREADS 1

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <setjmp.h>
#include <memory.h>

#include "stklosconf.h"
#include "extraconf.h"

#ifdef HAVE_GC
# include <gc/gc.h>
#else
# include <gc.h>
#endif

/*===========================================================================*\
 * 
 * 		Declaration of some constants (mainly maxima) 
 *
\*===========================================================================*/

#ifndef FALSE
#  define FALSE 0
#endif
  
#ifndef TRUE
#  define TRUE (!FALSE)
#endif

#ifdef _POSIX_PATH_MAX
#  define MAX_PATH_LENGTH	 _POSIX_PATH_MAX
#else
#  define MAX_PATH_LENGTH	 256
#endif

#define MAX_TOKEN_SIZE 		1024		/* max size of a token */
#define MAX_CHAR_CODE		255		/* Max code for a char */


#define CPP_CONCAT(x, y) 	x##y
#define Inline inline 

#define AS_LONG(x)		((unsigned long) (x))
#define AS_SCM(x)		((SCM) ((unsigned long) (x)))


/*===========================================================================*\
 * 
 * 			Memory allocation
 *
\*===========================================================================*/
  /* 
   * This code is an excerpt of the function used in Boehm GC (i.e. all the 
   * functions of the GC used in the interpreter must be declared here since 
   * the file <gc.h> is not included in the source file in order to simplify
   * header file management (i.e. only this header file is necessary to use
   * the stklos library.
   * Don't use the functions GC_* they can be changed. The only allocation 
   * functions that must be used are functions of the form STk_*
   */

  /* GC interface. *** DON'T USE IT DIRECTLY *** */
// 
// #define GC_API extern 
// 
// typedef void (*GC_finalization_proc) (void * obj, void * client_data);
// 
// GC_API void * GC_malloc(size_t size_in_bytes);
// GC_API void * GC_malloc_atomic(size_t size_in_bytes);
// GC_API void * GC_realloc(void * old_object, size_t new_size_in_bytes);
// GC_API void GC_free(void * object_addr);
// 
// GC_API void GC_register_finalizer(void * obj, GC_finalization_proc fn,
// 				  void * cd, GC_finalization_proc *ofn,
// 				  void * *ocd);
// 
// GC_API void GC_gcollect(void);
// GC_API void GC_init(void);

  /* Scheme interface. *** THIS IS THE INTERFACE TO USE ***  */

#define STk_must_malloc(size) 		GC_malloc(size)
#define STk_must_malloc_atomic(size)	GC_malloc_atomic(size)
#define STk_must_realloc(ptr, size) 	GC_realloc((ptr), (size))
#define STk_free(ptr)			GC_free(ptr)
#define STk_register_finalizer(ptr, f)  GC_register_finalizer((void *) (ptr), \
					    (GC_finalization_proc)(f), 0, 0, 0)
#define STk_gc()			GC_gcollect()
#define STk_gc_init()			GC_init()

/*===========================================================================*\
 * 
 * 		Declaration of the SCM type 
 * 
\*===========================================================================*/

#define MAX_CELL_TYPES		256

typedef void* SCM;

typedef enum {
  tc_not_boxed=-1,						
  tc_cons, tc_integer, tc_real, tc_bignum,  tc_rational, 		/* 0 */
  tc_complex, tc_symbol, tc_keyword, tc_string, tc_module, 		/* 5 */
  tc_instance, tc_closure, tc_subr0, tc_subr1, tc_subr2, 		/* 10 */
  tc_subr3, tc_subr4, tc_subr5, tc_subr01, tc_subr12,  			/* 15 */
  tc_subr23, tc_vsubr, tc_apply, tc_vector, tc_uvector,			/* 20 */
  tc_hash_table, tc_port, tc_frame, tc_next_method, tc_promise, 	/* 25 */
  tc_regexp, tc_process, tc_continuation, tc_values, tc_parameter,	/* 30 */
  tc_socket, tc_struct_type, tc_struct, tc_thread, tc_mutex, 		/* 35 */
  tc_condv,								/* 40 */
  tc_last_standard /* must be last as indicated by its name */
} type_cell;


  /*
   * Internal representation of SCM object. Object use the two least
   * significant bit as tag. We have the following representation 
   *
   *     .........00		pointer on an object descriptor (a box)
   *     .........01		integer 
   *     .........10		small object (see below for more detail)
   *     .........11		small constant (#t #f '() ... see below for details)
   */

#define MAKE_SCONST(n)   (AS_SCM(n << 2 | 3))
#define SCONSTP(n)       ((AS_LONG(n) & 0x3) == 3)

  /* 
   * Header which must always be put in front of the various boxed types
   * used by STklos. This field must be declared as the first field of
   * the structure.
   */

typedef struct { 
  /* Order is important, changing it can improve the perfomances depending 
   * on the compiler. If you change this definition, change DEFINE_PRIMITIVE
   * accordingly 
   */
  short type, cell_info;
} stk_header;


#define BOXED_TYPE(x)	 	(((stk_header *) x)->type)
#define BOXED_INFO(x)	 	(((stk_header *) x)->cell_info)
#define BOXED_OBJP(x)		(!(AS_LONG(x) & 3))
#define BOXED_TYPE_EQ(x, y)	(BOXED_OBJP(x) && BOXED_TYPE(x) == y)

#define STYPE(x)		(BOXED_OBJP(x)? BOXED_TYPE(x): tc_not_boxed)


#define NEWCELL(_var, _type) 	{ 						\
 	_var = (SCM) STk_must_malloc(sizeof(struct CPP_CONCAT(_type,_obj)));	\
    	BOXED_TYPE(_var) = CPP_CONCAT(tc_, _type);				\
    	BOXED_INFO(_var) = 0;							\
	}

#define NEWCELL_WITH_LEN(_var, _type, _len)	{ 	\
 	_var = (SCM) STk_must_malloc(_len);		\
    	BOXED_TYPE(_var) = CPP_CONCAT(tc_, _type);	\
    	BOXED_INFO(_var) = 0;				\
	}

#define NEWCELL_ATOMIC(_var, _type, _len) 	{ 	\
 	_var = (SCM) STk_must_malloc_atomic(_len);	\
    	BOXED_TYPE(_var) = CPP_CONCAT(tc_, _type);	\
    	BOXED_INFO(_var) = 0;				\
	}

  /*
   * PRIMITIVES
   * 
   * Primitives are defined with the macro DEFINE_PRIMITIVE. An example of 
   * usage of this  macro is given below:
   *    DEFINE_PRIMITIVE("pair?", pairp, subr1, (SCM obj)) {
   *       <body>
   *    }
   * It will be expansed in
   *    SCM STk_pairp(SCM obj); 
   *    static struct obj_primitive obj_pairp = { "pair?", tc_subr1, STk_pairp};
   *	SCM STk_pairp(SCM obj){
   *	  <body>
   *    }
   */

struct primitive_obj {
  stk_header header;
  char *name;
  SCM (*code)();
  SCM plist;
};

#define PRIMITIVE_NAME(p)	(((struct primitive_obj *) (p))->name)	
#define PRIMITIVE_FUNC(p)	(((struct primitive_obj *) (p))->code)
#define PRIMITIVE_PLIST(p)	(((struct primitive_obj *) (p))->plist)

#define DEFINE_PRIMITIVE(_sname, _cname, _type, _params)	\
  SCM CPP_CONCAT(STk_, _cname) _params;				\
  struct primitive_obj CPP_CONCAT(STk_o_, _cname) = { 	\
	{CPP_CONCAT(tc_, _type), 0},				\
      	_sname, CPP_CONCAT(STk_, _cname), STk_nil};		\
  SCM CPP_CONCAT(STk_, _cname) _params

#define EXTERN_PRIMITIVE(_sname, _cname, _type, _params)	\
  /* the same one as before but without function definition */  \
  SCM CPP_CONCAT(STk_, _cname) _params;


#define ENTER_PRIMITIVE(x)     /* here for compability with pre 0.62 version */
#define ADD_PRIMITIVE(_name)   STk_add_primitive(CPP_CONCAT(&STk_o_, _name))
#define THE_PRIMITIVE(_name)   ((SCM) CPP_CONCAT(&STk_o_, _name))



/*
  ------------------------------------------------------------------------------
  ----
  ---- 				 B O O L E A N . C
  ----
  ------------------------------------------------------------------------------
*/

#define MAKE_BOOLEAN(_cond) 	((_cond) ? STk_true : STk_false)
#define BOOLEANP(o)		(((o) == STk_true) || ((o) == STk_false))


EXTERN_PRIMITIVE("eq?",    eq,    subr2, (SCM x,SCM y))
EXTERN_PRIMITIVE("eqv?",   eqv,   subr2, (SCM x,SCM y))
EXTERN_PRIMITIVE("equal?", equal, subr2, (SCM x,SCM y))


int STk_init_boolean(void);

/*
  ------------------------------------------------------------------------------
  ----
  ---- 				  C H A R  . C
  ----
  ------------------------------------------------------------------------------
*/

  /*
   * characters are coded as .....XXXXX110 where XXXXX is the code of the 
   * character. Consequently, we can have 29 bits long characters (on a 32 bits
   * machine)
   */

#define MAKE_CHARACTER(n) (AS_SCM((n) << 3 | 0x6))
#define CHARACTER_VAL(n)  ((unsigned char) (AS_LONG(n) >> 3))
#define CHARACTERP(n)	  ((AS_LONG(n) & 7) == 6)

char *STk_char2string(char c);
unsigned char STk_string2char(char *s);
int STk_init_char(void);


/*
  ------------------------------------------------------------------------------
  ----
  ---- 				 C O N D  . C
  ----
  ------------------------------------------------------------------------------
*/

extern SCM STk_message_condition, STk_err_mess_condition;

SCM STk_make_C_cond(SCM type, int nargs, ...);

EXTERN_PRIMITIVE("make-condition-type", make_cond_type, subr3, 
		 (SCM name, SCM parent, SCM slots));

SCM STk_defcond_type(char *name, SCM parent, SCM slots, SCM module);

int STk_init_cond(void);


/*
  ------------------------------------------------------------------------------
  ----
  ---- 				  D Y N L O A D . C
  ----
  ------------------------------------------------------------------------------
*/

#define MODULE_ENTRY_START(_name)  				\
    void STk_module_main(void)	{				\
      static int __already_loaded = 0;				\
      if (__already_loaded++) {					\
	STk_error("module %S already loaded", (_name));		\
	return;							\
      } {

#define MODULE_ENTRY_END	} }


SCM STk_load_object_file(SCM f, char *fname);


/*
  ------------------------------------------------------------------------------
  ----
  ---- 				  E R R O R . C
  ----
  ------------------------------------------------------------------------------
*/

void STk_signal_error(SCM who, SCM str);
void STk_error(char *format, ...);
SCM STk_format_error(char *format, ...);
void STk_warning(char *format, ...);
void STk_panic(char *format, ...);
void STk_signal(char *str);

#ifdef STK_DEBUG
   void STk_debug(char *format, ...);
#  define Debug  STk_debug

#endif

/*
  ------------------------------------------------------------------------------
  ----
  ---- 				  E N V . C
  ----
  ------------------------------------------------------------------------------
*/
struct frame_obj {		
  stk_header header;
  SCM next_frame;
  SCM owner;
  SCM locals[1];	/* the values associated to the names */
};

#define FRAME_LENGTH(p)		(BOXED_INFO(p))
#define FRAME_NEXT(p)		(((struct frame_obj *) (p))->next_frame)
#define FRAME_OWNER(p)		(((struct frame_obj *) (p))->owner)
#define FRAME_LOCALS(p)		(((struct frame_obj *) (p))->locals)
#define FRAME_LOCAL(p, i)	(FRAME_LOCALS(p)[i])
#define FRAMEP(p)		(BOXED_TYPE_EQ((p), tc_frame))

/* modules are defined in env.c but are private */
#define MODULEP(p)		(BOXED_TYPE_EQ((p), tc_module))

SCM STk_make_frame(int len);
SCM STk_clone_frame(SCM f);

SCM STk_lookup(SCM symbol, SCM env, SCM *ref, int err_if_unbound);
void STk_define_variable(SCM symbol, SCM value, SCM module);


int STk_init_env(void);
int STk_late_init_env(void); /* must be done after symbol initialization */

extern SCM STk_STklos_module;

EXTERN_PRIMITIVE("%create-module", create_module, subr1, (SCM name))
EXTERN_PRIMITIVE("current-module", current_module, subr0, (void))
EXTERN_PRIMITIVE("%select-module", select_module, subr1, (SCM module))

/*
  ------------------------------------------------------------------------------
  ----
  ---- 				E X T E N D . C
  ----
  ------------------------------------------------------------------------------
*/
  /* The `extended_type_descr' structure is used for the types which need
   *  more information (such as modules, ports, ....). All the extended 
   * descriptors are stored in the STk_xtypes array.
   */
struct extended_type_descr {
  char *name;
  void (*print)(SCM exp, SCM port, int mode);
};

extern struct extended_type_descr *STk_xtypes[];

#define BOXED_XTYPE(o)	     	    	 (STk_xtypes[((stk_header *) o)->type])
#define XTYPE_NAME(d)		    	 (d->name)
#define XTYPE_PRINT(d)		    	 (d->print)
#define DEFINE_XTYPE(_type, _descr) 	 (STk_xtypes[CPP_CONCAT(tc_, _type)]=_descr)
#define DEFINE_USER_TYPE(_type, _descr) {	\
   _type = STk_new_user_type();			\
   STk_xtypes[_type]=_descr;			\
}

int STk_new_user_type(void);
int STk_init_extend(void);

/*
  ------------------------------------------------------------------------------
  ----
  ---- 				 K E Y W O R D . C
  ----
  ------------------------------------------------------------------------------
*/
struct keyword_obj {
  stk_header header;
  char *pname;			/* must be at the same offset than for symbols */
};

#define KEYWORD_PNAME(p) 	(((struct keyword_obj *) (p))->pname)
#define KEYWORDP(p)		(BOXED_TYPE_EQ((p),tc_keyword))

#define KEYWORD_NEEDS_BARS 	(1 << 0)	/* Info flag */
#define KEYWORD_HAS_UPPER 	(1 << 1)	

EXTERN_PRIMITIVE("key-set!", key_set, subr3, (SCM l, SCM key, SCM val))
EXTERN_PRIMITIVE("key-get", key_get, subr23, (SCM l, SCM key, SCM dflt))

SCM STk_makekey(char *tok);
int STk_init_keyword(void);

/*
  ------------------------------------------------------------------------------
  ----
  ---- 					L I B . C
  ----
  ------------------------------------------------------------------------------
*/
extern int   STk_library_initialized; /* True when successfully initialized */
extern char *STk_library_path;	      /* The base directory where files are found */
extern void *STk_start_stack;	      /* An approx. of main thread stack addr */

  int STk_init_library(int *argc, char ***argv, int stack_size);

/*
  ------------------------------------------------------------------------------
  ----
  ---- 				      L I S T . C
  ----
  ------------------------------------------------------------------------------
*/

struct cons_obj {
  stk_header header;
  SCM car;
  SCM cdr;
};

#define CAR(p) 		(((struct cons_obj *) (p))->car)
#define CDR(p) 		(((struct cons_obj *) (p))->cdr)
#define CONSP(p)	(BOXED_TYPE_EQ((p), tc_cons))
#define NULLP(p)	((p) == STk_nil)

#define CONS_CONST		(1 << 0)
#define CONS_PLACEHOLDER	(1 << 1)	/* used for #n= and #n# notation */
#define CONS_ECONS		(1 << 2)	/* used for extended conses      */

#define LIST1(a)		 STk_cons((a), STk_nil)
#define LIST2(a,b) 		 STk_cons((a), LIST1(b))
#define LIST3(a,b,c)		 STk_cons((a), LIST2((b),(c)))
#define LIST4(a,b,c,d)		 STk_cons((a), LIST3((b),(c),(d)))
#define LIST5(a,b,c,d,e)	 STk_cons((a), LIST4((b),(c),(d),(e)))
#define LIST6(a,b,c,d,e,f)	 STk_cons((a), LIST5((b),(c),(d),(e),(f)))
#define LIST7(a,b,c,d,e,f,g)	 STk_cons((a), LIST6((b),(c),(d),(e),(f),(g)))
#define LIST8(a,b,c,d,e,f,g,h)	 STk_cons((a), LIST7((b),(c),(d),(e),(f),(g),(h)))
#define LIST9(a,b,c,d,e,f,g,h,i) \
  STk_cons((a), LIST8((b),(c),(d),(e),(f),(g),(h),(i)))
#define LIST10(a,b,c,d,e,f,g,h,i,j) \
  STk_cons((a), LIST9((b),(c),(d),(e),(f),(g),(h),(i),(j)))


int STk_int_length(SCM l); 		/* len of a list -1 if badly formed */
SCM STk_int_assq(SCM obj, SCM alist);   /* internal version of assq */
SCM STk_argv2list(int argc, SCM *argv);
SCM STk_append2(SCM l1, SCM l2);
SCM STk_dappend2(SCM l1, SCM l2);	/* destructive append */
SCM STk_dremq(SCM obj, SCM list);	/* destructive remove with eq? */
SCM STk_econs(SCM car, SCM cdr, char *file, int line, int pos);

EXTERN_PRIMITIVE("cons", cons, subr2, (SCM x, SCM y))
EXTERN_PRIMITIVE("car", car, subr1, (SCM x))
EXTERN_PRIMITIVE("cdr", cdr, subr1, (SCM x))
EXTERN_PRIMITIVE("list", list, vsubr, (int argc, SCM * argv))
EXTERN_PRIMITIVE("memq", memq, subr2, (SCM obj, SCM list))
EXTERN_PRIMITIVE("reverse", reverse, subr1, (SCM l))
EXTERN_PRIMITIVE("reverse!", dreverse, subr1, (SCM l))
EXTERN_PRIMITIVE("copy-tree", copy_tree, subr1, (SCM l))
EXTERN_PRIMITIVE("assq", assq, subr2, (SCM obj, SCM alist))
EXTERN_PRIMITIVE("assv", assv, subr2, (SCM obj, SCM alist))
EXTERN_PRIMITIVE("assoc", assoc, subr2, (SCM obj, SCM alist))


int STk_init_list(void);


/*
  ------------------------------------------------------------------------------
  ----
  ---- 					M I S C . C
  ----
  ------------------------------------------------------------------------------
*/

#ifdef STK_DEBUG
extern int STk_interactive_debug;
#endif

char *STk_strdup(const char *s);
void STk_add_primitive(struct primitive_obj *o);
SCM STk_eval_C_string(char *str, SCM module);

int STk_init_misc(void);


/*
  ------------------------------------------------------------------------------
  ----
  ---- 				     N U M B E R . C
  ----
  ------------------------------------------------------------------------------
*/
  /****
   **** Integer
   ****/
  /* As said before, integers are not allocated but have their two
   * least significant bits set to 01.
   */

#define INT_VAL(p)	(((long) p) >> 2)
#define INTP(p)		((((unsigned long) p) & 3) == 1)
#define SCM_LONG(n)	(((n) << 2) | 1)
#define MAKE_INT(n)	(AS_SCM(SCM_LONG(n)))
#define INT_MIN_VAL	((LONG_MIN & ~3) >> 2)
#define INT_MAX_VAL	((LONG_MAX & ~3) >> 2)

long STk_integer_value(SCM x); /* Returns LONG_MIN if not representable as long */
unsigned long STk_uinteger_value(SCM x); /* Returns ULONG_MAX if not an ulong */


  /****
   **** Real 
   ****/

#define REAL_FORMAT_SIZE         15 /* default format for real */

struct real_obj {
  stk_header header;
  double val;
};

#define REAL_VAL(p) 	(((struct real_obj *) (p))->val)
#define REALP(p) 	(BOXED_TYPE_EQ((p), tc_real))

extern double STk_NaN;		/* IEEE NaN special value */
			 
  /****
   **** Bignum
   ****/
struct bignum_obj; 	/* complete deflaration is in number.c */

#define BIGNUMP(p) 	(BOXED_TYPE_EQ((p), tc_bignum))

  /****
   **** Rational
   ****/
struct rational_obj {
  stk_header header;
  SCM num, den;
};

#define RATIONAL_NUM(p)		(((struct rational_obj *) (p))->num)
#define RATIONAL_DEN(p)		(((struct rational_obj *) (p))->den)
#define RATIONALP(p) 		(BOXED_TYPE_EQ((p), tc_rational))
#define EXACT_RATIONALP(p)	(RATIONALP(p) 		  && \
				 !REALP(RATIONAL_NUM(p))  && \
				 !REALP(RATIONAL_DEN(p)))

  /****
   **** Complex
   ****/
struct complex_obj {
  stk_header header;
  SCM real, imag;
};

#define COMPLEX_REAL(p)		(((struct complex_obj *) (p))->real)
#define COMPLEX_IMAG(p)		(((struct complex_obj *) (p))->imag)
#define COMPLEXP(p) 		(BOXED_TYPE_EQ((p), tc_complex))
#define EXACT_COMPLEXP(p)	(COMPLEXP(p) 		  && \
				 !REALP(COMPLEX_REAL(p))  && \
				 !REALP(COMPLEX_IMAG(p)))


  /**** 
   **** Conversions 
   ****/
SCM    		STk_Cstr2number(char *str, long base);
char  	       *STk_bignum2Cstring(SCM n, int base);
SCM    		STk_long2integer(long n);
SCM    		STk_ulong2integer(unsigned long n);
SCM    		STk_double2real(double d);
double 		STk_number2double(SCM n);
long   	      	STk_integer2int32(SCM n, int *overflow);
unsigned long 	STk_integer2uint32(SCM n, int *overflow);

  /****
   **** Arithmetic
   ****/
SCM STk_add2(SCM o1, SCM o2);
SCM STk_sub2(SCM o1, SCM o2);
SCM STk_mul2(SCM o1, SCM o2);
SCM STk_div2(SCM o1, SCM o2);

long STk_numeq2(SCM o1, SCM o2);
long STk_numdiff2(SCM o1, SCM o2);
long STk_numlt2(SCM o1, SCM o2);
long STk_numgt2(SCM o1, SCM o2);
long STk_numle2(SCM o1, SCM o2);
long STk_numge2(SCM o1, SCM o2);
void STk_double2Cstr(char *buffer, double n);
EXTERN_PRIMITIVE("number->string", number2string, subr12, (SCM n, SCM base));

int    STk_init_number(void);


#define NUMBERP(x)	(INTP(x) || BIGNUMP(x) || REALP(x) || RATIONALP(x) || \
		         COMPLEXP(x))
#define EXACTP(x)	(INTP(x) || BIGNUMP(x) || EXACT_RATIONALP(x) || \
			 EXACT_COMPLEXP(x))

/*
  ------------------------------------------------------------------------------
  ----
  ---- 				     O B J E C T . C
  ----
  ------------------------------------------------------------------------------
*/

int STk_init_object(void);


/*
  ------------------------------------------------------------------------------
  ----
  ---- 				   P A R A M E T E R . C
  ----
  ------------------------------------------------------------------------------
*/

int STk_init_parameter(void);

SCM STk_get_parameter(SCM param);
SCM STk_set_parameter(SCM param, SCM value);
SCM STk_make_C_parameter(SCM symbol, SCM value, SCM (*proc)(SCM new_value));
SCM STk_make_C_parameter2(SCM symbol,SCM (*value)(void),SCM (*proc)(SCM new_value));



/*
  ------------------------------------------------------------------------------
  ----
  ---- 					P A T H . C
  ----
  ------------------------------------------------------------------------------
*/
char *STk_expand_file_name(char *s);
SCM STk_do_glob(int argc, SCM *argv);
SCM STk_resolve_link(char *path, int count);


/*
  ------------------------------------------------------------------------------
  ----
  ---- 					P O R T . C
  ----
  ---- (and sio.c, fport.c, sport.c, vport.c)
  ----
  ------------------------------------------------------------------------------
*/

  /* Code for port is splitted in several files:
   * 	- sio.c contains the low level IO functions which mimic the C IO. All 
   *      these functions take Scheme ports as parameter instead of FILE *
   *	- fport.c contains the specific code for port associated to files
   *	- sport.c contains the specific code for port associated to string ports
   *	- vport.c contains the specific code for port associated to virtual ports
   *	- port.c contains the code which can be used on any kind of port 
   */

struct port_obj {
  stk_header header;
  void *stream;			/* stream descriptor != for strings, file, virt. */
  int  flags;			/* associated flags */
  int  ungetted_char;		/* character ungetted, EOF if none */
  char *filename;		/* File name (NULL if not a file port) */
  int  line;			/* Line number  (unused when writing) */
  int  pos;			/* position from the start of file */

  /* virtual functions (in the object 'cause the # of ports should be low ) */
  void  (*print_it)  (SCM obj, SCM port);  /* used to display or print object */
  void  (*release_it)(SCM obj);
  int   (*creadyp)   (void *stream); 
  int   (*cgetc)     (void *stream);
  int   (*ceofp)     (void *stream);
  int   (*cclose)    (void *stream);
  int   (*cputc)     (int c, void * stream);
  int   (*cputs)     (char *s, void * stream);
  int   (*cnputs)    (void *stream, char *str, int len);
  int   (*cputstring)(void *stream, SCM str);
  int   (*cflush)    (void *stream);
  int   (*read_buff) (void *stream, void *buf, int count);
  int   (*write_buff)(void *stream, void *buf, int count);
  off_t (*seek)	     (void *stream, off_t offset, int whence);
}; 

#define PORT_MAX_PRINTF	1024	/* max size for sprintf buffer */

#define PORT_READ		(1<<0)
#define PORT_WRITE		(1<<1)
#define PORT_RW			(1<<2)
#define PORT_CLOSED 		(1<<3)
#define PORT_IS_PIPE		(1<<4)
#define PORT_IS_FILE		(1<<5)
#define PORT_IS_STRING		(1<<6)
#define PORT_IS_VIRTUAL 	(1<<7)
#define PORT_IS_INTERACTIVE	(1<<8)


#define PORT_STREAM(x)	  (((struct port_obj *) (x))->stream)
#define PORT_FLAGS(x)     (((struct port_obj *) (x))->flags)
#define PORT_UNGETC(x)    (((struct port_obj *) (x))->ungetted_char)
#define PORT_LINE(x)	  (((struct port_obj *) (x))->line)
#define PORT_POS(x)	  (((struct port_obj *) (x))->pos)
#define PORT_FNAME(x)	  (((struct port_obj *) (x))->filename)

#define PORT_PRINT(x)     (((struct port_obj *) (x))->print_it)
#define PORT_RELEASE(x)   (((struct port_obj *) (x))->release_it)
#define PORT_READY(x)	  (((struct port_obj *) (x))->creadyp)
#define PORT_GETC(x)	  (((struct port_obj *) (x))->cgetc)
#define PORT_EOFP(x)	  (((struct port_obj *) (x))->ceofp)
#define PORT_CLOSE(x)	  (((struct port_obj *) (x))->cclose)
#define PORT_PUTC(x)	  (((struct port_obj *) (x))->cputc)
#define PORT_PUTS(x)	  (((struct port_obj *) (x))->cputs)
#define PORT_PUTSTRING(x) (((struct port_obj *) (x))->cputstring)
#define PORT_NPUTS(x)	  (((struct port_obj *) (x))->cnputs)
#define PORT_FLUSH(x)	  (((struct port_obj *) (x))->cflush)
#define PORT_BREAD(x)	  (((struct port_obj *) (x))->read_buff)
#define PORT_BWRITE(x)	  (((struct port_obj *) (x))->write_buff)
#define PORT_SEEK(x)	  (((struct port_obj *) (x))->seek)

#define PORTP(x)   (BOXED_TYPE_EQ((x), tc_port))
#define IPORTP(x)  (PORTP(x) && (PORT_FLAGS(x) & (PORT_READ|PORT_RW)))
#define OPORTP(x)  (PORTP(x) && (PORT_FLAGS(x) & (PORT_WRITE|PORT_RW)))

#define FPORTP(x)  (PORTP(x)  && (PORT_FLAGS(x) & (PORT_IS_FILE|PORT_IS_PIPE)))
#define IFPORTP(x) (FPORTP(x) && (PORT_FLAGS(x) & (PORT_READ|PORT_RW)))
#define OFPORTP(x) (FPORTP(x) && (PORT_FLAGS(x) & (PORT_WRITE|PORT_RW)))

#define SPORTP(x)  (PORTP(x)  && (PORT_FLAGS(x) & PORT_IS_STRING))
#define ISPORTP(x) (SPORTP(x) && (PORT_FLAGS(x) & (PORT_READ|PORT_RW)))
#define OSPORTP(x) (SPORTP(x) && (PORT_FLAGS(x) & (PORT_WRITE|PORT_RW)))

#define VPORTP(x)  (PORTP(x)  && (PORT_FLAGS(x) & PORT_IS_VIRTUAL))
#define IVPORTP(x) (VPORTP(x) && (PORT_FLAGS(x) & (PORT_READ|PORT_RW)))
#define OVPORTP(x) (VPORTP(x) && (PORT_FLAGS(x) & (PORT_WRITE|PORT_RW)))




#define PORT_IS_CLOSEDP(x)	(PORT_FLAGS(x) & PORT_CLOSED)

/****
 **** 		sio.h primitives
 ****/

int STk_readyp(SCM port);
int STk_getc(SCM port);
int STk_ungetc(int c, SCM port);
int STk_close(SCM port);
int STk_putc(int c, SCM port);
int STk_puts(char *s, SCM port);
int STk_putstring(SCM s, SCM port);
int STk_nputs(SCM port, char *s, int len);
off_t STk_seek(SCM port, off_t offset, int whence);
off_t STk_tell(SCM port);
void STk_rewind(SCM port);
int STk_flush(SCM port);
int STk_feof(SCM port);
int STk_fprintf(SCM port, char *format, ...);
int STk_read_buffer(SCM port, void *buff, int count);
int STk_write_buffer(SCM port, void *buff, int count);


/****
 ****		fport.h primitives
 ****/
SCM STk_rewind_file_port(SCM port);
SCM STk_open_file(char *filename, char *mode);
SCM STk_add_port_idle(SCM port, SCM idle_func);
SCM STk_fd2scheme_port(int fd, char *mode, char *identification);
void STk_set_line_buffered_mode(SCM port);
int STk_init_fport(void);
SCM STk_current_input_port(void);
SCM STk_current_output_port(void);
SCM STk_current_error_port(void);


/****
 ****		sport.h primitives
 ****/
EXTERN_PRIMITIVE("open-output-string", open_output_string, subr0, (void))
SCM STk_get_output_string(SCM port);
SCM STk_open_C_string(char *str);
int STk_init_sport(void);

/****
 ****		vport.h primitives
 ****/
int STk_init_vport(void);


/****
 **** 		port.h primitives
 ****/
EXTERN_PRIMITIVE("close-port", close_port, subr1, (SCM port))

void STk_error_bad_port(SCM p);
void STk_error_bad_file_name(SCM f);
void STk_error_cannot_load(SCM f);
void STk_error_bad_io_param(char *fmt, SCM p);
void STk_error_file_name(char *fmt, SCM fn);

int STk_init_port(void);



/**** 
 **** 		Port global variables 
 ****/

extern char *STk_current_filename;		 /* Name of the file we read */ 

extern SCM STk_stdin, STk_stdout, STk_stderr;		  /* unredirected ports   */
extern int STk_interactive;			/* We are in intearctive mode */

/*
  ------------------------------------------------------------------------------
  ----
  ---- 				  P R I N T . C
  ----
  ------------------------------------------------------------------------------
*/
void STk_print(SCM exp, SCM port, int mode);
void STk_print_star(SCM exp, SCM port);

#define DSP_MODE		0
#define WRT_MODE		1


/*
  ------------------------------------------------------------------------------
  ----
  ---- 				P R O C E S S . C
  ----
  ------------------------------------------------------------------------------
*/
int STk_init_process(void);


/*
  ------------------------------------------------------------------------------
  ----
  ---- 				P R O M I S E . C
  ----
  ------------------------------------------------------------------------------
*/
int STk_init_promise(void);


/*
  ------------------------------------------------------------------------------
  ----
  ---- 					P R O C . C
  ----
  ------------------------------------------------------------------------------
*/

typedef short STk_instr;

struct closure_obj {
  stk_header header;
  short arity;
  unsigned short code_size;
  /*  SCM formals; */
  SCM env;
  SCM plist;
  SCM name;
  SCM* constants;
  STk_instr *bcode;
};

/* FIXME:
#define CLOSURE_FORMALS(p)	(((struct closure_obj *) (p))->formals) 
#define CLOSURE_CODE(p)   	(((struct closure_obj *) (p))->code)
*/

#define CLOSURE_ARITY(p)	(((struct closure_obj *) (p))->arity)
#define CLOSURE_SIZE(p)		(((struct closure_obj *) (p))->code_size)
#define CLOSURE_ENV(p)		(((struct closure_obj *) (p))->env)
#define CLOSURE_PLIST(p)	(((struct closure_obj *) (p))->plist)
#define CLOSURE_NAME(p)		(((struct closure_obj *) (p))->name)
#define CLOSURE_CONST(p)	(((struct closure_obj *) (p))->constants)
#define CLOSURE_BCODE(p)	(((struct closure_obj *) (p))->bcode)
#define CLOSUREP(p)		(BOXED_TYPE_EQ((p), tc_closure))

EXTERN_PRIMITIVE("procedure?", procedurep, subr1, (SCM obj))
EXTERN_PRIMITIVE("%procedure-arity", proc_arity, subr1, (SCM proc))


SCM STk_make_closure(STk_instr *code, int size, int arity, SCM *cst, SCM env);
int STk_init_proc(void);


/*
  ------------------------------------------------------------------------------
  ----
  ---- 				  R E A D . C
  ----
  ------------------------------------------------------------------------------
*/
SCM   STk_read(SCM port, int case_significant);
SCM   STk_read_constant(SCM port, int case_significant);
char *STk_quote2str(SCM symb);
int   STk_init_reader(void);
extern SCM STk_sym_quote;		/* The interned value of symbol "quote" */
extern int STk_read_case_sensitive;


/*
  ------------------------------------------------------------------------------
  ----
  ---- 				  R E G E X P . C
  ----
  ------------------------------------------------------------------------------
*/
int STk_init_regexp(void);
EXTERN_PRIMITIVE("string->regexp", str2regexp, subr1, (SCM re));
EXTERN_PRIMITIVE("regexp-match", regexec, subr2, (SCM re, SCM str));


/*
  ------------------------------------------------------------------------------
  ----
  ---- 				S I G N A L  . C
  ----
  ------------------------------------------------------------------------------
*/
int STk_get_signal_value(SCM sig);
int STk_init_signal(void);

/*
  ------------------------------------------------------------------------------
  ----
  ---- 				S O C K E T . C
  ----
  ------------------------------------------------------------------------------
*/
int STk_init_socket(void);


/*
  ------------------------------------------------------------------------------
  ----
  ---- 				  S T R  . C
  ----
  ------------------------------------------------------------------------------
*/
struct string_obj {
  stk_header header;
  int size;
  char chars[1];   /* will be sized to a different value when allocated */
};

#define STRING_SIZE(p)   (((struct string_obj *) (p))->size)
#define STRING_CHARS(p)	 (((struct string_obj *) (p))->chars)
#define STRINGP(p)	 (BOXED_TYPE_EQ((p), tc_string))

#define STRING_CONST	 (1 << 0)

#define BOX_CSTRING(s)	 STk_makestring(strlen(s), (s))


SCM STk_makestring(int len, char *init);
SCM STk_Cstring2string(char *str); 	     /* Embed a C string in Scheme world  */
SCM STk_chars2string(char *str, size_t len); /* Original can have null characters */


EXTERN_PRIMITIVE("string=?", streq, subr2, (SCM s1, SCM s2))
EXTERN_PRIMITIVE("string-ref", string_ref, subr2, (SCM str, SCM index))
EXTERN_PRIMITIVE("string-set!", string_set, subr3, (SCM str, SCM index, SCM value))
EXTERN_PRIMITIVE("string-downcase!", string_ddowncase, vsubr, (int argc, SCM *argv))
int STk_init_string(void);

/*
  ------------------------------------------------------------------------------
  ----
  ---- 				 S T R U C T  . C
  ----
  ------------------------------------------------------------------------------
*/
int STk_init_struct(void);


/*
  ------------------------------------------------------------------------------
  ----
  ---- 				 S Y M B O L . C
  ----
  ------------------------------------------------------------------------------
*/
struct symbol_obj {
  stk_header header;	/* must be at the same offset than for keywords */
  char *pname;
};

#define SYMBOL_PNAME(p) (((struct symbol_obj *) (p))->pname)
#define SYMBOLP(p)	(BOXED_TYPE_EQ((p),tc_symbol))

#define SYMBOL_NEEDS_BARS 	(1 << 0)	/* Info flag */
#define SYMBOL_HAS_UPPER	(1 << 1)

EXTERN_PRIMITIVE("string->symbol", string2symbol, subr1, (SCM string))

int STk_symbol_flags(register char *s);
SCM STk_intern(char *name);
SCM STk_intern_ci(char *name);
SCM STk_make_uninterned_symbol(char *name);
int STk_init_symbol(void);


/*
  ------------------------------------------------------------------------------
  ----
  ---- 				 S Y S T E M . C
  ----
  ------------------------------------------------------------------------------
*/

int STk_dirp(const char *path);
int STk_init_system();
EXTERN_PRIMITIVE("exit", quit, subr01, (SCM retcode));

/*
  ------------------------------------------------------------------------------
  ----
  ---- 				 T H R E A D . C
  ----
  ------------------------------------------------------------------------------
*/
EXTERN_PRIMITIVE("current-thread", current_thread, subr0, (void))


/*
  ------------------------------------------------------------------------------
  ----
  ---- 				 U V E C T O R . C
  ----
  ------------------------------------------------------------------------------
*/
extern int STk_uvectors_allowed;

int STk_uniform_vector_tag(char *s);
SCM STk_list2uvector(int type, SCM l);
int STk_init_uniform_vector(void);
/*
  ------------------------------------------------------------------------------
  ----
  ---- 				 V E C T O R . C
  ----
  ------------------------------------------------------------------------------
*/

struct vector_obj {
  stk_header header;
  int size;
  SCM data[1];
};

#define VECTOR_SIZE(p)	(((struct vector_obj *) (p))->size)
#define VECTOR_DATA(p) 	(((struct vector_obj *) (p))->data)
#define VECTORP(p)	(BOXED_TYPE_EQ((p), tc_vector))

#define VECTOR_CONST	 (1 << 0)

EXTERN_PRIMITIVE("vector-ref", vector_ref, subr2, (SCM v, SCM index))
EXTERN_PRIMITIVE("vector-set!", vector_set, subr3, (SCM v, SCM index, SCM value))
EXTERN_PRIMITIVE("vector->list", vector2list, subr1, (SCM v))
EXTERN_PRIMITIVE("list->vector", list2vector, subr1, (SCM l));

SCM STk_makevect(int len, SCM init);
int STk_init_vector(void);

/*
  ------------------------------------------------------------------------------
  ----
  ---- 				 V M . C
  ----
  ------------------------------------------------------------------------------
*/
#define DEFAULT_STACK_SIZE 50000

void STk_execute_current_handler(SCM kind, SCM location, SCM message);
void STk_raise_exception(SCM cond);
SCM STk_C_apply(SCM func, int nargs, ...);
SCM STk_C_apply_list(SCM func, SCM l);
void STk_get_stack_pointer(void **addr);
SCM STk_n_values(int n, ...);

EXTERN_PRIMITIVE("%vm-backtrace", vm_bt, subr0, (void));

SCM STk_load_bcode_file(SCM f);
int STk_load_boot(char *s);
int STk_boot_from_C(void);

int STk_init_vm();

#ifdef THREADS_LURC
typedef int (*STk_main_t)(int, char**);
int STk_thread_main(STk_main_t themain, int argc, char **argv);

SCM *STk_save_vm(void);
void STk_restore_vm(SCM *sp);
#endif /* ! THREADS_LURC */

/*****************************************************************************/

extern char *STk_boot_consts;
extern STk_instr STk_boot_code[];



/* Special constants */

#define STk_nil		((SCM) MAKE_SCONST(0))
#define STk_false 	((SCM) MAKE_SCONST(1))
#define STk_true	((SCM) MAKE_SCONST(2))
#define STk_eof   	((SCM) MAKE_SCONST(3))
#define STk_void 	((SCM) MAKE_SCONST(4))

#define STk_dot		((SCM) MAKE_SCONST(5)) /* special pupose value see read.c */
#define STk_close_par	((SCM) MAKE_SCONST(6)) /* special pupose value see read.c */


/* Misc */

#ifdef __cplusplus
}
#endif


#endif /* STKLOS_H */
