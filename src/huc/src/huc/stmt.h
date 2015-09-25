/*	File stmt.c: 2.1 (83/03/20,16:02:17) */
/*% cc -O -c %
 *
 */


#ifndef _STMT_H
#define _STMT_H

int statement (INTPTR_T func);
int stdecl (void);
int doldcls(int stclass);
void stst (void );
void compound (INTPTR_T func);
void doif (void );
void dowhile (void );
void dodo (void );
void dofor (void );
void doswitch (void );
void docase (void );
void dodefault (void );
void doreturn (void );
void dobreak (void );
void docont (void );
void dumpsw (INTPTR_T *ws);
void test (INTPTR_T label, int ft);

#endif

