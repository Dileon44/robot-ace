#ifndef __FILTER_H
#define __FILTER_H

#define FILTER_DEFAULTS \
	{ 0.0f, 0.0f, 1.0f, TFilter_Calc }

struct SFilter {
	float input;
	float output;
	float T;  // T = T_sample / T_filter
	void (*calc)(volatile struct SFilter*);
};

typedef volatile struct SFilter TFilter;

void TFilter_Calc(TFilter*);

#endif /* __FILTER_H */