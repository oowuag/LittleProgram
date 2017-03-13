// crt_vsnprintf.cpp
// compile with: /W3
#include <stdio.h>
#include <wtypes.h>

void FormatOutput(LPCSTR formatstring, ...) 
{
	int nSize = 0;
	char buff[10];
	memset(buff, 0, sizeof(buff));
	va_list args;
	va_start(args, formatstring);
	nSize = vsnprintf( buff, sizeof(buff) - 1, formatstring, args); // C4996
	// Note: vsnprintf is deprecated; consider vsnprintf_s instead
	printf("nSize: %d, buff: %s\n", nSize, buff);
}

int main() {
	FormatOutput("%s %s", "Hi", "there");
	FormatOutput("%s %s", "Hi", "there!");
	FormatOutput("%s %s", "Hi", "there!!");
}