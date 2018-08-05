% Helpful hints
% https://sourceforge.net/p/cmusphinx/discussion/help/thread/61e35142/

/* Major tasks:c
   * Downcase keywords for ease of parsing by Prolog
*/

:-module(computer,
	 [computer/0]).

:-use_foreign_library(sphinx).

computer:-
	on_signal(term, _, halt),
	wait_for_keyword('COMPUTER'),
	listen_for_utterance(UtteranceTokens, Confidence),
	writeln(Confidence),
	parse_utterance(UtteranceTokens, Command),
	effect_command(Command).

effect_command(Command):-
	writeln(effect(Command)).

parse_utterance(Tokens, parse_tree(Tokens)).
	
    
