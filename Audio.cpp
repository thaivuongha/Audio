/*
 * Audio.cpp
 *
 *  Created on: Sep 20, 2019
 *      Author: TA QUOC ANH
 */

#include "Audio.h"

#include "sys/dirent.h"
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include "string.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "esp_vfs_fat.h"
#include "audio_pipeline.h"

static const char *TAG = "MP3_COMPONENTS";
static const char *mp3_file[] = { "/sdcard/test.mp3", "/sdcard/test1.mp3",
		"/sdcard/test2.mp3", };

#define MP3_FILE_COUNT sizeof(mp3_file)/sizeof(char*)

#define CURRENT 0
#define NEXT    4
#define SD_BOARD_IOTMAKER 1
#define SD_BOARD_LYRA 0
#define BCK_PIN   5
#define DATA_PIN  26
#define LRCK_PIN  25

static audio_pipeline_handle_t pipeline;

static audio_element_handle_t i2s_stream_writer, mp3_decoder;

static char **nameListMp3;

static int numMp3;

static int currentIndex;

static FILE *file;

Audio::Audio() {
	// TODO Auto-generated constructor stub
}

Audio::~Audio() {
	// TODO Auto-generated destructor stub
}

void Audio::initSD(void) {

}

void Audio::beginAudio(void) {

	xTaskCreate(taskRunPipelieAudio, "Task audio", 4 * 1024, NULL, 3, NULL);
	/*
	 * wait 1s for complete create task and init SD card before working with file system
	 */
	vTaskDelay(2000 / portTICK_PERIOD_MS);

	DIR *folder;
	struct dirent *entry;
	folder = opendir("/sdcard");
	int files = 0;
	if (folder == NULL) {
		perror("Unable to read directory");
	}
	while ((entry = readdir(folder))) {
		if (strstr(strlwr(entry->d_name), ".mp3") != NULL) {
			numMp3++;
		}
	}
	closedir(folder);
	nameListMp3 = (char**) calloc(numMp3, sizeof(char*));
	folder = opendir("/sdcard");
	if (folder == NULL) {
		perror("Unable to read directory");
	}
	while ((entry = readdir(folder))) {
		nameListMp3[files] = (char*) calloc(30, sizeof(char));
		if (nameListMp3[files] == NULL) {
			printf("err call memory\n");
		}
		if (strstr(strlwr(entry->d_name), ".mp3") != NULL) {
			strcpy(nameListMp3[files], "/sdcard/");
			strcat(nameListMp3[files], strlwr(entry->d_name));
			printf("file name %s\n", nameListMp3[files]);
			files++;
		}
	}
	closedir(folder);
}

void Audio::pauseMp3(void) {
	audio_element_state_t el_state = audio_element_get_state(i2s_stream_writer);
	if (el_state == AEL_STATE_RUNNING) {
		audio_pipeline_pause(pipeline);
	}
}

void Audio::resumeMp3(void) {
	audio_element_state_t el_state = audio_element_get_state(i2s_stream_writer);
	if (el_state == AEL_STATE_PAUSED) {
		audio_pipeline_resume(pipeline);
	}
}

void Audio::playMp3File(char *filename) {

}

void Audio::playMp3File(int index) {

	currentIndex = index;
	if (currentIndex - 1 >= numMp3 || currentIndex <= 0) {
		printf("Index is invalid.\nPlease check number of file Mp3\n");
		return;
	}
	index--;
	audio_element_state_t el_state = audio_element_get_state(i2s_stream_writer);
	if (el_state == AEL_STATE_INIT) {
		Audio::stopMp3();
		getFile(index);
		audio_pipeline_run(pipeline);

	} else if (el_state == AEL_STATE_PAUSED) {
		Audio::stopMp3();
		getFile(index);
		audio_pipeline_resume(pipeline);

	} else if (el_state == AEL_STATE_FINISHED) {
		Audio::stopMp3();
		getFile(index);
		audio_pipeline_run(pipeline);

	} else if (el_state == AEL_STATE_STOPPED) {
		Audio::stopMp3();
		getFile(index);
		audio_pipeline_run(pipeline);

	} else if (el_state == AEL_STATE_RUNNING) {
		Audio::stopMp3();
		getFile(index);
		audio_pipeline_run(pipeline);
	}

}

void Audio::playNextMp3(void) {

	if (currentIndex >= numMp3) {
		currentIndex = 0;
	}
	currentIndex++;
	Audio::playMp3File(currentIndex);
}

void Audio::playRepeatMp3File(char *filename) {

}

void Audio::stopMp3(void) {
		audio_pipeline_pause(pipeline);
		if (file != NULL) {
			fclose(file);
			file = NULL;
		}
		audio_pipeline_stop(pipeline);
		audio_pipeline_wait_for_stop(pipeline);
}

bool Audio::isPlayingMp3(void) {

	audio_element_state_t el_state = audio_element_get_state(i2s_stream_writer);
	if (el_state == AEL_STATE_RUNNING)
		return true;
	else
		return false;
}

