 /********************************************************************************
  *                                                                              *
  *  Header file timeouts.h                                                      *
  *                                                                              *
  *  This file contains a set of timeout values for the EFTOS DIR Net            *
  *  By Eidon (Eidon@tutanota.com)                                               *
  *                                                                              *
  ********************************************************************************/

#ifndef _T_M_O__H_
#define _T_M_O__H_



/* Number of available nodes
 */
#define MAX_PROCS                 4


/* maximum period between two clear(IA-flag)'s
 */
#define IMALIVE_CLEAR_TIMEOUT     600000


/* maximum period between sending two MIA's (manager-to-backups)
 */
#define MIA_SEND_TIMEOUT          800000


/* maximum period between sending two TAIA's (backup-to-manager)
 */
#define TAIA_SEND_TIMEOUT         1200000



/* maximum suspicion period
 */
#define IMALIVE_SET_TIMEOUT       1400000


/* maximum period between two receiving two MIA's (manager-to-backups)
 */
#define MIA_RECV_TIMEOUT          1500000


/* maximum period between two receiving two TAIA's (backup-to-manager)
 */
#define TAIA_RECV_TIMEOUT         1500000


/* maximum time to send a REQUEST_DB message
 */
#define REQUEST_DB_TIMEOUT        2000000


/* maximum delay for a response to a REPLY_DB message
 */
#define REPLY_DB_TIMEOUT          4000000

#endif /* _T_M_O__H_ */
