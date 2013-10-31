/*
* NullLiteral.cpp
*
*  Created on: Oct 29, 2013
*      Author: yiwu
*/

#include "NullLiteral.h"

namespace swift {

NullLiteral::NullLiteral(int l, int c)
	:Literal(l, c) {
}

NullLiteral::~NullLiteral() {
}

// For Debugging Use
void NullLiteral::print(FILE* file, int indent) {
	fprintf(file, "%*s(NullLiteral: NULL )\n", indent, "");
}

}