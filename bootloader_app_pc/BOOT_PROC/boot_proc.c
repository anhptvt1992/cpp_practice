#include "./boot_proc.h"

typedef struct _boot_proc_data_
{
	uint32_t add_flash;
	uint8_t  *data_flash;
	uint16_t check_sum;
	uint16_t msg_lenght;
}BOOT_PROC_DATA;


static BOOT_PROC_DATA boot_data;
static void boot_process_check_sum_data(BOOT_PROC_DATA *boot_data);
static int8_t boot_process_separate_data_file(uint8_t *data, uint16_t data_len, BOOT_PROC_DATA *boot_data);
static int8_t boot_process_trans_data(BOOT_PROC_DATA boot_data);


int8_t boot_process_data_handle(uint8_t *data, uint16_t data_len)
{
	memset(&boot_data, 0, sizeof(BOOT_PROC_DATA));

	while(boot_data.add_flash < data_len)
	{
		if(boot_process_separate_data_file(data, data_len, &(boot_data)) == BOOT_PROC_FAIL)
		{
			free(boot_data.data_flash);
			return BOOT_PROC_FAIL;
		}
		boot_process_check_sum_data(&boot_data);

		if(boot_process_trans_data(boot_data) != BOOT_PROC_FAIL)
		{
			free(boot_data.data_flash);
			return BOOT_PROC_FAIL;
		}
		boot_data.add_flash += boot_data.msg_lenght;
	}
	return BOOT_PROC_SUCCESS;
}

static void boot_process_check_sum_data(BOOT_PROC_DATA *boot_data)
{
	uint8_t *data;
	uint16_t idx;
	uint16_t check_sum;

	data = (uint8_t *)malloc(boot_data->msg_lenght - BOOT_PROC_MAX_CHECKSUM_LEN);

	data[0] = (uint8_t)(boot_data->add_flash >> 24);
	data[1] = (uint8_t)(boot_data->add_flash >> 16);
	data[2] = (uint8_t)(boot_data->add_flash >> 8);
	data[3] = (uint8_t)(boot_data->add_flash);

	for(idx = 4; idx < (boot_data->msg_lenght - BOOT_PROC_MAX_CHECKSUM_LEN); idx++)
	{
		data[idx] = boot_data->data_flash[idx - 4];
	}

	check_sum = 0;
	for(idx = 0; idx < (boot_data->msg_lenght - BOOT_PROC_MAX_CHECKSUM_LEN); idx++)
	{
		check_sum +=(uint16_t)data[idx];
	}

	boot_data->check_sum = 0x01 + (~check_sum);
	free(boot_data->data_flash);
	boot_data->data_flash = data;
}

static int8_t *boot_process_separate_data_file(uint8_t *data, uint16_t data_len, BOOT_PROC_DATA *boot_data)
{
	uint16_t idx;
	uint8_t idx_tr;
	uint16_t data_len_tr;

	if(data_len == 0)
	{
		return BOOT_PROC_FAIL;
	}

	if((data_len - boot_data->add_flash) < BOOT_PROC_MAX_DATA_LEN_FILE)
	{
		data_len_tr = data_len - boot_data->add_flash;
		boot_data->data_flash = (uint8_t *)malloc((size_t)(data_len_tr));
		idx_tr = 0;
		for(idx = boot_data->add_flash; idx < data_len; idx++)
		{
			boot_data->data_flash[idx_tr] = data[idx];
			idx_tr++;
		}
		boot_data->msg_lenght = data_len_tr + 6;
	}
	else
	{
		boot_data->data_flash = (uint8_t *)malloc((size_t)data_len_tr);
		idx_tr = 0;
		for(idx = boot_data->add_flash; idx < ((uint16_t)boot_data->add_flash + (uint16_t)BOOT_PROC_MAX_DATA_LEN_FILE); idx++)
		{
			boot_data->data_flash[idx_tr] = data[idx];
			idx_tr++;
		}
		boot_data->msg_lenght = BOOT_PROC_MAX_DATA_LEN_FILE + 6;
	}

	return BOOT_PROC_SUCCESS;
}

static int8_t boot_process_trans_data(BOOT_PROC_DATA boot_data)
{
	uint8_t *boot_trans;
	uint16_t idx;
	char recv_buff[SERIAL_PORT_PROC_RECV_MAX];
	uint8_t fail_count;

	fail_count = 0;
	boot_trans = (uint8_t *)malloc(boot_data.msg_lenght);

	boot_trans[0] = (uint8_t)(boot_data.add_flash >> 24);
	boot_trans[1] = (uint8_t)(boot_data.add_flash >> 16);
	boot_trans[2] = (uint8_t)(boot_data.add_flash >> 8);
	boot_trans[3] = (uint8_t)(boot_data.add_flash);

	for(idx = 4; idx < (boot_data->msg_lenght - BOOT_PROC_MAX_CHECKSUM_LEN); idx++)
	{
		boot_trans[idx] = boot_data.data_flash[idx - 4];
	}

	boot_trans[idx] = (uint8_t)(boot_data.check_sum >> 8);
	boot_trans[idx + 1] = (uint8_t)(boot_data.check_sum);

	while(1)
	{
		(void)serial_port_write(boot_trans, boot_data.msg_lenght);
		serial_port_read(recv_buff);

		if(strcmp(recv_buff, BOOT_PROC_DATA_TRANS_SUCCESS) == 0)
		{
			break;
		}
		else if(strcmp(recv_buff, BOOT_PROC_DATA_TRANS_FAIL) == 0)
		{
			fail_count++;
			if(fail_count == BOOT_PROC_DATA_TRANS_FAIL_MAX_COUNT)
			{
				return BOOT_PROC_FAIL;
			}
		}
		else if(strcmp(recv_buff, BOOT_PROC_FLASH_MEM_ERROR) == 0)
		{
			return BOOT_PROC_FAIL;
		}
	}

	return BOOT_PROC_SUCCESS;
}