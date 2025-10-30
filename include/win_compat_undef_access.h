// win_compat_undef_access.h - Undefine Windows macros that conflict
#ifndef WIN_COMPAT_UNDEF_ACCESS_H
#define WIN_COMPAT_UNDEF_ACCESS_H

// Undefine Windows macros that might conflict with other libraries
#ifdef _ACCESS
#undef _ACCESS
#endif

#ifdef DELETE
#undef DELETE
#endif

#ifdef IGNORE
#undef IGNORE
#endif

#endif // WIN_COMPAT_UNDEF_ACCESS_H