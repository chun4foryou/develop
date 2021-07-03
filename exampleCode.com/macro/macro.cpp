#include  <iostream>

#define PRINT(s, ...) printf(#s, ##__VA_ARGS__)

int main()
{
    PRINT(TEST000   0 %d\n, 10);
}
