#pragma once

#include <new> // for std::align_val_t
#include <cstdio> // for printf()
#include <cstdlib> // for malloc()ºÍaligned_alloc()

#ifdef _MSC_VER
#include <malloc.h> // for _aligned_malloc()ºÍ_aligned_free()
#endif



class TrackNew {
private:
	static inline int numMalloc = 0; 
	static inline int numFree = 0;
	static inline size_t sumSize = 0;
	static inline bool doTrace = false; 
	static inline bool inNew = false;
public:
	static void reset() {
		numMalloc = 0;
		sumSize = 0;
	}
	static void trace(bool b) {
		doTrace = b;
	}
	static void* allocate(std::size_t size, std::size_t align, const char* call) {
		++numMalloc;
		sumSize += size;

		void* p;
		if (align == 0) {
			p = std::malloc(size);
		}
		else {
		#ifdef _MSC_VER
			p = _aligned_malloc(size, align);
		#else
			p = std::aligned_alloc(align, size);
		#endif
		}
		if (doTrace) {
			printf("#%d %s ", numMalloc, call);
			printf("(%zu bytes, ", size);
			if (align > 0) {
				printf("%zu-byte aligned) ", align);
			}
			else {
				printf("def-aligned) ");
			}
			printf("=> %p (total: %zu bytes)\n", (void*)p, sumSize);
		}
		return p;
	}
	static void deallocate(bool align_dealloc, void * p)
	{
		++numFree;
		if (align_dealloc==false)
		{
			std::free(p);
		}
		else
		{
		#ifdef _MSC_VER
			_aligned_free(p);
		#else
			std::free(p); 
		#endif
		}

		if (doTrace) {
			printf("#%d ", numFree);
			if (align_dealloc) {
				printf("aligned)");
			}
			else {
				printf("def-aligned) ");
			}
			printf("=> %p \n", (void*)p);
		}
	}

	static void status() {
		printf("%d allocations for %zu bytes\n", numMalloc, sumSize);
		printf("%d deallocations\n", numFree);
	}
};

[[nodiscard]]  void* operator new (std::size_t size) {
	return TrackNew::allocate(size, 0, "::new");
}

[[nodiscard]]  void* operator new (std::size_t size, std::align_val_t align) {
	return TrackNew::allocate(size, static_cast<std::size_t>(align), "::new aligned");
}

[[nodiscard]]  void* operator new[](std::size_t size) {
	return TrackNew::allocate(size, 0, "::new[]");
}

[[nodiscard]]  void* operator new[](std::size_t size, std::align_val_t align) {
	return TrackNew::allocate(size, static_cast<std::size_t>(align), "::new[] aligned");
}

void operator delete (void* p) noexcept {
	TrackNew::deallocate(false, p);
}

void operator delete (void* p, std::align_val_t) noexcept {
	TrackNew::deallocate(true, p);
}

void operator delete[](void* p) noexcept {
	TrackNew::deallocate(false, p);
}

void operator delete[](void* p, std::align_val_t) noexcept {
	TrackNew::deallocate(true, p);
}

