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
	/**
	 *@brief Init audio
	 *
	 *@param NULL
	 *
	 *@return NULL
	 */
	void beginAudio(void);
	/**
	 *@brief Play mp3 file with directory to mp3 file
	 *
	 *@param[in] Directory to mp3 file ex: /sdcard/song1.mp3
	 *
	 *@return NULL
	 */
	void playMp3File(char *filename);
	/**
	 *@brief Play mp3 file with index of mp3 file
	 *
	 *@param[in] Index of file
	 *
	 *@return NULL
	 */
	void playMp3File(int index);
	/**
	 *@brief
	 *
	 *@param
	 *
	 *@return
	 */
	void playRepeatMp3File(char *filename);
	/**
	 *@brief Pause mp3 file that is playing
	 *
	 *@param[in] NULL
	 *
	 *@return NULL
	 */
	void pauseMp3(void);
	/**
	 *@brief Resume mp3 file that is paused before
	 *
	 *@param[in] NULL
	 *
	 *@return NULL
	 */
	void resumeMp3(void);
	/**
	 *@brief Stop mp3 file that is playing
	 *
	 *@param[in] NULL
	 *
	 *@return NULL
	 */
	void stopMp3(void);
	/**
	 *@brief Check state of Audio periperal
	 *
	 *@param[in] NULL
	 *
	 *@return NULL
	 */
	bool isPlayingMp3(void);
	/**
	 *@brief Init Audio task
	 *
	 *@param
	 *
	 *@return
	 */
	void getFilename(void);
	/**
	 *@brief Play next mp3 file	on list
	 *
	 *@param[in] NULL
	 *
	 *@return NULL
	 */
	void playNextMp3(void);
	/**
	 *
	 */
	audio_element_state_t getStateAudio(void);



private:

	static void initSD(void);
	static FILE *getFile(int nextFile);

	static FILE *closeFile(void);

	static int my_sdcard_read_cb(audio_element_handle_t el, char *buf, int len,
			TickType_t wait_time, void *ctx);

	static void taskRunPipelieAudio(void *param);

};

#ifdef __cplusplus
}
#endif
#endif /* COMPONENTS_AUDIO_AUDIO_H_ */
