/* andhook - Android Hooking Framework */
/* David Kaplan, 2012 (david@2of1.org) */

/* use this fcn within your library's __init() to set up hook */
void and_hook(void *orig_fcn, void* new_fcn, void **orig_fcn_ptr);
