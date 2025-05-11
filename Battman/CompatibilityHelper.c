#if __ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__ < 110000

void *memcpy(void *a,const void *b, unsigned long c) {
	return __builtin___memcpy_chk(a,b,c,c);
}

void *memset(void *a,int b,unsigned long c) {
	return __builtin___memset_chk(a,b,c,c);
}

int strcmp(const char *a, const char *b) {
	extern int _platform_strcmp(const char *,const char *);
	return _platform_strcmp(a,b);
}

int strncmp(const char *a, const char *b,unsigned long c) {
	extern int _platform_strncmp(const char *,const char *,unsigned long);
	return _platform_strncmp(a,b,c);
}

void _Unwind_Resume() {}

#endif