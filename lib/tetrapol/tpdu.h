#pragma once

#include <tetrapol/hdlc_frame.h>
#include <tetrapol/data_frame.h>
#include <tetrapol/tsdu.h>
#include <tetrapol/timer.h>

#include <stdbool.h>
#include <stdint.h>

typedef struct _tpdu_t tpdu_t;
typedef struct _tpdu_ui_t tpdu_ui_t;

tpdu_t *tpdu_create(void);
bool tpdu_push_hdlc_frame(tpdu_t *tpdu, const hdlc_frame_t *hdlc_fr);
void tpdu_destroy(tpdu_t *tpdu);
tsdu_t *tpdu_ui_get_tsdu(tpdu_ui_t *tpdu);
void tpdu_du_tick(const timeval_t *tv, void *tpdu_du);

tpdu_ui_t *tpdu_ui_create(frame_type_t fr_type);
void tpdu_ui_destroy(tpdu_ui_t *tpdu);

/**
 * @brief tpdu_ui_push_hdlc_frame
 * Process HDLC frame, optionaly compose frame from segments.
 * @param tpdu
 * @param hdlc_fr
 * @return true if TSDU is available false othervise
 */
bool tpdu_ui_push_hdlc_frame(tpdu_ui_t *tpdu, const hdlc_frame_t *hdlc_fr);

/**
 * @brief tpdu_ui_push_hdlc_frame2
 * Process HDLC frame, does not allow segmented frames.
 * @param tpdu
 * @param hdlc_fr
 * @return true if TSDU is available false othervise
 */
bool tpdu_ui_push_hdlc_frame2(tpdu_ui_t *tpdu, const hdlc_frame_t *hdlc_fr);
