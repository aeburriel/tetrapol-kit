#pragma once

#include "data_block.h"

typedef struct _data_frame_t data_frame_t;

/**
  Create new data frame instance.
  */
data_frame_t *data_frame_create(void);

/**
  Reset internal state of data frame decoder.

  Should be called when stream is interrupted or corrupted.
  */
void data_frame_reset(data_frame_t *data_fr);

/**
  @return number of blocks in current data frame.
  */
int data_frame_blocks(data_frame_t *data_fr);

/**
  Add new decoded frame into data frame processing chain.

  @return true if new data frame is decoded, false otherwise.
  */
bool data_frame_push_data_block(data_frame_t *data_fr, data_block_t *data_blk);

/**
  Copy data from data frame into output buffer.

  @return size of result.
  */
int data_frame_get_data(data_frame_t *data_fr, uint8_t *bits);

/**
  Destroy data frame instance
  */
void data_frame_destroy(data_frame_t *data_fr);

