/*
 * multi_heap_4.h
 *
 *  Created on: Oct 13, 2019
 *      Author: Benjami
 */

/*
 * If used freertos: the original function call can also be used for compatibility
 * (for the internal sram memory region: pvPortMalloc, vPortFree, xPortGetFreeHeapSize)
 */

#ifndef __MULTI_HEAP_4_H_
#define __MULTI_HEAP_4_H_

#ifdef __cplusplus
 extern "C" {
#endif

/*-----------------------------------------------------------------------------
  Parameter section (set the parameters)
*/

/*
 * Please setting:
 * - HEAP_NUM : how many heap ram regions are
 * - configTOTAL_HEAP_SIZE : internal sram region heap size (only not used the freertos)
 * - HEAP_REGIONS : memory address and size for regions
 *
 * This example contains 3 regions (stm32f407zet board for external 1MB sram chip with FSMC)
 * - 0: default memory region (internal sram) -> iramMalloc, iramFree, iramGetFreeHeapSize
 * - 1: ccm internal 64kB ram (0x10000000..0x1000FFFF) -> cramMalloc, cramFree, cramGetFreeHeapSize
 * - 2: external 1MB sram (0x68000000..0x680FFFFF) -> eramMalloc, eramFree, eramGetFreeHeapSize
 *   #define HEAP_REGIONS  {{ (uint8_t *) &ucHeap0, sizeof(ucHeap0) },\
                            { (uint8_t *) 0x10000000, 0x10000       },\
                            { (uint8_t *) 0x68000000, 0x100000      }};
 *
 * This example contains 3 regions (stm32f429zit discovery for external 8MB sdram chip with FSMC)
 * - 0: default memory region (internal sram) -> iramMalloc, iramFree, iramGetFreeHeapSize
 * - 1: ccm internal 64kB ram (0x10000000..0x1000FFFF) -> cramMalloc, cramFree, cramGetFreeHeapSize
 * - 2: external 8MB sdram (0xD0000000..0xD07FFFFF) -> eramMalloc, eramFree, eramGetFreeHeapSize
 *   #define HEAP_REGIONS  {{ (uint8_t *) &ucHeap0, sizeof(ucHeap0) },\
                            { (uint8_t *) 0x10000000, 0x10000       },\
                            { (uint8_t *) 0xD0000000, 0x800000      }};
*/

/* Heap region number (1..6) */
#define HEAP_NUM      3

/* region 0 heap static reservation (if not used freertos -> check the free RAM size for setting) */
#ifndef configTOTAL_HEAP_SIZE
#define configTOTAL_HEAP_SIZE    0x8000
#endif

// #define HEAP_0        ucHeap0[configTOTAL_HEAP_SIZE] // internal sram heap reservation if freertos used

/* regions table: adress and size (internal sram region (0), ccmram region (1), external ram region (2) ) */
#define HEAP_REGIONS  {{ (uint8_t *) 0x24000000, 0x80000       },\
                       { (uint8_t *) 0x30000000, 0x48000       },\
                       { (uint8_t *) 0x38000000, 0x10000       }};

/* d1 ram memory (region 1) procedures (*d1Malloc, d1Free, d1GetFreeHeapSize) */
#define d1Malloc(a)        multiPortMalloc(0, a)
#define d1Free(a)          multiPortFree(0, a)
#define d1GetFreeHeapSize(a)  multiPortGetFreeHeapSize(0)
#define d1GetMinimumEverFreeHeapSize multiPortGetMinimumEverFreeHeapSize(0)

/* d2 ram memory (region 2) procedures (*d2Malloc, d2Free, d2GetFreeHeapSize) */
#define d2Malloc(a)        multiPortMalloc(1, a)
#define d2Free(a)          multiPortFree(1, a)
#define d2GetFreeHeapSize(a)  multiPortGetFreeHeapSize(1)
#define d2GetMinimumEverFreeHeapSize multiPortGetMinimumEverFreeHeapSize(1)

/* d3 ram memory (region 3) procedures (*d3Malloc, d3Free, d3GetFreeHeapSize) */
#define d3Malloc(a)        multiPortMalloc(1, a)
#define d3Free(a)          multiPortFree(1, a)
#define d3GetFreeHeapSize(a)  multiPortGetFreeHeapSize(1)
#define d3GetMinimumEverFreeHeapSize multiPortGetMinimumEverFreeHeapSize(1)

/*-----------------------------------------------------------------------------
  Fix section, do not change
*/
void*   multiPortMalloc(uint32_t i, size_t xWantedSize);
void    multiPortFree(uint32_t i, void *pv);
size_t  multiPortGetFreeHeapSize(uint32_t i);
size_t  multiPortGetMinimumEverFreeHeapSize(uint32_t i);

#ifdef __cplusplus
}
#endif

#endif /* __MULTI_HEAP_4_H_ */
