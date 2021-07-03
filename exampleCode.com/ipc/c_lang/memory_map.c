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

  // memory�� mapping�� file ����
  if ((fd = open(argv[1], O_RDWR|O_CREAT)) < 0) {
    perror("File Open Error");
    return 0;
  }

  // ����ũ�� ���ϱ� 
  // memory mapping�� ���� �޸� �Ҵ��� ���� �뵵
  if (fstat(fd, &sb) < 0) {
    perror("fstat error");
    return 0;
  }

  printf("file size[%zu]\n", sb.st_size);

  file = (char *)malloc(sb.st_size);
  // mmap�� �̿��ؼ� ���� ������ �޸𸮿� ������Ų��.
  // file�� ������ �ּҸ� ����Ű��, file�� �̿��ؼ� �ʿ��� �۾���
  // �ϸ� �ȴ�.
  if ((file = (char *) mmap(0, sb.st_size, flag, MAP_SHARED, fd, 0)) == NULL) {
    perror("mmap error");
    return 0;
  }

  printf("%s\n", file);
//  memset(file, 0, sb.st_size);
  snprintf(file,sb.st_size, "hello");
  munmap(file, sb.st_size);
  close(fd);
  return 0;
}
  
