#include "Audio.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_types.h"

void example(void) {

	Audio Module_Mp3;
	printf("start audio\n");
	Module_Mp3.beginAudio();
	vTaskDelay(2000 / portTICK_PERIOD_MS);
	Module_Mp3.playMp3File(1);
	vTaskDelay(10000 / portTICK_PERIOD_MS);
	while (1) {
		Module_Mp3.playNextMp3();
		vTaskDelay(6000 / portTICK_PERIOD_MS);
	}

}

