#include "Status.h"

wostream & operator<<(wostream& os, Status& s)
{
	for (auto it : *s.items) {
		os << it << endl;
	}
	os << endl;
	return os;
}

