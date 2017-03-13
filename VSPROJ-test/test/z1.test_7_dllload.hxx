#include <windows.h>
#include <stdio.h>

#include <string>


//声明DLL里的函数原型  
typedef PVOID (CALLBACK* PFNEXPORTFUNC)(PIMAGE_NT_HEADERS,PVOID,ULONG,PIMAGE_SECTION_HEADER*);

char* ATE_GetDllFuncNames(const char *fileName)
{
	std::string FunctionName;
	LPWIN32_FIND_DATA lpwfd_first=new WIN32_FIND_DATA;//接受findfirstfile的结构指针
	HANDLE hFile,hFileMap;//文件句柄和内存映射文件句柄
	DWORD fileAttrib=0;//存储文件属性用，在createfile中用到。
	void* mod_base;//内存映射文件的起始地址，也是模块的起始地址
	//首先取得ImageRvaToVa函数本来只要#include <Dbghelp.h>就可以使用这个函数，但是我的vc没有这个头文件
	PFNEXPORTFUNC ImageRvaToVax=NULL;
	HMODULE hModule=::LoadLibrary("DbgHelp.dll");
	if(hModule!=NULL){
		ImageRvaToVax=(PFNEXPORTFUNC)::GetProcAddress(hModule,"ImageRvaToVa");
		if(ImageRvaToVax==NULL){
			::FreeLibrary(hModule);
			printf("取得函数失败\n");
			return strdup(FunctionName.c_str()); 
		}  

	}else{
		printf("加载模块失败\n");
		return strdup(FunctionName.c_str());
	}

	if(FindFirstFile(fileName,lpwfd_first)==NULL){//返回值为NULL，则文件不存在，退出
		printf("文件不存在: %s ",fileName);
		return strdup(FunctionName.c_str());
	}else{
		DWORD fileAttrib=lpwfd_first->dwFileAttributes;
	}
	hFile=CreateFile(fileName,GENERIC_READ,0,0,OPEN_EXISTING,fileAttrib,0);
	if(hFile==INVALID_HANDLE_VALUE){
		printf("打开文件出错！");
		return strdup(FunctionName.c_str()); 
	}
	hFileMap=CreateFileMapping(hFile,0,PAGE_READONLY,0,0,0);
	if(hFileMap==NULL){
		CloseHandle(hFile);
		printf("建立内存映射文件出错！");
		return strdup(FunctionName.c_str());
	}
	mod_base=MapViewOfFile(hFileMap,FILE_MAP_READ,0,0,0);
	if (mod_base==NULL)
	{
		printf("建立内存映射文件出错！");
		CloseHandle(hFileMap);
		CloseHandle(hFile);
		return strdup(FunctionName.c_str());
	}
	IMAGE_DOS_HEADER* pDosHeader = (IMAGE_DOS_HEADER*)mod_base;
	IMAGE_NT_HEADERS * pNtHeader =
		(IMAGE_NT_HEADERS *)((BYTE*)mod_base+ pDosHeader->e_lfanew);//得到NT头首址
	//IMAGE_NT_HEADERS * pNtHeader =ImageNtHeader (mod_base);
	IMAGE_OPTIONAL_HEADER * pOptHeader =
		(IMAGE_OPTIONAL_HEADER *)((BYTE*)mod_base + pDosHeader->e_lfanew + 24);//optional头首址

	IMAGE_EXPORT_DIRECTORY* pExportDesc = (IMAGE_EXPORT_DIRECTORY*)ImageRvaToVax(pNtHeader,mod_base,pOptHeader->DataDirectory[0].VirtualAddress,0);
	//导出表首址
	PDWORD nameAddr=(PDWORD)ImageRvaToVax(pNtHeader,mod_base,pExportDesc->AddressOfNames,0);//函数名称表首地址每个DWORD代表一个函数名字字符串的地址
	PCHAR func_name = (PCHAR)ImageRvaToVax(pNtHeader,mod_base,(DWORD)nameAddr[0],0);
	DWORD i=0;
	DWORD unti=pExportDesc->NumberOfNames;
	//FunctionName = "[";
	for(i=0; i < unti; i++){
		//FunctionName += "\"";
		func_name = (PCHAR)ImageRvaToVax(pNtHeader,mod_base,(DWORD)nameAddr[i],0);
		FunctionName += func_name;
		//FunctionName += "\"";
		if (i != unti - 1)
		{
			FunctionName += ",";
		}
	}
	//FunctionName += "]";
	::FreeLibrary(hModule);
	CloseHandle(hFileMap);
	CloseHandle(hFile);
	return strdup(FunctionName.c_str());
}


int main()
{

    char* p = ATE_GetDllFuncNames("ATE_COMMON.dll");
    free(p);

    return 0;
}