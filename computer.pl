% Helpful hints
% https://sourceforge.net/p/cmusphinx/discussion/help/thread/61e35142/

/* Major tasks:

*/

:-module(computer,
         [computer/0]).

computer:-
        thread_create(prolog, _, [detached(true)]),
	on_signal(term, _, halt),
	load_tensorflow_model('qqq.pb', Model),
	init_sphinx(default, computer, 1e-40),
	main_loop(Model).

:-meta_predicate(random_solution(0)).
random_solution(Goal):-
        bagof(Goal, Goal, Goals),
        random_member(Goal, Goals).

greeting('Aye, what now?').
greeting('Och, what is it?').
greeting('Yeh, what is it?').
greeting('Ah, I was asleep. What?'):-
        get_time(Time),
        stamp_date_time(Time, date(_Year, _Month, _Day, Hour, _Minute, _Second, _Offset, _TZ, _HasDST), local),
        ( Hour >= 21 -> true
        ; Hour =< 5  -> true
        ).

did_not_understand('I did nay understand ye').
did_not_understand('Speak up, I canna hear ye').
did_not_understand('You what now?').

main_loop(Model):-
        writeln('Waiting for wake-word...'),
        wait_for_model(Model, 0.9),
        random_solution(greeting(Greeting)),
        say(Greeting, []),
	writeln(listening),
	listen_for_utterance(UtteranceTokens, Confidence),
	writeln(Confidence),
	parse_utterance(UtteranceTokens, Command),
        ( effect_command(Command)->
            true
        ; otherwise->
            random_solution(did_not_understand(Response)),
            say(Response, []),
            writeln(effect(Command))
        ),
	main_loop(Model).

:-multifile(computer:effect_command/1).

effect_command(parse_tree([what, is, the, weather, like|Garbage])):-
	Garbage \== [],
	!,
	format('Retrying with the weather model...\n', []),
	retry_last_utterance(weather, NewTokens, _Confidence),
	(NewTokens = [what, is, the, weather, like, in, Location]->
	 format(atom(Message), 'I have no idea about the weather in ~w', [Location]),
	 say(Message, [])
	;otherwise->
	 say('I dinnae catch that', [])
	).
	%writeln(NewTokens-Confidence).

effect_command(parse_tree([what, is, the, weather, like])):-
	!,
	say('Ah, well its Scotland, so its probably shite', []).

effect_command(parse_tree([what, the, weather, like])):-
	!,
	say('Ah, well its Scotland, so its probably shite', []).

effect_command(parse_tree([_, the, house])):-
	!,
	say('Do eh look like a fecking janitor to ye?', []).

effect_command(parse_tree([what, time, is, it])):-
        !,
        speak_time.

effect_command(parse_tree([time, is, it])):-
        !,
        speak_time.

effect_command(parse_tree([what time, is])):-
        !,
        speak_time.


speak_time:-
        get_time(Time),
	time_in_words(with_orientation, Time, Words),
	writeln(got(Words)),
        say(Words, [stretch(0.8)]).

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
	
    
