% Helpful hints
% https://sourceforge.net/p/cmusphinx/discussion/help/thread/61e35142/

/* Major tasks:

*/

:-module(computer,
	 [computer/0]).

:-use_foreign_library(sphinx).

computer:-
	on_signal(term, _, halt),
	init_sphinx(computer, computer),
	main_loop.

main_loop:-
	wait_for_keyword(computer),
	listen_for_utterance(UtteranceTokens, Confidence),
	writeln(Confidence),
	parse_utterance(UtteranceTokens, Command),
	effect_command(Command),
	halt,
	main_loop.

effect_command(Command):-
	writeln(effect(Command)).

parse_utterance(Tokens, parse_tree(Tokens)).
	
    
