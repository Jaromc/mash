
#ifndef EXTERN_C_H
#define EXTERN_C_H
#ifdef __cplusplus
#define EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }
#else
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#endif
#endif

#ifndef EXTERN_CPP_H
#define EXTERN_CPP_H
#ifndef __cplusplus
#define EXTERN_CPP_BEGIN extern "C++" {
#define EXTERN_CPP_END }
#else
#define EXTERN_CPP_BEGIN
#define EXTERN_CPP_END
#endif
#endif