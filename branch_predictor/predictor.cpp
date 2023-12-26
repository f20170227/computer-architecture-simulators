//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include <math.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "PRASHIL BHAVESH PAREKH";
const char *studentID = "A59026482";
const char *email = "prparekh@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = {"Static", "Gshare",
                         "Tournament", "Custom"};

// define number of bits required for indexing the BHT here.
int ghistoryBits = 14; // Number of bits used for Global History
int lhistoryBits = 14; // Number of bits used for local History
int pcIndexBits = 11; //Number of PC indexing bits for pshare predictor
int metaIndexBits = 12; //Number of PC indexing bits for meta predictor
int bpType;            // Branch Prediction Type
int verbose;



uint8_t gshare_prediction;
uint8_t pshare_prediction;


//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
// TODO: Add your own Branch Predictor data structures here
//
// gshare
uint8_t *bht_gshare;
uint64_t ghistory;

// pshare
uint8_t *bht_lshare;
uint16_t *pht_lshare;

//tournament
uint8_t *choose_meta;

//custom
uint8_t *lru_count;
uint8_t *vld_cache;


//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//


// gshare functions


void init_gshare()
{
  int bht_entries = 1 << ghistoryBits;
  bht_gshare = (uint8_t *)malloc(bht_entries * sizeof(uint8_t));
  int i = 0;
  for (i = 0; i < bht_entries; i++)
  {
    bht_gshare[i] = WN;
  }
  ghistory = 0;
}




