/**
 * stm32f4xx.h — Minimal CMSIS-style peripheral definitions for STM32F407
 *
 * Register addresses derived from RM0090 Reference Manual.
 * Only peripherals used in this project are defined here.
 */

#ifndef STM32F4XX_H
#define STM32F4XX_H

#include <stdint.h>

/* ── Core ─────────────────────────────────────────────────────────────── */
#define __IO volatile

/* ── Memory map base addresses (RM0090 §2.3) ─────────────────────────── */
#define PERIPH_BASE         0x40000000UL
#define APB1PERIPH_BASE     (PERIPH_BASE + 0x00000000UL)
#define APB2PERIPH_BASE     (PERIPH_BASE + 0x00010000UL)
#define AHB1PERIPH_BASE     (PERIPH_BASE + 0x00020000UL)

/* ── RCC (Reset and Clock Control) — AHB1 bus, RM0090 §6 ─────────────── */
#define RCC_BASE            (AHB1PERIPH_BASE + 0x3800UL)

typedef struct {
    __IO uint32_t CR;         /* 0x00  Clock control */
    __IO uint32_t PLLCFGR;    /* 0x04  PLL config */
    __IO uint32_t CFGR;       /* 0x08  Clock config */
    __IO uint32_t CIR;        /* 0x0C  Clock interrupt */
    __IO uint32_t AHB1RSTR;   /* 0x10  AHB1 peripheral reset */
    __IO uint32_t AHB2RSTR;   /* 0x14 */
    __IO uint32_t AHB3RSTR;   /* 0x18 */
         uint32_t RESERVED0;
    __IO uint32_t APB1RSTR;   /* 0x20  APB1 peripheral reset */
    __IO uint32_t APB2RSTR;   /* 0x24  APB2 peripheral reset */
         uint32_t RESERVED1[2];
    __IO uint32_t AHB1ENR;    /* 0x30  AHB1 peripheral clock enable */
    __IO uint32_t AHB2ENR;    /* 0x34 */
    __IO uint32_t AHB3ENR;    /* 0x38 */
         uint32_t RESERVED2;
    __IO uint32_t APB1ENR;    /* 0x40  APB1 peripheral clock enable */
    __IO uint32_t APB2ENR;    /* 0x44  APB2 peripheral clock enable */
         uint32_t RESERVED3[2];
    __IO uint32_t AHB1LPENR;  /* 0x50 */
    __IO uint32_t AHB2LPENR;  /* 0x54 */
    __IO uint32_t AHB3LPENR;  /* 0x58 */
         uint32_t RESERVED4;
    __IO uint32_t APB1LPENR;  /* 0x60 */
    __IO uint32_t APB2LPENR;  /* 0x64 */
         uint32_t RESERVED5[2];
    __IO uint32_t BDCR;       /* 0x70 */
    __IO uint32_t CSR;        /* 0x74 */
         uint32_t RESERVED6[2];
    __IO uint32_t SSCGR;      /* 0x80 */
    __IO uint32_t PLLI2SCFGR; /* 0x84 */
} RCC_TypeDef;

#define RCC     ((RCC_TypeDef *) RCC_BASE)

/* RCC_AHB1ENR bit positions */
#define RCC_AHB1ENR_GPIOAEN     (1UL << 0)
#define RCC_AHB1ENR_GPIOBEN     (1UL << 1)
#define RCC_AHB1ENR_GPIOCEN     (1UL << 2)

/* RCC_APB1ENR bit positions */
#define RCC_APB1ENR_USART2EN    (1UL << 17)

/* RCC_APB2ENR bit positions */
#define RCC_APB2ENR_TIM1EN      (1UL << 0)
#define RCC_APB2ENR_SYSCFGEN    (1UL << 14)

/* ── GPIO — AHB1 bus, RM0090 §8 ──────────────────────────────────────── */
#define GPIOA_BASE  (AHB1PERIPH_BASE + 0x0000UL)
#define GPIOB_BASE  (AHB1PERIPH_BASE + 0x0400UL)
#define GPIOC_BASE  (AHB1PERIPH_BASE + 0x0800UL)

