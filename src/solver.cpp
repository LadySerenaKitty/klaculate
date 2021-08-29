#include "klaculate.h"
#include "solver.h"

#include "util/TokenData.h"

#include <cmath>
#include <sstream>

std::string numcheck(token &t) {
	std::stringstream ss;
	return ss.str();
}
std::string stringmaker(util::TokenArray &data, bool html) {
	std::stringstream ss;
	std::string cls;
	util::TokenArray it = data.getIterator();
	token t = it.get();
	while (t.type != TOKEN_INVALID) {
		if (html) {
			switch (t.type.num) {
				case 40://TOKEN_PAREN_OPEN:
				case 41://TOKEN_PAREN_CLOSE:
					cls = "parens";
					break;
				case 94://TOKEN_EXPONENT:
					cls = "exp";
					break;
				case 42://TOKEN_MULT:
					cls = "mult";
					break;
				case 47://TOKEN_DIV:
				case 37://TOKEN_MOD:
					cls = "divi";
					break;
				case 43://TOKEN_ADD:
					cls = "add";
					break;
				case 45://TOKEN_SUB:
					cls = "sub";
					break;
				default: cls = "number";
			}
			ss << "<span class=\"" << cls << "\">" << t << "</span> ";
		}
		else { ss << t; }
		t = it.next();
	}
	return ss.str();
}
void simpleTexter(util::OutputHelper &oh, std::string type, std::string text) {
	util::OutputHelper *dt = new util::OutputHelper();
	dt->addValue("text", text);
	dt->addValue("type", type);
	dt->addTemplate("banner.html");
	oh.addHelper(dt);

}
void stringer(util::OutputHelper &oh, std::string &first, util::TokenArray &data, std::string type, token &left, token &right, token &res) {
	std::string str = stringmaker(data, true);
	util::OutputHelper *dt = new util::OutputHelper();
	dt->addValue("texta", first);
	dt->addValue("textb", str);
	dt->addValue("type", type);

	std::stringstream sl;
	std::stringstream sr;
	std::stringstream sa;
	sl << left;
	sr << right;
	sa << res;

	dt->addValue("left", sl.str());
	dt->addValue("right", sr.str());
	dt->addValue("result", sa.str());

	dt->addTemplate("step.html");

	oh.addHelper(dt);
}

bool hasType(unsigned char type[sizeof(int)], util::TokenArray &data) {
	util::TokenArray it = data.getIterator();
	token t = it.get();
	while (t.type != TOKEN_INVALID) {
		if (t.type == type) { return true; }
		t = it.next();
	}
	return false;
}


