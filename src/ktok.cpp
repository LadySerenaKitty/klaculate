#include "ktok.h"
#include <cmath>
#include <iomanip>

token operator*(token &l, token &r) {
	return token{TOKEN_NUMBER, l.value * r.value};
}
token operator/(token &l, token &r) {
	return token{TOKEN_NUMBER, l.value / r.value};
}
token operator%(token &l, token &r) {
	return token{TOKEN_NUMBER, std::fmod(l.value, r.value)};
}
token operator+(token &l, token &r) {
	return token{TOKEN_NUMBER, l.value + r.value};
}
token operator-(token &l, token &r) {
	return token{TOKEN_NUMBER, l.value - r.value};
}

std::ostream& operator<<(std::ostream &l, token &r) {
	switch (r.type) {
		case TOKEN_PAREN_OPEN: l << "("; break;
		case TOKEN_PAREN_CLOSE: l << ")"; break;
		case TOKEN_EXPONENT: l << "^"; break;
		case TOKEN_MULT: l << "*"; break;
		case TOKEN_DIV: l << "/"; break;
		case TOKEN_MOD: l << "%"; break;
		case TOKEN_ADD: l << "+"; break;
		case TOKEN_SUB: l << "-"; break;
		default: l << r.value;
	}
	l << " ";
	return l;
}
