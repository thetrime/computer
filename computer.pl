% Helpful hints
% https://sourceforge.net/p/cmusphinx/discussion/help/thread/61e35142/

/* Major tasks:

*/

:-module(computer,
	 [computer/0]).

:-use_foreign_library(sphinx).
:-use_foreign_library(flite).

computer:-
	on_signal(term, _, halt),
	init_sphinx(default, computer, 1e-30),
	main_loop.

main_loop:-
	wait_for_keyword(computer),
	say('Aye, what is it now?', []),
	writeln(listening),
	listen_for_utterance(UtteranceTokens, Confidence),
	writeln(Confidence),
	parse_utterance(UtteranceTokens, Command),
	effect_command(Command),
	main_loop.

effect_command(parse_tree([what, is, the, weather, like|Garbage])):-
	Garbage \== [],
	!,
	format('Retrying with the weather model...\n', []),
	retry_last_utterance(weather, NewTokens, Confidence),
	writeln(NewTokens-Confidence).

effect_command(parse_tree([what, is, the, weather, like])):-
	!,
	say('Ah, well its Scotland, so its probably shite', []).

effect_command(parse_tree([what, the, weather, like])):-
	!,
	say('Ah, well its Scotland, so its probably shite', []).

effect_command(parse_tree([add, _, to, the, shopping, list])):-
	!,
	say('Ah, do it yerself yeh lazy basterd', []).

effect_command(parse_tree([_, the, house])):-
	!,
	say('Do eh look like a fecking janitor to ye?', []).

effect_command(parse_tree([what, time, is, it])):-
	!,
	get_time(Time),
	stamp_date_time(Time, date(_Year, _Month, _Day, Hour, Minute, _Second, _Offset, _TZ, _HasDST), local),
	number_to_words(Hour, HourWords),
	number_to_words(Minute, MinuteWords),
	format(atom(Message), 'It is ~w ~w', [HourWords, MinuteWords]),
	say(Message, []).

effect_command(Command):-
	say('What now?', []),
	writeln(effect(Command)).

number_to_words(N, N):-
	N < 10, !.
number_to_words(10, ten):-!.
number_to_words(11, eleven):-!.
number_to_words(12, twelve):-!.
number_to_words(13, thirteen):-!.
number_to_words(14, fourteen):-!.
number_to_words(15, fifteen):-!.
number_to_words(16, sixteen):-!.
number_to_words(17, seventeen):-!.
number_to_words(18, eighteen):-!.
number_to_words(19, nineteen):-!.
number_to_words(20, twenty):-!.
number_to_words(21, 'twenty one'):-!.
number_to_words(22, 'twenty two'):-!.
number_to_words(23, 'twenty three'):-!.



parse_utterance(Tokens, parse_tree(Tokens)).
	
    
