/* Copyright (C) 
* 2018 - chun4foryou
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
* 
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

int main(void) {
  int     fd[2], nbytes, rc = 0;
  pid_t   childpid;
  char    string[] = "Hello, world!\n";
  char    readbuffer[80];

  if ((rc = pipe(fd)) < 0) {
    printf("Creating Pipe is Error [%d]\n", rc);
  }

  if((childpid = fork()) == -1) {
    perror("fork");
    return 0;
  }

  if (childpid == 0) {
    /* 자식 프로세스는 Write할꺼기에 Read FD는 닫아준다 */
    close(fd[0]);

    /* Pipe에 메시지 보내기 */
    write(fd[1], string, (strlen(string)+1));
    return 0;
  } else {
    /* 부모 프로세스는 Read할꺼기에 Write FD는 닫아준다 */
    close(fd[1]);

    /* Pipe에서 메시지 읽기 */
    nbytes = read(fd[0], readbuffer, sizeof(readbuffer));
    printf("Received Parent string: %s [%d]", readbuffer, nbytes);
  }

  return 0;
}
