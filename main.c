#include "cy_pdl.h"
#include "stdio.h"


/******************************************************************************
 * Function prototype
 * ***************************************************************************/
/* SysClk function */
static void systemStart(void);

static void clock_FllConfig(void);
static void clock_EcoConfig(void);

/* Error fucntion */
static void errorOccur(void);

/* PWM config function */
static void pwmStart(void);

/* UART config function */
static void uartStart(void);

static void uartStartLoopCycle(void);

/* Print sestem settings function */
static void printSystemSettings(void);


/******************************************************************************
 * Clock and system defines
 * ***************************************************************************/
#define MHZ                                     (1000000U)

#define SYSTEM_CLOCKS_FREQ                      (100U) /* MHz */
#define CLK_HF_NUM                              (0U)
#define CLK_HF0                                 (SYSTEM_CLOCKS_FREQ * MHZ)
#define CLK_FAST_DIV                            (0U)
#define CLK_FAST                                (SYSTEM_CLOCKS_FREQ * MHZ)
#define CLK_PERI_DIV                            (0U)
#define CLK_PERI                                (SYSTEM_CLOCKS_FREQ * MHZ)

#define IMO_FREQ                                (8U * MHZ)
#define CLK_PATH_0                              (0U)
#define CLK_PATH_2                              (2U)

#define ECO_FREQ                                (17203200U)
#define ECO_CLOAD                               (29U)
#define ECO_ESR                                 (40U)
#define ECO_DRIVE_LEVEL                         (500U)

#define FLL_INPUT                               (ECO_FREQ)
#define FLL_OUTPUT                              (CLK_HF0)

#define CPU_SPEED                               (CLK_FAST / MHZ)

#define PIN_ERROR_PORT                          (GPIO_PRT0)
#define PIN_ERROR_PIN                           (3U)

#define PIN_STATUS_PORT                         (GPIO_PRT11)
#define PIN_STATUS_PIN                          (1U)



/******************************************************************************
 * PWM defines
 * ***************************************************************************/
#define PWM_BLOCK                               (TCPWM0)
#define PWM_NUM                                 (0U)
#define PWM_MASK                                (1UL << PWM_NUM)

#define PWM_PIN_PORT                            (GPIO_PRT10)
#define PWM_PIN_PIN                             (4U)

#define PWM_TCPWM_CLOCK                         (PCLK_TCPWM0_CLOCKS0)
#define PWM_PERI_DIV_TYPE                       (CY_SYSCLK_DIV_16_BIT)
#define PWM_PERI_DIV_NUM                        (0U)
#define PWM_PERI_DIV_VAL                        (0U)


/******************************************************************************
 * UART defines
 * ***************************************************************************/
#define UART                                    (SCB5)
#define UART_SCB_CLOCK                          (PCLK_SCB5_CLOCK)

/* UART SPEED */
#define UART_BAUDRATE                           (115200U)
#define UART_OVERSAMPLE                         (8U)
#define UART_PERI_DIV                           (108U) /* (CLK_PERI / UART_BAUDRATE / UART_OVERSAMPLE) - 1 */

#define UART_PERI_DIV_TYPE                      (CY_SYSCLK_DIV_16_BIT)
#define UART_PERI_DIV_NUM                       (1U)
#define UART_PERI_DIV_VAL                       (UART_PERI_DIV)

#define UART_RX_PIN_PORT                        (GPIO_PRT5)
#define UART_RX_PIN_PIN                         (0U)
#define UART_TX_PIN_PORT                        (GPIO_PRT5)
#define UART_TX_PIN_PIN                         (1U)

#define UART_INTR_NUM                           ((IRQn_Type) scb_5_interrupt_IRQn)
#define UART_INTR_PRIORITY                      (7U)

#define UART_BUFFER_SIZE                        (128U)

#define UART_CLEAR_SCREEN                       ("\x1b[2J\x1b[;H")


/******************************************************************************
 * Variables
 * ***************************************************************************/
/* sysClk API status */
static cy_en_sysclk_status_t sysClkStatus;

/* Context for UART operation */
static cy_stc_scb_uart_context_t uartContext;

/* UART buffer */
static uint8_t uartBuffer[UART_BUFFER_SIZE];

