#include "soundlistener.h"

int getConnect(int port){
  struct sockaddr_in addr;
  struct sockaddr_in client;
  int len;
  int sock, sock0;
  int yes = 1;

  sock0 = socket(AF_INET, SOCK_STREAM, 0);

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;
  //make it available immediately to connect
  setsockopt(sock0,SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));
  bind(sock0, (struct sockaddr *)&addr, sizeof(addr));
  listen(sock0, 5);
  len = sizeof(client);

  sock = accept(sock0, (struct sockaddr *)&client, &len);
  if(sock == -1){
    printf("ERROR: socket cannot accept\n");
    return -1;
  }

  return sock;
}

int getFileSize(int sock){
  int filesize = 0;

  // recive data of file size	
  if(recv(sock, &filesize, sizeof(filesize), 0) == -1){
    printf("ERROR: can not recieve info\n");
    return -1;
  }

  return filesize;
}

char *getSoundFile(int sock, int filesize, FILE* fp, char*soundData){
  int recieveSize = 0, recieveSum = 0;


  while(1){
    // recive data of file size
    recieveSize = recv(sock, soundData + recieveSum, filesize - recieveSum, 0);
    printf("recieve size: %d\n", recieveSize);
    if(recieveSize == -1 || recieveSize == 0){
      printf("ERROR: can not recieve data\n");
      return -1;
    }
    recieveSum += recieveSize;
    if(recieveSum == filesize*2)
      break;
    else if(recieveSum > filesize){
      printf("ERROR: recive data over\n");
      return -1;
    }
  }
  /* write sound data */
  fwrite(soundData, sizeof( char ), filesize*2, fp);
  return soundData;
}
