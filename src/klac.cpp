#include "klaculate.h"
#include "klac.h"
#include "ktok.h"

#include "solver.h"
#include "util/base64.h"
#include "util/InputHelper.h"
#include "util/OutputHelper.h"
#include "util/TokenArray.h"
#include "util/TokenData.h"
#include "util/TokenHolder.h"

#include <algorithm>
#include <iomanip>
#include <regex>
#include <sstream>

#include <sys/stat.h>

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
		int pcount = 0;

		// everything in lowercase makes life easier
		std::transform(pt.begin(), pt.end(), pt.begin(), [](unsigned char c){ return std::tolower(c); });

		// normalize tokens
		// parens
		findAndReplaceAny(pt, "\\{\\[", "(");
		findAndReplaceAny(pt, "\\}\\]", ")");
		// exp
		//findAndReplaceAny(pt, "e^", "e");
		// m/d/mod
		findAndReplaceAny(pt, "x×∗", "*");
		findAndReplaceAny(pt, "÷∕", "/");
		// a/s
		findAndReplaceAll(pt, "−", "-");
		// specials
		findAndReplaceAny(pt, "Ππ", "pi");

		// fixup/sanitize
		bool same = false;
		std::stringstream rtoken;
		rtoken << "([^0-9a-z.\\(\\)^*\\/%+-]+)";
		std::regex fixer(rtoken.str());
		while (!same) {
			std::string tmp = std::regex_replace(pt, fixer, "");
			same = pt.length() == tmp.length();
			pt = tmp;
		}

		// tokenizer
		rtoken.str(std::string());
		rtoken << "([0-9.]+";
		rtoken << "|[\\(\\)^e*\\/%+-]";
		rtoken << "|pi";
		rtoken << ")";

		std::regex re(rtoken.str());
		auto tbeg = std::sregex_iterator(pt.begin(), pt.end(), re);
		auto tend = std::sregex_iterator();

		for(std::sregex_iterator t = tbeg; t != tend && tks.size() < 1000; t++) {
			std::smatch match = *t;
			std::string m = match.str().substr(0, 50);
			token tk;
			if (m.size() <= 4) {
				for (int a = 0; a < m.size(); a++) { tk.type.str[a] = m[a]; }
				if (isValidOperatorToken(tk)) {
					token tid = tokenIdentity(tk);
					if (tid.type == TOKEN_NUMBER) {
						//tk.type = tid.type;
						tk.value = tid.value;
					}
					else {
						//tk.type = te;
						tk.value = pcount;
					}
				}
				else {
					tk.type = TOKEN_NUMBER;
					try { tk.value = std::stod(m, nullptr); }
					catch (std::invalid_argument ia) { tk.value = 0; }
					catch (std::out_of_range) { tk.value = 0; }
				}
			}
			else {
				tk.type = TOKEN_NUMBER;
				try { tk.value = std::stod(m, nullptr); }
				catch (std::invalid_argument ia) { tk.value = 0; }
				catch (std::out_of_range) { tk.value = 0; }
			}

			bool paro = tk.type == TOKEN_PAREN_OPEN;
			bool parc = tk.type == TOKEN_PAREN_CLOSE;

			if (t == tbeg && tokenLeftIsOperator(tk) && tk.type != TOKEN_PAREN_CLOSE) { tks.add(tokenInsertLeft(tk)); }

			// parenthesis are a special case
			if (paro) {
				pcount++;
				tk.value = pcount;
			}
			else if (parc) {
				if (pcount) {
					tks.add(tk);
					pcount--;
				}
			}

			if (!parc) {
				tks.add(tk);
			}
		} // end tokenizer */

		if (pcount > 0) { // close out all open parens
			double value;
			for (int a = pcount; a > 0; --a) {
				value = a;
				tks.add(token{TOKEN_PAREN_CLOSE, value});
			}
		}

		// fixup using the new TokenData system
		if (tks.size() > 0) {
			tks.setCurrentPos(0);
			token t = tks.get();
			token n = tks.getNext();

			while (tks.hasNext()) {
				if (tokenRightIsNumber(t) && tokenLeftIsNumber(n)) { tks.add(tokenInsertLeft(n)); }
				else if (tokenRightIsOperator(t) && tokenLeftIsOperator(n)) { tks.add(tokenInsertLeft(n)); }
				t = tks.next();
				n = tks.getNext();
			}
			if (tokenRightIsOperator(t)) { tks.add(tokenInsertRight(t)); }
		}

		if (tks.size() > 0) {
			tks.setCurrentPos(0);
			oh.addValue("eqi", stringmaker(tks, false));
			oh.addValue("coded", base64_encode(stringmaker(tks, false), true));
			solver(oh, tks);
			if (tks.size() == 0) { tks.add(token{TOKEN_NUMBER, 0}); }
			tks.setCurrentPos(0);

			oh.addValue("answer", stringmaker(tks, true));

			std::stringstream ss_d;
			ss_d << std::defaultfloat << tks.get().value;
			oh.addValue("answer.def", ss_d.str());

			std::stringstream ss_f;
			ss_f << std::fixed << tks.get().value;
			oh.addValue("answer.fix", ss_f.str());

			std::stringstream ss_s;
			ss_s << std::scientific << tks.get().value;
			oh.addValue("answer.sci", ss_s.str());

			std::stringstream ss_h;
			ss_h << std::hexfloat << tks.get().value;
			oh.addValue("answer.hex", ss_h.str());

			union rawr { double dbl; unsigned char ch[sizeof(double)]; };
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
				tp << "answers/" << m << ans << ".html";
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
	std::string content;
	if (ih.getHeader("HTTP_METHOD").compare("HEAD") != 0) {
		content = oh.getOutput();
	}

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