/* UART interrupt structure */
cy_stc_sysint_t uartIntrConfig =
{
    .intrSrc      = UART_INTR_NUM,
    .intrPriority = UART_INTR_PRIORITY,
};


/******************************************************************************
 * Pins config strutures
 * ***************************************************************************/
/* Config structure for LED pin */
static const cy_stc_gpio_pin_config_t errorPin =
{
    .outVal = 1UL,
    .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
    .hsiom = P0_3_GPIO,
    .intEdge = CY_GPIO_INTR_DISABLE,
    .intMask = 0UL,
    .vtrip = CY_GPIO_VTRIP_CMOS,
    .slewRate = CY_GPIO_SLEW_FAST,
    .driveSel = CY_GPIO_DRIVE_FULL,
    .vregEn = 0UL,
    .ibufMode = 0UL,
    .vtripSel = 0UL,
    .vrefSel = 0UL,
    .vohSel = 0UL
};

/* Config structure for PWM pin */
static const cy_stc_gpio_pin_config_t pwmPin =
{
    .outVal = 1UL,
    .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
    .hsiom = P10_4_TCPWM0_LINE0,
    .intEdge = CY_GPIO_INTR_DISABLE,
    .intMask = 0UL,
    .vtrip = CY_GPIO_VTRIP_CMOS,
    .slewRate = CY_GPIO_SLEW_FAST,
    .driveSel = CY_GPIO_DRIVE_FULL,
    .vregEn = 0UL,
    .ibufMode = 0UL,
    .vtripSel = 0UL,
    .vrefSel = 0UL,
    .vohSel = 0UL
};

/* Config structure for UART RX pin */
static const cy_stc_gpio_pin_config_t uartRxPin =
{
    .outVal = 1UL,
    .driveMode = CY_GPIO_DM_HIGHZ,
    .hsiom = P5_0_SCB5_UART_RX,
    .intEdge = CY_GPIO_INTR_DISABLE,
    .intMask = 0UL,
    .vtrip = CY_GPIO_VTRIP_CMOS,
    .slewRate = CY_GPIO_SLEW_FAST,
    .driveSel = CY_GPIO_DRIVE_FULL,
    .vregEn = 0UL,
    .ibufMode = 0UL,
    .vtripSel = 0UL,
    .vrefSel = 0UL,
    .vohSel = 0UL
};

/* Config structure for UART TX pin */
static const cy_stc_gpio_pin_config_t uartTxPin =
{
    .outVal = 1UL,
    .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
    .hsiom = P5_1_SCB5_UART_TX,
    .intEdge = CY_GPIO_INTR_DISABLE,
    .intMask = 0UL,
    .vtrip = CY_GPIO_VTRIP_CMOS,
    .slewRate = CY_GPIO_SLEW_FAST,
    .driveSel = CY_GPIO_DRIVE_FULL,
    .vregEn = 0UL,
    .ibufMode = 0UL,
    .vtripSel = 0UL,
    .vrefSel = 0UL,
    .vohSel = 0UL
};

/******************************************************************************
 * Peripheral config structure
 * ***************************************************************************/
/* Config structure for PWM */
static const cy_stc_tcpwm_pwm_config_t pwmConfig =
{
    .pwmMode           = CY_TCPWM_PWM_MODE_PWM,
    .clockPrescaler    = CY_TCPWM_PWM_PRESCALER_DIVBY_1,
    .pwmAlignment      = CY_TCPWM_PWM_RIGHT_ALIGN,
    .deadTimeClocks    = 0U,
    .runMode           = CY_TCPWM_PWM_CONTINUOUS,
    .period0           = 999U,
    .period1           = 199U,
    .enablePeriodSwap  = true,
    .compare0          = 499U,
    .compare1          = 66U,
    .enableCompareSwap = false,
    .interruptSources  = CY_TCPWM_INT_NONE,
    .invertPWMOut      = 0U,
    .invertPWMOutN     = 0U,
    .killMode          = CY_TCPWM_PWM_STOP_ON_KILL,
    .swapInputMode     = CY_TCPWM_INPUT_RISINGEDGE,
    .swapInput         = CY_TCPWM_INPUT_0,
    .reloadInputMode   = CY_TCPWM_INPUT_RISINGEDGE,
    .reloadInput       = CY_TCPWM_INPUT_0, 
    .startInputMode    = CY_TCPWM_INPUT_RISINGEDGE,
    .startInput        = CY_TCPWM_INPUT_0, 
    .killInputMode     = CY_TCPWM_INPUT_RISINGEDGE,
    .killInput         = CY_TCPWM_INPUT_0,
    .countInputMode    = CY_TCPWM_INPUT_LEVEL,
    .countInput        = CY_TCPWM_INPUT_1,
};

