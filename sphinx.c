#include <pocketsphinx.h>
#include <sphinxbase/ad.h>
#include <sphinxbase/err.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <SWI-Prolog.h>
#include <SWI-Stream.h>

#define MODELDIR "/opt/sphinx/share/pocketsphinx/model"
#define HMM "/en-us/en-us"
//#define HMM "/voxforge.cd_ptm_5000"
#define MODEL "computer"
#define BUFSIZE 8192

static ps_decoder_t *ps = NULL;
ad_rec_t *microphone = NULL;
static char* current_dictionary = NULL;

static
void
sleep_msec(int32 ms)
{
    struct timeval tmo;
    tmo.tv_sec = 0;
    tmo.tv_usec = ms * 1000;
    select(0, NULL, NULL, NULL, &tmo);
}

static void log_cb(void* ignored, err_lvl_t level, const char* format, ...)
{
   
}

static void install_grammar(const char* name, const char* model)
{
   char* filename = PL_malloc(strlen(model) + 4);
   strcpy(filename, model);
   strcat(filename, ".lm");
   ps_set_lm_file(ps, name, filename);
   PL_free(filename);
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
   if (strcmp(current_dictionary, "default.dic") != 0)
       ps_load_dict(ps, "default.dic", NULL, NULL);
   ps_set_search(ps, "kws");
   int rv = ps_start_utt(ps);
   Sdprintf("Ready!\n");
   assert(ad_start_rec(microphone) >= 0);
   while (1)
   {
      int32 bytes_read = ad_read(microphone, buffer, BUFSIZE);
      assert(bytes_read >= 0);
      ps_process_raw(ps, buffer, bytes_read, FALSE, FALSE);
      const char* hypothesis = ps_get_hyp(ps, &score);
      if (hypothesis != NULL && strcasecmp(hypothesis, keyword) == 0)
      {
           ps_end_utt(ps);
	   Sdprintf("*** Keyword detected!\n");
	   assert(ad_stop_rec(microphone) >= 0);   
	   PL_succeed;
      }
      //Sdprintf("Heard something in those %d bytes, but it was not %s (%s)\n", bytes_read, keyword, hypothesis);
      //sleep_msec(10);
   }
   // End of buffer but no keyword detected
   Sdprintf("failed to find keyword\n");
   assert(ad_stop_rec(microphone) >= 0);   
   PL_fail;
}

foreign_t retry_last_utterance(term_t Model, term_t Tokens, term_t Score)
{
   char* model;
   assert(PL_get_atom_chars(Model, &model));
   FILE* retry_buffer = fopen("/tmp/retry.raw", "rb");
   int16 buffer[BUFSIZE];
   int32 score;
   int rv, rc;
   char const *hypothesis;

   // FIXME: This is obviously terrible
   install_grammar("retry", model);
   ps_set_search(ps, "retry");   
   ps_load_dict(ps, "weather.dic", NULL, NULL);
   current_dictionary = "weather.dic";
   rv = ps_start_utt(ps);   
   while (!feof(retry_buffer))
   {
      size_t samples_read;
      samples_read = fread(buffer, 2, BUFSIZE, retry_buffer);
      ps_process_raw(ps, buffer, samples_read, FALSE, FALSE);
   }
   fclose(retry_buffer);
   rv = ps_end_utt(ps);
   hypothesis = ps_get_hyp(ps, &score);
   Sdprintf("*** utterance detected. Tokenizing...\n");
   rc = tokenize(hypothesis, Tokens);
   Sdprintf("*** tokenzied. Probability is %d, score is %d\n", ps_get_prob(ps), score);
   return PL_unify_integer(Score, score);
}

