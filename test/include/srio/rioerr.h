#ifndef __RIO_ERR_H
#define __RIO_ERR_H

#include <stdtypes.h>
#include <errors.h>

/* Error message table entry */
typedef struct
{
        INT32 srioErrorCode;
        UINT32 srioTestErrorCode;
} SRIO_ERR_CODES;

#define EPERM        1  /* Operation not permitted */
#define ENOENT       2  /* No such file or directory */
#define ESRCH        3  /* No such process */
#define EINTR        4  /* Interrupted system call */
#define EIO          5  /* I/O error */
#define ENXIO        6  /* No such device or address */
#define E2BIG        7  /* Argument list too long */
#define ENOEXEC      8  /* Exec format error */
#define EBADF        9  /* Bad file number */
#define ECHILD      10  /* No child processes */
#define EAGAIN      11  /* Try again */
#define ENOMEM      12  /* Out of memory */
#define EACCES      13  /* Permission denied */
#define EFAULT      14  /* Bad address */
#define ENOTBLK     15  /* Block device required */
#define EBUSY       16  /* Device or resource busy */
#define EEXIST      17  /* File exists */
#define EXDEV       18  /* Cross-device link */
#define ENODEV      19  /* No such device */
#define ENOTDIR     20  /* Not a directory */
#define EISDIR      21  /* Is a directory */
#define EINVAL      22  /* Invalid argument */
#define ENFILE      23  /* File table overflow */
#define EMFILE      24  /* Too many open files */
#define ENOTTY      25  /* Not a typewriter */
#define ETXTBSY     26  /* Text file busy */
#define EFBIG       27  /* File too large */
#define ENOSPC      28  /* No space left on device */
#define ESPIPE      29  /* Illegal seek */
#define EROFS       30  /* Read-only file system */
#define EMLINK      31  /* Too many links */
#define EPIPE       32  /* Broken pipe */
#define EDOM        33  /* Math argument out of domain of func */
#define ERANGE      34  /* Math result not representable */

#define E__SRIO_FAIL       0x80000000
#define E__SRIO_OK          0
#define E__SRIO_EPERM      (E__SRIO_FAIL+1)
#define E__SRIO_ENOENT     (E__SRIO_FAIL+2)
#define E__SRIO_ESRCH      (E__SRIO_FAIL+3)
#define E__SRIO_EINTR      (E__SRIO_FAIL+4)
#define E__SRIO_EIO        (E__SRIO_FAIL+5)
#define E__SRIO_ENXIO      (E__SRIO_FAIL+6)
#define E__SRIO_E2BIG      (E__SRIO_FAIL+7)
#define E__SRIO_ENOEXEC    (E__SRIO_FAIL+8)
#define E__SRIO_EBADF      (E__SRIO_FAIL+9)
#define E__SRIO_ECHILD     (E__SRIO_FAIL+10)
#define E__SRIO_EAGAIN     (E__SRIO_FAIL+11)
#define E__SRIO_ENOMEM     (E__SRIO_FAIL+12)
#define E__SRIO_EACCES     (E__SRIO_FAIL+13)
#define E__SRIO_EFAULT     (E__SRIO_FAIL+14)
#define E__SRIO_ENOTBLK    (E__SRIO_FAIL+15)
#define E__SRIO_EBUSY      (E__SRIO_FAIL+16)
#define E__SRIO_EEXIST     (E__SRIO_FAIL+17)
#define E__SRIO_EXDEV      (E__SRIO_FAIL+18)
#define E__SRIO_ENODEV     (E__SRIO_FAIL+19)
#define E__SRIO_ENOTDIR    (E__SRIO_FAIL+20)
#define E__SRIO_EISDIR     (E__SRIO_FAIL+21)
#define E__SRIO_EINVAL     (E__SRIO_FAIL+22)
#define E__SRIO_ENFILE     (E__SRIO_FAIL+23)
#define E__SRIO_EMFILE     (E__SRIO_FAIL+24)
#define E__SRIO_ENOTTY     (E__SRIO_FAIL+25)
#define E__SRIO_ETXTBSY    (E__SRIO_FAIL+26)
#define E__SRIO_EFBIG      (E__SRIO_FAIL+27)
#define E__SRIO_ENOSPC     (E__SRIO_FAIL+28)
#define E__SRIO_ESPIPE     (E__SRIO_FAIL+29)
#define E__SRIO_EROFS      (E__SRIO_FAIL+30)
#define E__SRIO_EMLINK     (E__SRIO_FAIL+31)
#define E__SRIO_EPIPE      (E__SRIO_FAIL+32)
#define E__SRIO_EDOM       (E__SRIO_FAIL+33)
#define E__SRIO_ERANGE     (E__SRIO_FAIL+34)

