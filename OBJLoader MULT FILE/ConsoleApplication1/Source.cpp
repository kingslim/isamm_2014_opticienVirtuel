
#include <iostream> 
#include <stdlib.h> 


using namespace std;

int main()
{
	int x=524;

	int* a=&x;
	
	cout << ++*a;
	cout << x;
	system("pause");

	return 0;
}