/* Config structure for UART */
static const cy_stc_scb_uart_config_t uartConfig =
{
    .uartMode                   = CY_SCB_UART_STANDARD,
    .enableMutliProcessorMode   = false,
    .smartCardRetryOnNack       = false,
    .irdaInvertRx               = false,
    .irdaEnableLowPowerReceiver = false,
    .oversample                 = UART_OVERSAMPLE,
    .enableMsbFirst             = false,
    .dataWidth                  = 8U,
    .parity                     = CY_SCB_UART_PARITY_NONE,
    .stopBits                   = CY_SCB_UART_STOP_BITS_1,
    .enableInputFilter          = false,
    .breakWidth                 = 11U,
    .dropOnFrameError           = false,
    .dropOnParityError          = false,
    .receiverAddress            = 0U,
    .receiverAddressMask        = 0U,
    .acceptAddrInFifo           = false,
    .enableCts                  = false,
    .ctsPolarity                = CY_SCB_UART_ACTIVE_LOW,
    .rtsRxFifoLevel             = 0UL,
    .rtsPolarity                = CY_SCB_UART_ACTIVE_LOW,
    .rxFifoTriggerLevel  = 0UL,
    .rxFifoIntEnableMask = 0UL,
    .txFifoTriggerLevel  = 0UL,
    .txFifoIntEnableMask = 0UL,
};


/******************************************************************************
 * Interrupt handler
 * ***************************************************************************/
/* Interrupt handler for UART */
void UART_Isr(void)
{
    uint32_t num = 0U;
    num = Cy_SCB_UART_GetNumInRxFifo(UART);
    Cy_SCB_UART_GetArray(UART, uartBuffer, num);
    Cy_SCB_UART_PutArray(UART, uartBuffer, num);

    Cy_SCB_ClearRxInterrupt(UART, CY_SCB_RX_INTR_NOT_EMPTY);
}


/******************************************************************************
 * Name: main
 * ***************************************************************************/
int main(void)
{
    __enable_irq();

    /* Init error and pwm pin */
    Cy_GPIO_Pin_Init(PIN_ERROR_PORT, PIN_ERROR_PIN, &errorPin);

    pwmStart();
    systemStart();
    uartStart();
    printSystemSettings();

    uartStartLoopCycle();

    for (;;)
    {
    }
}


/******************************************************************************
 * Name: errorOccur
 * ****************************************************************************
 * Description: Called in case if something go wrong.
 * ***************************************************************************/
static void errorOccur(void)
{
    SystemCoreClockUpdate();
    __disable_irq();

    for(;;)
    {
        Cy_SysLib_Delay(200U);
        Cy_GPIO_Inv(PIN_ERROR_PORT, PIN_ERROR_PIN);
    }
}


/******************************************************************************
 * Name: systemStart
 * ****************************************************************************
 * Description: Config CLK_FAST and CLK_PERI.
 * Default freq after reset = 8 MHz
 * ***************************************************************************/
