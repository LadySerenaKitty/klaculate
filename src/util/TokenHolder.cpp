#include "TokenHolder.h"

util::TokenHolder::TokenHolder(token &input) {
	p = nullptr;
	n = nullptr;
	data = input;
}
util::TokenHolder::TokenHolder(const token &input) {
	p = nullptr;
	n = nullptr;
	data = input;
}
token& util::TokenHolder::get() { return data; }
util::TokenHolder* util::TokenHolder::getPrev() { return p; }
util::TokenHolder* util::TokenHolder::getNext() { return n; }
void util::TokenHolder::setPrev(TokenHolder *input) { p = input; }
void util::TokenHolder::setNext(TokenHolder *input) { n = input; }
bool util::TokenHolder::hasPrev() { return p != nullptr; }
bool util::TokenHolder::hasNext() { return n != nullptr; }
