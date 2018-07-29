#include <SWI-Prolog.h>

predicate_t process_query = 0;

int init_prolog(int argc, char **argv)
{
   if (!PL_initialise(argc, argv))
      return 0;
   process_query = PL_predicate("process_query", 1, "parser");
}

term_t tokenize(const char* input, term_t tokens)
{
   term_t list = PL_copy_term_ref(tokens);
   term_t item = PL_new_term_ref();
   const char* base = input;
   int i = 0;
   while (*base != 0)
   {
      if (base[i] == ' ')
      {
	 if (!PL_unify_list(list, item, list) || !PL_unify_atom_nchars(item, i, (char*)base))
	    return 0;
	 base += i+1;
      }
   }
   return 1;
}

int effect_request(const char* request)
{
   term_t tokens = PL_new_term_ref();
   if (!tokenize(request, tokens))
      return 0;
   qid_t query = PL_open_query(NULL, PL_Q_NORMAL, process_query, tokens);
   int rc = PL_next_solution(query);
   PL_close_query(query);
   return rc;
}
