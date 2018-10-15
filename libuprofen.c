#include <SWI-Prolog.h>
#include <SWI-Stream.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include "holmes.h"
#include "wav.h"

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

// Keep a buffer of the last 3 seconds
static int16_t buffer[16000 * 3];
static int buffer_ptr = 0;

static void save_activation()
{
   time_t current_time;
   char buffer[1024];
   struct tm* tm_info;

   time(&current_time);
   tm_info = localtime(&current_time);
   strftime(buffer, 1024, "/tmp/activation-%Y-%m-%d %H:%M:%S.wav", tm_info);
   FILE* fd = fopen(buffer, "wb");
   assert(fd != NULL);
   wav_header_t header;
   memcpy(&header.riff_header, "RIFF", 4);
   header.wav_size = (sizeof(int16_t) * 16000 * 3) - 8;
   memcpy(&header.wave_header, "WAVE", 4);
   memcpy(&header.fmt_header, "fmt ", 4);
   header.fmt_chunk_size = 16;
   header.audio_format = 1;
   header.num_channels = 1;
   header.sample_rate = 16000;
   header.byte_rate = 16000 * 1 * sizeof(int16_t);
   header.sample_alignment = sizeof(int16_t) * 1;
   header.bit_depth = 8 * sizeof(int16_t);
   memcpy(&header.data_header, "data", 4);
   header.data_bytes = 16000 * 3 * 1 * sizeof(int16_t);
   fwrite(&header, sizeof(wav_header_t), 1, fd);
   Sdprintf("Writing %d records from %d\n", (16000 * 3) - buffer_ptr, buffer_ptr);
   fwrite(&buffer[buffer_ptr], sizeof(int16_t), (16000 * 3) - buffer_ptr, fd);
   Sdprintf("Writing %d records from the start\n", buffer_ptr);
   fwrite(buffer, sizeof(int16_t), buffer_ptr, fd);
   fclose(fd);
}

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
         // Store the samples in the (circular) buffer
         int n = 0;
         while (n < sampleCount)
         {
            int buffer_space = 3*16000 - buffer_ptr;
            if (buffer_space < (sampleCount - n))
            {
               memcpy(&buffer[buffer_ptr], &samples[n], buffer_space * sizeof(int16_t));
               buffer_ptr = 0;
               n += buffer_space;
            }
            else
            {
               memcpy(&buffer[buffer_ptr], &samples[n], (sampleCount - n) * sizeof(int16_t));
               buffer_ptr += (sampleCount - n);
               n += (sampleCount - n);
            }
         }
         if (process_block_int16(context, samples, sampleCount, threshhold))
	 {
            stop_recording();
            save_activation();
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
   memset(buffer, 0, sizeof(int16_t)*3*16000);
}
