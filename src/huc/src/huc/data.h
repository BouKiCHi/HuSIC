/*	File data.h: 2.2 (84/11/27,16:26:11) */


#include "defs.h"

/* constant arrays storage */

extern struct const_array *const_ptr;
extern struct const_array  const_var[MAX_CONST];
extern INTPTR_T  const_val[MAX_CONST_VALUE];
extern char const_data[MAX_CONST_DATA];
extern int  const_val_start;
extern int  const_val_idx;
extern int  const_data_start;
extern int  const_data_idx;
extern int  const_size;
extern int  const_nb;

/* storage words */

extern	char  symtab[];
extern	char *glbptr, *rglbptr, *locptr;
extern	INTPTR_T   ws[];
extern	INTPTR_T  *wsptr;
extern	INTPTR_T   swstcase[];
extern	INTPTR_T   swstlab[];
extern	INTPTR_T   swstp;
extern	char  litq[];
extern	char  litq2[];
extern	INTPTR_T   litptr;
extern	char  macq[];
extern	INTPTR_T   macptr;
extern	char  line[];
extern	char  mline[];
extern	INTPTR_T   lptr, mptr;

extern  char  asmdefs[];

/* miscellaneous storage */

extern	int	nxtlab,
		litlab,
		stkp,
		zpstkp,
		argstk,
		ncmp,
		errcnt,
		glbflag,
		indflg,
		ctext,
		cmode,
		lastst,
    overlayflag,
    optimize,
		globals_h_in_process;

extern	FILE	*input, *input2, *output;
extern	FILE	*inclstk[];

extern char    inclstk_name[INCLSIZ][FILENAMESIZE];
extern char    fname_copy[FILENAMESIZE];
extern int     inclstk_line[];
extern int     line_number;

extern	int		inclsp;
extern	char	fname[];

extern	char	quote[];
extern	char	*cptr;
extern	int	*iptr;
extern	int	fexitlab;
extern	int	iflevel, skiplevel;
extern	int	errfile;
extern	int	sflag;
extern	int	cdflag;
extern	int	verboseflag;
extern	int	startup_incl;
extern	int	errs;
