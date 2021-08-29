#include "TokenArray.h"

static token invalid = token{TOKEN_INVALID, INVALID_TOKEN};

util::TokenArray::TokenArray() {
	count = 0;
	curpos = 0;
	root = nullptr;
	cur = nullptr;
}

util::TokenArray util::TokenArray::getIterator() {
	util::TokenArray ta;
	ta.root = root;
	ta.cur = root;
	ta.count = count;
	ta.curpos = 0;
	return ta;
}
void util::TokenArray::add(token &data) {
	TokenHolder *th = new TokenHolder(data);
	curpos = addunit(cur, th);
	next();
}
void util::TokenArray::add(const token &data) {
	TokenHolder *th = new TokenHolder(data);
	curpos = addunit(cur, th);
	next();
}
void util::TokenArray::addBefore(token &data) {
	if (cur->hasPrev()) {
		TokenHolder *th = new TokenHolder(data);
		std::size_t st = addunit(cur->getPrev(), th);
		if (st == (curpos + 1)) { curpos++; }
	}
}
void util::TokenArray::addBefore(const token &data) {
	if (cur->hasPrev()) {
		TokenHolder *th = new TokenHolder(data);
		std::size_t st = addunit(cur->getPrev(), th);
		if (st == (curpos + 1)) { curpos++; }
	}
}
void util::TokenArray::addAfter(token &data) {
	TokenHolder *th = new TokenHolder(data);
	addunit(cur, th);
}
void util::TokenArray::addAfter(const token &data) {
	TokenHolder *th = new TokenHolder(data);
	addunit(cur, th);
}

token& util::TokenArray::get() { return cur->get(); }
token& util::TokenArray::prev() {
	if (curpos > 0 && hasPrev()) {
		curpos--;
		cur = cur->getPrev();
		return get();
	}
	return invalid;
}
token& util::TokenArray::next() {
	curpos++;
	if (hasNext()) {
		cur = cur->getNext();
		return get();
	}
	return invalid;

}

token& util::TokenArray::getPrev() {
	if (hasPrev()) {
		return cur->getPrev()->get();
	}
	return invalid;
}
token& util::TokenArray::getNext() {
	if (hasNext()) {
		return cur->getNext()->get();
	}
	return invalid;
}

void util::TokenArray::remove() {
	curpos = removeunit(cur);
	if (count == 0) {
		curpos = 0;
		cur = nullptr;
		root = nullptr;
	}
	else { setCurrentPos(curpos); }
}
void util::TokenArray::removePrev() {
	if (cur->hasPrev()) {
		if (root == cur->getPrev()) { root = cur; }
		std::size_t st = removeunit(cur->getPrev());
		if (st == (curpos - 1)) { curpos--; }
	}
}
void util::TokenArray::removeNext() {
	if (cur->hasNext()) {
		std::size_t st = removeunit(cur->getNext());
	}
}

bool util::TokenArray::hasPrev() {
	if (cur == nullptr) { return false; }
	return cur->hasPrev();
}
bool util::TokenArray::hasNext() {
	if (cur == nullptr) { return false; }
	return cur->hasNext();
}

std::size_t util::TokenArray::size() { return count; }
std::size_t util::TokenArray::getCurrentPos() { return curpos; }
void util::TokenArray::setCurrentPos(std::size_t input) {
	curpos = 0;
	cur = root;
	if (input == 0) { return; }
	if (input >= count) { input = (count - 1); }
	for (curpos = 1; curpos <= input; curpos++) {
		cur->getNext();
	}
}
void util::TokenArray::clear() {
	TokenHolder *th;
	cur = root;

	count = 0;
	curpos = 0;

	while (cur->hasNext()) {
		th = cur->getNext();
		delete cur;
		cur = th;
	}
	if (cur != nullptr) { delete cur; }
	cur = nullptr;
	root = nullptr;
}

token& util::TokenArray::operator[](std::size_t idx) {
	if (idx == 0) { return root->get(); }
	TokenHolder *tmp = root;

	if (idx >= count) { idx = (count - 1); }
	for (int a = 1; a <= idx; a++) {
		tmp = tmp->getNext();
	}
	return tmp->get();
}

std::size_t util::TokenArray::addunit(TokenHolder *c, TokenHolder *th) {
	if (c == nullptr) {
		count = 1;
		curpos = 0;
		root = th;
		cur = th;
		return 0;
	}

	std::size_t newpos = curpos;

	if (c->hasNext()) {
		TokenHolder *tmp_n;
		tmp_n = c->getNext();
		tmp_n->setPrev(th);
		th->setNext(tmp_n);
	}
	th->setPrev(c);
	c->setNext(th);
	count++;
	return newpos;
}

std::size_t util::TokenArray::removeunit(TokenHolder *c) {
	if (c == nullptr) {
		count = 0;
		curpos = 0;
		return 0;
	}

	std::size_t newpos = curpos;
	TokenHolder *tmp_n;
	TokenHolder *tmp_p;

	bool hNext = c->hasNext();
	bool hPrev = c->hasPrev();
	if (hNext) { tmp_n = c->getNext(); }
	if (hPrev) { tmp_p = c->getPrev(); }

	delete c;
	count--;
	if (count == 0) { return 0; }

	if (hNext && hPrev) {
		tmp_p->setNext(tmp_n);
		tmp_n->setPrev(tmp_p);
		c = tmp_n;
	}
	else if (hNext) {
		tmp_n->setPrev(nullptr);
		c = tmp_n;
	}
	else if (hPrev) {
		tmp_p->setNext(nullptr);
		c = tmp_p;
		newpos--;
	}
	return newpos;
}
