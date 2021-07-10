#include <stdio.h>

#include <windows.h>

struct ParamList
{
	long flag[4];
	char *src;
	char *extname;
	// Function pointers for open, read, seek, tell, eof, and close.
	void *pfnFILE[6];
	char *dest;
	// Function pointers.
	void *pfnoss[4];
};

typedef int (*PCAJFILE_DistillPageEx1)(ParamList p);

int main(int argc, char *argv[])
{
	HMODULE hMod = LoadLibrary("ReaderEx_x64.dll");
	if (hMod)
	{
		printf("[D] hMod = %8p\n", hMod);
		PCAJFILE_DistillPageEx1 CAJFILE_DistillPageEx1 = (PCAJFILE_DistillPageEx1)GetProcAddress(hMod, (LPCSTR)216);
		if (CAJFILE_DistillPageEx1)
		{
			printf("[D] CAJFILE_DistillPageEx1 = %8p\n", (void *)CAJFILE_DistillPageEx1);
			ParamList p;
			ZeroMemory(&p, sizeof p);
			p.flag[0] = 0x78;
			p.flag[3] = 0x26;
			char src[256], extname[256], dest[256];
			printf("[?] Source pathname = ");
			scanf("%s", src);
			p.src = src;
			printf("[?] Source extname = ");
			scanf("%s", extname);
			p.extname = extname;
			printf("[?] Destination pathname = ");
			scanf("%s", dest);
			p.dest = dest;
			int result = CAJFILE_DistillPageEx1(p);
			if (result)
			{
				printf("[I] Success\n");
			}
			else
			{
				printf("[E] CAJFILE_DistillPageEx1 failed (%d)\n", result);
			}
		}
		else
		{
			printf("[E] GetProcAddress failed (%d)\n", GetLastError());
		}
	}
	else
	{
		printf("[E] LoadLibrary failed (%d)\n", GetLastError());
	}
}