audio_element_state_t Audio::getStateAudio(void) {

	audio_element_state_t el_state = audio_element_get_state(i2s_stream_writer);

	if (el_state == (audio_element_state_t) AEL_STATE_NONE) {
		return (audio_element_state_t) AEL_STATE_NONE;
	} else if (el_state == (audio_element_state_t) AEL_STATE_INIT) {
		return (audio_element_state_t) AEL_STATE_INIT;
	} else if (el_state == (audio_element_state_t) AEL_STATE_RUNNING) {
		return (audio_element_state_t) AEL_STATE_RUNNING;
	} else if (el_state == (audio_element_state_t) AEL_STATE_PAUSED) {
		return (audio_element_state_t) AEL_STATE_PAUSED;
	} else if (el_state == (audio_element_state_t) AEL_STATE_STOPPED) {
		return (audio_element_state_t) AEL_STATE_STOPPED;
	} else if (el_state == (audio_element_state_t) AEL_STATE_FINISHED) {
		return (audio_element_state_t) AEL_STATE_FINISHED;
	} else {
		return (audio_element_state_t) AEL_STATE_ERROR;
	}

}

void Audio::taskRunPipelieAudio(void *param) {

	esp_log_level_set("*", ESP_LOG_INFO);


	ESP_LOGI(TAG, "[1.0] Initialize peripherals management");
	esp_periph_config_t periph_cfg =
					DEFAULT_ESP_PERIPH_SET_CONFIG();
					esp_periph_set_handle_t set = esp_periph_set_init(&periph_cfg);

#if (SD_BOARD_IOTMAKER)
					ESP_LOGI(TAG, "Initializing SD card");
					sdmmc_host_t host = SDMMC_HOST_DEFAULT();
					host.flags = SDMMC_HOST_FLAG_1BIT;
					sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
					esp_vfs_fat_sdmmc_mount_config_t mount_config = {.format_if_mount_failed =
						false, .max_files = 5};
					sdmmc_card_t* card;
					esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config,
							&mount_config, &card);
					if (ret != ESP_OK) {
						if (ret == ESP_FAIL) {
							ESP_LOGE(TAG,
									"Failed to mount filesystem. If you want the card to be formatted, set format_if_mount_failed = true.");
						} else {
							ESP_LOGE(TAG,
									"Failed to initialize the card (%d). Make sure SD card lines have pull-up resistors in place.",
									ret);
							ESP_LOGE(TAG,
									"Please insert SD Card");

						}
					}

			//		sdmmc_card_print_info(stdout, card);
					// All done, unmount partition and disable SDMMC host peripheral
#endif
#if (SD_BOARD_LYRA)

					periph_sdcard_cfg_t sdcard_cfg;
					sdcard_cfg.root = "/sdcard";
					sdcard_cfg.card_detect_pin = GPIO_NUM_34;
					esp_periph_handle_t sdcard_handle = periph_sdcard_init(&sdcard_cfg);
					ESP_LOGI(TAG, "[1.1] Start SD card peripheral");
					esp_periph_start(set, sdcard_handle);

					while (!periph_sdcard_is_mounted(sdcard_handle)) {
						vTaskDelay(100 / portTICK_PERIOD_MS);
					}

					ESP_LOGI(TAG, "[ 2 ] Start codec chip");
					audio_board_handle_t board_handle = audio_board_init();
					audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_DECODE,
							AUDIO_HAL_CTRL_START);
#endif

					ESP_LOGI(TAG, "[4.0] Create audio pipeline for playback");
					audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
					pipeline = audio_pipeline_init(&pipeline_cfg);
					mem_assert(pipeline);

					ESP_LOGI(TAG, "[4.1] Create i2s stream to write data to codec chip");
					i2s_stream_cfg_t i2s_cfg;	// = I2S_STREAM_CFG_DEFAULT();

					i2s_cfg.type = AUDIO_STREAM_WRITER,
	i2s_cfg.task_prio = I2S_STREAM_TASK_PRIO, i2s_cfg.task_core =
	I2S_STREAM_TASK_CORE, i2s_cfg.task_stack = I2S_STREAM_TASK_STACK, i2s_cfg.out_rb_size =
	I2S_STREAM_RINGBUFFER_SIZE,

	i2s_cfg.i2s_config.mode = (i2s_mode_t) (I2S_MODE_MASTER | I2S_MODE_TX
			| I2S_MODE_RX), i2s_cfg.i2s_config.sample_rate = (int) 44100, i2s_cfg.i2s_config.bits_per_sample =
			(i2s_bits_per_sample_t) 16, i2s_cfg.i2s_config.channel_format =
			I2S_CHANNEL_FMT_RIGHT_LEFT, i2s_cfg.i2s_config.communication_format =
			I2S_COMM_FORMAT_I2S, i2s_cfg.i2s_config.dma_buf_count = 3, i2s_cfg.i2s_config.dma_buf_len =
			300, i2s_cfg.i2s_config.use_apll = 1, i2s_cfg.i2s_config.intr_alloc_flags =
	ESP_INTR_FLAG_LEVEL2,

	i2s_cfg.i2s_port = (i2s_port_t) 0, i2s_cfg.i2s_config.sample_rate = 48000;
	i2s_cfg.type = AUDIO_STREAM_WRITER;
	i2s_stream_writer = i2s_stream_init(&i2s_cfg);

