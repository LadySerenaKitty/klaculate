#ifndef util_TokenHolder_h
#define util_TokenHolder_h

#include "../ktok.h"
namespace util {
	class TokenHolder {
		public:
			TokenHolder(token &input);
			TokenHolder(const token &input);
			token& get();
			TokenHolder* getPrev();
			TokenHolder* getNext();
			void setPrev(TokenHolder *input);
			void setNext(TokenHolder *input);
			bool hasPrev();
			bool hasNext();
		private:
			TokenHolder* p;
			TokenHolder* n;
			token data;
	};
}
#endif /* util_TokenHolder_h */