foreign_t listen_for_utterance(term_t Tokens, term_t Score)
{
   int16 buffer[BUFSIZE];
   char const *hypothesis;
   int32 score;
   int rc;
   assert(ad_start_rec(microphone) >= 0);   
   ps_set_search(ps, "default");
   if (strcmp(current_dictionary, "default.dic") != 0)
       ps_load_dict(ps, "default.dic", NULL, NULL);
   current_dictionary = "default.dic";
   int rv = ps_start_utt(ps);   
   int started_speaking = 0;

   // This file can be played back using a command like: aplay -f S16_LE -r 16000 /tmp/retry.raw 
   FILE* retry_buffer = fopen("/tmp/retry.raw", "wb");
   int total_samples = 0;
   Sdprintf("Listening now...\n");
   while (1)
   {
      int32 samples_read = ad_read(microphone, buffer, BUFSIZE);
      total_samples += samples_read;
      assert(samples_read >= 0);
      rv = ps_process_raw(ps, buffer, samples_read, FALSE, FALSE);
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
      else if (total_samples > 60 * 16000)
      {
	 // 60 second limit to avoid creating a huge retry buffer. This is ~2MB
	 break;
      }
      // Log the utterances to disk in case we want to redo the recognition later
      // Each sample is 16 bits, remember
      fwrite(buffer, samples_read, 2, retry_buffer);
      if (PL_handle_signals() == -1)
      {
	 fclose(retry_buffer);
	 assert(ad_stop_rec(microphone) >= 0);   
	 return FALSE;
      }
      // Sleep for a bit so we do not crowd the CPU
      sleep_msec(100);

   }
   fclose(retry_buffer);
   rv = ps_end_utt(ps);
   hypothesis = ps_get_hyp(ps, &score);
   Sdprintf("*** utterance detected. Tokenizing...\n");
   rc = tokenize(hypothesis, Tokens);
   Sdprintf("*** tokenzied. Probability is %d, score is %d\n", ps_get_prob(ps), score);
   assert(ad_stop_rec(microphone) >= 0);   
   return PL_unify_integer(Score, score);
}

foreign_t init_sphinx(term_t Model, term_t Name, term_t Threshold)
{
   char* name;
   char* model;
   double threshold;
   char threshold_string[128];
   assert(PL_get_atom_chars(Name, &name));
   assert(PL_get_atom_chars(Model, &model));
   assert(PL_get_float(Threshold, &threshold));
   snprintf(threshold_string, 128, "%g", threshold);
   
   /* Upper-case the name */
   char* name_uc = PL_malloc(strlen(name));
   name_uc[strlen(name)] = '\0';
   for (char* source = name, *dest = name_uc; *source; source++, dest++)
      *dest = toupper(*source);
   
   Sdprintf("Hello, my name is %s\n", name_uc);
   /* initialize CMU Sphinx */
//   err_set_callback(log_cb, NULL);   
   cmd_ln_t *config;
   config = cmd_ln_init(NULL, ps_args(), TRUE,
			"-hmm", MODELDIR HMM,
//			"-dict", "master_dictionary",
			"-kws_threshold", threshold_string,
			NULL);
   assert(config != NULL);
   ps = ps_init(config);

   ps_load_dict(ps, "default.dic", NULL, NULL);
   current_dictionary = "default.dic";
   ps_set_keyphrase(ps, "kws", name_uc);
   install_grammar("default", model);
   PL_free(name_uc);
   assert(ps != NULL);

   microphone = ad_open_dev("plughw:1", 16000);
   assert(microphone != NULL);

   
   // FIXME: We do not free the cmd_ln anywhere. Might be OK for continuous usage
   // FIXME: This cmd_ln_* stuff is deprecated in favour of a re-entrant API
   PL_succeed;
}

void stop_recording()
{
  assert(ad_stop_rec(microphone) >= 0);   
}

void start_recording()
{
  assert(ad_start_rec(microphone) >= 0);
}

int read_audio_samples(int16_t* buffer, int buffer_length)
{
   assert(microphone != NULL);
   return ad_read(microphone, buffer, buffer_length);
}


install_t install_sphinx()
{
   PL_register_foreign("wait_for_keyword", 1, wait_for_keyword, 0);
   PL_register_foreign("listen_for_utterance", 2, listen_for_utterance, 0);
   PL_register_foreign("retry_last_utterance", 3, retry_last_utterance, 0);
   PL_register_foreign("init_sphinx", 3, init_sphinx, 0);
}
