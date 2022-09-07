#include "global.h"

/* initialize global variable for logger */

/* module_flag is used to filter module level loggers using command line argument.
 * Example: telematic -m 6
 * This enables logger for the cloud write thread
 */
int module_flag = 1;

/* write_to_file is used to store all logs in a file using command line argument.
 * Example: telematic -f 1
 */
int write_to_file = 0;