#include <pocketsphinx.h>
#include <assert.h>
#include <SWI-Prolog.h>
#include <SWI-Stream.h>

#define MODELDIR "/opt/sphinx/share/pocketsphinx/model"
#define MODEL "1698"

static ps_decoder_t *ps = NULL;
FILE* test_fd = NULL;

int tokenize(const char* input, term_t Tokens)
{
   term_t list = PL_copy_term_ref(Tokens);
   term_t item = PL_new_term_ref();
   const char* base = input;
   int i = 0;
   while (1)
   {
      if (base[i] == ' ' || base[i] == 0)
      {
	 Sdprintf("Token found from %s of length %d\n", base, i);
	 if (!PL_unify_list(list, item, list) || !PL_unify_atom_nchars(item, i, (char*)base))
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
   int16 buffer[512];
   char* keyword;
   int32 score;
   assert(PL_get_atom_chars(Keyword, &keyword));
   ps_set_search(ps, "kws");   
   int rv = ps_start_utt(ps);
   while (!feof(test_fd))
   {
      size_t sample_count = fread(buffer, 2, 512, test_fd);      
      ps_process_raw(ps, buffer, sample_count, FALSE, FALSE);
      const char* hypothesis = ps_get_hyp(ps, &score);
      Sdprintf("Read %d bytes. Comparing %s and %s\n", sample_count, hypothesis, keyword);
      if (hypothesis != NULL && strcmp(hypothesis, keyword) == 0)
      {
           ps_end_utt(ps);
	   Sdprintf("*** Keyword detected!\n");
	   PL_succeed;
      }
   }
   // End of buffer but no keyword detected
   Sdprintf("failed to find keyword\n");
   PL_fail;
}

foreign_t listen_for_utterance(term_t Tokens, term_t Score)
{
   int16 buffer[512];
   char const *hypothesis;
   int32 score;
   int rc;
   ps_set_search(ps, "grammar");
   int rv = ps_start_utt(ps);
   while (!feof(test_fd))
   {
      size_t sample_count = fread(buffer, 2, 512, test_fd);
      rv = ps_process_raw(ps, buffer, sample_count, FALSE, FALSE);
   }
   rv = ps_end_utt(ps);
   hypothesis = ps_get_hyp(ps, &score);
   fclose(test_fd);   
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
//			"-kws_threshold", "1e-5",
			NULL);
   assert(config != NULL);

   ps = ps_init(config);

   ps_set_kws(ps, "kws", MODEL ".name");
   ps_set_lm_file(ps, "grammar", MODEL ".lm");
   
   assert(ps != NULL);
   test_fd = fopen("goforward.raw", "rb");
   assert(test_fd != NULL);

   // FIXME: We do not free the cmd_ln anywhere. Might be OK for continuous usage
   // FIXME: This cmd_ln_* stuff is deprecated in favour of a re-entrant API
      

}
