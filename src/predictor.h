//========================================================//
//  predictor.h                                           //
//  Header file for the Branch Predictor                  //
//                                                        //
//  Includes function prototypes and global predictor     //
//  variables and defines                                 //
//========================================================//

#ifndef PREDICTOR_H
#define PREDICTOR_H

#include <stdint.h>
#include <stdlib.h>

//
// Student Information
//
extern const char *studentName;
extern const char *studentID;
extern const char *email;

//------------------------------------//
//      Global Predictor Defines      //
//------------------------------------//
#define NOTTAKEN 0
#define TAKEN 1

// The Different Predictor Types
#define STATIC 0
#define GSHARE 1
#define TOURNAMENT 2
#define CUSTOM 3
extern const char *bpName[];

// Definitions for 2-bit counters
#define SN 0 // predict NT, strong not taken
#define WN 1 // predict NT, weak not taken
#define WT 2 // predict T, weak taken
#define ST 3 // predict T, strong taken

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//
extern int ghistoryBits; // Number of bits used for Global History
extern int lhistoryBits; // Number of bits used for Local History
extern int pcIndexBits;  // Number of bits used for PC index
extern int bpType;       // Branch Prediction Type
extern int verbose;

//------------------------------------//
//    Predictor Function Prototypes   //
//------------------------------------//

// Initialize the predictor
//
void init_predictor();

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint32_t make_prediction(uint32_t pc, uint32_t target, uint32_t direct);

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void train_predictor(uint32_t pc, uint32_t target, uint32_t outcome, uint32_t condition, uint32_t call, uint32_t ret, uint32_t direct);

// Please add your code below, and DO NOT MODIFY ANY OF THE CODE ABOVE
// 

uint16_t readTb(uint8_t *table, int idx, unsigned int entryBits);
void writeTb(uint8_t *table, int idx, uint16_t data, unsigned int entryBits);
void steerPred(uint8_t *table, int idx, bool direction, unsigned int predBitWidth);

void init_tournament();

uint8_t tournament_predict(uint32_t pc);

void train_tournament(uint32_t pc, uint8_t outcome);

typedef struct tournamentPredictor_t {
  int tLocalPcIndexBits = 10;
  int tLocalHistoryEntries = 1 << tLocalPcIndexBits;
  int tLocalHistoryBits = 10;
  int tLocalBhtEntries = 1 << tLocalHistoryBits;
  int tLocalBhtBits = 3;

  int tGlobalHistoryBits = 12;
  int tGlobalPredEntries = 1 << tGlobalHistoryBits;
  int tGlobalPredBits = 2;

  int tChoiceEntries = 1 << tGlobalHistoryBits;
  int tChoicePredBits = 2;

  uint8_t *localHistoryTable;
  uint8_t *localPredTable;
  uint8_t *globalPredTable;
  uint8_t *choicePredTable;
  uint16_t pathHistory;
  bool lastLocalPred;
  bool lastGlobalPred;

  // read
  inline uint16_t readLocalHistoryTb(uint32_t pc) {
    return readTb(localHistoryTable, pc & ((1<<tLocalPcIndexBits)-1), tLocalHistoryBits);
  }
  inline uint16_t readLocalPredTb(uint32_t pattern) {
    return readTb(localPredTable, pattern & ((1<<tLocalHistoryBits)-1), tLocalBhtBits);
  }
  inline uint16_t readGlobalPredTb() {
    return readTb(globalPredTable, pathHistory & ((1<<tGlobalHistoryBits)-1), tGlobalPredBits);
  }
  inline uint16_t readChoicePredTb() {
    return readTb(choicePredTable, pathHistory & ((1<<tGlobalHistoryBits)-1), tChoicePredBits);
  }

  // write
  inline void writeLocalHistoryTb(uint32_t pc, uint16_t data) {
    return writeTb(localHistoryTable, pc & ((1<<tLocalPcIndexBits)-1), data, tLocalHistoryBits);
  }
  inline void writeLocalPredTb(uint32_t pattern, uint16_t data) {
    return writeTb(localPredTable, pattern & ((1<<tLocalHistoryBits)-1), data, tLocalBhtBits);
  }
  inline void writeGlobalPredTb(uint16_t data) {
    return writeTb(globalPredTable, pathHistory & ((1<<tGlobalHistoryBits)-1), data, tGlobalPredBits);
  }
  inline void writeChoicePredTb(uint16_t data) {
    return writeTb(choicePredTable, pathHistory & ((1<<tGlobalHistoryBits)-1), data, tChoicePredBits);
  }

  // steer (predictors only)
  inline void steerLocal(uint32_t pattern, bool dir) {
      // readLocalHistoryTb will mask extraneous pc bits for us
      steerPred(localPredTable, pattern, dir, tLocalBhtBits);
  }
  inline void steerGlobal(bool dir) {
      steerPred(globalPredTable, pathHistory & ((1<<tGlobalHistoryBits)-1), dir, tGlobalPredBits);
  }
  inline void steerChoice(bool dir) {
      steerPred(choicePredTable, pathHistory & ((1<<tGlobalHistoryBits)-1), dir, tChoicePredBits);
  }
} tData;

#endif
