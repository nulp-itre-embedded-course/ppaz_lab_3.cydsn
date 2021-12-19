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
#define NAME "Yaroslav "
#define SURNAME "Mentukh "
#define GROUP "TR-44 "


int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    
    UART_Start();
    LCD_Start();
    LCD_Position(0u,0u);
    LCD_PrintString(NAME);
    LCD_Position(1u,0u);
    LCD_PrintString(SURNAME);
    LCD_Position(1u,9u);
    LCD_PrintString(GROUP);
    
    for(;;)
    {
       UART_PutString(NAME);
       UART_PutString(SURNAME);
       UART_PutString(GROUP);
       CyDelay(1000);
    }
}

/* [] END OF FILE */