#define E__SRIO_EREG_ACCESS   (E__SRIO_FAIL+35)
#define E__SRIO_ESPEED        (E__SRIO_FAIL+36)
#define E__SRIO_ELINK         (E__SRIO_FAIL+37)
#define E__SRIO_EOUTB_MAP     (E__SRIO_FAIL+38)
#define E__SRIO_ELPBACK_DATA  (E__SRIO_FAIL+39) 
#define E__SRIO_CS_CRC        (E__SRIO_FAIL+40) 
#define E__SRIO_CS_ILL_ID     (E__SRIO_FAIL+41) 
#define E__SRIO_PKT_CRC       (E__SRIO_FAIL+42) 
#define E__SRIO_PKT_ILL_ACKID (E__SRIO_FAIL+43) 
#define E__SRIO_PKT_ILL_SIZE  (E__SRIO_FAIL+44) 
#define E__SRIO_PROT_ERR      (E__SRIO_FAIL+45) 
#define E__SRIO_DELIN_ERR     (E__SRIO_FAIL+46) 
#define E__SRIO_CS_ACK_ILL    (E__SRIO_FAIL+47) 

static const SRIO_ERR_CODES SrioErrCodes[] =
{
  { (E__OK),     E__SRIO_OK            },
  { (-EPERM),    E__SRIO_EPERM         },
  { (-ENOENT),   E__SRIO_ENOENT        },      
  { (-ESRCH),    E__SRIO_ESRCH         },        
  { (-EINTR),    E__SRIO_EINTR         },
  { (-EIO),      E__SRIO_EIO           },
  { (-ENXIO),    E__SRIO_ENXIO         },
  { (-E2BIG),    E__SRIO_E2BIG         },       
  { (-ENOEXEC),  E__SRIO_ENOEXEC       },      
  { (-EBADF),    E__SRIO_EBADF         },       
  { (-ECHILD),   E__SRIO_ECHILD        },      
  { (-EAGAIN),   E__SRIO_EAGAIN        },      
  { (-ENOMEM),   E__SRIO_ENOMEM        },      
  { (-EACCES),   E__SRIO_EACCES        },      
  { (-EFAULT),   E__SRIO_EFAULT        }, 
  { (-ENOTBLK),  E__SRIO_ENOTBLK       },     
  { (-EBUSY),    E__SRIO_EBUSY         },       
  { (-EEXIST),   E__SRIO_EEXIST        },      
  { (-EXDEV),    E__SRIO_EXDEV         },       
  { (-ENODEV),   E__SRIO_ENODEV        },      
  { (-ENOTDIR),  E__SRIO_ENOTDIR       },     
  { (-EISDIR),   E__SRIO_EISDIR        },      
  { (-EINVAL),   E__SRIO_EINVAL        },      
  { (-ENFILE),   E__SRIO_ENFILE        },      
  { (-EMFILE),   E__SRIO_EMFILE        },      
  { (-ENOTTY),   E__SRIO_ENOTTY        },      
  { (-ETXTBSY),  E__SRIO_ETXTBSY       },     
  { (-EFBIG),    E__SRIO_EFBIG         },       
  { (-ENOSPC),   E__SRIO_ENOSPC        },      
  { (-ESPIPE),   E__SRIO_ESPIPE        },      
  { (-EROFS),    E__SRIO_EROFS         },       
  { (-EMLINK),   E__SRIO_EMLINK        },      
  { (-EPIPE),    E__SRIO_EPIPE         },       
  { (-EDOM),     E__SRIO_EDOM          },        
  { (-ERANGE),   E__SRIO_ERANGE        },      
  { (0xFFFFFFFF), 0xFFFFFFFF},
};

#endif
