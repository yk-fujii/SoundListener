#include "soundlistener.h"

#define RECOGNITION_DISABLE

#define MAX_COUNT 10000000
#define MAX_BUFFER_SIZE 500

int main(int argc, char *argv[])
{

  Jconf *jconf; /* configuration parameter holder*/
  Recog *recog; /* Recognition instance */

 // static char speechfilename[MAXPATHLEN]; /* speechfile name for MFCC file input*/
  int ret;

  int sock; /* socket */
  FILE* fp; /* FILE pointer */
  int filesize = 0;
  int count = 0; /* recieve counter */

  char *buffer;
  buffer = (char*)malloc( MAX_BUFFER_SIZE * sizeof(char));
  static char *soundData;
  soundData = (char *)malloc(sizeof(char)*filesize);


#ifndef RECOGNITION_DISABLE
  /* if no argument, output usage and exit */
  if (argc == 1) {
    fprintf(stderr, "Julius rev.%s - based on ", JULIUS_VERSION);
    j_put_version(stderr);
    fprintf(stderr, "Try '-setting' for built-in engine configuration.\n");
    fprintf(stderr, "Try '-help' for run time options.\n");
    return -1;
  }

 
  /* 1. load configurations from command arguments */
  jconf = j_config_load_args_new(argc, argv);
  /* else, you can load configurations from a jconf file */
  //jconf = j_config_load_file_new(jconf_filename);
  if (jconf == NULL) {		/* error */
    fprintf(stderr, "Try `-help' for more information.\n");
    return -1;
  }

  /* 2. create recognition instance according to the jconf */
  /* it loads models, setup final parameters, build lexicon
     and set up work area for recognition */
  recog = j_create_instance_from_jconf(jconf);
  if (recog == NULL) {
    fprintf(stderr, "Error in startup\n");
    return -1;
  }

  /* register result callback functions */
 // callback_add(recog, CALLBACK_EVENT_SPEECH_READY, status_recready, NULL);
 // callback_add(recog, CALLBACK_EVENT_SPEECH_START, status_recstart, NULL);
 // callback_add(recog, CALLBACK_RESULT, output_result, NULL);

  /* initialize audio input device */
  /* ad-in thread starts at this time for microphone */
  if (j_adin_init(recog) == FALSE) {    /* error */
    return -1;
  }

  /* output system information to log */
  j_recog_info(recog);
#endif


  /*
   * SOCKET INITIALIZATIOn START
   */

  // open the file
  fp = fopen("rec.raw", "wb" );
  if( fp == NULL ){
    fputs( "ERROR: File\n", stderr );
    exit( EXIT_FAILURE );
  }

  fprintf(stdout,"Connection Waiting....\n");
  // connect client	
  sock = getConnect(12345);
  printf("connected\n");

  /***********************************/
  /* Open input stream and recognize */
  /***********************************/

  /* raw speech input (microphone etc.) */
#ifndef RECOGNISTION_DISABLE
 switch(j_open_stream(recog, NULL)) {
    case 0:			/* succeeded */
      break;
    case -1:      		/* error */
      fprintf(stderr, "error in input stream\n");
      return -1;
    case -2:			/* end of recognition process */
      fprintf(stderr, "failed to begin input stream\n");
      return -2;
  }
#endif

  while(count < MAX_COUNT){
    // get file size
    filesize = getFileSize(sock);
    if(filesize == -1){
      printf("ERROR\n");
      break;
    }
    else{
      printf("filesize*aaa", filesize);
    }

    // get sound data
    if((getSoundFile(sock, filesize, fp, buffer)) == -1){
      break;
    }
    count++;

#ifndef RECOGNITION_DISABLE
    // RAWデータをアドインバッファへコピー
    memcpy(recog->adin->buffer, buffer, (filesize/sizeof(SP16))*sizeof(SP16));
    // // アドインパラメータを設定
    recog->adin->bp = (filesize/sizeof(SP16));
    recog->adin->sblen = 0; // 認識開始位置を先頭にする
    recog->adin->is_valid_data = FALSE; // 繰り返し処理する場合に必要
    recog->adin->end_of_stream = TRUE; // stream処理は終わった事にする
    recog->adin->input_side_segment = TRUE; // 認識処理を開始させる
    recog->adin->need_init = FALSE; // パラメータを勝手に初期化しないよう指示

    printf("recog->adin->bp%d\n",recog->adin->bp);
 

    /**********************/
    /* Recognization Loop */
    /**********************/
    /* enter main loop to recognize the input stream */
    /* finish after whole input has been processed and input reaches end */
    ret = j_recognize_stream(recog);
    if (ret == -1) return -1;	/* error */
#endif
  }  

#ifndef RECOGNITION_DISABLE
  /* calling j_close_stream(recog) at any time will terminate
     recognition and exit j_recognize_stream() */
  j_close_stream(recog);
  j_recog_free(recog);
#endif

  fclose(fp);
  close(sock);

  /* exit program */
  return(0);
}
