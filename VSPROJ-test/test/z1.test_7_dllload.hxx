#include <windows.h>
#include <stdio.h>

#include <string>


//����DLL��ĺ���ԭ��  
typedef PVOID (CALLBACK* PFNEXPORTFUNC)(PIMAGE_NT_HEADERS,PVOID,ULONG,PIMAGE_SECTION_HEADER*);

char* ATE_GetDllFuncNames(const char *fileName)
{
	std::string FunctionName;
	LPWIN32_FIND_DATA lpwfd_first=new WIN32_FIND_DATA;//����findfirstfile�Ľṹָ��
	HANDLE hFile,hFileMap;//�ļ�������ڴ�ӳ���ļ����
	DWORD fileAttrib=0;//�洢�ļ������ã���createfile���õ���
	void* mod_base;//�ڴ�ӳ���ļ�����ʼ��ַ��Ҳ��ģ�����ʼ��ַ
	//����ȡ��ImageRvaToVa��������ֻҪ#include <Dbghelp.h>�Ϳ���ʹ����������������ҵ�vcû�����ͷ�ļ�
	PFNEXPORTFUNC ImageRvaToVax=NULL;
	HMODULE hModule=::LoadLibrary("DbgHelp.dll");
	if(hModule!=NULL){
		ImageRvaToVax=(PFNEXPORTFUNC)::GetProcAddress(hModule,"ImageRvaToVa");
		if(ImageRvaToVax==NULL){
			::FreeLibrary(hModule);
			printf("ȡ�ú���ʧ��\n");
			return strdup(FunctionName.c_str()); 
		}  

	}else{
		printf("����ģ��ʧ��\n");
		return strdup(FunctionName.c_str());
	}

	if(FindFirstFile(fileName,lpwfd_first)==NULL){//����ֵΪNULL�����ļ������ڣ��˳�
		printf("�ļ�������: %s ",fileName);
		return strdup(FunctionName.c_str());
	}else{
		DWORD fileAttrib=lpwfd_first->dwFileAttributes;
	}
	hFile=CreateFile(fileName,GENERIC_READ,0,0,OPEN_EXISTING,fileAttrib,0);
	if(hFile==INVALID_HANDLE_VALUE){
		printf("���ļ�����");
		return strdup(FunctionName.c_str()); 
	}
	hFileMap=CreateFileMapping(hFile,0,PAGE_READONLY,0,0,0);
	if(hFileMap==NULL){
		CloseHandle(hFile);
		printf("�����ڴ�ӳ���ļ�����");
		return strdup(FunctionName.c_str());
	}
	mod_base=MapViewOfFile(hFileMap,FILE_MAP_READ,0,0,0);
	if (mod_base==NULL)
	{
		printf("�����ڴ�ӳ���ļ�����");
		CloseHandle(hFileMap);
		CloseHandle(hFile);
		return strdup(FunctionName.c_str());
	}
	IMAGE_DOS_HEADER* pDosHeader = (IMAGE_DOS_HEADER*)mod_base;
	IMAGE_NT_HEADERS * pNtHeader =
		(IMAGE_NT_HEADERS *)((BYTE*)mod_base+ pDosHeader->e_lfanew);//�õ�NTͷ��ַ
	//IMAGE_NT_HEADERS * pNtHeader =ImageNtHeader (mod_base);
	IMAGE_OPTIONAL_HEADER * pOptHeader =
		(IMAGE_OPTIONAL_HEADER *)((BYTE*)mod_base + pDosHeader->e_lfanew + 24);//optionalͷ��ַ

	IMAGE_EXPORT_DIRECTORY* pExportDesc = (IMAGE_EXPORT_DIRECTORY*)ImageRvaToVax(pNtHeader,mod_base,pOptHeader->DataDirectory[0].VirtualAddress,0);
	//��������ַ
	PDWORD nameAddr=(PDWORD)ImageRvaToVax(pNtHeader,mod_base,pExportDesc->AddressOfNames,0);//�������Ʊ��׵�ַÿ��DWORD����һ�����������ַ����ĵ�ַ
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