// a regex NFA implementation with mordern C++
// '.' -> concatenation
// '|' -> Alternation
// '*' zero or more
// '?' zero or one
// '+' one or more
// '(' and ')' for grouping

#include <bits/stdc++.h>
using namespace std;

class State{
public:
    int id;
    bool is_end;
    map<char, vector<State*>> transitions; // char -> list of next states
    vector<State*> epsilon_transitions; // epsilon transitions
    State(int id): id(id), is_end(false) {}
};
class NFA{
public:
    State* start;
    State* end;
    NFA(): start(nullptr), end(nullptr) {}
    NFA(State* start, State* end): start(start), end(end) {}
    static NFA* concat(NFA* nfa1, NFA* nfa2);
    static NFA* alternation(NFA* nfa1, NFA* nfa2);
    static NFA* kleene_star(NFA* nfa);
    static NFA* plus(NFA* nfa);
    static NFA* optional(NFA* nfa);
    static NFA* from_char(char c);
    static NFA* from_string(const string& s);
    static NFA* regex_to_nfa(const string& regex);
    bool matches(const string& s);
private:
    static int state_id;
    static void epsilon_closure(State* state, set<State*>& closure);
    static void move(State* state, char c, set<State*>& result);
    static NFA* regex_to_nfa_helper(const string& regex, int& pos);
};
int NFA::state_id = 0;


// Implementations of NFA
NFA* NFA::concat(NFA* nfa1, NFA* nfa2) {
    nfa1->end->is_end = false;
    nfa1->end->epsilon_transitions.push_back(nfa2->start);
    return new NFA(nfa1->start, nfa2->end);
}

NFA* NFA::alternation(NFA* nfa1, NFA* nfa2) {
    State* start = new State(state_id++);
    State* end = new State(state_id++);
    start->epsilon_transitions.push_back(nfa1->start);
    start->epsilon_transitions.push_back(nfa2->start);
    nfa1->end->is_end = false;
    nfa2->end->is_end = false;
    nfa1->end->epsilon_transitions.push_back(end);
    nfa2->end->epsilon_transitions.push_back(end);
    end->is_end = true;
    return new NFA(start, end);
}

NFA* NFA::kleene_star(NFA* nfa) {
    State* start = new State(state_id++);
    State* end = new State(state_id++);
    start->epsilon_transitions.push_back(nfa->start);
    start->epsilon_transitions.push_back(end);
    nfa->end->is_end = false;
    nfa->end->epsilon_transitions.push_back(nfa->start);
    nfa->end->epsilon_transitions.push_back(end);
    end->is_end = true;
    return new NFA(start, end);
}

NFA* NFA::plus(NFA* nfa) {
    State* start = new State(state_id++);
    State* end = new State(state_id++);
    start->epsilon_transitions.push_back(nfa->start);
    nfa->end->is_end = false;
    nfa->end->epsilon_transitions.push_back(nfa->start);
    nfa->end->epsilon_transitions.push_back(end);
    end->is_end = true;
    return new NFA(start, end);
}

NFA* NFA::optional(NFA* nfa) {
    State* start = new State(state_id++);
    State* end = new State(state_id++);
    start->epsilon_transitions.push_back(nfa->start);
    start->epsilon_transitions.push_back(end);
    nfa->end->is_end = false;
    nfa->end->epsilon_transitions.push_back(end);
    end->is_end = true;
    return new NFA(start, end);
}

NFA* NFA::from_char(char c) {
    State* start = new State(state_id++);
    State* end = new State(state_id++);
    start->transitions[c].push_back(end);
    end->is_end = true;
    return new NFA(start, end);
}

NFA* NFA::from_string(const string& s) {
    if (s.empty()) return nullptr;
    NFA* nfa = from_char(s[0]);
    for (size_t i = 1; i < s.size(); ++i) {
        NFA* next_nfa = from_char(s[i]);
        nfa = concat(nfa, next_nfa);
    }
    return nfa;
}

NFA* NFA::regex_to_nfa(const string& regex) {
    int pos = 0;
    return regex_to_nfa_helper(regex, pos);
}

