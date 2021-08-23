#include "klaculate.h"
#include "klac.h"
#include "ktok.h"

#include "util/InputHelper.h"
#include "util/OutputHelper.h"

#include <regex>
#include <vector>
#include <cmath>
#include <sstream>

std::string stringmaker(std::vector<token> &data, bool html);

std::string stringmaker(std::vector<token> &data) { return stringmaker(data, false); }

void solver(util::OutputHelper &oh, std::vector<token> &input);

void runtask(FCGX_Request *rq) {
	util::InputHelper ih(rq->in, rq->envp);
	util::OutputHelper oh;

	oh.addValue((char *) "scheme", ih.getHeader("REQUEST_SCHEME"));
	oh.addValue("host", ih.getHeader("HTTP_HOST"));

	std::string pt;
	if (ih.existGetter("e") || ih.existFormer("e")) {
		if (ih.existFormer("e")) { pt = ih.getFormer("e"); }
		else { pt = ih.getGetter("e"); }
	}

	oh.addTemplate("header.html");

	if (!pt.empty()) {
		std::vector<token> tks;
		unsigned char tt = 0;
		char te = 0;
		int pcount = 0;
		double value = 0;
		std::regex re("([0-9.]+|[-+*^%ex\\/\\(\\)])");
		auto tbeg = std::sregex_iterator(pt.begin(), pt.end(), re);
		auto tend = std::sregex_iterator();
		for(std::sregex_iterator t = tbeg; t != tend; t++) {
			std::smatch match = *t;
			tt = 0;
			if (match.str().size() == 1) {
				te = match.str()[0];
				if (te == '(') { tt = TOKEN_PAREN_OPEN; pcount++;}
				else if (te == ')') { tt = TOKEN_PAREN_CLOSE; }
				else if (te == '^' || te == 'e') { tt = TOKEN_EXPONENT; }
				else if (te == '*' || te == 'x') { tt = TOKEN_MULT; }
				else if (te == '/') { tt = TOKEN_DIV; }
				else if (te == '%') { tt = TOKEN_MOD; }
				else if (te == '+') { tt = TOKEN_ADD; }
				else if (te == '-') { tt = TOKEN_SUB; }
				else { tt = TOKEN_NUMBER; }
			}
			else { tt = TOKEN_NUMBER; }

			if (tt == TOKEN_NUMBER) { value = std::stod(match.str(), nullptr); }
			else { value = pcount; }

			tks.push_back(token{tt, value});

			if (tt == TOKEN_PAREN_CLOSE) { pcount--; }
		}

		if (pcount > 0) { // close out all open parens
			for (; pcount > 0; pcount--) {
				value = pcount;
				tks.push_back(token{TOKEN_PAREN_CLOSE, value});
			}
		}
		oh.addValue("eqi", stringmaker(tks));
		solver(oh, tks);
	}
	oh.addTemplate("footer.html");
	std::string content = oh.getOutput();

	std::stringstream ss;
	ss << "Content-Type: text/html; charset=UTF-8\r\n";
	ss << "Content-Length: " << std::to_string(content.length()) << "\r\n\r\n";
	std::string s;
	s = ss.str();

	FCGX_PutStr(s.c_str(), s.size(), rq->out);
	FCGX_PutStr(content.c_str(), content.size(), rq->out);

}

void replacer(std::vector<token> &input, unsigned int pos, token nval) {
	std::vector<token> tmp;
	for (int a = 0; a < input.size(); a++) {
		if (a == pos - 1 || a == pos + 1) {}
		else if (a == pos) { tmp.push_back(nval); }
		else { tmp.push_back(input[a]); }
	}
	input.clear();
	for (int a = 0; a < tmp.size(); a++) {
		input.push_back(tmp[a]);
	}
}

