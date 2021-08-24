#include "klaculate.h"
#include "klac.h"
#include "ktok.h"

#include "util/base64.h"
#include "util/InputHelper.h"
#include "util/OutputHelper.h"

#include <regex>
#include <vector>
#include <cmath>
#include <sstream>
#include <map>

#include <sys/stat.h>

#define AADD token{TOKEN_ADD, (double)pcount}
#define AONE token{TOKEN_NUMBER, (double)1}
#define AZRO token{TOKEN_NUMBER, (double)0}

std::string stringmaker(std::vector<token> &data, bool html);

std::string stringmaker(std::vector<token> &data) { return stringmaker(data, false); }

void solver(util::OutputHelper &oh, std::vector<token> &input);

void runtask(FCGX_Request *rq) {
	util::InputHelper ih(rq->in, rq->envp);
	util::OutputHelper oh;

	oh.addValue((char *) "scheme", ih.getHeader("REQUEST_SCHEME"));
	oh.addValue("host", ih.getHeader("HTTP_HOST"));

	oh.addTemplate("header.html");
	std::string pt = "";
	if (ih.existGetter("e") || ih.existFormer("e") || ih.existGetter("b") || ih.existPath(0)) {
		if (ih.existFormer("e")) { pt = ih.getFormer("e"); }
		else if (ih.existGetter("e")) { pt = ih.getGetter("e"); }
		else if (ih.existGetter("b")) {
			try { pt = base64_decode(ih.getGetter("b"), true); }
			catch (std::runtime_error re) { pt.clear(); }
		}
		else if (ih.existPath(0)) {
			try { pt = base64_decode(ih.getPath(0), true); }
			catch (std::runtime_error re) {
				pt.clear();
				std::stringstream ss;
				ss << "pages/" << ih.getPath(0);
				struct stat buffer;
				if (stat(ss.str().c_str(), &buffer) != 0) {
					oh.addTemplate(ss.str());
				}
			}
		}
	}

	if (!pt.empty()) {
		std::vector<token> tks;
		unsigned char tt = 0;
		char te = 0;
		int pcount = 0;
		double value = 0;

		// normalize tokens
		// parens
		findAndReplaceAll(pt, "{", "(");
		findAndReplaceAll(pt, "[", "(");
		findAndReplaceAll(pt, "}", ")");
		findAndReplaceAll(pt, "]", ")");
		// exp
		findAndReplaceAll(pt, "e", "^");
		// m/d/mod
		findAndReplaceAll(pt, "x", "*");
		findAndReplaceAll(pt, "×", "*");
		findAndReplaceAll(pt, "÷", "/");

		// fixup/sanitize
		bool same = false;
		std::string validtokens = "\\(\\)^*\\/%+-";
		std::stringstream rtoken;
		rtoken << "([^0-9." << validtokens << "]+)";
		std::regex fixer(rtoken.str());
		while (!same) {
			std::string tmp = std::regex_replace(pt, fixer, "");
			same = pt.length() == tmp.length();
			pt = tmp;
		}

		// tokenizer
		rtoken.str(std::string());
		rtoken << "([0-9.]+|[";
		rtoken << validtokens;
		rtoken << "])";

		std::regex re(rtoken.str());
		auto tbeg = std::sregex_iterator(pt.begin(), pt.end(), re);
		auto tend = std::sregex_iterator();
		token prev = token{TOKEN_ADD, 0};
		for(std::sregex_iterator t = tbeg; t != tend; t++) {
			std::smatch match = *t;
			tt = 0;
			if (match.str().size() == 1) {
				te = match.str()[0];
				if (te == '(') { tt = TOKEN_PAREN_OPEN; pcount++;}
				else if (te == ')') { tt = TOKEN_PAREN_CLOSE; }
				else if (te == '^') { tt = TOKEN_EXPONENT; }
				else if (te == '*') { tt = TOKEN_MULT; }
				else if (te == '/') { tt = TOKEN_DIV; }
				else if (te == '%') { tt = TOKEN_MOD; }
				else if (te == '+') { tt = TOKEN_ADD; }
				else if (te == '-') { tt = TOKEN_SUB; }
				else { tt = TOKEN_NUMBER; }
			}
			else { tt = TOKEN_NUMBER; }

			if (tt == TOKEN_NUMBER) { value = std::stod(match.str(), nullptr); }
			else { value = pcount; }

			bool number = tt == TOKEN_NUMBER;
			bool paro = tt == TOKEN_PAREN_OPEN;
			bool parc = tt == TOKEN_PAREN_CLOSE;
			bool paren = paro || parc;

			// TOKEN_NUMBER cannot adjoin TOKEN_NUMBER
			// With parenthesis, the following arrangement is required:
			// TOKEN_PAREN_OPEN -> TOKEN_PAREN_OPEN | TOKEN_NUMBER | TOKEN_PAREN_CLOSE
			// These are invalid:
			// TOKEN_NUMBER -> TOKEN_PAREN_OPEN
			// TOKEN_PAREN_CLOSE -> TOKEN_PAREN_OPEN


			if (paro) {
				if (prev.type == TOKEN_NUMBER || prev.type == TOKEN_PAREN_CLOSE) { tks.push_back(AADD); }
				pcount++;
				value = pcount;
			}
			else if (parc) {
				if (pcount) {
					if (prev.type != TOKEN_NUMBER && prev.type != TOKEN_PAREN_OPEN && prev.type != TOKEN_PAREN_CLOSE) {
						tks.push_back(AZRO);
					}
					tks.push_back(token{tt, value});
					prev.type = tt;
					prev.value = value;
					pcount--;
				}
			}
			else if (number) {
				if (prev.type == TOKEN_NUMBER || prev.type == TOKEN_PAREN_CLOSE) { tks.push_back(AADD); }
			}
			else {
				if (prev.type != TOKEN_NUMBER && prev.type != TOKEN_PAREN_CLOSE) {
					tks.push_back(AONE);
				}
			}

			if (!parc) {
				tks.push_back(token{tt, value});
				prev.type = tt;
				prev.value = value;
			}
		}

		if (tks.size() > 1 && prev.type != TOKEN_NUMBER && prev.type != TOKEN_PAREN_CLOSE) {
			tks.pop_back();
		}
		if (pcount > 0) { // close out all open parens
			for (int a = pcount; a > 0; a--) {
				value = a;
				tks.push_back(token{TOKEN_PAREN_CLOSE, value});
			}
		}

		if (tks.size() > 0) {
			oh.addValue("eqi", stringmaker(tks));
			oh.addValue("coded", base64_encode(stringmaker(tks, false), true));
			solver(oh, tks);
			oh.addValue("answer", stringmaker(tks, true));
			oh.addTemplate("answer.html");
		}
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

std::string numcheck(token &t) {
	std::stringstream ss;
	return ss.str();
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
void stringer(util::OutputHelper &oh, std::string &first, std::vector<token> &data, std::string type, token &left, token &right, token &res) {
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
	else {
		std::string pt = "Begin!";
		std::string str = stringmaker(input, true);
		util::OutputHelper *dt = new util::OutputHelper();
		dt->addValue("texta", pt);
		dt->addValue("textb", str);
		dt->addTemplate("start.html");
		oh.addHelper(dt);
	}


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
				token l = input[a-1], r = input[a + 1];
				token q = token{TOKEN_NUMBER, std::pow(input[a - 1].value, input[a + 1].value)};
				replacer(input, a, q);
				stringer(oh, temp, input, "exp", l, r, q);
				a = 0;
			}
		}
	}

	// mult/div/mod
	if (hasType(TOKEN_MULT, input) || hasType(TOKEN_DIV, input) || hasType(TOKEN_MOD, input)) {
		simpleTexter(oh, "mult divi", "Multiply and Divide/Modulo");
		for (a = 0; a < input.size(); a++) {
			if (input[a].type == TOKEN_MULT || input[a].type == TOKEN_DIV || input[a].type == TOKEN_MOD) {
				std::string temp = stringmaker(input, true);
				token l = input[a-1], r = input[a + 1];
				token q;
				switch (input[a].type) {
					case TOKEN_MULT:
						q = input[a - 1] * input[a + 1];
						replacer(input, a, q);
						stringer(oh, temp, input, "mult", l, r, q);
						break;
					case TOKEN_DIV:
						q = input[a - 1] / input[a + 1];
						replacer(input, a, q);
						stringer(oh, temp, input, "divi", l, r, q);
						break;
					case TOKEN_MOD:
						q = input[a - 1] % input[a + 1];
						replacer(input, a, q);
						stringer(oh, temp, input, "divi", l, r, q);
						break;
				}
				a = 0;
			}
		}
	}


	// add/sub
	if (hasType(TOKEN_ADD, input) || hasType(TOKEN_SUB, input)) {
		simpleTexter(oh, "add sub", "Add and Subtract");
		for (a = 0; a < input.size(); a++) {
			if (input[a].type == TOKEN_ADD || input[a].type == TOKEN_SUB) {
				std::string temp = stringmaker(input, true);
				token l = input[a-1], r = input[a + 1];
				token q;
				switch (input[a].type) {
					case TOKEN_ADD:
						q = input[a - 1] + input[a + 1];
						replacer(input, a, q);
						stringer(oh, temp, input, "add", l, r, q);
						break;
					case TOKEN_SUB:
						q = input[a - 1] - input[a + 1];
						replacer(input, a, q);
						stringer(oh, temp, input, "sub", l, r, q);
						break;
				}
				a = 0;
			}
		}
	}
}
