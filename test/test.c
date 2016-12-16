/**                                                  
 * @brief TEST Progoram
 * @file test.c
 * @date 20161206
 * @author kimjinkwon
*/

#include <stdio.h>

/**
 * @brief 더하기 프로그램 
 * 
 * @param a 첫번재 인자 
 * @param b 두번재 인자.
 * 
 * @return 
 */
static int Add(int a, int b)
{
	return a+b;
}

/**
 * @brief 메인 함수 
 *
 * @param argc 인자다
 * @param argv 두번재 인자다 
 *
 * @return 
 */
int main(int argc, char** argv)
{

	printf("%d\n",Add(atoi(argv[0]),atoi(argv[1])));
	return 0;
}

