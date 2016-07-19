/*
 ..\toolchain\Windows\Tools\Cross\i686-elf\bin\i686-elf-g++ foo.c -nostdlib -o runme.o --entry main
*/
int main(int argc, char ** argv) {
	char i = 1;
	for(;;) {
		char * c = (char*)0xb8000;

		char msg[20];
		msg[0]='h';
		msg[1]=i++;
		msg[2]='l';
		msg[3]='l';
		msg[4]='o';
		msg[5]=' ';
		msg[6]='w';
		msg[7]='o';
		msg[8]='r';
		msg[9]='l';
		msg[10]='d';
		msg[11]='\0';
		int i;
		for(i = 0;i < 20; i++) {
			*c = msg[i];
			c += 2;
		}
	}
	return 15;
}