#include "klaculate.h"
#include "klac.h"
#include "ktok.h"

#include "util/base64.h"
#include "util/InputHelper.h"
#include "util/OutputHelper.h"
#include "util/TokenArray.h"
#include "util/TokenData.h"
#include "util/TokenHolder.h"

#include <cctype>
#include <cmath>
#include <iomanip>
#include <map>
#include <regex>
#include <sstream>
#include <vector>

#include <sys/stat.h>

#define AADD token{TOKEN_ADD, (double)pcount}
#define AMUL token{TOKEN_MULT, (double)pcount}
#define AONE token{TOKEN_NUMBER, (double)1}
#define AZRO token{TOKEN_NUMBER, (double)0}

std::string stringmaker(util::TokenArray &data, bool html);

std::string stringmaker(util::TokenArray &data) { return stringmaker(data, false); }

void solver(util::OutputHelper &oh, util::TokenArray &input);

void runtask(FCGX_Request *rq) {
	util::InputHelper ih(rq->in, rq->envp);
	util::OutputHelper oh;

	oh.addValue((char *) "scheme", ih.getHeader("REQUEST_SCHEME"));
	oh.addValue("host", ih.getHeader("HTTP_HOST"));

	oh.addTemplate("header.html");
	std::string pt = "";
	bool runcalc = true;
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
					runcalc = false;
				}
			}
		}
	}

	double ans = 0;
	bool hasans = false;
	if (!pt.empty() && runcalc) {
		util::TokenArray tks;
		ttype tt;
		char te = 0;
		int pcount = 0;
		double value = 0;

		// normalize tokens
		// parens
		findAndReplaceAny(pt, "\\{\\[", "(");
		findAndReplaceAny(pt, "\\}\\]", ")");
		// exp
		findAndReplaceAny(pt, "e^", "^");
		// m/d/mod
		findAndReplaceAny(pt, "x×∗", "*");
		findAndReplaceAny(pt, "÷∕", "/");
		// a/s
		findAndReplaceAll(pt, "−", "-");

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
				if (std::isdigit(te)) { tt = TOKEN_NUMBER; }
				else { tt = te; }
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

			// TODO: rewrite this section to use the new TokenData system
			if (paro) {
				if (prev.type == TOKEN_NUMBER || prev.type == TOKEN_PAREN_CLOSE) { tks.add(AMUL); }
				pcount++;
				value = pcount;
			}
			else if (parc) {
				if (pcount) {
					if (prev.type != TOKEN_NUMBER && prev.type != TOKEN_PAREN_OPEN && prev.type != TOKEN_PAREN_CLOSE) {
						tks.add(AZRO);
					}
					tks.add(token{tt, value});
					prev.type = tt;
					prev.value = value;
					pcount--;
				}
			}
			else if (number) {
				if (prev.type == TOKEN_NUMBER) { tks.add(AADD); }
				else if (prev.type == TOKEN_PAREN_CLOSE) { tks.add(AMUL); }
			}
			else {
				if (prev.type != TOKEN_NUMBER && prev.type != TOKEN_PAREN_CLOSE) {
					tks.add((AONE));
				}
			}

			if (!parc) {
				tks.add(token{tt, value});
				prev.type = tt;
				prev.value = value;
			}
		} // end tokenizer

		if (pcount > 0) { // close out all open parens
			for (int a = pcount; a > 0; --a) {
				value = a;
				tks.add(token{TOKEN_PAREN_CLOSE, value});
				prev.type = TOKEN_PAREN_CLOSE;
				prev.value = value;
			}
		}

		if (tks.size() > 1 && prev.type != TOKEN_NUMBER && prev.type != TOKEN_PAREN_CLOSE) {
			tks.remove();
		}

		if (tks.size() > 0) {
			oh.addValue("eqi", stringmaker(tks));
			oh.addValue("coded", base64_encode(stringmaker(tks, false), true));
			solver(oh, tks);
			oh.addValue("answer", stringmaker(tks, true));

			std::stringstream ss_f;
			ss_f << std::fixed << tks.get().value;
			oh.addValue("answer.fixed", ss_f.str());

			std::stringstream ss_s;
			ss_s << std::scientific << tks.get().value;
			oh.addValue("answer.sci", ss_s.str());

			std::stringstream ss_h;
			ss_h << std::hexfloat << tks.get().value;
			oh.addValue("answer.hex", ss_h.str());

			union rawr { double dbl; char ch[sizeof(double)]; };
			std::stringstream ss_r;
			ss_r << "0x" << std::setfill('0');;
			rawr r{tks.get().value};
			for (int a = 0; a < sizeof(double); a++) {
				ss_r << std::setw(2) << std::hex << (int)r.ch[a];
			}
			oh.addValue("answer.raw", ss_r.str());
			oh.addValue("answer.len", std::to_string(sizeof(double)));

			oh.addTemplate("answer.html");
			ans = tks[0].value;
			hasans = true;
			std::stringstream ss;
			ss << ans;
			oh.addValue("title", ss.str());
			std::stringstream tp;
			for (auto m : {std::fixed, std::scientific, std::hexfloat, std::defaultfloat}) {
				std::stringstream tp;
				tp << "answers/" << std::setprecision(2) << m << ans << ".html";
				struct stat buffer;
				if (stat(std::string("/usr/home/klaculate/templates/").append(tp.str()).c_str(), &buffer) == 0) {
					oh.addTemplate(tp.str());
					break; // get out of the loop when we find one
				}
			}
		}
	}

	if (!hasans && runcalc) {
		oh.addValue("title", "klaculate");
		oh.addTemplate("default.html");
	}

	oh.addTemplate("footer.html");
	std::string content = oh.getOutput();

	std::stringstream ss;
	if (hasans) {
		ss << "X-Answer: " << ans << "\r\n";
	}
	ss << "Content-Type: text/html; charset=UTF-8\r\n";
	ss << "Content-Length: " << std::to_string(content.length()) << "\r\n\r\n";
	std::string s;
	s = ss.str();

	FCGX_PutStr(s.c_str(), s.size(), rq->out);
	FCGX_PutStr(content.c_str(), content.size(), rq->out);

}

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

bool hasType(char type[sizeof(int)], util::TokenArray &data) {
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
		input.add(AZRO);
		return;
	}
	else if (input.size() == 1) {
		if (input[0].type != TOKEN_NUMBER) {
			input.remove();
			input.add(AZRO);
		}
		return;
	}
	else if (input.size() == 2) {
		if (input[0].type == TOKEN_NUMBER) { input.remove(); }
		else if (input[1].type == TOKEN_NUMBER) {
			input[0] = input[1];
			input.remove();
		}
		else {
			input.clear();
			input.add(AZRO);
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
		while (t.type != TOKEN_INVALID) {
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
		while (t.type != TOKEN_INVALID) {
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
		while (t.type != TOKEN_INVALID) {
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
