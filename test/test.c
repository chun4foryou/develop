/**                                                  
 * @brief TEST Progoram
 * @file test.c
 * @date 20161206
 * @author kimjinkwon
*/

#include <stdio.h>

/**
 * @brief ���ϱ� ���α׷� 
 * 
 * @param a ù���� ���� 
 * @param b �ι��� ����.
 * 
 * @return 
 */
static int Add(int a, int b)
{
	return a+b;
}

/**
 * @brief ���� �Լ� 
 *
 * @param argc ���ڴ�
 * @param argv �ι��� ���ڴ� 
 *
 * @return 
 */
int main(int argc, char** argv)
{

	printf("%d\n",Add(atoi(argv[0]),atoi(argv[1])));
	return 0;
}

