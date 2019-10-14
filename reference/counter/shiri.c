#include <stdio.h>
#include <string.h>

#define MAX 30

int main(int argc, char *argv[]){
char data[MAX][200] = {0};/* 入力データ保存用 */
int i,nIdx,n;

for(i=0;i<MAX;i++){
printf("%02d番目：",i+1);
fgets(data[i], sizeof(data[i]), stdin);
n=strlen(data[i])-3; /* 文末の位置 */
if( memcmp( &data[i][n], "ん", 2) == 0){
printf("んが付いたから終了\n" );
break;
}
if( i!=0 && /* 初回は比較しない */ 
memcmp( &data[i-1][nIdx], &data[i][0], 2) != 0){ /* 今回の文頭と前回の文末を比較*/
printf("つながらないから終了\n" );
break;;
}
nIdx =n;
}
if(i==MAX){
printf("%d回で終了\n",MAX );
}
return 0;
}