NFA* NFA::regex_to_nfa_helper(const string& regex, int& pos) {
    vector<NFA*> nfa_stack;
    vector<char> op_stack;

    auto apply_operator = [&]() {
        char op = op_stack.back();
        op_stack.pop_back();
        if (op == '.') {
            NFA* nfa2 = nfa_stack.back(); nfa_stack.pop_back();
            NFA* nfa1 = nfa_stack.back(); nfa_stack.pop_back();
            nfa_stack.push_back(concat(nfa1, nfa2));
        } else if (op == '|') {
            NFA* nfa2 = nfa_stack.back(); nfa_stack.pop_back();
            NFA* nfa1 = nfa_stack.back(); nfa_stack.pop_back();
            nfa_stack.push_back(alternation(nfa1, nfa2));
        } else if (op == '*') {
            NFA* nfa = nfa_stack.back(); nfa_stack.pop_back();
            nfa_stack.push_back(kleene_star(nfa));
        } else if (op == '+') {
            NFA* nfa = nfa_stack.back(); nfa_stack.pop_back();
            nfa_stack.push_back(plus(nfa));
        } else if (op == '?') {
            NFA* nfa = nfa_stack.back(); nfa_stack.pop_back();
            nfa_stack.push_back(optional(nfa));
        }
    };

    auto precedence = [](char op) {
        if (op == '*') return 3;
        if (op == '+') return 3;
        if (op == '?') return 3;
        if (op == '.') return 2;
        if (op == '|') return 1;
        return 0;
    };

    while (pos < regex.size()) {
        char c = regex[pos];
        if (isalnum(c)) {
            nfa_stack.push_back(from_char(c));
            pos++;
            // Implicit concatenation
            if (pos < regex.size() && (isalnum(regex[pos]) || regex[pos] == '(')) {
                while (!op_stack.empty() && precedence(op_stack.back()) >= precedence('.')) {
                    apply_operator();
                }
                op_stack.push_back('.');
            }
        } else if (c == '(') {
            pos++;
            NFA* sub_nfa = regex_to_nfa_helper(regex, pos);
            nfa_stack.push_back(sub_nfa);
            // Implicit concatenation
            if (pos < regex.size() && (isalnum(regex[pos]) || regex[pos] == '(')) {
                while (!op_stack.empty() && precedence(op_stack.back()) >= precedence('.')) {
                    apply_operator();
                }
                op_stack.push_back('.');
            }
        } else if (c == ')') {
            pos++;
            break;
        } else if (c == '*') {
            while (!op_stack.empty() && precedence(op_stack.back()) >= precedence('*')) {
                apply_operator();
            }
            op_stack.push_back('*');
            pos++;
        } else if (c == '+') {
            while (!op_stack.empty() && precedence(op_stack.back()) >= precedence('+')) {
                apply_operator();
            }
            op_stack.push_back('+');
            pos++;
        } else if (c == '?') {
            while (!op_stack.empty() && precedence(op_stack.back()) >= precedence('?')) {   
                apply_operator();
            }
            op_stack.push_back('?');
            pos++;
        } else if (c == '|') {
            while (!op_stack.empty() && precedence(op_stack.back()) >= precedence('|')) {
                apply_operator();
            }
            op_stack.push_back('|');
            pos++;
        } else {
            pos++; // Ignore unrecognized characters
        }
    }
    while (!op_stack.empty()) {
        apply_operator();
    }
    return nfa_stack.back();
}

bool NFA::matches(const string& s) {
    set<State*> current_states;
    epsilon_closure(start, current_states);
    for (char c : s) {
        set<State*> next_states;
        for (State* state : current_states) {
            move(state, c, next_states);
        }
        current_states.clear();
        for (State* state : next_states) {
            epsilon_closure(state, current_states);
        }
    }
    for (State* state : current_states) {
        if (state->is_end) return true;
    }
    return false;
}
void NFA::epsilon_closure(State* state, set<State*>& closure) {
    if (closure.count(state)) return;
    closure.insert(state);
    for (State* next_state : state->epsilon_transitions) {
        epsilon_closure(next_state, closure);
    }
}
void NFA::move(State* state, char c, set<State*>& result) {
    if (state->transitions.count(c)) {
        for (State* next_state : state->transitions[c]) {
            result.insert(next_state);
        }
    }
}

