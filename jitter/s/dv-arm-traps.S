/*	dv-arm-traps.S - arm trap handler code for davroska
 *
 *	Copyright 2018 David Haworth
 *
 *	This file is part of davros.
 *
 *	davros is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	davros is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with davros.  If not, see <http://www.gnu.org/licenses/>.
*/
#define DV_ASM	1
#include <arm/h/dv-arm-registers.h>

/* dv_trap_***() - handlers for all unused traps
 *
*/
	.globl	dv_trap_undef
	.extern	dv_catch_undef

	.text

dv_trap_undef:
	msr		cpsr_c, #(DV_ARM_MODE_SVC+DV_ARM_IRQ_DIS+DV_ARM_FIQ_DIS)
	b		dv_catch_undef


	.globl	dv_trap_sbreak
	.extern	dv_catch_sbreak

	.text

dv_trap_sbreak:
	msr		cpsr_c, #(DV_ARM_MODE_SVC+DV_ARM_IRQ_DIS+DV_ARM_FIQ_DIS)
	b		dv_catch_sbreak


	.globl	dv_trap_prefetch_abort
	.extern	dv_catch_prefetch_abort

	.text

dv_trap_prefetch_abort:
	msr		cpsr_c, #(DV_ARM_MODE_SVC+DV_ARM_IRQ_DIS+DV_ARM_FIQ_DIS)
	b		dv_catch_prefetch_abort


	.globl	dv_trap_data_abort
	.extern	dv_catch_data_abort

	.text

dv_trap_data_abort:
	msr		cpsr_c, #(DV_ARM_MODE_SVC+DV_ARM_IRQ_DIS+DV_ARM_FIQ_DIS)
	b		dv_catch_data_abort


	.globl	dv_trap_reserved
	.extern	dv_catch_reserved

	.text

dv_trap_reserved:
	msr		cpsr_c, #(DV_ARM_MODE_SVC+DV_ARM_IRQ_DIS+DV_ARM_FIQ_DIS)
	b		dv_catch_reserved


	.globl	dv_trap_fiq
	.extern	dv_catch_fiq

	.text

dv_trap_fiq:
	msr		cpsr_c, #(DV_ARM_MODE_SVC+DV_ARM_IRQ_DIS+DV_ARM_FIQ_DIS)
	b		dv_catch_fiq

#if 0
	.globl	dv_trap_irq
	.extern	dv_catch_irq

	.text

dv_trap_irq:
	msr		cpsr_c, #(DV_ARM_MODE_SVC+DV_ARM_IRQ_DIS+DV_ARM_FIQ_DIS)
	b		dv_catch_irq
#endif
