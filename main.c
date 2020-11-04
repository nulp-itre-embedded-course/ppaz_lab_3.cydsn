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
    char buf;
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */

    for(;;)
    {
        buf = UART_1_GetChar();
        if(0u != buf)
        {
        UART_1_PutCRLF(buf);
        }
        /* Place your application code here. */
    }
}

/* [] END OF FILE */
