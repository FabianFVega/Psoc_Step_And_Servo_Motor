/*******************************************************************************
* File Name: RPWM.h  
* Version 2.20
*
* Description:
*  This file contains Pin function prototypes and register defines
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_PINS_RPWM_H) /* Pins RPWM_H */
#define CY_PINS_RPWM_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "RPWM_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 RPWM__PORT == 15 && ((RPWM__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    RPWM_Write(uint8 value);
void    RPWM_SetDriveMode(uint8 mode);
uint8   RPWM_ReadDataReg(void);
uint8   RPWM_Read(void);
void    RPWM_SetInterruptMode(uint16 position, uint16 mode);
uint8   RPWM_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the RPWM_SetDriveMode() function.
     *  @{
     */
        #define RPWM_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define RPWM_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define RPWM_DM_RES_UP          PIN_DM_RES_UP
        #define RPWM_DM_RES_DWN         PIN_DM_RES_DWN
        #define RPWM_DM_OD_LO           PIN_DM_OD_LO
        #define RPWM_DM_OD_HI           PIN_DM_OD_HI
        #define RPWM_DM_STRONG          PIN_DM_STRONG
        #define RPWM_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define RPWM_MASK               RPWM__MASK
#define RPWM_SHIFT              RPWM__SHIFT
#define RPWM_WIDTH              1u

/* Interrupt constants */
#if defined(RPWM__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in RPWM_SetInterruptMode() function.
     *  @{
     */
        #define RPWM_INTR_NONE      (uint16)(0x0000u)
        #define RPWM_INTR_RISING    (uint16)(0x0001u)
        #define RPWM_INTR_FALLING   (uint16)(0x0002u)
        #define RPWM_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define RPWM_INTR_MASK      (0x01u) 
#endif /* (RPWM__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define RPWM_PS                     (* (reg8 *) RPWM__PS)
/* Data Register */
#define RPWM_DR                     (* (reg8 *) RPWM__DR)
/* Port Number */
#define RPWM_PRT_NUM                (* (reg8 *) RPWM__PRT) 
/* Connect to Analog Globals */                                                  
#define RPWM_AG                     (* (reg8 *) RPWM__AG)                       
/* Analog MUX bux enable */
#define RPWM_AMUX                   (* (reg8 *) RPWM__AMUX) 
/* Bidirectional Enable */                                                        
#define RPWM_BIE                    (* (reg8 *) RPWM__BIE)
/* Bit-mask for Aliased Register Access */
#define RPWM_BIT_MASK               (* (reg8 *) RPWM__BIT_MASK)
/* Bypass Enable */
#define RPWM_BYP                    (* (reg8 *) RPWM__BYP)
/* Port wide control signals */                                                   
#define RPWM_CTL                    (* (reg8 *) RPWM__CTL)
/* Drive Modes */
#define RPWM_DM0                    (* (reg8 *) RPWM__DM0) 
#define RPWM_DM1                    (* (reg8 *) RPWM__DM1)
#define RPWM_DM2                    (* (reg8 *) RPWM__DM2) 
/* Input Buffer Disable Override */
#define RPWM_INP_DIS                (* (reg8 *) RPWM__INP_DIS)
/* LCD Common or Segment Drive */
#define RPWM_LCD_COM_SEG            (* (reg8 *) RPWM__LCD_COM_SEG)
/* Enable Segment LCD */
#define RPWM_LCD_EN                 (* (reg8 *) RPWM__LCD_EN)
/* Slew Rate Control */
#define RPWM_SLW                    (* (reg8 *) RPWM__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define RPWM_PRTDSI__CAPS_SEL       (* (reg8 *) RPWM__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define RPWM_PRTDSI__DBL_SYNC_IN    (* (reg8 *) RPWM__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define RPWM_PRTDSI__OE_SEL0        (* (reg8 *) RPWM__PRTDSI__OE_SEL0) 
#define RPWM_PRTDSI__OE_SEL1        (* (reg8 *) RPWM__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define RPWM_PRTDSI__OUT_SEL0       (* (reg8 *) RPWM__PRTDSI__OUT_SEL0) 
#define RPWM_PRTDSI__OUT_SEL1       (* (reg8 *) RPWM__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define RPWM_PRTDSI__SYNC_OUT       (* (reg8 *) RPWM__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(RPWM__SIO_CFG)
    #define RPWM_SIO_HYST_EN        (* (reg8 *) RPWM__SIO_HYST_EN)
    #define RPWM_SIO_REG_HIFREQ     (* (reg8 *) RPWM__SIO_REG_HIFREQ)
    #define RPWM_SIO_CFG            (* (reg8 *) RPWM__SIO_CFG)
    #define RPWM_SIO_DIFF           (* (reg8 *) RPWM__SIO_DIFF)
#endif /* (RPWM__SIO_CFG) */

/* Interrupt Registers */
#if defined(RPWM__INTSTAT)
    #define RPWM_INTSTAT            (* (reg8 *) RPWM__INTSTAT)
    #define RPWM_SNAP               (* (reg8 *) RPWM__SNAP)
    
	#define RPWM_0_INTTYPE_REG 		(* (reg8 *) RPWM__0__INTTYPE)
#endif /* (RPWM__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_RPWM_H */


/* [] END OF FILE */