void systemStart(void)
{
    sysClkStatus = CY_SYSCLK_SUCCESS;
    Cy_SysLib_SetWaitStates(false, CPU_SPEED);

    /* Change clock source for hf[0] from CLK_PATH0 to CLK_PATH2 */
    sysClkStatus = Cy_SysClk_ClkPathSetSource(CLK_PATH_2, CY_SYSCLK_CLKPATH_IN_IMO);
    if(sysClkStatus != CY_SYSCLK_SUCCESS)
    {
        errorOccur();
    }

    sysClkStatus = Cy_SysClk_ClkHfSetSource(CLK_HF_NUM, CY_SYSCLK_CLKHF_IN_CLKPATH2);
    if(sysClkStatus != CY_SYSCLK_SUCCESS)
    {
        errorOccur();
    }

    /* Config ECO */
    clock_EcoConfig();

    /* Change clock source for PATH_MUX0 from IMO to ECO */
    sysClkStatus = Cy_SysClk_ClkPathSetSource(CLK_PATH_0, CY_SYSCLK_CLKPATH_IN_ECO);
    if(sysClkStatus != CY_SYSCLK_SUCCESS)
    {
        errorOccur();
    }

    /* Config FLL */
    clock_FllConfig();

    sysClkStatus = Cy_SysClk_ClkHfSetSource(CLK_HF_NUM, CY_SYSCLK_CLKHF_IN_CLKPATH0);
    if(sysClkStatus != CY_SYSCLK_SUCCESS)
    {
        errorOccur();
    }

    sysClkStatus = Cy_SysClk_ClkHfSetDivider(CLK_HF_NUM, CY_SYSCLK_CLKHF_NO_DIVIDE);
    if(sysClkStatus != CY_SYSCLK_SUCCESS)
    {
        errorOccur();
    }

    Cy_SysClk_ClkFastSetDivider(CLK_FAST_DIV);
    Cy_SysClk_ClkPeriSetDivider(CLK_PERI_DIV);

    SystemCoreClockUpdate();
}


/******************************************************************************
 * Name: clock_EcoConfig
 * ****************************************************************************
 * Description: Config ECO.
 * ***************************************************************************/
static void clock_EcoConfig(void)
{
    Cy_SysClk_EcoDisable();

    sysClkStatus = Cy_SysClk_EcoConfigure(ECO_FREQ, ECO_CLOAD, ECO_ESR, ECO_DRIVE_LEVEL);
    if(sysClkStatus != CY_SYSCLK_SUCCESS)
    {
        errorOccur();
    }

    /* Set timeout to 0. Check if ECO is enabled by Cy_SysClk_EcoGetStatus */
    sysClkStatus = Cy_SysClk_EcoEnable(0U);
    if(sysClkStatus != CY_SYSCLK_SUCCESS)
    {
        errorOccur();
    }

    Cy_GPIO_Clr(PIN_ERROR_PORT, PIN_ERROR_PIN);
    while(CY_SYSCLK_ECOSTAT_STABLE != Cy_SysClk_EcoGetStatus())
    {

    }
    Cy_GPIO_Set(PIN_ERROR_PORT, PIN_ERROR_PIN);

}


/******************************************************************************
 * Name: clock_FllConfig
 * ****************************************************************************
 * Description: Config FLL.
 * ***************************************************************************/
static void clock_FllConfig(void)
{
    /* Disable FLL before config */
    if(Cy_SysClk_FllIsEnabled())
    {
        sysClkStatus = Cy_SysClk_FllDisable();
        if(sysClkStatus != CY_SYSCLK_SUCCESS)
        {
            errorOccur();
        }
    }

    sysClkStatus = Cy_SysClk_FllConfigure(FLL_INPUT, FLL_OUTPUT, CY_SYSCLK_FLLPLL_OUTPUT_OUTPUT);
    if(sysClkStatus != CY_SYSCLK_SUCCESS)
    {
        errorOccur();
    }

    /* Set timeout to 0. Check if FLL is locked by Cy_SysClk_FllLocked */
    sysClkStatus = Cy_SysClk_FllEnable(0U);
    if(sysClkStatus != CY_SYSCLK_SUCCESS)
    {
        errorOccur();
    }

    Cy_GPIO_Clr(PIN_ERROR_PORT, PIN_ERROR_PIN);
    while(!Cy_SysClk_FllLocked())
    {

    }
    Cy_GPIO_Set(PIN_ERROR_PORT, PIN_ERROR_PIN);
}


/******************************************************************************
 * Name: pwmStart
 * ****************************************************************************
 * Description: Config CLK_FAST and CLK_PERI.
 * Default freq = 8 MHz
 * ***************************************************************************/
