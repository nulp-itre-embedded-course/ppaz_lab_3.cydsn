/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    UART_1_Start();
    uint32 i = 32;
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */

    for(;;)
    {
        UART_1_PutCRLF(i);
        if(i == 127)
        {
            i = 32;
        }
        i++;
        CyDelay(5000);
        /* Place your application code here. */
    }
}

/* [] END OF FILE */
