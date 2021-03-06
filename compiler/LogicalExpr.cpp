/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * The Initial Developer of this code is David Baum.
 * Portions created by David Baum are Copyright (C) 1999 David Baum.
 * All Rights Reserved.
 *
 * Portions created by John Hansen are Copyright (C) 2005 John Hansen.
 * All Rights Reserved.
 *
 */
#include "LogicalExpr.h"
#include "Bytecode.h"


LogicalExpr::LogicalExpr (Expr *lhs, int op, Expr *rhs)
	: NodeExpr(lhs, rhs), fOp(op)
{
}



Expr* LogicalExpr::Clone(Mapping *b) const
{
	return new LogicalExpr(Get(0)->Clone(b), fOp, Get(1)->Clone(b));
}


bool LogicalExpr::Evaluate(int &v) const
{
	int v1, v2;
	bool d1, d2;
	bool b1, b2;

	bool dom;

	dom = (fOp==kLogicalOr) ? true : false;

	d1 = Get(0)->Evaluate(v1);
	d2 = Get(1)->Evaluate(v2);

	// convert to bool for easy check
	b1 = v1;
	b2 = v2;

	if ((d1 && (b1 == dom)) ||
		(d2 && (b2 == dom)))
	{
		v = dom ? 1 : 0;
		return true;
	}

	if (d1 && d2)
	{
		v = dom ? 0 : 1;
		return true;
	}

	return false;
}


bool LogicalExpr::EmitBranch_(Bytecode &b, int label, bool condition) const
{
	bool eitherOne;

	if (fOp == kLogicalOr)
		eitherOne = condition;
	else
		eitherOne = !condition;


	if (eitherOne)
	{
		/*
			test c1 -> label
			test c2 -> label
		*/

		if (!Get(0)->EmitBranch(b, label, condition)) return false;
		if (!Get(1)->EmitBranch(b, label, condition)) return false;
	}
	else
	{
		/*
			test !c1 -> out
			test c2->label
		out:
		*/
		int out = b.NewLabel();

		if (!Get(0)->EmitBranch(b, out, !condition)) return false;
		if (!Get(1)->EmitBranch(b, label, condition)) return false;

		b.SetLabel(out);
	}

	return true;
}
