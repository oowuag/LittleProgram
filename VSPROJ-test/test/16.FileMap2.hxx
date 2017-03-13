#include <iostream>
#include <assert.h>
#include <Windows.h>
#include <WinBase.h>
using namespace std;

#define BAD_POS 0xFFFFFFFF // returned by SetFilePointer and GetFileSize
#define SUCCESS 0

typedef DWORD mmf_share_mode;
typedef DWORD mmf_access_mode;
typedef DWORD mmf_flags;

int main()
{
	cout<<"create memorymapfile..."<<endl;
	const char* shared_name = "testMmf";
	const char* file_name = "d:\\testMmf.mmf";
	const DWORD mmf_size = 512*1024;
	//��ȡģʽ
	mmf_access_mode access_mode = (GENERIC_READ|GENERIC_WRITE);
	//����ģʽ
	mmf_share_mode share_mode = FILE_SHARE_READ | FILE_SHARE_WRITE;
	//�ļ�����
	mmf_flags flags = FILE_FLAG_SEQUENTIAL_SCAN;//|FILE_FLAG_WRITE_THROUGH|FILE_FLAG_NO_BUFFERING;
	DWORD error_code;
	//�����ļ�
	HANDLE mmHandle =
		CreateFile(file_name,
		access_mode,
		share_mode,
		NULL,
		OPEN_ALWAYS,
		flags,
		NULL);
	if (mmHandle == INVALID_HANDLE_VALUE) {
		error_code = GetLastError();
		cout<<"����mmfʧ��:"<<error_code<<endl;
	}else{
		DWORD high_size;
		DWORD file_size = GetFileSize(mmHandle, &high_size);
		if (file_size == BAD_POS && (error_code = GetLastError()) != SUCCESS) {
			CloseHandle(mmHandle);
			cout<<"error��"<<error_code<<endl;
		}
		cout<<"create mmf sucessfully"<<endl;
		//assert(file_size == 0);
		DWORD size_high = 0;
		//�����ļ�ӳ�䣬���Ҫ�����ڴ�ҳ���ļ���ӳ�䣬��һ����������ΪINVALID_HANDLE_VALUE
		HANDLE mmfm = CreateFileMapping(mmHandle,
			NULL,
			PAGE_READWRITE,
			size_high,
			mmf_size,
			shared_name);
		error_code = GetLastError();
		if(SUCCESS != error_code){
			cout<<"createFileMapping error"<<error_code<<endl;
		}else{
			if(mmfm == NULL){
				if(mmHandle != INVALID_HANDLE_VALUE){
					CloseHandle(mmHandle);
				}
			}else{
				//char write_chars[] = "hello chars";
				//size_t position = 0;
				//DWORD written = 0;
				//const size_t write_chars_size = sizeof(write_chars);
				//WriteFile(mmHandle,write_chars,write_chars_size,&written,NULL);
				size_t view_size = 1024*256;
				DWORD view_access = FILE_MAP_ALL_ACCESS;
				//���ӳ����ͼ
				char* mmfm_base_address = (char*)MapViewOfFile(mmfm,view_access,0,0,view_size);
				if(mmfm_base_address == NULL){
					error_code = GetLastError();
					if(error_code != SUCCESS){
						cout<<"error code "<<error_code<<endl;
					}
				}else{
					char write_chars[] = "hello chars";
					const size_t write_chars_size = sizeof(write_chars);
					//���ڴ�ӳ����ͼ��д����
					CopyMemory((PVOID)mmfm_base_address, write_chars, write_chars_size);
					//memcpy(mmfm_base_address,write_chars,write_chars_size);
					size_t position = 0;
					char read_chars[write_chars_size];
					//������
					memcpy(read_chars,mmfm_base_address,write_chars_size);
					cout<<"read chars "<<read_chars<<endl;
					//ж��ӳ��
					UnmapViewOfFile(mmfm_base_address);
					//�ر��ڴ�ӳ���ļ�
					CloseHandle(mmfm);
					//�ر��ļ�
					CloseHandle(mmHandle);
				}
			}
		}
	}
	system("pause");
	exit(0);
	return EXIT_SUCCESS;
}