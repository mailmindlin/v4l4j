//Provided AS-IS, etc.
/**
 * Profiles v4lconvert conversion functions on the current processor.
 * Useful for making estimateCost methods.
 */
#include "libvideo.h"
#include "libvide-palettes.h"
#include "libv4lconvert.h"
#include "libv4lconvert/libv4lconvert-flat.h"
#include "log.h"

size_t numImages = 10;
size_t numIterations = 100;

typedef v4lconvert_buffer* (*imageGenerator)(v4lconvert_converter* converter, size_t index);

typedef struct {
	unsigned long min;
	unsigned long q1;
	unsigned long median;
	unsigned long q3;
	unsigned long max;
	double mean;
	double stdDev;
} ProfileResult;

static void sort(unsigned long[] array, size_t size) {
	//In-place merge sort (modeled after stackoverflow.com/a/15657134/2759984)
	for (unsigned int width = 1; width < len; width *= 2) {
		//TODO finish
		for (unsigned int left = 0; left + width < size; left += width * 2) {
			unsigned int middle = left + width;
			unsigned int right = middle + width;
			if (right > size)
				right = size;
			unsigned int i = left;
			unsigned int j = middle;
			while (i < middle && j < right) {
				unsigned long a = array[i];
				unsigned long b = array[j];
				
			}
		}
	}
}

static ProfileResults* _doProfile(v4lconvert_converter* converter, size_t num_buffers, v4lconvert_buffer* buffers[]) {
	const size_t num_times = num_iterations * num_buffers;
	if (num_times == 0)
		return NULL;
	unsigned long* times = calloc(num_times, sizeof(size_t));
	if (!times)
		return NULL;
	
	unsigned long* timePtr = times;
	for(unsigned int iteration = 0; iteration < numIterations; iteration++) {
		for(unsigned int bufferIdx = 0; bufferIdx < num_buffers; bufferIdx++) {
			v4lconvert_buffer* buffer = buffers[bufferIdx];
			clock_t startTime = clock();
			converter->apply(converter, buffer);
			clock_t endTime = clock();
			
			clock_t timeElapsed = endTime - startTime;
			if (sizeof(clock_t) > sizeof(size_t) && timeElapsed > LONG_MAX)
				timeElapsed = (clock_t) LONG_MAX;
			*timePtr++ = (unsigned long)timeElapsed;
		}
	}
	
	//Calculate median
	sort(times, num_times);
	ProfileResult* result = malloc(sizeof(ProfileResult));
	#define LOOKUP_TIME(frac)\
		({\
			__auto_type candidate1 = times[(num_times + 0) * frac];\
			__auto_type candidate2 = times[(num_times + 1) * frac];\
			(candidate2 - candidate1) / 2 + candidate2;\
		});
	})
	result->min = times[0];
	result->q1 = LOOKUP_TIME(1/4);
	result->median = LOOKUP_TIME(1/2);
	result->q3  = LOOKUP_TIME(3/4);
	result->max = times[num_times - 1];
	{
		//Thanks to stackoverflow.com/a/572091/2759984, stackoverflow.com/a/19370637/2759984
		result->mean = 0;
		result->stddev = 0;
		for(unsigned int i = 0; i < num_times; i++) {
			double delta = (double) times[i] - result->mean;
			result->mean += delta / (double) i;
			result->stddev += delta * ((double) times[i] - mean);
		}
		result->stddev /= (double) num_times;
	}
	free(times);
	return median;
}

int main(unsigned int argc, char** argv) {
	v4lconvert_conversion_type conversionType = 0;
	unsigned int converterId = -1u;
	for (unsigned int i = 0; i < argc; i++) {
		char* arg = argv[i];
		if (arg[0] == '-') {
			switch(arg[1]) {
				case 'i':
					numIterations = atoi(argv[++i]);
					break;
				case 'b':
					numImages = atoi(argv[++i]);
					break;
				case 't':
					v4lconvert_conversion_type = atoi(argv[++i]);
					break;
				case 'c':
					converterId = c;
					break;
				default:
					printf("Unknown switch: %s\n", arg);
					break;
			}
		}
	}
	if (numIterations == 0 || numImages == 0)
		return -1;
	
	if (converterId != -1u && conversionType == 0)
		//Undefined behavior
		return -1;
	
	//TODO profile stuff
}