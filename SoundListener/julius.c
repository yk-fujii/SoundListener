#include "soundlistener.h"

/** 
 * Callback to be called when start waiting speech input. 
 * 
 */
void status_recready(Recog *recog, void *dummy)
{
  if (recog->jconf->input.speech_input == SP_MIC || recog->jconf->input.speech_input == SP_NETAUDIO) {
    fprintf(stderr, "<<< please speak >>>");
  }
}

/** 
 * Callback to be called when speech input is triggered.
 * 
 */
void status_recstart(Recog *recog, void *dummy)
{
  if (recog->jconf->input.speech_input == SP_MIC || recog->jconf->input.speech_input == SP_NETAUDIO) {
    fprintf(stderr, "\r                    \r");
  }
}
/** 
 * Sub function to output phoneme sequence.
 * 
 */
void put_hypo_phoneme(WORD_ID *seq, int n, WORD_INFO *winfo)
{
  int i,j;
  WORD_ID w;
  static char buf[MAX_HMMNAME_LEN];

  if (seq != NULL) {
    for (i=0;i<n;i++) {
      if (i > 0) printf(" |");
      w = seq[i];
      for (j=0;j<winfo->wlen[w];j++) {
	center_name(winfo->wseq[w][j]->name, buf);
	printf(" %s", buf);
      }
    }
  }
  printf("\n");  
}

/** 
 * Callback to output final recognition result.
 * This function will be called just after recognition of an input ends
 * 
 */
void output_result(Recog *recog, void *dummy)
{
  int i, j;
  int len;
  WORD_INFO *winfo;
  WORD_ID *seq;
  int seqnum;
  int n;
  Sentence *s;
  RecogProcess *r;
  HMM_Logical *p;
  SentenceAlign *align;

  /* all recognition results are stored at each recognition process
     instance */
  for(r=recog->process_list;r;r=r->next) {

    /* skip the process if the process is not alive */
    if (! r->live) continue;

    /* result are in r->result.  See recog.h for details */

    /* check result status */
    if (r->result.status < 0) {      /* no results obtained */
      /* outout message according to the status code */
      switch(r->result.status) {
      case J_RESULT_STATUS_REJECT_POWER:
	printf("<input rejected by power>\n");
	break;
      case J_RESULT_STATUS_TERMINATE:
	printf("<input teminated by request>\n");
	break;
      case J_RESULT_STATUS_ONLY_SILENCE:
	printf("<input rejected by decoder (silence input result)>\n");
	break;
      case J_RESULT_STATUS_REJECT_GMM:
	printf("<input rejected by GMM>\n");
	break;
      case J_RESULT_STATUS_REJECT_SHORT:
	printf("<input rejected by short input>\n");
	break;
      case J_RESULT_STATUS_REJECT_LONG:
	printf("<input rejected by long input>\n");
	break;
      case J_RESULT_STATUS_FAIL:
	printf("<search failed>\n");
	break;
      }
      /* continue to next process instance */
      continue;
    }

    /* output results for all the obtained sentences */
    winfo = r->lm->winfo;

    for(n = 0; n < r->result.sentnum; n++) { /* for all sentences */

      s = &(r->result.sent[n]);
      seq = s->word;
      seqnum = s->word_num;

      /* output word sequence like Julius */
      printf("sentence%d:", n+1);
      for(i=0;i<seqnum;i++) printf(" %s", winfo->woutput[seq[i]]);
      printf("\n");
      /* LM entry sequence */
      printf("wseq%d:", n+1);
      for(i=0;i<seqnum;i++) printf(" %s", winfo->wname[seq[i]]);
      printf("\n");
      /* phoneme sequence */
      printf("phseq%d:", n+1);
      put_hypo_phoneme(seq, seqnum, winfo);
      printf("\n");
      /* confidence scores */
      printf("cmscore%d:", n+1);
      for (i=0;i<seqnum; i++) printf(" %5.3f", s->confidence[i]);
      printf("\n");
      /* AM and LM scores */
      printf("score%d: %f", n+1, s->score);
      if (r->lmtype == LM_PROB) { /* if this process uses N-gram */
	printf(" (AM: %f  LM: %f)", s->score_am, s->score_lm);
      }
      printf("\n");
      if (r->lmtype == LM_DFA) { /* if this process uses DFA grammar */
	/* output which grammar the hypothesis belongs to
	   when using multiple grammars */
	if (multigram_get_all_num(r->lm) > 1) {
	  printf("grammar%d: %d\n", n+1, s->gram_id);
	}
      }
      
      /* output alignment result if exist */
      for (align = s->align; align; align = align->next) {
	printf("=== begin forced alignment ===\n");
	switch(align->unittype) {
	case PER_WORD:
	  printf("-- word alignment --\n"); break;
	case PER_PHONEME:
	  printf("-- phoneme alignment --\n"); break;
	case PER_STATE:
	  printf("-- state alignment --\n"); break;
	}
	printf(" id: from  to    n_score    unit\n");
	printf(" ----------------------------------------\n");
	for(i=0;i<align->num;i++) {
	  printf("[%4d %4d]  %f  ", align->begin_frame[i], align->end_frame[i], align->avgscore[i]);
	  switch(align->unittype) {
	  case PER_WORD:
	    printf("%s\t[%s]\n", winfo->wname[align->w[i]], winfo->woutput[align->w[i]]);
	    break;
	  case PER_PHONEME:
	    p = align->ph[i];
	    if (p->is_pseudo) {
	      printf("{%s}\n", p->name);
	    } else if (strmatch(p->name, p->body.defined->name)) {
	      printf("%s\n", p->name);
	    } else {
	      printf("%s[%s]\n", p->name, p->body.defined->name);
	    }
	    break;
	  case PER_STATE:
	    p = align->ph[i];
	    if (p->is_pseudo) {
	      printf("{%s}", p->name);
	    } else if (strmatch(p->name, p->body.defined->name)) {
	      printf("%s", p->name);
	    } else {
	      printf("%s[%s]", p->name, p->body.defined->name);
	    }
	    if (r->am->hmminfo->multipath) {
	      if (align->is_iwsp[i]) {
		printf(" #%d (sp)\n", align->loc[i]);
	      } else {
		printf(" #%d\n", align->loc[i]);
	      }
	    } else {
	      printf(" #%d\n", align->loc[i]);
	    }
	    break;
	  }
	}
	
	printf("re-computed AM score: %f\n", align->allscore);

	printf("=== end forced alignment ===\n");
      }
    }
  }

  /* flush output buffer */
  fflush(stdout);
}
