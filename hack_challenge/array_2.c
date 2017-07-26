#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <stdbool.h>

int main(){
  int arr[6][6];
  int col_step=0;
  int row_step=0;
  int row, col;

  for(int arr_i = 0; arr_i < 6; arr_i++){
    for(int arr_j = 0; arr_j < 6; arr_j++){
      scanf("%d",&arr[arr_i][arr_j]);
    }
  }

  while(1){
    for(int row = row_step ; row < 3 + row_step ; row++ ){
      for(int col = col_step ; col < 3 + col_step; col++ ){
        fprintf(stdout,"%d ",arr[row][col]);
      }
      fprintf(stdout,"\n");
    }
    fprintf(stdout,"\n");

    if(col_step++ == 2 && row_step++ == 2){
      fprintf(stdout,"End");
      break;
    }
  }
  return 0;
}
