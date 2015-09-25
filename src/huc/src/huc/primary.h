/*	File primary.c: 2.4 (84/11/27,16:26:07) */
/*% cc -O -c %
 *
 */

#ifndef _PRIMARY_H
#define _PRIMARY_H

INTPTR_T primary (INTPTR_T* lval);
INTPTR_T dbltest (INTPTR_T *val1,INTPTR_T *val2);
void result (INTPTR_T *lval,INTPTR_T *lval2);
INTPTR_T constant (INTPTR_T *val);
INTPTR_T number (INTPTR_T val[]);
INTPTR_T pstr (INTPTR_T val[]);
INTPTR_T qstr (INTPTR_T val[]);
int readqstr (void );
int readstr (void );
int spechar(void );

#endif

