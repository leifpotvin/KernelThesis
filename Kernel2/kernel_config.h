/*
 * kernel_config.h
 *
 * Created: 4/30/2020 2:32:27 PM
 *  Author: Nathan Potvin
 */ 


#ifndef KERNEL_CONFIG_H_
#define KERNEL_CONFIG_H_

/****************************************************************************
*	Define scheduler parameters
****************************************************************************/

#define PREEMPTIVE
#define TIME_SLICE 0x4000

/****************************************************************************
*	Define stack parameters
****************************************************************************/

#define DEFAULT_STACK_SZ 0x80

#define T0_STACKSZ DEFAULT_STACK_SZ
#define T1_STACKSZ DEFAULT_STACK_SZ
#define T2_STACKSZ DEFAULT_STACK_SZ
#define T3_STACKSZ DEFAULT_STACK_SZ
#define T4_STACKSZ DEFAULT_STACK_SZ
#define T5_STACKSZ DEFAULT_STACK_SZ
#define T6_STACKSZ DEFAULT_STACK_SZ
#define T7_STACKSZ DEFAULT_STACK_SZ

#endif /* KERNEL_CONFIG_H_ */