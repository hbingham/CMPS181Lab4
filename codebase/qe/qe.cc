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
    bool theCond;
    while (rc = iter->getNextTuple(data) == SUCCESS)
    {
	if (theCond = checkScanCondition(data))
{
	   break;
}
    }
    if(!theCond) return -1;
    return rc;
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
    bool result = false;
    if(attrs[attrIndex].type == TypeVarChar)
    {
        uint32_t varcharSize;
        memcpy(&varcharSize, leftVal + offset, VARCHAR_LENGTH_SIZE);

        char recordString[varcharSize + 1];
        memcpy(recordString, leftVal + offset + VARCHAR_LENGTH_SIZE, varcharSize);

        recordString[varcharSize] = '\0';

        result = checkScanCondition(recordString);
    }
    else if(attrs[attrIndex].type == TypeInt)
    {
	uint32_t compData;
	memcpy(&compData, leftVal + offset, INT_SIZE);
	result = checkScanCondition(compData);
    }
    else if(attrs[attrIndex].type == TypeReal)
    {
        float recordReal;
        memcpy(&recordReal, leftVal + offset, REAL_SIZE);
        result = checkScanCondition(recordReal);

    }
    if (result) printf("WOW\n");
    return result;
}





bool Filter::checkScanCondition(uint32_t recordInt)
{
    void * value = cond.rhsValue.data;
    int32_t intValue;
    CompOp compOp = cond.op;
    memcpy (&intValue, value, INT_SIZE);
    printf("%" PRIu32 "\n",intValue);
    switch (compOp)
    {
        case EQ_OP: return recordInt == intValue;
        case LT_OP: return recordInt < intValue;
        case GT_OP: return recordInt > intValue;
        case LE_OP: return recordInt <= intValue;
        case GE_OP: return recordInt >= intValue;
        case NE_OP: return recordInt != intValue;
        case NO_OP: return true;
        // Should never happen
        default: return false;
    }
}


bool Filter::checkScanCondition(float recordReal)
{
    void * value = cond.rhsValue.data;
    float realValue;
    memcpy (&realValue, value, REAL_SIZE);
    CompOp compOp = cond.op;
    switch (compOp)
    {
        case EQ_OP: return recordReal == realValue;
        case LT_OP: return recordReal < realValue;
        case GT_OP: return recordReal > realValue;
        case LE_OP: return recordReal <= realValue;
        case GE_OP: return recordReal >= realValue;
        case NE_OP: return recordReal != realValue;
        case NO_OP: return true;
        // Should never happen
        default: return false;
    }
}

bool Filter::checkScanCondition(char *recordString)
{
    void * value = cond.rhsValue.data;
    CompOp compOp = cond.op;
    if (compOp == NO_OP)
        return true;

    int32_t valueSize;
    memcpy(&valueSize, value, VARCHAR_LENGTH_SIZE);
    char valueStr[valueSize + 1];

    valueStr[valueSize] = '\0';

    memcpy(valueStr, (char*) value + VARCHAR_LENGTH_SIZE, valueSize);

    int cmp = strcmp(recordString, valueStr);
    switch (compOp)
    {
        case EQ_OP: return cmp == 0;
        case LT_OP: return cmp <  0;
        case GT_OP: return cmp >  0;
        case LE_OP: return cmp <= 0;
        case GE_OP: return cmp >= 0;
        case NE_OP: return cmp != 0;
        // Should never happen
        default: return false;
    }
}

