#pragma once

#include "hdlc_frame.h"
#include "data_frame.h"
#include "tsdu.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct _tpdu_ui_t tpdu_ui_t;

tpdu_ui_t *tpdu_ui_create(frame_type_t fr_type);
void tpdu_ui_destroy(tpdu_ui_t *tpdu);
bool tpdu_ui_push_hdlc_frame(tpdu_ui_t *tpdu, const hdlc_frame_t *hdlc_fr);

/**
 * @brief tpdu_ui_get_tsdu Get TSDU if anny decoded is available.
 *
 * Reciever of TSDU is responsible for calling tsdu_destroy.
 *
 * @param tpdu
 * @return TSDU or NULL
 */
tsdu_t *tpdu_ui_get_tsdu(tpdu_ui_t *tpdu);


// old methods
void segmentation_reset(void);
void tpdu_process(const uint8_t *t, int length, int *frame_no);

