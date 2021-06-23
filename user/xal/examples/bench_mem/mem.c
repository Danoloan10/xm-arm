#include <stdio.h>
#include <xm.h>
#include <irqs.h>

#define LOG(...) do { \
	xmTime_t hw; \
	XM_get_time(XM_HW_CLOCK, &hw); \
	printf("[P%d] [%d] - ", XM_PARTITION_SELF, hw); \
	printf(__VA_ARGS__); \
	printf("\n"); \
} while (0)

#ifdef CONFIG_MMU
#define N_ITERATIONS 1024
#define SHARED_ADDRESS  0x1C000000

xm_u32_t *sem = (xm_u32_t *)  SHARED_ADDRESS;
xm_u32_t *pno = (xm_u32_t *) (SHARED_ADDRESS + 0x100);

xm_u32_t it = 0;
xmTime_t hw[N_ITERATIONS];
xm_u32_t pr[N_ITERATIONS];

#else
#error MMU not enabled
#endif

void CycleStartHandler(trapCtxt_t *ctxt)
{
	if (it < N_ITERATIONS) {
		pr[it] = *pno;
		XM_get_time(XM_HW_CLOCK, (hw + it));
		it++;
	} else {

		if (*sem != XM_PARTITION_SELF)
			return;

		LOG("Start times:\n");

		for (; it > 0; it--) {
			printf("\t[ %d:%d, %d ]\n",
					XM_PARTITION_SELF,
					hw[N_ITERATIONS-it],
					pr[N_ITERATIONS-it]
				);
		}
		printf("\n");

		LOG("Halting...\n");

		*sem = *sem + 1;

		XM_halt_partition(XM_PARTITION_SELF);
	}
}

void PartitionMain(void)
{
#ifdef CONFIG_ARM
	InstallIrqHandler(XAL_XMEXT_TRAP(XM_VT_EXT_CYCLIC_SLOT_START), CycleStartHandler);
#else
#	error Architecture not supported
#endif

	HwSti();

	XM_clear_irqmask(0, (1<<XM_VT_EXT_CYCLIC_SLOT_START));
	
	*sem = 0;

	while(1) { 
		*pno = XM_PARTITION_SELF;		
	}
}
