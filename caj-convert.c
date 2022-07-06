/*
  Code originally posted by Anonymous at https://github.com/caj2pdf/caj2pdf/issues/63 .

  Build instruction - Linux:
      gcc -Wall caj-convert.c -ldl

  Cross-compile for 64-bit Windows:
      x86_64-w64-mingw32-gcc -Wall caj-convert.c

  Cross-compile for 32-bit Windows:
      i686-w64-mingw32-gcc -Wall caj-convert.c

  You need "ReaderEx_x64.dll" or "libreaderex_x64.so" from the official app
  in the same directory. In some cases, the "Resource" directory is also needed.

  Issue 90, 4-[21].caj and 4-[24].caj definitely requires extname set to ".caj" to work.
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Cross-platform support for Windows & Linux.
#if !(defined(CAJ2PDF_OS_WINDOWS) || defined(CAJ2PDF_OS_LINUX))
	#if (defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__))
		#define CAJ2PDF_OS_WINDOWS
	#endif
	#if (defined(linux) || defined(__linux) || defined(__linux__) || defined(__gnu_linux__))
		#define CAJ2PDF_OS_LINUX
	#endif
#endif
#if !(defined(CAJ2PDF_OS_WINDOWS) || defined(CAJ2PDF_OS_LINUX))
	#error unsupported platform
#endif
#if defined(CAJ2PDF_OS_WINDOWS) && defined(CAJ2PDF_OS_LINUX)
	#error conflicting platform definition
#endif

#if defined(CAJ2PDF_OS_WINDOWS)
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#elif defined(CAJ2PDF_OS_LINUX)
	#include <dlfcn.h>
#endif

struct Parameter
{
	size_t cb;
	uint32_t flag[2];
	char *src;
	char *extname;
	// Function pointers for open, read, seek, tell, eof, and close.
	void *pfnFILE[6];
	char *dest;
	// Function pointers.
	void *pfnoss[4];
};

#if defined(CAJ2PDF_OS_LINUX)
// CAJFILE_Init required on Linux.
typedef int (*PCAJFILE_Init)(char const *exeDir, char const **, int, char const *tmpDir);
#endif
typedef int (*PCAJFILE_DistillPageEx1)(struct Parameter *param);

char src[4096], extname[256], dest[4096];

FILE *debugopen(const char *pathname, const char *mode)
{
	FILE *ret = fopen(pathname, mode);
	fprintf(stderr, "fopen(pathname = \"%s\", mode = \"%s\") = %p\n", pathname, mode, ret);
	return ret;
}

size_t debugread(FILE *stream, void *ptr, size_t size)
{
	size_t ret = fread(ptr, size, 1l, stream);
	fprintf(stderr, "fread(ptr = %p, size = %lu, nmemb = %ld, stream = %p) = %lu\n", ptr, size, 1l, stream, ret);
	return ret;
}

int debugseek(FILE *stream, long offset, int whence)
{
	int ret = fseek(stream, offset, whence);
	fprintf(stderr, "fseek(stream = %p, offset = %ld, whence = %d) = %d\n", stream, offset, whence, ret);
	return ret;
}

long debugtell(FILE *stream)
{
	long ret = ftell(stream);
	fprintf(stderr, "ftell(stream = %p) = %ld\n", stream, ret);
	return ret;
}

int debugeof(FILE *stream)
{
	int ret = feof(stream);
	fprintf(stderr, "feof(stream = %p) = %d\n", stream, ret);
	return ret;
}

int debugclose(FILE *stream)
{
	int ret = fclose(stream);
	fprintf(stderr, "fclose(stream = %p) = %d\n", stream, ret);
	return ret;
}

int main(int argc, char *argv[])
{
#if defined(CAJ2PDF_OS_WINDOWS)
#ifdef _WIN64
	HMODULE handle = LoadLibrary(TEXT(".\\ReaderEx_x64.dll"));
#else
	HMODULE handle = LoadLibrary(TEXT(".\\ReaderEx.dll"));
#endif
#elif defined(CAJ2PDF_OS_LINUX)
	void *handle = dlopen("./libreaderex_x64.so", RTLD_LAZY);
#endif
	if (handle)
	{
		printf("[D] handle = %p\n", handle);
#if defined(CAJ2PDF_OS_WINDOWS)
		PCAJFILE_DistillPageEx1 CAJFILE_DistillPageEx1;
		// ReaderEx_DIS_C 2.3.0 Build 3982: ordinal value = 216
		// 2.0.3949.0 to 2.3.3983.0, 2.3.3982.0 x64 : ordinal value = 216
                //
                // The customized jbig introduced between 2.0.3920.0 and 2.0.3949.0
                // ImageCodec.dll jumped from 800k to 930k
                //
		// 2.0.3920.0                            HN : jpeg + jbig2
		// 2.0.3684.0, 2.0.3799.0                HN : jpeg only
		*(void **)(&CAJFILE_DistillPageEx1) = GetProcAddress(handle, (LPCSTR)216);
#elif defined(CAJ2PDF_OS_LINUX)
		// CAJFILE_Init required on Linux.
		PCAJFILE_Init CAJFILE_Init;
		PCAJFILE_DistillPageEx1 CAJFILE_DistillPageEx1;
		*(void **)(&CAJFILE_Init) = dlsym(handle, "CAJFILE_Init");
		if (CAJFILE_Init)
		{
			printf("[D] CAJFILE_Init = %p\n", CAJFILE_Init);
			int result = CAJFILE_Init(".", NULL, 0, "./tmp");
			if (result)
			{
				printf("[E] CAJFILE_Init failed (%d)\n", result);
				goto cleanup;
			}
		}
		else
		{
			printf("[E] dlsym failed (%s)\n", dlerror());
			goto cleanup;
		}
		*(void **)(&CAJFILE_DistillPageEx1) = dlsym(handle, "CAJFILE_DistillPageEx1");
#endif
		if (CAJFILE_DistillPageEx1)
		{
			printf("[D] CAJFILE_DistillPageEx1 = %p\n", CAJFILE_DistillPageEx1);
			struct Parameter param;
			memset(&param, 0, sizeof param);
			param.cb = sizeof param;
			param.flag[1] = 0x26;
			if (argc > 3)
			{
				printf("[?] Source pathname = %s\n", argv[1]);
				param.src = strcpy(src, argv[1]);
				printf("[?] Source extname = %s\n", argv[3]);
				param.extname = strcpy(extname, argv[3]);
				printf("[?] Destination pathname = %s\n", argv[2]);
				param.dest = strcpy(dest, argv[2]);;
			}
			else
			{
				printf("[?] Source pathname = ");
				scanf("%s", src);
				param.src = src;
				printf("[?] Source extname = ");
				scanf("%s", extname);
				param.extname = extname;
				printf("[?] Destination pathname = ");
				scanf("%s", dest);
				param.dest = dest;
			}
#ifdef DEBUG
			param.pfnFILE[0] = debugopen;
			param.pfnFILE[1] = debugread;
			param.pfnFILE[2] = debugseek;
			param.pfnFILE[3] = debugtell;
			param.pfnFILE[4] = debugeof;
			param.pfnFILE[5] = debugclose;
#endif
			int result = CAJFILE_DistillPageEx1(&param);
			if (result)
			{
				printf("[I] Success\n");
			}
			else
			{
				printf("[E] CAJFILE_DistillPageEx1 failed (%d)\n", result);
				goto cleanup;
			}
		}
		else
		{
#if defined(CAJ2PDF_OS_WINDOWS)
			printf("[E] GetProcAddress failed (%lu)\n", GetLastError());
#elif defined(CAJ2PDF_OS_LINUX)
			printf("[E] dlsym failed (%s)\n", dlerror());
#endif
			goto cleanup;
		}
	cleanup:
#if defined(CAJ2PDF_OS_WINDOWS)
		// If the function succeeds, the return value is nonzero.
		// If the function fails, the return value is zero.
		if (!FreeLibrary(handle))
		{
			printf("[E] FreeLibrary failed (%lu)\n", GetLastError());
		}
#elif defined(CAJ2PDF_OS_LINUX)
		// On success, dlclose() returns 0; on error, it returns a nonzero value.
		if (dlclose(handle))
		{
			printf("[E] dlclose failed (%s)\n", dlerror());
		}
#endif
	}
	else
	{
#if defined(CAJ2PDF_OS_WINDOWS)
		printf("[E] LoadLibrary failed (%lu)\n", GetLastError());
#elif defined(CAJ2PDF_OS_LINUX)
		printf("[E] dlopen failed (%s)\n", dlerror());
#endif
	}
}
