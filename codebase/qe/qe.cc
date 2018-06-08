#include <inttypes.h>
#include <cmath>
#include <typeinfo>
#include <string>
#include <cstring>
#include "qe.h"


Filter::Filter(Iterator* input, const Condition &condition) {
    iter = input;
    cond.lhsAttr  = condition.lhsAttr;
    cond.op = condition.op;
    cond.bRhsIsAttr = condition.bRhsIsAttr;

    cond.rhsAttr = condition.rhsAttr;
    cond.rhsValue = condition.rhsValue;
    attrs.clear();
    input->getAttributes(attrs);
}





RC Filter::getNextTuple(void * data)
{
    RID rid;
    RC rc;
    rc = iter->getNextTuple(data);
    checkScanCondition(data);
    return rc;
    return QE_EOF;
}

void Filter::getAttributes(vector<Attribute> &attrs) const
{
    attrs.clear();
    attrs = this->attrs;
}



bool Filter::checkScanCondition(void  * leftVal)
{
    if (cond.op == NO_OP) return true;
    void * compVal = malloc(PAGE_SIZE);
    uint32_t attrIndex, varCharLen;

//offset = that because of nullByte(s)
    unsigned offset = ceil((float)attrs.size()/8);
    for(int i = 0; i < attrs.size(); i++)
    {
	if(strcmp(attrs[i].name.c_str(), cond.lhsAttr.c_str()) == 0)
	{
	    attrIndex = i;
	    break;
	}
	if (attrs[i].type == TypeVarChar)
	{
	    memcpy(&varCharLen, leftVal + offset, INT_SIZE);
	    offset += INT_SIZE + varCharLen;
	    printf("The varCharLen: %d\n", varCharLen);
	}
	else 
	{
	    offset+= INT_SIZE;
	}
    }

    if (attrIndex == attrs.size())
    {
	printf("couldnt find the attribute hmm\n");
	return 69;
    }

    if(attrs[attrIndex].type == TypeVarChar)
    {
	printf("we dont go here yet\n");
    }
    else
    {
	uint32_t someStuiff;
	memcpy(&someStuiff, leftVal + offset, INT_SIZE);
	printf("%" PRIu32 "\n",someStuiff);
    }

    return true;
/*
    bool result = false;
    if (null)
    {   
        result = false;
    }
    // Checkscan condition on record data and scan value
    else if (attr.type == TypeInt)
    {
        int32_t recordInt;
        memcpy(&recordInt, (char*)data + 1, INT_SIZE);
        result = checkScanCondition(recordInt, compOp, value);
    }
    else if (attr.type == TypeReal)
    {
        float recordReal;
        memcpy(&recordReal, (char*)data + 1, REAL_SIZE);
        result = checkScanCondition(recordReal, compOp, value);
    }
    else if (attr.type == TypeVarChar)
    {
        uint32_t varcharSize;
        memcpy(&varcharSize, (char*)data + 1, VARCHAR_LENGTH_SIZE);

        char recordString[varcharSize + 1];
        memcpy(recordString, (char*)data + 1 + VARCHAR_LENGTH_SIZE, varcharSize);

        recordString[varcharSize] = '\0';

        result = checkScanCondition(recordString, compOp, value);
    }
    free (data);
    return result;
*/
}
