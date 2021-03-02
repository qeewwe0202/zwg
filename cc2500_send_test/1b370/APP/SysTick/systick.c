#include "stm32f0xx.h"
#include "system.h"


/*<span style="font-size:14px;">/** \brief  System Tick Configuration

    The function initializes the System Timer and its interrupt, and starts the System Tick Timer.
    Counter is in free running mode to generate periodic interrupts.

    \param [in]  ticks  Number of ticks between two interrupts.

    \return          0  Function succeeded.
    \return          1  Function failed.

    \note     When the variable <b>__Vendor_SysTickConfig</b> is set to 1, then the
    function <b>SysTick_Config</b> is not included. In this case, the file <b><i>device</i>.h</b>
    must contain a vendor-specific implementation of this function.

 */
__STATIC_INLINE uint32_t SysTick_Config(uint32_t ticks)
{
  if ((ticks - 1) > SysTick_LOAD_RELOAD_Msk)  return (1);      /* Reload value impossible */

  SysTick->LOAD  = ticks - 1;                                  /* set reload register */
  NVIC_SetPriority (SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);  /* set Priority for Systick Interrupt */
  SysTick->VAL   = 0;                                          /* Load the SysTick Counter Value */
  SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                   SysTick_CTRL_TICKINT_Msk   |
                   SysTick_CTRL_ENABLE_Msk;                    /* Enable SysTick IRQ and SysTick Timer */
  return (0);                                                  /* Function successful */
}</span>



<span style="font-size:14px;">#include "SysTick.h"

__IO uint32_t TimingDelay;
uint32_t time_count = 0;
uint16_t idx_led = 0;
//SystemCoreClock/100000) 10 us ??????
void SysTick_Config_Init(void)
{
	/* SystemFrequency / 1000    1ms????
	* SystemFrequency / 100000	 10us????
	* SystemFrequency / 1000000 1us????
	*/
    if(SysTick_Config(SystemCoreClock/1000))
	{
	    while(1);
	}
		// ???????  
	//SysTick->CTRL &= ~ SysTick_CTRL_ENABLE_Msk;
}

/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in milliseconds.
  * @retval None
  */
void Delay_ms(__IO uint32_t nTime)
{ 
	TimingDelay = nTime;
	// ???????  
//	SysTick->CTRL |=  SysTick_CTRL_ENABLE_Msk;

	while(TimingDelay != 0);
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}

void SysTick_Handler(void)
{    
	
	TimingDelay_Decrement();
	
	idx_led++;
        if(idx_led >= 1000){
		idx_led = 0;
	    GPIOA->ODR^=GPIO_Pin_4;	
	}
}</span>