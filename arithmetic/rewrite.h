#pragma once

#include "expression.h"

namespace arithmetic {

// load basic simplifcation rules into rewrite vector
Expression rewriteBasic();
Expression rewriteUndirected();

}


