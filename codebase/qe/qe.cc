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
}

RC Filter::getNextTuple(void * data)
{
    RID rid;
    RC rc;
    TableScan *ts = dynamic_cast<TableScan *>(iter);
    if (ts)
    {
	rc = ts->iter->getNextTuple(rid, data);
	printf("theRCis: %d\n", rc);
	return rc;
    }
    IndexScan *is = dynamic_cast<IndexScan *>(iter);
    if (is)
    {
	printf("some other logic!\n");
    }
    return QE_EOF;
}
// ... the rest of your implementations go here


bool Filter::checkScanCondition()
{
    if (compOp == NO_OP) return true;
    if (value == NULL) return false;
    Attribute attr = recordDescriptor[attrIndex];
    // Allocate enough memory to hold attribute and 1 byte null indicator
    void *data = malloc(1 + attr.length);
    // Get record entry to get offset
    SlotDirectoryRecordEntry recordEntry = rbfm->getSlotDirectoryRecordEntry(pageData, currSlot);
    // Grab the given attribute and store it in data
    rbfm->getAttributeFromRecord(pageData, recordEntry.offset, attrIndex, attr.type, data);


    char null;
    memcpy(&null, data, 1);

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
}

