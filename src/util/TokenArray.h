#ifndef util_TokenArray_h
#define util_TokenArray_h

#define INVALID_TOKEN 0xffffffff

#include "TokenHolder.h"

namespace util {
	class TokenArray {
		public:
			TokenArray();

			TokenArray getIterator();

			void add(token &data);
			void add(const token &data);
			void addBefore(token &data);
			void addBefore(const token &data);
			void addAfter(token &data);
			void addAfter(const token &data);

			token& get();
			token& prev();
			token& next();
			token& getPrev();
			token& getNext();

			void remove();
			void removePrev();
			void removeNext();

			bool hasPrev();
			bool hasNext();

			std::size_t size();
			std::size_t getCurrentPos();
			void setCurrentPos(std::size_t);
			void clear();

			token& operator[](std::size_t idx);

		private:
			std::size_t count;
			std::size_t curpos;
			TokenHolder *root;
			TokenHolder *cur;

			std::size_t addunit(TokenHolder *c, TokenHolder *th);
			std::size_t removeunit(TokenHolder *c);
	};
}
#endif /* util_TokenArray_h */