typedef struct {
    __IO uint32_t MODER;    /* 0x00  Mode: 00=in, 01=out, 10=AF, 11=analog */
    __IO uint32_t OTYPER;   /* 0x04  Output type: 0=push-pull, 1=open-drain */
    __IO uint32_t OSPEEDR;  /* 0x08  Output speed */
    __IO uint32_t PUPDR;    /* 0x0C  Pull-up/down: 00=none, 01=PU, 10=PD */
    __IO uint32_t IDR;      /* 0x10  Input data register */
    __IO uint32_t ODR;      /* 0x14  Output data register */
    __IO uint32_t BSRR;     /* 0x18  Bit set/reset (atomic) */
    __IO uint32_t LCKR;     /* 0x1C  Lock */
    __IO uint32_t AFR[2];   /* 0x20  Alternate function low[0], high[1] */
} GPIO_TypeDef;

#define GPIOA   ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB   ((GPIO_TypeDef *) GPIOB_BASE)
#define GPIOC   ((GPIO_TypeDef *) GPIOC_BASE)

/* GPIO MODER values (2 bits per pin) */
#define GPIO_MODER_INPUT    0x0U
#define GPIO_MODER_OUTPUT   0x1U
#define GPIO_MODER_AF       0x2U
#define GPIO_MODER_ANALOG   0x3U

/* GPIO OSPEEDR values */
#define GPIO_OSPEEDR_LOW    0x0U
#define GPIO_OSPEEDR_MED    0x1U
#define GPIO_OSPEEDR_HIGH   0x2U
#define GPIO_OSPEEDR_VHIGH  0x3U

/* ── TIM1 — APB2 bus, RM0090 §17 (advanced-control timer) ───────────── */
#define TIM1_BASE   (APB2PERIPH_BASE + 0x0000UL)

typedef struct {
    __IO uint32_t CR1;    /* 0x00  Control register 1 */
    __IO uint32_t CR2;    /* 0x04  Control register 2 */
    __IO uint32_t SMCR;   /* 0x08  Slave mode control */
    __IO uint32_t DIER;   /* 0x0C  DMA/interrupt enable */
    __IO uint32_t SR;     /* 0x10  Status register */
    __IO uint32_t EGR;    /* 0x14  Event generation */
    __IO uint32_t CCMR1;  /* 0x18  Capture/compare mode 1 (CH1, CH2) */
    __IO uint32_t CCMR2;  /* 0x1C  Capture/compare mode 2 (CH3, CH4) */
    __IO uint32_t CCER;   /* 0x20  Capture/compare enable */
    __IO uint32_t CNT;    /* 0x24  Counter */
    __IO uint32_t PSC;    /* 0x28  Prescaler */
    __IO uint32_t ARR;    /* 0x2C  Auto-reload (period) */
    __IO uint32_t RCR;    /* 0x30  Repetition counter (advanced only) */
    __IO uint32_t CCR1;   /* 0x34  Capture/compare value CH1 */
    __IO uint32_t CCR2;   /* 0x38  Capture/compare value CH2 */
    __IO uint32_t CCR3;   /* 0x3C  Capture/compare value CH3 */
    __IO uint32_t CCR4;   /* 0x40  Capture/compare value CH4 */
    __IO uint32_t BDTR;   /* 0x44  Break and dead-time (advanced only) */
    __IO uint32_t DCR;    /* 0x48  DMA control */
    __IO uint32_t DMAR;   /* 0x4C  DMA address */
    __IO uint32_t OR;     /* 0x50  Option register */
} TIM_TypeDef;

#define TIM1    ((TIM_TypeDef *) TIM1_BASE)

/* TIM CR1 bits */
#define TIM_CR1_CEN     (1UL << 0)   /* Counter enable */
#define TIM_CR1_ARPE    (1UL << 7)   /* Auto-reload preload enable */
#define TIM_CR1_CMS_MASK (3UL << 5)
#define TIM_CR1_CMS_CENTER1 (1UL << 5) /* Center-aligned mode 1 */

