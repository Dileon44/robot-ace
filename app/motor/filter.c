#include "filter.h"

void TFilter_Calc(TFilter* pFilter) {
	pFilter->output += pFilter->T * (pFilter->input - pFilter->output);
}
