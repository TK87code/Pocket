#include <stdio.h> // fopen, fclose, fprintf, fflush
#include <stdarg.h> // va_list, va_start, vfprintf, va_end
#include <sys/time.h> // gettimeofday
#include <time.h> // localtime
#include "pocket.h"

static int is_initialized = 0;

// Output formatted log message to ./pocket_log.txt.
// Return 0 on success, -1 when failed to open the txt file.
int __pkt_log_out(enum pkt_log_level level, const char *fmt, ...)
{
	const char *mode = "a";
	if (is_initialized == 0) {
		mode = "w";
		is_initialized = 1;
	}

	FILE *fp = fopen("pocket_log.txt", mode);
	if (fp == NULL)
		return -1;
	
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d] ", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
			t->tm_hour, t->tm_min, t->tm_sec);

	switch (level) {
	case PKT_LOG_INFO:
		fprintf(fp, "[INFO] ");	
		break;
	case PKT_LOG_WARN:
		fprintf(fp, "[WARNING] ");	
		break;
	case PKT_LOG_ERROR:
		fprintf(fp, "[ERROR] ");	
		break;
	}

	va_list args;
	va_start(args, fmt);
	vfprintf(fp, fmt, args);
	va_end(args);

	fprintf(fp, "\n");
	fclose(fp);

	return 0;
}