// we're going to use recursion here, this will handle all the parenthesis separation
// and run those in recursed calls
void solver(util::OutputHelper &oh, util::TokenArray &input) {
	// FAST FAIL
	if (input.size() == 0) {
		input.add(token{TOKEN_NUMBER, 0});
		return;
	}
	else if (input.size() == 1) {
		if (!tokenNumeric(input[0])) {
			input.remove();
			input.add(token{TOKEN_NUMBER, 0});
		}
		return;
	}
	else if (input.size() == 2) {
		if (tokenNumeric(input[0])) {
			input.setCurrentPos(1);
			input.remove();
		}
		else if (tokenNumeric(input[1])) {
			input[0] = input[1];
			input.remove();
		}
		else {
			input.clear();
			input.add(token{TOKEN_NUMBER, 0});
		}
		return;
	}
	else {
		std::string pt = "Begin!";
		std::string str = stringmaker(input, true);
		util::OutputHelper *dt = new util::OutputHelper();
		dt->addValue("texta", pt);
		dt->addValue("textb", str);
		dt->addTemplate("start.html");
		oh.addHelper(dt);
	}
	input.setCurrentPos(0);

	unsigned int a = 0;
	if (hasType(TOKEN_PAREN_OPEN, input)) {
		simpleTexter(oh, "paren", "Parenthesis");
		while (hasType(TOKEN_PAREN_OPEN, input)) {
			oh.addTemplate("paren_start.html");
			int lev = 0;
			int start = -1, end = -1;
			for (a = 0; a < input.size() && lev == 0; a++) {
				if (input[a].type == TOKEN_PAREN_OPEN) {
					lev = input[a].value;
					start = a;
				}
			}
			for (a = start; a < input.size() && end == -1; a++) {
				token s = token{TOKEN_PAREN_CLOSE, (double)lev};
				if (input[a] == s) {
					end = a;
				}
			}
			if (start != -1 && end != -1) {
				util::TokenArray wrk;
				for (a = (start + 1); a < end; a++) {
					wrk.add(input[a]);
				}
				solver(oh, wrk);
				util::TokenArray tmp;
				for (a = 0; a < input.size(); a++) {
					if (start <= a && a <= end) {
						if (a == start) { tmp.add(wrk[0]); }
					}
					else { tmp.add(input[a]); }
				}
				input.clear();
				for (a = 0; a < tmp.size(); a++) {
					input.add(tmp[a]);
				}
			}
			oh.addTemplate("paren_end.html");
		}
	}

	// exponents
	if (hasType(TOKEN_EXPONENT, input)) {
		simpleTexter(oh, "exp", "Exponents");
		input.setCurrentPos(0);
		token t = input.get();
		while (input.hasNext()) {
			if (t.type == TOKEN_EXPONENT) {
				std::string temp = stringmaker(input, true);
				token l = input.getPrev(), r = input.getNext();
				token q = token{TOKEN_NUMBER, std::pow(l.value, r.value)};
				input.removePrev();
				input.removeNext();
				input.get().type = TOKEN_NUMBER;
				input.get().value = q.value;
				stringer(oh, temp, input, "exp", l, r, q);
				input.setCurrentPos(0);
				t = input.get();
			}
			else { t = input.next(); }
		}
	}

	// mult/div/mod
	if (hasType(TOKEN_MULT, input) || hasType(TOKEN_DIV, input) || hasType(TOKEN_MOD, input)) {
		simpleTexter(oh, "mult divi", "Multiply and Divide/Modulo");
		input.setCurrentPos(0);
		token t = input.get();
		while (input.hasNext()) {
			if (t.type == TOKEN_MULT || t.type == TOKEN_DIV || t.type == TOKEN_MOD) {
				std::string temp = stringmaker(input, true);
				token l = input.getPrev(), r = input.getNext();
				input.removeNext();
				input.removePrev();
				input.get().type = TOKEN_NUMBER;

				token q;
				switch (t.type.num) {
					case 42: //TOKEN_MUL:
						q = l * r;
						input.get().value = q.value;
						stringer(oh, temp, input, "mult", l, r, q);
						break;
					case 47: //TOKEN_DIV:
						q = l / r;
						input.get().value = q.value;
						stringer(oh, temp, input, "divi", l, r, q);
						break;
					case 37: //TOKEN_MOD:
						q = l % r;
						input.get().value = q.value;
						stringer(oh, temp, input, "divi", l, r, q);
						break;
				}
				input.setCurrentPos(0);
				t = input.get();
			}
			else { t = input.next(); }
		}
	}

	// add/sub
	if (hasType(TOKEN_ADD, input) || hasType(TOKEN_SUB, input)) {
		simpleTexter(oh, "add sub", "Add and Subtract");
		input.setCurrentPos(0);
		token t = input.get();
		while (input.hasNext()) {
			if (t.type == TOKEN_ADD || t.type == TOKEN_SUB) {
				std::string temp = stringmaker(input, true);
				token l = input.getPrev(), r = input.getNext();
				input.removeNext();
				input.removePrev();
				input.get().type = TOKEN_NUMBER;

				token q;
				switch (t.type.num) {
					case 43: //TOKEN_ADD:
						q = l + r;
						input.get().value = q.value;
						stringer(oh, temp, input, "add", l, r, q);
						break;
					case 45: //TOKEN_SUB:
						q = l - r;
						input.get().value = q.value;
						stringer(oh, temp, input, "sub", l, r, q);
						break;
				}
				input.setCurrentPos(0);
				t = input.get();
			}
			else { t = input.next(); }
		}
	}
}
