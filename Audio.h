/*
 * Audio.h
 *
 *  Created on: Sep 20, 2019
 *      Author: TA QUOC ANH
 */

#ifndef COMPONENTS_AUDIO_AUDIO_H_
#define COMPONENTS_AUDIO_AUDIO_H_

#include "string.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_event_iface.h"
#include "audio_common.h"
#include "fatfs_stream.h"
#include "i2s_stream.h"
#include "mp3_decoder.h"
#include "filter_resample.h"

#include "esp_peripherals.h"
#include "periph_sdcard.h"
#include "periph_touch.h"
#include "input_key_service.h"
#include "periph_adc_button.h"
#include "board.h"
#include "sdkconfig.h"
#include <dirent.h>

#ifdef __cplusplus
extern "C" {
#endif

class Audio {
public:
	Audio();

	virtual ~Audio();

	void beginAudio(void);

	void playMp3File(char *filename);

	void playMp3File(int index);

	void playRepeatMp3File(char *filename);

	void pauseMp3(void);

	void resumeMp3(void);

	void stopMp3(void);

	bool isPlayingMp3(void);

	void getFilename(void);

	void increaseVolum(void);






private:



	static FILE *getFile(int nextFile);

	static int my_sdcard_read_cb(audio_element_handle_t el, char *buf, int len,
			TickType_t wait_time, void *ctx);

	static void taskRunPipelieAudio(void *param);

};

#ifdef __cplusplus
}
#endif
#endif /* COMPONENTS_AUDIO_AUDIO_H_ */
