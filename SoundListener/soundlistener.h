#ifndef __SOUNDLISTENER_H__
#define __SOUNDLISTENER_H__

#include <julius/juliuslib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int getConnect(int port);
int getFileSize(int sock);
char *getSoundFile(int sock, int filesize, FILE* fp, char *buffer);

void put_hypo_phoneme(WORD_ID *seq, int n, WORD_INFO *winfo);
void output_result(Recog *recog, void *dummy);
void status_recready(Recog *recog, void *dummy);
void status_recstart(Recog *recog, void *dummy);

#endif
