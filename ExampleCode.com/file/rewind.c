/* Copyright (C) 
* 2018 - doitnow-man
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

int main(int argc, char** argv) {
  FILE *fp = NULL;

  fp = fopen("./test.txt", "w");
  if (fp == NULL)  {
    perror("Error File Writing : ");
    return 0;
  }

  fprintf(fp, "First Test Code!!!\n");

  // ReWind 수행 
  // 함수 원형 void rewind ( FILE * stream );
  rewind(fp);

  fprintf(fp, "Seconde Test Code!!!\n");
  fclose(fp);
  return 0;
}

