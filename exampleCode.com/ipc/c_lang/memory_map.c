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

  // memory에 mapping할 file 열기
  if ((fd = open(argv[1], O_RDWR|O_CREAT)) < 0) {
    perror("File Open Error");
    return 0;
  }

  // 파일크기 구하기 
  // memory mapping을 위한 메모리 할당을 위한 용도
  if (fstat(fd, &sb) < 0) {
    perror("fstat error");
    return 0;
  }

  printf("file size[%zu]\n", sb.st_size);

  file = (char *)malloc(sb.st_size);
  // mmap를 이용해서 열린 파일을 메모리에 대응시킨다.
  // file은 대응된 주소를 가리키고, file을 이용해서 필요한 작업을
  // 하면 된다.
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
  
