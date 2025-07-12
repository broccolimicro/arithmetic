#pragma once

#include "state.h"
#include "iterator.h"
#include "operation.h"

namespace arithmetic {

Value evaluate(ConstOperationSet expr, Operand top, State values);

}

