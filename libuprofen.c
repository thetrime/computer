#include <SWI-Prolog.h>
#include <SWI-Stream.h>
#include <assert.h>
#include <string.h>
#include "holmes.h"

#define BUFFER_SIZE 8192
extern int read_audio_samples(int16_t* buffer, int buffer_length);
extern void start_recording();
extern void stop_recording();

static
void
sleep_msec(int ms)
{
    struct timeval tmo;
    tmo.tv_sec = 0;
    tmo.tv_usec = ms * 1000;
    select(0, NULL, NULL, NULL, &tmo);
}


static int release_model(atom_t symbol)
{
   model_t* model = PL_blob_data(symbol, NULL, NULL);
   free_model(model);
   return TRUE;
}

static PL_blob_t model_blob =
{ PL_BLOB_MAGIC,
  PL_BLOB_NOCOPY,
  "tensorflow_model",
  release_model,
  NULL,
  NULL,
  NULL
};

foreign_t load_tensorflow_model(term_t Filename, term_t Model)
{
   char* filename;
   context_t* context;
   assert(PL_get_atom_chars(Filename, &filename));
   context = alloc_context(filename, 16000);
   return PL_unify_blob(Model, context, sizeof(*context), &model_blob);
}

static double buffer[1600];

foreign_t wait_for_model(term_t Model, term_t Threshhold)
{
   PL_blob_t *type;
   void *data;
   double threshhold;
   
   if (!PL_get_float(Threshhold, &threshhold))
      return PL_type_error("float", Threshhold);
   if (PL_get_blob(Model, &data, NULL, &type) && type == &model_blob)
   {
      context_t* context = (context_t*)data;
      purge_context(context);
      start_recording();
      while (1)
      {
         int16_t samples[BUFFER_SIZE];
         int sampleCount = read_audio_samples(samples, BUFFER_SIZE);
         assert (sampleCount >= 0);
	 //printf("Samples: %d\n", sampleCount);	 
         if (process_block_int16(context, samples, sampleCount, threshhold))
	 {
	    stop_recording();
            PL_succeed;
	 }
	 sleep_msec(300);
         if (PL_handle_signals() == -1)
	 {
	    stop_recording();
            return FALSE;
	 }
      }
   }
   return PL_type_error("tensorflow_model", Model);
}

install_t install_libuprofen()
{
   PL_register_foreign("load_tensorflow_model", 2, load_tensorflow_model, 0);
   PL_register_foreign("wait_for_model", 2, wait_for_model, 0);
   memset(buffer, 0, sizeof(double)*1600);
}
