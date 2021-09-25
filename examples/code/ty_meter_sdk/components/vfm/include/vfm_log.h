/*
uni_log.h
Copyright(C),2018-2020, 涂鸦科技 www.tuya.comm
*/
#ifndef _UNI_LOG_H
#define _UNI_LOG_H

typedef int OPERATE_RET;//A:xiaoJian


#ifdef __cplusplus
extern "C" {
#endif

#define TY_LOG_LEVEL_ERR       0  // 错误信息，程序正常运行不应发生的信息 
#define TY_LOG_LEVEL_WARN      1  // 警告信息
#define TY_LOG_LEVEL_NOTICE    2  // 需要注意的信息
#define TY_LOG_LEVEL_INFO      3  // 通知信息
#define TY_LOG_LEVEL_DEBUG     4  // 程序运行调试信息，RELEASE版本中删除
#define TY_LOG_LEVEL_TRACE     5  // 程序运行路径信息，RELEASE版本中删除

#define SAK_PRINT_LOG(module,level,fmt, ...) \
do \
{ \
    PrintLog(module,level, \
             __FILE__,__LINE__,(const char*)__func__, \
             fmt,##__VA_ARGS__); \
}while(0)

#define PR_ERR(fmt, ...) SAK_PRINT_LOG(NULL,TY_LOG_LEVEL_ERR, fmt, ##__VA_ARGS__)
#define PR_WARN(fmt, ...) SAK_PRINT_LOG(NULL,TY_LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define PR_NOTICE(fmt, ...) SAK_PRINT_LOG(NULL,TY_LOG_LEVEL_NOTICE, fmt, ##__VA_ARGS__)
#define PR_INFO(fmt, ...) SAK_PRINT_LOG(NULL,TY_LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif


