/*
 * created at 2022-08-08 12:04.
 *
 * Company HashedIn By Deloitte
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include <time.h>
#include "logger.h"

//#include <syscall.h>
#include <sys/time.h>

extern int module_flag;

#ifndef PTHREAD_MUTEX_RECURSIVE
#define PTHREAD_MUTEX_RECURSIVE PTHREAD_MUTEX_RECURSIVE_NP
#endif

typedef struct logger
{
    unsigned int nTdSafe : 1;
    pthread_mutex_t mutex;
    logger_config_t config;
} logger_t;

typedef struct XLogCtx
{
    const char *pFormat;
    logger_flag_t eFlag;
    logger_date_t date;
    uint8_t nFullColor;
    uint8_t nNewLine;
} logger_context_t;

static logger_t g_logger;

static void logger_sync_init(logger_t *plogger)
{
    if (!plogger->nTdSafe)
        return;
    pthread_mutexattr_t mutexAttr;

    if (pthread_mutexattr_init(&mutexAttr) ||
        pthread_mutexattr_settype(&mutexAttr, PTHREAD_MUTEX_RECURSIVE) ||
        pthread_mutex_init(&plogger->mutex, &mutexAttr) ||
        pthread_mutexattr_destroy(&mutexAttr))
    {
        printf("<%s:%d> %s: [ERROR] Can not initialize mutex: %d\n",
               __FILE__, __LINE__, __FUNCTION__, errno);

        exit(EXIT_FAILURE);
    }
}

static void logger_lock(logger_t *plogger)
{
    if (plogger->nTdSafe && pthread_mutex_lock(&plogger->mutex))
    {
        printf("<%s:%d> %s: [ERROR] Can not lock mutex: %d\n",
               __FILE__, __LINE__, __FUNCTION__, errno);

        exit(EXIT_FAILURE);
    }
}

static void logger_unlock(logger_t *plogger)
{
    if (plogger->nTdSafe && pthread_mutex_unlock(&plogger->mutex))
    {
        printf("<%s:%d> %s: [ERROR] Can not unlock mutex: %d\n",
               __FILE__, __LINE__, __FUNCTION__, errno);

        exit(EXIT_FAILURE);
    }
}

static const char *logger_get_indent(logger_flag_t eFlag)
{
    logger_config_t *pCfg = &g_logger.config;
    if (!pCfg->nIndent)
        return LOGGER_EMPTY;

    switch (eFlag)
    {
    case LOGGER_INFO:
    case LOGGER_WARN:
        return LOGGER_SPACE;
    case LOGGER_DEBUG:
    case LOGGER_ERROR:
    default:
        break;
    }

    return LOGGER_EMPTY;
}

static const char *logger_get_tag(logger_flag_t eFlag)
{
    switch (eFlag)
    {
    case LOGGER_INFO:
        return "info";
    case LOGGER_WARN:
        return "warn";
    case LOGGER_DEBUG:
        return "debug";
    case LOGGER_ERROR:
        return "error";
    default:
        break;
    }

    return NULL;
}

static const char *logger_get_color(logger_flag_t eFlag)
{
    switch (eFlag)
    {
    case LOGGER_INFO:
        return LOGGER_COLOR_GREEN;
    case LOGGER_WARN:
        return LOGGER_COLOR_YELLOW;
    case LOGGER_DEBUG:
        return LOGGER_COLOR_BLUE;
    case LOGGER_ERROR:
        return LOGGER_COLOR_RED;
    default:
        break;
    }

    return LOGGER_EMPTY;
}

uint8_t logger_get_usec()
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) < 0)
        return 0;
    return (uint8_t)(tv.tv_usec / 10000);
}

void logger_get_date(logger_date_t *pDate)
{
    struct tm timeinfo;
    time_t rawtime = time(NULL);
    localtime_r(&rawtime, &timeinfo);

    pDate->nYear = timeinfo.tm_year + 1900;
    pDate->nMonth = timeinfo.tm_mon + 1;
    pDate->nDay = timeinfo.tm_mday;
    pDate->nHour = timeinfo.tm_hour;
    pDate->nMin = timeinfo.tm_min;
    pDate->nSec = timeinfo.tm_sec;
    pDate->nUsec = logger_get_usec();
}

static uint32_t logger_get_tid()
{
    return 0; // syscall(__NR_gettid);
}

static void logger_create_tag(char *pOut, size_t nSize, logger_flag_t eFlag, const char *pColor)
{
    logger_config_t *pCfg = &g_logger.config;
    pOut[0] = LOGGER_NUL;

    const char *pIndent = logger_get_indent(eFlag);
    const char *pTag = logger_get_tag(eFlag);

    if (pTag == NULL)
    {
        snprintf(pOut, nSize, pIndent);
        return;
    }

    if (pCfg->eColorFormat != LOGGER_COLORING_TAG)
        snprintf(pOut, nSize, "<%s>%s", pTag, pIndent);
    else
        snprintf(pOut, nSize, "%s<%s>%s%s", pColor, pTag, LOGGER_COLOR_RESET, pIndent);
}

static void logger_create_tid(char *pOut, int nSize, uint8_t nTraceTid)
{
    if (!nTraceTid)
        pOut[0] = LOGGER_NUL;
    else
        snprintf(pOut, nSize, "(%u) ", logger_get_tid());
}

static void logger_display_message(const logger_context_t *pCtx, const char *pInfo, int nInfoLen, const char *pInput)
{
    logger_config_t *pCfg = &g_logger.config;
    int nCbVal = 1;

    const char *pSeparator = nInfoLen > 0 ? pCfg->sSeparator : LOGGER_EMPTY;
    const char *pReset = pCtx->nFullColor ? LOGGER_COLOR_RESET : LOGGER_EMPTY;
    const char *pNewLine = pCtx->nNewLine ? LOGGER_NEWLINE : LOGGER_EMPTY;
    const char *pMessage = pInput != NULL ? pInput : LOGGER_EMPTY;

    if (pCfg->logCallback != NULL)
    {
        size_t nLength = 0;
        char *pLog = NULL;

        nLength += asprintf(&pLog, "%s%s%s%s%s", pInfo, pSeparator, pMessage, pReset, pNewLine);
        if (pLog != NULL)
        {
            nCbVal = pCfg->logCallback(pLog, nLength, pCtx->eFlag, pCfg->pCallbackCtx);
            free(pLog);
        }
    }

    if (pCfg->nToScreen && nCbVal > 0)
    {
        printf("%s%s%s%s%s", pInfo, pSeparator, pMessage, pReset, pNewLine);
        if (pCfg->nFlush)
            fflush(stdout);
    }

    if (!pCfg->nToFile || nCbVal < 0)
        return;
    const logger_date_t *pDate = &pCtx->date;

    char sFilePath[LOGGER_PATH_MAX + LOGGER_NAME_MAX + LOGGER_DATE_MAX];
    snprintf(sFilePath, sizeof(sFilePath), "%s/%s-%04d-%02d-%02d.log",
             pCfg->sFilePath, pCfg->sFileName, pDate->nYear, pDate->nMonth, pDate->nDay);

    FILE *pFile = fopen(sFilePath, "a");
    if (pFile == NULL)
        return;

    fprintf(pFile, "%s%s%s%s%s", pInfo, pSeparator, pMessage, pReset, pNewLine);
    fclose(pFile);
}

static int logger_create_info(const logger_context_t *pCtx, char *pOut, size_t nSize)
{
    logger_config_t *pCfg = &g_logger.config;
    const logger_date_t *pDate = &pCtx->date;

    char sDate[LOGGER_DATE_MAX + LOGGER_NAME_MAX];
    sDate[0] = LOGGER_NUL;

    if (pCfg->eDateControl == LOGGER_TIME_ONLY)
    {
        snprintf(sDate, sizeof(sDate), "%02d:%02d:%02d.%03d ",
                 pDate->nHour, pDate->nMin, pDate->nSec, pDate->nUsec);
    }
    else if (pCfg->eDateControl == LOGGER_DATE_FULL)
    {
        snprintf(sDate, sizeof(sDate), "%04d.%02d.%02d-%02d:%02d:%02d.%03d ",
                 pDate->nYear, pDate->nMonth, pDate->nDay, pDate->nHour,
                 pDate->nMin, pDate->nSec, pDate->nUsec);
    }

    char sTid[LOGGER_TAG_MAX], sTag[LOGGER_TAG_MAX];
    const char *pColorCode = logger_get_color(pCtx->eFlag);
    const char *pColor = pCtx->nFullColor ? pColorCode : LOGGER_EMPTY;

    logger_create_tid(sTid, sizeof(sTid), pCfg->nTraceTid);
    logger_create_tag(sTag, sizeof(sTag), pCtx->eFlag, pColorCode);
    return snprintf(pOut, nSize, "%s%s%s%s", pColor, sTid, sDate, sTag);
}

static void logger_display_heap(const logger_context_t *pCtx, va_list args)
{
    size_t nBytes = 0;
    char *pMessage = NULL;
    char loggerInfo[LOGGER_INFO_MAX];

    nBytes += vasprintf(&pMessage, pCtx->pFormat, args);
    va_end(args);

    if (pMessage == NULL)
    {
        printf("<%s:%d> %s<error>%s %s: Can not allocate memory for input: errno(%d)\n",
               __FILE__, __LINE__, LOGGER_COLOR_RED, LOGGER_COLOR_RESET, __FUNCTION__, errno);

        return;
    }

    int nLength = logger_create_info(pCtx, loggerInfo, sizeof(loggerInfo));
    logger_display_message(pCtx, loggerInfo, nLength, pMessage);
    if (pMessage != NULL)
        free(pMessage);
}

static void logger_display_stack(const logger_context_t *pCtx, va_list args)
{
    char sMessage[LOGGER_MESSAGE_MAX];
    char loggerInfo[LOGGER_INFO_MAX];

    vsnprintf(sMessage, sizeof(sMessage), pCtx->pFormat, args);
    int nLength = logger_create_info(pCtx, loggerInfo, sizeof(loggerInfo));
    logger_display_message(pCtx, loggerInfo, nLength, sMessage);
}

void logger_display(logger_flag_t eFlag, uint8_t nNewLine, int inLogModule, const char *pFormat, ...)
{

    printf("inLogModule = %d\n", inLogModule);
    printf("extern module_flag = %d\n", module_flag);

    logger_lock(&g_logger);
    logger_config_t *pCfg = &g_logger.config;

    if ((LOGGER_FLAGS_CHECK(g_logger.config.nFlags, eFlag)) &&
        (g_logger.config.nToScreen || g_logger.config.nToFile) &&
        (module_flag == 1 || module_flag == inLogModule))
    {
        logger_context_t ctx;
        logger_get_date(&ctx.date);

        ctx.eFlag = eFlag;
        ctx.pFormat = pFormat;
        ctx.nNewLine = nNewLine;
        ctx.nFullColor = pCfg->eColorFormat == LOGGER_COLORING_FULL ? 1 : 0;

        void (*logger_display_args)(const logger_context_t *pCtx, va_list args);
        logger_display_args = pCfg->nUseHeap ? logger_display_heap : logger_display_stack;

        va_list args;
        va_start(args, pFormat);
        logger_display_args(&ctx, args);
        va_end(args);
    }

    logger_unlock(&g_logger);
}

void logger_config_get(logger_config_t *pCfg)
{
    logger_lock(&g_logger);
    *pCfg = g_logger.config;
    logger_unlock(&g_logger);
}

void logger_config_set(logger_config_t *pCfg)
{
    logger_lock(&g_logger);
    g_logger.config = *pCfg;
    logger_unlock(&g_logger);
}

void logger_enable(logger_flag_t eFlag)
{
    logger_lock(&g_logger);

    if (!LOGGER_FLAGS_CHECK(g_logger.config.nFlags, eFlag))
        g_logger.config.nFlags |= eFlag;

    logger_unlock(&g_logger);
}

// flag for selecting module

void logger_disable(logger_flag_t eFlag)
{
    logger_lock(&g_logger);

    if (LOGGER_FLAGS_CHECK(g_logger.config.nFlags, eFlag))
        g_logger.config.nFlags &= ~eFlag;

    logger_unlock(&g_logger);
}

void logger_separator_set(const char *pFormat, ...)
{
    logger_lock(&g_logger);
    logger_config_t *pCfg = &g_logger.config;

    va_list args;
    va_start(args, pFormat);

    if (vsnprintf(pCfg->sSeparator, sizeof(pCfg->sSeparator), pFormat, args) <= 0)
    {
        pCfg->sSeparator[0] = ' ';
        pCfg->sSeparator[1] = '\0';
    }

    va_end(args);
    logger_unlock(&g_logger);
}

void logger_indent(uint8_t nEnable)
{
    logger_lock(&g_logger);
    g_logger.config.nIndent = nEnable;
    logger_unlock(&g_logger);
}

void logger_callback_set(logger_cb_t callback, void *pContext) //
{
    logger_lock(&g_logger);
    logger_config_t *pCfg = &g_logger.config;
    pCfg->pCallbackCtx = pContext;
    pCfg->logCallback = callback;
    logger_unlock(&g_logger);
}

void logger_init(const char *pName, uint16_t nFlags, uint8_t nTdSafe)
{
    /* Set up default values */
    logger_config_t *pCfg = &g_logger.config;
    pCfg->eColorFormat = LOGGER_COLORING_TAG;
    pCfg->eDateControl = LOGGER_TIME_ONLY;
    pCfg->pCallbackCtx = NULL;
    pCfg->logCallback = NULL;
    pCfg->sSeparator[0] = ' ';
    pCfg->sSeparator[1] = '\0';
    pCfg->sFilePath[0] = '.';
    pCfg->sFilePath[1] = '\0';
    pCfg->nTraceTid = 0;
    pCfg->nToScreen = 1;
    pCfg->nUseHeap = 0;
    pCfg->nToFile = 0;
    pCfg->nIndent = 0;
    pCfg->nFlush = 0;
    pCfg->nFlags = nFlags;

    // const char* pFileName = (pName != NULL) ? pName : logger_NAME_DEFAULT;
    // snprintf(pCfg->sFileName, sizeof(pCfg->sFileName), "%s", pFileName);

    /* Initialize mutex */
    g_logger.nTdSafe = nTdSafe;
    logger_sync_init(&g_logger);
}

void logger_destroy()
{
    g_logger.config.pCallbackCtx = NULL;
    g_logger.config.logCallback = NULL;

    if (g_logger.nTdSafe)
    {
        pthread_mutex_destroy(&g_logger.mutex);
        g_logger.nTdSafe = 0;
    }
}