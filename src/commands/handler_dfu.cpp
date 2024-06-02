#include <usbd_if.h>

#include "handler_dfu.h"

#define CMD_DFU "DFU"

typedef struct {
    uint32_t Initial_SP;
    void (*ResetHandler)(void);
} t_bootVecTable;


// ----------------------------
cmndDFU::cmndDFU() :
    Command( CMD_DFU ) {
}

void cmndDFU::_doCommand(CmndParser *pars) {
    if ( 1 == (pars->nTokens()) ) {
        processChallenge(0);
    } else {
        int32_t response = strtol(pars->paramStr(1), NULL, 10);
        processChallenge(response);
    }
}

void cmndDFU::processChallenge(int response) {
    if ( response == 0 || challenge == 0) {
        // challenge bootloader trigger command
        challenge = micros() & 0x0FFF;
        Serial.print(F("DFU challenge: "));
        Serial.println(challenge);
        timer.reset();
    } else {
        if ( (!timer.hasTicked()) && response == challenge) {
            // got challenge respone before the timeout
            enterDFU();
        } else {
            challenge = 0;
        }
    }
}

void cmndDFU::enterDFU() {
    t_bootVecTable *_bootVec = (t_bootVecTable*) BOOTLOADER_ADDR;

	/**
	 * Step: Disable RCC, set it to default (after reset) settings
	 *       Internal clock, no PLL, etc.
	 */
#ifdef USBCON
    USBD_reenumerate();
#endif

	HAL_RCC_DeInit();
#if defined(USE_STDPERIPH_DRIVER)
	RCC_DeInit();
#endif /* defined(USE_STDPERIPH_DRIVER) */
	
	/**
	 * Step: Disable systick timer and reset it to default values
	 */
	SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL = 0;
 
	/**
	 * Step: Disable all interrupts
	 */
	__disable_irq();
	
	/**
	 * Step: Remap system memory to address 0x0000 0000 in address space
	 *       For each family registers may be different. 
	 *       Check reference manual for each family.
	 *
	 *       For STM32F4xx, MEMRMP register in SYSCFG is used (bits[1:0])
	 *       For STM32F0xx, CFGR1 register in SYSCFG is used (bits[1:0])
	 *       For others, check family reference manual
	 */
	//Remap by hand... {
#if defined(STM32F4)
	SYSCFG->MEMRMP = 0x01;
#endif
	//} ...or if you use HAL drivers
	//__HAL_SYSCFG_REMAPMEMORY_SYSTEMFLASH();	//Call HAL macro to do this for you
	/* Clear Interrupt Enable Register & Interrupt Pending Register */
	for (uint16_t i = 0; i < sizeof(NVIC->ICER) / sizeof(NVIC->ICER[0]); i++) {
      NVIC->ICER[i] = 0xFFFFFFFF;
      NVIC->ICPR[i] = 0xFFFFFFFF;
    }

	/* Re-enable all interrupts */
	__enable_irq();

/**
	 * Step: Set main stack pointer.
	 *       This step must be done last otherwise local variables in this function
	 *       don't have proper value since stack pointer is located on different position
	 *
	 *       Set direct address location which specifies stack pointer in SRAM location
	 */
	__set_MSP(_bootVec->Initial_SP);
	/**
	 * Step: Actually call our function to jump to set location
	 *       This will start system memory execution
	 */

	_bootVec->ResetHandler();
	
	/**
	 * Step: Connect USB<->UART converter to dedicated USART pins and test
	 *       and test with bootloader works with STM32 Flash Loader Demonstrator software
	 */
}
