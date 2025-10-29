#include "predictor.h"
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

int main() {
    // TODO: Make some test cases to debug
    tData tdata{};
    // initialize tournament predictor
    tdata.localHistoryTable = (uint8_t*)malloc(tdata.tLocalHistoryBits*tdata.tLocalHistoryEntries/8);
    tdata.localPredTable = (uint8_t*)malloc(tdata.tLocalBhtEntries*tdata.tLocalBhtBits/8);
    tdata.globalPredTable = (uint8_t*)malloc(tdata.tGlobalPredBits*tdata.tGlobalPredEntries/8);
    tdata.choicePredTable = (uint8_t*)malloc(tdata.tChoicePredBits*tdata.tChoiceEntries/8);

    uint32_t storeVal0 = 0x123;
    uint32_t storeVal1 = 0x356;
    uint32_t storeVal2 = 0x289;

    // write a value to the table and check for correct write
    tdata.writeLocalHistoryTb(16, storeVal0);
    int result = tdata.readLocalHistoryTb(16);
    printf("%b -> %b\n", storeVal0, result);
    assert(result == storeVal0);

    // write values to addresses adjacent to the original address
    // make sure previously written values have not been tampered with
    tdata.writeLocalHistoryTb(17, storeVal1);
    result = tdata.readLocalHistoryTb(17);
    printf("%b -> %b\n", storeVal1, result);
    assert(result == storeVal1);
    result = tdata.readLocalHistoryTb(16);
    printf("%b -> %b\n", storeVal0, result);
    assert(result == storeVal0);
    

    tdata.writeLocalHistoryTb(15, storeVal2);
    result = tdata.readLocalHistoryTb(15);
    printf("%b -> %b\n", storeVal2, result);
    assert(result == storeVal2);
    result = tdata.readLocalHistoryTb(17);
    printf("%b -> %b\n", storeVal1, result);
    assert(result == storeVal1);
    result = tdata.readLocalHistoryTb(16);
    printf("%b -> %b\n", storeVal0, result);
    assert(result == storeVal0);

    // write values to the edges of the table
    // read values from the edges of the table
    tdata.writeLocalHistoryTb(0, storeVal0);
    result = tdata.readLocalHistoryTb(0);
    printf("%b -> %b\n", storeVal0, result);
    assert(result == storeVal0);

    tdata.writeLocalHistoryTb(tdata.tLocalHistoryEntries-1, storeVal0);
    result = tdata.readLocalHistoryTb(tdata.tLocalHistoryEntries-1);
    printf("%b -> %b\n", storeVal0, result);
    assert(result == storeVal0);

    // Try with 2, 3, and 12 bit widths
}
