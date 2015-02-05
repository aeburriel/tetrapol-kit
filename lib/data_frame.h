#pragma once

#include "decoded_frame.h"

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
  Add new decoded frame into data frame processing chain.

  @return true if new data frame is decoded, false otherwise.
  */
bool data_frame_push_decoded_frame(data_frame_t *data_fr, decoded_frame_t *df);

/**
  Copy data frame into output buffer.

  @return size of result.
  */
int data_frame_get_tpdu_data(data_frame_t *data_fr, uint8_t *tpdu_data);

/**
  Destroy data frame instance
  */
void data_frame_destroy(data_frame_t *data_fr);

