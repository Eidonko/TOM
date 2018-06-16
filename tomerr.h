/*************************************************
 *
 *  TOM - a time-out manager
 *
 *  By Eidon (Eidon@tutanota.com)
 *
 *  V.1.1, Sat, Jun 16, 2018  4:10:58 PM
 *        (added TOM_ALREADY_PRESENT_ENTRY)
 *
 *  Module name: tomerr.h
 *  Contents: symbolic constants for errors and actions.
 *
 *************************************************/
#ifndef _TOM_ERR_H_
#define _TOM_ERR_H_

#define TOM_WRONG_SIZE              -1
#define TOM_NULL_PTRS               -2
#define TOM_REFUSED                 -3
#define TOM_ENTRY_NOT_FOUND         -4
#define TOM_EMPTY_LIST              -5
#define TOM_ALREADY_PRESENT_ENTRY   -6

#define TOM_INSERT                   1
#define TOM_DELETE                   2
#define TOM_SUSPEND                  3
#define TOM_ENABLE                   4
#define TOM_RENEW                    5
#define TOM_CLOSE                    6
#define TOM_EXIT                     7
#endif
