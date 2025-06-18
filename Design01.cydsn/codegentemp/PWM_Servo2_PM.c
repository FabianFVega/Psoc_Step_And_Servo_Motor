/*******************************************************************************
* File Name: PWM_Servo2_PM.c
* Version 3.30
*
* Description:
*  This file provides the power management source code to API for the
*  PWM.
*
* Note:
*
********************************************************************************
* Copyright 2008-2014, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "PWM_Servo2.h"

static PWM_Servo2_backupStruct PWM_Servo2_backup;


/*******************************************************************************
* Function Name: PWM_Servo2_SaveConfig
********************************************************************************
*
* Summary:
*  Saves the current user configuration of the component.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  PWM_Servo2_backup:  Variables of this global structure are modified to
*  store the values of non retention configuration registers when Sleep() API is
*  called.
*
*******************************************************************************/
void PWM_Servo2_SaveConfig(void) 
{

    #if(!PWM_Servo2_UsingFixedFunction)
        #if(!PWM_Servo2_PWMModeIsCenterAligned)
            PWM_Servo2_backup.PWMPeriod = PWM_Servo2_ReadPeriod();
        #endif /* (!PWM_Servo2_PWMModeIsCenterAligned) */
        PWM_Servo2_backup.PWMUdb = PWM_Servo2_ReadCounter();
        #if (PWM_Servo2_UseStatus)
            PWM_Servo2_backup.InterruptMaskValue = PWM_Servo2_STATUS_MASK;
        #endif /* (PWM_Servo2_UseStatus) */

        #if(PWM_Servo2_DeadBandMode == PWM_Servo2__B_PWM__DBM_256_CLOCKS || \
            PWM_Servo2_DeadBandMode == PWM_Servo2__B_PWM__DBM_2_4_CLOCKS)
            PWM_Servo2_backup.PWMdeadBandValue = PWM_Servo2_ReadDeadTime();
        #endif /*  deadband count is either 2-4 clocks or 256 clocks */

        #if(PWM_Servo2_KillModeMinTime)
             PWM_Servo2_backup.PWMKillCounterPeriod = PWM_Servo2_ReadKillTime();
        #endif /* (PWM_Servo2_KillModeMinTime) */

        #if(PWM_Servo2_UseControl)
            PWM_Servo2_backup.PWMControlRegister = PWM_Servo2_ReadControlRegister();
        #endif /* (PWM_Servo2_UseControl) */
    #endif  /* (!PWM_Servo2_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: PWM_Servo2_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores the current user configuration of the component.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  PWM_Servo2_backup:  Variables of this global structure are used to
*  restore the values of non retention registers on wakeup from sleep mode.
*
*******************************************************************************/
void PWM_Servo2_RestoreConfig(void) 
{
        #if(!PWM_Servo2_UsingFixedFunction)
            #if(!PWM_Servo2_PWMModeIsCenterAligned)
                PWM_Servo2_WritePeriod(PWM_Servo2_backup.PWMPeriod);
            #endif /* (!PWM_Servo2_PWMModeIsCenterAligned) */

            PWM_Servo2_WriteCounter(PWM_Servo2_backup.PWMUdb);

            #if (PWM_Servo2_UseStatus)
                PWM_Servo2_STATUS_MASK = PWM_Servo2_backup.InterruptMaskValue;
            #endif /* (PWM_Servo2_UseStatus) */

            #if(PWM_Servo2_DeadBandMode == PWM_Servo2__B_PWM__DBM_256_CLOCKS || \
                PWM_Servo2_DeadBandMode == PWM_Servo2__B_PWM__DBM_2_4_CLOCKS)
                PWM_Servo2_WriteDeadTime(PWM_Servo2_backup.PWMdeadBandValue);
            #endif /* deadband count is either 2-4 clocks or 256 clocks */

            #if(PWM_Servo2_KillModeMinTime)
                PWM_Servo2_WriteKillTime(PWM_Servo2_backup.PWMKillCounterPeriod);
            #endif /* (PWM_Servo2_KillModeMinTime) */

            #if(PWM_Servo2_UseControl)
                PWM_Servo2_WriteControlRegister(PWM_Servo2_backup.PWMControlRegister);
            #endif /* (PWM_Servo2_UseControl) */
        #endif  /* (!PWM_Servo2_UsingFixedFunction) */
    }


/*******************************************************************************
* Function Name: PWM_Servo2_Sleep
********************************************************************************
*
* Summary:
*  Disables block's operation and saves the user configuration. Should be called
*  just prior to entering sleep.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  PWM_Servo2_backup.PWMEnableState:  Is modified depending on the enable
*  state of the block before entering sleep mode.
*
*******************************************************************************/
void PWM_Servo2_Sleep(void) 
{
    #if(PWM_Servo2_UseControl)
        if(PWM_Servo2_CTRL_ENABLE == (PWM_Servo2_CONTROL & PWM_Servo2_CTRL_ENABLE))
        {
            /*Component is enabled */
            PWM_Servo2_backup.PWMEnableState = 1u;
        }
        else
        {
            /* Component is disabled */
            PWM_Servo2_backup.PWMEnableState = 0u;
        }
    #endif /* (PWM_Servo2_UseControl) */

    /* Stop component */
    PWM_Servo2_Stop();

    /* Save registers configuration */
    PWM_Servo2_SaveConfig();
}


/*******************************************************************************
* Function Name: PWM_Servo2_Wakeup
********************************************************************************
*
* Summary:
*  Restores and enables the user configuration. Should be called just after
*  awaking from sleep.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  PWM_Servo2_backup.pwmEnable:  Is used to restore the enable state of
*  block on wakeup from sleep mode.
*
*******************************************************************************/
void PWM_Servo2_Wakeup(void) 
{
     /* Restore registers values */
    PWM_Servo2_RestoreConfig();

    if(PWM_Servo2_backup.PWMEnableState != 0u)
    {
        /* Enable component's operation */
        PWM_Servo2_Enable();
    } /* Do nothing if component's block was disabled before */

}


/* [] END OF FILE */
