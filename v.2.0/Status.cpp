#include "Status.h"

namespace tinyYACC {
	wostream & operator<<(wostream& os, const Status& s)
	{
		for (auto it : *s.items) {
			os << it << endl;
		}
		os << endl;
		return os;
	}
}

