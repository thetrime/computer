#include <flite/flite.h>
#include <SWI-Prolog.h>

void* register_cmu_us_awb(void*);

atom_t ATOM_stretch;
static cst_voice *voice = NULL;

typedef struct
{
   double stretch;
} say_options_t;


void init_options(say_options_t* options)
{
   options->stretch = 1;
}

int parse_options(term_t Options, say_options_t* options)
{
   term_t tail = PL_copy_term_ref(Options);
   term_t head = PL_new_term_ref();
   while (PL_get_list(tail, head, tail))
   {
      atom_t name;
      int arity;
      term_t arg = PL_new_term_ref();
      if (!PL_get_name_arity(head, &name, &arity) || arity != 1)
         return PL_type_error("option", head);
      if (!PL_get_arg(1, head, arg))
         return FALSE;

      if (name == ATOM_stretch)
      {
         if (!PL_get_float_ex(arg, &options->stretch))
            return FALSE;
      }
   }
   if (!PL_get_nil(tail))
      return PL_type_error("list", tail);
   return 1;
}

foreign_t say(term_t Phrase, term_t Options)   
{
   char* phrase;
   cst_features *f = new_features();
   say_options_t options;
   init_options(&options);

   if (!PL_get_chars(Phrase, &phrase, CVT_ATOM | CVT_STRING | BUF_DISCARDABLE))
      return PL_type_error("atom", Phrase);
   if (!parse_options(Options, &options))
      return FALSE;
   flite_feat_set_float(f, "duration_stretch", options.stretch);
   feat_copy_into(f, voice->features);
   delete_features(f);
   flite_text_to_speech(phrase, voice, "play");
   PL_succeed;
}

install_t install_flite()
{
    flite_init();
    voice = register_cmu_us_awb(NULL);
    PL_register_foreign("say", 2, say, 0);
    ATOM_stretch = PL_new_atom("stretch");
}