#if (SD_BOARD_IOTMAKER)
	i2s_pin_config_t pin_config = { .bck_io_num = BCK_PIN,
			.ws_io_num = LRCK_PIN, .data_out_num = DATA_PIN, .data_in_num = -1 //Not used
			};
	i2s_set_pin((i2s_port_t) 0, &pin_config);
#endif

	ESP_LOGI(TAG, "[4.2] Create mp3 decoder to decode mp3 file");
	mp3_decoder_cfg_t mp3_cfg = DEFAULT_MP3_DECODER_CONFIG();
	mp3_decoder = mp3_decoder_init(&mp3_cfg);
	audio_element_set_read_cb(mp3_decoder, (stream_func) my_sdcard_read_cb,
	NULL);

	/* ZL38063 audio chip on board of ESP32-LyraTD-MSC does not support 44.1 kHz sampling frequency,
	 so resample filter has been added to convert audio data to other rates accepted by the chip.
	 You can resample the data to 16 kHz or 48 kHz.
	 */

	ESP_LOGI(TAG, "[4.3] Create resample filter");
	rsp_filter_cfg_t rsp_cfg = DEFAULT_RESAMPLE_FILTER_CONFIG();
	audio_element_handle_t rsp_handle = rsp_filter_init(&rsp_cfg);

	ESP_LOGI(TAG, "[4.4] Register all elements to audio pipeline");
	audio_pipeline_register(pipeline, mp3_decoder, "mp3");
	audio_pipeline_register(pipeline, i2s_stream_writer, "i2s");
	audio_pipeline_register(pipeline, rsp_handle, "filter");
	const char *link[] = { "mp3", "filter", "i2s" };

	ESP_LOGI(TAG,
			"[4.5] Link it together [my_sdcard_read_cb]-->mp3_decoder-->i2s_stream-->[codec_chip]");
	audio_pipeline_link(pipeline, link, 3);

	ESP_LOGI(TAG, "[5.0] Set up  event listener");
	audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
	audio_event_iface_handle_t evt = audio_event_iface_init(&evt_cfg);

	ESP_LOGI(TAG, "[5.1] Listen for all pipeline events");
	audio_pipeline_set_listener(pipeline, evt);

	ESP_LOGW(TAG, "[ 6 ] Press the keys to control music player:");
	ESP_LOGW(TAG, "      [Play] to start, pause and resume, [Set] next song.");
	ESP_LOGW(TAG, "      [Vol-] or [Vol+] to adjust volume.");

	while (1) {
		/* Handle event interface messages from pipeline
		 to set music info and to advance to the next song
		 */
		audio_event_iface_msg_t msg;
		esp_err_t ret = audio_event_iface_listen(evt, &msg, portMAX_DELAY);
		if (ret != ESP_OK) {
			ESP_LOGE(TAG, "[ * ] Event interface error : %d", ret);
			continue;
		}
		if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT) {
			// Set music info for a new song to be played
			if (msg.source == (void *) mp3_decoder
					&& msg.cmd == AEL_MSG_CMD_REPORT_MUSIC_INFO) {
				audio_element_info_t music_info = { 0 };
				audio_element_getinfo(mp3_decoder, &music_info);
				ESP_LOGI(TAG,
						"[ * ] Received music info from mp3 decoder, sample_rates=%d, bits=%d, ch=%d",
						music_info.sample_rates, music_info.bits,
						music_info.channels);
				audio_element_setinfo(i2s_stream_writer, &music_info);
				rsp_filter_set_src_info(rsp_handle, music_info.sample_rates,
						music_info.channels);
				continue;
			}
			// Advance to the next song when previous finishes
			if (msg.source == (void *) i2s_stream_writer
					&& msg.cmd == AEL_MSG_CMD_REPORT_STATUS) {
				audio_element_state_t el_state = audio_element_get_state(
						i2s_stream_writer);
				if (el_state == AEL_STATE_FINISHED) {
					ESP_LOGI(TAG, "[ * ] Finished, advancing to the next song");
//					audio_pipeline_stop(pipeline);
//					audio_pipeline_wait_for_stop(pipeline);
					audio_pipeline_terminate(pipeline);
//					getFile(NEXT);
//					audio_pipeline_run(pipeline);
				} else if (el_state == AEL_STATE_STOPPED) {
					printf("stopped heroinertup\n");
				}
				continue;
			}
		}
	}
}

int Audio::my_sdcard_read_cb(audio_element_handle_t el, char *buf, int len,
		TickType_t wait_time, void *ctx) {

	int read_len = fread(buf, 1, len, getFile(currentIndex));
	if (read_len == 0) {
		read_len = AEL_IO_DONE;
	}
	return read_len;
}

void Audio::getFilename(void) {

}

FILE *Audio::getFile(int next_file) {

	if (file == NULL) {
		file = fopen(nameListMp3[next_file], "r");
		if (!file) {
			ESP_LOGE(TAG, "Error opening file");
			return NULL;
		}
	}

	return file;
}
