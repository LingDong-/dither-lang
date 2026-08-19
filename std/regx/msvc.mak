lib:
	cl /EHsc /c impl_cppstd.cpp /Fo:impl_cppstd.h.obj
	lib impl_cppstd.h.obj