std::string stringmaker(std::vector<token> &data, bool html) {
	std::stringstream ss;
	std::string cls;
	for (token t : data) {
		if (html) {
			switch (t.type) {
				case TOKEN_PAREN_OPEN:
				case TOKEN_PAREN_CLOSE:
					cls = "parens";
					break;
				case TOKEN_EXPONENT: cls = "exp"; break;
				case TOKEN_MULT: cls = "mult"; break;
				case TOKEN_DIV:
				case TOKEN_MOD:
					cls = "divi";
					break;
				case TOKEN_ADD: cls = "add"; break;
				case TOKEN_SUB: cls = "sub"; break;
				default: cls = "number";
			}
			ss << "<span class=\"" << cls << "\">" << t << "</span> ";
		}
		else { ss << t; }
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
void stringer(util::OutputHelper &oh, std::string &first, std::vector<token> &data, std::string type) {
	std::string str = stringmaker(data, true);
	util::OutputHelper *dt = new util::OutputHelper();
	dt->addValue("texta", first);
	dt->addValue("textb", str);
	dt->addValue("type", type);
	dt->addTemplate("step.html");
	oh.addHelper(dt);
}

bool hasType(char type, std::vector<token> &data) {
	for (token t : data) {
		if (t.type == type) { return true; }
	}
	return false;
}

// we're going to use recursion here, this will handle all the parenthesis separation
// and run those in recursed calls
void solver(util::OutputHelper &oh, std::vector<token> &input) {
	// FAST FAIL
	if (input.size() <= 2) { return; }
	std::string pt = "Begin!";
	stringer(oh, pt, input, "none");

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
				if (input[a].type == TOKEN_PAREN_CLOSE && input[a].value == lev) {
					end = a;
				}
			}
			if (start != -1 && end != -1) {
				std::vector<token> wrk;
				for (a = (start + 1); a < end; a++) {
					wrk.push_back(input[a]);
				}
				solver(oh, wrk);
				std::vector<token> tmp;
				for (a = 0; a < input.size(); a++) {
					if (start <= a && a <= end) {
						if (a == start) { tmp.push_back(wrk[0]); }
					}
					else { tmp.push_back(input[a]); }
				}
				input.clear();
				for (a = 0; a < tmp.size(); a++) {
					input.push_back(tmp[a]);
				}
			}
			oh.addTemplate("paren_end.html");
		}
	}

	// exponents
	if (hasType(TOKEN_EXPONENT, input)) {
		simpleTexter(oh, "exp", "Exponents");
		for (a = 0; a < input.size(); a++) {
			if (input[a].type == TOKEN_EXPONENT) {
				std::string temp = stringmaker(input, true);
				replacer(input, a, token{TOKEN_NUMBER, std::pow(input[a - 1].value, input[a + 1].value)});
				stringer(oh, temp, input, "exp");
				a = 0;
			}
		}
	}

	// mult/div/mod
	if (hasType(TOKEN_MULT, input)) {
		simpleTexter(oh, "mult", "Multiply and Divide/Modulo");
		for (a = 0; a < input.size(); a++) {
			if (input[a].type == TOKEN_MULT || input[a].type == TOKEN_DIV || input[a].type == TOKEN_MOD) {
				std::string temp = stringmaker(input, true);
				switch (input[a].type) {
					case TOKEN_MULT:
						replacer(input, a, input[a - 1] * input[a + 1]);
						stringer(oh, temp, input, "mult");
						break;
					case TOKEN_DIV:
						replacer(input, a, input[a - 1] / input[a + 1]);
						stringer(oh, temp, input, "divi");
						break;
					case TOKEN_MOD:
						replacer(input, a, input[a - 1] % input[a + 1]);
						stringer(oh, temp, input, "divi");
						break;
				}
				a = 0;
			}
		}
	}


	// add/sub
	if (hasType(TOKEN_ADD, input)) {
		simpleTexter(oh, "add sub", "Add and Subtract");
		for (a = 0; a < input.size(); a++) {
			if (input[a].type == TOKEN_ADD || input[a].type == TOKEN_SUB) {
				std::string temp = stringmaker(input, true);
				switch (input[a].type) {
					case TOKEN_ADD:
						replacer(input, a, input[a - 1] + input[a + 1]);
						stringer(oh, temp, input, "add");
						break;
					case TOKEN_SUB:
						replacer(input, a, input[a - 1] - input[a + 1]);
						stringer(oh, temp, input, "sub");
						break;
				}
				a = 0;
			}
		}
	}

	// sub
	if (hasType(TOKEN_SUB, input)) {
		simpleTexter(oh, "sub", "Subtract");
		for (a = 0; a < input.size(); a++) {
			if (input[a].type == TOKEN_SUB) {
				std::string temp = stringmaker(input, true);
				replacer(input, a, input[a - 1] - input[a + 1]);
				stringer(oh, temp, input, "sub");
				a = 0;
			}
		}
	}
}
