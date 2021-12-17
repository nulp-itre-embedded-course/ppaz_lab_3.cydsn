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

int16 delay =0;

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    Timer_Start();
    LCD_Init();

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    LCD_Position(0,0);
    LCD_PrintString("Distance:");
    CyDelay(1000);

    for(;;)
    {
        //Send Pulse
        Control_Write(0);
        CyDelayUs(2u);
        Control_Write(1);
        CyDelayUs(10u);
        Control_Write(0);
        CyDelayUs(500);
        
        //Wait for response
        while (ECHO_Read()==1){}
        
        //Calculate distance
        delay=(65535 - Timer_ReadCounter())/58;
        
        //Display measurment results
        LCD_Position(1,0);
        LCD_PrintString("            ");
        LCD_Position(1,0);
        LCD_PrintNumber(delay);
        
        //Wait before next measurment
        CyDelay(500);
    }
}

/* [] END OF FILE */
