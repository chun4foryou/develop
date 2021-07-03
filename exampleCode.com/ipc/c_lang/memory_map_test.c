#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

int main(int argc, char **argv) {
  int fd;
  char *file = NULL;
  struct stat sb;
  int flag = PROT_WRITE | PROT_READ;

  if (argc < 2) {
    fprintf(stderr, "Usage: input\n");
    return 0;
  }

  if ((fd = open(argv[1], O_RDWR|O_CREAT)) < 0) {
    perror("File Open Error");
    return 0;
  }

  if (fstat(fd, &sb) < 0) {
    perror("fstat error");
    return 0;
  }

  file = (char *)malloc(40);

  // mmap�� �̿��ؼ� ���� ������ �޸𸮿� ������Ų��.
  // file�� ������ �ּҸ� ����Ű��, file�� �̿��ؼ� �ʿ��� �۾���
  // �ϸ� �ȴ�.
  if ((file = (char *) mmap(0, 40, flag, MAP_SHARED, fd, 0)) == NULL) {
    perror("mmap error");
    return 0;
  }
  printf("%s\n", file);
  memset(file, 0x00, 40);
  munmap(file, 40);
  close(fd);
  return 0;
}
  
