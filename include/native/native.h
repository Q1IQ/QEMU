#ifndef __NATIVE_H__
#define __NATIVE_H__
/*
 * Check if the native bypass feature is enabled.
 */
#if defined(CONFIG_USER_ONLY) && defined(CONFIG_NATIVE_CALL)
extern char *native_lib_path;
#define native_bypass_enabled() native_lib_path ? true : false
#else
#define native_bypass_enabled() false
#endif

#endif /* __NATIVE_H__ */
