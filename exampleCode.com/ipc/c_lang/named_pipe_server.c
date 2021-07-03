#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define MSG_SIZE 80
#define PIPENAME "./named_pipe_file"

int main(void) {
  char msg[MSG_SIZE];
  int fd;
  int nread, rc;

  /* ������ named pipe�� ������ ���� */
  if (access(PIPENAME,F_OK) == 0) {
    unlink(PIPENAME);
  }

  /* named pipe �����ϱ� */
  if ((rc = mkfifo(PIPENAME,0666)) < 0) {
    printf("fail to make named pipe\n");
    return 0;
  }

  /* named pipe ����, Read Write���� �ؾ� �Ѵ� */
  if ((fd = open(PIPENAME, O_RDWR)) < 0) {
    printf("fail to open named pipe\n");
    return 0;
  }

  while (1) {
    if ((nread = read(fd, msg, sizeof(msg))) < 0 ) {
      printf("fail to call read()\n");
      return 0;
    }
    printf("recv: %s\n", msg);
  }
  return 0;
}
