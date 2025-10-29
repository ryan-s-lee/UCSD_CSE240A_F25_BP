//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include "predictor.h"
#include <cstdint>
#include <cstdlib>
#include <math.h>
#include <stdio.h>

#define BITS_PER_BYTE 8

//
// TODO:Student Information
//
const char *studentName = "Ryan Lee";
const char *studentID = "A17045234";
const char *email = "rsl001@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = {"Static", "Gshare", "Tournament", "Custom"};

// define number of bits required for indexing the BHT here.
int ghistoryBits = 15; // Number of bits used for Global History
int bpType;            // Branch Prediction Type
int verbose;

// tournament

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
// TODO: Add your own Branch Predictor data structures here
//
// gshare
uint8_t *bht_gshare;
uint64_t ghistory;

// tournament

// general purpose
uint16_t readTb(uint8_t *table, int idx, unsigned int entryBits) {
  // TODO: given an index, number of entries, and bits per entry,
  // return the value. Consider adding numEntries for memory safety
  // computers are dumb, can only access at byte level.
  // convert index to byte position
  int bitOffset = entryBits * idx;
  int byteOffset = bitOffset / 8;
  int intrabyteBitOffset = bitOffset - byteOffset * 8;
  uint8_t *vicinity = table + byteOffset;
  uint8_t v1{0}, v2{0};

  v1 = vicinity[0] >> intrabyteBitOffset |
       ((vicinity[1] & ((1 << intrabyteBitOffset) - 1))
        << (8 - intrabyteBitOffset));
  v2 = vicinity[1] >> intrabyteBitOffset |
       (intrabyteBitOffset <= 16 - entryBits
            ? 0
            : (vicinity[2] & ((1 << intrabyteBitOffset) - 1))
                  << (8 - intrabyteBitOffset));
  return ((uint16_t)v1 | ((uint16_t)v2 << 8)) & ((1 << entryBits) - 1);
}

void writeTb(uint8_t *table, int idx, uint16_t data, unsigned int entryBits) {
  int bitOffset = entryBits * idx;
  int byteOffset = bitOffset / 8;
  int intrabyteBitOffset = bitOffset - byteOffset * 8;
  uint8_t *vicinity = table + byteOffset;

  // special case: all of the data goes in one byte
  if (intrabyteBitOffset + entryBits <= 8) {
    uint8_t real_data = (data & ((1 << entryBits) - 1)) << intrabyteBitOffset;
    uint8_t occlusion_mask = ~(((1 << entryBits) - 1) << intrabyteBitOffset);
    *vicinity = (*vicinity & occlusion_mask) | real_data;
    return;
  }

  unsigned int remainingBits = entryBits;

  // determine which portion of the first byte should be replaced with
  // new data
  *vicinity = (*vicinity & ((1 << intrabyteBitOffset) - 1)) |
              (data << intrabyteBitOffset);
  // pop data/shift
  data >>= 8 - intrabyteBitOffset;
  ++vicinity;
  remainingBits -= 8 - intrabyteBitOffset;

  // fill middle bytes with new data
  while (remainingBits > 8) {
    *vicinity = data;

    // pop data/shift
    data >>= 8;
    ++vicinity;
    remainingBits -= 8;
  }

  // determine which portion of the last byte should be replaced with
  // new data
  *vicinity = (*vicinity & ~((1 << remainingBits) - 1)) | (data & ((1<<remainingBits)-1));
}

void steerPred(uint8_t *table, int idx, bool direction, unsigned int predBitWidth) {
    int maxPred = (1 << predBitWidth) - 1;
    int minPred = 0;  // please const propagate lmao
    uint8_t curBias = readTb(table, idx, predBitWidth);
    // compute new bias, clamping between max/min values
    uint8_t newBias = curBias;
    if (direction && newBias < maxPred) {
            newBias += 1;
    } else if (!direction && newBias > minPred) {
        newBias -= 1;
    }
    writeTb(table, idx, newBias, predBitWidth);
}

tData tdata{};

// custom

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// tournament predictor functions

// Initialize the predictor

// gshare functions
void init_gshare() {
  int bht_entries = 1 << ghistoryBits;
  bht_gshare = (uint8_t *)malloc(bht_entries * sizeof(uint8_t));
  int i = 0;
  for (i = 0; i < bht_entries; i++) {
    bht_gshare[i] = WN;
  }
  ghistory = 0;
}

uint8_t gshare_predict(uint32_t pc) {
  // get lower ghistoryBits of pc
  uint32_t bht_entries = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (bht_entries - 1);
  uint32_t ghistory_lower_bits = ghistory & (bht_entries - 1);
  uint32_t index = pc_lower_bits ^ ghistory_lower_bits;
  switch (bht_gshare[index]) {
  case WN:
    return NOTTAKEN;
  case SN:
    return NOTTAKEN;
  case WT:
    return TAKEN;
  case ST:
    return TAKEN;
  default:
    printf("Warning: Undefined state of entry in GSHARE BHT!\n");
    return NOTTAKEN;
  }
}

