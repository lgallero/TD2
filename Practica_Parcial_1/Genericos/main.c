void HAL_IncTick(void)
{
    uwTick += uwTickFreq;
    fsm_tick();
}