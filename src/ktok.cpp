#include "ktok.h"
#include <cmath>
#include <iomanip>
#include <cmath>

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
	if (r.type == TOKEN_NUMBER) {
		if (std::isinf(r.value)) { l << "∞"; }
		else { l << r.value; }
	}
	else { l << r.type; }
	return l;
}