uint8_t gshare_predict(uint32_t pc)
{
  // get lower ghistoryBits of pc
  uint32_t bht_entries = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (bht_entries - 1);
  uint32_t ghistory_lower_bits = ghistory & (bht_entries - 1);
  uint32_t index = ghistory_lower_bits;
  switch (bht_gshare[index])
  {
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

uint8_t gshare_predict_custom(uint32_t pc)
{
  // get lower ghistoryBits of pc
  uint32_t bht_entries = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (bht_entries - 1);
  uint32_t ghistory_lower_bits = ghistory & (bht_entries - 1);
  uint32_t index = (pc_lower_bits ^ ghistory_lower_bits);
  switch (bht_gshare[index])
  {
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


void train_gshare(uint32_t pc, uint8_t outcome)
{
  // get lower ghistoryBits of pc
  uint32_t bht_entries = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (bht_entries - 1);
  uint32_t ghistory_lower_bits = ghistory & (bht_entries - 1);
  uint32_t index = ghistory_lower_bits;

  // Update state of entry in bht based on outcome
  switch (bht_gshare[index])
  {
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

void train_gshare_custom(uint32_t pc, uint8_t outcome)
{
  // get lower ghistoryBits of pc
  uint32_t bht_entries = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (bht_entries - 1);
  uint32_t ghistory_lower_bits = ghistory & (bht_entries - 1);
  uint32_t index = (pc_lower_bits ^ ghistory_lower_bits);

  // Update state of entry in bht based on outcome
  switch (bht_gshare[index])
  {
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


void cleanup_gshare()
{
  free(bht_gshare);
}



// tournament functions

void init_pshare()
{
    // For 2-level local predictor
    int bht_entries = 1 << lhistoryBits;
    int pht_entries = 1 << pcIndexBits;
    pht_lshare = (uint16_t *)malloc(pht_entries * sizeof(uint16_t));
    bht_lshare = (uint8_t *)malloc(bht_entries * sizeof(uint8_t));
    int i = 0;
    for (int i = 0; i < pht_entries; i++)
    {
        pht_lshare[i] = 0;
    }
    for (int j = 0; j < bht_entries; j++)
    {
        bht_lshare[j] = ST;
    }
    
}



uint8_t pshare_predict(uint32_t pc)
{
    // Prediction for 2-level local shared
  uint32_t pht_entries = 1 << pcIndexBits;
  uint32_t bht_entries = 1 << lhistoryBits;
  uint32_t pc_lower_bits = pc & (pht_entries - 1);
  uint32_t index = pht_lshare[pc_lower_bits];
    switch (bht_lshare[index])
    {
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

void train_pshare(uint32_t pc, uint8_t outcome)
{
  // get lower ghistoryBits of pc
    uint32_t pht_entries = 1 << pcIndexBits;
    uint32_t bht_entries = 1 << lhistoryBits;
    uint32_t pc_lower_bits = pc & (pht_entries - 1);
    uint32_t index = pht_lshare[pc_lower_bits];
    switch (bht_lshare[index])
    {
    case WN:
      bht_lshare[index] = (outcome == TAKEN) ? WT : SN;
      break;
    case SN:
      bht_lshare[index] = (outcome == TAKEN) ? WN : SN;
      break;
    case WT:
      bht_lshare[index] = (outcome == TAKEN) ? ST : WN;
      break;
    case ST:
      bht_lshare[index] = (outcome == TAKEN) ? ST : WT;
      break;
    default:
      printf("Warning: Undefined state of entry in PSHARE BHT!\n");
      break;
    }
  // Update history register
    pht_lshare[pc_lower_bits] = ((pht_lshare[pc_lower_bits] << 1) | outcome) & ((1<<lhistoryBits) - 1);
}



// Initialize the meta predictor for tournament prediction

void init_meta()
{
  int meta_entries = 1 << metaIndexBits;
  choose_meta = (uint8_t *)malloc(meta_entries * sizeof(uint8_t));
  for (int i = 0; i < meta_entries; i++)
  {
      choose_meta[i] = 3;
  }
}


uint8_t meta_predict(uint8_t gshare_prediction, uint8_t pshare_prediction, uint32_t pc)
{
    uint32_t meta_entries = 1 << metaIndexBits;
    uint32_t index = pc & (meta_entries - 1);
    switch (choose_meta[index])
    {
    case 0:
      return gshare_prediction;
    case 1:
      return gshare_prediction;
    case 2:
      return pshare_prediction;
    case 3:
      return pshare_prediction;
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT!\n");
      return gshare_prediction;
    }
}

void train_meta(uint32_t pc, uint8_t outcome, uint8_t gshare_prediction, uint8_t pshare_prediction)
{
    uint32_t meta_entries = 1 << metaIndexBits;
    uint32_t index = pc & (meta_entries - 1);
    int pred = choose_meta[index];
    
    
    if (outcome != gshare_prediction && (gshare_prediction != pshare_prediction))
    {
        if (choose_meta[index] < 3)
        {
            choose_meta[index] = choose_meta[index] + 1;
        }
    }
    
    else if (outcome != pshare_prediction && (gshare_prediction != pshare_prediction))
    {
        if (choose_meta[index] > 0)
        {
            choose_meta[index] = choose_meta[index] - 1;
        }
    }
}



void init_predictor()
{
  switch (bpType)
  {
  case STATIC:
    break;
  case GSHARE:
    init_gshare();
    break;
  case TOURNAMENT:
    init_pshare();
    init_gshare();
    init_meta();
    break;
  case CUSTOM:
          init_pshare();
          init_gshare();
          init_meta();
          break;
  default:
    break;
  }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint32_t make_prediction(uint32_t pc, uint32_t target, uint32_t direct)
{

  // Make a prediction based on the bpType
  switch (bpType)
  {
  case STATIC:
    return TAKEN;
  case GSHARE:
    return gshare_predict(pc);
  case TOURNAMENT:
    gshare_prediction = gshare_predict(pc);
    pshare_prediction = pshare_predict(pc);
    return meta_predict(gshare_prediction,pshare_prediction,pc);
  case CUSTOM:
          gshare_prediction = gshare_predict_custom(pc);
          pshare_prediction = pshare_predict(pc);
          return meta_predict(gshare_prediction,pshare_prediction,pc);
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

void train_predictor(uint32_t pc, uint32_t target, uint32_t outcome, uint32_t condition, uint32_t call, uint32_t ret, uint32_t direct)
{
  if (condition)
  {
    switch (bpType)
    {
    case STATIC:
      return;
    case GSHARE:
      return train_gshare(pc, outcome);
    case TOURNAMENT:
      train_gshare(pc, outcome);
      train_pshare(pc, outcome);
      return train_meta(pc, outcome, gshare_prediction, pshare_prediction);
    case CUSTOM:
            train_gshare_custom(pc, outcome);
            train_pshare(pc, outcome);
            return train_meta(pc, outcome, gshare_prediction, pshare_prediction);
    default:
      break;
    }
  }
}
