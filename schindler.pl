:-module(schindler, []).

% Schindler integration

:-multifile(computer:effect_command/1).
:-use_module(library(http/http_open)).

computer:effect_command(parse_tree([add, Item, to, the, shopping, list])):-
        !,
        atom_codes(Item, Codes),
        setup_call_cleanup(http_open('http://schindlerx.strangled.net:8080/api/add_item?key=matt',
                                     Stream,
                                     [method(post),
                                      post(codes('timtext/plain', Codes))]),
                           read_string(Stream, _, Reply),
                           close(Stream)),
        say(Reply, []).