/* TIM CCMR1/2 bits — output compare mode PWM1 = 0b110 */
#define TIM_CCMR_OC1M_PWM1    (0x6UL << 4)   /* CH1 PWM mode 1 */
#define TIM_CCMR_OC1PE        (1UL << 3)      /* CH1 preload enable */
#define TIM_CCMR_OC2M_PWM1    (0x6UL << 12)  /* CH2 PWM mode 1 */
#define TIM_CCMR_OC2PE        (1UL << 11)     /* CH2 preload enable */
#define TIM_CCMR_OC3M_PWM1    (0x6UL << 4)   /* CH3 PWM mode 1 (in CCMR2) */
#define TIM_CCMR_OC3PE        (1UL << 3)      /* CH3 preload enable */

/* TIM CCER bits */
#define TIM_CCER_CC1E   (1UL << 0)   /* CH1 output enable */
#define TIM_CCER_CC1NE  (1UL << 2)   /* CH1N (complementary) enable */
#define TIM_CCER_CC1P   (1UL << 1)   /* CH1 polarity */
#define TIM_CCER_CC2E   (1UL << 4)   /* CH2 output enable */
#define TIM_CCER_CC2NE  (1UL << 6)   /* CH2N enable */
#define TIM_CCER_CC3E   (1UL << 8)   /* CH3 output enable */
#define TIM_CCER_CC3NE  (1UL << 10)  /* CH3N enable */

/* TIM BDTR bits */
#define TIM_BDTR_MOE    (1UL << 15)  /* Main output enable — must be set for TIM1 */
#define TIM_BDTR_OSSR   (1UL << 11)  /* Off-state selection for Run mode */
#define TIM_BDTR_OSSI   (1UL << 10)  /* Off-state selection for Idle mode */

/* TIM EGR bits */
#define TIM_EGR_UG      (1UL << 0)   /* Update generation — forces preload shadow copy */

/* TIM DIER bits */
#define TIM_DIER_UIE    (1UL << 0)   /* Update interrupt enable */

/* TIM SR bits */
#define TIM_SR_UIF      (1UL << 0)   /* Update interrupt flag */

/* ── USART2 — APB1 bus, RM0090 §30 ───────────────────────────────────── */
#define USART2_BASE (APB1PERIPH_BASE + 0x4400UL)

typedef struct {
    __IO uint32_t SR;   /* 0x00  Status */
    __IO uint32_t DR;   /* 0x04  Data */
    __IO uint32_t BRR;  /* 0x08  Baud rate */
    __IO uint32_t CR1;  /* 0x0C  Control 1 */
    __IO uint32_t CR2;  /* 0x10  Control 2 */
    __IO uint32_t CR3;  /* 0x14  Control 3 */
    __IO uint32_t GTPR; /* 0x18  Guard time and prescaler */
} USART_TypeDef;

#define USART2  ((USART_TypeDef *) USART2_BASE)

/* USART SR bits */
#define USART_SR_TC     (1UL << 6)   /* Transmission complete */
#define USART_SR_TXE    (1UL << 7)   /* Transmit data register empty */
#define USART_SR_RXNE   (1UL << 5)   /* Read data register not empty */

/* USART CR1 bits */
#define USART_CR1_UE    (1UL << 13)  /* USART enable */
#define USART_CR1_TE    (1UL << 3)   /* Transmitter enable */
#define USART_CR1_RE    (1UL << 2)   /* Receiver enable */

/* ── SYSCFG — APB2 bus, RM0090 §9 ────────────────────────────────────── */
#define SYSCFG_BASE (APB2PERIPH_BASE + 0x3800UL)

typedef struct {
    __IO uint32_t MEMRMP;       /* 0x00 */
    __IO uint32_t PMC;          /* 0x04 */
    __IO uint32_t EXTICR[4];    /* 0x08–0x14  EXTI line source selection */
         uint32_t RESERVED[2];
    __IO uint32_t CMPCR;        /* 0x20 */
} SYSCFG_TypeDef;

