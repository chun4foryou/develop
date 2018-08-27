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
#include "fseek.h"

#define STRUCT_TEST
#ifndef STRUCT_TEST
#define  MAX_LEN  30

/**
* @brief 문자열 fseek example
*        원하는 byte만큼 건더 뛴 후 data를 Read한다.
*
* @param argc
* @param argv
*
* @return 
*/
int main(int argc, char **argv) {
  FILE *fp = NULL;
  char buffer[MAX_LEN+1];
  int  result = 0;
  int  i = 0, nbyte = 0;
  char ch = 0;

  fp = fopen("./fseek_test.txt", "r");
  if (fp == NULL) {
    return 0;
  }

  // 건너뛸 바이트 수 설정
  nbyte = 1;
  /* moves the pointer after the N byte */
  result = fseek(fp, nbyte, SEEK_SET); 
  if (result == 0) {
    printf("Pointer successfully moved to the beginning of the file.\n");
  } else {
    printf("Failed moving pointer to the beginning of the file.\n");
  }

  for (i = 0; (i  < (sizeof(buffer)) &&
        ((ch = fgetc(fp)) != EOF)); i++) {
    buffer[i] = ch;
    fprintf(stderr, "%c", buffer[i]);
  }
  fclose(fp);
  return 0;
}

#else 



/**
* @brief 구조체 fseek example
*        원하는 구조체 갯수 만큼 건더 뛴 후 data를 Read한다.
*
* @param argc
* @param argv
*
* @return 
*/
int main(int argc, char **argv) {
  FILE *fp;
  int  i = 0, result = 0, skip_data_cnt = 0;
  DATA data[10];  // write할 구조체 배열 생성

  fprintf(stderr, "%zu\n", sizeof(DATA));

  result = write_struct("./fseek_struct_test");
  if (result != 0 ) {
    return 0;
  }

  fp = fopen("./fseek_struct_test", "rb");
  if (fp == NULL) {
    return 0;
  }

  // 건너뛸 데이터 갯수 
  skip_data_cnt = 0;
  /* moves the pointer after the N byte */
  result = fseek(fp, skip_data_cnt * sizeof(DATA) , SEEK_SET);  
  if (result == 0) {
    printf("Pointer successfully moved to the beginning of the file.\n");
  } else {
    printf("Failed moving pointer to the beginning of the file.\n");
  }

  // 문자열 데이터 읽기
  for (i = 0 ; i < 10; i++) {
    if (fread((char *)&data[i], 1, sizeof(DATA), fp) == sizeof(DATA)) {
      fprintf(stderr, "name [%s]\n", data[i].name);
      fprintf(stderr, "value[%d]\n", data[i].value);
    } else { 
      fprintf(stderr, "Size is Invalied\n");
    }
  }
  fclose(fp);
  return 0;
}

/**
* @brief Sample Data file 생성
*
* @param file_name
*
* @return 
*/
int write_struct(char *file_name) {
  FILE *fp = NULL;
  int i = 0, result = 0;
  DATA data[10];  // write할 구조체 배열 생성

  // 기존 파일 삭제
  unlink("./fseek_struct_test");
  fp = fopen("./fseek_struct_test", "ab");
  if (fp == NULL) {
    return -1;
  } 

  // 구조체 데이터 읽기  
  memset(data, 0, sizeof(data));
  for (i = 0; i < 10; i++) {
    snprintf(data[i].name, sizeof(data[i].name),"Struct %d",i);
    data[i].value = i;
    result = fwrite((char *)&data[i], sizeof(DATA), 1, fp);
    if (result == 0) {
      break;
    }
  }
  fclose(fp);
  return 0;
}
#endif 
