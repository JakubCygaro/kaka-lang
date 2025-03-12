#include "args_parse.h"
#include <stdlib.h>

ParseResult parse_args(char** args, int argn, Args *ret) {
    ret->preserve = 0;
    ret->out_path = NULL;
    ret->source_path = NULL;
    if(argn > 3) return TooManyArgs;
    if(argn == 0) return NoArguments;

    int current_arg = 0;

    while(current_arg < argn) {
        switch (args[current_arg][0]) {
            case '-': {
                if(args[current_arg][1] == 'p' && !ret->preserve){
                    ret->preserve = 1;
                } else if (ret->preserve) {
                    return InvalidFlags;
                } else {
                    return UnknownFlag;
                }
            } break;
            default: {
                if(ret->source_path == NULL){
                    ret->source_path = args[current_arg];
                } else {
                    ret->out_path = args[current_arg];
                }
            } break;
        }
        current_arg++;
    }
    if(ret->source_path == NULL){
        return NoSource;
    }
    return Success;
}