#define SYSCFG  ((SYSCFG_TypeDef *) SYSCFG_BASE)

/* ── EXTI — APB2 bus, RM0090 §10 ─────────────────────────────────────── */
#define EXTI_BASE   (APB2PERIPH_BASE + 0x3C00UL)

typedef struct {
    __IO uint32_t IMR;    /* 0x00  Interrupt mask */
    __IO uint32_t EMR;    /* 0x04  Event mask */
    __IO uint32_t RTSR;   /* 0x08  Rising trigger selection */
    __IO uint32_t FTSR;   /* 0x0C  Falling trigger selection */
    __IO uint32_t SWIER;  /* 0x10  Software interrupt event */
    __IO uint32_t PR;     /* 0x14  Pending register */
} EXTI_TypeDef;

#define EXTI    ((EXTI_TypeDef *) EXTI_BASE)

/* ── NVIC — Cortex-M4 core, ARM DDI0439 §4.3 ─────────────────────────── */
#define NVIC_BASE   0xE000E100UL

typedef struct {
    __IO uint32_t ISER[8];   /* 0x000  Interrupt set-enable */
         uint32_t RESERVED0[24];
    __IO uint32_t ICER[8];   /* 0x080  Interrupt clear-enable */
         uint32_t RESERVED1[24];
    __IO uint32_t ISPR[8];   /* 0x100  Interrupt set-pending */
         uint32_t RESERVED2[24];
    __IO uint32_t ICPR[8];   /* 0x180  Interrupt clear-pending */
         uint32_t RESERVED3[24];
    __IO uint32_t IABR[8];   /* 0x200  Interrupt active bit */
         uint32_t RESERVED4[56];
    __IO uint8_t  IP[240];   /* 0x300  Interrupt priority */
         uint32_t RESERVED5[644];
    __IO uint32_t STIR;      /* 0xE00  Software trigger interrupt */
} NVIC_TypeDef;

#define NVIC    ((NVIC_TypeDef *) NVIC_BASE)

/* STM32F4 IRQ numbers (RM0090 Table 43) */
#define EXTI0_IRQn          6
#define EXTI1_IRQn          7
#define TIM1_UP_TIM10_IRQn  25
#define USART2_IRQn         38

static inline void NVIC_EnableIRQ(int irqn) {
    NVIC->ISER[irqn >> 5] = (1UL << (irqn & 0x1F));
}

static inline void NVIC_SetPriority(int irqn, uint32_t priority) {
    NVIC->IP[irqn] = (uint8_t)((priority << 4) & 0xFFU);
}

/* ── Flash — controls wait states for high-speed clocking ────────────── */
#define FLASH_BASE      0x40023C00UL
#define FLASH_ACR       (*((__IO uint32_t *)(FLASH_BASE + 0x00)))

#define FLASH_ACR_LATENCY_5WS   0x5UL   /* 5 wait states for 168 MHz */
#define FLASH_ACR_PRFTEN        (1UL << 8)
#define FLASH_ACR_ICEN          (1UL << 9)
#define FLASH_ACR_DCEN          (1UL << 10)

/* ── SysTick — Cortex-M4 core, ARM DDI0439 §4.4 ──────────────────────── */
#define SYSTICK_BASE    0xE000E010UL

typedef struct {
    __IO uint32_t CTRL;    /* 0x00  Control and status */
    __IO uint32_t LOAD;    /* 0x04  Reload value */
    __IO uint32_t VAL;     /* 0x08  Current value */
    __IO uint32_t CALIB;   /* 0x0C  Calibration */
} SysTick_TypeDef;

#define SysTick     ((SysTick_TypeDef *) SYSTICK_BASE)

#define SYSTICK_CTRL_ENABLE     (1UL << 0)
#define SYSTICK_CTRL_TICKINT    (1UL << 1)
#define SYSTICK_CTRL_CLKSOURCE  (1UL << 2)  /* 1 = processor clock */
#define SYSTICK_CTRL_COUNTFLAG  (1UL << 16)

#endif /* STM32F4XX_H */
