/*
 * created at 2022-08-08 12:04.
 *
 * Company HashedIn By Deloitte
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#ifndef __logger_H__
#define __logger_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <inttypes.h>
#include <pthread.h>

/* Supported colors */
#define LOGGER_COLOR_NORMAL "\x1B[0m"
#define LOGGER_COLOR_RED "\x1B[31m"
#define LOGGER_COLOR_GREEN "\x1B[32m"
#define LOGGER_COLOR_YELLOW "\x1B[33m"
#define LOGGER_COLOR_BLUE "\x1B[34m"
#define LOGGER_COLOR_MAGENTA "\x1B[35m"
#define LOGGERlogger_COLOR_CYAN "\x1B[36m"
#define LOGGER_COLOR_WHITE "\x1B[37m"
#define LOGGER_COLOR_RESET "\033[0m"

/* Trace source location helpers */
#define LOGGER_TRACE_LVL1(LINE) #LINE
#define LOGGER_TRACE_LVL2(LINE) LOGGER_TRACE_LVL1(LINE)
#define LOGGER_THROW_LOCATION "[" __FILE__ ":" LOGGER_TRACE_LVL2(__LINE__) "] "

/* logger limits (To be safe while avoiding dynamic allocations) */
#define LOGGER_MESSAGE_MAX 8196
#define LOGGER_VERSION_MAX 128
#define LOGGER_PATH_MAX 2048
#define LOGGER_INFO_MAX 512
#define LOGGER_NAME_MAX 256
#define LOGGER_DATE_MAX 64
#define LOGGER_TAG_MAX 32
#define LOGGER_COLOR_MAX 16

#define LOGGER_FLAGS_CHECK(c, f) (((c) & (f)) == (f))
#define LOGGER_FLAGS_ALL 255

#define LOGGER_NAME_DEFAULT "logger_RPI"
#define LOGGER_NEWLINE "\n"
#define LOGGER_INDENT "       "
#define LOGGER_SPACE " "
#define LOGGER_EMPTY ""
#define LOGGER_NUL '\0'

#define MAIN_LOG_MODULE_ID 1
#define GPS_LOG_MODULE_ID 2
#define SERIAL_LOG_MODULE_ID 3
#define CAN_LOG_MODULE_ID 4
#define CC_LOG_MODULE_ID 5
#define CLOUD_LOG_MODULE_ID 6
#define DB_LOG_MODULE_ID 7
#define BLE_CAN_MODULE_ID 8

    typedef struct loggerDate
    {
        uint16_t nYear;
        uint8_t nMonth;
        uint8_t nDay;
        uint8_t nHour;
        uint8_t nMin;
        uint8_t nSec;
        uint8_t nUsec;
    } logger_date_t;

    uint8_t logger_get_usec(void);
    void logger_get_date(logger_date_t *pDate);

    /* Log level flags */
    typedef enum
    {
        LOGGER_INFO = (1 << 2),
        LOGGER_WARN = (1 << 3),
        LOGGER_DEBUG = (1 << 4),
        LOGGER_ERROR = (1 << 6)
    } logger_flag_t;

    typedef int (*logger_cb_t)(const char *pLog, size_t nLength, logger_flag_t eFlag, void *pCtx);

    /* Output coloring control flags */
    typedef enum
    {
        LOGGER_COLORING_DISABLE = 0,
        LOGGER_COLORING_TAG,
        LOGGER_COLORING_FULL
    } logger_coloring_t;

    typedef enum
    {
        LOGGER_TIME_DISABLE = 0,
        LOGGER_TIME_ONLY,
        LOGGER_DATE_FULL
    } logger_date_ctrl_t;

#define logger_info(int, ...) \
    logger_display(LOGGER_INFO, 1, int, __VA_ARGS__)

#define logger_warn(int, ...) \
    logger_display(LOGGER_WARN, 1, int, __VA_ARGS__)

#define logger_debug(int, ...) \
    logger_display(LOGGER_DEBUG, 1, int, __VA_ARGS__)

#define logger_error(int, ...) \
    logger_display(LOGGER_ERROR, 1, int, __VA_ARGS__)

    /* Short name definitions */
#define loggeri(...) logger_info(__VA_ARGS__)
#define loggerw(...) logger_warn(__VA_ARGS__)
#define loggerd(...) logger_debug(__VA_ARGS__)
#define loggere(...) logger_error(__VA_ARGS__)

    typedef struct loggerConfig
    {
        logger_date_ctrl_t eDateControl; // Display output with date format
        logger_coloring_t eColorFormat;  // Output color format control
        logger_cb_t logCallback;         // Log callback to collect logs
        void *pCallbackCtx;              // Data pointer passed to log callback

        uint8_t nTraceTid;   // Trace thread ID and display in output
        uint8_t nToScreen;   // Enable screen logging
        uint8_t nUseHeap;    // Use dynamic allocation
        uint8_t nToFile;     // Enable file logging
        uint8_t nIndent;     // Enable indentations
        uint8_t nFlush;      // Flush stdout after screen log
        uint16_t nFlags;     // Allowed log level flags
        uint16_t log_module; // Filter to select module wise logs

        char sSeparator[LOGGER_NAME_MAX]; // Separator between info and log
        char sFileName[LOGGER_NAME_MAX];  // Output file name for logs
        char sFilePath[LOGGER_PATH_MAX];  // Output file path for logs
    } logger_config_t;

    size_t logger_version(char *pDest, size_t nSize, uint8_t nMin);
    void logger_config_get(logger_config_t *pCfg);
    void logger_config_set(logger_config_t *pCfg);

    void logger_separator_set(const char *pFormat, ...);
    void logger_callback_set(logger_cb_t callback, void *pContext);
    void logger_indent(uint8_t nEnable);

    void logger_enable(logger_flag_t eFlag);
    void logger_disable(logger_flag_t eFlag);

    void logger_init(const char *pName, uint16_t nFlags, uint8_t nTdSafe);
    void logger_display(logger_flag_t eFlag, uint8_t nNewLine, int inModuleLog, const char *pFormat, ...);
    void logger_destroy(void); // Needed only if the logger_init() function argument nTdSafe > 0

    void logger_setup(void);

#ifdef __cplusplus
}
#endif

#endif /* __logger_H__ */