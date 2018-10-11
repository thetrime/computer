:-module(schindler, []).

% Schindler integration

:-multifile(computer:effect_command/1).
:-use_module(http/http_open).

computer:effect_command(parse_tree([add, Item, to, the, shopping, list])):-
        !,
        atom_codes(Item, Codes),
        setup_call_cleanup(http_open('http://schindlerx.strangled.net:8080?key=matt',
                                     Stream,
                                     [method(post),
                                      post(codes('text/plain', ItemCodes))]),
                           read_string(Stream, Reply, _),
                           close(Stream)),
        say(Reply, []).