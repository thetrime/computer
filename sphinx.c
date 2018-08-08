#include <pocketsphinx.h>
#include <sphinxbase/ad.h>
#include <assert.h>
#include <ctype.h>
#include <SWI-Prolog.h>
#include <SWI-Stream.h>

#define MODELDIR "/opt/sphinx/share/pocketsphinx/model"
#define MODEL "computer"
#define BUFSIZE 8192

static ps_decoder_t *ps = NULL;
ad_rec_t *microphone = NULL;

static
void
sleep_msec(int32 ms)
{
    struct timeval tmo;
    tmo.tv_sec = 0;
    tmo.tv_usec = ms * 1000;
    select(0, NULL, NULL, NULL, &tmo);
}

static
int tokenize(const char* input, term_t Tokens)
{
   term_t list = PL_copy_term_ref(Tokens);
   term_t item = PL_new_term_ref();
   const char* base = input;
   char *p, *q;
   char lcbuf[256];
   int i = 0;
   int j = 0;
   while (1)
   {
      if (base[i] == ' ' || base[i] == 0)
      {
	 if (!PL_unify_list(list, item, list))
	    return 0;
	 // Downcase the string
	 assert(i < 256);
	 for (j = 0; j < i; j++)
	    lcbuf[j] = tolower(base[j]);
	 // Then unify it
	 if (!PL_unify_atom_nchars(item, i, lcbuf))
	    return 0;
	 if (base[i] == 0)
	    break;
	 base += i+1;
	 i=0;
      }
      i++;
   }
   return PL_unify_nil(list);
}

foreign_t wait_for_keyword(term_t Keyword)
{
   int16 buffer[BUFSIZE];
   char* keyword;
   int32 score;
   assert(PL_get_atom_chars(Keyword, &keyword));
   ps_set_search(ps, "kws");   
   int rv = ps_start_utt(ps);
   Sdprintf("Ready!\n");
   while (1)
   {
      int32 bytes_read = ad_read(microphone, buffer, BUFSIZE);
      assert(bytes_read >= 0);
      /*
      if (bytes_read == 0)
      {
	 //sleep_msec(200);
	 continue;
      }
      */
      ps_process_raw(ps, buffer, bytes_read, FALSE, FALSE);
      const char* hypothesis = ps_get_hyp(ps, &score);
      if (hypothesis != NULL && strcasecmp(hypothesis, keyword) == 0)
      {
           ps_end_utt(ps);
	   Sdprintf("*** Keyword detected!\n");
	   PL_succeed;
      }
      //Sdprintf("Heard something in those %d bytes, but it was not %s (%s)\n", bytes_read, keyword, hypothesis);
      sleep_msec(10);
   }
   // End of buffer but no keyword detected
   Sdprintf("failed to find keyword\n");
   PL_fail;
}

foreign_t listen_for_utterance(term_t Tokens, term_t Score)
{
   int16 buffer[BUFSIZE];
   char const *hypothesis;
   int32 score;
   int rc;
   ps_set_search(ps, "grammar");
   int rv = ps_start_utt(ps);
   int started_speaking = 0;
   while (1)
   {
      int32 bytes_read = ad_read(microphone, buffer, BUFSIZE);
      assert(bytes_read >= 0);
      rv = ps_process_raw(ps, buffer, bytes_read, FALSE, FALSE);
      int is_speaking_now = ps_get_in_speech(ps);
      if (!started_speaking && is_speaking_now)
      {
	 // They have started. Once they pause for a bit, we assume they have finished
	 started_speaking = 1;
      }
      else if (started_speaking && !is_speaking_now)
      {
	 // Looks like that is it!
	 break;
      }
      //sleep_msec(100);
   }
   rv = ps_end_utt(ps);
   hypothesis = ps_get_hyp(ps, &score);
   Sdprintf("*** utterance detected. Tokenizing...\n");
   rc = tokenize(hypothesis, Tokens);
   Sdprintf("*** tokenzied!\n");
   return PL_unify_integer(Score, score);
}

install_t install_sphinx()
{
   PL_register_foreign("wait_for_keyword", 1, wait_for_keyword, 0);
   PL_register_foreign("listen_for_utterance", 2, listen_for_utterance, 0);

   /* Also, initialize CMU Sphinx */
   cmd_ln_t *config;
   config = cmd_ln_init(NULL, ps_args(), TRUE,
			"-hmm", MODELDIR "/en-us/en-us",
//			"-lm", MODEL ".lm",
			"-dict", MODEL ".dic",
//			"-kws", "computer",
			"-kws_threshold", "1e-80",
			NULL);
   assert(config != NULL);

   ps = ps_init(config);

   ps_set_kws(ps, "kws", MODEL ".name");
   ps_set_lm_file(ps, "grammar", MODEL ".lm");
   
   assert(ps != NULL);

   microphone = ad_open_dev("plughw:1", 16000);
   assert(microphone != NULL);

   assert(ad_start_rec(microphone) >= 0);

   // FIXME: We do not free the cmd_ln anywhere. Might be OK for continuous usage
   // FIXME: This cmd_ln_* stuff is deprecated in favour of a re-entrant API
      

}
