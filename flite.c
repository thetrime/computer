#include <flite/flite.h>
#include <SWI-Prolog.h>

void* register_cmu_us_awb(void*);

static cst_voice *voice = NULL;

foreign_t say(term_t Phrase, term_t Options)   
{
   char* phrase;
   cst_features *f = new_features();

   if (!PL_get_chars(Phrase, &phrase, CVT_ATOM | CVT_STRING | BUF_DISCARDABLE))
      return PL_type_error("atom", Phrase);

   //flite_feat_set_float(f, "duration_stretch", 1);
   //feat_copy_into(f, voice->features);
   delete_features(f);
   flite_text_to_speech(phrase, voice, "play");
   PL_succeed;
}

install_t install_flite()
{
    flite_init();
    voice = register_cmu_us_awb(NULL);
    PL_register_foreign("say", 2, say, 0);
}