void train_gshare(uint32_t pc, uint8_t outcome) {
  // get lower ghistoryBits of pc
  uint32_t bht_entries = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (bht_entries - 1);
  uint32_t ghistory_lower_bits = ghistory & (bht_entries - 1);
  uint32_t index = pc_lower_bits ^ ghistory_lower_bits;

  // Update state of entry in bht based on outcome
  switch (bht_gshare[index]) {
  case WN:
    bht_gshare[index] = (outcome == TAKEN) ? WT : SN;
    break;
  case SN:
    bht_gshare[index] = (outcome == TAKEN) ? WN : SN;
    break;
  case WT:
    bht_gshare[index] = (outcome == TAKEN) ? ST : WN;
    break;
  case ST:
    bht_gshare[index] = (outcome == TAKEN) ? ST : WT;
    break;
  default:
    printf("Warning: Undefined state of entry in GSHARE BHT!\n");
    break;
  }

  // Update history register
  ghistory = ((ghistory << 1) | outcome);
}

void cleanup_gshare() { free(bht_gshare); }

// TOURNAMENT
// -----------------------------------------------------------------------------

void init_tournament() {
    tdata.localHistoryTable = (uint8_t*)malloc(tdata.tLocalHistoryBits*tdata.tLocalHistoryEntries/8);
    tdata.localPredTable = (uint8_t*)malloc(tdata.tLocalBhtEntries*tdata.tLocalBhtBits/8);
    tdata.globalPredTable = (uint8_t*)malloc(tdata.tGlobalPredBits*tdata.tGlobalPredEntries/8);
    tdata.choicePredTable = (uint8_t*)malloc(tdata.tChoicePredBits*tdata.tChoiceEntries/8);
}

uint8_t tournament_predict(uint32_t pc) {
  bool localPred = tdata.readLocalPredTb(tdata.readLocalHistoryTb(pc)) >= (1 << (tdata.tLocalBhtBits-1));
  tdata.lastLocalPred = localPred;
  bool globPred = tdata.readGlobalPredTb() >= (uint16_t)(1 << (tdata.tGlobalPredBits-1));
  tdata.lastGlobalPred = globPred;

  if (localPred == globPred) {
      return localPred;
  }

  // conflict
  bool choicePred = tdata.readChoicePredTb() >= (uint16_t)(1 << (tdata.tChoicePredBits-1));
  return choicePred ? globPred : localPred;
}

void train_tournament(uint32_t pc, uint8_t outcome) {
    uint16_t localPattern = tdata.readLocalHistoryTb(pc);
    tdata.steerLocal(localPattern, outcome);
    tdata.steerGlobal(outcome);

    // if conflict, steer bias for choice toward the correct predictor
    if (tdata.lastGlobalPred != tdata.lastLocalPred) {
        // steer direction is positive (1) for global, negative (0) for local
        bool steerDirection = tdata.lastGlobalPred == outcome;
        tdata.steerChoice(steerDirection);
    }

    // update path history
    tdata.pathHistory <<= 1;
    tdata.pathHistory += outcome == TAKEN ? TAKEN : NOTTAKEN;
    
    // TODO: update local history
    localPattern <<= 1;
    localPattern += outcome == TAKEN ? TAKEN : NOTTAKEN;
    tdata.writeLocalHistoryTb(pc, localPattern);

}

// CUSTOM (HYBRID PERCEPTRON/TAGE)
// -----------------------------------------------------------------------------

void init_custom() {
  // TODO
}

uint8_t custom_predict(uint32_t pc) {
  // TODO
  return 0;
}

void train_custom(uint32_t pc, uint8_t outcome) {
  // TODO
}

void init_predictor() {
  switch (bpType) {
  case STATIC:
    break;
  case GSHARE:
    init_gshare();
    break;
  case TOURNAMENT:
    init_tournament();
    break;
  case CUSTOM:
    init_custom();
    break;
  default:
    break;
  }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint32_t make_prediction(uint32_t pc, uint32_t target, uint32_t direct) {

  // Make a prediction based on the bpType
  switch (bpType) {
  case STATIC:
    return TAKEN;
  case GSHARE:
    return gshare_predict(pc);
  case TOURNAMENT:
    return tournament_predict(pc);
  case CUSTOM:
    return custom_predict(pc);
  default:
    break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//

void train_predictor(uint32_t pc, uint32_t target, uint32_t outcome,
                     uint32_t condition, uint32_t call, uint32_t ret,
                     uint32_t direct) {
  if (condition) {
    switch (bpType) {
    case STATIC:
      return;
    case GSHARE:
      return train_gshare(pc, outcome);
    case TOURNAMENT:
      return train_tournament(pc, outcome);
    case CUSTOM:
      return train_custom(pc, outcome);
    default:
      break;
    }
  }
}
