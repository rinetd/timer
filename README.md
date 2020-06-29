# timer function

```cpp

void time_callback(void * arg ){
	printf("%s\n",(char *)arg);
}

int main(int argc, char *argv[])
{

	char *str ="test timer func call ";

	timer_add(1000,time_callback,str);
	getchar();
	timer_stop();
	return 0;
}
```
