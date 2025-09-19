#include "nfa.h"

int main(){
    string pattern = "a.c";
    NFA* nfa = NFA::regex_to_nfa(pattern);
    string test_str = "ac";
    if(nfa->matches(test_str)){
        cout << test_str << " matches the pattern " << pattern << endl;
    } else {
        cout << test_str << " does not match the pattern " << pattern << endl;
    }
    return 0;
}