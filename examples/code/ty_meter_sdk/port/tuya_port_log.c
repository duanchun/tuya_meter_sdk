
void tuya_log_output(const char *log, int size)
{
	ty_uart2_send((void*)log, size);
}