static void pwmStart(void)
{
    if (CY_TCPWM_SUCCESS != Cy_TCPWM_PWM_Init(PWM_BLOCK, PWM_NUM, &pwmConfig))
    {
        errorOccur();
    }

    /* Config peri divider for PWM */
    Cy_SysClk_PeriphDisableDivider(PWM_PERI_DIV_TYPE, PWM_PERI_DIV_NUM);
    Cy_SysClk_PeriphAssignDivider(PWM_TCPWM_CLOCK, PWM_PERI_DIV_TYPE, PWM_PERI_DIV_NUM);
    Cy_SysClk_PeriphSetDivider(PWM_PERI_DIV_TYPE,PWM_PERI_DIV_NUM, PWM_PERI_DIV_VAL);
    Cy_SysClk_PeriphEnableDivider(PWM_PERI_DIV_TYPE, PWM_PERI_DIV_NUM);

    Cy_GPIO_Pin_Init(PWM_PIN_PORT, PWM_PIN_PIN, &pwmPin);
    
    /* Enable the initialized PWM */
    Cy_TCPWM_PWM_Enable(PWM_BLOCK, PWM_NUM);
    
    /* Then start the PWM */
    Cy_TCPWM_TriggerReloadOrIndex(PWM_BLOCK, PWM_MASK);
}


/******************************************************************************
 * Name: uartStart
 * ****************************************************************************
 * Description: Config peri divider and pins for UART. Init and enable UART.
 * ***************************************************************************/
static void uartStart(void)
{
    if (CY_SCB_UART_SUCCESS != Cy_SCB_UART_Init(UART, &uartConfig, &uartContext))
    {
        errorOccur();
    }

    /* Config peri divider for UART */
    Cy_SysClk_PeriphDisableDivider(UART_PERI_DIV_TYPE, UART_PERI_DIV_NUM);
    Cy_SysClk_PeriphAssignDivider(UART_SCB_CLOCK, UART_PERI_DIV_TYPE, UART_PERI_DIV_NUM);
    Cy_SysClk_PeriphSetDivider(UART_PERI_DIV_TYPE, UART_PERI_DIV_NUM, UART_PERI_DIV_VAL);
    Cy_SysClk_PeriphEnableDivider(UART_PERI_DIV_TYPE, UART_PERI_DIV_NUM);

    Cy_GPIO_Pin_Init(UART_RX_PIN_PORT, UART_RX_PIN_PIN, &uartRxPin);
    Cy_GPIO_Pin_Init(UART_TX_PIN_PORT, UART_TX_PIN_PIN, &uartTxPin);

    Cy_SCB_UART_Enable(UART);
}


/******************************************************************************
 * Name: printSystemSettings
 * ****************************************************************************
 * Description: Print base system settings
 * ***************************************************************************/
static void printSystemSettings(void)
{
    char str[100U];

    Cy_SCB_UART_PutString(UART, UART_CLEAR_SCREEN);

    Cy_SCB_UART_PutString(UART, "\n\r<--------------------------------------------------------------------------->\n\r");

    sprintf(str, "Fast clock freq = %d Hz\n\r", CLK_FAST);
    Cy_SCB_UART_PutString(UART, str);

    sprintf(str, "Peri clock freq = %d Hz\n\r", CLK_PERI);
    Cy_SCB_UART_PutString(UART, str);

    Cy_SCB_UART_PutString(UART, "Set PWM to 8.0 Pin to measure Peri clock freq. PWM period - 999; PWM compare - 499\n\r");

    Cy_SCB_UART_PutString(UART, "<--------------------------------------------------------------------------->\n\r");
}


/******************************************************************************
 * Name: uartStartLoopCycle
 * ***************************************************************************/
static void uartStartLoopCycle(void)
{
    (void) Cy_SysInt_Init(&uartIntrConfig, &UART_Isr);
    NVIC_EnableIRQ(UART_INTR_NUM);

    /* Set interrupt mask */
    Cy_SCB_SetRxInterruptMask(UART, CY_SCB_RX_INTR_NOT_EMPTY);

    Cy_SCB_UART_PutString(UART, "Ppaz lab 3\n\r");
    Cy_SCB_UART_PutString(UART, "UART Interrupt config complete\n\r");
    Cy_SCB_UART_PutString(UART, "Do not send more than 128 bytes data in one transmit\n\r");
    Cy_SCB_UART_PutString(UART, "Wait until terminal show data, then send new data\n\r");
    Cy_SCB_UART_PutString(UART, "<--------------------------------------------------------------------------->\n\r");
}
/* [] END OF FILE */