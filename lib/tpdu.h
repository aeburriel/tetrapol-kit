#pragma once

#include "hdlc_frame.h"
#include "data_frame.h"
#include "tsdu.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct _tpdu_ui_t tpdu_ui_t;

tpdu_ui_t *tpdu_ui_create(frame_type_t fr_type);
void tpdu_ui_destroy(tpdu_ui_t *tpdu);

/**
 * @brief tpdu_ui_push_hdlc_frame
 * Process HDLC frame, optionaly compose frame from segments.
 * @param tpdu
 * @param hdlc_fr
 * @return pointer to HDLC frame for reuse or NULL
 */
hdlc_frame_t *tpdu_ui_push_hdlc_frame(tpdu_ui_t *tpdu, hdlc_frame_t *hdlc_fr);

/**
 * @brief tpdu_ui_push_hdlc_frame2
 * Process HDLC frame, does not allow segmented frames.
 * @param tpdu
 * @param hdlc_fr
 * @return pointer to original HDLC frame for reuse
 */
hdlc_frame_t *tpdu_ui_push_hdlc_frame2(tpdu_ui_t *tpdu, hdlc_frame_t *hdlc_fr);

bool tpdu_ui_has_tsdu(tpdu_ui_t *tpdu);

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

