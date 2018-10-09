:-module(time,
         [time_in_words/3]).

time_in_words(with_orientation, Timestamp, Words):-
        !,
        stamp_date_time(Timestamp, date(_Year, _Month, _Day, Hour, Minute, _Second, _Offset, _TZ, _HasDST), local),
        orientation_time_pronunciation(Hour, Minute, Words).

time_in_words(military, Timestamp, Words):-
        stamp_date_time(Timestamp, date(_Year, _Month, _Day, Hour, Minute, _Second, _Offset, _TZ, _HasDST), local),
        military_time_pronunciation(Hour, Minute, Words).

military_time_pronunciation(Hour, Minute, Words):-
        number_name(Hour, HourName),
        number_name(Minute, MinuteName),
        ( Minute < 10->
            format(atom(Words), '~w oh ~w', [HourName, MinuteName])
        ; otherwise->
            format(atom(Words), '~w ~w', [HourName, MinuteName])
        ).


orientation_time_pronunciation(Hour, Minute, Words):-
        orientation(Minute, Orientation, Hour, AdjustedHour),
	(AdjustedHour > 12 ->
	 H is AdjustedHour - 12
	; H = AdjustedHour
	),

	hour_name(H, HourName),
        ( Hour =:= 0 ->
            Meridian = ''
        ; Hour < 12 ->
            Meridian = 'in the morning'
        ; Hour < 18 ->
            Meridian = 'in the afternoon'
        ; otherwise->
            Meridian = 'at night'
        ),

        format(atom(Words), '~w ~w ~w', [Orientation, HourName, Meridian]).


hour_name(0, midnight):- !.
hour_name(1, one):- !.
hour_name(2, two):- !.
hour_name(3, three):- !.
hour_name(4, four):- !.
hour_name(5, five):- !.
hour_name(6, six):- !.
hour_name(7, seven):- !.
hour_name(8, eight):- !.
hour_name(9, nine):- !.
hour_name(10, ten):- !.
hour_name(11, eleven):- !.
hour_name(12, midday):- !.

orientation(N, Orientation, H, Hour):-
        ( N =:= 0 ->
	  Orientation = 'exactly',
	  Hour = H
        ; N =< 30 ->
	  minute_name(N, Name),
	  format(atom(Orientation), '~w past', [Name]),
	  Hour = H
        ; otherwise->
	  NN is 60 - N,
	  minute_name(NN, Name),
	  format(atom(Orientation), '~w to', [Name]),
	  Hour is (H+1) mod 24
        ).

minute_name(1, one):- !.
minute_name(2, two):- !.
minute_name(3, three):- !.
minute_name(4, four):- !.
minute_name(5, five):- !.
minute_name(6, six):- !.
minute_name(7, seven):- !.
minute_name(8, eight):- !.
minute_name(9, nine):- !.
minute_name(10, ten):- !.
minute_name(11, eleven):- !.
minute_name(12, twelve):- !.
minute_name(13, thirteen):- !.
minute_name(14, fourteen):- !.
minute_name(15, quarter):- !.
minute_name(16, sixteen):- !.
minute_name(17, seventeen):- !.
minute_name(18, eighteen):- !.
minute_name(19, nineteen):- !.
minute_name(20, twenty):- !.
minute_name(21, 'twenty one'):- !.
minute_name(22, 'twenty two'):- !.
minute_name(23, 'twenty three'):- !.
minute_name(24, 'twenty four'):- !.
minute_name(25, 'twenty five'):- !.
minute_name(26, 'twenty six'):- !.
minute_name(27, 'twenty seven'):- !.
minute_name(28, 'twenty eight'):- !.
minute_name(29, 'twenty nine'):- !.
minute_name(30, half):- !.

number_name(0, zero):- !.
number_name(1, one):- !.
number_name(2, two):- !.
number_name(3, three):- !.
number_name(4, four):- !.
number_name(5, five):- !.
number_name(6, six):- !.
number_name(7, seven):- !.
number_name(8, eight):- !.
number_name(9, nine):- !.
number_name(10, ten):- !.
number_name(11, eleven):- !.
number_name(12, twelve):- !.
number_name(13, thirteen):- !.
number_name(14, fourteen):- !.
number_name(15, fifteen):- !.
number_name(16, sixteen):- !.
number_name(17, seventeen):- !.
number_name(18, eighteen):- !.
number_name(19, nineteen):- !.
number_name(20, twenty):- !.
number_name(30, thirty):- !.
number_name(40, forty):- !.
number_name(50, fifty):- !.
number_name(60, sixty):- !.
number_name(70, seventy):- !.
number_name(80, eighty):- !.
number_name(90, ninety):- !.

number_name(N, Name):-
        N > 20,
        Tens is 10 * (N // 10),
        Ones is N mod 10,
        ( Tens > 99 ->
            SmallPart is N mod 100,
            number_name(SmallPart, SmallName),
            large_number_name(N, 100, LargeName),
            format(atom(Name), '~w and ~w', [LargeName, SmallName])
        ; otherwise->
            number_name(Tens, TensName),
            number_name(Ones, OnesName),
            format(atom(Name), '~w ~w', [TensName, OnesName])
        ).

large_number_name(Number, Divisor, Name):-
        divisor_name(Divisor, DivisorName),
        Digit is (Number // Divisor) mod 10,
        ( Digit =:= 0 ->
            % Avoid 'two thousand, zero hundred'
            Chunk = ''
        ; otherwise->
            number_name(Digit, DigitName),
            format(atom(Chunk), '~w ~w', [DigitName, DivisorName])
        ),
        NewDivisor is Divisor * 10,
        ( Number // NewDivisor =:= 0 ->
            Name = Chunk
        ; otherwise->
            large_number_name(Number, NewDivisor, RestOfName),
            ( Chunk == '' ->
                Name = RestOfName
            ; otherwise->
                format(atom(Name), '~w, ~w', [RestOfName, Chunk])
            )
        ).

divisor_name(100, hundred):- !.
divisor_name(1000, thousand):- !.
divisor_name(1000000, million):- !.